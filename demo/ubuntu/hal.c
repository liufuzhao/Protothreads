#include "pt_port.h"
#include "tty_ctr.h"
#include "hal.h"

#include "socket_serial_ctr.h"
static int s_fd = 0;

void socket_serial_fd_cb(uint8_t state)
{
    if (state == CONNECT_STATE)
    {
        printf("CONNECT_STATE");
        s_fd = 1;
    }

    else
    {
        printf("DISCONNECT_STATE");
        s_fd = 0;
    }
}

void serial_init(void)
{
#if DEF_UART_SERIAL
    uint8_t *name = "/dev/ttyUSB0";
    s_fd = tty_init(name);

#else
    tcp_server_serial_init(socket_serial_fd_cb);
#endif
}

void serial_deinit(void)
{
    if (s_fd == 0)
    {
        return;
    }
#if DEF_UART_SERIAL
    tty_deinit(s_fd)
#else
    tcp_server_serial_deinit();
#endif
}

void serial_send(uint8_t *data, uint16_t len)
{
    if (s_fd == 0)
    {
        return;
    }
#if DEF_UART_SERIAL
    tty_send(s_fd, data, len);
#else
    tcp_serial_send(data, len);
#endif
}

int serial_receive_char(uint8_t *data)
{
    if (s_fd == 0)
    {
        return 0;
    }
#if DEF_UART_SERIAL
    return tty_get_char(s_fd, data);
#else
    return tcp_serial_get_char(data);
#endif
}