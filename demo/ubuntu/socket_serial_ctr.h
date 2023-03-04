
#ifndef SOCKET_SERIAL_CTR_H
#define SOCKET_SERIAL_CTR_H

#define TCP_SERVER_PORT  7856
#define CONNECT_STATE    0
#define DISCONNECT_STATE 1
typedef void (*client_fd_state_cb)(uint8_t state);

int tcp_server_serial_init(client_fd_state_cb cb);
int tcp_server_serial_deinit(void);
int tcp_serial_get_char(char *data);
void tcp_serial_send(char *msg, int msg_len);

#endif