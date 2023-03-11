#ifndef _HAL_H_
#define _HAL_H_

void serial_init(void);
void serial_deinit(void);
void serial_send(uint8_t *data, uint16_t len);
int serial_receive_char(uint8_t *data);

#endif