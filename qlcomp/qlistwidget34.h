#ifndef QLISTWIDGET34_H
#define QLISTWIDGET34_H

/*
 * QListWidget34 —— Qt4 QListWidget 的 Qt3 兼容层
 *
 * Qt4/5/6：typedef 原生 QListWidget，item 类型即 QListWidgetItem（零包装）。
 * Qt3    ：QListBox 没有 QListWidget 的 item 对象模型，故用组合方式实现：
 *          QListWidget34 : QWidget（内部持有一个 QListBox）
 *          QListWidgetItem : QListBoxItem（自绘：图标/文本/对齐/颜色/勾选框）
 *          方法名、枚举值、返回语义均对齐 Qt4 QListWidget。
 *
 * 无法实现的部分（Qt3 无 model/view 框架）：
 *   model()/setModel()/indexFromItem()/itemFromIndex()/QModelIndex 系列
 *   setItemWidget()/itemWidget()/removeItemWidget()（per-item QWidget）
 *   openPersistentEditor()/closePersistentEditor()/itemDelegate()
 *   iconMode/gridSize/wrapping/movement（QListBox 无图标网格布局）
 * 不支持 Qt4 信号语法（Qt3 moc 无法处理双分支头中的 Q_OBJECT），
 * 改用静态 connect 助手：
 *   QListWidget34::connectItemClicked(w, recv, SLOT(...))
 *   ... connectItemDoubleClicked / connectItemPressed / connectItemActivated
 *   ... connectCurrentRowChanged / connectItemSelectionChanged
 * 注：Qt3 端槽函数不要声明 item 参数（Qt3 传 QListBoxItem 指针或 int）。
 */

#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qfont.h>
#include <qcolor.h>
#include <qvariant.h>

#ifdef QT3_BUILD

#include <qlistbox.h>

class QListWidget34;

class QListWidgetItem : public QListBoxItem {
public:
    /* Qt4 同名枚举，取值一致 */
    enum ItemDataRole {
        DisplayRole = 0, DecorationRole = 1, EditRole = 2, ToolTipRole = 3,
        StatusTipRole = 4, WhatsThisRole = 5, FontRole = 6,
        TextAlignmentRole = 7, BackgroundRole = 8, ForegroundRole = 9,
        CheckStateRole = 10, SizeHintRole = 13
    };
    enum CheckState { Unchecked = 0, PartiallyChecked = 1, Checked = 2 };
    enum ItemFlag {
        NoItemFlags = 0,
        ItemIsSelectable = 1, ItemIsEditable = 2, ItemIsDragEnabled = 4,
        ItemIsDropEnabled = 8, ItemIsUserCheckable = 16,
        ItemIsEnabled = 32, ItemIsTristate = 64
    };

    explicit QListWidgetItem(QListWidget34* list = 0, int type = 0);
    QListWidgetItem(const QString& text, QListWidget34* list = 0, int type = 0);
    QListWidgetItem(const QListWidgetItem& other);
    ~QListWidgetItem();

    QString     text() const;
    void        setText(const QString& text);
    QPixmap     icon() const;
    void        setIcon(const QPixmap& icon);
    QVariant    data(int role) const;
    void        setData(int role, const QVariant& value);
    int         flags() const;
    void        setFlags(int flags);
    CheckState  checkState() const;
    void        setCheckState(CheckState state);
    bool        isHidden() const;
    void        setHidden(bool hide);
    bool        isSelected() const;
    void        setSelected(bool select);
    QString     toolTip() const;
    void        setToolTip(const QString& tip);
    QFont       font() const;
    void        setFont(const QFont& font);
    int         textAlignment() const;
    void        setTextAlignment(int alignment);
    QColor      textColor() const;
    void        setTextColor(const QColor& color);
    QColor      backgroundColor() const;
    void        setBackgroundColor(const QColor& color);
    QListWidget34* listWidget() const;
    int         type() const;
    QListWidgetItem* clone() const;
    bool        operator<(const QListWidgetItem& other) const;

