#include "placeholderlineedit.h"

#ifdef QT3_BUILD
#include <qcursor.h>
#else
#include <QCursor>
#endif

PlaceholderLineEdit::PlaceholderLineEdit(const QString& placeholder, QWidget* parent)
    : QLineEdit(parent), m_placeholder(placeholder), m_showingPlaceholder(true) {
    showPlaceholder();
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
}

void PlaceholderLineEdit::setPlaceholderText(const QString& text) {
    m_placeholder = text;
    if (m_showingPlaceholder) {
        showPlaceholder();
    }
}

void PlaceholderLineEdit::setRealText(const QString& text) {
    blockSignals(true);
    QLineEdit::setText(text);
    blockSignals(false);
    m_showingPlaceholder = false;
}

void PlaceholderLineEdit::setClearButtonEnabled(bool on) {
    m_clearEnabled = on;
    if (!on) {
        if (m_clearLabel) { m_clearLabel->hide(); }
        return;
    }
    if (!m_clearLabel) {
        m_clearLabel = new QLabel(qFromUtf8("✕"), this);
        m_clearLabel->setFixedSize(16, 16);
        m_clearLabel->setAlignment(Qt::AlignCenter);
        m_clearLabel->setCursor(QCursor(Qt::PointingHandCursor));
        m_clearLabel->installEventFilter(this);
        m_clearLabel->hide();
    }
    positionClearLabel();
    if (!m_showingPlaceholder && !text().isEmpty()) { m_clearLabel->show(); }
    else { m_clearLabel->hide(); }
}

void PlaceholderLineEdit::positionClearLabel() {
    if (!m_clearLabel) { return; }
    m_clearLabel->move(width() - m_clearLabel->width() - 2,
                       (height() - m_clearLabel->height()) / 2);
}

void PlaceholderLineEdit::onTextChanged(const QString& text) {
    if (!m_clearLabel || !m_clearEnabled) { return; }
    positionClearLabel();
    if (!text.isEmpty()) { m_clearLabel->show(); }
    else { m_clearLabel->hide(); }
}

void PlaceholderLineEdit::focusInEvent(QFocusEvent* event) {
    clearIfPlaceholder();
    QLineEdit::focusInEvent(event);
}

void PlaceholderLineEdit::focusOutEvent(QFocusEvent* event) {
    if (text().isEmpty()) {
        showPlaceholder();
    }
    QLineEdit::focusOutEvent(event);
}

void PlaceholderLineEdit::keyPressEvent(QKeyEvent* event) {
    if (m_clearEnabled && event->key() == Qt::Key_Escape
        && !m_showingPlaceholder && !text().isEmpty()) {
        clear();
        emit cleared();
        return;
    }
    QLineEdit::keyPressEvent(event);
}

void PlaceholderLineEdit::resizeEvent(QResizeEvent* event) {
    positionClearLabel();
    QLineEdit::resizeEvent(event);
}

bool PlaceholderLineEdit::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_clearLabel && event->type() == QEvent::MouseButtonPress) {
        clear();
        emit cleared();
        if (!hasFocus()) { showPlaceholder(); }
        return true;
    }
    return QLineEdit::eventFilter(watched, event);
}

void PlaceholderLineEdit::showPlaceholder() {
    blockSignals(true);
    setText(m_placeholder);
    blockSignals(false);
    m_showingPlaceholder = true;
}

void PlaceholderLineEdit::clearIfPlaceholder() {
    if (m_showingPlaceholder) {
        blockSignals(true);
        clear();
        blockSignals(false);
        m_showingPlaceholder = false;
    }
}
