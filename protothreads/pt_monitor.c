#include "pt_port.h"
#include "pt_threads.h"
#include "pt_monitor.h"


void _pt_monitor(pt_thread_info *pti)
{

    uint8_t cnt = 0;
    pt_thread_info *node = NULL;
    slist_t *tmp_node = NULL;

    PT_LOG_RAW("threads func: %s line: %d \r\n", pti->pt_monitor, pti->pt.lc);
    slist_for_each_entry_safe(&pti->sub_head, tmp_node, node, pt_thread_info, list)
    {
        cnt++;
        for (uint8_t i = 0; i < cnt; i++)
        {
            PT_LOG_RAW("---");
        }
        PT_LOG_RAW("> threads func: %s line: %d\r\n", node->pt_monitor, node->pt.lc);
    }
}


void pt_monitor(struct pt *pt)
{

    pt_thread_info *pti = container_of(pt, pt_thread_info, pt);

    uint8_t view_flg = PT_FALSE;
    pt_thread_info *node = NULL;
    slist_t *tmp_node = NULL;

    if (pti->status.monitor_enbale)
    {
        if (pti->status.monitor_view == PT_TRUE)
        {
            pti->status.monitor_view = PT_FALSE;
            view_flg = PT_TRUE;
        }
    }


    slist_for_each_entry_safe(&pti->sub_head, tmp_node, node, pt_thread_info, list)
    {
        if (node->status.monitor_enbale)
        {
            if (node->status.monitor_view == PT_TRUE)
            {
                node->status.monitor_view = PT_FALSE;
                view_flg = PT_TRUE;
            }
        }
    }


    if (view_flg == PT_TRUE)
    {
        _pt_monitor(pti);
    }
}

void pt_monitor_enable(struct pt *pt, uint8_t flg)
{
    pt_thread_info *pti = container_of(pt, pt_thread_info, pt);
    if (flg == PT_TRUE)
    {
        pti->status.monitor_enbale = PT_TRUE;
    }
    else
    {
        pti->status.monitor_enbale = PT_FALSE;
    }
}

void pt_monitor_review(struct pt *pt)
{
    pt_thread_info *pti = container_of(pt, pt_thread_info, pt);
    _pt_monitor(pti);
}
