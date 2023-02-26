#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include "pt_threads.h"

volatile uint32_t gu32_SystemCount;

static pthread_t s_tick_task;
static void tick_task(void *arg)
{
    while (1)
    {
        usleep(1000 * 5);
        gu32_SystemCount += 5;
    }
}


extern void register_test_threads(void);
void main(int argc, char *argv[])
{
    // new a system tick thread in ubuntu
    pthread_create(&s_tick_task, NULL, (void *)tick_task, NULL);
    pthread_detach(s_tick_task);

    pt_threads_init();
    register_test_threads();
    while (1)
    {
        pt_threads_run();
    }
}