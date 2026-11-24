#include "toastmac.h"

#include <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

namespace {

struct OrderSnapshot {
    NSWindow* host;      /* 不 retain，由 Qt 管理 */
    NSWindow* above;     /* retain：快照时紧邻宿主上方的窗口 */
    int index;           /* 快照时在 [NSApp orderedWindows] 里的下标 */
};

int frontIsSelfValue()
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

int windowGlobalZ(NSWindow* w)
{
    if (w == nil) {
        return -1;
    }
    CFArrayRef list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly,
                                                 kCGNullWindowID);
    if (list == 0) {
        return -1;
    }
    int count = (int)CFArrayGetCount(list);
    int index = -1;
    int num = (int)[w windowNumber];
    for (int i = 0; i < count; ++i) {
        CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(list, i);
        CFNumberRef got = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowNumber);
        if (got == 0) {
            continue;
        }
        int wid = 0;
        if (!CFNumberGetValue(got, kCFNumberIntType, &wid)) {
            continue;
        }
        if (wid == num) {
            index = count - 1 - i;   /* CG 是前→后，转成 0 = 最底 */
            break;
        }
    }
    CFRelease(list);
    return index;
}

} // namespace

/* 全局保存：同一时刻只保留一个待还的前台 app（多 toast 时后 snapshot 覆盖前一个） */
static ProcessSerialNumber g_frontPSN;
static int g_frontValid = 0;
static int g_frontWasSelf = 0;
static int g_frontRestored = 0;

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
    if (view == 0) {
        return -1;
    }
    return windowGlobalZ([(NSView*)view window]);
}

extern "C" int toastmac_set_collection_behavior(void* view)
{
    if (view == 0) {
        return 0;
    }
    NSWindow* w = [(NSView*)view window];
    if (w == nil) {
        return 0;
    }
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    NSWindowCollectionBehavior cb = [w collectionBehavior];
    cb |= NSWindowCollectionBehaviorCanJoinAllSpaces;
    cb |= NSWindowCollectionBehaviorFullScreenAuxiliary;
    [w setCollectionBehavior:cb];
#endif
    return 1;
}

extern "C" void* toastmac_order_snapshot(void* view)
{
    if (view == 0) {
        return 0;
    }
    NSWindow* w = [(NSView*)view window];
    if (w == nil) {
        return 0;
    }
    NSArray* ordered = [NSApp orderedWindows];
    if (ordered == nil) {
        return 0;
    }
    NSUInteger idx = [ordered indexOfObjectIdenticalTo:w];
    if (idx == NSNotFound) {
        return 0;
    }
    OrderSnapshot* snap = new OrderSnapshot();
    snap->host = w;
    snap->above = 0;
    if (idx > 0) {
        snap->above = (NSWindow*)[[ordered objectAtIndex:(idx - 1)] retain];
    }
    snap->index = (int)idx;
    return snap;
}

extern "C" int toastmac_order_violated(void* handle)
{
    OrderSnapshot* snap = (OrderSnapshot*)handle;
    if (snap == 0 || snap->host == nil) {
        return 0;
    }
    NSArray* ordered = [NSApp orderedWindows];
    if (ordered == nil) {
        return 0;
    }
    NSUInteger hostIdx = [ordered indexOfObjectIdenticalTo:snap->host];
    if (hostIdx == NSNotFound) {
        return 0;
    }
    if ((int)hostIdx < snap->index) {
        return 1;   /* 宿主向队首移动 = 被抬升 */
    }
    if (snap->above != nil) {
        NSUInteger aboveIdx = [ordered indexOfObjectIdenticalTo:snap->above];
        if (aboveIdx != NSNotFound && aboveIdx > hostIdx) {
            return 1;   /* 原上层窗口现在排到宿主后面 */
        }
    }
    return 0;
}

extern "C" int toastmac_order_restore(void* handle)
{
    OrderSnapshot* snap = (OrderSnapshot*)handle;
    if (snap == 0 || snap->host == nil || snap->above == nil) {
        return 0;
    }
    if (![snap->above isVisible]) {
        return 0;
    }
    [snap->host orderWindow:NSWindowBelow relativeTo:[snap->above windowNumber]];
    return 1;
}

extern "C" void toastmac_order_release(void* handle)
{
    OrderSnapshot* snap = (OrderSnapshot*)handle;
    if (snap == 0) {
        return;
    }
    if (snap->above != nil) {
        [snap->above release];   /* 非 ARC，配对 snapshot 里的 retain */
        snap->above = nil;
    }
    delete snap;
}
