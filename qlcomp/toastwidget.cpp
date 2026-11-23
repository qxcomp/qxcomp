#include "toastwidget.h"
#ifdef QT3_BUILD
#include <qpainter.h>
#else
#include <QPainter>
#endif
#include <stdlib.h>
#if defined(Q_OS_LINUX) && !defined(QT3_BUILD)
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#if defined(Q_OS_MAC) && !defined(QT3_BUILD) && QT_VERSION < 0x050000
/* Qt4/Cocoa 的 QWidget::raise() 会调用 SetFrontProcessWithOptions()，把整个 app
   提到前台——后台 app 弹 toast 时，它的窗口就是这样被升到最上层的。
   Qt4-mac 导出了这个开关：临时关掉，只保留 [window orderFront]，toast 仍按
   NSScreenSaverWindowLevel 显示在最上层，但不再激活 app。
   符号不存在时（其他 Qt 构建）弱链接为 0，判空跳过。 */
extern void qt_mac_set_raise_process(bool) __attribute__((weak));
#define TOAST_HAS_MAC_RAISE_HOOK 1
#endif

namespace {

const char* toastPlatformName()
{
#if defined(Q_OS_MAC)
    return "mac";
#elif defined(Q_OS_WIN)
    return "win";
#elif defined(Q_OS_LINUX)
    return "linux";
#else
    return "other";
#endif
}

/* TOAST_PROBE=1 打印每次取样（含正常）；TOAST_PROBE_SILENT=1 一律静音 */
bool toastProbeVerbose()
{
    const char* v = ::getenv("TOAST_PROBE");
    if (v != 0 && v[0] != 0 && v[0] != '0') {
        return true;
    }
    return false;
}

bool toastProbeSilent()
{
    const char* v = ::getenv("TOAST_PROBE_SILENT");
    if (v != 0 && v[0] != 0 && v[0] != '0') {
        return true;
    }
    return false;
}

/* Qt3 用 topLevelWidget()，Qt4+ 用 window() */
QWidget* toastTopLevel(QWidget* w)
{
    if (w == 0) {
        return 0;
    }
#ifdef QT3_BUILD
    return w->topLevelWidget();
#else
    return w->window();
#endif
}

#if defined(Q_OS_LINUX) && !defined(QT3_BUILD)
/* 读窗口上的 CARDINAL 属性，成功返回 1 */
int x11Cardinal(Display* dpy, Window win, const char* name, unsigned long* value)
{
    Atom type = 0;
    int format = 0;
    unsigned long nitems = 0;
    unsigned long after = 0;
    unsigned char* data = 0;
    if (XGetWindowProperty(dpy, win, XInternAtom(dpy, name, False), 0, 1, False,
                           XA_CARDINAL, &type, &format, &nitems, &after,
                           &data) != Success) {
        return 0;
    }
    if (data == 0) {
        return 0;
    }
    if (nitems != 1) {
        XFree(data);
        return 0;
    }
    *value = *(unsigned long*)data;
    XFree(data);
    return 1;
}

/* _NET_ACTIVE_WINDOW 是否本进程：1=是，0=否，-1=取不到 */
int x11ActiveIsUs(Display* dpy)
{
    Atom type = 0;
    int format = 0;
    unsigned long nitems = 0;
    unsigned long after = 0;
    unsigned char* data = 0;
    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy),
                           XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False),
                           0, 1, False, XA_WINDOW, &type, &format,
                           &nitems, &after, &data) != Success) {
        return -1;
    }
    if (data == 0) {
        return -1;
    }
    Window active = 0;
    if (nitems == 1) {
        active = *(Window*)data;
    }
    XFree(data);
    if (active == 0) {
        return 0;   /* 无激活窗口（桌面/无焦点） */
    }
    unsigned long pid = 0;
    if (x11Cardinal(dpy, active, "_NET_WM_PID", &pid) == 0) {
        return -1;
    }
    return (pid == (unsigned long)getpid()) ? 1 : 0;
}

