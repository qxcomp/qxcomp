#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include "qlcompat34.h"

class ToastWidget : public QWidget {
public:
    /* 级别数值可映射到 SystemTrayIcon::MessageIcon（Info/Warning/Error = 1/2/3） */
    enum Level { Info = 1, Warning = 2, Error = 3 };

    /* 兼容旧行为：底部居中、300x36、无级别样式 */
    static void show(QWidget* parent, const QString& text, int durationMs = 2000);

    /* 新增：窗口中央、360x44、按级别着色；同一父窗口只保留一个实例 */
    static void showCenter(QWidget* parent, const QString& text,
                           Level level = Info, int durationMs = 2500);

    ~ToastWidget();

protected:
    void paintEvent(QPaintEvent* event);
    void timerEvent(QTimerEvent* event);
    void mousePressEvent(QMouseEvent* event);

private:
    ToastWidget(QWidget* parent, const QString& text, int durationMs,
                Level level, bool center);
    void positionAtBottom();
    void positionAtCenter();

    QString m_text;
    int m_timerId;
    Level m_level;
    bool m_center;
    /* 构造时快照调用方窗口的全局矩形。Qt4+ 的 toast 不再以调用方为 QWidget
       parent（mac 除外，mac 用 raise 抑制代替），定位只依赖这份快照。 */
    QRect m_area;
    static ToastWidget* s_center;
};

#endif
