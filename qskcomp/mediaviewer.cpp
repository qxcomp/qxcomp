#include "mediaviewer.h"

#include <QBuffer>
#include <QGuiApplication>
#include <QIODevice>
#include <QKeyEvent>
#include <QLineF>
#include <QMovie>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>
#include <QTouchEvent>
#include <QTimer>
#include <QWheelEvent>

namespace
{
    constexpr qreal CLOSE_SIZE = 36.0;
    constexpr qreal CLOSE_MARGIN = 12.0;
    constexpr qreal kTapMovedThreshold = 8.0;
    constexpr qreal kZoomStep = 1.25;    // qltox PhotoCanvas::wheelEvent :325 口径
    constexpr qreal kZoomMax = 8.0;
    constexpr qreal kDblTapZoom = 2.5;   // qltox toggleFitMode 双击换挡口径

    // 背景不透明度（ConfirmPopup s_defaultPanelOpacity 同口径，默认 0.9）
    qreal s_backdropOpacity = 0.9;
}

// ── MediaViewerNode ──

MediaViewerNode::MediaViewerNode()
{
    setRenderHint(QskPaintedNode::Raster);
}

qreal MediaViewerNode::backdropOpacity()
{
    return s_backdropOpacity;
}

void MediaViewerNode::setBackdropOpacity(qreal opacity)
{
    s_backdropOpacity = qBound(0.0, opacity, 1.0);
}

void MediaViewerNode::setData(const QImage& image, const QPointF& drawPos,
    qreal drawW, qreal drawH, const QString& statusText, const QSizeF& size,
    int paintTick)
{
    m_image = image;
    m_drawPos = drawPos;
    m_drawW = drawW;
    m_drawH = drawH;
    m_statusText = statusText;
    m_size = size;
    m_paintTick = paintTick;
}

void MediaViewerNode::triggerUpdate(QQuickWindow* window, const QRectF& rect,
    const QSizeF& size)
{
    update(window, rect, size, nullptr);
}

void MediaViewerNode::paint(QPainter* painter, const QSize& /*size*/, const void*)
{
    const QRectF full(QPointF(0, 0), m_size);
    painter->fillRect(full, QColor(0, 0, 0,
        qRound(s_backdropOpacity * 255.0)));

    // 关闭钮（右上 ✕，MsgEnlarge/PhotoViewer 同位）
    const QRectF cb(CLOSE_MARGIN * 0 + m_size.width() - CLOSE_SIZE
        - CLOSE_MARGIN, CLOSE_MARGIN, CLOSE_SIZE, CLOSE_SIZE);
    painter->setPen(QColor(255, 255, 255, 220));
    painter->setBrush(QColor(0, 0, 0, 120));
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawEllipse(cb);
    painter->drawText(cb, Qt::AlignCenter, QString::fromUtf8("✕"));

    if (!m_image.isNull()) {
        painter->drawImage(QRectF(m_drawPos, QSizeF(m_drawW, m_drawH)),
            m_image);
        return;
    }
    // 字节空/decode 失败：状态文案（pending 或 无法加载）
    painter->setPen(QColor(255, 255, 255, 180));
    painter->drawText(full, Qt::AlignCenter, m_statusText);
}

QskHashValue MediaViewerNode::hash(const void*) const
{
    return qHash(m_paintTick) ^ qHash(int(m_size.width()))
        ^ qHash(int(m_size.height()));
}

// ── MediaViewerCanvas ──

MediaViewerCanvas::MediaViewerCanvas(QQuickItem* parent)
    : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptTouchEvents(true);
    setFlag(ItemHasContents, true);
}

MediaViewerCanvas::~MediaViewerCanvas()
{
    if (m_movie) {
        m_movie->stop();
        m_movie->deleteLater();
        m_movie = nullptr;
    }
}

QImage MediaViewerCanvas::currentFrame() const
{
    if (m_movie) {
        return m_movie->currentImage();
    }
    return m_image;
}

qreal MediaViewerCanvas::fitScale() const
{
    const QImage cur = currentFrame();
    if (cur.isNull() || width() <= 0 || height() <= 0) {
        return 0;
    }
    const qreal fs = qMin(width() / cur.width(), height() / cur.height());
    return qBound<qreal>(0.01, fs, 100.0);
}

QRectF MediaViewerCanvas::displayRect() const
{
    const QImage cur = currentFrame();
    if (cur.isNull()) {
        return QRectF();
    }
    const qreal fs = fitScale() * m_zoom;
    const qreal dw = cur.width() * fs;
    const qreal dh = cur.height() * fs;
    return QRectF((width() - dw) / 2.0 + m_pan.x(),
        (height() - dh) / 2.0 + m_pan.y(), dw, dh);
}

QRectF MediaViewerCanvas::closeRect() const
{
    return QRectF(width() - CLOSE_SIZE - CLOSE_MARGIN, CLOSE_MARGIN,
        CLOSE_SIZE, CLOSE_SIZE);
}

