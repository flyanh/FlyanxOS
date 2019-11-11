/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 关于进程的头文件
 */

#ifndef FLYANX_PROCESS_H
#define FLYANX_PROCESS_H

/* 进程，也称进程表项 */
typedef struct process_t {
    Stackframe reg;   /* 进程寄存器保存在堆栈结构中 */

#if (CHIP == INTEL)
    reg_t ldt_selector;		        /* LDT选择子 */
    SegDescriptor ldt[4];	            /* LDT的数据 */
    /* 4 is LDT_SIZE - avoid include protect.h */
#endif /* (CHIP == INTEL) */

    reg_t *stack_guard_word;      /* 堆栈保护字 */

    int nr;                         /* 进程索引号（在表中的索引），主要用于快速访问 */

    /* p_flags域中的标志位定义了表项的状态。如果其中任一位被置位,则进程将无法运行。
     * 各种标志被定义和描述在本文件后边，如果该表项未被使用,则P_SLOT_FREE被置位。
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

    struct process_t *caller_queue;     /* 指向一个队列：当发送消息进程由于目标进程未处在等待状态而无法完成一个SEND操作时,它被送到一个队列中。 */
    struct process_t *send_link;        /* 当目标进程最终执行RECEIVE操作时,它很容易找到等待向其发送消息的所有进程。p_sendlink域用于将队列中的成员链接起来。 */
    int get_form;           /* 当一个进程执行RECEIVE操作,但是没有任何消息在等待它接收时,该进程将阻塞,同时将它期望接收消息的源进程序号保存在p_getfrom中。 */
    int send_to;            /* 消息需要发送给谁？ */

    struct process_t *next_ready;       /* 用于将进程链接在调度程序队列中 */
    /* pending是一个位图,用于记录那些尚未被传送到内存管理器的信号(因为内存管理器没有在等待一条消息)
     * 下边的pend_count域是这些信号的计数值。
     */
    sigset_t pending;
    unsigned pend_count;

    char name[17];          /* 这个没啥好说的，就是进程的名称，记得起个好名字哦 */
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
#define NO_MAP		0x01	/* keeps unmapped forked child from running */
#define SENDING		0x02	/* set when process blocked trying to send */
#define RECEIVING	0x04	/* set when process blocked trying to recv */
#define PENDING		0x08	/* set when inform() of signal pending */
#define SIG_PENDING	0x10	/* keeps to-be-signalled proc from running */
#define P_STOP		0x20	/* set when process is being traced */

/* 进程权限定义 */
#define PROC_PRI_NONE	0	/* 插槽未使用 */
#define PROC_PRI_TASK	1	/* 部分内核，即系统任务 */
#define PROC_PRI_SERVER	2	/* 内核之外的系统进程 */
#define PROC_PRI_USER	3	/* 用户进程 */
#define PROC_PRI_IDLE	4	/* 空闲进程 */
/* 对过程表地址操作的一些宏定义。 */
#define BEG_PROC_ADDR (p_process_addr[0])
#define END_PROC_ADDR (p_process_addr[NR_TASKS + NR_PROCS])
#define END_TASK_ADDR (p_process_addr[NR_TASKS])
#define BEG_SERV_ADDR (p_process_addr[NR_TASKS])
#define BEG_USER_ADDR (p_process_addr[NR_TASKS + LOW_USER])

#define NIL_PROC          ((Process *) 0)   /* 空进程指针 */
#define isidlehardware(n) ((n) == IDLE || (n) == HARDWARE)
#define isokprocn(n)      ((unsigned) ((n) + NR_TASKS) < NR_PROCS + NR_TASKS)
#define isoksrc_dest(n)   (isokprocn(n) || (n) == ANY)
#define isrxhardware(n)   ((n) == ANY || (n) == HARDWARE)
#define issysentn(n)      ((n) == FS_PROC_NR || (n) == MM_PROC_NR)  /*  */
#define is_empty_proc(p)       ((p)->priority == PROC_PRI_NONE)          /* 是个空进程？ */
#define is_task_proc(p)        ((p)->priority == PROC_PRI_TASK)          /* 是个系统任务进程？ */
#define is_serv_proc(p)        ((p)->priority == PROC_PRI_SERVER)        /* 是个系统服务进程？ */
#define is_user_proc(p)        ((p)->priority == PROC_PRI_USER)          /* 是个用户进程？ */

EXTERN Process process[NR_TASKS + NR_PROCS];        /* 进程表，EXTERN关键字使其声明内存 */
/* 因为进程表的访问非常频繁,并且计算数组中的一个地址需要用到很慢的乘法操作,
 * 所以使用一个指向进程表项的指针数组p_process_addr来加快操作速度。
 */
EXTERN Process *p_process_addr[NR_TASKS + NR_PROCS];

/* bill_ptr指向正在对其CPU使用计费的进程。当一个用户进程调用文件系统,而文件系统正在运行
 * 时,proc_ptr(在gloal.h中)指向文件系统进程,但是bill_ptr将指向发出该调用的用户进程。因为文件系统使用的
 * CPU时间被作为调用者的系统时间来计费。
 */
EXTERN Process *bill_ptr;

/* 两个数组rdy_head和rdy_tail用来维护调度队列。例如,rdy_head[TASK_Q]指向任务队列中的第一个进程。
 * 任务队列一共分为三个
 * rdy_head[TASK_Q]：系统任务队列
 * rdy_head[SERVER_Q]：服务进程队列
 * rdy_head[USER_Q]：用户进程队列
 * 再举个例子，我们需要拿到用户进程队列的第3个进程，则应该这么拿：rdy_head[USER_Q]->p_nextready->p_nextready，简单吧？
 */
EXTERN Process *ready_head[NQ];
EXTERN Process *ready_tail[NQ];

#endif //FLYANX_PROCESS_H
