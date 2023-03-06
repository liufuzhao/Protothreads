#include "pt_port.h"
#include "pt_threads.h"
#include "pt_list.h"

#define PT_VERSION "0.2.00"
AOS_SLIST_HEAD(threads_head);

static uint32_t __pt_thread_get_block_time_ms(pt_thread_info *pti)
{
    pt_timer_t *t = &pti->timer;
    uint32_t tmp = PT_GET_TICK();
    if (t->status == PT_TIME_RUN_STATE || t->status == PT_TIME_PRE_RUN_STATE)
    {
        tmp = tmp - t->start_tick;
        if (tmp <= t->timeout_tick)
        {
            return t->timeout_tick - tmp;
        }
        else
        {
            return 0;
        }
    }
    return PT_WAIT_FOREVER;
}


uint32_t pt_thread_get_block_time_ms(pt_thread_info *pti)
{
    pt_thread_info *node = NULL;
    slist_t *tmp_node = NULL;
    uint32_t min_times = __pt_thread_get_block_time_ms(pti);
    uint32_t tmp_times = 0;

    slist_for_each_entry_safe(&pti->sub_head, tmp_node, node, pt_thread_info, list)
    {
        if (node)
        {
            pt_timer_t *t = &node->timer;
            tmp_times = PT_GET_TICK();
            if (t->status == PT_TIME_RUN_STATE || t->status == PT_TIME_PRE_RUN_STATE)
            {
                tmp_times = tmp_times - t->start_tick;
                if (tmp_times <= t->timeout_tick)
                {
                    tmp_times = t->timeout_tick - tmp_times;
                }
                else
                {
                    tmp_times = 0;
                }
            }
            else
            {
                tmp_times = PT_WAIT_FOREVER;
            }

            if (min_times > tmp_times)
            {
                min_times = tmp_times;
            }
        }
    }
    return min_times;
}


void pt_threads_pre_run(pt_thread_info *pti)
{
#if PT_BOX_FUNC_ENABLE
    extern void pt_boxs_auto_handled(struct pt * pt);
    pt_thread_info *node = NULL;
    slist_t *tmp_node = NULL;
    slist_for_each_entry_safe(&pti->sub_head, tmp_node, node, pt_thread_info, list)
    {
        if (node)
        {
            pt_boxs_auto_handled(&(node->pt));
        }
    }
    return;
#endif
}

void pt_thread_register(pt_thread_info *pti, pt_thread_t ptt, const uint8_t *pt_monitor)
{
    pti->pt_thread = ptt;
    pti->status.super = PT_TRUE;
#if PT_MONITOR_FUNC_ENABLE
    pti->pt_monitor = pt_monitor;
#endif
    PT_INIT(&pti->pt);
    slist_add(&pti->list, &threads_head);
}

void pt_thread_register_runing(pt_thread_info *pti, pt_thread_t ptt)
{
    pti->pt_thread = ptt;
    slist_add(&pti->list, &threads_head);
}

void pt_thread_unregister(pt_thread_info *pti)
{
    slist_del(&pti->list, &threads_head);
    pt_thread_reset(pti);
}

int pt_thread_is_register(pt_thread_info *pti)
{
    pt_thread_info *node = NULL;
    slist_t *tmp = NULL;
    slist_for_each_entry_safe(&threads_head, tmp, node, pt_thread_info, list)
    {
        if (node == pti)
        {
            return PT_TRUE;
        }
    }
    return PT_FALSE;
}

void pt_threads_init(void)
{
    slist_init(&threads_head);
    pt_timers_server_init();
    PT_LOG("Welcome to Protothreads \r\n");
    PT_LOG("Built:\"%s\" ,Version :\"%s\" \r\n", __TIME__, PT_VERSION);
}

void pt_thread_reset(pt_thread_info *pti)
{
    memset(&pti->pt, 0, sizeof(pti->pt));
    memset(&pti->timer, 0, sizeof(pti->timer));
    memset(&pti->status, 0, sizeof(pti->status));
    slist_init(&pti->sub_head);
}

uint32_t pt_threads_get_block_time_ms(void)
{
    pt_thread_info *node = NULL;
    slist_t *tmp = NULL;
    uint32_t min_times = PT_WAIT_FOREVER;
    uint32_t tmp_times = 0;
    slist_for_each_entry_safe(&threads_head, tmp, node, pt_thread_info, list)
    {
        if (node)
        {
            if (node->status.idle == PT_TRUE)
            {
                tmp_times = pt_thread_get_block_time_ms(node);
                if (min_times > tmp_times)
                {
                    min_times = tmp_times;
                }
            }
        }
    }

    tmp_times = timers_server_get_block_time_ms();
    if (min_times > tmp_times)
    {
        min_times = tmp_times;
    }
    return min_times;
}


int pt_threads_run(void)
{
    pt_thread_info *node = NULL;
    slist_t *tmp = NULL;
    int status = 0;
    int run_status = PT_FALSE;
    pt_thread_info *ignore = get_timers_server_thread();
    slist_for_each_entry_safe(&threads_head, tmp, node, pt_thread_info, list)
    {
        if (node && node->pt_thread)
        {
            pt_threads_pre_run(node);
            status = node->pt_thread(&(node->pt));
#if PT_MONITOR_FUNC_ENABLE
            pt_monitor(&(node->pt));
#endif
            if (node != ignore)
            {
                if (status == PT_YIELDED)
                {
                    run_status |= PT_TRUE;
                    node->status.idle = PT_FALSE;
                }
                else
                {
                    run_status |= PT_FALSE;
                    node->status.idle = PT_TRUE;
                }
            }
        }
    }
    return run_status;
}
