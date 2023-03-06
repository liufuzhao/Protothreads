#ifndef __PT_PORT_H
#define __PT_PORT_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

/** \hideinitializer */
typedef unsigned short lc_t;

#define PT_TRUE  1
#define PT_FALSE 0

extern volatile uint32_t gu32_system_count;
#define PT_GET_TICK() gu32_system_count

#define WAKE_UP_PT_THREAD_SEM()

#define PT_LOG_HEX(data, len)                                                    \
    do                                                                           \
    {                                                                            \
        struct timeval tv;                                                       \
        gettimeofday(&tv, NULL);                                                 \
        printf("[%ld][%04d]", (tv.tv_sec * 1000 + tv.tv_usec / 1000), __LINE__); \
        for (uint16_t i = 0; i < (len); i++)                                     \
            printf("%02x", (data)[i]);                                           \
        printf("\r\n");                                                          \
    }                                                                            \
    while (0)

#define PT_LOG_RAW(format, ...) printf(format, ##__VA_ARGS__)

#define PT_LOG(format, ...)                                                                            \
    do                                                                                                 \
    {                                                                                                  \
        struct timeval tv;                                                                             \
        gettimeofday(&tv, NULL);                                                                       \
        printf("[%ld][%04d]" format, (tv.tv_sec * 1000 + tv.tv_usec / 1000), __LINE__, ##__VA_ARGS__); \
    }                                                                                                  \
    while (0)

#define LOG PT_LOG

#define PT_BOX_FUNC_ENABLE     1
#define PT_MONITOR_FUNC_ENABLE 1
#endif