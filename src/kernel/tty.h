/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * tty.h为所有类型的终端设备提供支持
 */

#ifndef FLYANX_TTY_H
#define FLYANX_TTY_H

#define TTY_INPUT_BYTES     256     /* 终端输入队列大小 */
#define TAB_SIZE            8       /* 制表符间距 */
#define TAB_MASK            7       /* 计算制表符停止位置的掩码。 */

#define ESCAPE              '\33'   /* 转移字符 */

// 这两个标志在"include/fcntl.h"中已经定义过了。在这里再重写一次是为了避免需要再包含一个文件
#define O_NOCTTY            00400
#define O_NONBLOCK          04000

typedef _PROTOTYPE( void (*device_func_t), (struct tty_s *tty) );
typedef _PROTOTYPE( void (*device_func_arg_t), (struct tty_s *tty, int c) );

/* 终端结构 */
typedef struct tty_s {
    int events;     /* 标志：当某个中断引起变化需要终端任务来处理设备
                     * 分为读、写和io控制三种，其标志在本文件下面有说明。 */

    /* 输入队列，输入的字符将存在这里，直接被程序获取使用 */
    u32_t input_buffer[256];    /* 输入缓冲区 */
    u32_t *input_free;          /* 指向输入队列的下一个空闲点 */
    u32_t *input_handle;        /* 指向应该被处理并返回给终端的扫描码 */
    int input_count;   /* 队列中字符的计数 */
    int eot_count;     /* 字符的行数计数 */
    device_func_t device_read;  /* 执行设备读操作的指定例程的指针保存在这 */
    device_func_t input_cancel; /* 执行输入取消操作指定例程的指针保存在这 */

    /* min用来和eot_count比较，当两者一致时，就被认为一个读操作结束。
     * 规范输入模式：min被置成1，eot_count用于对输入的行数计数。
     * 原始模式下：eot_count对字符计数，min则由终端控制结构(termios)
     * 中控制字符数组中的MIN域决定。
     * 像上面那样设定，通过比较这两个变量就可以判断出一行是否准备
     * 好或者何时达到最小字符计数。
     */
    int min;
    clock_t wake_time;          /* 终端应该被唤醒的时间，这个值决定了终端任务何时被时钟中断处理程序唤醒 */
    struct tty_s *next_wake;    /* 下一个会被唤醒的终端，头部永远是被唤醒了正在活动的终端 */

    /* 输出部分
     * 由于输出队列是由设备指定写例程处理的，所以终端的输出部分没有变量说明，全部都由
     * 指向写、回显、中断信号和取消输出等设备指定例程的指针组成。
     */
    device_func_t device_write; /* 设备写例程指针，例程是设备实际的设备输出 */
    device_func_arg_t echo;     /* 终端回显函数，例程将回显字符到屏幕上 */
    device_func_t output_cancel;/* 取消设备的所以正进行的输出操作 */
    device_func_t device_break; /* 让设备发送一个中断 */

    /* 终端的参数和状态 */
    int position;          /* 当前屏幕所处位置 */
    bool reprint;      /* 状态：当回显输入混乱时为1,否则为0 */
    char status_escaped;      /* 状态：当看到LNEXT(^V，转义)时为1，否则为0 */
    char status_inhibited;    /* 运行状态：当为1时就停止当前的所有处理 */
    char p_group;               /* 终端的控制进程(master)的进程号 */
    char open_count;            /* 终端被打开的次数 */

    /* 一些I/O请求相关的信息在这，首先是输入 */
    char in_reply_code;         /* 回复代码，TASK_REPLY(任务回复)或者REVIVE(恢复) */
    char in_caller;             /* 管理系统调用的系统服务进程（通常是文件系统） */
    char in_proc;               /* 想要从终端读取数据的进程 */
    vir_bytes in_vir;           /* 数据要到达的地方（虚拟地址） */
    int in_left;                /* 还需要读取多少字符 */
    int in_cum;                 /* 已经读取了多少字符 */

    /* 输出(write)系统调用也需要以上类似的变量集 */
    char out_reply_code;        /* 回复代码，TASK_REPLY(任务回复)或者REVIVE(恢复) */
    char out_caller;            /* 管理系统调用的系统服务进程（通常是文件系统） */
    char out_proc;              /* 想要写入数据到终端的进程 */
    vir_bytes out_vir;          /* 数据要到达的地方（虚拟地址） */
    int out_left;               /* 还需要写入多少字符 */
    int out_cum;                /* 已经写入了多少字符 */

    /* IOCTL（输入输出控制）操作相关 */
    char ioc_caller;            /* 管理终端io控制系统调用的系统服务进程（通常是文件系统） */
    char ioc_proc;              /* 想要对终端io进行控制操作的进程 */
    int ioc_request;            /* 终端io控制的请求代码 */
    vir_bytes ioc_vir_addr;     /* 终端io控制缓冲区的虚拟地址 */

    /* 其他用途 */
    device_func_t ioctl;        /* 设置设备的io速率等等 */
    device_func_t close;                    /* 告诉设备终端已经关闭 */
    void *priv;                             /* 指向每个设备私有数据的指针 */
    Termios termios;               /* POSIX格式的终端控制结构 */
    WinFrame win_frame;                     /* 窗口框架结构,用于描述终端的屏幕信息 */
} TTY;

/* 终端表，存储了系统所有的终端 */
EXTERN TTY tty_table[NR_CONSOLES + NR_RS_LINES + NR_PTYS];

/* events标志值 */
#define EVENTS_READ         1       /* 终端设备需要处理读请求 */
#define EVENTS_WRITE        2       /* 终端设备需要处理写请求 */
#define EVENTS_IOCTL        4       /* 终端设备需要处理终端io控制 */
#define EVENTS_ALL          7       /* 终端设备有多项工作要处理 */

/* 字段值 */
#define NOT_ESCAPED        0	/* 前一个字符不是LNEXT（^V），即转义 */
#define ESCAPED            1	/* 前一个字符是LNEXT（^V） */
#define RUNNING            0	/* no STOP (^S) has been typed to stop output */
#define STOPPED            1	/* STOP (^S) has been typed to stop output */

/* 输入队列中字符上的字段和标志 */
#define IN_CHAR             0x00FF	/* low 8 bits are the character itself */
#define IN_LENGTH           0x0F00	/* length of char if it has been echoed */
#define IN_LSHIFT           8	    /* length = (c & IN_LEN) >> IN_LSHIFT */
#define IN_EOT              0x1000	/* 字符是一个换行符(^D, LF) */
#define IN_EOF              0x2000	/* 字符是一个EOF(^D), 表示文件结束, 不要返回给用户 */
#define IN_ESCAPE           0x4000	/* 由LNEXT（^V）转义，没有解释 */

/* 时间与终端唤醒 */
#define TIME_NEVER	((clock_t) -1 < 0 ? (clock_t) LONG_MAX : (clock_t) -1)
#define tty_wake()	((void) (tty_wake_time = 0))

/* 有闹钟的终端链表，终端结构中讨论过 */
EXTERN TTY *tty_wake_list;

/* 这里定义了两个宏：
 *  1、buffer_len，得到一个缓冲区的长度
 *  2、buffer_end：得到一个缓冲区的界线，这里的数据是空的。
 * 终端任务经常使用这两个宏把数据拷入和拷出缓冲区。
 */
#define buffer_len(buf)	(sizeof(buf) / sizeof((buf)[0]))
#define buffer_end(buf)	((buf) + buffer_len(buf))

#endif //FLYANX_TTY_H
