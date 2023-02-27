#ifndef __PT_BOX_EXTERN_H__
#define __PT_BOX_EXTERN_H__
// 广播邮箱

#include "pt_port.h"
#include "pt.h"
#include "pt_list.h"
#include "pt_timers.h"

#define PT_BOX_EMPTY_STATE        0
#define PT_BOX_SEND_STATE         1
#define PT_BOX_ACCEPT_STATE       2
#define PT_BOX_AUTO_HANDLED_STATE 3
#define PT_BOX_MAN_HANDLED_STATE  4
#define PT_BOX_SEND_TIMEOUT_STATE 5

typedef struct
{
    slist_t list;
    volatile uint32_t state;
    struct pt *fir_accept_pt;
    void *p_addr;
    void *p_msg;
} pt_box_t;

typedef int (*pt_box_filter_cb)(pt_box_t *box, void *para);

typedef struct
{
    pt_box_filter_cb cb;
    void *para;
} pt_box_filter_info_t;

#define PT_BOX_INIT(b) pt_box_init(b)

#define PT_BOX_GET_TARGET(b, target) pt_box_get_target(b, target)
#define PT_BOX_GET_MSG(b, msg)       pt_box_get_msg(b, msg)
#define PT_BOX_GET_STATE(b, state)   pt_box_get_msg(b, state)
#define PT_BOX_HANDLED(b)            pt_boxs_handled(b)

#define _PT_BOX_IS_EMPTY(b) ((b)->state == PT_BOX_EMPTY_STATE)
#define _PT_BOX_IS_ACCEPT(pt, b, cb, para) \
    ((((b)->state == PT_BOX_SEND_STATE || (b)->state == PT_BOX_ACCEPT_STATE)) && cb(b, para))
#define _PT_BOX_IS_HANDLED(b) ((b)->state == PT_BOX_AUTO_HANDLED_STATE || (b)->state == PT_BOX_MAN_HANDLED_STATE)

#define _PT_BOX_SEND(b, target, msg)    \
    do                                  \
    {                                   \
        (b)->state = PT_BOX_SEND_STATE; \
        (b)->fir_accept_pt = NULL;      \
        (b)->p_addr = target;           \
        (b)->p_msg = msg;               \
        WAKE_UP_PT_THREAD_SEM();        \
    }                                   \
    while (0)

#define PT_BOX_SEND_BLOCK(pt, b, target, msg, ms)       \
    _PT_BOX_SEND(b, target, msg);                       \
    _PT_TIME_ENABLE(                                    \
        pt, _PT_BOX_IS_HANDLED(b),                      \
        {                                               \
            if (PT_IS_TIMEOUT(pt))                      \
                (b)->state = PT_BOX_SEND_TIMEOUT_STATE; \
        },                                              \
        (cnt));

#define PT_BOX_SEND_YIELD(pt, b, target, msg, cnt)      \
    _PT_BOX_SEND(b, target, msg);                       \
    _PT_YIELD_ENABLE(                                   \
        pt, _PT_BOX_IS_HANDLED(b),                      \
        {                                               \
            if (PT_IS_TIMEOUT(pt))                      \
                (b)->state = PT_BOX_SEND_TIMEOUT_STATE; \
        },                                              \
        (cnt));


#define PT_BOX_ACCEPT_BLOCK(pt, b, cb, para, ms)                       \
    _PT_TIME_ENABLE(                                                   \
        pt, _PT_BOX_IS_ACCEPT(pt, b, cb, para),                        \
        {                                                              \
            if (!PT_IS_TIMEOUT(pt) && (b)->state == PT_BOX_SEND_STATE) \
            {                                                          \
                (b)->fir_accept_pt = pt;                               \
                (b)->state = PT_BOX_ACCEPT_STATE;                      \
            }                                                          \
        },                                                             \
        (ms))

#define PT_BOX_ACCEPT_YEILD(pt, b, cb, para, ms)                       \
    _PT_YIELD_ENABLE(                                                  \
        pt, _PT_BOX_IS_ACCEPT(pt, b, cb, para),                        \
        {                                                              \
            if (!PT_IS_TIMEOUT(pt) && (b)->state == PT_BOX_SEND_STATE) \
            {                                                          \
                (b)->fir_accept_pt = pt;                               \
                (b)->state = PT_BOX_ACCEPT_STATE;                      \
            }                                                          \
            (b)->state = PT_BOX_ACCEPT_STATE;                          \
        },                                                             \
        (ms))

// 非协程可用
void pt_box_init(pt_box_t *box);
void pt_box_get_target(pt_box_t *box, void **target);
void pt_box_get_msg(pt_box_t *box, void **msg);
void pt_box_get_state(pt_box_t *box, uint32_t *state);
int pt_boxs_handled(pt_box_t *box);
#endif
