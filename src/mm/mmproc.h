/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/1.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内存管理器的进程表
 * 因为内存管理器以2特权级运行在内核上，所以它必须也要知道其他的用户进程
 * 才能对他们进行管理，但是这里的进程表和内核的有些不同，内存管理器只想要
 * 知道自己想知道的进程信息，其它的都不会声明。
 */
#ifndef _MM_MMPROC_H
#define _MM_MMPROC_H

typedef struct mm_process_s {
    MemoryMap map;          /* 参考内核中的进程结构map */
    char exit_status;       /* 退出状态：在进程已经结束而父进程还没有执行对它的WAIT时的终止状态。 */
    pid_t pid;           /* 进程号 */
    pid_t proc_group;       /* 进程组长的pid（用于信号） */
    pid_t wiat_pid;         /* 正在等待的进程的进程号 */
    pid_t parent;           /* 父进程的进程号 */

    /* 真实/有效的用户号和组号 */
    uid_t real_uid;		        /* 真实用户号 */
    uid_t effective_uid;		/* 有效用户号 */
    gid_t real_gid;		        /* 真实组号 */
    gid_t effective_gid;		/* 有效组号 */

    /* 信号处理相关@TODO */

    unsigned flags;                     /* 标志 */
    vir_bytes proc_args;                /* 初始堆栈参数指针 */
    struct mm_process_s *swap_queue;    /* 等待被换入的进程队列 */
    Message reply;                      /* 存放进程要回复的消息 */
} MMProcess;
EXTERN MMProcess mmproc[NR_PROCS];      /* 进程表 */

/* 标志值 */
#define IN_USE              0x001	/* 当进程插槽在使用时被设置 */
#define WAITING             0x002	/* 当WAIT（等待）系统调用时被设置 */
#define ZOMBIE              0x004	/* 当EXIT时被置位，当WAIT时被复位清除。 */
#define PAUSED              0x008	/* set by PAUSE system call */
#define ALARM_ON            0x010	/* set when SIGALRM timer started */
#define	TRACED		        0x040	/* set if process is to be traced */
#define STOPPED		        0x080	/* set if process stopped for tracing */
#define SIG_SUSPENDED 	    0x100	/* 系统调用SIGSUSPEND */
#define REPLY	 	        0x200	/* set if a reply message is pending */
#define ON_SWAP	 	        0x400	/* 数据段被换出 */
#define SWAP_IN	 	        0x800	/* set if on the "swap this in" queue */

#define NIL_MPROC ((struct mproc *) 0)

#endif//_MM_MMPROC_H
