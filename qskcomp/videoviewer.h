#ifndef VIDEO_VIEWER_POPUP_H
#define VIDEO_VIEWER_POPUP_H

#include <QskPaintedNode.h>
#include <QskPopup.h>
#include <QImage>
#include <QQuickItem>
#include <QPointF>
#include <QRectF>
#include <QSizeF>

class QMediaPlayer;
class QAudioOutput;
class QTemporaryFile;
class QVideoSink;

// ── qskcomp 通用视频查看器（媒体类型分型组件之三）──
// 机制同 dialogpopup（QskPopup 同窗口、非阻塞、DeleteOnClose 后关），
// canvas 走 MsgEnlargeOverlay 同款 QskPaintedNode 路径。字节进（调用
// 方管下载源）。内嵌播放：QMediaPlayer + QAudioOutput + QVideoSink，
// videoFrameChanged → QVideoFrame::toImage() → 光栅重绘；
// pending（字节缺）→ status 文案；递料即自动 play
//（qltox pendingPlay mainwindow.cpp:4350 口径）。tap 画面=暂停/继续。
// 注：宿主工程须链接 Qt6::Multimedia；QVideoFrame 旋正依 rotationAngle。
class VideoViewerNode : public QskPaintedNode
{
public:
    VideoViewerNode();
    // 背景不透明度（MediaViewerNode 同口径独立份，默认 0.9）
    static qreal  backdropOpacity();
    static void   setBackdropOpacity(qreal opacity);

    void setData(const QImage& frame, bool playing, qint64 positionMs,
                 qint64 durationMs, const QString& statusText,
                 const QSizeF& size, int paintTick);
    void triggerUpdate(QQuickWindow* window, const QRectF& rect,
                       const QSizeF& size);
    void paint(QPainter*, const QSize& size, const void*) override;
    QskHashValue hash(const void*) const override;

private:
    QImage m_frame;
    bool m_playing = false;
    qint64 m_positionMs = 0;
    qint64 m_durationMs = 0;
    QString m_statusText;
    QSizeF m_size;
    int m_paintTick = 0;
};

class VideoViewerCanvas : public QQuickItem
{
    Q_OBJECT
public:
    explicit VideoViewerCanvas(QQuickItem* parent = nullptr);
    ~VideoViewerCanvas() override;

    // raw 空 + pending=true → status 态；递料（pending → 非 pending）即
    // temp file → setSource → 自动 play；重复 setData → 重播
    void setData(const QByteArray& raw, const QString& mime,
                 bool pending, const QString& statusText);

    // 布局/几何变化后由 popup 调用（fitScale 依存视口）
    void triggerRepaint();

Q_SIGNALS:
    void closeRequested();

protected:
    void touchEvent(QTouchEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    void playPauseToggle();

    QMediaPlayer* m_player = nullptr;
    QAudioOutput* m_output = nullptr;
    QVideoSink* m_sink = nullptr;
    QTemporaryFile* m_tmpFile = nullptr;
    QImage m_frame;
    QString m_statusText;
    bool m_pending = false;
    bool m_playing = false;
    qint64 m_positionMs = 0;
    qint64 m_durationMs = 0;
    bool m_ignoreFirstTouch = false;
    bool m_dirty = true;
    int m_paintTick = 0;
};

class VideoViewerPopup : public QskPopup
{
    Q_OBJECT
public:
    static VideoViewerPopup* show(QQuickItem* parent, const QByteArray& raw,
                                  const QString& mime, bool pending,
                                  const QString& statusText);

    // 画布访问（pending 递料：mediaUpdated 到达后 setData 新字节 → 自动播）
    VideoViewerCanvas* canvas() const { return m_canvas; }

protected:
    void updateLayout() override;
    void keyPressEvent(QKeyEvent* event) override;   // Esc/Back 双键关闭

private:
    VideoViewerPopup(QQuickItem* parent);
    void updateGeometry();

    VideoViewerCanvas* m_canvas = nullptr;
};

#endif // VIDEO_VIEWER_POPUP_H
