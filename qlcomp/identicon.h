#ifndef IDENTICON_H
#define IDENTICON_H

#include <qpixmap.h>
#include <qstring.h>

QPixmap generateIdenticon(const QString& seed, int size);

/* 诊断：identicon 缓存条目数 / 估算字节数 */
int identiconCacheCount();
long long identiconCacheBytes();

#endif
