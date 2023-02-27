#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include "pt_threads.h"
#include "pt_simple_test.h"
#include "pt_broadcast_box_test.h"

volatile uint32_t gu32_system_count;
static pthread_t s_tick_task;
static void tick_task(void *arg)
{
    while (1)
    {
        usleep(1000 * 5);
        gu32_system_count += 5;
    }
}


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

PT_THREAD(pt_test_thread_1(struct pt *pt))
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
        frame.resend_time_100ms = 30; // 3秒
        frame.resend_cnt = 3;         // 重发3次

        filter.para = frame.send.head.cmd;
        PT_SPAWN_EX(pt, &sub_thread.pt, stat = pt_serial_send_thread(&sub_thread.pt, &frame, &filter));
        if (stat == PT_ENDED)
        {
            LOG("pt_test_thread_1 send ok !!\r\n");
            LOG("recevie msg is :\r\n");
            PT_LOG_HEX((uint8_t *)frame.receive->receive.data, frame.receive->receive.head.len + 4);
        }
    }
    PT_END_EX(pt);
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
    frame.send.head.cmd = 0x01;
    uart_send("hello", strlen("hello") + 1);
    while (1)
    {
        frame.send.head.cmd++;
        frame.send.head.len = 10;
        frame.send.head.sum = 0;
        frame.resend_time_100ms = 30; // 3秒
        frame.resend_cnt = 3;         // 重发3次

        filter.para = frame.send.head.cmd;
        PT_SPAWN_EX(pt, &sub_thread.pt, stat = pt_serial_send_thread(&sub_thread.pt, &frame, &filter));
        if (stat == PT_ENDED)
        {
            LOG("pt_test_thread_2 send ok !!\r\n");
            LOG("recevie msg is :\r\n");
            PT_LOG_HEX((uint8_t *)frame.receive->receive.data, frame.receive->receive.head.len + 4);
        }
    }
    PT_END_EX(pt);
}

void test_broadcast_box_init()
{
    static pt_thread_info test_1;
    static pt_thread_info test_2;
    pt_serial_driver_init(NULL);
    if (pt_thread_is_register(&test_1))
    {
        pt_thread_unregister(&test_1);
    }
    pt_thread_reset(&test_1);
    pt_thread_register(&test_1, pt_test_thread_1, "pt_test_thread_1");

    if (pt_thread_is_register(&test_2))
    {
        pt_thread_unregister(&test_2);
    }
    pt_thread_reset(&test_2);
    pt_thread_register(&test_2, pt_test_thread_2, "pt_test_thread_2");
}

void main(int argc, char *argv[])
{
    // new a system tick thread in ubuntu
    uart_init();
    pthread_create(&s_tick_task, NULL, (void *)tick_task, NULL);
    pthread_detach(s_tick_task);

    pt_threads_init();
    test_broadcast_box_init();
    // register_test_threads();
    while (1)
    {
        pt_threads_run();
    }
}
