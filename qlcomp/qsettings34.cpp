#include "qsettings34.h"

#ifdef QT3_BUILD
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qcstring.h>
#else
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QVariant>
#include <QList>
#include <QByteArray>
#endif

// 双版本的 QVariant 列表类型（Qt3 为 QValueList，Qt4 为 QList）。
#ifdef QT3_BUILD
typedef QValueList<QVariant> QVariantList34;
#else
typedef QList<QVariant> QVariantList34;
#endif

// 双版本的基础整数类型（Qt3 无 q34u32/q34i64 等 typedef）。
#ifdef QT3_BUILD
typedef Q_UINT32 q34u32;
typedef Q_UINT64 q34u64;
typedef Q_INT32  q34i32;
typedef Q_INT64  q34i64;
#else
typedef unsigned int       q34u32;
typedef unsigned long long q34u64;
typedef int                q34i32;
typedef long long          q34i64;
#endif

#include <stdlib.h>             // getenv
#include <string.h>
#include <cstdio>               // ::rename(2)

namespace {

// ══ 与 Qt 6.7.3 src/corelib/io/qsettings.cpp 逐一对齐的小工具 ══════════════

const unsigned char kSpace   = 1;
const unsigned char kSpecial = 2;

// readIniLine 用的字符分类表（qsettings.cpp:1525 原表）。
const unsigned char charTraits[256] = {
    // Space: '\t', '\n', '\r', ' '
    // Special: '\n', '\r', '"', ';', '=', '\\'
    0, 0, 0, 0, 0, 0, 0, 0, 0, kSpace, kSpace | kSpecial, 0, 0, kSpace | kSpecial, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    kSpace, 0, kSpecial, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, kSpecial, 0, kSpecial, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, kSpecial, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

bool isAsciiLetterOrNumber(uint ch)
{
    return (ch >= '0' && ch <= '9')
        || (ch >= 'a' && ch <= 'z')
        || (ch >= 'A' && ch <= 'Z');
}

char toHexUpper(uint c)
{
    return (c < 10) ? char('0' + c) : char('A' + (c - 10));
}

bool isHexDigit(char c)
{
    return (c >= '0' && c <= '9')
        || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
}

int fromHex(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int fromOct(char c)
{
    return (c >= '0' && c <= '7') ? (c - '0') : -1;
}

void appendHexLower(QSettingsBuffer& out, uint ch)
{
    // 与 QByteArray::number(ch, 16) 一致：小写、无前导零。
    // 注：Qt3 的 QCString 无 setNum(uint, int)，故手写。
    char buf[16];
    int n = 0;
    if (ch == 0) {
        buf[n++] = '0';
    }
    while (ch) {
        const uint d = ch & 0xF;
        buf[n++] = (d < 10) ? char('0' + d) : char('a' + (d - 10));
        ch >>= 4;
    }
    while (n > 0)
        out += buf[--n];
}

bool isUpperChar(ushort ch)
{
    // Qt3 无 QChar::isUpper()，用 Unicode 类别（Letter_Uppercase）判定。
    return QChar(ch).category() == QChar::Letter_Uppercase;
}

void iniEscapedKey(const QString& key, QSettingsBuffer& result)
{
    for (uint i = 0; i < key.length(); ++i) {
        uint ch = key[i].unicode();
        if (ch == '/') {
            result += '\\';
        } else if (isAsciiLetterOrNumber(ch) || ch == '_' || ch == '-' || ch == '.') {
            result += char(ch);
        } else if (ch <= 0xFF) {
            result += '%';
            result += toHexUpper(ch / 16);
            result += toHexUpper(ch % 16);
        } else {
            result += "%U";
            char hexCode[5];
            for (int j = 3; j >= 0; --j) {
                hexCode[j] = toHexUpper(ch % 16);
                ch >>= 4;
            }
            hexCode[4] = '\0';
            result.append(hexCode);
        }
    }
}

bool iniUnescapedKey(const QSettingsBuffer& keyBytes, QString& result)
{
    const QString decoded =
        QString::fromUtf8(keyBytes.data(), int(keyBytes.size()));
    const int size = int(decoded.length());
    int i = 0;
    bool lowercaseOnly = true;
    while (i < size) {
        ushort ch = decoded[i].unicode();
        if (ch == '\\') {
            result += QChar('/');
            ++i;
            continue;
        }
        if (ch != '%' || i == size - 1) {
            if (isUpperChar(ch)) lowercaseOnly = false;
            result += QChar(ch);
            ++i;
            continue;
        }
        int numDigits = 2;
        int firstDigitPos = i + 1;
        ch = decoded[i + 1].unicode();
        if (ch == 'U') {
            ++firstDigitPos;
            numDigits = 4;
        }
        if (firstDigitPos + numDigits > size) {
            result += QChar('%');
            ++i;
            continue;
        }
        bool ok = false;
        ch = decoded.mid(uint(firstDigitPos), uint(numDigits)).toUShort(&ok, 16);
        if (!ok) {
            result += QChar('%');
            ++i;
            continue;
        }
        if (isUpperChar(ch)) lowercaseOnly = false;
        result += QChar(ch);
        i = firstDigitPos + numDigits;
    }
    return lowercaseOnly;
}

void iniEscapedString(const QString& str, QSettingsBuffer& result)
{
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    const bool useCodec = !(str.startsWith(QString::fromLatin1("@ByteArray("))
                            || str.startsWith(QString::fromLatin1("@Variant("))
                            || str.startsWith(QString::fromLatin1("@DateTime(")));
    const uint startPos = result.length();

    for (uint i = 0; i < str.length(); ++i) {
        const ushort qch = str[i].unicode();
        const uint ch = qch;
        if (ch == ';' || ch == ',' || ch == '=') needsQuotes = true;

        if (escapeNextIfDigit && isHexDigit(char(ch))) {
            result += "\\x";
            appendHexLower(result, ch);
            continue;
        }
        escapeNextIfDigit = false;

        switch (ch) {
        case '\0': result += "\\0"; escapeNextIfDigit = true; break;
        case '\a': result += "\\a"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        case '\v': result += "\\v"; break;
        case '"':
        case '\\':
            result += '\\';
            result += char(ch);
            break;
        default:
            if (ch <= 0x1F || (ch >= 0x7F && !useCodec)) {
                result += "\\x";
                appendHexLower(result, ch);
                escapeNextIfDigit = true;
            } else if (useCodec) {
                // 与 Qt6 QStringEncoder(Utf8) 对齐：按 UTF-8 追加该码点字节。
#ifdef QT3_BUILD
                const QSettingsBuffer u = QString(QChar(qch)).utf8();
#else
                const QSettingsBuffer u = QString(QChar(qch)).toUtf8();
#endif
                result += (const char*)u;
            } else {
                result += char(ch);
            }
        }
    }

    if (needsQuotes
        || (startPos < result.length()
            && (result.at(startPos) == ' ' || result.at(result.length() - 1) == ' '))) {
        result.insert(startPos, "\"");
        result += '"';
    }
}

void iniChopTrailingSpaces(QString& str, int limit)
{
    int n = int(str.length()) - 1;
    QChar ch;
    while (n >= limit && ((ch = str.at(uint(n))) == QChar(' ') || ch == QChar('\t'))) {
        str.truncate(uint(n));
        --n;
    }
}

bool iniUnescapedStringList(const QSettingsBuffer& str, QString& stringResult,
                            QStringList& stringListResult)
{
    static const char escapeCodes[][2] = {
        { 'a', '\a' }, { 'b', '\b' }, { 'f', '\f' }, { 'n', '\n' },
        { 'r', '\r' }, { 't', '\t' }, { 'v', '\v' }, { '"', '"' },
        { '?', '?' },  { '\'', '\'' }, { '\\', '\\' }
    };

    bool isStringList = false;
    bool inQuotedString = false;
    bool currentValueIsQuoted = false;
    ushort escapeVal = 0;
    const int size = int(str.size());
    int i = 0;
    char ch = 0;

StSkipSpaces:
    while (i < size && ((ch = str.at(uint(i))) == ' ' || ch == '\t'))
        ++i;
    // fallthrough

StNormal:
    {
        int chopLimit = int(stringResult.length());
        while (i < size) {
            switch (str.at(uint(i))) {
            case '\\': {
                ++i;
                if (i >= size) goto end;
                ch = str.at(uint(i++));
                for (int e = 0; e < 11; ++e) {
                    if (ch == escapeCodes[e][0]) {
                        stringResult += QChar(uchar(escapeCodes[e][1]));
                        goto StNormal;
                    }
                }
                if (ch == 'x') {
                    escapeVal = 0;
                    if (i >= size) goto end;
                    ch = str.at(uint(i));
                    if (isHexDigit(ch)) goto StHexEscape;
                } else {
                    const int o = fromOct(ch);
                    if (o != -1) {
                        escapeVal = ushort(o);
                        goto StOctEscape;
                    } else if (ch == '\n' || ch == '\r') {
                        if (i < size) {
                            const char ch2 = str.at(uint(i));
                            if ((ch2 == '\n' || ch2 == '\r') && ch2 != ch)
                                ++i;
                        }
                    }
                    // 其余字符被跳过
                }
                chopLimit = int(stringResult.length());
                break;
            }
            case '"':
                ++i;
                currentValueIsQuoted = true;
                inQuotedString = !inQuotedString;
                if (!inQuotedString) goto StSkipSpaces;
                break;
            case ',':
                if (!inQuotedString) {
                    if (!currentValueIsQuoted)
                        iniChopTrailingSpaces(stringResult, chopLimit);
                    if (!isStringList) {
                        isStringList = true;
                        stringListResult.clear();
                        stringResult.squeeze();
                    }
                    stringListResult.append(stringResult);
                    stringResult = QString();
                    currentValueIsQuoted = false;
                    ++i;
                    goto StSkipSpaces;
                }
                // fallthrough
            default: {
                int j = i + 1;
                while (j < size) {
                    ch = str.at(uint(j));
                    if (ch == '\\' || ch == '"' || ch == ',')
                        break;
                    ++j;
                }
                stringResult += QString::fromUtf8(str.data() + i, j - i);
                i = j;
            }
            }
        }
        if (!currentValueIsQuoted)
            iniChopTrailingSpaces(stringResult, chopLimit);
        goto end;
    }

StHexEscape:
    if (i >= size) {
        stringResult += QChar(escapeVal);
        goto end;
    }
    ch = str.at(uint(i));
    {
        const int h = fromHex(ch);
        if (h != -1) {
            escapeVal = ushort((escapeVal << 4) + h);
            ++i;
            goto StHexEscape;
        }
        stringResult += QChar(escapeVal);
        goto StNormal;
    }

StOctEscape:
    if (i >= size) {
        stringResult += QChar(escapeVal);
        goto end;
    }
    ch = str.at(uint(i));
    {
        const int o = fromOct(ch);
        if (o != -1) {
            escapeVal = ushort((escapeVal << 3) + o);
            ++i;
            goto StOctEscape;
        }
        stringResult += QChar(escapeVal);
        goto StNormal;
    }

end:
    if (isStringList)
        stringListResult.append(stringResult);
    return isStringList;
}

// readIniLine：qsettings.cpp:1553 的逐行移植（Qt3 用 int 索引）。
bool readIniLine(const char* data, int dataLen, int& dataPos, int& lineStart,
                 int& lineLen, int& equalsPos)
{
    bool inQuotes = false;
    equalsPos = -1;

    lineStart = dataPos;
    while (lineStart < dataLen && (charTraits[uchar(data[lineStart])] & kSpace))
        ++lineStart;

    int i = lineStart;
    while (i < dataLen) {
        char ch = data[i];
        while (!(charTraits[uchar(ch)] & kSpecial)) {
            if (++i == dataLen)
                goto break_out_of_outer_loop;
            ch = data[i];
        }

        ++i;
        if (ch == '=') {
            if (!inQuotes && equalsPos == -1)
                equalsPos = i - 1;
        } else if (ch == '\n' || ch == '\r') {
            if (i == lineStart + 1) {
                ++lineStart;
            } else if (!inQuotes) {
                --i;
                goto break_out_of_outer_loop;
            }
        } else if (ch == '\\') {
            if (i < dataLen) {
                const char c = data[i++];
                if (i < dataLen) {
                    const char c2 = data[i];
                    if ((c == '\n' && c2 == '\r') || (c == '\r' && c2 == '\n'))
                        ++i;
                }
            }
        } else if (ch == '"') {
            inQuotes = !inQuotes;
        } else {
            // ';'
            if (i == lineStart + 1) {
                while (i < dataLen && ((ch = data[i]) != '\n' && ch != '\r'))
                    ++i;
                while (i < dataLen && (charTraits[uchar(data[i])] & kSpace))
                    ++i;
                lineStart = i;
            } else if (!inQuotes) {
                --i;
                goto break_out_of_outer_loop;
            }
        }
    }

break_out_of_outer_loop:
    dataPos = i;
    lineLen = i - lineStart;
    return lineLen > 0;
}

// ══ QVariant ↔ Qt6 QDataStream(Qt_4_0) 编解码（@Variant 注解） ══════════════
// 对齐 qsettings.cpp:variantToString()/stringToVariant()：QSettings 对非
// QDateTime 类型固定用 QDataStream::Qt_4_0（=7）序列化 QVariant，落成
// @Variant(<字节>)。Qt3 自带 QVariant::save 是 Qt3 私有格式、不可用，故自写
// 一份最小 Qt_4_0 编解码器，只覆盖 QSettings 实际会出现的类型。
// 规则（qt/qtbase 6.7 源码逐处核对）：
//   · v7 < Qt_4_2，QVariant 头**不写 null flag**：q34u32 typeId + 值；
//   · 内建 id 不改写（重映射只发生于 <Qt_4_0 或 id≥31）；
//   · 整数一律大端；bool 1 字节；double 8 字节大端 IEEE-754；
//   · QString：null→0xFFFFFFFF，否则 q34u32 字节长(2×QChar)+UTF-16 大端；
//   · 容器：q34u32 计数 + 项；QMap 为 计数 + (QString 键, QVariant 值)*。
// 加减过程中的字节值一律存进 QString 的 QChar（每 QChar 持 1 字节），避开
// Qt3 QCString 以 strlen 计长的坑（内嵌 NUL 会截断）。

enum {
    kQt6Unknown = 0, kQt6Bool = 1, kQt6Int = 2, kQt6UInt = 3,
    kQt6LongLong = 4, kQt6ULongLong = 5, kQt6Double = 6,
    kQt6Map = 8, kQt6List = 9, kQt6String = 10,
    kQt6StringList = 11, kQt6ByteArray = 12
};

int dsQt6IdFor(QVariant::Type t)
{
    switch (t) {
    case QVariant::Bool:       return kQt6Bool;
    case QVariant::Int:        return kQt6Int;
    case QVariant::UInt:       return kQt6UInt;
    case QVariant::LongLong:   return kQt6LongLong;
    case QVariant::ULongLong:  return kQt6ULongLong;
    case QVariant::Double:     return kQt6Double;
    case QVariant::Map:        return kQt6Map;
    case QVariant::List:       return kQt6List;
    case QVariant::String:     return kQt6String;
    case QVariant::StringList: return kQt6StringList;
    case QVariant::ByteArray:  return kQt6ByteArray;
    default:                   return kQt6Unknown;
    }
}

// ── 读 ──────────────────────────────────────────────────────────────────
struct DsReader {
    QString b;
    uint pos;
    bool ok;
    DsReader(const QString& buf) : b(buf), pos(0), ok(true) {}
    uchar u8() {
        if (pos + 1 > uint(b.length())) { ok = false; return 0; }
        return uchar(b.at(pos++).unicode());
    }
    q34u32 u32() {
        if (pos + 4 > uint(b.length())) { ok = false; return 0; }
        return (q34u32(u8()) << 24) | (q34u32(u8()) << 16)
             | (q34u32(u8()) <<  8) |  q34u32(u8());
    }
    q34u64 u64() { const q34u64 hi = u32(); const q34u64 lo = u32(); return (hi << 32) | lo; }
};

// 读 QString；返回 false 表示 null（0xFFFFFFFF）。ok=false 表示越界/坏长度。
bool dsReadQString(DsReader& r, QString& out)
{
    const q34u32 len = r.u32();
    if (!r.ok) return false;
    if (len == 0xFFFFFFFFu) { out = QString(); return false; }
    if (len & 1u) { r.ok = false; return false; }
    const uint n = len / 2;
    if (r.pos + 2u * n > uint(r.b.length())) { r.ok = false; return false; }
    out = QString();
    for (uint i = 0; i < n; ++i) {
        const uint hi = uchar(r.b.at(r.pos++).unicode());
        const uint lo = uchar(r.b.at(r.pos++).unicode());
        out += QChar(ushort((hi << 8) | lo));
    }
    return true;
}

QVariant dsReadQVariant(DsReader& r)
{
    const q34u32 id = r.u32();
    if (!r.ok) return QVariant();
    switch (id) {
    case kQt6Unknown: {
        QString tmp; dsReadQString(r, tmp);
        return QVariant();
    }
    case kQt6Bool:
        return QVariant(bool(r.u8() != 0));
    case kQt6Int:
        return QVariant(q34i32(r.u32()));
    case kQt6UInt:
        return QVariant(q34u32(r.u32()));
    case kQt6LongLong:
        return QVariant(q34i64(r.u64()));
    case kQt6ULongLong:
        return QVariant(q34u64(r.u64()));
    case kQt6Double: {
        const q34u64 bits = r.u64();
        double d;
        ::memcpy(&d, &bits, 8);
        return QVariant(d);
    }
    case kQt6String: {
        QString s; dsReadQString(r, s);
        return QVariant(s);
    }
    case kQt6StringList: {
        const q34u32 n = r.u32();
        if (!r.ok || n > uint(r.b.length())) { r.ok = false; return QVariant(); }
        QStringList sl;
        for (uint i = 0; i < n && r.ok; ++i) {
            QString s; dsReadQString(r, s);
            sl.append(s);
        }
        return QVariant(sl);
    }
    case kQt6ByteArray: {
        const q34u32 n = r.u32();
        if (!r.ok) return QVariant();
        if (n == 0xFFFFFFFFu) return QVariant(QByteArray());
        if (r.pos + n > uint(r.b.length())) { r.ok = false; return QVariant(); }
        QByteArray a;
        a.resize(int(n));
        for (int i = 0; i < int(n); ++i)
            a[i] = char(uchar(r.b.at(r.pos++).unicode()));
        return QVariant(a);
    }
    case kQt6Map: {
        const q34u32 n = r.u32();
        if (!r.ok || n > uint(r.b.length())) { r.ok = false; return QVariant(); }
        QMap<QString, QVariant> m;
        for (uint i = 0; i < n && r.ok; ++i) {
            QString k; dsReadQString(r, k);
            m[k] = dsReadQVariant(r);
        }
        return QVariant(m);
    }
    case kQt6List: {
        const q34u32 n = r.u32();
        if (!r.ok || n > uint(r.b.length())) { r.ok = false; return QVariant(); }
        QVariantList34 l;
        for (uint i = 0; i < n && r.ok; ++i)
            l.append(dsReadQVariant(r));
        return QVariant(l);
    }
    default:
        r.ok = false;
        return QVariant();
    }
}

// ── 写：把字节追加为「每 QChar 一字节」的 QString ────────────────────────
void dsPutU32(QString& o, q34u32 v)
{
    o += QChar(ushort((v >> 24) & 0xFF));
    o += QChar(ushort((v >> 16) & 0xFF));
    o += QChar(ushort((v >>  8) & 0xFF));
    o += QChar(ushort( v        & 0xFF));
}

void dsPutU64(QString& o, q34u64 v)
{
    dsPutU32(o, q34u32((v >> 32) & 0xFFFFFFFFu));
    dsPutU32(o, q34u32( v        & 0xFFFFFFFFu));
}

void dsPutQString(QString& o, const QString& s)
{
    if (s.isNull()) {                          // null → 0xFFFFFFFF
        dsPutU32(o, 0xFFFFFFFFu);
        return;
    }
    dsPutU32(o, q34u32(uint(s.length()) * 2));
    for (uint i = 0; i < s.length(); ++i) {    // UTF-16 大端
        const uint c = s.at(i).unicode();
        o += QChar(ushort((c >> 8) & 0xFF));
        o += QChar(ushort(c & 0xFF));
    }
}

void dsWriteQVariant(QString& o, const QVariant& v)
{
    dsPutU32(o, q34u32(dsQt6IdFor(v.type())));
    switch (dsQt6IdFor(v.type())) {
    case kQt6Bool:
        o += QChar(ushort(v.toBool() ? 1 : 0));
        break;
    case kQt6Int:
        dsPutU32(o, q34u32(v.toInt()));
        break;
    case kQt6UInt:
        dsPutU32(o, q34u32(v.toUInt()));
        break;
    case kQt6LongLong:
        dsPutU64(o, q34u64(v.toLongLong()));
        break;
    case kQt6ULongLong:
        dsPutU64(o, q34u64(v.toULongLong()));
        break;
    case kQt6Double: {
        const double d = v.toDouble();
        q34u64 bits;
        ::memcpy(&bits, &d, 8);
        dsPutU64(o, bits);
        break;
    }
    case kQt6String:
        dsPutQString(o, v.toString());
        break;
    case kQt6StringList: {
        const QStringList sl = v.toStringList();
        dsPutU32(o, q34u32(uint(sl.count())));
        for (uint i = 0; i < uint(sl.count()); ++i)
            dsPutQString(o, sl[i]);
        break;
    }
    case kQt6ByteArray: {
        const QByteArray a = v.toByteArray();
        if (a.isNull()) {
            dsPutU32(o, 0xFFFFFFFFu);
        } else {
            dsPutU32(o, q34u32(uint(a.size())));
            for (int i = 0; i < int(a.size()); ++i)
                o += QChar(ushort(uchar(a[i])));
        }
        break;
    }
    case kQt6Map: {
        const QMap<QString, QVariant> m = v.toMap();
        dsPutU32(o, q34u32(uint(m.count())));
        for (QMap<QString, QVariant>::const_iterator it = m.constBegin();
             it != m.constEnd(); ++it) {
            dsPutQString(o, it.key());
            dsWriteQVariant(o, *it);
        }
        break;
    }
    case kQt6List: {
        const QVariantList34 l = v.toList();
        dsPutU32(o, q34u32(uint(l.count())));
        for (QVariantList34::const_iterator it = l.constBegin();
             it != l.constEnd(); ++it)
            dsWriteQVariant(o, *it);
        break;
    }
    default:
        dsPutQString(o, QString());            // 不支持的类型 → Unknown + null
        break;
    }
}

// QByteArray 的每个字节 → 一个 QChar（Qt6 用 QLatin1StringView，语义相同）。
QString dsBytesToQString(const QByteArray& a)
{
    QString r;
    for (int i = 0; i < int(a.size()); ++i)
        r += QChar(ushort(uchar(a[i])));
    return r;
}

// ══ @String/@ByteArray/@Variant/@Invalid/@@ ↔ QVariant ════════════════════
QVariant variantFromAnnotation(const QString& s)
{
    if (s.startsWith(QChar('@'))) {
        if (s.endsWith(QChar(')'))) {
            if (s.startsWith(QString::fromLatin1("@ByteArray(")) && s.length() >= 12) {
                const QString body = s.mid(11, s.length() - 12);
                QByteArray a;
                a.resize(int(body.length()));
                for (int i = 0; i < int(body.length()); ++i)
                    a[i] = char(uchar(body.at(uint(i)).unicode()));
                return QVariant(a);
            }
            if (s.startsWith(QString::fromLatin1("@String(")) && s.length() >= 9)
                return QVariant(s.mid(8, s.length() - 9));
            if (s.startsWith(QString::fromLatin1("@Variant(")) && s.length() >= 10) {
                DsReader r(s.mid(9, s.length() - 10));
                const QVariant v = dsReadQVariant(r);
                if (!r.ok)
                    return QVariant(s);        // 解不出 → 原样，绝不半截
                return v;
            }
            if (s == QString::fromLatin1("@Invalid()"))
                return QVariant();
        }
        if (s.startsWith(QString::fromLatin1("@@")))
            return QVariant(s.mid(1));
    }
    return QVariant(s);
}

// 对齐 variantToString()（qsettings.cpp:371）。@DateTime 本实现不生成、不解码。
QString variantAnnotation(const QVariant& v)
{
    switch (v.type()) {
    case QVariant::Invalid:
        return QString::fromLatin1("@Invalid()");

    case QVariant::ByteArray:
        return QString::fromLatin1("@ByteArray(")
               + dsBytesToQString(v.toByteArray()) + QChar(')');

    case QVariant::String:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Bool:
    case QVariant::Double: {
        QString result = v.toString();
        if (result.contains(QChar(0)))
            result = QString::fromLatin1("@String(") + result + QChar(')');
        else if (result.startsWith(QChar('@')))
            result = QString(QChar('@')) + result;
        return result;
    }

    default: {
        QString bytes;
        dsWriteQVariant(bytes, v);
        return QString::fromLatin1("@Variant(") + bytes + QChar(')');
    }
    }
}

// 对齐 variantListToStringList()/stringListToVariantList()（qsettings.cpp:342）。
QStringList variantListToStringList(const QVariantList34& l)
{
    QStringList result;
    for (QVariantList34::const_iterator it = l.constBegin(); it != l.constEnd(); ++it)
        result.append(variantAnnotation(*it));
    return result;
}

QVariant stringListToVariantList(const QStringList& l)
{
    QStringList out = l;
    for (uint i = 0; i < uint(out.count()); ++i) {
        const QString str = out[i];
        if (str.startsWith(QChar('@'))) {
            if (str.length() < 2 || str.at(1) != QChar('@')) {
                QVariantList34 variantList;
                for (uint j = 0; j < uint(l.count()); ++j)
                    variantList.append(variantFromAnnotation(l[j]));
                return QVariant(variantList);
            }
            out[i] = out[i].mid(1);
        }
    }
    return QVariant(out);
}

// 对齐 iniEscapedStringList()（qsettings.cpp:676）。
void iniEscapedStringList(const QStringList& strs, QSettingsBuffer& result)
{
    if (strs.isEmpty()) {
        result += "@Invalid()";
    } else {
        for (uint i = 0; i < uint(strs.count()); ++i) {
            if (i != 0)
                result += ", ";
            iniEscapedString(strs[i], result);
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════
// 自包含子模块（不依赖 stikcommon 的 shim）：原子写回所需的最小工具。
// QSaveFile（Qt5.1 才原生）的语义 = 写同目录临时文件，commit() 时 ::rename(2)
// 原子替换；中断/崩溃/写失败都不破坏原文件。
// ═════════════════════════════════════════════════════════════════════════

// qAbsPath(fi)：Qt3 的 QFileInfo 用 dirPath(true)，Qt4 用 absolutePath()，
// 语义一致，都返回所在目录的绝对路径（Qt3 无 absolutePath()）。
static inline QString localAbsPath(const QFileInfo& fi)
{
#ifdef QT3_BUILD
    return fi.dirPath(true);
#else
    return fi.absolutePath();
#endif
}

// qFind(s, c, from)：QString 查 QChar——Qt3 叫 find()，Qt4 的 find() 被
// QT3_SUPPORT 守卫（本机 Qt4 为 -no-qt3support 构建，未定义），改名 indexOf()。
static inline int q34Find(const QString& s, QChar c, int from)
{
#ifdef QT3_BUILD
    return s.find(c, from);
#else
    return s.indexOf(c, from);
#endif
}

// qMkpath(path)：对齐 QDir::mkpath（Qt3 的 QDir 只有单层 mkdir）。
// 契约：已存在 → true；任一中间级失败 → false。已存在目录用 isDir 短路
// （Qt3 的 QDir::mkdir 对已存在返回 false）。
static bool localMkpath(const QString& path)
{
    QString p = path;
    while (p.endsWith(QChar('/')))
        p.truncate(p.length() - 1);
    if (p.isEmpty())
        return false;
    if (QFileInfo(p).isDir())
        return true;
    int slashPos = 0;
    while ((slashPos = q34Find(p, QChar('/'), slashPos + 1)) != -1)
        QDir().mkdir(p.left(uint(slashPos)));
    // 末级可能已被并发创建，补一次 isDir 判定。
    return QDir().mkdir(p) || QFileInfo(p).isDir();
}

// 目标同目录生成一个未被占用的临时名（同目录 = 同文件系统，rename 不会 EXDEV）。
static QString localTmpName(const QString& target)
{
    const QFileInfo fi(target);
    const QString dir = localAbsPath(fi);
    const QString base = fi.fileName();
    for (int i = 0; i < 4096; ++i) {
        const QString cand = dir + QString::fromLatin1("/") + base
                             + QString::fromLatin1(".tmp") + QString::number(i);
        if (!QFileInfo(cand).exists())
            return cand;
    }
    return QString();
}

// ::rename(2) 原子替换，路径用 QFile::encodeName（与文件 API 同一套编码）。
static bool localAtomicRename(const QString& from, const QString& to)
{
    const QSettingsBuffer f = QFile::encodeName(from);
    const QSettingsBuffer t = QFile::encodeName(to);
    if (f.isEmpty() || t.isEmpty())
        return false;
    return ::rename(f.data(), t.data()) == 0;
}

// QSaveFile 的最小版（只用写到 sync() 的调用面）。
class SaveFile
{
public:
    explicit SaveFile(const QString& fileName)
        : m_fileName(fileName), m_tmpFile(0), m_open(false), m_error(false)
    {
    }
    ~SaveFile()
    {
        cancelWriting();
    }

    bool open(int mode)
    {
        if (m_tmpFile)
            return false;                       // 已打开
        m_tmpName = localTmpName(m_fileName);
        if (m_tmpName.isEmpty()) {
            m_error = true;
            return false;
        }
        m_tmpFile = new QFile(m_tmpName);
#ifdef QT3_BUILD
        if (!m_tmpFile->open(mode)) {
#else
        if (!m_tmpFile->open(QIODevice::OpenModeFlag(mode))) {
#endif
            delete m_tmpFile;
            m_tmpFile = 0;
            m_error = true;
            return false;
        }
        m_open = true;
        m_error = false;
        return true;
    }

    long long write(const char* data, long long len)
    {
        if (!m_tmpFile || !m_open) {
            m_error = true;
            return -1;
        }
#ifdef QT3_BUILD
        return (long long)m_tmpFile->writeBlock(data, (Q_ULONG)len);
#else
        return (long long)m_tmpFile->write(data, (q34i64)len);
#endif
    }

    bool commit()
    {
        if (!m_tmpFile || !m_open) {
            m_error = true;
            return false;
        }
        m_tmpFile->flush();
        m_tmpFile->close();
        m_open = false;
        if (!localAtomicRename(m_tmpName, m_fileName)) {
            m_error = true;
            return false;                       // 临时文件留待析构清理
        }
        delete m_tmpFile;
        m_tmpFile = 0;
        return true;
    }

    void cancelWriting()
    {
        if (m_tmpFile) {
            if (m_open)
                m_tmpFile->close();
            m_tmpFile->remove();                // 未 commit 即放弃本次写入
            delete m_tmpFile;
            m_tmpFile = 0;
        }
        m_open = false;
    }

private:
    SaveFile(const SaveFile&);                  // 不可拷贝
    SaveFile& operator=(const SaveFile&);

    QString m_fileName;    // 目标路径
    QString m_tmpName;     // 临时路径（同目录）
    QFile*  m_tmpFile;
    bool    m_open;
    bool    m_error;
};

} // namespace

QSettings34::QSettings34(const QString& org, const QString& app)
    : m_org(org), m_app(app), m_dirty(false)
{
    load();
}

QString QSettings34::userConfigDir()
{
    // 与 QStandardPaths::ConfigLocation 对齐：$XDG_CONFIG_HOME 为绝对路径时优先，
    // 否则 $HOME/.config。
    const char* xdg = ::getenv("XDG_CONFIG_HOME");
    if (xdg && xdg[0] == '/')
        return QString::fromLocal8Bit(xdg);
    const char* home = ::getenv("HOME");
    const QString h = home ? QString::fromLocal8Bit(home) : QString::fromLatin1(".");
    return h + QString::fromLatin1("/.config");
}

QString QSettings34::filePathFor(const QString& org, const QString& app)
{
    return userConfigDir() + QString::fromLatin1("/") + org
         + QString::fromLatin1("/") + app + QString::fromLatin1(".conf");
}

QString QSettings34::fileName() const
{
    return filePathFor(m_org, m_app);
}

void QSettings34::load()
{
    m_sections.clear();
    m_dirty = false;

    QFile f(fileName());
    if (!f.exists())
        return;
#ifdef QT3_BUILD
    if (!f.open(IO_ReadOnly))
#else
    if (!f.open(QIODevice::ReadOnly))
#endif
        return;

    const QByteArray rawAll = f.readAll();
    const char* data = rawAll.data();
    int dataLen = int(rawAll.size());
    if (dataLen == 0 || data == 0)
        return;

    // 跳过可能的 UTF-8 BOM（qsettings.cpp:1651）。
    if (dataLen >= 3 && uchar(data[0]) == 0xEF && uchar(data[1]) == 0xBB
        && uchar(data[2]) == 0xBF) {
        data += 3;
        dataLen -= 3;
    }

    QString currentSection;   // "" 表示 General，否则 "段/"(带尾斜杠)
    int dataPos = 0;
    int lineStart = 0, lineLen = 0, equalsPos = 0;

    while (readIniLine(data, dataLen, dataPos, lineStart, lineLen, equalsPos)) {
        if (data[lineStart] == '[') {
            // 段头：[Name]，取 ] 之前的内容并 trim；[General]→""，[%General]→"General/"。
            int idx = -1;
            for (int k = lineStart; k < lineStart + lineLen; ++k) {
                if (data[k] == ']') { idx = k; break; }
            }
            QSettingsBuffer sec;
            if (idx < 0) {
                sec = QSettingsBuffer(data + lineStart + 1, uint(lineLen));
            } else {
                sec = QSettingsBuffer(data + lineStart + 1, uint(idx - lineStart));
            }
            sec = sec.stripWhiteSpace();
            const QString s = QString::fromLatin1(sec.data(), int(sec.size()));
#ifdef QT3_BUILD
            const QString sLower = s.lower();
#else
            const QString sLower = s.toLower();
#endif
            if (sLower == QString::fromLatin1("general")) {
                currentSection = QString();
            } else if (sLower == QString::fromLatin1("%general")) {
                currentSection = s.mid(1) + QChar('/');
            } else {
                QString us;
                iniUnescapedKey(sec, us);
                currentSection = us + QChar('/');
            }
            continue;
        }

        if (equalsPos == -1)
            continue;   // 注释行或非法行

        const int eq = equalsPos - lineStart;
        if (eq < 0 || eq >= lineLen)
            continue;

        QSettingsBuffer keyBytes(data + lineStart, uint(eq) + 1);
        keyBytes = keyBytes.stripWhiteSpace();
        QSettingsBuffer valueBytes(data + lineStart + eq + 1, uint(lineLen - eq));

        QString leaf;
        iniUnescapedKey(keyBytes, leaf);
        if (leaf.isEmpty())
            continue;

        // 段名一律用「非 null 的空串」表示 General：Qt3 的 QString()（null）在
        // QMap 里排在**最后**（与 Qt6 的空键排最前相反），会让 [General] 落尾。
        QString section = QString::fromLatin1("");
        if (!currentSection.isEmpty())
            section = currentSection.left(currentSection.length() - 1);

        Entry e;
        QString strValue;
        QStringList strList;
        if (iniUnescapedStringList(valueBytes, strValue, strList))
            e.variant = stringListToVariantList(strList);
        else
            e.variant = variantFromAnnotation(strValue);
        e.rawValue = valueBytes;
        e.touched = false;
        m_sections[section][leaf] = e;
    }
}

QString QSettings34::normalizeKey(const QString& key)
{
    QString k = key;
    while (k.startsWith(QChar('/')))
        k.remove(0, 1);
    return k;
}

void QSettings34::splitKey(const QString& key, QString& section, QString& leaf)
{
    const QString k = normalizeKey(key);
    const int slash = q34Find(k, QChar('/'), 0);
    if (slash < 0) {
        section = QString::fromLatin1("");
        leaf = k;
    } else {
        section = k.left(uint(slash));
        leaf = k.mid(uint(slash + 1));
    }
}

bool QSettings34::contains(const QString& key) const
{
    QString section, leaf;
    splitKey(key, section, leaf);
    QMap<QString, QMap<QString, Entry> >::const_iterator s = m_sections.find(section);
    if (s == m_sections.constEnd())
        return false;
    return (*s).contains(leaf);
}

QVariant QSettings34::value(const QString& key, const QVariant& def) const
{
    QString section, leaf;
    splitKey(key, section, leaf);
    QMap<QString, QMap<QString, Entry> >::const_iterator s = m_sections.find(section);
    if (s == m_sections.constEnd())
        return def;
    QMap<QString, Entry>::const_iterator e = (*s).find(leaf);
    if (e == (*s).constEnd())
        return def;
    return (*e).variant;
}

QString QSettings34::valueStr(const QString& key, const QString& def) const
{
    QString section, leaf;
    splitKey(key, section, leaf);
    QMap<QString, QMap<QString, Entry> >::const_iterator s = m_sections.find(section);
    if (s == m_sections.constEnd())
        return def;
    QMap<QString, Entry>::const_iterator e = (*s).find(leaf);
    if (e == (*s).constEnd())
        return def;
    const QVariant& v = (*e).variant;
    if (v.type() == QVariant::StringList || v.type() == QVariant::List)
        return def;
    return v.toString();
}

QStringList QSettings34::valueStringList(const QString& key,
                                         const QStringList& def) const
{
    const QVariant v = value(key, QVariant());
    if (!v.isValid())
        return def;
    return v.toStringList();
}

int QSettings34::valueInt(const QString& key, int def) const
{
    const QVariant v = value(key, QVariant());
    if (!v.isValid())
        return def;
    bool ok = false;
    const int r = v.toInt(&ok);
    return ok ? r : def;
}

bool QSettings34::valueBool(const QString& key, bool def) const
{
    const QVariant v = value(key, QVariant());
    if (!v.isValid())
        return def;
    if (v.type() == QVariant::Bool)
        return v.toBool();
    const QString s = v.toString();
    if (s == QString::fromLatin1("true") || s == QString::fromLatin1("1"))
        return true;
    if (s == QString::fromLatin1("false") || s == QString::fromLatin1("0"))
        return false;
    return def;
}

double QSettings34::valueDouble(const QString& key, double def) const
{
    const QVariant v = value(key, QVariant());
    if (!v.isValid())
        return def;
    if (v.type() == QVariant::Double)
        return v.toDouble();
    bool ok = false;
    const double r = v.toString().toDouble(&ok);
    return ok ? r : def;
}

void QSettings34::setValue(const QString& key, const QVariant& value)
{
    QString section, leaf;
    splitKey(key, section, leaf);
    Entry e;
    e.variant = value;
    e.touched = true;
    m_sections[section][leaf] = e;
    m_dirty = true;
}

void QSettings34::setValueStringList(const QString& key, const QStringList& value)
{
    setValue(key, QVariant(value));
}

void QSettings34::setValueInt(const QString& key, int value)
{
    setValue(key, QVariant(value));
}

void QSettings34::remove(const QString& key)
{
    QString section, leaf;
    splitKey(key, section, leaf);
    QMap<QString, QMap<QString, Entry> >::iterator s = m_sections.find(section);
    if (s == m_sections.end())
        return;
    (*s).remove(leaf);
    if ((*s).isEmpty())
        m_sections.remove(section);
    m_dirty = true;
}

void QSettings34::clear()
{
    m_sections.clear();
    m_dirty = true;
}

QStringList QSettings34::allKeys() const
{
    QStringList out;
    for (QMap<QString, QMap<QString, Entry> >::const_iterator s = m_sections.constBegin();
         s != m_sections.constEnd(); ++s) {
        for (QMap<QString, Entry>::const_iterator e = (*s).constBegin();
             e != (*s).constEnd(); ++e) {
            if (s.key().isEmpty())
                out.append(e.key());
            else
                out.append(s.key() + QChar('/') + e.key());
        }
    }
    out.sort();
    return out;
}

QStringList QSettings34::childKeys(const QString& prefix) const
{
    const QString p = normalizeKey(prefix);
    const QString withSlash = p.isEmpty() ? QString() : (p + QChar('/'));

    QStringList out;
    const QStringList keys = allKeys();
    for (uint i = 0; i < keys.count(); ++i) {
        const QString k = keys[i];
        if (!p.isEmpty()) {
            if (!k.startsWith(withSlash))
                continue;
        }
        const QString rest = k.mid(withSlash.length());
        if (!rest.isEmpty() && q34Find(rest, QChar('/'), 0) < 0)
            out.append(rest);
    }
    out.sort();
    return out;
}

bool QSettings34::isWritable() const
{
    const QString path = fileName();
    QFileInfo fi(path);
    if (fi.exists())
        return fi.isWritable();
    // 文件不存在：看父目录可否建文件。
    QFileInfo pfi(localAbsPath(fi));
    return pfi.isDir() && pfi.isWritable();
}

bool QSettings34::sync()
{
    if (!m_dirty)
        return true;

    const QString path = fileName();
    if (!localMkpath(localAbsPath(QFileInfo(path))))
        return false;

    SaveFile f(path);
#ifdef QT3_BUILD
    if (!f.open(IO_WriteOnly))
#else
    if (!f.open(QIODevice::WriteOnly))
#endif
        return false;

    QSettingsBuffer out;
    bool firstSection = true;

    for (QMap<QString, QMap<QString, Entry> >::const_iterator s = m_sections.constBegin();
         s != m_sections.constEnd(); ++s) {
        QSettingsBuffer header;
        const QString section = s.key();
        if (section.isEmpty()) {
            header = "[General]";
        } else {
            QSettingsBuffer esc;
            iniEscapedKey(section, esc);
            const QString escStr = QString::fromLatin1(esc.data(), int(esc.size()));
#ifdef QT3_BUILD
            if (escStr.lower() == QString::fromLatin1("general")) {
#else
            if (escStr.toLower() == QString::fromLatin1("general")) {
#endif
                header = "[%General]";
            } else {
                header = "[";
                header += (const char*)esc;
                header += "]";
            }
        }
        if (!firstSection)
            out += '\n';
        out += header;
        out += '\n';
        firstSection = false;

        for (QMap<QString, Entry>::const_iterator e = (*s).constBegin();
             e != (*s).constEnd(); ++e) {
            QSettingsBuffer block;
            iniEscapedKey(e.key(), block);
            block += '=';
            if ((*e).touched) {
                const QVariant& v = (*e).variant;
                if (v.type() == QVariant::StringList
                    || (v.type() == QVariant::List && v.toList().count() != 1)) {
                    iniEscapedStringList(variantListToStringList(v.toList()), block);
                } else {
                    iniEscapedString(variantAnnotation(v), block);
                }
            } else {
                block += (const char*)(*e).rawValue;
            }
            block += '\n';
            out += block;
        }
    }

    // QSettingsBuffer::size() 在 Qt3 下含尾随 NUL（转自真 QCString 语义），
    // 写文件必须按 length() 取精确字节数。
    if (f.write(out.data(), out.length()) < 0)
        return false;
    if (!f.commit())
        return false;

    m_dirty = false;
    return true;
}
