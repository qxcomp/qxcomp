#include "qlistwidget34.h"

#ifdef QT3_BUILD

#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qlayout.h>

/* ==================== QListWidgetItem ==================== */

QListWidgetItem::QListWidgetItem(QListWidget34* list, int type)
    : QListBoxItem(0)  /* 0：不自动插入，由 QListWidget34 显式插入 */,
      m_owner(list), m_type(type), m_flags(ItemIsSelectable | ItemIsEnabled),
      m_check(Unchecked), m_hidden(false), m_align(Qt::AlignLeft | Qt::AlignVCenter),
      m_hasTextColor(false), m_hasBgColor(false) {
}

QListWidgetItem::QListWidgetItem(const QString& text, QListWidget34* list, int type)
    : QListBoxItem(0)  /* 0：不自动插入，由 QListWidget34 显式插入 */,
      m_owner(list), m_type(type), m_flags(ItemIsSelectable | ItemIsEnabled),
      m_check(Unchecked), m_hidden(false), m_align(Qt::AlignLeft | Qt::AlignVCenter),
      m_hasTextColor(false), m_hasBgColor(false) {
    setText(text);
}

QListWidgetItem::QListWidgetItem(const QListWidgetItem& other)
    : QListBoxItem(0),
      m_owner(0), m_type(other.m_type), m_icon(other.m_icon),
      m_flags(other.m_flags), m_check(other.m_check), m_hidden(other.m_hidden),
      m_font(other.m_font), m_align(other.m_align),
      m_textColor(other.m_textColor), m_bgColor(other.m_bgColor),
      m_hasTextColor(other.m_hasTextColor), m_hasBgColor(other.m_hasBgColor) {
    setText(other.text());
    for (int i = 0; i < 32; ++i) { m_data[i] = other.m_data[i]; }
}

QListWidgetItem::~QListWidgetItem() {
}

QString QListWidgetItem::text() const {
    return QListBoxItem::text();
}

void QListWidgetItem::setText(const QString& text) {
    QListBoxItem::setText(text);
    syncToData();
}

QPixmap QListWidgetItem::icon() const { return m_icon; }

void QListWidgetItem::setIcon(const QPixmap& icon) {
    m_icon = icon;
    m_data[DecorationRole] = icon;
}

QVariant QListWidgetItem::data(int role) const {
    if (role < 0 || role >= 32) { return QVariant(); }
    if (role == DisplayRole) { return QVariant(text()); }
    if (role == DecorationRole) { return QVariant(m_icon); }
    if (role == CheckStateRole) { return QVariant((int)m_check); }
    return m_data[role];
}

void QListWidgetItem::setData(int role, const QVariant& value) {
    if (role < 0 || role >= 32) { return; }
    m_data[role] = value;
    syncFromData();
}

int QListWidgetItem::flags() const { return m_flags; }

void QListWidgetItem::setFlags(int flags) {
    m_flags = flags;
    setSelectable((flags & ItemIsSelectable) != 0);
}

QListWidgetItem::CheckState QListWidgetItem::checkState() const { return m_check; }

void QListWidgetItem::setCheckState(CheckState state) {
    m_check = state;
    m_data[CheckStateRole] = QVariant((int)state);
    if (listBox()) { listBox()->triggerUpdate(false); }
}

bool QListWidgetItem::isHidden() const { return m_hidden; }

void QListWidgetItem::setHidden(bool hide) {
    m_hidden = hide;
    if (listBox()) { listBox()->triggerUpdate(true); }
}

bool QListWidgetItem::isSelected() const { return QListBoxItem::isSelected(); }

void QListWidgetItem::setSelected(bool select) {
    if (m_owner) { m_owner->setItemSelected(this, select); }
}

QString QListWidgetItem::toolTip() const {
    return m_data[ToolTipRole].toString();
}

void QListWidgetItem::setToolTip(const QString& tip) {
    m_data[ToolTipRole] = QVariant(tip);
}

QFont QListWidgetItem::font() const { return m_font; }
void QListWidgetItem::setFont(const QFont& font) { m_font = font; }
int QListWidgetItem::textAlignment() const { return m_align; }
void QListWidgetItem::setTextAlignment(int alignment) { m_align = alignment; }
QColor QListWidgetItem::textColor() const { return m_textColor; }

void QListWidgetItem::setTextColor(const QColor& color) {
    m_textColor = color;
    m_hasTextColor = true;
}

QColor QListWidgetItem::backgroundColor() const { return m_bgColor; }

void QListWidgetItem::setBackgroundColor(const QColor& color) {
    m_bgColor = color;
    m_hasBgColor = true;
}

QListWidget34* QListWidgetItem::listWidget() const { return m_owner; }
int QListWidgetItem::type() const { return m_type; }

QListWidgetItem* QListWidgetItem::clone() const {
    return new QListWidgetItem(*this);
}

