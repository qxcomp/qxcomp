#include "toastmac.h"

#include <Carbon/Carbon.h>

/* Qt3-mac 只有 Carbon（没有 Cocoa）：
   - 前台 app 记录/还原可用（SetFrontProcessWithOptions）
   - 全局 z 序号、进程内窗口重排不可靠，返回 -1/0，调用方降级为 log-only */

static ProcessSerialNumber g_frontPSN;
static int g_frontValid = 0;
static int g_frontWasSelf = 0;
static int g_frontRestored = 0;

static int frontIsSelfValue()
{
    ProcessSerialNumber self;
    ProcessSerialNumber front;
    Boolean same = false;
    if (GetCurrentProcess(&self) != noErr) {
        return -1;
    }
    if (GetFrontProcess(&front) != noErr) {
        return -1;
    }
    if (SameProcess(&front, &self, &same) != noErr) {
        return -1;
    }
    return same ? 1 : 0;
}

extern "C" int toastmac_snapshot_front(void)
{
    ProcessSerialNumber self;
    Boolean same = false;
    if (GetCurrentProcess(&self) != noErr) {
        return 0;
    }
    if (GetFrontProcess(&g_frontPSN) != noErr) {
        return 0;
    }
    if (SameProcess(&g_frontPSN, &self, &same) != noErr) {
        return 0;
    }
    g_frontWasSelf = same ? 1 : 0;
    g_frontValid = 1;
    g_frontRestored = 0;
    return 1;
}

extern "C" int toastmac_front_is_self(void)
{
    return frontIsSelfValue();
}

extern "C" int toastmac_restore_front(void)
{
    if (g_frontValid == 0 || g_frontWasSelf != 0 || g_frontRestored != 0) {
        return 0;
    }
    g_frontRestored = 1;
    OSStatus st = SetFrontProcessWithOptions(&g_frontPSN, kSetFrontProcessFrontWindowOnly);
    return (st == noErr) ? 1 : -1;
}

extern "C" int toastmac_global_z(void* view)
{
    (void)view;
    return -1;
}

extern "C" int toastmac_set_collection_behavior(void* view)
{
    (void)view;
    return 0;
}

extern "C" void* toastmac_order_snapshot(void* view)
{
    (void)view;
    return 0;
}

extern "C" int toastmac_order_violated(void* handle)
{
    (void)handle;
    return 0;
}

extern "C" int toastmac_order_restore(void* handle)
{
    (void)handle;
    return 0;
}

extern "C" void toastmac_order_release(void* handle)
{
    (void)handle;
}
