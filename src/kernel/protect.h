/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件的内容主要是为了支持保护模式的Intel处理器(80286,80386、80486、奔腾、高能奔腾)
 * 体系结构的细节相关。
 *
 * 处理器的保护模式提供了四种特权级，0~4，0特权最大。
 * Flyanx使用了三个特权级(0、1和3)，和Minix一样，而Linux和Windows则只使用了两种（0、3）。
 */

#ifndef FLYANX_PROTECT_H
#define FLYANX_PROTECT_H

/*================================================================================================*/
/* 描述符表项结构 */
/*================================================================================================*/
typedef struct descriptor_s {
    char limit[sizeof(u16_t)];
    char base[sizeof(u32_t)];		/* really u24_t + pad for 286 */
} Descriptor;

/*================================================================================================*/
/* 门描述符 */
/*================================================================================================*/
typedef struct gate_s
{
    u16_t	offset_low;	/* Offset Low */
    u16_t	selector;	/* Selector */
    u8_t	dcount;		    /* 该字段只在调用门描述符中有效。
                               如果在利用调用门调用子程序时引起特权级的转换和堆栈的改变，需要将外层堆栈中的参数复制到内层堆栈。
				               该双字计数字段就是用于说明这种情况发生时，要复制的双字参数的数量。 */

    u8_t	attr;		    /* P(1) DPL(2) DT(1) TYPE(4) */
    u16_t	offset_high;	/* Offset High */
} Gate;

/*================================================================================================*/
/* 任务状态段 */
/*================================================================================================*/
typedef struct tss_s
{
    reg_t   backlink;
    reg_t	esp0;	    /* stack pointer to use during interrupt */
    reg_t	ss0;	    /*   "   segment  "  "    "        "     */
    reg_t	esp1;
    reg_t	ss1;
    reg_t	esp2;
    reg_t	ss2;
#if _WORD_SIZE == 4
    reg_t   cr3;
#endif
    reg_t	eip;
    reg_t	flags;
    reg_t	eax;
    reg_t	ecx;
    reg_t	edx;
    reg_t	ebx;
    reg_t	esp;
    reg_t	ebp;
    reg_t	esi;
    reg_t	edi;
    reg_t	es;
    reg_t	cs;
    reg_t	ss;
    reg_t	ds;
#if _WORD_SIZE == 4
    reg_t   fs;
    reg_t   gs;
#endif
    reg_t	ldt;
#if _WORD_SIZE == 4
    u16_t trap;
    u16_t iobase;     /* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
/* u8_t iomap[0]; */
#endif
} Tss;

/*================================================================================================*/
/* 表大小 */
/*================================================================================================*/
#define GDT_SIZE (LDT_FIRST_INDEX + NR_TASKS + NR_PROCS) /* 全局描述符表 */
#define IDT_SIZE (INT_VECTOR_IRQ8 + 8)	    /* 只取最高的向量 */
#define LDT_SIZE         4	                /* 包含CS, DS和两个额外的；否则应为2 */

/*================================================================================================*/
/* 固定的全局描述符。 */
/*================================================================================================*/
#define DUMMY_INDEX         0   /* GDT的头，无用 */
#define FLAT_C_INDEX        1   /* 0~4G，32位可读代码段 */
#define FLAT_RW_INDEX       2   /* 0~4G，32位可读写数据段 */
#define VIDEO_INDEX         3   /* 显存首地址，特权级3 */

#define TSS_INDEX           4   /* 任务状态段 */
#define LDT_FIRST_INDEX     5   /* 本地描述符 */
/*================================================================================================*/
/* 接下来是选择子，选择子 = (描述符索引 * 描述符大小) */
/*================================================================================================*/
#define SELECTOR_DUMMY      DUMMY_INDEX * DESCRIPTOR_SIZE
#define SELECTOR_FLAT_C     FLAT_C_INDEX * DESCRIPTOR_SIZE
#define SELECTOR_FLAT_RW    FLAT_RW_INDEX * DESCRIPTOR_SIZE
#define SELECTOR_VIDEO      VIDEO_INDEX * DESCRIPTOR_SIZE + 3       /* 加3是因为这个段的 DPL(特权级) = 3 */