bool QListWidgetItem::operator<(const QListWidgetItem& other) const {
    return text() < other.text();
}

int QListWidgetItem::height(const QListBox* lb) const {
    if (m_hidden) { return 0; }
    QFont f = (m_font == QFont()) ? (lb ? lb->font() : QFont()) : m_font;
    QFontMetrics fm(f);
    int h = fm.height() + 4;
    if (!m_icon.isNull() && m_icon.height() + 4 > h) { h = m_icon.height() + 4; }
    return h;
}

int QListWidgetItem::width(const QListBox* lb) const {
    QFont f = (m_font == QFont()) ? (lb ? lb->font() : QFont()) : m_font;
    QFontMetrics fm(f);
    int w = fm.width(QListBoxItem::text()) + 8;
    if (!m_icon.isNull()) { w += m_icon.width() + 4; }
    if (m_flags & ItemIsUserCheckable) { w += 20; }
    return w;
}

void QListWidgetItem::syncToData() {
    m_data[DisplayRole] = QVariant(QListBoxItem::text());
}

void QListWidgetItem::syncFromData() {
    if (m_data[DisplayRole].isValid()) {
        QListBoxItem::setText(m_data[DisplayRole].toString());
    }
    if (m_data[DecorationRole].isValid()) {
        m_icon = m_data[DecorationRole].toPixmap();
    }
    if (m_data[CheckStateRole].isValid()) {
        m_check = (CheckState)m_data[CheckStateRole].toInt();
    }
    if (m_data[FontRole].isValid()) {
        m_font = m_data[FontRole].toFont();
    }
    if (m_data[TextAlignmentRole].isValid()) {
        m_align = m_data[TextAlignmentRole].toInt();
    }
    if (listBox()) { listBox()->triggerUpdate(true); }
}

void QListWidgetItem::paint(QPainter* p) {
    if (m_hidden) { return; }
    /* QListBox 传入的 painter 已经平移到本项左上角，宽度取列表 viewport 宽 */
    QListBox* box = listBox();
    int w = box ? box->viewport()->width() : 200;
    int h = height(box);
    int x = 4;

    if (m_hasBgColor) { p->fillRect(0, 0, w, h, m_bgColor); }
    if (m_flags & ItemIsUserCheckable) {
        int by = (h - 12) / 2;
        p->setPen(QPen(Qt::black, 1));
        p->drawRect(x, by, 12, 12);
        if (m_check == Checked || m_check == PartiallyChecked) {
            p->drawLine(x + 2, by + 6, x + 5, by + 9);
            p->drawLine(x + 5, by + 9, x + 10, by + 3);
        }
        x += 18;
    }
    if (!m_icon.isNull()) {
        p->drawPixmap(x, (h - m_icon.height()) / 2, m_icon);
        x += m_icon.width() + 4;
    }
    if (m_hasTextColor) { p->setPen(QPen(m_textColor)); }
    if (m_font != QFont()) { p->setFont(m_font); }
    int flags = m_align;
    p->drawText(x, 0, w - x - 2, h, flags, QListBoxItem::text());
}

/* ==================== QListWidget34 ==================== */

QListWidget34::QListWidget34(QWidget* parent) : QWidget(parent) {
    m_list = new QListBox(this);
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setMargin(0);
    lay->setSpacing(0);
    lay->addWidget(m_list);
    setFocusProxy(m_list);
}

void QListWidget34::addItem(const QString& label) {
    QListWidgetItem* item = new QListWidgetItem(label, this);
    m_list->insertItem(item);
}

void QListWidget34::addItem(QListWidgetItem* item) {
    if (!item) { return; }
    m_list->insertItem(item);
}

void QListWidget34::addItems(const QStringList& labels) {
    for (int i = 0; i < (int)labels.count(); ++i) {
        addItem(labels[i]);
    }
}

void QListWidget34::insertItem(int row, const QString& label) {
    QListWidgetItem* item = new QListWidgetItem(label, this);
    m_list->insertItem(item, row);
}

void QListWidget34::insertItem(int row, QListWidgetItem* item) {
    if (!item) { return; }
    m_list->insertItem(item, row);
}

