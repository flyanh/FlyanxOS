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

/* 系统调用，系统层次上的，区别与posix系统调用。 */
#define SEND		    1	/* function code for sending messages */
#define RECEIVE		    2	/* function code for receiving messages */
#define SEND_REC	    3	/* function code for SEND + RECEIVE */
#define ANY		    0x679	/* 魔数，它是一个错误的进程索引号，用于接收消息
                             * receive(ANY, buf)表示想接收任何人的消息
                             */

/* 任务号，函数索引号(消息类型)和回复代码，将在下面开始定义 */
#define NO_EXIST_TASK       -808    /* 用于不可能存在的任务号 */

#define TTY_TASK                (CONTROLLER(NR_CONTROLLERS) - 2)  /* 终端I/O任务 */
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
#	define TTY_EXIT	            11	/* 一个进程组长已经退出 */
#	define SUSPEND	        -998    /* 当终端设备没有数据可被读取时，用于挂起请求者 */

#define CLOCK_TASK          (CONTROLLER(NR_CONTROLLERS) - 1)  /* 时钟任务 */
#	define SET_ALARM        1	/* 时钟功能索引代码，设置闹钟 */
#	define GET_TIME	        3	/* 时钟功能索引代码，获得真实时间（秒） */
#	define SET_TIME	        4	/* 时钟功能索引代码，设置真实时间（秒） */
#	define GET_UPTIME       5	/* 时钟功能索引代码，获取时钟的运行时间（滴答） */
#	define SET_SYNC_ALARM   6	/* 时钟功能索引代码，设置同步闹钟 */
#	define REAL_TIME        1	/* 时钟任务的回复代码，用于告诉请求者：这是一个真实时间 */
#	define CLOCK_INT   HARD_INT /* 此代码仅由同步闹钟任务发送，用来请求一个同步闹钟 */

#define CONTROLLER(n)       (-4 - (n))  /* 控制器任务 */

#define TEST_TASK           -4  /* 测试任务 */

#define IDLE_TASK           -3  /* 闲置任务的进程插槽号 */

#define SYS_TASK            -2  /* 系统任务，用于服务器跟内部通信获得系统内部功能 */
#	define SYS_EXIT         1   /* 系统功能索引代码，sys_exit(parent, proc) */
#   define SYS_GET_SP       2   /* 系统功能索引代码，sys_sp(proc, &new_sp) */
#   define SYS_FORK         3   /* 系统功能索引代码，sys_fork(parent, child, pid) */
#	define SYS_PUTS         4	/* 系统功能索引代码，sys_puts(count, buf) */
#   define SYS_FIND_PROC    5   /* 系统功能索引代码，sys_find_proc(name, &task_nr, flags) */
#   define SYS_SUDDEN       6   /* 系统功能索引代码，sys_sudden() */
#   define SYS_BLUES        7   /* 系统功能索引代码，sys_bules() */
#   define SYS_COPY         8   /* sys_copy(...) */
#   define SYS_GET_MAP      9   /* sys_get_map(proc_nr, mm_ptr) */
#   define SYS_NEW_MAP      10  /* sys_new_map(proc_nr, mm_ptr)  */
#   define SYS_EXEC         11  /* sys_exec(procno, new_sp) */
#   define SYS_SET_PROG_FRAME 12    /* sys_set_prog_frame(int argc, u32_t argv, u32_t envp) */

#define HARDWARE            -1	    /* 用作中断生成消息的源 */

/* 终端任务消息中使用的消息字段名称。 */
#define TTY_LINE        DEVICE      /* 终端线 */
#define TTY_REQUEST     COUNT       /* io控制请求代码 */
#define TTY_SPEK        m2_l1       /* io控制速率，清除 */
#define TTY_FLAGS       m2_l2       /* io控制终端模式 */
#define TTY_PROC_GROUP  m2_i3       /* 进程组 */

/* 块设备和字符设备任务消息中使用的消息字段名称。 */
#define DEVICE          m2_i1	/* 主-次设备号 */
#define PROC_NR         m2_i2	/* 哪个进程需要I/O服务？ */
#define FLAGS           m2_i3   /* 用于打开设备，以什么权限打开？例如只读。 */
#define COUNT           m2_i3	/* 有多少字节将要被传送 */
#define REQUEST         m2_i3	/* io控制请求代码 */
#define POSITION        m2_l1	/* 文件偏移地址 */
#define ADDRESS         m2_p1	/* 内核的缓冲区地址 */

/* 时钟任务消息中使用的消息字段名称。 */
#define DELTA_TICKS     m6_l1	/* 闹钟间隔（时钟滴答数） */
#define FUNC_TO_CALL    m6_f1	/* 指向要闹钟响起要调用的函数的指针 */
#define CLOCK_TIME      m6_l1	/* 时钟的值（滴答） */
#define CLOCK_PROC_NR   m6_i1	/* 哪个进程（或任务）想要一个闹钟？ */
#define SECONDS_LEFT    m6_l1	/* 还剩几秒钟触发闹钟？ */

/* 复制消息到系统任务的消息字段名称。 */
#define SRC_SPACE       m5_c1	/* 空间类型，代码段或数据段空间，（堆栈段也被认为是数据段） */
#define SRC_PROC_NR     m5_i1	/* 复制从哪个进程 */
#define SRC_BUFFER      m5_l1	/* 数据来自的虚拟地址 */
#define DEST_SPACE      m5_c2	/* 空间类型，代码段或数据段空间，（堆栈段也被认为是数据段） */
#define DEST_PROC_NR    m5_i2	/* 复制到的进程 */
#define DEST_BUFFER     m5_l2	/* 数据前往的虚拟地址 */
#define COPY_BYTES      m5_l3	/* 要复制的字节数 */

/* 任务回复消息中使用的消息字段名称。 */
#define REPLY_PROC_NR   m2_i1       /* 代表I/O完成的进程索引号 */
#define REPLY_STATUS    m2_i2       /* 传输的字节数或错误号 */

/* 系统任务使用的消息字段名称 */
#define MEM_MAP_PTR     m1_p1   /* 存放一个内存映像指针，sys_get_map和sys_new_map需要这个消息字段。 */
#define PROC_NR1        m1_i1	/* 一个进程号 */
#define PROC_NR2        m1_i2	/* 同上 */
#define PID             m1_i3   /* 进程号 */
#define STACK_PTR       m1_p1	/* sys_exec()、sys_get_sp()需要传递栈指针。 */
#define NAME_PTR        m1_p2   /* 程序名称字符串地址 */
#define PC_PTR          m1_p3   /* 初始程序计数器，指向程序的开始地址 */
#define PROC_NR3        m2_i1   /* 一个进程号，设置它是因为sys_set_prog_frame()需要，但消息类型1满足不了。 */
#define ARGC            m2_i2   /* 程序的命令行参数计数 */
#define ARGV            m2_l1   /* 程序的命令行参数数组地址 */
#define ENVP            m2_l2   /* 程序的环境变量数组地址 */

#endif //_FLYANX_COMMON_H
