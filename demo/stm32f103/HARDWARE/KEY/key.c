#include "stm32f10x.h"
#include "key.h"
#include "sys.h"
#include "delay.h"


// ������ʼ������
void KEY_Init(void) // IO��ʼ��
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE); // ʹ��PORTA,PORTEʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_3; // KEY0-KEY1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;          // ���ó���������
    GPIO_Init(GPIOE, &GPIO_InitStructure);                 // ��ʼ��GPIOE4,3

    // ��ʼ�� WK_UP-->GPIOA.0	  ��������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // PA0���ó����룬Ĭ������
    GPIO_Init(GPIOA, &GPIO_InitStructure);        // ��ʼ��GPIOA.0
}

//u8 KEY_Scan(u8 mode)
//{
//    static u8 key_up = 1; // �������ɿ���־
//    if (mode)
//        key_up = 1; // ֧������
//    if (key_up && (KEY0 == 0 || KEY1 == 0 || WK_UP == 1))
//    {
//        delay_ms(10); // ȥ����
//        key_up = 0;
//        if (KEY0 == 0)
//            return KEY0_PRES;
//        else if (KEY1 == 0)
//            return KEY1_PRES;
//        else if (WK_UP == 1)
//            return WKUP_PRES;
//    }
//    else if (KEY0 == 1 && KEY1 == 1 && WK_UP == 0)
//        key_up = 1;
//    return 0; // �ް�������
//}
