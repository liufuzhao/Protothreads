#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>

#include <semaphore.h>
#include <errno.h>
#include <pthread.h>

#include <fcntl.h>
#include <unistd.h>

#include "socket_serial_ctr.h"
#include "pt_port.h"

static int s_server_fd = 0;
static int s_client_fd = 0;
static client_fd_state_cb s_state_cb;
static sem_t s_client_sem;
static pthread_t s_server_tcp_thread;
static void server_tcp_thread(void *arg)
{
    
    s_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s_server_fd < 0)
    {
        perror("socket creation failed");
        return;
    }
    int val = 1;
    int ret = setsockopt(s_server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
    if (ret == -1)
    {
        printf("setsockopt");
        return;
    }
    // 绑定地址和端口
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TCP_SERVER_PORT);
    if (bind(s_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(s_server_fd);
        return;
    }

    // 监听端口
    if (listen(s_server_fd, 3) < 0)
    {
        perror("listen failed");
        close(s_server_fd);
        return;
    }

    // 接收连接请求并处理数据
    struct sockaddr_in client_address;
    int addrlen = sizeof(client_address);
    char buffer[1024] = {0};

    while (1)
    {
        s_state_cb(DISCONNECT_STATE);
        if ((s_client_fd = accept(s_server_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept failed");
            close(s_server_fd);
            return;
        }

        int flags;
        flags = fcntl(s_client_fd, F_GETFL, 0);          // 获取文件的flags值。
        fcntl(s_client_fd, F_SETFL, flags | O_NONBLOCK); // 设置成非阻塞模式；

        printf("client connect: %d\n", s_client_fd);
        if (s_state_cb)
        {
            s_state_cb(CONNECT_STATE);
        }
        sem_wait(&s_client_sem);
    }
}

int tcp_server_serial_init(client_fd_state_cb cb)
{
    tcp_server_serial_deinit();

    s_state_cb = cb;
    sem_init(&s_client_sem, 0, 0);
    pthread_create(&s_server_tcp_thread, NULL, (void *)server_tcp_thread, NULL);
    pthread_detach(s_server_tcp_thread);

    return PT_TRUE;
}

int tcp_server_serial_deinit()
{
    if (s_server_fd == 0)
    {
        close(s_server_fd);
        s_server_fd = 0;
    }
    if (s_client_fd == 0)
    {
        close(s_client_fd);
        s_client_fd = 0;
    }
}

int tcp_serial_get_char(char *data)
{
    int n;
    if (s_client_fd == 0)
    {
        return PT_FALSE;
    }
    n = read(s_client_fd, data, 1);
    if (1 == n)
    {
        return PT_TRUE;
    }
    else if (n == 0 && errno == 11)
    {
        printf("connect error: %d errno %d ", n, errno);
        close(s_client_fd);
        sem_post(&s_client_sem);
    }
    else
    {
        return PT_FALSE;
    }
}

void tcp_serial_send(char *msg, int msg_len)
{
    if (s_client_fd)
    {
        write(s_client_fd, msg, msg_len);
    }
}
