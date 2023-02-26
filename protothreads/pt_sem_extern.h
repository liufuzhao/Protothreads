#ifndef __PT_SEM_EXTERN_H__
#define __PT_SEM_EXTERN_H__
#include "pt_port.h"
#include "pt.h"

#define PT_SEM_WAIT(pt, s, ms) \
    __PT_TIME_ENABLE(                  \
        pt, (s)->count > 0,            \
        {                              \
            if (!PT_IS_TIMEOUT(pt))   \
            {                          \
                --(s)->count;          \
            }                          \
        },                             \
        (ms))

#define PT_SEM_SIGNAL_ONCE(s)    \
    do                           \
    {                            \
        (s)->count = 1;          \
        WAKE_UP_PT_THREAD_SEM(); \
    }                            \
    while (0)

#define PT_SEM_SIGNAL_ADD(s)     \
    do                           \
    {                            \
        (s)->count++;            \
        WAKE_UP_PT_THREAD_SEM(); \
    }                            \
    while (0)

#endif
