/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 定义了Flyanx使用的数据类型
 *
 * 使用这里提供的定义可以避免对特定情况下所使用的基本数据结构的理解错误而导致的故障。
 *
 * 注意：所有的类型名都以“_t”结尾,这不仅仅是一种习惯,而是POSIX
 * 标准的规定。这是“保留后缀”的一个例子,而且“_t”不用于非数据类型的其他任何符号名。
 */

#ifndef _FLYANX_TYPES_H
#define _FLYANX_TYPES_H

#ifndef _ANSI_H
#include <ansi.h>
#endif

/* 类型size_t包含sizeof操作符的所有结果。乍一看，似乎很明显它应该是无符号整数，但情况
 * 并不总是如此。例如，有些时候(例如在68000处理器的机器上)有32位指针和16位整数。当要求
 * 70K结构或数组的大小时，结果需要表示17位，因此size_t必须是长类型。类型ssize_t是
 * size_t的有符号版本。
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long time_t;		   /* 时间自1970年1月1日格林尼治时间0000 */
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef long clock_t;		   /* 系统时钟计时单位 */
#endif

#ifndef _SIGSET_T
#define _SIGSET_T
typedef unsigned long sigset_t;     /* 信号集 */
#endif

/* 以下类型在磁盘使用，索引节点等数据结构。 */
typedef short          dev_t;	   /* 持有（主|次）设备对 */
typedef char           gid_t;	   /* 组号 */
typedef unsigned short ino_t; 	   /* 索引节点数 */
typedef unsigned short mode_t;	   /* 文件类型和权限位 */
typedef char         nlink_t;	   /* 指向文件的链接数 */
typedef unsigned long  off_t;	   /* 文件内的偏移量 */
typedef int            pid_t;	   /* 进程号（必须是有符号） */
typedef short          uid_t;	   /* 用户号 */
typedef unsigned long zone_t;	   /* 区段号 */
typedef unsigned long block_t;	   /* 块号 */
typedef unsigned long  bit_t;	   /* 位图中的位数 */
typedef unsigned short zone1_t;	   /* V1文件系统的区域号 */
typedef unsigned short bitchunk_t; /* 位图中的位集合 */

typedef unsigned char   u8_t;	   /* 8位类型 == db */
typedef unsigned short u16_t;	   /* 16位类型 == dw */
typedef unsigned long  u32_t;	   /* 32位类型 == dd */

typedef char            i8_t;      /* 8位有符号类型 */
typedef short          i16_t;      /* 16位有符号类型 */
typedef long           i32_t;      /* 32位有符号类型 */

typedef struct { u32_t _[2]; } u64_t;

/* The following types are needed because MINIX uses K&R style function
 * definitions (for maximum portability).  When a short, such as dev_t, is
 * passed to a function with a K&R definition, the compiler automatically
 * promotes it to an int.  The prototype must contain an int as the parameter,
 * not a short, because an int is what an old-style function definition
 * expects.  Thus using dev_t in a prototype would be incorrect.  It would be
 * sufficient to just use int instead of dev_t in the prototypes, but Dev_t
 * is clearer.
 */
typedef int            Dev_t;
typedef int 	       Gid_t;
typedef int 	     Nlink_t;
typedef int 	       Uid_t;
typedef int             U8_t;
typedef unsigned long  U32_t;
typedef int             I8_t;
typedef int            I16_t;
typedef long           I32_t;

/* ANSI C makes writing down the promotion of unsigned types very messy.  When
 * sizeof(short) == sizeof(int), there is no promotion, so the type stays
 * unsigned.  When the compiler is not ANSI, there is usually no loss of
 * unsignedness, and there are usually no prototypes so the promoted type
 * doesn't matter.  The use of types like Ino_t is an attempt to use ints
 * (which are not promoted) while providing information to the reader.
 */

#if _EM_WSIZE == 2
typedef unsigned int        Ino_t;
typedef unsigned int        Zone1_t;
typedef unsigned int        Bitchunk_t;
typedef unsigned int        U16_t;
typedef unsigned int        Mode_t;

#else /* _EM_WSIZE == 4, or _EM_WSIZE undefined */
typedef int	            Ino_t;
typedef int 	        Zone1_t;
typedef int	            Bitchunk_t;
typedef int	            U16_t;
typedef int             Mode_t;

#endif /* _EM_WSIZE == 2, etc */

/* 信号处理程序类型，例如SIG_IGN */
typedef void _PROTOTYPE( (*sighandler_t), (int) );

#endif //_FLYANX_TYPES_H