void QListWidget34::insertItems(int row, const QStringList& labels) {
    for (int i = 0; i < (int)labels.count(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(labels[i], this);
        m_list->insertItem(item, row + i);
    }
}

int QListWidget34::count() const { return (int)m_list->count(); }

void QListWidget34::clear() { m_list->clear(); }

QListWidgetItem* QListWidget34::item(int row) const {
    return (QListWidgetItem*)m_list->item(row);
}

QListWidgetItem* QListWidget34::takeItem(int row) {
    QListBoxItem* it = m_list->item(row);
    if (!it) { return 0; }
    m_list->takeItem(it);
    return (QListWidgetItem*)it;
}

int QListWidget34::row(const QListWidgetItem* item) const {
    return m_list->index((QListBoxItem*)item);
}

int QListWidget34::currentRow() const {
    return m_list->currentItem();
}

void QListWidget34::setCurrentRow(int row) {
    if (row < 0) { m_list->clearSelection(); return; }
    m_list->setCurrentItem(row);
}

QListWidgetItem* QListWidget34::currentItem() const {
    int r = m_list->currentItem();
    return (r < 0) ? 0 : (QListWidgetItem*)m_list->item(r);
}

void QListWidget34::setCurrentItem(QListWidgetItem* item) {
    if (!item) { return; }
    m_list->setCurrentItem(item);
}

QStringList QListWidget34::findItems(const QString& text, int flags) const {
    QStringList result;
    bool cs = (flags & MatchCaseSensitive) != 0;
    int mode = flags & 0x0f;
    for (int i = 0; i < (int)m_list->count(); ++i) {
        QString t = m_list->text(i);
        bool hit = false;
        if (mode == MatchContains) {
            hit = (t.find(text, 0, cs) >= 0);
        } else if (mode == MatchStartsWith) {
            hit = cs ? t.startsWith(text) : (t.lower().startsWith(text.lower()));
        } else if (mode == MatchEndsWith) {
            hit = cs ? t.endsWith(text) : (t.lower().endsWith(text.lower()));
        } else {
            hit = cs ? (t == text) : (t.lower() == text.lower());
        }
        if (hit) { result.append(t); }
    }
    return result;
}

QListWidgetItem* QListWidget34::itemAt(const QPoint& p) const {
    return (QListWidgetItem*)m_list->itemAt(p);
}

QRect QListWidget34::visualItemRect(const QListWidgetItem* item) const {
    if (!item) { return QRect(); }
    return m_list->itemRect((QListBoxItem*)item);
}

void QListWidget34::sortItems(SortOrder order) {
    m_list->sort(order == AscendingOrder);
}

QStringList QListWidget34::selectedItems() const {
    QStringList result;
    for (int i = 0; i < (int)m_list->count(); ++i) {
        if (m_list->isSelected(i)) { result.append(m_list->text(i)); }
    }
    return result;
}

bool QListWidget34::isItemSelected(const QListWidgetItem* item) const {
    if (!item) { return false; }
    return m_list->isSelected((QListBoxItem*)item);
}

void QListWidget34::setItemSelected(const QListWidgetItem* item, bool select) {
    if (!item) { return; }
    m_list->setSelected((QListBoxItem*)item, select);
}

QListWidget34::SelectionMode QListWidget34::selectionMode() const {
    switch (m_list->selectionMode()) {
    case QListBox::NoSelection: return NoSelection;
    case QListBox::Multi:       return MultiSelection;
    case QListBox::Extended:    return ExtendedSelection;
    default:                    return SingleSelection;
    }
}

void QListWidget34::setSelectionMode(SelectionMode mode) {
    switch (mode) {
    case NoSelection:       m_list->setSelectionMode(QListBox::NoSelection); break;
    case MultiSelection:    m_list->setSelectionMode(QListBox::Multi); break;
    case ExtendedSelection: m_list->setSelectionMode(QListBox::Extended); break;
    default:                m_list->setSelectionMode(QListBox::Single); break;
    }
}

void QListWidget34::scrollToItem(QListWidgetItem* item, ScrollHint hint) {
    if (!item) { return; }
    int r = row(item);
    if (r < 0) { return; }
    if (hint == PositionAtTop) {
        m_list->setTopItem(r);
    }
    m_list->setCurrentItem(r);
    m_list->ensureCurrentVisible();
}

void QListWidget34::connectItemClicked(QListWidget34* w, const QObject* recv, const char* slot) {
    if (w) { QObject::connect(w->m_list, SIGNAL(clicked(QListBoxItem*)), recv, slot); }
}

void QListWidget34::connectItemDoubleClicked(QListWidget34* w, const QObject* recv, const char* slot) {
    if (w) { QObject::connect(w->m_list, SIGNAL(doubleClicked(QListBoxItem*)), recv, slot); }
}

void QListWidget34::connectItemPressed(QListWidget34* w, const QObject* recv, const char* slot) {
    if (w) { QObject::connect(w->m_list, SIGNAL(pressed(QListBoxItem*)), recv, slot); }
}

void QListWidget34::connectItemActivated(QListWidget34* w, const QObject* recv, const char* slot) {
    if (w) { QObject::connect(w->m_list, SIGNAL(selected(int)), recv, slot); }
}

void QListWidget34::connectCurrentRowChanged(QListWidget34* w, const QObject* recv, const char* slot) {
    if (w) { QObject::connect(w->m_list, SIGNAL(highlighted(int)), recv, slot); }
}

void QListWidget34::connectItemSelectionChanged(QListWidget34* w, const QObject* recv, const char* slot) {
    if (w) { QObject::connect(w->m_list, SIGNAL(selectionChanged()), recv, slot); }
}

#endif /* QT3_BUILD */
