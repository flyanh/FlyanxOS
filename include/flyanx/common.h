/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 主要包含从MM(内存管理器)和FS(文件系统)发送到I/O任务的消息中所使用的公共定义。
 * 其中也包括了任务序号。在这里为了与进程号分开，任务号是负数的。
 *
 * 该文件还定义了可被发送到每个系统任务的消息类型(函数代码)，例如,
 * 时钟任务接收代码SET_ALARM(用于设置定时器)、CLOCK_TICK(当时钟中断发生时)、 GET_TIME(取真实时间)、
 * SET_TIME(设置一天中的当前时间)、 REAL_TIME 的值就是对GET_TIME请求所返回的消息类型。
 */

#ifndef _FLYANX_COMMON_H
#define _FLYANX_COMMON_H

/* System calls. */
#define SEND		    1	/* function code for sending messages */
#define RECEIVE		    2	/* function code for receiving messages */
#define SEND_REC	    3	/* function code for SEND + RECEIVE */
#define ANY		    0x7fac	/* a magic, invalid process number.
                         * receive(ANY, buf) accepts from any source
                         */

/* 任务号，函数索引号(消息类型)和回复代码，将在下面开始定义 */
#define TTY_TASK                -5  /* 终端I/O任务 */
#	define CANCEL               0	/* 强制取消任务的一般要求 */
#	define HARD_INT             2	/* 所有硬件中断的索引代码 */
#	define DEVICE_READ	        3	/* 终端功能索引代码，从终端读取数据 */
#	define DEVICE_WRITE         4	/* 终端功能索引代码，写入数据到终端 */
#	define DEVICE_IOCTL         5	/* 终端功能索引代码，终端io控制 */
#	define DEVICE_OPEN          6	/* 终端功能索引代码， */
#	define DEVICE_CLOSE         7	/* 终端功能索引代码，关闭一个终端设备  */
#	define DEVICE_SCATTER       8	/* fcn code for writing from a vector */
#	define DEVICE_GATHER        9	/* fcn code for reading into a vector */
#	define TTY_SET_PROC_GROUP   10	/* 终端功能索引代码，设置终端的进程组 */
#	define TTY_EXIT	        11	/* a process group leader has exited */
#	define SUSPEND	        -998/* used in interrupts when tty has no data */

//#define IDLE_TASK           -6  /* 闲置任务的进程插槽号 */
#define IDLE_TASK           -4  /* 闲置任务的进程插槽号 */


#define CLOCK_TASK          -3  /* 时钟任务 */
#	define SET_ALARM        1	/* 时钟功能索引代码，设置闹钟 */
#	define GET_TIME	        3	/* 时钟功能索引代码，获得真实时间（秒） */
#	define SET_TIME	        4	/* 时钟功能索引代码，设置真实时间（秒） */
#	define GET_UPTIME       5	/* 时钟功能索引代码，获取时钟的运行时间（滴答） */
#	define SET_SYNC_ALARM   6	/* 时钟功能索引代码，设置同步闹钟 */
#	define REAL_TIME        1	/* 时钟任务的回复代码，用于告诉请求者：这是一个真实时间 */
#	define CLOCK_INT   HARD_INT /* 此代码仅由同步闹钟任务发送，用来请求一个同步闹钟 */

#define SYS_TASK            -2  /* 系统任务，用于服务器跟内部通信获得系统内部功能 */
#	define SYS_EXIT         1   /* 系统功能索引代码，sys_exit(parent, proc) */
#   define SYS_GET_SP       2   /* 系统功能索引代码，sys_sp(proc, &new_sp) */
#   define SYS_FORK         3   /* 系统功能索引代码，sys_fork(parent, child, pid) */
#	define SYS_PUTS         4	/* 系统功能索引代码，sys_puts(count, buf) */
#   define SYS_FIND_PROC    5   /* 系统功能索引代码，sys_find_proc(name, &task_nr, flags) */

#define HARDWARE            -1	    /* 用作中断生成消息的源 */

/* 块设备和字符设备任务消息中使用的消息字段名称。 */
#define DEVICE          m2_i1	/* 主-次设备号 */
#define PROC_NR         m2_i2	/* 那个进程需要I/O服务？ */
#define COUNT           m2_i3	/* 有多少字节将要被传送 */
#define REQUEST         m2_i3	/* io控制请求代码 */
#define POSITION        m2_l1	/* 文件偏移地址 */
#define ADDRESS         m2_p1	/* 内核的缓冲区地址 */

/* 终端任务消息中使用的消息字段名称。 */
#define TTY_LINE        DEVICE      /* 终端线 */
#define TTY_REQUEST     COUNT       /* io控制请求代码 */
#define TTY_SPEK        m2_l1       /* io控制速率，清除 */
#define TTY_FLAGS       m2_l2       /* io控制终端模式 */
#define TTY_PROC_GROUP  m2_i3       /* 进程组 */

/* 时钟任务消息中使用的消息字段名称。 */
#define DELTA_TICKS     m6_l1	/* 闹钟间隔（时钟滴答数） */
#define FUNC_TO_CALL    m6_f1	/* 指向要闹钟响起要调用的函数的指针 */
#define CLOCK_TIME      m6_l1	/* 时钟的值（滴答） */
#define CLOCK_PROC_NR   m6_i1	/* 哪个进程（或任务）想要一个闹钟？ */
#define SECONDS_LEFT    m6_l1	/* 还剩几秒钟触发闹钟？ */

/* 系统任务回复消息中使用的消息字段名称。 */
#define REPLY_PROC_NR   m2_i1       /* 代表I/O完成的进程索引号 */
#define REPLY_STATUS    m2_i2       /* 传输的字节数或错误号 */

#endif //_FLYANX_COMMON_H
