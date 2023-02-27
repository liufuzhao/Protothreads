#include "pt_port.h"
#include "pt_sem_extern.h"

void pt_sem_init(pt_sem_t *sem, uint8_t bin)
{
    sem->count = 0;
    if (bin)
    {
        sem->bin = PT_TRUE;
    }
    else
    {
        sem->bin = PT_FALSE;
    }
}

void pt_sem_post(pt_sem_t *sem)
{
    if (sem->bin == PT_TRUE)
    {
        sem->count = 1;
    }
    else
    {
        sem->count++;
    }
    WAKE_UP_PT_THREAD_SEM();
}
