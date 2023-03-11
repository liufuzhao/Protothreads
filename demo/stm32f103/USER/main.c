#include "pt_port.h"
#include "pt_threads.h"
#include "pt_monitor.h"

#include "delay.h"
#include "hal.h"
#include "led.h"
#include "pt_broadcast_box_test.h"


static int pt_2_box_filter(pt_box_t *box, void *para)
{
    void *tmp = box->p_addr;
    if (tmp == (void *)para)
    {
        return PT_TRUE;
    }
    else
    {
        return PT_FALSE;
    }
}


PT_THREAD(pt_test_thread_2(struct pt *pt))
{
    static pt_thread_info sub_thread;
    static send_frame_t frame;
    static pt_box_filter_info_t filter;
    uint8_t stat;
    PT_BEGIN_EX(pt);
    pt_thread_reset(&sub_thread);
    filter.cb = pt_2_box_filter;
    frame.send.head.head = 0X5a;
    frame.send.head.cmd = 0xA0;

    while (1)
    {
        frame.send.head.cmd++;
        frame.send.head.len = 10;
        frame.send.head.sum = 0;
        frame.resend_time_100ms = 10 * 10; // 30秒
        frame.resend_cnt = 2;             // 重发3次

        filter.para = (void *)frame.send.head.cmd;
        PT_SPAWN_EX(pt, &sub_thread.pt, stat = pt_serial_send_thread(&sub_thread.pt, &frame, &filter));
        if (stat == PT_ENDED)
        {
            LOG("pt_sleep_thread send ok !!\r\n");
            LOG("recevie msg is :\r\n");
            PT_LOG_HEX((uint8_t *)frame.receive->receive.data, frame.receive->receive.head.len + 4);
        }
        else if (stat == PT_EXITED)
        {
            LOG("pt_sleep_thread send not receive ack !!\r\n");
        }
    }
    PT_END_EX(pt);
}


void pt_protocol_init()
{

    static pt_thread_info test_2;
    pt_serial_driver_init(NULL);

    if (pt_thread_is_register(&test_2))
    {
        pt_thread_unregister(&test_2);
    }
    pt_thread_reset(&test_2);
    pt_thread_register(&test_2, pt_test_thread_2, "pt_test_thread_2");
}


int main(void)
{
    delay_init(); // 延时函数初始化
    serial_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    pt_threads_init();
    pt_protocol_init();
    pt_led_init();
    while (1)
    {
        pt_threads_run();
    }
}
