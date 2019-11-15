/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内核所需的常量定义
 */

#ifndef FLYANX_CONST_H
#define FLYANX_CONST_H

/*
 * 当配置头文件config.h中CHIP是INTEL时生效
 * 这些值用于Intel的CPU芯片,但在别的硬件上编译时则可能不同。
 **/
#if (CHIP == INTEL)

/* 内核栈大小，系统任务将使用这么大的栈空间 */
#define K_STACK_BYTES   1024	/* 内核堆栈有多少字节 */

#define INIT_PSW      0x0202	/* initial psw */
#define INIT_TASK_PSW 0x1202	/* initial psw for tasks (with IOPL 1) */

/* 硬件中断数量 */
#define NR_IRQ_VECTORS      16      /* 中断请求的数量 */
#define	CLOCK_IRQ		    0       /* 时钟中断请求号 */
#define	KEYBOARD_IRQ	    1       /* 键盘中断请求号 */
#define	CASCADE_IRQ		    2	    /* cascade enable for 2nd AT controller */
#define	ETHER_IRQ		    3	    /* default ethernet interrupt vector */
#define	SECONDARY_IRQ	    3	    /* RS232 interrupt vector for port 2 */
#define	RS232_IRQ		    4	    /* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ		    5	    /* xt winchester */
#define	FLOPPY_IRQ		    6	    /* floppy disk */
#define	PRINTER_IRQ		    7
#define	AT_WINI_IRQ		    14	    /* at winchester */

/* 系统调用数量 */
#define NR_SYS_CALL         2

/* 8259A interrupt controller ports. */
#define INT_M_CTL           0x20    /* I/O port for interrupt controller         <Master> */
#define INT_M_CTLMASK       0x21    /* setting bits in this port disables ints   <Master> */
#define INT_S_CTL           0xA0    /* I/O port for second interrupt controller  <Slave>  */
#define INT_S_CTLMASK       0xA1    /* setting bits in this port disables ints   <Slave>  */

/* === 常用的颜色代码  ===  */

/*
 * e.g. MAKE_COLOR(BLUE, RED)
 *      MAKE_COLOR(BLACK, RED) | BRIGHT
 *      MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
 */
#define BLACK   0x0     /* 0000 */
#define WHITE   0x7     /* 0111 */
#define RED     0x4     /* 0100 */
#define GREEN   0x2     /* 0010 */
#define BLUE    0x1     /* 0001 */
#define FLASH   0x80    /* 1000 0000 */
#define BRIGHT  0x08    /* 0000 1000 */
#define MAKE_COLOR(x,y) (x | y) /* MAKE_COLOR(Background,Foreground) */

/* 中断控制器的神奇数字。 */
#define ENABLE          0x20	/* 用于在中断后重新启用中断的代码 */

/* Sizes of memory tables. */
/* 内存表的大小。 */
#define NR_MEMS            8	/* 内存块的数量 */

/* 其他的端口 */
#define PCR		0x65			/* 平面控制寄存器 */
#define PORT_B          0x61	/* 8255端口B的I/O端口(kbd，蜂鸣…) */
#define TIMER0          0x40	/* 定时器通道0的I/O端口 */
#define TIMER2          0x42	/* 定时器通道2的I/O端口 */
#define TIMER_MODE      0x43	/* 用于定时器模式控制的I/O端口 */

/* 固定系统调用向量。 */
#define INT_VECTOR_SYS286_CALL      120	/* flyanx 286系统调用向量 */
#define INT_VECTOR_SYS386_CALL      121	/* flyanx 386系统调用向量 */
#define INT_VECTOR_LEVEL0           37	/* 用于系统任务提权到0的调用向量 */

#endif /* (CHIP == INTEL) */

/*
 * 当配置头文件config.h中CHIP是M68000时生效
 * 这些值用于Motorola 68000的系统,但在别的硬件上编译时则可能不同。
 **/
#if (CHIP == M68000)

#define K_STACK_BYTES   1024	/* how many bytes for the kernel stack */

/* Sizes of memory tables. */
#define NR_MEMS            2	/* number of chunks of memory */

/* p_reg contains: d0-d7, a0-a6,   in that order. */
#define NR_REGS           15	/* number of general regs in each proc slot */

#define TRACEBIT      0x8000	/* or this with psw in proc[] for tracing */
#define SETPSW(rp, new)		/* permits only certain bits to be set */ \
	((rp)->p_reg.psw = (rp)->p_reg.psw & ~0xFF | (new) & 0xFF)

#define MEM_BYTES  0xffffffff	/* memory size for /dev/mem */

#ifdef __ACK__
#define FSTRUCOPY
#endif

#endif /* (CHIP == M68000) */

/*================================================================================================*/
/* 以下的定义是机器无关的，但他们被核心代码的许多部分用到。 */
/*================================================================================================*/
/* 下面定义了MINIX进程调度的三个优先级队列 */
#define TASK_QUEUE             0	/* 就绪的系统任务通过队列0调度 */
#define SERVER_QUEUE           1	/* 就绪的系统服务通过队列1调度 */
#define USER_QUEUE             2	/* 就绪的系统服务通过队列2调度 */
#define NR_PROC_QUEUE          3	/* 调度队列的数量 */

/* 将内核空间中的地址转换为物理地址。这与umap(proc ptr, D, vir, sizeof(*vir))函数
 * 相同，但成本要低得多。
 */
#define	vir2phys(seg_base, vir) (vir_bytes)(((vir_bytes)seg_base) + (vir_bytes)(vir))

#endif //FLYANX_CONST_H
