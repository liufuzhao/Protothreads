#include "pt_port.h"
#include "pt_threads.h"

// gcc -E 可以用来查看PT宏展开之后的代码
// 用于了解PT的任务调度原理

static PT_THREAD(source_thread(struct pt *pt))
{
    static uint8_t s_cond = PT_FALSE;
    LOG("每次都进入 \r\n");
    PT_BEGIN_EX(pt);
    LOG("线程的开始了 ,while 1 在下面 \r\n");
    while (1)
    {
        LOG("PT_YIELD 的前面 \r\n");
        PT_YIELD(pt);
        LOG("PT_YIELD 的后面 \r\n");

        LOG("PT_YIELD_UNTIL 的前面：等待条件变真 \r\n");
        PT_YIELD_UNTIL(pt, s_cond);

        LOG("PT_YIELD_UNTIL 的后面：条件变真后进入 \r\n");
    }

    LOG("线程的结束 ,由于while 1 在上面 \r\n");
    PT_END_EX(pt);
}


static PT_THREAD(demo_thread(struct pt *pt))
{
    static uint8_t s_cond = PT_FALSE;

    PT_BEGIN_EX(pt);

    LOG("线程的开始了 ,while 1 在下面 demo thread \r\n");
    while (1)
    {
        LOG("PT_YIELD 的前面 \r\n");
        PT_YIELD(pt);
        LOG("PT_YIELD 的后面 \r\n");

        LOG("PT_YIELD_UNTIL 的前面：等待条件变真 \r\n");
        PT_YIELD_UNTIL(pt, s_cond);

        LOG("PT_YIELD_UNTIL 的后面：条件变真后进入 \r\n");
    }

    LOG("线程的结束 ,由于while 1 在上面 \r\n");
    PT_END_EX(pt);
}


static PT_THREAD(Protothread(struct pt *pt))
{
    static uint8_t s_case ;
    static uint8_t s_cond = PT_FALSE;
    PT_BEGIN_EX(pt);
    LOG("while的上面 \r\n");
    while (1)
    {
        switch (s_case)
        {
            case 0:
                PT_YIELD_UNTIL(pt, s_cond);
                break;
            case 1:
                break;
        }
    }
    LOG("while的下面 \r\n");
    PT_END_EX(pt);
}