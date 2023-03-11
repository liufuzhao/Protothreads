
#ifndef __PT_MONITOR_H
#define __PT_MONITOR_H

#if PT_MONITOR_FUNC_ENABLE

#include "pt_port.h"
#include "pt.h"

#define PT_BEGIN_EX(pt)                                                           \
    container_of(pt, pt_thread_info, pt)->pt_monitor = (const uint8_t *)__func__; \
    container_of(pt, pt_thread_info, pt)->status.begin = PT_FALSE;                \
    container_of(pt, pt_thread_info, pt)->status.monitor_view = PT_FALSE;         \
    PT_BEGIN(pt);                                                                 \
    container_of(pt, pt_thread_info, pt)->status.begin = PT_TRUE;                 \
    container_of(pt, pt_thread_info, pt)->status.monitor_view = PT_TRUE;


#define PT_END_EX(pt) PT_END(pt)

#define PT_EXIT_EX(pt) PT_EXIT(pt)

void pt_monitor(struct pt *pt);
void pt_monitor_enable(struct pt *pt, uint8_t flg);
void pt_monitor_review(struct pt *pt);
#endif
#endif
