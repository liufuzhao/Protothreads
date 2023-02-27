#include "pt_port.h"
#include "pt_threads.h"
#include "pt_sem_extern.h"
#include "pt_box_extern.h"
#include "pt_event_extern.h"
#include "pt_queue_extern.h"

// pt extern的简单工具的测试使用
// 主要是sem，event, queue 以及如何通过二元信号量sem 传递消息
// box 的简单使用
// 软件定时器如何使用

#define EVENT_INIT_MARK   (1 << 0)
#define EVENT_PRINTF_MARK (1 << 1)

static pt_sem_t s_sem;
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

static pt_thread_info s_sem_post_thread_info;
static pt_thread_info s_sem_wait_thread_info;

static pt_thread_info s_simple_thread_info;

static int box_filter_cb(pt_box_t *box, void *para)
{
    void *tmp = box->p_addr;
    if (tmp == (void *)2 || tmp == (void *)5)
    {
        return PT_TRUE;
    }
    else
    {
        return PT_FALSE;
    }
}
// 有点像一个item的队列
static PT_THREAD(box_dispose_thread(struct pt *pt))
{
    static char cnt = 0;
    void *target = NULL;
    void *msg;
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_BOX_ACCEPT_BLOCK(pt, &s_box, box_filter_cb, NULL, PT_WAIT_FOREVER);
        if (PT_IS_TIMEOUT(pt))
        {
            LOG(" s_box time out \r\n");
        }
        else
        {
            pt_box_get_msg(&s_box, &msg);
            pt_box_get_target(&s_box, &target);
            LOG(" s_box receive condition,dispose it %d \r\n", (uint32_t)target);
        }
    }
    PT_END_EX(pt);
}

// 有点像一个item的队列
static PT_THREAD(box_general_thread(struct pt *pt))
{
    static char box_target[256];
    static char cnt = 0;
    void *target = NULL;
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_SLEEP_MS(pt, 100);
        snprintf(box_target, sizeof(box_target), "box_general_thread post box msg %d", cnt++);
        // 发送信息有一个,让出cpu 100
        target = (void *)cnt;
        PT_BOX_SEND_YIELD(pt, &s_box, target, box_target, 100);
        if (PT_IS_TIMEOUT(pt))
        {
            LOG("box is not disposed by other \r\n");
        }
        else
        {
            LOG(" box is disposed by other \r\n");
        }
    }
    PT_END_EX(pt);
}

// select的处理机制
static PT_THREAD(event_dispose_thread(struct pt *pt))
{
    static void *msg = NULL;
    PT_BEGIN_EX(pt);
    while (1)
    {
        // 永久等待
        PT_EVNET_WAIT_BLOCK(pt, &s_event, PT_WAIT_FOREVER);
        if (PT_EVNET_CLEAR_BIT(&s_event, EVENT_INIT_MARK))
        {
            LOG(" event init trigger\r\n");
        }
        else if (PT_EVNET_CLEAR_BIT(&s_event, EVENT_PRINTF_MARK))
        {
            LOG(" event printf mark \r\n");
        }
        else
        {
            LOG("error \r\n");
            PT_EVNET_INIT(&s_event);
        }
    }
    PT_END_EX(pt);
}

// 事件生成机制
static PT_THREAD(event_general_thread(struct pt *pt))
{
    static char cnt = 0;
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_SLEEP_MS(pt, 500);
        PT_EVNET_SET_BIT(&s_event, EVENT_INIT_MARK);
        PT_SLEEP_MS(pt, 1000);
        PT_EVNET_SET_BIT(&s_event, EVENT_PRINTF_MARK);
    }
    PT_END_EX(pt);
}


// 往队列里发送数据
static PT_THREAD(queue_write_thread(struct pt *pt))
{
    static char frame[128] = {0};
    static char cnt = 0;
    PT_BEGIN_EX(pt);
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
    PT_END_EX(pt);
}

// 有消息就读
static PT_THREAD(queue_read_thread(struct pt *pt))
{
    static char frame[128] = {0};
    static char cnt = 0;
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_QUEUE_READ_BLOCK(pt, &s_queue, frame, PT_WAIT_FOREVER);
        LOG("read queue\"%s\"\r\n", frame);
        PT_SLEEP_MS(pt, 1000);
    }
    PT_END_EX(pt);
}


// 最简单的simple thread sleep
static PT_THREAD(simple_thread(struct pt *pt))
{
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_SLEEP_MS(pt, 1000);
        LOG("each 1000ms wake up \r\n");
    }
    PT_END_EX(pt);
}


static PT_THREAD(sem_post_thread(struct pt *pt))
{
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_SLEEP_MS(pt, 1500);
        PT_SEM_POST(&s_sem);
    }
    PT_END_EX(pt);
}


static PT_THREAD(sem_wait_thread(struct pt *pt))
{
    PT_BEGIN_EX(pt);
    while (1)
    {
        PT_SEM_WAIT_BLOCK(pt, &s_sem, 1000);
        if (PT_IS_TIMEOUT(pt))
        {
            LOG("sem_wait_thread time out \r\n");
        }
        else
        {
            LOG("sem_wait_thread success \r\n");
        }
    }
    PT_END_EX(pt);
}


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
    // pt_queue_init(&s_queue, &s_queue_entry, 128 * 4, 128);
    // pt_thread_register(&s_queue_read_thread_info, queue_read_thread,"queue_read_thread");
    // pt_thread_register(&s_queue_write_thread_info, queue_write_thread,"queue_write_thread");

    // 最简单的sleep的使用  32字节损耗
    // pt_thread_register(&s_simple_thread_info, simple_thread, "simple_thread");

    // 最省资源的timer使用  24字节损耗 (定时轮询机制)
    // pt_timer_register(&s_timer_info, simple_soft_timer_callback);
    // pt_timer_start(&s_timer_info.time, 500);

    // event的使用
    // pt_event_init(&s_event);
    // pt_thread_register(&s_event_general_thread_info, event_general_thread,"event_general_thread");
    // pt_thread_register(&s_event_dispose_thread_info, event_dispose_thread,"event_dispose_thread");

    // box的使用
    // pt_box_init(&s_box);
    // pt_thread_register(&s_box_general_thread_info, box_general_thread, "box_general_thread");
    // pt_thread_register(&s_box_dispose_thread_info, box_dispose_thread, "box_dispose_thread");

    // sem的使用
    pt_sem_init(&s_sem, PT_TRUE);
    pt_thread_register(&s_sem_post_thread_info, sem_post_thread, "sem_post_thread");
    pt_thread_register(&s_sem_wait_thread_info, sem_wait_thread, "sem_wait_thread");
}
