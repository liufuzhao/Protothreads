#include "pt_port.h"
#include "pt_threads.h"
#include "led.h"

#define LED0 PCout(13)

static void led_drv_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // ʹ��PC,PE�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;        // LED0-->PC.13 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);            // �����趨������ʼ��GPIOC.13
    GPIO_SetBits(GPIOC, GPIO_Pin_13);                 // PC.13 �����
}


PT_THREAD(pt_led_thread(struct pt *pt))
{
    PT_BEGIN_EX(pt);

    while (1)
    {
        PT_SLEEP_MS(pt, 1000);
        LED0 = 1;
        PT_SLEEP_MS(pt, 1000);
        LED0 = 0;
        PT_YIELD(pt);
    }
    PT_END_EX(pt);
}

void pt_led_init(void)
{
    led_drv_Init();
    static pt_thread_info led_thread;
    if (pt_thread_is_register(&led_thread))
    {
        pt_thread_unregister(&led_thread);
    }
    pt_thread_reset(&led_thread);
    pt_thread_register(&led_thread, pt_led_thread, "pt_led_thread");
}
