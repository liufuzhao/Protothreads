#include "pt_port.h"
#include "usart.h"


#define USART_REC_LEN 20
typedef struct
{
    uint8_t read;
    uint8_t write;
    uint8_t array[USART_REC_LEN];
} usart_queue_t;

static usart_queue_t s_debug_queue;

void uart_init_queue(void)
{
    memset((uint8_t *)&s_debug_queue, 0, sizeof(s_debug_queue));
}

static uint8_t uart_write_queue(uint8_t data)
{
    if ((s_debug_queue.write + 1) % USART_REC_LEN == s_debug_queue.read)
    {
        return FALSE;
    }
    else
    {
        s_debug_queue.array[s_debug_queue.read] = data;
        s_debug_queue.write = (s_debug_queue.write + 1) % USART_REC_LEN;
        return TRUE;
    }
}

uint8_t uart_read_queue(uint8_t *data)
{
    if (s_debug_queue.read == s_debug_queue.write)
    {
        return FALSE;
    }
    else
    {
        *data = s_debug_queue.array[s_debug_queue.read];
        s_debug_queue.read = (s_debug_queue.read + 1) % USART_REC_LEN;
        return TRUE;
    }
}


int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);

    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {}

    return ch;
}
void uart_send_data(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        USART_SendData(USART1, (uint8_t)data[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {}
    }
}

void uart_init(uint32_t bound)
{
    // GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); // 使能USART1，GPIOA时钟

    // USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);          // 初始化GPIOA.9

    // USART1_RX	  GPIOA.10初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;            // PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                // 初始化GPIOA.10

    // Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器

    // USART 初始化设置

    USART_InitStructure.USART_BaudRate = bound;                                     // 串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式

    USART_Init(USART1, &USART_InitStructure);      // 初始化串口1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 开启串口接受中断
    USART_Cmd(USART1, ENABLE);                     // 使能串口1
}

void USART1_IRQHandler(void) // 串口1中断服务程序
{
    uint8_t Res;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(USART1);
        uart_write_queue(Res);
    }
}
