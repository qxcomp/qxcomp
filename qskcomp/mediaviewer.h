#ifndef MEDIA_VIEWER_POPUP_H
#define MEDIA_VIEWER_POPUP_H

#include <QskPaintedNode.h>
#include <QskPopup.h>
#include <QImage>
#include <QQuickItem>
#include <QPointF>
#include <QRectF>

class QMovie;
class QBuffer;

// ── qskcomp 通用媒体查看器 ──
// 机制沿用 dialogpopup（同窗口 QskPopup、非阻塞、Android 安全、DeleteOnClose），
// 画布与手势走 MsgEnlargeOverlay 同款 QskPaintedNode 路径。
// 字节进（QByteArray）：调用方自己负责字节来源/下载，组件不含业务。
//
// 能力（qltox PhotoViewer 对位端口）：等比 fit、拖拽平移、双指捏合/滚轮
// 缩放、双击 fit/2.5 切换、GIF 逐帧动画（QBuffer+QMovie）、字节缺省时
// pending 文案（调用方就绪后再 setData 上真内容）、Esc/✕/背景点按关闭。
//
// class MediaViewerNode : public QskPaintedNode {...};

class MediaViewerNode : public QskPaintedNode
{
public:
    MediaViewerNode();
    // 背景不透明度（0..1，默认 0.9 —— dialogpopup 面板默认同口径）；
    // 全局默认两面弹层共享（ConfirmPopup defaultPanelOpacity 同机制）
    static qreal  backdropOpacity();
    static void   setBackdropOpacity(qreal opacity);
    // 最终绘制参数由 canvas 直填（paint 纯绘制，无 GUI 对象）
    void setData(const QImage& image, const QPointF& drawPos,
                 qreal drawW, qreal drawH, const QString& statusText,
                 const QSizeF& size, int paintTick);
    void triggerUpdate(QQuickWindow* window, const QRectF& rect,
                       const QSizeF& size);
    void paint(QPainter*, const QSize& size, const void*) override;
    QskHashValue hash(const void*) const override;

private:
    QImage m_image;
    QPointF m_drawPos;
    qreal m_drawW = 0;
    qreal m_drawH = 0;
    QString m_statusText;
    QSizeF m_size;
    int m_paintTick = 0;
};

class MediaViewerCanvas : public QQuickItem
{
    Q_OBJECT
public:
    explicit MediaViewerCanvas(QQuickItem* parent = nullptr);
    ~MediaViewerCanvas() override;

    // 内容进料（重复调用覆盖旧内容并复位缩放）：
    //   raw     字节（图片原字节 / GIF 原字节；空 = status 状态态）
    //   isGif   —— true 则以 QBuffer+QMovie 逐帧播放（组件内自管）
    //   pending —— raw 空 且 true：显示 status 文案（如"⏳ 下载中…"）；
    //              空且 false：显示"无法加载"
    void setData(const QByteArray& raw, bool isGif, bool pending,
                 const QString& statusText);

    // 布局/几何变化后由 popup 调用：fitScale 依存视口，需重算重绘
    void triggerRepaint();

Q_SIGNALS:
    void closeRequested();

protected:
    void touchEvent(QTouchEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    QImage currentFrame() const;
    qreal fitScale() const;     // 当前尺寸下等比满屏系数（zoom=1 基准）
    QRectF displayRect() const; // 当前帧实际绘制区（fit*zoom + clamp pan）
    QRectF closeRect() const;
    void clampPan();
    void setZoom(qreal zoom);

    QImage m_image;
    QMovie* m_movie = nullptr;
    QBuffer* m_gifBuf = nullptr;
    QString m_statusText;
    bool m_showPending = false;
    // 缩放/平移（zoom=1 → fit；wheel 1.25/0.8；双击 1↔2.5；pinch 距离比；
    // pan 经 clampPan 进内容边界）
    qreal m_zoom = 1.0;
    QPointF m_pan;
    // 交互状态
    QPointF m_lastPanPos;
    qreal m_pinchStartDist = 0;
    qreal m_pinchStartZoom = 1.0;
    bool m_touchPinching = false;
    bool m_touchMoved = false;
    bool m_dragging = false;
    bool m_mouseMoved = false;
    bool m_ignoreFirstTouch = false;
    bool m_dirty = true;
    int m_paintTick = 0;
};

// 全屏媒体查看弹出层（kisk 核 ConfirmPopup 同机制）。静态入口：
//   show(parent, 字节, isGif, pending, statusText) → 创建/pending 打开；
//   关闭即 DeleteOnClose（closed→deleteLater），或持有返回值后续 setData 递料。
class MediaViewerPopup : public QskPopup
{
    Q_OBJECT
public:
    static MediaViewerPopup* show(QQuickItem* parent, const QByteArray& raw,
                                  bool isGif, bool pending,
                                  const QString& statusText);

    // 画布访问（pending 递料：外部 mediaUpdated 到达后 setData 新字节）
    MediaViewerCanvas* canvas() const { return m_canvas; }

    static QRectF viewerParentRect(QQuickItem* parent);

protected:
    void updateLayout() override;
    // modal 弹层持焦点时键盘事件先落弹层：Esc/Back 同样关闭
    // （canvas 与弹层双保险，Qt::Key_Back 是 Android 返回键）
    void keyPressEvent(QKeyEvent* event) override;

private:
    MediaViewerPopup(QQuickItem* parent);
    void updateGeometry();

    MediaViewerCanvas* m_canvas = nullptr;
};

#endif // MEDIA_VIEWER_POPUP_H
