
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define DEF_BAUD  115200
#define BUF_SIZE  128

static pthread_t s_tty_task_tid;

char buffer[BUF_SIZE];

int speed_arr[] = {B9600, B19200, B38400, B57600, B115200};

int name_arr[] = {9600, 19200, 38400, 57600, 115200};

/**
 *@brief  设置串口通信速率
 *@param  fd     类型 int  打开串口的文件句柄
 *@param  speed  类型 int  串口速度
 *@return  0 success or -1 err
 */
static int set_speed(int fd, int speed)
{
    int i;
    int status;
    struct termios opt;

    tcgetattr(fd, &opt);

    for (i = 0; i < sizeof(speed_arr)/sizeof(int); i++)
    {
        if (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            /*  设置串口的波特率 */
            cfsetispeed(&opt, speed_arr[i]);
            cfsetospeed(&opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &opt);

            if (status != 0)
            {
                perror("tcsetattr set_speed");
                return -1;
            }

            return 0;
        }
        /*清空所有正在发生的IO数据*/
        tcflush(fd, TCIOFLUSH);
    }

    printf("Cannot find suitable speed\n");
    return -1;
}

/**
 *@brief   设置串口数据位，停止位和效验位
 *@param  fd     类型  int  打开的串口文件句柄*
 *@param  databits 类型  int 数据位   取值 为 7 或者8*
 *@param  stopbits 类型  int 停止位   取值为 1 或者2*
 *@param  parity  类型  int  效验类型 取值为N,E,O,,S
 *@return  0 success or -1 err
 */
static int set_parity(int fd, int databits, int stopbits, int parity)
{

    struct termios options;
    if (tcgetattr(fd, &options) != 0)
    {
        perror("tcgetattr");
        return -1;
    }

    options.c_cflag &= ~CSIZE;
    switch (databits) /*设置数据位数*/
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "Unsupported data size\n");

            return -1;
    }

    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB; /* Clear parity enable */
            options.c_iflag &= ~INPCK;  /* Enable parity checking */
            options.c_iflag &= ~(ICRNL | IGNCR);
            options.c_lflag &= ~(ICANON);
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;  /* Enable parity */
            options.c_cflag &= ~PARODD; /* 转换为偶效验*/
            options.c_iflag |= INPCK;   /* Disnable parity checking */
            break;
        case 'S':
        case 's': /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return -1;
    }

    /* 设置停止位*/
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported stop bits\n");
            return -1;
    }

    /* Set input parity option */
    if ((parity != 'n') || (parity != 'N'))
        options.c_iflag |= INPCK;

    /* 若以O_NONBLOCK 方式open，这两个设置没有作用，等同于都为0 */
    /* 若非O_NONBLOCK 方式open，具体作用可参考其他博客，关键词linux VTIME */
    options.c_cc[VTIME] = 10; // 1s
    options.c_cc[VMIN] = 0;

    /* 清空正读的数据，且不会读出 */
    tcflush(fd, TCIFLUSH);

    /*采用原始模式通讯*/
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_cflag &= ~CRTSCTS; // 设置无RTS/CTS硬件流控
    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    /*解决发送0x0A的问题*/
    //  options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    options.c_oflag &= ~(ONLCR | OCRNL | ONOCR | ONLRET);

    /* Update the options and do it NOW */
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("SetupSerial 3");
        return -1;
    }

    return 0;
}

int tty_deinit(int fd)
{
    if (fd)
    {
        close(fd);
        fd = 0;
    }

    return 0;
}

int tty_init(char *device_path)
{
    int fd;
    int ret = -1;
    const char *dev = NULL;

    dev = device_path;
    /* 2、打开串口 */
    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    // fd = open(dev, O_RDWR | O_NOCTTY);//阻塞打开串口
    if (-1 == fd)
    {
        printf("Cannot open %s:%s\n", dev, strerror(errno));
        // exit(1);
    }

    /* 3、初始化设备 */
    if (-1 == set_speed(fd, DEF_BAUD))
    {
        printf("Cannot set baudrate to 115200\n");
        close(fd);
        exit(1);
    }

    if (-1 == set_parity(fd, 8, 1, 'N'))
    {
        printf("Set Parity Error\n");
        close(fd);
        exit(1);
    }

    return fd;
}

int tty_get_char(int fd, uint8_t *data)
{
    if (read(fd, data, 1) == 1)
    {
        return 1;
    }
    return 0;
}

void tty_send(int fd, char *msg, int msg_len)
{
    if (fd)
    {
        write(fd, msg, msg_len);
    }
}
