#ifndef __PT_QUEUE_EXTERN_H
#define __PT_QUEUE_EXTERN_H
#include "pt.h"
#include "pt_port.h"
#include "pt_timers.h"

typedef struct
{
    uint8_t *addr;                // 基本指针
    volatile uint32_t read;       // 读
    volatile uint32_t write;      // 写
    volatile uint16_t item_size;  // 每一个成员的大小
    volatile uint16_t queue_size; // 总队列大小
} pt_queue_t;

#define _IS_WRITEABLE(q) ((((q)->write + (q)->item_size) % (q)->queue_size) != (q)->read)
#define _IS_READABLE(q)  ((q)->write != (q)->read)

#define PT_QUEUE_INIT(q, a, mS, iS) pt_queue_init(q, a, mS, iS)
#define PT_QUEUE_WRITE(q, item)     pt_queue_write(q, item)
#define PT_QUEUE_READ(q, item)      pt_queue_read(q, item)

#define PT_QUEUE_HAS_DATA(q)                   _IS_READABLE(q)
#define PT_QUEUE_WRITE_BLOCK(pt, q, item, ms)  _PT_TIME_ENABLE(pt, _IS_WRITEABLE(q), PT_QUEUE_WRITE(q, item), (ms))
#define PT_QUEUE_READ_BLOCK(pt, q, item, ms)   _PT_TIME_ENABLE(pt, _IS_READABLE(q), PT_QUEUE_READ(q, item), (ms))
#define PT_QUEUE_WRITE_YEILD(pt, q, item, cnt) _PT_YIELD_ENABLE(pt, _IS_WRITEABLE(q), PT_QUEUE_WRITE(q, item), (cnt))
#define PT_QUEUE_READ_YEILD(pt, q, item, cnt)  _PT_YIELD_ENABLE(pt, _IS_READABLE(q), PT_QUEUE_READ(q, item), (cnt))

// 非协程可用
void pt_queue_init(pt_queue_t *queue, uint8_t *addr, uint16_t memery_size, uint16_t item_size);
int pt_queue_write(pt_queue_t *queue, void *item);
int pt_queue_read(pt_queue_t *queue, void *item);

#endif