/* 宿主顶层在 _NET_CLIENT_LIST_STACKING 中的序号（0 = 最下层，越大越靠上） */
int x11StackIndex(Display* dpy, Window self, int* topIsUs)
{
    Atom type = 0;
    int format = 0;
    unsigned long nitems = 0;
    unsigned long after = 0;
    unsigned char* data = 0;
    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy),
                           XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False),
                           0, 4096, False, XA_WINDOW, &type, &format,
                           &nitems, &after, &data) != Success) {
        return -1;
    }
    if (data == 0) {
        return -1;
    }
    Window* list = (Window*)data;
    int index = -1;
    for (unsigned long i = 0; i < nitems; ++i) {
        if (list[i] == self) {
            index = (int)i;
        }
    }
    if (nitems > 0) {
        unsigned long pid = 0;
        if (x11Cardinal(dpy, list[nitems - 1], "_NET_WM_PID", &pid) != 0) {
            *topIsUs = (pid == (unsigned long)getpid()) ? 1 : 0;
        }
    }
    XFree(data);
    return index;
}
#endif

} // namespace

/* 当前显示中的中央 toast（同一父窗口替换，静态指针） */
ToastWidget* ToastWidget::s_center = 0;

void ToastWidget::show(QWidget* parent, const QString& text, int durationMs) {
    ToastWidget* tw = new ToastWidget(parent, text, durationMs, Info, false);
    tw->positionAtBottom();
    tw->probeBeforeShow(parent);
#ifdef TOAST_HAS_MAC_RAISE_HOOK
    if (qt_mac_set_raise_process) {
        qt_mac_set_raise_process(false);
    }
#endif
    tw->QWidget::show();   /* Qt 会给 ToolTip 自动 raise()，必须包在抑制区间内 */
#ifdef TOAST_HAS_MAC_RAISE_HOOK
    if (qt_mac_set_raise_process) {
        qt_mac_set_raise_process(true);
    }
#endif
    tw->probeCompare(parent, 0, "after-show");
}

void ToastWidget::showCenter(QWidget* parent, const QString& text,
                             Level level, int durationMs) {
    if (s_center) {
        ToastWidget* old = s_center;
        s_center = 0;
        old->deleteLater();   /* 析构会再清一次 s_center，已是 0 无影响 */
    }
    ToastWidget* tw = new ToastWidget(parent, text, durationMs, level, true);
    s_center = tw;
    tw->positionAtCenter();
    tw->probeBeforeShow(parent);
#ifdef TOAST_HAS_MAC_RAISE_HOOK
    if (qt_mac_set_raise_process) {
        qt_mac_set_raise_process(false);
    }
#endif
    tw->QWidget::show();
#ifdef QT3_BUILD
    tw->raise();
#else
    /* 应用在后台时不 raise:raise 可能被 WM 当成激活/置顶 */
    if (parent != 0 && parent->isActiveWindow()) {
        tw->raise();
    }
#endif
#ifdef TOAST_HAS_MAC_RAISE_HOOK
    if (qt_mac_set_raise_process) {
        qt_mac_set_raise_process(true);
    }
#endif
    tw->probeCompare(parent, 0, "after-show");
}

void ToastWidget::positionAtBottom() {
#ifdef QT3_BUILD
    if (!parentWidget()) { return; }
    QRect pr = parentWidget()->rect();
    int x = (pr.width() - width()) / 2;
    int y = pr.height() - height() - 60;
    move(x, y);
#else
    int x = m_area.x() + (m_area.width() - width()) / 2;
    int y = m_area.y() + m_area.height() - height() - 60;
    move(x, y);
#endif
}

