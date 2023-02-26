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


#define PT_EVNET_INIT(s, c) (s)->mark = c

#define PT_EVNET_SET_BIT(s, e)   \
    do                           \
    {                            \
        (s)->mark |= (e);        \
        WAKE_UP_PT_THREAD_SEM(); \
    }                            \
    while (0)


#define PT_EVNET_CLEAR_BIT(s, e) (s)->mark &= (~e)

#define PT_EVNET_GET_MARK(s) ((s)->mark)

#define PT_EVNET_WAIT(pt, s, ms) __PT_TIME_ENABLE(pt, (s)->mark, {}, ms)

#endif
