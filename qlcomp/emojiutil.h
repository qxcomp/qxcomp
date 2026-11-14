#ifndef EMOJIUTIL_H
#define EMOJIUTIL_H

#include <stdint.h>
#include <vector>
#include <string>
#include <qstring.h>
#include <qfontmetrics.h>
#include <qpainter.h>

bool isEmojiChar(uint32_t codepoint);
int emojiCharWidth(const QFontMetrics& fm);
std::vector<uint32_t> toCodepoints(const QString& text);

#ifdef EMOJI_RENDER_QT34
#include <list>
#include <map>

bool textHasEmoji(const QString& text);

class EmojiRenderer {
public:
    static EmojiRenderer& instance();
    void drawText(QPainter& p, const QRect& textRect, const QString& text);
    void drawText(QPainter& p, const QRect& textRect, const QString& text, int emojiSize);
    QPixmap renderEmoji(uint32_t codepoint, int size);

    /* 诊断：缓存条目数 / 估算字节数 */
    int cacheCount() const { return (int)m_cache.size(); }
    long long cacheBytes() const { return m_cacheBytes; }
    static const int kCacheMax = 4096;
private:
    EmojiRenderer();
    ~EmojiRenderer();
    QPixmap loadEmoji(uint32_t codepoint, int size);
    static long long pixmapBytes(const QPixmap& pm);
    void cachePut(uint64_t key, const QPixmap& pm);
    void* m_lib;
    void* m_face;
    bool m_ok;
    std::string m_fontPath;
    std::map<uint64_t, QPixmap> m_cache;
    std::list<uint64_t> m_order;      /* FIFO 淘汰顺序 */
    long long m_cacheBytes = 0;
};
#endif

#endif
