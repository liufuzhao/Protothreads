#include "pt_port.h"
#include "pt_queue_extern.h"


void pt_queue_init(pt_queue_t *queue, uint8_t *addr, uint16_t memery_size, uint16_t item_size)
{
    queue->addr = (char *)addr;
    queue->read = 0;
    queue->write = 0;
    queue->item_size = item_size;
    queue->queue_size = memery_size / item_size * item_size;
}

int pt_queue_write(pt_queue_t *queue, void *item)
{
    if (_IS_WRITEABLE(queue))
    {
        memcpy(&queue->addr[queue->write], (item), queue->item_size);
        queue->write += queue->item_size;
        if (queue->write >= queue->queue_size)
        {
            queue->write = 0;
        }
        WAKE_UP_PT_THREAD_SEM();
        return PT_TRUE;
    }
    return PT_FALSE;
}

int pt_queue_read(pt_queue_t *queue, void *item)
{
    if (_IS_READABLE(queue))
    {
        memcpy((item), &queue->addr[queue->read], queue->item_size);
        queue->read += queue->item_size;
        if (queue->read >= queue->queue_size)
        {
            queue->read = 0;
        }
        return PT_TRUE;
    }
    return PT_FALSE;
}
