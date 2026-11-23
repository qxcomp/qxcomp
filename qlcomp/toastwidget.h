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
    /* 显示前后的窗口层级快照，仅用于自检日志（不参与布局/行为） */
    struct LevelSnapshot {
        int valid;          /* 0 = 本平台不支持探测 */
        int appActive;      /* 1 = 本进程在前台 */
        int parentActive;   /* 1 = 宿主窗口被激活 */
        int toastVisible;   /* -1 = 不适用 */
        int zIndex;         /* 宿主在 _NET_CLIENT_LIST_STACKING 中的序号，-1 = 未知 */
        int topIsUs;        /* 1 = 系统最上层窗口属于本进程，-1 = 未知 */
    };

    ToastWidget(QWidget* parent, const QString& text, int durationMs,
                Level level, bool center);
    void positionAtBottom();
    void positionAtCenter();
    void probeBeforeShow(QWidget* anchor);
    void probeCompare(QWidget* anchor, WId fallbackWin, const char* tag);
    void probeAtClose(const char* tag);
    static LevelSnapshot probeLevel(QWidget* anchor, WId fallbackWin,
                                    QWidget* toast, int withStack);
    static void logLevel(int isWarn, const char* tag,
                         const LevelSnapshot& before, const LevelSnapshot& now);

    QString m_text;
    int m_timerId;
    int m_probeTimerId;   /* +80ms 复检定时器，0 = 未启动 */
    Level m_level;
    bool m_center;
    /* 构造时快照调用方窗口的全局矩形。Qt4+ 的 toast 不再以调用方为 QWidget
       parent（mac 除外，mac 用 raise 抑制代替），定位只依赖这份快照。 */
    QRect m_area;
    WId m_topWin;             /* 宿主顶层窗口 id（0 = 未知），+80ms 复检用 */
    LevelSnapshot m_before;   /* probeBeforeShow() 记录 */
    static ToastWidget* s_center;
};

#endif
