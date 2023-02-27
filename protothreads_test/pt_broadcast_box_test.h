#ifndef __PT_BROADCAST_MAILBOX_H
#define __PT_BROADCAST_MAILBOX_H

#include "pt_port.h"
#include "pt_threads.h"
#include "pt_box_extern.h"

// 协议是 head 0x5a
#pragma pack(1)
typedef struct
{
    uint8_t head;
    uint8_t cmd;
    uint8_t sum;
    uint8_t len;
    uint8_t data[];
} protocol_t;

typedef union
{
    protocol_t head;
    uint8_t data[255];
} base_frame_t;

typedef struct
{
    base_frame_t receive;
    uint8_t dispose;
} receive_frame_t;

typedef struct
{
    uint8_t resend_cnt;
    uint8_t resend_time_100ms;
    base_frame_t send;
    receive_frame_t *receive;
} send_frame_t;

#pragma pack(0)

typedef int (*pt_serial_receive_cb)(receive_frame_t *frame);

int pt_serial_driver_init(pt_serial_receive_cb cb);
PT_THREAD(pt_serial_send_thread(struct pt *pt, send_frame_t *frame, pt_box_filter_info_t *filter));

#endif