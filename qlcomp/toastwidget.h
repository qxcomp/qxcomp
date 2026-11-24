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
        int zIndex;         /* 宿主在全局 stacking 中的序号（0 = 最底），-1 = 未知 */
        int topIsUs;        /* 1 = 系统最上层窗口属于本进程，-1 = 未知 */
    };

    /* 宿主窗口 z 序快照：只记录“原来在宿主上面/下面的邻居”，
       判违规用相对关系，避免其它窗口开关造成序号平移误报。 */
    struct ZOrderSnapshot {
        int valid;                          /* 0 = 采集失败 */
        unsigned long long self;            /* 宿主顶层窗口 id */
        int index;                          /* 采集时全局下标（仅供日志） */
        unsigned long long above;           /* 紧邻宿主上方，0 = 原来就在最顶 */
        unsigned long long below;           /* 紧邻宿主下方，0 = 原来就在最底 */
        unsigned long long aboveList[64];   /* 原来在宿主之上的窗口 */
        int aboveCount;
        int layer;                          /* 1 = 宿主处于 always-on-top 层 */
        void* macHandle;                    /* mac: toastmac_order_snapshot() 句柄 */
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

    /* === 宿主 z 序检测/恢复 === */
    static int sampleZOrder(QWidget* anchor, WId fallbackWin,
                            ZOrderSnapshot* out, int withHandle);
    static int zOrderViolated(const ZOrderSnapshot& before, const ZOrderSnapshot& now);
    static void releaseZOrder(ZOrderSnapshot* snap);
    static void logZOrder(int isWarn, const char* tag,
                          const ZOrderSnapshot& before, const ZOrderSnapshot& now,
                          int rc);
    void snapshotZOrder(QWidget* anchor);
    void maybeZFix(QWidget* anchor, WId fallbackWin, const char* tag);
    void closeZFix();
    int restoreZOrder();

    QString m_text;
    int m_timerId;
    int m_probeTimerId;   /* +80ms 复检定时器，0 = 未启动 */
    int m_zfixTimerId;    /* z 序恢复后的 +170ms 校验定时器，0 = 未启动 */
    Level m_level;
    bool m_center;
    /* 构造时快照调用方窗口的全局矩形。Qt4+ 的 toast 不再以调用方为 QWidget
       parent（mac 除外，mac 用 raise 抑制代替），定位只依赖这份快照。 */
    QRect m_area;
    WId m_topWin;             /* 宿主顶层窗口 id（0 = 未知），+80ms 复检用 */
    LevelSnapshot m_before;   /* probeBeforeShow() 记录 */
    ZOrderSnapshot m_zBefore; /* snapshotZOrder() 记录 */
    int m_zfixState;          /* 0=off 1=armed 2=restored 3=verified 4=gaveup */
    int m_zfixTries;
    static ToastWidget* s_center;
};

#endif
