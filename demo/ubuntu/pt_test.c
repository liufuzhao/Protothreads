#include "pt_port.h"
#include "pt_threads.h"
#include "pt_sem_extern.h"
#include "pt_box_extern.h"
#include "pt_event_extern.h"
#include "pt_queue_extern.h"

#define LOG(...) printf(__VA_ARGS__)

#define EVENT_INIT_MARK   (1 << 0)
#define EVENT_PRINTF_MARK (1 << 1)

static pt_box_t s_box;
static pt_event_t s_event;
static pt_queue_t s_queue;
static timer_extern_t s_timer_info;

static pt_thread_info s_event_dispose_thread_info;
static pt_thread_info s_event_general_thread_info;

static pt_thread_info s_box_general_thread_info;
static pt_thread_info s_box_dispose_thread_info;

static pt_thread_info s_queue_read_thread_info;
static pt_thread_info s_queue_write_thread_info;

static pt_thread_info s_simple_thread_info;

bool box_filter_cb(pt_box_t *box)
{
    uint32_t tmp = (int)box;
    if (tmp == 1000)
    {
        return PT_TRUE;
    }
    else
    {
        // LOG("prt_condition_struct %d \r\n", prt_condition_struct);
        return PT_FALSE;
    }
}
// 有点像一个item的队列
static PT_THREAD(box_dispose_thread(struct pt *pt))
{
    static void *msg;
    static char cnt = 0;
    PT_BEGIN(pt);
    while (1)
    {
        PT_BOX_WAIT_UNTIL(pt, &s_box, box_filter_cb, msg, PT_WAIT_FOREVER);
        if (PT_IS_TIMEOUT(pt))
        {
            LOG(" s_box time out \r\n");
        }
        else
        {
            LOG(" s_box receive condition,dispose it \r\n");
            PT_BOX_HANDLED(&s_box); // 属于自己的必须处理掉！！！
        }
    }
    PT_END(pt);
}

// 有点像一个item的队列
static PT_THREAD(box_general_thread(struct pt *pt))
{
    static char box_target[256];
    static char cnt = 0;
    PT_BEGIN(pt);
    while (1)
    {
        PT_SLEEP_MS(pt, 1000);
        snprintf(box_target, sizeof(box_target), "box_general_thread post box msg %d", cnt++);
        // 发送信息有一个,让出cpu 100
        PT_BOX_SEND_YIELD(pt, &s_box, (void *)box_target, box_target, 100);
        if (PT_BOX_IS_HANDLER(&s_box))
        {
            LOG(" box is dispose by other \r\n");
        }
        else if (PT_IS_TIMEOUT(pt))
        {
            LOG(" send box is error,because is full \r\n");
            PT_BOX_HANDLED(&s_box);
        }
        else
        {
            LOG(" send box succes ,but not dipose by other \r\n");
        }
    }
    PT_END(pt);
}

// select的处理机制
static PT_THREAD(event_dispose_thread(struct pt *pt))
{
    static void *msg = NULL;
    PT_BEGIN(pt);
    while (1)
    {
        // 永久等待
        PT_EVNET_WAIT(pt, &s_event, PT_WAIT_FOREVER);
        if (PT_EVNET_GET_MARK(&s_event) & EVENT_INIT_MARK)
        {

            LOG(" event init trigger\r\n");
            PT_EVNET_CLEAR_BIT(&s_event, EVENT_INIT_MARK);
        }
        else if (PT_EVNET_GET_MARK(&s_event) & EVENT_PRINTF_MARK)
        {
            LOG(" event printf mark \r\n");
            PT_EVNET_CLEAR_BIT(&s_event, EVENT_PRINTF_MARK);
        }
        else
        {
            LOG("error \r\n");
            PT_EVNET_INIT(&s_event, PT_EVENT_NULL);
        }
    }
    PT_END(pt);
}

// 事件生成机制
static PT_THREAD(event_general_thread(struct pt *pt))
{
    static char cnt = 0;
    PT_BEGIN(pt);
    while (1)
    {
        PT_SLEEP_MS(pt, 500);
        PT_EVNET_SET_BIT(&s_event, EVENT_INIT_MARK);
        PT_SLEEP_MS(pt, 1000);
        PT_EVNET_SET_BIT(&s_event, EVENT_PRINTF_MARK);
    }
    PT_END(pt);
}


// 往队列里发送数据
static PT_THREAD(queue_write_thread(struct pt *pt))
{
    static char frame[128] = {0};
    static char cnt = 0;
    PT_BEGIN(pt);
    while (1)
    {
        cnt++;
        snprintf(frame, sizeof(frame), "queue_write_thread  %d  msg", cnt);
        PT_QUEUE_WRITE_BLOCK(pt, &s_queue, frame, 6000);
        if (PT_IS_TIMEOUT(pt))
        {
            LOG("write error !! \"%s\"\r\n", frame);
        }
        else
        {}
    }
    PT_END(pt);
}

