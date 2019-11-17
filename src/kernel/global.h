/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内核所需要的全局变量
 */

#ifndef FLYANX_GLOBAL_H
#define FLYANX_GLOBAL_H

/* 当该文件被包含在定义了宏_TABLE的 table.c中时，宏EXTERN的定义被取消。 */
#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif

/* 内核内存 */
EXTERN phys_bytes code_base;	/* base of kernel code ： 核心代码段基地址 */
EXTERN phys_bytes data_base;	/* base of kernel data ： 核心数据段基地址 */
EXTERN phys_bytes aout;		    /* address of a.out headers ： a.out可执行文件头文件的地址 */

/* GDT 和 IDT 以及显示位置 */
EXTERN u16_t display_position;       /* 256显示模式下，文字显示位置，注意：这不是光标 */
EXTERN u8_t gdt_ptr[6];             /* GDT指针，0~15：Limit 16~47：Base */
EXTERN u8_t idt_ptr[6];             /* IDT指针，同上 */

/* held_head 和 held_tail是指向挂起中断队列的头尾指针 */
EXTERN struct process_s *held_head;
EXTERN struct process_s *held_tail;

EXTERN unsigned char kernel_reenter;	/* 记录内核中断重入的次数 */

extern struct tss_s tss;                            /* 任务状态段 */
EXTERN struct process_s *curr_proc;	                /* 当前运行进程的指针 */

/* 在别处初始化的变量在这里只是extern。 */
extern SegDescriptor gdt[];     /* 全局描述符表 */

extern char *task_stack[];		/* 系统任务栈task_stack，每个任务在task_stack中都有其自己的堆栈 */

/* 其他 */
extern TaskTab tasktab[];   /* 系统任务表 */
EXTERN unsigned lost_ticks;	/* 时钟滴答在时钟任务之外的计数 */

/* 机器状态 */
EXTERN int pc_at;		/* PC-AT兼容硬件接口 */
EXTERN int ps_mca;		/* PS/2与微通道总线 */
EXTERN unsigned int processor;	/* 标识CPU类别，86,186,286,386... */
#if _WORD_SIZE == 2
EXTERN int protected_mode;	/* 如果以Intel保护模式运行，则为非零 */
#else
#define protected_mode	1	/* 386模式暗含保护模式 */
#endif

/* 其他 */
EXTERN irq_handler_t  int_request_table[NR_IRQ_VECTORS];    /* 中断请求处理程序表 */
EXTERN int int_request_used;                                /* 中断请求处理启用位图 */

EXTERN flyanx_syscall_t level0_func;            /* 提权函数：系统任务提权调用将会把提权的函数地址放在这 */


#endif // FLYANX_GLOBAL_H
