
#ifndef _TTY_CTR_H_
#define _TTY_CTR_H_

int tty_init(char *device_path);
int tty_deinit(int fd);
int tty_get_char(int fd, char *data);
void tty_send(int fd, char *msg, int msg_len);
#endif