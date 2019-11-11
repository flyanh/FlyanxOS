/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含flyanx的编译首选项，同时也是编译器实际上处理的第一个文件。
 *
 * flyanx暂时只支持32位有保护模式的机器，我们怎么知道这个呢？我们通过gcc编译器
 * 提供的宏，如果是16位机器，gcc会定义一个__8086__宏，而如果是32位机器，则会定义一个
 * __i386__宏(64为__x86_64__)，我们通过这个宏确定了_WORD_SIZE。我们也支持Amsterdam Comiler
 * Kit (ACK)编译器，但我们在这将不再细讲。
 * 16位机器_WORD_SIZE设置为2；而32_WORD_SIZE则为4；
 * 虽然我们暂未支持其他，但可以识别他们Flyanx以后的扩展性会更好。
 */
#ifndef FLYANX_CONFIG_H
#define FLYANX_CONFIG_H

/* Flyanx发行版和版本号。 */
#define OS_RELEASE "0"
#define OS_VERSION "0.1"

/*===========================================================================*
 *		本节包含用户可设置的参数		     *
 *===========================================================================*/
#define MACHINE        		IBM_PC	/* Must be one of the names listed below */

#define IBM_PC				1		/* any  8088 or 80x86-based system */
#define SUN_4             	40		/* any Sun SPARC-based system */
#define SUN_4_60	  		40		/* Sun-4/60 (aka SparcStation 1 or Campus) */
#define ATARI             	60		/* ATARI本节包含用户可设置的参数 ST/STe/TT (68000/68030) */
#define AMIGA             	61		/* Commodore Amiga (68000) */
#define MACINTOSH         	62		/* Apple Macintosh (68000) */

/* 机器字大小（以字节为单位），等于sizeof(int）的常量 */
#if __ACK__     /* 确定是不是Amsterdam Comiler Kit (ACK)编译器 */
#define _WORD_SIZE	_EM_WSIZE
#endif

#if __i386__    /* gcc编译器 */
#define _WORD_SIZE      4       /* 32位机器 */
#elif __x86_64__
#define _WORD_SIZE      8       /* 64位机器 */
#endif


/* 进程表中的用户进程的槽数。 */
//#define NR_PROCS          32
#define NR_PROCS          0

/* 缓冲区高速缓存应尽可能地大。 */
#if (MACHINE == IBM_PC && _WORD_SIZE == 2)
#define NR_BUFS           40	/* # blocks in the buffer cache */
#define NR_BUF_HASH       64	/* size of buf hash table; MUST BE POWER OF 2*/
#endif

#if (MACHINE == IBM_PC && _WORD_SIZE == 4)
#define NR_BUFS           80	/* # blocks in the buffer cache */
#define NR_BUF_HASH      128	/* size of buf hash table; MUST BE POWER OF 2*/
#endif

#if (MACHINE == SUN_4_60)
#define NR_BUFS		 512	/* # blocks in the buffer cache (<=1536) */
#define NR_BUF_HASH	 512	/* size of buf hash table; MUST BE POWER OF 2*/
#endif

/* minix所启用的控制台的数量计数定义
 */
#define NR_CONS           	2	/* # system consoles (1 to 8) ： 系统控制台(1 ~ 7) */
#define	NR_RS_LINES	   		0	/* # rs232 terminals (0, 1, or 2) ： rs232终端(0 ~ 2) */
#define	NR_PTYS		  	 	0	/* # pseudo terminals (0 to 64) ： 伪终端(0 ~ 64) */

/*===========================================================================*
 *	在这一行之后没有用户可设置的参数		     *
 *===========================================================================*/
/* Set the CHIP type based on the machine selected. The symbol CHIP is actually
 * indicative of more than just the CPU.  For example, machines for which
 * CHIP == INTEL are expected to have 8259A interrrupt controllers and the
 * other properties of IBM PC/XT/AT/386 types machines in general. */
#define INTEL             1	/* CHIP type for PC, XT, AT, 386 and clones */
#define M68000            2	/* CHIP type for Atari, Amiga, Macintosh    */
#define SPARC             3	/* CHIP type for SUN-4 (e.g. SPARCstation)  */

/* Set the FP_FORMAT type based on the machine selected, either hw or sw    */
#define FP_NONE		  0	/* no floating point support                */
#define FP_IEEE		  1	/* conform IEEE floating point standard     */

#if (MACHINE == IBM_PC)
#define CHIP          INTEL
#endif

#if (MACHINE == ATARI) || (MACHINE == AMIGA) || (MACHINE == MACINTOSH)
#define CHIP         M68000
#endif

#if (MACHINE == SUN_4) || (MACHINE == SUN_4_60)
#define CHIP          SPARC
#define FP_FORMAT   FP_IEEE
#endif

#if (MACHINE == ATARI) || (MACHINE == SUN_4)
#define ASKDEV            1	/* ask for boot device */
#define FASTLOAD          1	/* use multiple block transfers to init ram */
#endif

#if (ATARI_TYPE == TT) /* and all other 68030's */
#define FPP
#endif

#ifndef FP_FORMAT
#define FP_FORMAT   FP_NONE
#endif

#ifndef MACHINE
error "In <minix/config.h> please define MACHINE"
#endif

#ifndef CHIP
error "In <minix/config.h> please define MACHINE to have a legal value"
#endif

#if (MACHINE == 0)
error "MACHINE has incorrect value (0)"
#endif

#endif //FLYANX_CONFIG_H
