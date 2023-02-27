#ifndef __PT_SEM_EXTERN_H__
#define __PT_SEM_EXTERN_H__

// 整形信号量
#include "pt_port.h"
#include "pt.h"
#include "pt_timers.h"

typedef struct
{
    uint8_t bin   : 1;
    uint8_t count : 7;
} pt_sem_t;

#define PT_SEM_INIT(s) pt_sem_init(s, bin)
#define PT_SEM_POST(s) pt_sem_post(s)

#define PT_SEM_WAIT_BLOCK(pt, s, ms) \
    _PT_TIME_ENABLE(                \
        pt, (s)->count > 0,          \
        {                            \
            if (!PT_IS_TIMEOUT(pt))  \
            {                        \
                --(s)->count;        \
            }                        \
        },                           \
        (ms))

#define PT_SEM_WAIT_YEILD(pt, s, cnt) \
    _PT_YIELD_ENABLE(                \
        pt, (s)->count > 0,           \
        {                             \
            if (!PT_IS_TIMEOUT(pt))   \
            {                         \
                --(s)->count;         \
            }                         \
        },                            \
        (cnt))

// 非协程可用
void pt_sem_init(pt_sem_t *sem, uint8_t bin);
void pt_sem_post(pt_sem_t *sem);

#endif
