#ifndef __PT_QUEUE_EXTERN_H
#define __PT_QUEUE_EXTERN_H
#include "pt.h"
#include "pt_port.h"
#include "pt_timers.h"


typedef struct
{
    char *addr;                  // 基本指针
    volatile uint32_t read;      // 读
    volatile uint32_t write;     // 写
    volatile uint32_t itemSize;  // 每一个成员的大小
    volatile uint32_t queueSize; // 总队列大小
} pt_queue_t;

#define PT_QUEUE_INIT(q, p, qS, iS)          \
    do                                       \
    {                                        \
        (q)->addr = (char *)(p);             \
        (q)->read = 0;                       \
        (q)->write = 0;                      \
        (q)->itemSize = (iS);                \
        (q)->queueSize = (qS) / (iS) * (iS); \
    }                                        \
    while (0)

#define _IS_WRITEABLE(q) ((((q)->write + (q)->itemSize) % (q)->queueSize) != (q)->read)

#define _IS_READABLE(q) ((q)->write != (q)->read)

#define PT_QUEUE_WRITE(q, item)                                    \
    do                                                             \
    {                                                              \
        if (_IS_WRITEABLE(q))                                      \
        {                                                          \
            memcpy(&(q)->addr[(q)->write], (item), (q)->itemSize); \
            (q)->write += (q)->itemSize;                           \
            if ((q)->write >= (q)->queueSize)                      \
            {                                                      \
                (q)->write = 0;                                    \
            }                                                      \
            WAKE_UP_PT_THREAD_SEM();                               \
        }                                                          \
    }                                                              \
    while (0)

#define PT_QUEUE_READ(q, item)                                    \
    do                                                            \
    {                                                             \
        if (_IS_READABLE(q))                                      \
        {                                                         \
            memcpy((item), &(q)->addr[(q)->read], (q)->itemSize); \
            (q)->read += (q)->itemSize;                           \
            if ((q)->read >= (q)->queueSize)                      \
            {                                                     \
                (q)->read = 0;                                    \
            }                                                     \
        }                                                         \
    }                                                             \
    while (0)

#define PT_QUEUE_HAS_DATA(q) _IS_READABLE(q)

#define PT_QUEUE_WRITE_BLOCK(pt, q, item, ms) __PT_TIME_ENABLE(pt, _IS_WRITEABLE(q), PT_QUEUE_WRITE(q, item), (ms))

#define PT_QUEUE_READ_BLOCK(pt, q, item, ms) __PT_TIME_ENABLE(pt, _IS_READABLE(q), PT_QUEUE_READ(q, item), (ms))

#endif
