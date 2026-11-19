#include "searchline.h"
#include "LimeStyle.h"
#include "StyleParams.h"

#ifdef QT3_BUILD
#include <qpen.h>             // QPen
#include <qpainter.h>         // QPainter
#include <qtimer.h>           // QTimer::singleShot
#else
#include <QPen>               // QPen
#include <QPainter>
#include <QTimer>             // QTimer::singleShot
#endif

#ifdef QT3_BUILD
#define QP_BG QColorGroup::Background
#define QP_FG QColorGroup::Foreground
#define QP_TX QColorGroup::Text
#else
#define QP_BG QPalette::Base
#define QP_FG QPalette::WindowText
#define QP_TX QPalette::Text
#endif

// ==================== SearchInputEdit ====================

SearchInputEdit::SearchInputEdit(QWidget* parent)
    : QLineEdit(parent), m_showing(true), m_normalPalette(palette()) {
    setFrame(false);
    showBlurPlaceholder();
}

void SearchInputEdit::setPlaceholders(const QString& blurText, const QString& focusText) {
    m_blurText = blurText;
    m_focusText = focusText;
    if (m_showing) {
        showBlurPlaceholder();
    }
}

void SearchInputEdit::showBlurPlaceholder() {
    m_showing = true;
    updatePlaceholder(m_blurText);
}

void SearchInputEdit::releasePlaceholder() {
    if (!m_showing) {
        return;
    }
    m_showing = false;
    blockSignals(true);
    clear();
    blockSignals(false);
    setGrayed(false);
}

void SearchInputEdit::focusInEvent(QFocusEvent* event) {
    if (m_showing) {
        /* 点击/获得焦点时真正清空占位，进入可输入状态 */
        releasePlaceholder();
    }
    QLineEdit::focusInEvent(event);
}

void SearchInputEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    /* 焦点移出且无实际输入时恢复占位 */
    if (!m_showing && text().isEmpty()) {
        showBlurPlaceholder();
    }
}

void SearchInputEdit::keyPressEvent(QKeyEvent* event) {
    if (m_showing) {
        int key = event->key();
        if (key == Qt::Key_Backspace || key == Qt::Key_Delete || !event->text().isEmpty()) {
            releasePlaceholder();
        }
    }
    QLineEdit::keyPressEvent(event);

    /* 规则：首字符禁止为空格（docs/空格键播控规则.md 配套 1）
     * 用 QChar(' ') 兼容 Qt3（QLatin1Char 是 Qt4.7+） */
    if (text().startsWith(QChar(' '))) {
        const int pos = cursorPosition();
        setText(qTrim(text()));
        setCursorPosition(pos > 0 ? pos - 1 : 0);
    }
    /* 回车提交即交还焦点（配套 2：避免输入框长期吃掉空格） */
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        clearFocus();
}

void SearchInputEdit::updatePlaceholder(const QString& text) {
    blockSignals(true);
    setText(text);
    blockSignals(false);
    setGrayed(true);
}

void SearchInputEdit::setGrayed(bool gray) {
    if (gray) {
        if (m_showing) {
            QPalette p = m_normalPalette;
            QColor muted = currentPalette().textMuted;
            p.setColor(QPalette::Active, QP_TX, muted);
            p.setColor(QPalette::Inactive, QP_TX, muted);
            p.setColor(QPalette::Disabled, QP_TX, muted);
            setPalette(p);
        }
    } else {
        setPalette(m_normalPalette);
    }
}

void SearchInputEdit::capturePalette() {
    m_normalPalette = palette();
}

// ==================== SearchLineEdit ====================

SearchLineEdit::SearchLineEdit(QWidget* parent)
    : QFrame(parent) {
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);

    QBoxLayout* layout = qNewBoxLayout(this, QBoxLayout::LeftToRight, 0, -1);
    qSetMargins(layout, 0, 0, 0, 0);
    layout->setSpacing(0);

    const StyleParams::Palette& pal = currentPalette();

    m_iconLabel = new QLabel(qFromUtf8("🔍"), this);
    m_iconLabel->setFixedWidth(24);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    {
        QPalette p = m_iconLabel->palette();
        p.setColor(QPalette::Active, QP_FG, pal.textMuted);
        p.setColor(QPalette::Inactive, QP_FG, pal.textMuted);
        p.setColor(QPalette::Disabled, QP_FG, pal.textMuted);
        m_iconLabel->setPalette(p);
    }
    layout->addWidget(m_iconLabel);

    m_edit = new SearchInputEdit(this);
#ifdef QT3_BUILD
    m_edit->setName("SearchLineInnerEdit");
#else
    m_edit->setObjectName("SearchLineInnerEdit");
