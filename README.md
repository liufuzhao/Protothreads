# Protothreads
A lightweight, non-preemptive, and stackless operating system implemented in C based on the principles of task scheduling using Protothreads.

基于protothreads的任务调度原理,实现的轮询式操作系统
支持：
1. PT_SLEEP 休眠指定MS
2. PT_QUEUE_WRITE_BLOCK 、PT_QUEUE_READ_BLOCK 读写消息队列,阻塞指定MS
3. PT_EVNET_WAIT_BLOCK 等待事件置位,阻塞指定MS
4. PT_BOX_ACCEPT_BLOCK 表示等待邮箱,阻塞指定MS,PT_BOX_SEND_BLOCK 表示发送邮箱消息,最多等待MS,如果没有线程处理,将会超时
5. 以YEILD后缀结尾的,也表示阻塞函数,但阻塞的超时时间以轮询次数为单位 
