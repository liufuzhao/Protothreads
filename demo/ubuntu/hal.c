#include "pt_port.h"
#include "tty_ctr.h"
#include "hal.h"

static int s_fd = 0;
void uart_init(void)
{
    uint8_t *name = "/dev/ttyUSB0";
    s_fd = tty_init(name);
}

void uart_send(uint8_t *data, uint16_t len)
{
    tty_send(s_fd, data, len);

}

int uart_receive_char(uint8_t *data)
{
    return tty_get_char(s_fd, data);
}