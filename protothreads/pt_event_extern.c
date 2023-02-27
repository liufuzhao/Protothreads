#include "pt_port.h"
#include "pt_event_extern.h"


void pt_event_init(pt_event_t *event)
{
    event->mark = PT_EVENT_NULL;
}

void pt_event_set_bit(pt_event_t *event, uint32_t bit_mark)
{
    event->mark |= (bit_mark);
    WAKE_UP_PT_THREAD_SEM();
}

int pt_event_clear_bit(pt_event_t *event, uint32_t bit_mark)
{
    if (bit_mark == (event->mark & bit_mark))
    {
        event->mark &= (~bit_mark);
        return PT_TRUE;
    }
    else
    {
        return PT_FALSE;
    }
}
