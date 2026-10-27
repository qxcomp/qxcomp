#include "videoviewer.h"

#include <QAudioOutput>
#include <QDir>
#include <QKeyEvent>
#include <QLineF>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>
#include <QTemporaryFile>
#include <QTimer>
#include <QTouchEvent>
#include <QTransform>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoSink>

namespace
{
    constexpr qreal CLOSE_SIZE = 36.0;
    constexpr qreal CLOSE_MARGIN = 12.0;
    constexpr qreal kTapMovedThreshold = 8.0;

    // 背景不透明度（MediaViewerNode 同口径独立份，默认 0.9）
    qreal s_backdropOpacity = 0.9;

    QRectF closeRectFor(const QSizeF& size)
    {
        return QRectF(size.width() - CLOSE_SIZE - CLOSE_MARGIN, CLOSE_MARGIN,
            CLOSE_SIZE, CLOSE_SIZE);
    }

    // m:ss / m:ss（qltox mediaDurationLabel 口径）
    QString durLabel(qint64 ms)
    {
        const qint64 total = qMax<qint64>(0, ms / 1000);
        return QString::fromUtf8("%1:%2")
            .arg(total / 60).arg((int)(total % 60), 2, 10, QLatin1Char('0'));
    }

    QString videoSuffix(const QString& mime)
    {
        if (mime.contains(QStringLiteral("quicktime"))) {
            return QStringLiteral(".mov");
        }
        if (mime.contains(QStringLiteral("webm"))) {
            return QStringLiteral(".webm");
        }
        if (mime.contains(QStringLiteral("matroska"))) {
            return QStringLiteral(".mkv");
        }
        return QStringLiteral(".mp4");
    }
}

// ── VideoViewerNode ──

VideoViewerNode::VideoViewerNode()
{
    setRenderHint(QskPaintedNode::Raster);
}

qreal VideoViewerNode::backdropOpacity()
{
    return s_backdropOpacity;
}

void VideoViewerNode::setBackdropOpacity(qreal opacity)
{
    s_backdropOpacity = qBound(0.0, opacity, 1.0);
}

void VideoViewerNode::setData(const QImage& frame, bool playing,
    qint64 positionMs, qint64 durationMs, const QString& statusText,
    const QSizeF& size, int paintTick)
{
    m_frame = frame;
    m_playing = playing;
    m_positionMs = positionMs;
    m_durationMs = durationMs;
    m_statusText = statusText;
    m_size = size;
    m_paintTick = paintTick;
}

void VideoViewerNode::triggerUpdate(QQuickWindow* window, const QRectF& rect,
    const QSizeF& size)
{
    update(window, rect, size, nullptr);
}

void VideoViewerNode::paint(QPainter* painter, const QSize& /*size*/,
    const void*)
{
    const QRectF full(QPointF(0, 0), m_size);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(full, QColor(0, 0, 0,
        qRound(s_backdropOpacity * 255.0)));

    // 关闭钮（左上 ✕，其余组件同位）
    const QRectF cb = closeRectFor(m_size);
    painter->setPen(QColor(255, 255, 255, 220));
    painter->setBrush(QColor(0, 0, 0, 120));
    painter->drawEllipse(cb);
    painter->drawText(cb, Qt::AlignCenter, QString::fromUtf8("✕"));

    if (!m_frame.isNull()) {
        // 等比满屏（无缩放/平移 —— 预览口径）
        const qreal iw = m_frame.width();
        const qreal ih = m_frame.height();
        if (iw > 0 && ih > 0) {
            const qreal fs = qMin(m_size.width() / iw, m_size.height() / ih);
            const qreal dw = iw * fs;
            const qreal dh = ih * fs;
            painter->drawImage(QRectF((m_size.width() - dw) / 2.0,
                (m_size.height() - dh) / 2.0, dw, dh), m_frame);
        }
        // 底部进度条：22px 半透明带 + 3px 白填充线 + 时长 m:ss
        const QRectF bar(12.0, m_size.height() - 34.0,
            m_size.width() - 24.0, 22.0);
        painter->fillRect(bar, QColor(0, 0, 0, 140));
        if (m_durationMs > 0) {
            painter->fillRect(QRectF(bar.x(), bar.bottom() - 3.0,
                bar.width()
                    * qreal(qBound<qint64>(0, m_positionMs, m_durationMs))
                    / qreal(m_durationMs), 3.0),
                QColor(255, 255, 255, 200));
        }
        painter->setPen(QColor(255, 255, 255, 230));
        painter->drawText(bar, Qt::AlignRight | Qt::AlignVCenter,
            durLabel(m_positionMs) + QString::fromUtf8(" / ")
            + durLabel(m_durationMs));
        if (!m_playing) {
            // 暂停/播完 → 中央 ▶（tap 画面恢复）
            painter->setPen(QColor(255, 255, 255, 230));
            painter->drawText(full, Qt::AlignCenter,
                QString::fromUtf8("▶"));
        }
        return;
    }
    // 帧空：状态文案（canvas 侧已并入 pending 口径，缺省=无法加载）
    painter->setPen(QColor(255, 255, 255, 180));
    painter->drawText(full, Qt::AlignCenter,
        m_statusText.isEmpty() ? QString::fromUtf8("无法加载")
        : m_statusText);
}

