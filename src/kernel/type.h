/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内核所需的类型定义
 */

#ifndef FLYANX_TYPE_H
#define FLYANX_TYPE_H

#if (CHIP == INTEL)
/*
 * 端口数据类型，用于访问I/O端口
 */
typedef unsigned port_t;
/*
 * 寄存器数据类型，用于访问存储器段和CPU寄存器
 */
typedef unsigned reg_t;		/* machine register */

/* stackframe_s定义了如何将寄存器值保存到堆栈上的数据结构。
 * 这个结构非常重要-在进程被投入运行状态或被脱开运行状态时,它被用来保
 * 存和恢复CPU的内部状态。将其定义成可以用汇编语言高速读写的格式,这将减少进程上下文切换的时间。
 */
typedef struct stackframe_s {           /* proc_ptr points here */
#if _WORD_SIZE == 4
    u16_t gs;                     /* last item pushed by save */
  u16_t fs;                     /*  ^ */
#endif
    u16_t es;                     /*  | */
    u16_t ds;                     /*  | */
    reg_t di;			/* di through cx are not accessed in C */
    reg_t si;			/* order is to match pusha/popa */
    reg_t fp;			/* bp */
    reg_t st;			/* hole for another copy of sp */
    reg_t bx;                     /*  | */
    reg_t dx;                     /*  | */
    reg_t cx;                     /*  | */
    reg_t retreg;			/* ax and above are all pushed by save ：返回地址所在的段 */
    reg_t retadr;			/* return address for assembly code save() */
    reg_t pc;			/*  ^  last item pushed by interrupt ：由中断推送的最后一个项目 */
    reg_t cs;                     /*  | */
    reg_t psw;                    /*  | */
    reg_t sp;                     /*  | */
    reg_t ss;                     /* these are pushed by CPU during interrupt */
} Stackframe;

/*
 * segdesc_s是与Intel处理器结构相关的另一个结构,它是保证进程不会发生内存访问越限机制的一部分。
 */
typedef struct seg_descriptor_s {		    /* 受保护模式的段描述符 */
    u16_t limit_low;
    u16_t base_low;
    u8_t base_middle;
    u8_t access;			  /* | P | DL | 1 | X | E | R | A | */
    u8_t granularity;		/* | G | X  | 0 | A |   LIMT    | */
    u8_t base_high;
} SegDescriptor;

/* 中断处理函数原型 */
typedef _PROTOTYPE( void (*int_handler_t), (void) );
/* 中断请求处理函数原型 */
typedef _PROTOTYPE( int (*irq_handler_t), (int irq) );

/* 系统调用函数原型 */
typedef _PROTOTYPE( void* flyanx_syscall_t,  );

#endif /* (CHIP == INTEL) */

/*
 * Motorola 68000系列处理器时的类型定义
 * Flyanx0.1只实现INTEL
 */
#if (CHIP == M68000)
typedef _PROTOTYPE( void (*dmaint_t), (void) );

typedef u32_t reg_t;		/* machine register */

/* The name and fields of this struct were chosen for PC compatibility. */
struct stackframe_s {
  reg_t retreg;			/* d0 */
  reg_t d1;
  reg_t d2;
  reg_t d3;
  reg_t d4;
  reg_t d5;
  reg_t d6;
  reg_t d7;
  reg_t a0;
  reg_t a1;
  reg_t a2;
  reg_t a3;
  reg_t a4;
  reg_t a5;
  reg_t fp;			/* also known as a6 */
  reg_t sp;			/* also known as a7 */
  reg_t pc;
  u16_t psw;
  u16_t dummy;			/* make size multiple of reg_t for system.c */
};

struct fsave {
  struct cpu_state {
	u16_t i_format;
	u32_t i_addr;
	u16_t i_state[4];
  } cpu_state;
  struct state_frame {
	u8_t frame_type;
	u8_t frame_size;
	u16_t reserved;
	u8_t frame[212];
  } state_frame;
  struct fpp_model {
	u32_t fpcr;
	u32_t fpsr;
	u32_t fpiar;
	struct fpN {
		u32_t high;
		u32_t low;
		u32_t mid;
	} fpN[8];
  } fpp_model;
};
#endif /* (CHIP == M68000) */

#endif //FLYANX_TYPE_H
