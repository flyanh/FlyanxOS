/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 关于进程的头文件
 */

#ifndef _PROCESS_H
#define _PROCESS_H

/* 进程，也称进程表项 */
typedef struct process_s {
    Stackframe regs;   /* 进程寄存器保存在堆栈结构中 */

#if (CHIP == INTEL)
    reg_t ldt_selector;		            /* LDT选择子 */
    SegDescriptor ldt[2];	            /* LDT的数据，2是LDT_SIZE，它定义在头文件protect.h中 */
#endif /* (CHIP == INTEL) */

    reg_t *stack_guard_word;        /* 堆栈保护字 */

    int nr;                         /* 进程索引号，主要用于快速访问 */
    MemoryMap map[2];         /* 进程的内存映像，现在包括正文段和数据段（堆栈段） */
    char int_blocked;               /* 被置位，当目标进程有一条中断消息被繁忙的任务堵塞了 */
    char int_held;                  /* 被置位，当目标进程有一条中断消息被繁忙的系统调用挂起保留了 */
    struct process_s *next_held;    /* 被挂起保留的中断过程队列 */

    /* p_flags域中的标志位定义了表项的状态。如果其中任一位被置位,则进程将被堵塞
     * 无法运行。各种标志被定义和描述在后边，如果该表项未被使用,则P_SLOT_FREE被
     * 置位。
     */
    int flags;
    pid_t pid;              /* 进程号，用户可见的 */
    int priority;           /* 权限分为(任务，服务，或用户进程) */

    /* 时间相关 */
    clock_t user_time;          /* 用户时间(以时钟滴答为单位)，即进程使用的时间 */
    clock_t sys_time;           /* 系统时间(以时钟滴答为单位)，即进程调用了系统任务的时间，或者说进程本身就是系统任务 */
    clock_t child_utime;        /* 子进程累积的用户时间 */
    clock_t child_sys_time;     /* 子进程累积的系统时间 */
    clock_t alarm;              /* 进程下一次闹钟(警报)的时间 */

    /* 等待队列的头：当发送消息进程由于目标进程未处在等待状态而无法完成一个SEND操作时,它被送到一个队列中。
     *
     * 简单来说，这个等待队列，是想发消息给我的其他人的一个队列。举个例子，我是女神，每天给我送花可以提高
     * 好感度，但是我一次只能收一朵花，所以追我的人，必须得排成一个队伍，我就可以一个一个收花了。
     */
    struct process_s *caller_head;
    /* 上面所有等待队列的后继节点 */
    struct process_s *caller_link;
    Message *message;       /* 进程的消息缓冲区 */
    int get_form;           /* 当一个进程执行RECEIVE操作,但是没有任何消息在等待它接收时,该进程将阻塞,同时将它期望接收消息的源进程序号保存在p_getfrom中。 */
    int send_to;            /* 消息需要发送给谁？ */

    struct process_s *next_ready;       /* 用于将进程链接在调度程序队列中 */
    /* pending是一个位图,用于记录那些尚未被传送到内存管理器的信号(因为内存管理器没有在等待一条消息)
     * 下边的pend_count域是这些信号的计数值。
     */
    sigset_t pending;
    unsigned pend_count;

    char name[32];          /* 这个没啥好说的，就是进程的名称，记得起个好名字哦 */
} Process;

/* 系统任务堆栈的保护字。 */
#define SYS_TASK_STACK_GUARD	((reg_t) (sizeof(reg_t) == 2 ? 0xBEEF : 0xDEADBEEF))

/* flags域中的标志位状态定义
 *
 * NO_MAP：在执行一个FORK操作后,如果子进程的内存映像尚未建立起来,那么NO_MAP将被置位以阻止子进程运行。
 * SENDING和RECEIVING:表示该进程被阻塞,其原因是它正在试图发送或接收一条消息。
 * PENDING和SIG_PENDING：前者：正在等待一个信号；后者：一个进程正在发送信号
 * P_STOP：在调试期间为跟踪提供支持。
 */
#define NO_MAP		    0x01	/* keeps unmapped forked child from running */
#define SENDING		    0x02	/* set when process blocked trying to send */
#define RECEIVING	    0x04	/* set when process blocked trying to recv */
#define PENDING		    0x08	/* set when inform() of signal pending */
#define SIG_PENDING	    0x10	/* keeps to-be-signalled proc from running */
#define PROC_STOP		0x20	/* set when process is being traced */

#define SYSTEM_PID      -0x3EA  /* 系统任务进程的id */
#define SERVER_PID      -0x328  /* 系统服务器进程的id */


