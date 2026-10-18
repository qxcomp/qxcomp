#include "wraplabel.h"
#include <qpainter.h>

#ifdef QT3_BUILD
// Qt3: 换行标志名为 WordBreak (0x0800)
static const int kWrapFlag = Qt::WordBreak;
#else
// Qt4+: 换行标志名为 TextWordWrap (0x1000)
static const int kWrapFlag = Qt::TextWordWrap;
#endif

static void enableHeightForWidth(QWidget* w) {
#ifdef QT3_BUILD
    w->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum, TRUE));
#else
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Minimum);
    sp.setHeightForWidth(true);
    w->setSizePolicy(sp);
#endif
}

#ifdef QT3_BUILD
WrapLabel::WrapLabel(QWidget* parent, const char* name)
    : QLabel(parent, name) {
    enableHeightForWidth(this);
}
WrapLabel::WrapLabel(const QString& text, QWidget* parent, const char* name)
    : QLabel(text, parent, name) {
    enableHeightForWidth(this);
}
#else
WrapLabel::WrapLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f) {
    setWordWrap(true);
    enableHeightForWidth(this);
}
WrapLabel::WrapLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
    : QLabel(text, parent, f) {
    setWordWrap(true);
    enableHeightForWidth(this);
}
#endif

int WrapLabel::wrappedHeight(int w) const {
    int inner = w - 2 * margin();
    if (inner < 8) {
        inner = 8;
    }
    QRect r = fontMetrics().boundingRect(0, 0, inner, 100000,
                                         Qt::AlignLeft | Qt::AlignTop | kWrapFlag,
                                         text());
    return r.height() + 2 * margin();
}

int WrapLabel::heightForWidth(int w) const {
    return wrappedHeight(w);
}

QSize WrapLabel::sizeHint() const {
    int w = width();
    if (w <= 0) {
        w = QLabel::sizeHint().width();
    }
    return QSize(w, heightForWidth(w));
}

QSize WrapLabel::minimumSizeHint() const {
    int w = QLabel::minimumSizeHint().width();
    if (w <= 0) {
        w = 1;
    }
    return QSize(w, heightForWidth(w));
}

#ifdef QT3_BUILD
void WrapLabel::drawContents(QPainter* p) {
    int flags = alignment();
    flags &= ~Qt::AlignVertical_Mask;
    flags |= Qt::AlignTop | Qt::WordBreak;
    QRect cr = contentsRect();
    int m = margin();
    QRect tr(cr.x() + m, cr.y() + m, cr.width() - 2 * m, cr.height() - 2 * m);
    p->drawText(tr, flags, text());
}
#endif
