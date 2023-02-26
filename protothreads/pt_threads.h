/*
 * 对protothread的封装
 * 对protothread进行封装并提供了delays的功能
 * time calback 功能由pt_timers 提供
 */
#ifndef __PT_THREAD_H
#define __PT_THREAD_H

#include "pt_port.h"
#include "pt.h"
#include "pt_list.h"
#include "pt_timers.h"

#define __PT_STR_CONNECT2(__0, __1) __0##__1

#define container_of(ptr, type, member) ((type *)((unsigned char *)ptr - (unsigned char *)(&(((type *)0)->member))))

typedef char (*pt_thread_t)(struct pt *);

#define __PT_YIELD_CNT_ENABLE(pt, condition, uint8_t_cnt)                     \
    do                                                                        \
    {                                                                         \
        container_of(pt, pt_thread_info, pt)->status.yield_cnt = uint8_t_cnt; \
        container_of(pt, pt_thread_info, pt)->status.yield_run_cnt = 0;       \
        for (; container_of(pt, pt_thread_info, pt)->status.yield_run_cnt <=  \
               container_of(pt, pt_thread_info, pt)->status.yield_cnt;        \
             container_of(pt, pt_thread_info, pt)->status.yield_run_cnt++)    \
        {                                                                     \
            if (!condition)                                                   \
                PT_YIELD(pt);                                                 \
            else                                                              \
                break;                                                        \
        }                                                                     \
    }                                                                         \
    while (0)

// 区分yield和waiting
#define __PT_YIELD_WAIT_UNTIL(pt, yield_condition, condition) \
    do                                                        \
    {                                                         \
        LC_SET((pt)->lc);                                     \
        if (!(yield_condition))                               \
        {                                                     \
            return PT_YIELDED;                                \
        }                                                     \
        if (!(condition))                                     \
        {                                                     \
            return PT_WAITING;                                \
        }                                                     \
    }                                                         \
    while (0)

// 切换子线程，并且返回子线程的yield和waiting状态
// #define PT_SPAWN_EX(pt, thread)                                                                           \
//     do                                                                                                    \
//     {                                                                                                     \
//         pt_thread_info *pt_info = container_of(pt, pt_thread_info, pt);                                   \
//         if (!slist_empty(pt_info->sub_head))                                                              \
//         {                                                                                                 \
//             int __PT_STR_CONNECT2(ret, __LINE__);                                                         \
//             pt_thread_info *sub_pt_info = slist_first_entry(pt_info->sub_head, pt_thread_info, sub_head); \
//             PT_INIT((sub_pt_info->pt));                                                                   \
//             LC_SET((pt)->lc);                                                                             \
//             ret = thread;                                                                                 \
//             if (PT_YIELDED == ret)                                                                        \
//             {                                                                                             \
//                 return PT_YIELDED;                                                                        \
//             }                                                                                             \
//             else if (PT_WAITING == ret)                                                                   \
//             {                                                                                             \
//                 return PT_WAITING;                                                                        \
//             }                                                                                             \
//         }                                                                                                 \
//     }                                                                                                     \
//     while (0)

// 关联线程,切换子线程，并且返回子线程的yield和waiting状态
#define PT_SPAWN_EX(pt, child, thread)                                                                                       \
    do                                                                                                                       \
    {                                                                                                                        \
        if (container_of(pt, pt_thread_info, pt)->status.super)                                                              \
        {                                                                                                                    \
            slist_add_tail(&(container_of(child, pt_thread_info, pt)->list),                                                 \
                           &(container_of(pt, pt_thread_info, pt)->sub_head));                                               \
        }                                                                                                                    \
        else                                                                                                                 \
        {                                                                                                                    \
            slist_add_tail(&(container_of(child, pt_thread_info, pt)->list), &(container_of(pt, pt_thread_info, pt)->list)); \
        }                                                                                                                    \
        int __PT_STR_CONNECT2(ret, __LINE__);                                                                                \
        PT_INIT((child));                                                                                                    \
        LC_SET((pt)->lc);                                                                                                    \
        __PT_STR_CONNECT2(ret, __LINE__) = thread;                                                                           \
        if (PT_YIELDED == __PT_STR_CONNECT2(ret, __LINE__))                                                                  \
        {                                                                                                                    \
            return PT_YIELDED;                                                                                               \
        }                                                                                                                    \
        else if (PT_WAITING == __PT_STR_CONNECT2(ret, __LINE__))                                                             \
        {                                                                                                                    \
            return PT_WAITING;                                                                                               \
        }                                                                                                                    \
        if (container_of(pt, pt_thread_info, pt)->status.super)                                                              \
        {                                                                                                                    \
            slist_del(&(container_of(child, pt_thread_info, pt)->list), &(container_of(pt, pt_thread_info, pt)->sub_head));  \
        }                                                                                                                    \
        else                                                                                                                 \
        {                                                                                                                    \
            slist_del(&(container_of(child, pt_thread_info, pt)->list), &(container_of(pt, pt_thread_info, pt)->list));      \
        }                                                                                                                    \
    }                                                                                                                        \
    while (0)


#define PT_WAIT_UNTIL_EX(pt, condition, ms) __PT_TIME_ENABLE(pt, condition, {}, ms)


typedef struct
{
    uint32_t idle          : 2;
    uint32_t time_out      : 2;
    uint32_t yield_cnt     : 8;
    uint32_t yield_run_cnt : 8;
    uint32_t super         : 2;
    uint32_t no_use        : 10;
} pt_thread_status;


typedef struct
{
    slist_t list;
    struct pt pt;
    slist_t sub_head; // 子线程的
    pt_thread_t pt_thread;
    pt_thread_status status;
    pt_timer_t timer;
} pt_thread_info;

void pt_thread_register(pt_thread_info *pti, pt_thread_t ptt);
void pt_thread_unregister(pt_thread_info *pti);
void pt_thread_register_runing(pt_thread_info *pti, pt_thread_t ptt);
int pt_thread_is_register(pt_thread_info *pti);
void pt_threads_init(void);
int pt_threads_run(void);
uint32_t pt_threads_get_block_time_ms(void);
void pt_thread_reset(pt_thread_info *pti);
uint32_t pt_thread_get_block_time_ms(pt_thread_info *pti);
#endif // __PT_THREAD_H
