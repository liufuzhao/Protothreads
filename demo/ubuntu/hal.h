#ifndef _HAL_H_
#define _HAL_H_

void uart_init(void);
void uart_send(uint8_t *data, uint16_t len);
int uart_receive_char(uint8_t *data);

#endif