#ifndef WRAPLABEL_H
#define WRAPLABEL_H

#include "compat34.h"

// WrapLabel：按当前宽度自适应折行的 QLabel。
// - 打开 height-for-width：布局按实际宽度调用 heightForWidth() 得到折行高度。
// - 宽度跟随容器；最小宽度取“最长词宽”，避免词内断行。
// - 无 Q_OBJECT（无信号槽），规避 Qt3 moc 双分支陷阱。
// - Qt3 的 QLabel 不支持 setWordWrap，故自行在 drawContents 里按 WordBreak 绘制。
class WrapLabel : public QLabel {
public:
#ifdef QT3_BUILD
    WrapLabel(QWidget* parent = 0, const char* name = 0);
    WrapLabel(const QString& text, QWidget* parent = 0, const char* name = 0);
#else
    WrapLabel(QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    WrapLabel(const QString& text, QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
#endif
    int   heightForWidth(int w) const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
protected:
#ifdef QT3_BUILD
    void drawContents(QPainter* p);
#endif
private:
    int wrappedHeight(int w) const;
};

#endif  // WRAPLABEL_H
