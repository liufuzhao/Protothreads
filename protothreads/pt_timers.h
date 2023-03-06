/*
 * 定时器管理
 * 提供timer server 支持callback 函数调用
 * 给pt 进行delay 拓展
 *
 */
#ifndef _PT_TIMER_H
#define _PT_TIMER_H

#include "pt.h"
#include "pt_list.h"

// 确保调用time 一定会进行任务调度
#define PT_TIME_PRE_STOP_STATE    1
#define PT_TIME_STOP_STATE        2
#define PT_TIME_PRE_RUN_STATE     3
#define PT_TIME_RUN_STATE         4
#define PT_TIME_PRE_FOREVER_STATE 5
#define PT_TIME_FOREVER_STATE     6

// 预留4个bit用作状态处理
#define PT_WAIT_FOREVER 0x0FFFFFFFUL 
#define PT_NONBLOCK     0

#define PT_SLEEP_MS(pt, ms)                                                \
    _pt_timer_start(&((container_of(pt, pt_thread_info, pt))->timer), ms); \
    PT_WAIT_UNTIL(pt, pt_timer_timeout(&((container_of(pt, pt_thread_info, pt))->timer)));

#define PT_IS_TIMEOUT(pt) container_of(pt, pt_thread_info, pt)->status.time_out

#define _PT_TIME_ENABLE(pt, condition, action, ms)                                                                    \
    do                                                                                                                \
    {                                                                                                                 \
        _pt_timer_start(&((container_of(pt, pt_thread_info, pt))->timer), ms);                                        \
        container_of(pt, pt_thread_info, pt)->status.monitor_view = PT_TRUE;                                          \
        __PT_YIELD_WAIT_UNTIL(pt, (pt_timer_first_run(&((container_of(pt, pt_thread_info, pt))->timer)) == PT_FALSE), \
                              ((condition) || pt_timer_timeout(&((container_of(pt, pt_thread_info, pt))->timer))));   \
        container_of(pt, pt_thread_info, pt)->status.time_out =                                                       \
            pt_timer_timeout(&((container_of(pt, pt_thread_info, pt))->timer));                                       \
        action;                                                                                                       \
    }                                                                                                                 \
    while (0)

#define _PT_YIELD_ENABLE(pt, condition, action, uint8_t_cnt)                      \
    do                                                                            \
    {                                                                             \
        container_of(pt, pt_thread_info, pt)->status.time_out = PT_TRUE;          \
        container_of(pt, pt_thread_info, pt)->status.yield_cnt = uint8_t_cnt;     \
        container_of(pt, pt_thread_info, pt)->status.yield_run_cnt = 0;           \
        container_of(pt, pt_thread_info, pt)->status.monitor_view = PT_TRUE;      \
        for (; container_of(pt, pt_thread_info, pt)->status.yield_run_cnt <=      \
               container_of(pt, pt_thread_info, pt)->status.yield_cnt;            \
             container_of(pt, pt_thread_info, pt)->status.yield_run_cnt++)        \
        {                                                                         \
            if (!condition)                                                       \
                PT_YIELD(pt);                                                     \
            else                                                                  \
            {                                                                     \
                container_of(pt, pt_thread_info, pt)->status.time_out = PT_FALSE; \
                break;                                                            \
            }                                                                     \
        }                                                                         \
        action;                                                                   \
    }                                                                             \
    while (0)

typedef struct
{
    uint32_t status:4;
    uint32_t timeout_tick:28;
    uint32_t start_tick;
} pt_timer_t;

void _pt_timer_start(pt_timer_t *t, uint32_t timeout);

int pt_timer_timeout(pt_timer_t *t);

typedef void (*timers_callback_t)(pt_timer_t *time, void *para);

typedef struct
{
    slist_t list;
    pt_timer_t time;
    void *para;
    timers_callback_t prtCb;
} timer_extern_t;

int pt_timer_first_run(pt_timer_t *t);
uint32_t timers_server_get_block_time_ms(void);
int pt_timer_register(timer_extern_t *t, timers_callback_t cb);
int pt_timer_start(pt_timer_t *t, uint32_t ms);
int pt_timer_stop(pt_timer_t *t);
int pt_timers_server_init(void);
void *get_timers_server_thread(void);
PT_THREAD(timers_server_thread(struct pt *pt));

#endif // _PT_TIMER_H
