#ifndef QLCOMP_QSETTINGS34_H
#define QLCOMP_QSETTINGS34_H

// ══ QSettings34：为兼容高版本 QSettings 而写的 Qt3 实现 ═══════════════════
//
// Qt4.0 起 QSettings 换成 value()/setValue()/remove()/childKeys() + QVariant 模型，
// 并把 Unix 原生格式定为 INI 落盘（~/.config/<org>/<app>.conf）。Qt3 的 QSettings
// （QConf）只有类型化的 readEntry/writeEntry 且落到 $HOME/.qt/<domain>rc，**完全
// 不互通**（见 stikcommon/qsettings_shim.h:18-21 的自述）。
//
// 本类在 Qt3 上**独立实现高版本 QSettings 的 INI 编解码与落盘规则**，逐函数对齐
// Qt 6.7.3 src/corelib/io/qsettings.cpp，使 Qt3 程序能与 Qt4/5/6 程序**读写同一份
// 配置文件**：
//   · 路径       ~/.config/<org>/<app>.conf（$XDG_CONFIG_HOME 优先）
//   · 段/键排序   Qt::CaseSensitive 字典序（Unix 定义
//                QT_QSETTINGS_ALWAYS_CASE_SENSITIVE_AND_FORGET_ORIGINAL_KEY_ORDER）
//   · 顶层裸键   → [General]；显式 "General/x" → [%General]
//   · 编解码     iniEscapedKey / iniEscapedString / iniEscapedStringList /
//                iniUnescapedKey / iniUnescapedStringList / readIniLine
//   · 值模型     value()/setValue() 与 Qt6 同名同义；@Variant(...) 按
//                QDataStream(Qt_4_0) 解出 QVariantMap/List 等（只读还原）
//
// 典型用途：qlstik（Qt3）与 anystik（Qt6）共用 ~/.config/fedlet/anystik.conf，
// 共享 [General] 下的 DAV 凭据等。
//
// **挂载方式**：按 qlcomp 双版本编译（qltox 的 build-qt3 / build-qt4 都编 qsettings34）：
// Qt3 侧实际使用本类；Qt4 侧只保证**编过**（qltox 的 Qt4 目标不读取其值）。
//
// ⚠ 未调用 setValue 的条目，写回时**原样回写文件中的 value 字节**（rawValue）；
//   已 setValue 的条目按 Qt6 variantToString() 重新编码。二者共同保证
//   @Variant(...)/@DateTime(...) 不被改坏。
// ⚠ 与 Qt6 的 QLockFile 不同，本实现不建 .lock，极端并发同写未覆盖。
//
// 双版本：两版本的类型分岔**收敛到头部的一个 QSettingsBuffer**（Qt3 转真
// QCString，Qt4 垫 QByteArray，语义逐字节对齐），编解码/落盘逻辑对两版本**单一
// 实现**；仅文件打开模式、SaveFile::write 的 writeBlock/write 四处就地 #ifdef。
//
// 本文件（及 qsettings34.cpp）**自包含**：只用 Qt3 自身的库，不 include 任何
// stikcommon 垫片，可独立挂进 qlcomp 单独编译。

#ifdef QT3_BUILD
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qvariant.h>
#include <qcstring.h>
#else
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QByteArray>
#endif