#endif
    {
        QPalette p = m_edit->palette();
        p.setColor(QPalette::Active, QP_BG, pal.baseBg);
        p.setColor(QPalette::Inactive, QP_BG, pal.baseBg);
        p.setColor(QPalette::Disabled, QP_BG, pal.baseBg);
        p.setColor(QPalette::Active, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Inactive, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Disabled, QP_TX, pal.textDisabled);
        m_edit->setPalette(p);
    }
    m_edit->capturePalette();
    layout->addWidget(m_edit, 1);
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(onEditTextChanged(const QString&)));
    m_edit->installEventFilter(this);

    m_clearLabel = new QLabel(qFromUtf8("✕"), this);
    m_clearLabel->setFixedWidth(20);
    m_clearLabel->setAlignment(Qt::AlignCenter);
    m_clearLabel->setCursor(QCursor(Qt::PointingHandCursor));
    m_clearLabel->hide();
    {
        QPalette p = m_clearLabel->palette();
        p.setColor(QPalette::Active, QP_FG, pal.textMuted);
        p.setColor(QPalette::Inactive, QP_FG, pal.textMuted);
        p.setColor(QPalette::Disabled, QP_FG, pal.textMuted);
        m_clearLabel->setPalette(p);
    }
    m_clearLabel->installEventFilter(this);
    layout->addWidget(m_clearLabel);

#ifdef QT3_BUILD
    m_popup = new QFrame(0, 0, Qt::WType_Popup);
#else
    m_popup = new QFrame(0, Qt::Popup);
#endif
    m_popup->setFrameShape(QFrame::StyledPanel);
    m_popup->setFrameShadow(QFrame::Plain);
    m_popup->setLineWidth(1);
    m_popup->hide();
    {
        QPalette p = m_popup->palette();
        p.setColor(QPalette::Active, QP_BG, pal.baseBg);
        p.setColor(QPalette::Inactive, QP_BG, pal.baseBg);
        p.setColor(QPalette::Disabled, QP_BG, pal.baseBg);
        p.setColor(QPalette::Active, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Inactive, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Disabled, QP_TX, pal.textDisabled);
        m_popup->setPalette(p);
    }
    QBoxLayout* popupLayout = qNewBoxLayout(m_popup, QBoxLayout::LeftToRight, 1, -1);
    popupLayout->setSpacing(0);

#ifdef QT3_BUILD
    m_list = new QListBox(m_popup);
    m_list->setFrameShape(QFrame::NoFrame);
    {
        QPalette p = m_list->palette();
        p.setColor(QPalette::Active, QP_BG, pal.baseBg);
        p.setColor(QPalette::Inactive, QP_BG, pal.baseBg);
        p.setColor(QPalette::Disabled, QP_BG, pal.baseBg);
        p.setColor(QPalette::Active, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Inactive, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Disabled, QP_TX, pal.textDisabled);
        m_list->setPalette(p);
    }
    popupLayout->addWidget(m_list);
    connect(m_list, SIGNAL(clicked(QListBoxItem*)), this, SLOT(onSuggestionClicked()));
#else
    m_list = new QListWidget(m_popup);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setUniformItemSizes(true);
    {
        QPalette p = m_list->palette();
        p.setColor(QPalette::Active, QP_BG, pal.baseBg);
        p.setColor(QPalette::Inactive, QP_BG, pal.baseBg);
        p.setColor(QPalette::Disabled, QP_BG, pal.baseBg);
        p.setColor(QPalette::Active, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Inactive, QP_TX, pal.textPrimary);
        p.setColor(QPalette::Disabled, QP_TX, pal.textDisabled);
        m_list->setPalette(p);
    }
    popupLayout->addWidget(m_list);
    connect(m_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onSuggestionClicked()));
#endif
}

void SearchLineEdit::setPlaceholders(const QString& blurText, const QString& focusText) {
    m_edit->setPlaceholders(blurText, focusText);
}

void SearchLineEdit::setSuggestions(const QStringList& items) {
    m_suggestions = items;
    if (isVisible() && m_popup->isVisible()) {
        showPopup();
    }
}

void SearchLineEdit::setText(const QString& text) {
    m_edit->releasePlaceholder();
    m_edit->setText(text);
}

void SearchLineEdit::resetToPlaceholder() {
    m_edit->showBlurPlaceholder();
    m_clearLabel->hide();
    hidePopup();
}

void SearchLineEdit::paintEvent(QPaintEvent* event) {
    QFrame::paintEvent(event);

    const StyleParams::Palette& pal = currentPalette();
    QPainter p(this);
#ifndef QT3_BUILD
    p.setRenderHint(QPainter::Antialiasing);
#endif

#ifdef QT3_BUILD
    QRect body(rect().left() + 1, rect().top() + 1, rect().width() - 2, rect().height() - 2);
#else
    QRect body = rect().adjusted(1, 1, -1, -1);
#endif
    p.setPen(Qt::NoPen);
    p.setBrush(pal.baseBg);
#ifdef QT3_BUILD
    p.drawRoundRect(body, 15, 15);
#else
    p.drawRoundedRect(body, 15, 15);
#endif

    QColor border = m_edit->hasFocus() ? pal.borderFocus : pal.border;
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(border, 1));
#ifdef QT3_BUILD
    p.drawRoundRect(body, 15, 15);
