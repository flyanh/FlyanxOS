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
#if OPEN_TEST_TASK == 1     /* 开启了测试任务 */

#define TTY_TASK            -5  /* 终端I/O任务 */
#	define CANCEL           0	/* general req to force a task to cancel */
#	define HARD_INT         2	/* fcn code for all hardware interrupts */
#	define DEV_READ	        3	/* fcn code for reading from tty */
#	define DEV_WRITE        4	/* fcn code for writing to tty */
#	define DEV_IOCTL        5	/* fcn code for ioctl */
#	define DEV_OPEN         6	/* fcn code for opening tty */
#	define DEV_CLOSE        7	/* fcn code for closing tty */
#	define DEV_SCATTER      8	/* fcn code for writing from a vector */
#	define DEV_GATHER       9	/* fcn code for reading into a vector */
#	define TTY_SETPGRP      10	/* fcn code for setpgroup */
#	define TTY_EXIT	        11	/* a process group leader has exited */
#	define SUSPEND	        -998/* used in interrupts when tty has no data */

//#define IDLE_TASK           -6  /* 闲置任务的进程插槽号 */
#define IDLE_TASK           -4  /* 闲置任务的进程插槽号 */


#define CLOCK_TASK          -3  /* 时钟任务 */
#	define SET_ALARM        1	/* 时钟功能索引号，设置闹钟 */
#	define GET_TIME	        3	/* 时钟功能索引号，获得真实时间（秒） */
#	define SET_TIME	        4	/* 时钟功能索引号，设置真实时间（秒） */
#	define GET_UPTIME       5	/* 时钟功能索引号，获取时钟的运行时间（滴答） */
#	define SET_SYNC_ALARM   6	/* 时钟功能索引号，设置同步闹钟 */
#	define REAL_TIME        1	/* 时钟任务的回复代码，用于告诉请求者：这是一个真实时间 */
#	define CLOCK_INT   HARD_INT /* 此代码仅由同步闹钟任务发送，用来请求一个同步闹钟 */

#define TEST_TASK           -2      /* 测试任务：仅仅在开发中将其打开 */
#define HARDWARE            -1	    /* 用作中断生成消息的源 */

#else       /* 关闭测试任务 */

#define TTY_TASK            -4  /* 终端I/O任务 */
#	define CANCEL           0	/* general req to force a task to cancel */
#	define HARD_INT         2	/* fcn code for all hardware interrupts */
#	define DEV_READ	        3	/* fcn code for reading from tty */
#	define DEV_WRITE        4	/* fcn code for writing to tty */
#	define DEV_IOCTL        5	/* fcn code for ioctl */
#	define DEV_OPEN         6	/* fcn code for opening tty */
#	define DEV_CLOSE        7	/* fcn code for closing tty */
#	define DEV_SCATTER      8	/* fcn code for writing from a vector */
#	define DEV_GATHER       9	/* fcn code for reading into a vector */
#	define TTY_SETPGRP      10	/* fcn code for setpgroup */
#	define TTY_EXIT	        11	/* a process group leader has exited */
#	define SUSPEND	        -998/* used in interrupts when tty has no data */

//#define IDLE_TASK           -6  /* 闲置任务的进程插槽号 */
#define IDLE_TASK           -3  /* 闲置任务的进程插槽号 */


#define CLOCK_TASK          -2  /* 时钟任务 */
#	define SET_ALARM        1	/* 时钟功能索引号，设置闹钟 */
#	define GET_TIME	        3	/* 时钟功能索引号，获得真实时间（秒） */
#	define SET_TIME	        4	/* 时钟功能索引号，设置真实时间（秒） */
#	define GET_UPTIME       5	/* 时钟功能索引号，获取时钟的运行时间（滴答） */
#	define SET_SYNC_ALARM   6	/* 时钟功能索引号，设置同步闹钟 */
#	define REAL_TIME        1	/* 时钟任务的回复代码，用于告诉请求者：这是一个真实时间 */
#	define CLOCK_INT   HARD_INT /* 此代码仅由同步闹钟任务发送，用来请求一个同步闹钟 */

#define HARDWARE            -1	    /* 用作中断生成消息的源 */

#endif /* OPEN_TEST_TASK == 1 */

/* 时钟任务回复消息中使用的消息字段名称。 */
#define DELTA_TICKS     m6_l1	/* 闹钟间隔（时钟滴答数） */
#define FUNC_TO_CALL    m6_f1	/* 指向要闹钟响起要调用的函数的指针 */
#define CLOCK_TIME      m6_l1	/* 时钟的值（滴答） */
#define CLOCK_PROC_NR   m6_i1	/* 哪个进程（或任务）想要一个闹钟？ */
#define SECONDS_LEFT    m6_l1	/* 还剩几秒钟触发闹钟？ */

/* 系统任务回复消息中使用的消息字段名称。 */
#define REPLY_PROC_NR   m2_i1       /* 代表I/O完成的进程索引号 */
#define REPLY_STATUS    m2_i2       /* 传输的字节数或错误号 */

#endif //_FLYANX_COMMON_H
