#include "pt_port.h"
#include "pt_threads.h"
#include "pt_box_extern.h"
#include "pt_broadcast_box_test.h"

#include "hal.h"

// 广播邮箱的使用方式
// 支持多个线程往里send 和receive
#define serial_send_hal(data, len) serial_send(data, len)
#define serial_recevie_hal(data)   serial_receive_char(data)

// 广播邮箱
static pt_box_t s_broadcast_box;

static pt_serial_receive_cb s_receive_cb = NULL;

static receive_frame_t *parse_frame(uint8_t data, uint8_t reset)
{
    static receive_frame_t frame;
    static uint8_t rx_len;
    static uint8_t pro_step = 0;
    if (reset)
    {
        memset((uint8_t *)&frame, 0, sizeof(receive_frame_t));
        rx_len = 0;
        pro_step = 0;
        return NULL;
    }

    switch (pro_step)
    {
        case 0:
            if (data == 0X5a)
            {
                pro_step++;
                frame.receive.head.head = 0X5a;
            }
            else
            {
                memset((uint8_t *)&frame, 0, sizeof(receive_frame_t));
                rx_len = 0;
                pro_step = 0;
            }
            break;
        case 1:
            pro_step++;
            frame.receive.head.cmd = data;
            break;
        case 2:
            pro_step++;
            frame.receive.head.sum = data;
            break;
        case 3:
            pro_step++;
            frame.receive.head.len = data;
            break;
        case 4:
            frame.receive.head.data[rx_len] = data;
            rx_len++;
            if (rx_len == frame.receive.head.len)
            {
                pro_step = 0;
                rx_len = 0;
                return &frame;
            }
            break;
    }
    return NULL;
}


static PT_THREAD(_serial_rx_thread(struct pt *pt))
{
    static receive_frame_t *frame;
    static uint8_t data;
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_WAIT_UNTIL_EX(pt, serial_receive_char(&data), PT_WAIT_FOREVER);
        frame = parse_frame(data, PT_FALSE);
        if (frame)
        {
            PT_BOX_SEND_YIELD(pt, &s_broadcast_box, (void *)frame->receive.head.cmd, frame, 100);
            if (PT_IS_TIMEOUT(pt))
            {
                LOG("box is not accept by other \r\n");
            }
            else
            {
                LOG(" box is accept by other \r\n");
            }

            if (frame->dispose != PT_TRUE && s_receive_cb)
            {
                LOG(" send to cb funciton \r\n");
                s_receive_cb(frame);
            }
            frame = parse_frame(data, PT_TRUE);
        }
    }
    PT_END_EX(pt);
}

PT_THREAD(pt_serial_send_thread(struct pt *pt, send_frame_t *frame, pt_box_filter_info_t *filter))
{

    PT_BEGIN_EX(pt);

    if (frame == NULL || filter == NULL || filter->cb == NULL || filter->para == NULL)
    {
        PT_EXIT_EX(pt);
    }

    for (frame->resend_cnt; frame->resend_cnt > 0; frame->resend_cnt--)
    {
        serial_send_hal(frame->send.data, frame->send.head.len + 4);
        PT_BOX_ACCEPT_BLOCK(pt, &s_broadcast_box, filter->cb, filter->para, frame->resend_time_100ms * 100);
        if (PT_IS_TIMEOUT(pt))
        {
            LOG(" resend \r\n");
        }
        else
        {
            pt_box_get_msg(&s_broadcast_box, (void **)&frame->receive);
            frame->receive->dispose = PT_TRUE;
            LOG("receive ack \r\n");
            break;
        }
    }

    if (frame->resend_cnt == 0)
    {
        PT_EXIT_EX(pt);
    }

    PT_END_EX(pt);
}


int pt_serial_driver_init(pt_serial_receive_cb cb)
{
    static pt_thread_info s_serial_rx_pt;
    pt_box_init(&s_broadcast_box);

    if (pt_thread_is_register(&s_serial_rx_pt))
    {
        pt_thread_unregister(&s_serial_rx_pt);
    }
    pt_thread_reset(&s_serial_rx_pt);
    pt_thread_register(&s_serial_rx_pt, _serial_rx_thread, "_s_serial_rx_pt");

    s_receive_cb = cb;
}