ToastWidget::ToastWidget(QWidget* parent, const QString& text, int durationMs,
                         Level level, bool center)
    : QWidget(
#ifdef QT3_BUILD
        parent, 0, WStyle_StaysOnTop
#elif defined(Q_OS_MAC)
        /* mac：保持 parent（与 Qt4 自带 QToolTip 一致，ToolTip 是 NSPanel，
           不进任务栏/窗口列表），激活问题由上面的 raise_process 抑制解决 */
        parent, Qt::ToolTip | Qt::FramelessWindowHint
#else
        /* X11/Windows：不能挂到调用方窗口——WM_TRANSIENT_FOR / owner window
           会让 toast 被 map/raise 时把宿主窗口一起抬到最上层 */
        0, Qt::ToolTip | Qt::FramelessWindowHint
#endif
      ), m_text(text), m_timerId(0), m_probeTimerId(0), m_level(level),
      m_center(center),
      m_area(parent ? QRect(parent->mapToGlobal(QPoint(0, 0)), parent->size())
                    : qPrimaryAvailableGeometry())
{
    m_topWin = 0;
    m_before.valid = 0;
    m_before.appActive = 0;
    m_before.parentActive = 0;
    m_before.toastVisible = -1;
    m_before.zIndex = -1;
    m_before.topIsUs = -1;
#ifndef QT3_BUILD
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);   /* center/bottom 都设:显示不激活 */
    setFocusPolicy(Qt::NoFocus);
#ifdef Q_OS_LINUX
    /* 无父窗口时 Qt 仍会把 ToolTip 设成 transient-for(group leader)，部分 WM
       会因此带起主窗；彻底去掉这个 hint */
    setAttribute(Qt::WA_X11BypassTransientForHint);
#endif
#else
    setFocusPolicy(NoFocus);                       /* Qt3:仅去焦点策略 */
#endif
    setFixedSize(center ? 360 : 300, center ? 44 : 36);
    m_timerId = startTimer(durationMs);
}

ToastWidget::~ToastWidget() {
    if (s_center == this) s_center = 0;
}

void ToastWidget::positionAtCenter() {
#ifdef QT3_BUILD
    QRect area;
    if (parentWidget()) {
        QPoint pg = parentWidget()->mapToGlobal(QPoint(0, 0));
        area = QRect(pg, parentWidget()->size());
    } else {
        area = qPrimaryAvailableGeometry();
    }
#else
    QRect area = m_area;   /* 与 positionAtBottom 一致，构造时快照 */
#endif
    int x = area.x() + (area.width() - width()) / 2;
    int y = area.y() + (area.height() - height()) / 2 - 40;
    move(x, y);
}

void ToastWidget::probeBeforeShow(QWidget* anchor) {
    m_before = probeLevel(anchor, 0, 0, 1);
    QWidget* top = toastTopLevel(anchor);
    m_topWin = (top != 0) ? top->winId() : 0;
    m_probeTimerId = startTimer(80);   /* +80ms 复检：WM/Cocoa 的激活是异步的 */
}

void ToastWidget::probeCompare(QWidget* anchor, WId fallbackWin, const char* tag) {
    LevelSnapshot now = probeLevel(anchor, fallbackWin, this, 1);
    if (m_before.valid == 0 || now.valid == 0) {
        return;
    }
    const bool parentRaised = (m_before.parentActive == 0 && now.parentActive == 1);
    const bool appStolen = (m_before.appActive == 0 && now.appActive == 1);
    const bool hidden = (now.toastVisible == 0);
    const bool stackRaised = (m_before.zIndex >= 0 && now.zIndex >= 0
                              && now.zIndex > m_before.zIndex);
    if (parentRaised || appStolen || hidden || stackRaised) {
        logLevel(1, tag, m_before, now);
        return;
    }
    if (toastProbeVerbose()) {
        logLevel(0, tag, m_before, now);
    }
}

void ToastWidget::probeAtClose(const char* tag) {
    LevelSnapshot now = probeLevel(parentWidget(), m_topWin, this, 0);
    if (m_before.valid != 0 && m_before.appActive == 0 && now.appActive == 1) {
        logLevel(1, tag, m_before, now);
        return;
    }
    if (toastProbeVerbose()) {
        logLevel(0, tag, m_before, now);
    }
}

