# Protothreads
A lightweight, non-preemptive, and stackless operating system implemented in C based on the principles of task scheduling using Protothreads.

基于protothreads的任务调度原理,实现的轮询式操作系统
支持：
 1. PT_SLEEP 休眠指定MS
2. PT_QUEUE_WRITE_BLOCK 、PT_QUEUE_READ_BLOCK 读写消息队列,阻塞指定MS
3. PT_EVNET_WAIT 等待事件置位，阻塞指定MS<br>
