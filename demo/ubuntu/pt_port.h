#ifndef __PT_PORT_H
#define __PT_PORT_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/** \hideinitializer */
typedef unsigned short lc_t;

#define PT_TRUE  1
#define PT_FALSE 0

extern volatile uint32_t gu32_system_count;
#define PT_GET_TICK() gu32_system_count

#define WAKE_UP_PT_THREAD_SEM()

#define PT_LOG(...) printf(__VA_ARGS__)

#endif