#define SELECTOR_TSS        TSS_INDEX * DESCRIPTOR_SIZE
#define SELECTOR_LDT_FIRST  LDT_FIRST_INDEX * DESCRIPTOR_SIZE

#define SELECTOR_KERNEL_CS  SELECTOR_FLAT_C		/* 0~4G，32位可读代码段   */
#define SELECTOR_KERNEL_DS  SELECTOR_FLAT_RW	/* 0~4G，32位可读写数据段 */
#define SELECTOR_KERNEL_GS  SELECTOR_VIDEO		/* 显存首地址，特权级3 */

/*================================================================================================*/
/* 固定的局部描述符 */
/*================================================================================================*/
#define CS_LDT_INDEX     0	/* 进程的代码段 */
#define DS_LDT_INDEX     1	/* 进程数据段=ES=FS=GS=SS */
#define EXTRA_LDT_INDEX  2	/* 首先是额外的LDT条目 */

/*================================================================================================*/
/*
 * Flyanx使用的三种CPU特权级
 *
 * 核心的最中心部分,即运行于中断处理期间的部分和切换进程的部分运行在INTR_PRIVILEGE特权级。
 * 在该特权级上,进程可以访问全部的内存空间和CPU的全部寄存器。
 *
 * 系统任务运行在TASK_PRIVILEGE特权级。该特权级允许它们访问I/O,但不能使用那些修改特殊寄存器(如
 * 指向描述符表的寄存器)值的指令。
 *
 * 服务器进程运行在SERVER_PRIVILEGE特权级。运行在该特权级的进程
 * 不能执行某些指令,如访问I/O端口、改变内存分配状况,或改变处理器运行级别等等。
 *
 * 用户进程运行在USER_PRIVILEGE特权级。运行在该特权级的进程限制和SERVER_PRIVILEGE
 * 一样。
 */
/*================================================================================================*/
#define KERNEL_PRIVILEGE    0	/* 内核和中断处理程序 */
#define TASK_PRIVILEGE      1
#define SERVER_PRIVILEGE    2
#define USER_PRIVILEGE      3

/*================================================================================================*/
/* 选择子位 */
/*================================================================================================*/
#define TI            0x04	/* 表指示器 */
#define RPL           0x03	/* 请求者的特权级别 */

/*================================================================================================*/
/* 选择子类型值说明，其中　SA_ : Selector Attribute 选择子属性 */
/*================================================================================================*/
#define SA_RPL_MASK         0xFFFC
#define SA_RPL0             0
#define SA_RPL1             1
#define SA_RPL2             2
#define SA_RPL3             3

#define SA_TI_MASK          0xFFFB
#define SA_TIG              0
#define SA_TIL              4

/*================================================================================================*/
/* 描述符类型值说明，其中　DA_ : Descriptor Attribute 描述符属性 */
/*================================================================================================*/
#define	DA_32			    0x4000	/* 32 位段			     */
#define	DA_LIMIT_4K		    0x8000	/* 段界限粒度为 4K 字节	   */
#define	DA_DPL0			    0x00	/* DPL = 0	内核级		    */
#define	DA_DPL1			    0x20	/* DPL = 1				*/
#define	DA_DPL2			    0x40	/* DPL = 2				*/
#define	DA_DPL3			    0x60	/* DPL = 3	用户级			*/

/* 存储段描述符类型值说明 */
#define	DA_DR			    0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			    0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			    0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			    0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			    0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			    0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			    0x9E	/* 存在的可执行可读一致代码段属性值	*/

/* 系统段描述符类型值说明 */
#define	DA_LDT			    0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		    0x85	/* 任务门类型值				*/
#define	DA_386TSS		    0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		    0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		    0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		    0x8F	/* 386 陷阱门类型值			*/

#define DESCRIPTOR_SIZE             8   /* 描述符大小 */

/* 异常中断向量 */
#define	INT_VECTOR_DIVIDE		    0x0
#define	INT_VECTOR_DEBUG		    0x1
#define	INT_VECTOR_NMI			    0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW		    0x4
#define	INT_VECTOR_BOUNDS		    0x5
#define	INT_VECTOR_INVAL_OP		    0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT		    0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10

#define	INT_VECTOR_IRQ0				0x20
#define	INT_VECTOR_IRQ8				0x28

#endif //FLYANX_PROTECT_H
