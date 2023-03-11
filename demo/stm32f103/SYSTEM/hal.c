#include "usart.h"
#include "hal.h"
void serial_init(void)
{
    uart_init(115200);
}
void serial_deinit(void)
{}
void serial_send(uint8_t *data, uint16_t len)
{
    uart_send_data(data, len);
}
int serial_receive_char(uint8_t *data)
{
    return uart_read_queue(data);
}