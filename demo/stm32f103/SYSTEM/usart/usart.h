#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "sys.h"

void uart_init(uint32_t bound);
void uart_init_queue(void);
uint8_t uart_read_queue(uint8_t *data);
void uart_send_data(uint8_t *data, uint16_t len);
#endif