// 有消息就读
static PT_THREAD(queue_read_thread(struct pt *pt))
{
    static char frame[128] = {0};
    static char cnt = 0;
    PT_BEGIN(pt);
    while (1)
    {
        PT_QUEUE_READ_BLOCK(pt, &s_queue, frame, PT_WAIT_FOREVER);
        LOG("read queue\"%s\"\r\n", frame);
        PT_SLEEP_MS(pt, 1000);
    }
    PT_END(pt);
}

// 最简单的simple thread sleep
static PT_THREAD(simple_thread(struct pt *pt))
{
    PT_BEGIN(pt);
    while (1)
    {
        PT_SLEEP_MS(pt, 1000);
        LOG("each 1000ms wake up \r\n");
    }
    PT_END(pt);
}

// // 最简单的simple thread sleep
// static PT_THREAD(source_thread(struct pt *pt))
// {
//     static uint8_t s_cond = PT_FALSE;
//     LOG("每次都进入 \r\n");
//     PT_BEGIN(pt);
//     LOG("线程的开始了 ,while 1 在下面 \r\n");
//     while (1)
//     {
//         LOG("PT_YIELD 的前面 \r\n");
//         PT_YIELD(pt);
//         LOG("PT_YIELD 的后面 \r\n");

//         LOG("PT_YIELD_UNTIL 的前面：等待条件变真 \r\n");
//         PT_YIELD_UNTIL(pt, s_cond);

//         LOG("PT_YIELD_UNTIL 的后面：条件变真后进入 \r\n");
//     }

//     LOG("线程的结束 ,由于while 1 在上面 \r\n");
//     PT_END(pt);
// }

// // 最简单的simple thread sleep
// static PT_THREAD(demo_thread(struct pt *pt))
// {
//     static uint8_t s_cond = PT_FALSE;

//     PT_BEGIN(pt);

//     LOG("线程的开始了 ,while 1 在下面 demo thread \r\n");
//     while (1)
//     {
//         LOG("PT_YIELD 的前面 \r\n");
//         PT_YIELD(pt);
//         LOG("PT_YIELD 的后面 \r\n");

//         LOG("PT_YIELD_UNTIL 的前面：等待条件变真 \r\n");
//         PT_YIELD_UNTIL(pt, s_cond);

//         LOG("PT_YIELD_UNTIL 的后面：条件变真后进入 \r\n");
//     }

//     LOG("线程的结束 ,由于while 1 在上面 \r\n");
//     PT_END(pt);
// }


// static PT_THREAD(Protothread(struct pt *pt))
// {
//     static uint8_t s_case ;
//     static uint8_t s_cond = PT_FALSE;
//     PT_BEGIN(pt);
//     LOG("while的上面 \r\n");
//     while (1)
//     {
//         switch (s_case)
//         {
//             case 0:
//                 PT_YIELD_UNTIL(pt, s_cond);
//                 break;
//             case 1:
//                 break;
//         }
//     }
//     LOG("while的下面 \r\n");
//     PT_END(pt);
// }


// 最简单的软件定时器
static void simple_soft_timer_callback(pt_timer_t *time, void *para)
{
    LOG("simple_soft_timer_callback callback \r\n");
    pt_timer_start(time, 500);
}

void register_test_threads(void)
{
    // 本个队列，有一个item被浪费掉，还没进行优化
    // static char s_queue_entry[128 * 4];
    // PT_QUEUE_INIT(&s_queue, &s_queue_entry, 512, 128);
    // pt_thread_register(&s_queue_read_thread_info, queue_read_thread);
    // pt_thread_register(&s_queue_write_thread_info, queue_write_thread);

    // 最简单的sleep的使用  32字节损耗
    pt_thread_register(&s_simple_thread_info, simple_thread);

    // 最省资源的timer使用  24字节损耗 (定时轮询机制)
    // pt_timer_register(&s_timer_info, simple_soft_timer_callback);
    // pt_timer_start(&s_timer_info.time, 500);

    // // event的使用
    // PT_EVNET_INIT(&s_event, PT_EVENT_NULL);
    // pt_thread_register(&s_event_general_thread_info, event_general_thread);
    // pt_thread_register(&s_event_dispose_thread_info, event_dispose_thread);

    // // box的使用
    // PT_BOX_INIT(&s_box);
    // pt_thread_register(&s_box_general_thread_info, box_general_thread);
    // pt_thread_register(&s_box_dispose_thread_info, box_dispose_thread);
}