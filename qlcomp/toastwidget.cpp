#include "toastwidget.h"
#ifdef QT3_BUILD
#include <qpainter.h>
#else
#include <QPainter>
#endif

/* 当前显示中的中央 toast（同一父窗口替换，静态指针） */
ToastWidget* ToastWidget::s_center = 0;

void ToastWidget::show(QWidget* parent, const QString& text, int durationMs) {
    ToastWidget* tw = new ToastWidget(parent, text, durationMs, Info, false);
    tw->positionAtBottom();
    tw->QWidget::show();
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
    tw->QWidget::show();
    tw->raise();
}

void ToastWidget::positionAtBottom() {
    if (!parentWidget()) return;
    QRect pr = parentWidget()->rect();
#ifdef QT3_BUILD
    int x = (pr.width() - width()) / 2;
    int y = pr.height() - height() - 60;
    move(x, y);
#else
    QPoint pg = parentWidget()->mapToGlobal(QPoint(0, 0));
    int x = pg.x() + (pr.width() - width()) / 2;
    int y = pg.y() + pr.height() - height() - 60;
    move(x, y);
#endif
}

ToastWidget::ToastWidget(QWidget* parent, const QString& text, int durationMs,
                         Level level, bool center)
    : QWidget(parent
#ifdef QT3_BUILD
      , 0, WStyle_StaysOnTop
#else
      , Qt::ToolTip | Qt::FramelessWindowHint
#endif
      ), m_text(text), m_timerId(0), m_level(level), m_center(center)
{
#ifndef QT3_BUILD
    setAttribute(Qt::WA_DeleteOnClose);
    if (center) setAttribute(Qt::WA_ShowWithoutActivating);
#endif
    setFixedSize(center ? 360 : 300, center ? 44 : 36);
    m_timerId = startTimer(durationMs);
}

ToastWidget::~ToastWidget() {
    if (s_center == this) s_center = 0;
}

void ToastWidget::positionAtCenter() {
    QRect area;
    if (parentWidget()) {
        QPoint pg = parentWidget()->mapToGlobal(QPoint(0, 0));
        area = QRect(pg, parentWidget()->size());
    } else {
        area = qPrimaryAvailableGeometry();
    }
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
