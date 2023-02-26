#ifndef __PT_EVNET_EXTERN_H__
#define __PT_EVNET_EXTERN_H__
#include "pt_port.h"
#include "pt.h"
#include "pt_timers.h"

#define PT_EVENT_NULL 0
typedef struct
{
    volatile unsigned int mark;
} pt_event_t;

// 初始事件,可以在非PT协程中调用（调用后不进行任务切换）
#define PT_EVNET_INIT(s, c) (s)->mark = c
// 发送事件,可以在非PT协程中调用（调用后不进行任务切换）
#define PT_EVNET_SET_BIT(s, e)   \
    do                           \
    {                            \
        (s)->mark |= (e);        \
        WAKE_UP_PT_THREAD_SEM(); \
    }                            \
    while (0)

// 清空一个事件,可以在非PT协程中调用（调用后不进行任务切换）
#define PT_EVNET_CLEAR_BIT(s, e) (s)->mark &= (~e)
// 获取事件掩码,可以在非PT协程中调用（调用后不进行任务切换）
#define PT_EVNET_GET_MARK(s) ((s)->mark)

// 等待事件存在,最多等待多少MS，调用后必定进行任务调度
#define PT_EVNET_WAIT(pt, s, ms) __PT_TIME_ENABLE(pt, (s)->mark, {}, ms)

#endif