QskHashValue VideoViewerNode::hash(const void*) const
{
    return qHash(m_paintTick) ^ qHash(int(m_size.width()))
        ^ qHash(int(m_size.height()));
}

// ── VideoViewerCanvas ──

VideoViewerCanvas::VideoViewerCanvas(QQuickItem* parent)
    : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptTouchEvents(true);
    setFlag(ItemHasContents, true);

    m_player = new QMediaPlayer(this);
    m_output = new QAudioOutput(this);
    m_player->setAudioOutput(m_output);
    m_sink = new QVideoSink(this);
    m_player->setVideoSink(m_sink);

    // 解码帧（GUI 线程信号）→ toImage（+ rotationAngle 旋正）→ 重绘
    connect(m_sink, &QVideoSink::videoFrameChanged, this,
        [this](const QVideoFrame& frame) {
            if (!frame.isValid()) {
                return;
            }
            QImage img = frame.toImage();
            const int rot = frame.rotationAngle();
            if (rot != 0 && !img.isNull()) {
                img = img.transformed(QTransform().rotate(rot));
            }
            m_frame = img;
            triggerRepaint();
        });
    connect(m_player, &QMediaPlayer::positionChanged, this,
        [this](qint64 p) {
            m_positionMs = p;
            triggerRepaint();
        });
    connect(m_player, &QMediaPlayer::durationChanged, this,
        [this](qint64 d) {
            m_durationMs = d;
            triggerRepaint();
        });
    connect(m_player, &QMediaPlayer::playbackStateChanged, this,
        [this](QMediaPlayer::PlaybackState state) {
            m_playing = (state == QMediaPlayer::PlayingState);
            triggerRepaint();
        });
}

VideoViewerCanvas::~VideoViewerCanvas()
{
    if (m_player) {
        m_player->stop();
    }
    if (m_tmpFile) {
        m_tmpFile->close();
        delete m_tmpFile;
        m_tmpFile = nullptr;
    }
}

void VideoViewerCanvas::setData(const QByteArray& raw, const QString& mime,
    bool pending, const QString& statusText)
{
    m_pending = pending;
    m_statusText = statusText;
    if (raw.isEmpty()) {
        triggerRepaint();
        return;
    }
    if (m_tmpFile) {
        m_tmpFile->close();
        delete m_tmpFile;
        m_tmpFile = nullptr;
    }
    m_tmpFile = new QTemporaryFile(
        QDir::tempPath() + QStringLiteral("/qsk_vid_XXXXXX")
        + videoSuffix(mime), this);
    m_tmpFile->setAutoRemove(true);
    if (!m_tmpFile->open()) {
        delete m_tmpFile;
        m_tmpFile = nullptr;
        return;
    }
    m_tmpFile->write(raw);
    m_tmpFile->flush();
    m_frame = QImage();          // 清旧帧防上一条残留
    m_dirty = true;
    m_player->setSource(QUrl::fromLocalFile(m_tmpFile->fileName()));
    m_player->play();            // 打开即自动播（拍板口径）
    triggerRepaint();
}

void VideoViewerCanvas::playPauseToggle()
{
    if (m_frame.isNull()) {
        return;
    }
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
    }
}

void VideoViewerCanvas::triggerRepaint()
{
    ++m_paintTick;
    m_dirty = true;
    update();
}

