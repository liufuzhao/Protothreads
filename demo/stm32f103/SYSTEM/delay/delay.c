#include "delay.h"
#include "misc.h"

static uint8_t fac_us = 0; // us延时倍乘数
static uint8_t fac_ms = 0;

static uint32_t delay_ostickspersec = 200;
volatile uint32_t gu32_system_count;

void SysTick_Handler(void)
{
    gu32_system_count += fac_ms;
}

void delay_init()
{
    uint32_t reload;
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 
    fac_us = SystemCoreClock / 8000000;                   
    reload = SystemCoreClock / 8000000;
    reload *= 1000000 / delay_ostickspersec;

    fac_ms = 1000 / delay_ostickspersec;

    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = reload;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    gu32_system_count = 0;
}

void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;
    ticks = nus * fac_us;
    tcnt = 0;
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break;
        }
    };
}
