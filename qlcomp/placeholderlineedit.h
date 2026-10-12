#ifndef PLACEHOLDERLINEEDIT_H
#define PLACEHOLDERLINEEDIT_H

#include "compat34.h"

class PlaceholderLineEdit : public QLineEdit {
    Q_OBJECT
public:
    PlaceholderLineEdit(const QString& placeholder, QWidget* parent = 0);
    
    void setPlaceholderText(const QString& text);
    QString placeholderText() const { return m_placeholder; }
    void setRealText(const QString& text);
    void setClearButtonEnabled(bool on);

signals:
    void cleared();

protected:
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* watched, QEvent* event);

private slots:
    void onTextChanged(const QString& text);

private:
    QString m_placeholder;
    bool m_showingPlaceholder;

    QLabel* m_clearLabel = 0;
    bool m_clearEnabled = false;

    void showPlaceholder();
    void clearIfPlaceholder();
    void positionClearLabel();
};

#endif
