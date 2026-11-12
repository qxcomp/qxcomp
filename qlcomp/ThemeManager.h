#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "StyleParams.h"

#ifdef QT3_BUILD
#define THEME_API

#include <qwidget.h>
#include <qapplication.h>
#include <qpalette.h>

#else
#define THEME_API Q_DECL_EXPORT

#include <QWidget>
#include <QApplication>
#include <QPalette>
#endif

class THEME_API ThemeManager {
public:
    static void applyTheme(bool darkMode);
    static void setDarkMode(bool dark);
    static bool isDarkMode() { return m_darkMode; }
    static void setStyle(const char* id, bool dark);
    static const char* styleId() { return m_styleId; }

    /* macOS Qt4：QApplicationPrivate::initializeWidgetPaletteHash() 会按控件类安装
     * 系统调色板（QLabel/QTabBar/QGroupBox/QHeaderView/QMenu/...），且 app 每次
     * 激活都会重新安装；类调色板优先级高于 app palette，深色主题下这些控件的
     * 文字会回到系统黑色。返回该控件是否属于受影响的类。 */
    static bool hasMacClassPalette(QWidget* w);
    
private:
    static bool m_darkMode;
    static const char* m_styleId;
};

#endif
