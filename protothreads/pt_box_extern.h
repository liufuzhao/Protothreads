#ifndef __PT_BOX_EXTERN_H__
#define __PT_BOX_EXTERN_H__

#include "pt_port.h"
#include "pt.h"
#include "pt_timers.h"

#define PT_BOX_INVALID 0
#define PT_BOX_VALID   1

typedef struct
{
    volatile unsigned int use;
    void *p_addr;
    void *p_msg;
} pt_box_t;

#define PT_BOX_INIT(b)                             \
    do                                             \
    {                                              \
        (b)->use = PT_BOX_INVALID; \
        (b)->p_addr = NULL; \
        (b)->p_msg = NULL;                         \
    }                                              \
    while (0)

#define PT_BOX_SEND(b,target,msg)                 \
    do                                           \
    {                                            \
        (b)->use = PT_BOX_VALID; \
        (b)->p_addr = target; \
        (b)->p_msg = msg;                        \
        WAKE_UP_PT_THREAD_SEM();                 \
    }                                            \
    while (0)

#define PT_BOX_HANDLED(b)                      \
    do                                             \
    {                                              \
        (b)->use = PT_BOX_INVALID; \
    }                                              \
    while (0)

#define PT_BOX_IS_HANDLER(b) ((b)->use == PT_BOX_INVALID)

#define PT_BOX_SEND_YIELD(pt, b,target, msg, cnt) \
    PT_BOX_SEND(b,target,msg);                      \
    __PT_YIELD_CNT_ENABLE(pt, PT_BOX_IS_HANDLER(b), (cnt))

#define _PT_BOX_WAIT_UNTIL(b, cb) (((b)->use != PT_BOX_INVALID) && cb(b))

#define PT_BOX_WAIT_UNTIL(pt, b, cb, msg, ms) \
    __PT_TIME_ENABLE(                           \
        pt, _PT_BOX_WAIT_UNTIL(b, cb), { msg = (b)->p_msg; }, (ms))

#endif
