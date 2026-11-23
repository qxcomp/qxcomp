#include "toastwidget.h"
#ifdef QT3_BUILD
#include <qpainter.h>
#else
#include <QPainter>
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

/* 当前显示中的中央 toast（同一父窗口替换，静态指针） */
ToastWidget* ToastWidget::s_center = 0;

void ToastWidget::show(QWidget* parent, const QString& text, int durationMs) {
    ToastWidget* tw = new ToastWidget(parent, text, durationMs, Info, false);
    tw->positionAtBottom();
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
      ), m_text(text), m_timerId(0), m_level(level), m_center(center),
      m_area(parent ? QRect(parent->mapToGlobal(QPoint(0, 0)), parent->size())
                    : qPrimaryAvailableGeometry())
{
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

void ToastWidget::timerEvent(QTimerEvent*) {
    deleteLater();
}

void ToastWidget::mousePressEvent(QMouseEvent*) {
    deleteLater();
}
