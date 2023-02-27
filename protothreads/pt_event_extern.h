#ifndef __PT_EVNET_EXTERN_H__
#define __PT_EVNET_EXTERN_H__
#include "pt_port.h"
#include "pt.h"
#include "pt_timers.h"

#define PT_EVENT_NULL 0

typedef struct
{
    volatile uint32_t mark;
} pt_event_t;


#define PT_EVNET_INIT(e)             pt_event_init(e)
#define PT_EVNET_SET_BIT(e, bit)     pt_event_set_bit(e, bit)
#define PT_EVNET_CLEAR_BIT(e, bit)   pt_event_clear_bit(e, bit)

#define PT_EVNET_WAIT_BLOCK(pt, e, ms)  _PT_TIME_ENABLE(pt, (e)->mark, {}, ms)
#define PT_EVNET_WAIT_YEILD(pt, e, cnt) _PT_YIELD_ENABLE(pt, (e)->mark, {}, cnt)

// 非协程可用
void pt_event_init(pt_event_t *event);
void pt_event_set_bit(pt_event_t *event, uint32_t bit_mark);
int pt_event_clear_bit(pt_event_t *event, uint32_t bit_mark);

#endif
