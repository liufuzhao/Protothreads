#include "pt_port.h"
#include "pt_threads.h"
#include "pt_list.h"

AOS_SLIST_HEAD(s_timers_head);
static pt_thread_info s_timers_server_info;

void _pt_timer_start(pt_timer_t *t, unsigned long timeout)
{
    t->start_tick = PT_GET_TICK();
    t->timeout_tick = timeout;
    if (timeout != PT_WAIT_FOREVER)
    {
        t->status = PT_TIME_PRE_RUN_STATE;
    }
    else
    {
        t->status = PT_TIME_PRE_FOREVER_STATE;
    }
}

int pt_timer_start(pt_timer_t *t, unsigned long ms)
{
    _pt_timer_start(t, ms);
    return 0;
}

int pt_timer_stop(pt_timer_t *t)
{
    t->status = PT_TIME_STOP_STATE;
    return 0;
}

int pt_timer_first_run(pt_timer_t *t)
{
    if (t->status == PT_TIME_PRE_RUN_STATE)
    {
        t->status = PT_TIME_RUN_STATE;
        return PT_TRUE;
    }
    else if (t->status == PT_TIME_PRE_FOREVER_STATE)
    {
        t->status = PT_TIME_FOREVER_STATE;
        return PT_TRUE;
    }
    else
    {
        return PT_FALSE;
    }
}

int pt_timer_timeout(pt_timer_t *t)
{
    // return (uwTick > t->start_tick + t->timeout_tick); // 加法越界时会有bug
    if (t->status == PT_TIME_PRE_RUN_STATE)
    {
        t->status = PT_TIME_RUN_STATE;
        return 0;
    }
    else if (t->status == PT_TIME_PRE_STOP_STATE)
    {
        t->status = PT_TIME_STOP_STATE;
        return 0;
    }
    else if (t->status == PT_TIME_RUN_STATE)
    {
        return (PT_GET_TICK() - t->start_tick > t->timeout_tick); // 减法越界时不会有bug
    }
    else
        return 0;
}

int pt_timer_register(timer_extern_t *t, timers_callback_t cb)
{
    t->prtCb = cb;
    slist_add(&t->list, &s_timers_head);
    return 0;
}

int pr_timer_deregister(timer_extern_t *t)
{
    slist_del(&t->list, &s_timers_head);
    return 0;
}

// 定时器任务线程，每次轮询完所有的callback
PT_THREAD(timers_server_thread(struct pt *pt))
{
    timer_extern_t *node = NULL;
    slist_t *tmp = NULL;
    PT_BEGIN(pt);
    while (1)
    {
        slist_for_each_entry_safe(&s_timers_head, tmp, node, timer_extern_t, list)
        {
            if (node && pt_timer_timeout(&node->time) && node->prtCb)
            {
                node->time.status = PT_TIME_PRE_STOP_STATE;
                node->prtCb(&node->time, node->para);
            }
        }
        PT_YIELD(pt); // 执行完一次timer callback 让出cpu
    }
    PT_END(pt);
}

uint32_t timers_server_get_block_time_ms(void)
{
    timer_extern_t *node = NULL;
    slist_t *tmp = NULL;
    uint32_t min_times = PT_WAIT_FOREVER;
    uint32_t tmp_times = 0;
    uint32_t tmp_tick = PT_GET_TICK();
    slist_for_each_entry_safe(&s_timers_head, tmp, node, timer_extern_t, list)
    {
        if (node)
        {
            if (PT_TIME_PRE_STOP_STATE == node->time.status)
            {
                min_times = 0;
            }
            else if ((PT_TIME_RUN_STATE == node->time.status))
            {
                tmp_times = tmp_tick - node->time.start_tick;
                if (tmp_times <= node->time.timeout_tick)
                {
                    tmp_times = node->time.timeout_tick - tmp_times;
                }
                else
                {
                    tmp_times = 0;
                }

                if (min_times > tmp_times)
                {
                    min_times = tmp_times;
                }
            }
        }
    }
    return min_times;
}

int pt_timers_server_init(void)
{
    slist_init(&s_timers_head);
    pt_thread_register(&s_timers_server_info, timers_server_thread);
    return 0;
}

void *get_timers_server_thread(void)
{
    return &s_timers_server_info;
}