#else
    p.drawRoundedRect(body, 15, 15);
#endif
}

bool SearchLineEdit::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_edit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            int key = keyEvent->key();
            if (key == Qt::Key_Escape) {
                hidePopup();
                return true;
            }
            /* 回车:有联想项 → 选中项;无联想项 → 输入框原文(视频源搜索用) */
            if (key == Qt::Key_Return || key == Qt::Key_Enter) {
                if (m_popup->isVisible() && !currentSuggestion().isEmpty()) {
                    activateCurrentSuggestion();
                } else {
                    const QString t = m_edit->actualText();
                    if (!t.isEmpty()) {
                        hidePopup();
                        m_edit->clearFocus();
                        emit triggered(t);
                    }
                }
                return true;
            }
            if (m_popup->isVisible()) {
                if (key == Qt::Key_Down) {
                    moveListSelection(1);
                    return true;
                }
                if (key == Qt::Key_Up) {
                    moveListSelection(-1);
                    return true;
                }
            }
        } else if (event->type() == QEvent::FocusOut) {
            if (m_popup->isVisible()) {
                QTimer::singleShot(0, this, SLOT(hidePopup()));
            }
        }
    } else if (watched == m_clearLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            onClearClicked();
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

void SearchLineEdit::onEditTextChanged(const QString& text) {
    if (m_edit->isPlaceholderVisible()) {
        return;
    }
    emit textChanged(text);
    bool hasText = !text.isEmpty();
#ifdef QT3_BUILD
    m_clearLabel->setShown(hasText);
#else
    m_clearLabel->setVisible(hasText);
#endif
    if (hasText) {
        showPopup();
    } else {
        hidePopup();
    }
}

void SearchLineEdit::onClearClicked() {
    m_edit->showBlurPlaceholder();
    m_clearLabel->hide();
    hidePopup();
    emit textChanged(QString());
    emit cleared();
}

void SearchLineEdit::onSuggestionClicked() {
    activateCurrentSuggestion();
}

void SearchLineEdit::activateCurrentSuggestion() {
    QString t = currentSuggestion();
    if (t.isEmpty()) {
        hidePopup();
        return;
    }
    m_edit->setText(t);
    hidePopup();
    emit triggered(t);
}

void SearchLineEdit::moveListSelection(int delta) {
    int count = m_list->count();
    if (count <= 0) {
        return;
    }
#ifdef QT3_BUILD
    int next = m_list->currentItem() + delta;
#else
    int next = m_list->currentRow() + delta;
#endif
    if (next < 0) {
        next = count - 1;
    }
    if (next >= count) {
        next = 0;
    }
#ifdef QT3_BUILD
    m_list->setCurrentItem(next);
#else
    m_list->setCurrentRow(next);
#endif
}

QString SearchLineEdit::currentSuggestion() const {
#ifdef QT3_BUILD
    int index = m_list->currentItem();
    if (index < 0) {
        return QString();
    }
    return m_list->text(index);
#else
    QListWidgetItem* item = m_list->currentItem();
    if (!item) {
        return QString();
    }
    return item->text();
#endif
}

void SearchLineEdit::showPopup() {
    if (m_suggestions.isEmpty()) {
        hidePopup();
        return;
    }

    int rowHeight = m_list->fontMetrics().height() + 6;
    int count = m_suggestions.count();
    int popupHeight = count * rowHeight + 6;
    if (popupHeight > 200) {
        popupHeight = 200;
    }
    if (popupHeight < rowHeight + 6) {
        popupHeight = rowHeight + 6;
    }

#ifdef QT3_BUILD
    m_list->clear();
    for (int i = 0; i < count; ++i) {
        m_list->insertItem(m_suggestions[i], -1);
    }
    m_list->setCurrentItem(0);
#else
    m_list->clear();
    for (int i = 0; i < count; ++i) {
        m_list->addItem(new QListWidgetItem(m_suggestions.at(i)));
    }
    m_list->setCurrentRow(0);
#endif

    QPoint g = m_edit->mapToGlobal(QPoint(0, m_edit->height() + 1));
    m_popup->setGeometry(g.x(), g.y(), m_edit->width(), popupHeight);
    m_popup->show();
    m_popup->raise();
}

void SearchLineEdit::hidePopup() {
    m_popup->hide();
}

#ifdef QT3_BUILD
#undef QP_BG
#undef QP_FG
#undef QP_TX
#else
#undef QP_BG
#undef QP_FG
#undef QP_TX
#endif