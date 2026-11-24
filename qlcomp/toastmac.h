#ifndef TOASTMAC_H
#define TOASTMAC_H

#ifdef __cplusplus
extern "C" {
#endif

/* 记录/查询/还原 "show 之前的前台 app"（Carbon PSN） */
int toastmac_snapshot_front(void);   /* 1=成功，0=失败 */
int toastmac_front_is_self(void);    /* 1=本进程，0=否，-1=未知 */
int toastmac_restore_front(void);    /* 1=已还原，0=无需/已还，-1=失败 */

/* 宿主窗口全局 z 序号（0 = 最底），-1 = 未知 */
int toastmac_global_z(void* view);

/* toast 支持跨 Space / 全屏 Space 显示 */
int toastmac_set_collection_behavior(void* view);

/* 宿主窗口 z 序快照：仅本进程内窗口相对顺序；跨 app 用上面的 front 接口 */
void* toastmac_order_snapshot(void* view);   /* 返回不透明句柄，0=失败 */
int toastmac_order_violated(void* handle);   /* 1=宿主被抬到原上层窗口之上 */
int toastmac_order_restore(void* handle);    /* 1=已恢复，0=无法恢复 */
void toastmac_order_release(void* handle);

#ifdef __cplusplus
}
#endif

#endif