// ══ QSettingsBuffer：双版本的「缓冲类型缝」（唯一） ════════════════════════
// Qt3 有 QCString，Qt4 没有（QCString 只在 Qt3Support，build-qt4 不提供该库）。
// 为让 .cpp 的编解码/落盘逻辑对两版本**单一实现**，统一出一个窄缓冲类：
//   · Qt3 分支直接转真 QCString —— 含 (ptr,maxlen) 拷 maxlen-1 字符、尾 NUL、
//     size()=内容+1 等 Qt3 特性，语义逐字节不变；
//   · Qt4 分支垫 QByteArray —— 仅保证 build-qt4 编过，不参与业务。
// 两分支对外接口一致，即 .cpp 用到的全部方法。
class QSettingsBuffer
{
#ifdef QT3_BUILD
    QCString m_b;
public:
    QSettingsBuffer() {}
    QSettingsBuffer(const char* s, uint maxlen) : m_b(s, maxlen) {}
    QSettingsBuffer(const QCString& b) : m_b(b) {}
    QSettingsBuffer& operator=(const char* s)        { m_b = s; return *this; }
    QSettingsBuffer& operator=(const QSettingsBuffer& o)
        { m_b = o.m_b; return *this; }
    QSettingsBuffer& operator+=(const char* s)       { m_b += s; return *this; }
    QSettingsBuffer& operator+=(char c)              { m_b += c; return *this; }
    QSettingsBuffer& operator+=(const QSettingsBuffer& o)
        { m_b += (const char*)o.m_b; return *this; }
    void append(const char* s)               { m_b.append(s); }
    void insert(uint pos, const char* s)     { m_b.insert(pos, s); }
    QSettingsBuffer stripWhiteSpace() const
        { return QSettingsBuffer(m_b.stripWhiteSpace()); }
    char at(uint i) const                    { return m_b.at(i); }
    const char* data() const                 { return (const char*)m_b; }
    bool isEmpty() const                     { return m_b.isEmpty(); }
    int size() const                         { return int(m_b.size()); }
    int length() const                       { return int(m_b.length()); }
    operator const char*() const             { return (const char*)m_b; }
#else
    QByteArray m_b;
public:
    QSettingsBuffer() {}
    QSettingsBuffer(const char* s, uint maxlen)
        : m_b(s, maxlen > 0 ? int(maxlen) - 1 : 0) {}
    QSettingsBuffer(const QByteArray& b) : m_b(b) {}
    QSettingsBuffer& operator=(const char* s)        { m_b = s; return *this; }
    QSettingsBuffer& operator=(const QSettingsBuffer& o)
        { m_b = o.m_b; return *this; }
    QSettingsBuffer& operator+=(const char* s)       { m_b += s; return *this; }
    QSettingsBuffer& operator+=(char c)              { m_b += c; return *this; }
    QSettingsBuffer& operator+=(const QSettingsBuffer& o)
        { m_b += o.m_b; return *this; }
    void append(const char* s)               { m_b.append(s); }
    void insert(uint pos, const char* s)     { m_b.insert(int(pos), s); }
    QSettingsBuffer stripWhiteSpace() const
        { return QSettingsBuffer(m_b.trimmed()); }
    char at(uint i) const                    { return m_b.at(int(i)); }
    const char* data() const                 { return m_b.constData(); }
    bool isEmpty() const                     { return m_b.isEmpty(); }
    int size() const                         { return m_b.size(); }
    int length() const                       { return m_b.length(); }
    operator const char*() const             { return m_b.constData(); }
#endif
};

class QSettings34
{
public:
    // 默认与 anystik 的 QCoreApplication organizationName/applicationName 对齐
    // （anystik/src/main.cpp 设为 fedlet / anystik），从而命中同一文件。
    explicit QSettings34(const QString& org = QString::fromLatin1("fedlet"),
                         const QString& app = QString::fromLatin1("anystik"));

    QString fileName() const;
    bool    isWritable() const;

    // 逻辑键与高版本 QSettings 相同：[General] 下的键不带前缀（"davUrl"），
    // 其余段为 "段/键"（"dlProgress/0"）。
    bool    contains(const QString& key) const;

    // 与 Qt6 QSettings 同名同义（@Variant 等注解已解出为 QVariant）。
    QVariant value(const QString& key, const QVariant& def = QVariant()) const;
    void     setValue(const QString& key, const QVariant& value);

    // 便捷封装（等价 value().toString() 等；列表值 valueStr 返回 def）。
    QString     valueStr(const QString& key, const QString& def = QString()) const;
    QStringList valueStringList(const QString& key,
                                const QStringList& def = QStringList()) const;
    int         valueInt(const QString& key, int def = 0) const;
    bool        valueBool(const QString& key, bool def = false) const;
    double      valueDouble(const QString& key, double def = 0.0) const;
    void        setValueStringList(const QString& key, const QStringList& value);
    void        setValueInt(const QString& key, int value);

    void remove(const QString& key);
    void clear();

    QStringList allKeys() const;
    QStringList childKeys(const QString& prefix) const;

    // 全量原子写回（仅在有 setValue/remove/clear 时写）。返回是否成功。
    bool sync();

    // 供对拍/测试：定位规则
    static QString userConfigDir();
    static QString filePathFor(const QString& org, const QString& app);

private:
    struct Entry {
        QVariant       variant;    // 解码后的值（@Variant 已还原为 QVariantMap/List 等）
        QSettingsBuffer rawValue;  // 文件中的原始 value 字节（未触碰时原样回写）
        bool           touched;    // setValue/remove 后为 true（remove 用移除表示）
        Entry() : touched(false) {}
    };
    // 段名（"" 表示 General）→ 叶键 → Entry，均按高版本的字典序排列。
    QMap<QString, QMap<QString, Entry> > m_sections;

    QString m_org;
    QString m_app;
    bool    m_dirty;

    void    load();
    static  QString normalizeKey(const QString& key);
    static  void    splitKey(const QString& key, QString& section, QString& leaf);
};

#endif // QLCOMP_QSETTINGS34_H