void MediaViewerCanvas::clampPan()
{
    const QImage cur = currentFrame();
    if (cur.isNull()) {
        m_pan = QPointF();
        return;
    }
    const qreal fs = fitScale() * m_zoom;
    const qreal ofx = qMax<qreal>(0.0, cur.width() * fs - width());
    const qreal ofy = qMax<qreal>(0.0, cur.height() * fs - height());
    const qreal nx = (ofx > 0) ? qBound(-ofx, m_pan.x(), 0.0) : 0.0;
    const qreal ny = (ofy > 0) ? qBound(-ofy, m_pan.y(), 0.0) : 0.0;
    m_pan = QPointF(nx, ny);
}

void MediaViewerCanvas::setZoom(qreal zoom)
{
    m_zoom = qBound<qreal>(1.0, zoom, kZoomMax);
    clampPan();
    triggerRepaint();
}

void MediaViewerCanvas::triggerRepaint()
{
    ++m_paintTick;
    m_dirty = true;
    update();
}

void MediaViewerCanvas::setData(const QByteArray& raw, bool isGif,
    bool pending, const QString& statusText)
{
    // 重建内容（重复 setData 覆盖旧内容并复位缩放/平移）
    m_image = QImage();
    m_pan = QPointF();
    m_zoom = 1.0;
    m_statusText = statusText;
    m_showPending = pending;
    if (m_movie) {
        m_movie->stop();
        m_movie->deleteLater();
        m_movie = nullptr;
    }
    if (m_gifBuf) {
        m_gifBuf->deleteLater();
        m_gifBuf = nullptr;
    }
    if (raw.isEmpty()) {
        triggerRepaint();
        return;
    }
    if (isGif) {
        // GIF：行内/qltox QMovie 同款（组件内仍逐帧播放，比 qltox
        // PhotoViewer 静帧首帧更强）
        m_movie = new QMovie(this);
        m_gifBuf = new QBuffer(this);
        m_gifBuf->setData(raw);
        m_gifBuf->open(QIODevice::ReadOnly);
        m_movie->setDevice(m_gifBuf);
        connect(m_movie, &QMovie::frameChanged, this, [this](int) {
            triggerRepaint();
        });
        m_movie->start();
        m_movie->jumpToFrame(0);
    } else {
        m_image.loadFromData(raw);
    }
    triggerRepaint();
}

void MediaViewerCanvas::touchEvent(QTouchEvent* event)
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
            m_lastPanPos = p;
            m_touchMoved = false;
            if (event->points().size() >= 2) {
                const QPointF q = mapFromScene(
                    event->points().last().scenePosition());
                m_pinchStartDist = QLineF(p, q).length();
                m_pinchStartZoom = m_zoom;
                m_touchPinching = (m_pinchStartDist > 0);
            } else {
                m_touchPinching = false;
            }
        }
        event->accept();
    } else if (event->type() == QEvent::TouchUpdate) {
        if (!event->points().isEmpty()) {
            if (m_touchPinching && event->points().size() >= 2) {
                const QPointF p = mapFromScene(
                    event->points().first().scenePosition());
                const QPointF q = mapFromScene(
                    event->points().last().scenePosition());
                const qreal d = QLineF(p, q).length();
                if (m_pinchStartDist > 0) {
                    setZoom(m_pinchStartZoom * d / m_pinchStartDist);
                }
            } else if (m_touchPinching) {
                // 两指状态解除但 pinch 未结束：待 TouchEnd 复位
            } else {
                const QPointF p = mapFromScene(
                    event->points().first().scenePosition());
                m_touchMoved = m_touchMoved
                    || QLineF(p, m_lastPanPos).length() > kTapMovedThreshold;
                m_pan += p - m_lastPanPos;
                m_lastPanPos = p;
                clampPan();
                triggerRepaint();
            }
        }
        event->accept();
    } else if (event->type() == QEvent::TouchEnd) {
        if (m_touchPinching) {
            m_touchPinching = false;
        } else if (!m_touchMoved) {
            // 单指未移动：命中 ✕ 或图片外（背景）即关闭（MsgEnlarge 口径）
            const QPointF p = m_lastPanPos;
            if (closeRect().contains(p) || !displayRect().contains(p)) {
                Q_EMIT closeRequested();
            }
        }
        event->accept();
    } else {
        QQuickItem::touchEvent(event);
    }
}

void MediaViewerCanvas::mousePressEvent(QMouseEvent* event)
{
    if (m_ignoreFirstTouch) {
        m_ignoreFirstTouch = false;
        event->accept();
        return;
    }
    m_dragging = true;
    m_mouseMoved = false;
    m_lastPanPos = event->position();
    event->accept();
}

void MediaViewerCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        const QPointF p = event->position();
        m_mouseMoved = m_mouseMoved
            || QLineF(p, m_lastPanPos).length() > kTapMovedThreshold;
        m_pan += p - m_lastPanPos;
        m_lastPanPos = p;
        clampPan();
        triggerRepaint();
    }
    event->accept();
}

void MediaViewerCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_dragging) {
        m_dragging = false;
        if (!m_mouseMoved) {
            const QPointF p = event->position();
            if (closeRect().contains(p) || !displayRect().contains(p)) {
                Q_EMIT closeRequested();
            }
        }
    }
    event->accept();
}

void MediaViewerCanvas::mouseDoubleClickEvent(QMouseEvent* event)
{
    // 双击 fit/2.5 切换（qltox PhotoCanvas::toggleFitMode :378 口径）
    if (m_zoom > 1.0) {
        m_zoom = 1.0;
        m_pan = QPointF();
    } else {
        m_zoom = kDblTapZoom;
        clampPan();
    }
    triggerRepaint();
    event->accept();
}

void MediaViewerCanvas::wheelEvent(QWheelEvent* event)
{
    const qreal dy = event->angleDelta().y();
    if (dy != 0) {
        setZoom(m_zoom * (dy > 0 ? kZoomStep : 1.0 / kZoomStep));
    }
    event->accept();
}

void MediaViewerCanvas::keyPressEvent(QKeyEvent* event)
{
    // Esc（桌面）与 Back（Android 返回键）双键关闭（MsgEnlarge 仅 Esc，
    // 此处补 Back —— 触摸端零 Escape 键位）
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        Q_EMIT closeRequested();
        event->accept();
        return;
    }
    QQuickItem::keyPressEvent(event);
}

QSGNode* MediaViewerCanvas::updatePaintNode(QSGNode* oldNode,
    UpdatePaintNodeData*)
{
    auto* node = static_cast<MediaViewerNode*>(oldNode);
    if (!node) {
        node = new MediaViewerNode();
    }
    if (m_dirty) {
        const QImage cur = currentFrame();
        const QRectF d = displayRect();
        // 空内容态文案：pending 显示 statusText（例如"⏳ 下载中…"），
        // 否则"无法加载"（字节解码不出图）
        QString status = m_statusText;
        if (status.isEmpty()) {
            status = m_showPending ? QString::fromUtf8("⏳ 下载中…")
                : QString::fromUtf8("无法加载");
        }
        node->setData(cur, d.topLeft(), d.width(), d.height(),
            cur.isNull() ? status : QString(), size(), m_paintTick);
        node->triggerUpdate(window(),
            QRectF(QPointF(0, 0), size()), size());
        m_dirty = false;
    }
    return node;
}

// ── MediaViewerPopup ──

QRectF MediaViewerPopup::viewerParentRect(QQuickItem* parent)
{
    // dialogpopup.cpp dialogParentRect 同款
    if (!parent) {
        return {};
    }
    if (auto* w = parent->window()) {
        return QRectF(QPointF(), w->size());
    }
    return QRectF(-parent->x(), -parent->y(),
        parent->width(), parent->height());
}

MediaViewerPopup::MediaViewerPopup(QQuickItem* parent)
    : QskPopup(parent)
{
    setModal(true);
    setOverlay(true);                       // 全屏压暗遮罩
    setPopupFlag(QskPopup::DeleteOnClose, false);
    setPolishOnResize(true);
    setPolishOnParentResize(true);

    m_canvas = new MediaViewerCanvas(this);
    connect(m_canvas, &MediaViewerCanvas::closeRequested, this,
        [this]() { close(); });
}

void MediaViewerPopup::keyPressEvent(QKeyEvent* event)
{
    // modal 弹层持焦点时键盘事件先落弹层（canvas 是兜底路径）：
    // Esc（桌面）+ Back（Android 返回键）都关闭
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        close();
        event->accept();
        return;
    }
    QskPopup::keyPressEvent(event);
}

MediaViewerPopup* MediaViewerPopup::show(QQuickItem* parent,
    const QByteArray& raw, bool isGif, bool pending, const QString& statusText)
{
    auto* popup = new MediaViewerPopup(parent);
    connect(popup, &QskPopup::closed, popup, &QObject::deleteLater);
    QTimer::singleShot(0, popup, [popup, raw, isGif, pending, statusText]() {
        popup->open();
        popup->updateGeometry();
        popup->m_canvas->setData(raw, isGif, pending, statusText);
    });
    return popup;
}

void MediaViewerPopup::updateLayout()
{
    updateGeometry();
}

void MediaViewerPopup::updateGeometry()
{
    const auto parentRect = viewerParentRect(parentItem());
    if (parentRect.isEmpty()) {
        return;
    }
    // 全屏铺满（RawDataPopup QskPopup 铺满口径）
    setGeometry(parentRect);
    // raw QQuickItem 无 setGeometry(QRectF) —— x/width/height 分开
    m_canvas->setPosition(QPointF(0, 0));
    m_canvas->setWidth(parentRect.width());
    m_canvas->setHeight(parentRect.height());
    m_canvas->triggerRepaint();
}