/* 进程权限定义 */
#define PROC_PRI_NONE	0	/* 插槽未使用 */
#define PROC_PRI_TASK	1	/* 部分内核，即系统任务 */
#define PROC_PRI_SERVER	2	/* 内核之外的系统进程 */
#define PROC_PRI_USER	3	/* 用户进程 */
#define PROC_PRI_IDLE	4	/* 空闲进程 */

/* 对过程表地址操作的一些宏定义。 */
#define BEG_PROC_ADDR       (&process[0])
#define END_PROC_ADDR       (&process[NR_TASKS + NR_PROCS])
#define END_TASK_ADDR       (&process[NR_TASKS])
#define BEG_SERVER_ADDR     (&process[NR_TASKS])
#define BEG_USER_PROC_ADDR  (&process[NR_TASKS + LOW_USER])

#define NIL_PROC          ((Process *) 0)   /* 空进程指针 */
#define is_idle_hardware(n) ((n) == IDLE_TASK || (n) == HARDWARE)
#define is_ok_procn(n)      ((unsigned) ((n) + NR_TASKS) < NR_PROCS + NR_TASKS)
#define is_ok_src_dest(n)   (is_ok_procn(n) || (n) == ANY)
#define is_any_hardware(n)   ((n) == ANY || (n) == HARDWARE)
#define is_sys_server(n)      ((n) == FS_PROC_NR || (n) == MM_PROC_NR || (n) == FLY_PROC_NR)      /* 是系统服务？ */
#define is_empty_proc(p)       ((p)->priority == PROC_PRI_NONE)          /* 是个空进程？ */
#define is_task_proc(p)        ((p)->priority == PROC_PRI_TASK)          /* 是个系统任务进程？ */
#define is_serv_proc(p)        ((p)->priority == PROC_PRI_SERVER)        /* 是个系统服务进程？ */
#define is_user_proc(p)        ((p)->priority == PROC_PRI_USER)          /* 是个用户进程？ */

/* 提供宏proc_addr是因为在C语言中下标不能为负数。在逻辑上,数组proc应从 -NR_TASKS到+NR_PROCS(因为用户进程从1开始)。
 * 但在C语言中下标必须从0开始,所以proc[0]指向进程表项下标最小的任务,其他也依次类推。为了更便于记录进程表项与
 * 进程之间的对应关系,我们可以使用
 * proc = proc_addr(n);
 * 将进程n的进程表项地址赋给rp,无论它是正还是负。
 */
#define proc_addr(n)      (p_process_addr + NR_TASKS)[(n)]  /* 得到进程的指针 */
#define cproc_addr(n)     (&(process + NR_TASKS)[(n)])      /* 得到进程的地址 */

/* process.c文件所需要的两个函数，发送消息和接收消息，放在这是因为这两个函数不需要全部人都知道。 */
_PROTOTYPE( int flyanx_send, (struct process_s *caller_ptr, int dest, struct message_s *message_ptr) );
_PROTOTYPE( int flyanx_receive, (struct process_s *caller_ptr, int src, struct message_s *message_ptr) );

EXTERN Process process[NR_TASKS + NR_PROCS];        /* 进程表，EXTERN关键字使其声明内存 */
/* 因为进程表的访问非常频繁,并且计算数组中的一个地址需要用到很慢的乘法操作,
 * 所以使用一个指向进程表项的指针数组p_process_addr来加快操作速度。
 */
EXTERN Process *p_process_addr[NR_TASKS + NR_PROCS];

/* bill_proc指向正在对其CPU使用计费的进程。当一个用户进程调用文件系统,而文件系统正在运行
 * 时,curr_proc(在global.h中)指向文件系统进程,但是bill_proc将指向发出该调用的用户进程。因为文件系统使用的
 * CPU时间被作为调用者的系统时间来计费。
 */
EXTERN Process *bill_proc;

/* 两个数组ready_head和ready_tail用来维护调度队列。例如,ready_head[TASK_Q]指向就绪任务队列中的第一个进程。
 * 就绪进程队列一共分为三个
 * ready_head[TASK_QUEUE]：就绪系统任务队列
 * ready_head[SERVER_QUEUE]：就绪服务进程队列
 * ready_head[USER_QUEUE]：就绪用户进程队列
 * 再举个例子，我们需要拿到用户进程队列的第3个进程，则应该这么拿：ready_head[USER_QUEUE]->next_ready->next_ready，简单吧？
 */
EXTERN Process *ready_head[NR_PROC_QUEUE];
EXTERN Process *ready_tail[NR_PROC_QUEUE];

#endif //_PROCESS_H