ToastWidget::LevelSnapshot ToastWidget::probeLevel(QWidget* anchor, WId fallbackWin,
                                                   QWidget* toast, int withStack) {
    LevelSnapshot s;
    s.valid = 0;
    s.appActive = (qApp->activeWindow() != 0) ? 1 : 0;
    s.parentActive = 0;
    s.toastVisible = -1;
    s.zIndex = -1;
    s.topIsUs = -1;

    QWidget* top = toastTopLevel(anchor);
    WId topId = (top != 0) ? top->winId() : fallbackWin;
    if (top != 0) {
        s.valid = 1;
        s.parentActive = top->isActiveWindow() ? 1 : 0;
    }
    if (toast != 0) {
        s.valid = 1;
        s.toastVisible = toast->isVisible() ? 1 : 0;
    }
#if defined(Q_OS_LINUX) && !defined(QT3_BUILD)
    if (withStack != 0 && topId != 0) {
        Display* dpy = qX11Display();
        if (dpy != 0) {
            int activeIsUs = x11ActiveIsUs(dpy);   /* 真实前台判断，胜过 Qt 缓存 */
            if (activeIsUs >= 0) {
                s.appActive = activeIsUs;
            }
            int topIsUs = -1;
            int index = x11StackIndex(dpy, (Window)topId, &topIsUs);
            if (index >= 0) {
                s.valid = 1;
                s.zIndex = index;
                s.topIsUs = topIsUs;
            }
        }
    }
#else
    Q_UNUSED(withStack);
    Q_UNUSED(topId);
#endif
    return s;
}

void ToastWidget::logLevel(int isWarn, const char* tag,
                           const LevelSnapshot& before, const LevelSnapshot& now) {
    if (toastProbeSilent()) {
        return;
    }
    const char* fmt = "[toast] probe %s plat=%s before(app=%d,parent=%d,z=%d)"
                      " now(app=%d,parent=%d,z=%d,vis=%d,top=%d)";
    if (isWarn != 0) {
        qWarning(fmt, tag, toastPlatformName(),
                 before.appActive, before.parentActive, before.zIndex,
                 now.appActive, now.parentActive, now.zIndex,
                 now.toastVisible, now.topIsUs);
    } else {
        qDebug(fmt, tag, toastPlatformName(),
               before.appActive, before.parentActive, before.zIndex,
               now.appActive, now.parentActive, now.zIndex,
               now.toastVisible, now.topIsUs);
    }
}

void ToastWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
#ifndef QT3_BUILD
    p.setRenderHint(QPainter::Antialiasing);
#endif
#ifdef QT3_BUILD
    QColor bg(0x1a, 0x1a, 0x1a);
#else
    QColor bg(0x1a, 0x1a, 0x1a, 0xE0);
#endif
    p.setBrush(bg);
    p.setPen(Qt::NoPen);
#ifdef QT3_BUILD
    p.drawRoundRect(rect(), 25, 25);
#else
    p.drawRoundedRect(rect(), 25, 25);
#endif
    if (m_center) {
        QColor accent(0x4a, 0x90, 0xe2);
        if (m_level == Warning) accent = QColor(0xf5, 0xa6, 0x23);
        else if (m_level == Error) accent = QColor(0xe7, 0x4c, 0x3c);
        p.setBrush(accent);
#ifdef QT3_BUILD
        p.drawRoundRect(QRect(0, 8, 4, height() - 16), 2, 2);
#endif
#ifndef QT3_BUILD
        p.drawRoundedRect(QRect(0, 8, 4, height() - 16), 2, 2);
#endif
#ifdef QT3_BUILD
        p.setPen(Qt::white);   /* 恢复画笔，Qt3 下 setBrush 不影响 pen 但显式更稳 */
#else
        p.setPen(Qt::white);
#endif
    }
    p.setPen(Qt::white);
    QRect tr = rect();
    if (m_center) tr = QRect(16, 0, width() - 32, height());
    p.drawText(tr, Qt::AlignCenter, m_text);
}

void ToastWidget::timerEvent(QTimerEvent* event) {
    if (event->timerId() == m_probeTimerId) {
        killTimer(m_probeTimerId);
        m_probeTimerId = 0;
        probeCompare(parentWidget(), m_topWin, "+80ms");
        return;
    }
    if (event->timerId() != m_timerId) {
        return;
    }
    probeAtClose("close-expired");
    deleteLater();
}

void ToastWidget::mousePressEvent(QMouseEvent*) {
    probeAtClose("close-click");
    deleteLater();
}