    /* QListBoxItem 接口 */
    virtual int  height(const QListBox* lb) const;
    virtual int  width(const QListBox* lb) const;

protected:
    virtual void paint(QPainter* p);

private:
    friend class QListWidget34;
    void syncFromData();
    void syncToData();

    QListWidget34* m_owner;      /* Qt4 视角的父列表 */
    int      m_type;
    QPixmap  m_icon;
    QVariant m_data[32];
    int      m_flags;
    CheckState m_check;
    bool     m_hidden;
    QFont    m_font;
    int      m_align;
    QColor   m_textColor;
    QColor   m_bgColor;
    bool     m_hasTextColor;
    bool     m_hasBgColor;
};

class QListWidget34 : public QWidget {
public:
    /* Qt4 同名枚举，取值一致 */
    enum SelectionMode {
        NoSelection = 0, SingleSelection = 1, MultiSelection = 2,
        ExtendedSelection = 3, ContiguousSelection = 4
    };
    enum SortOrder { AscendingOrder = 0, DescendingOrder = 1 };
    enum MatchFlag {
        MatchExactly = 0x00000, MatchContains = 0x00001,
        MatchStartsWith = 0x00002, MatchEndsWith = 0x00003,
        MatchCaseSensitive = 0x00010, MatchCaseInsensitive = 0x00000
    };
    enum ScrollHint {
        EnsureVisible = 0, PositionAtTop = 1, PositionAtBottom = 2,
        PositionAtCenter = 3
    };

    explicit QListWidget34(QWidget* parent = 0);

    /* 项管理 */
    void addItem(const QString& label);
    void addItem(QListWidgetItem* item);
    void addItems(const QStringList& labels);
    void insertItem(int row, const QString& label);
    void insertItem(int row, QListWidgetItem* item);
    void insertItems(int row, const QStringList& labels);
    int  count() const;
    void clear();
    QListWidgetItem* item(int row) const;
    QListWidgetItem* takeItem(int row);
    int  row(const QListWidgetItem* item) const;

    /* 当前项 */
    int  currentRow() const;
    void setCurrentRow(int row);
    QListWidgetItem* currentItem() const;
    void setCurrentItem(QListWidgetItem* item);

    /* 查询 */
    QStringList findItems(const QString& text, int flags) const;
    QListWidgetItem* itemAt(const QPoint& p) const;
    QRect visualItemRect(const QListWidgetItem* item) const;
    void sortItems(SortOrder order = AscendingOrder);
    QStringList selectedItems() const;
    bool isItemSelected(const QListWidgetItem* item) const;
    void setItemSelected(const QListWidgetItem* item, bool select);

    /* 选择模式 / 滚动 */
    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode mode);
    void scrollToItem(QListWidgetItem* item, ScrollHint hint = EnsureVisible);

    /* Qt3 信号 → Qt4 语义（静态助手，无 moc） */
    static void connectItemClicked(QListWidget34* w, const QObject* recv, const char* slot);
    static void connectItemDoubleClicked(QListWidget34* w, const QObject* recv, const char* slot);
    static void connectItemPressed(QListWidget34* w, const QObject* recv, const char* slot);
    static void connectItemActivated(QListWidget34* w, const QObject* recv, const char* slot);
    static void connectCurrentRowChanged(QListWidget34* w, const QObject* recv, const char* slot);
    static void connectItemSelectionChanged(QListWidget34* w, const QObject* recv, const char* slot);

    /* 底层 Qt3 控件（需要 Qt3 专有能力时使用） */
    QListBox* listBox() const { return m_list; }

private:
    QListBox* m_list;
};

#else /* QT4+ */

#include <qlistwidget.h>

typedef QListWidget QListWidget34;

#endif

#endif /* QLISTWIDGET34_H */