void VideoViewerCanvas::touchEvent(QTouchEvent* event)
{
    if (event->type() == QEvent::TouchBegin) {
        if (m_ignoreFirstTouch) {
            m_ignoreFirstTouch = false;
            event->accept();
            return;
        }
        if (!event->points().isEmpty()) {
            const QPointF p = mapFromScene(
                event->points().first().scenePosition());
            if (closeRectFor(size()).contains(p)) {
                Q_EMIT closeRequested();
                event->accept();
                return;
            }
            // 画面区 tap → 播放/暂停（bg 关闭口径与 MediaViewer 不同：
            // 视频查看器背景 tap 不关闭，防误触中断播放）
            if (!m_frame.isNull()) {
                playPauseToggle();
            }
        }
        event->accept();
    } else {
        QQuickItem::touchEvent(event);
    }
}

void VideoViewerCanvas::mousePressEvent(QMouseEvent* event)
{
    if (m_ignoreFirstTouch) {
        m_ignoreFirstTouch = false;
        event->accept();
        return;
    }
    const QPointF p = event->position();
    if (closeRectFor(size()).contains(p)) {
        Q_EMIT closeRequested();
        event->accept();
        return;
    }
    if (!m_frame.isNull()) {
        playPauseToggle();
    }
    event->accept();
}

void VideoViewerCanvas::keyPressEvent(QKeyEvent* event)
{
    // Esc（桌面）与 Back（Android 返回键）双键关闭
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        Q_EMIT closeRequested();
        event->accept();
        return;
    }
    QQuickItem::keyPressEvent(event);
}

QSGNode* VideoViewerCanvas::updatePaintNode(QSGNode* oldNode,
    UpdatePaintNodeData*)
{
    auto* node = static_cast<VideoViewerNode*>(oldNode);
    if (!node) {
        node = new VideoViewerNode();
    }
    if (m_dirty) {
        QString status = m_statusText;
        if (status.isEmpty()) {
            status = m_pending ? QString::fromUtf8("⏳ 下载中…")
                : QString::fromUtf8("无法加载");
        }
        node->setData(m_frame, m_playing, m_positionMs, m_durationMs,
            m_frame.isNull() ? status : QString(), size(), m_paintTick);
        node->triggerUpdate(window(), QRectF(QPointF(0, 0), size()), size());
        m_dirty = false;
    }
    return node;
}

// ── VideoViewerPopup ──

VideoViewerPopup::VideoViewerPopup(QQuickItem* parent)
    : QskPopup(parent)
{
    setModal(true);
    setOverlay(true);                       // 全屏压暗遮罩
    setPopupFlag(QskPopup::DeleteOnClose, false);
    setPolishOnResize(true);
    setPolishOnParentResize(true);

    m_canvas = new VideoViewerCanvas(this);
    connect(m_canvas, &VideoViewerCanvas::closeRequested, this,
        [this]() { close(); });
}

void VideoViewerPopup::keyPressEvent(QKeyEvent* event)
{
    // modal 弹层持焦点时键盘事件先落弹层（canvas 是兜底路径）
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        close();
        event->accept();
        return;
    }
    QskPopup::keyPressEvent(event);
}

VideoViewerPopup* VideoViewerPopup::show(QQuickItem* parent,
    const QByteArray& raw, const QString& mime, bool pending,
    const QString& statusText)
{
    auto* popup = new VideoViewerPopup(parent);
    connect(popup, &QskPopup::closed, popup, &QObject::deleteLater);
    QTimer::singleShot(0, popup, [popup, raw, mime, pending, statusText]() {
        popup->open();
        popup->updateGeometry();
        popup->m_canvas->setData(raw, mime, pending, statusText);
    });
    return popup;
}

void VideoViewerPopup::updateLayout()
{
    updateGeometry();
}

void VideoViewerPopup::updateGeometry()
{
    // MediaViewerPopup::viewerParentRect 同款（全屏铺满）
    if (!parentItem()) {
        return;
    }
    QRectF parentRect;
    if (auto* w = parentItem()->window()) {
        parentRect = QRectF(QPointF(), w->size());
    } else {
        parentRect = QRectF(-parentItem()->x(), -parentItem()->y(),
            parentItem()->width(), parentItem()->height());
    }
    if (parentRect.isEmpty()) {
        return;
    }
    setGeometry(parentRect);
    m_canvas->setPosition(QPointF(0, 0));
    m_canvas->setWidth(parentRect.width());
    m_canvas->setHeight(parentRect.height());
    m_canvas->triggerRepaint();
}
