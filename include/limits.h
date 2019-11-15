/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 这个头文件定义了许多基本的大小值，既有语言中的数据类型，如整数所占的位数，
 * 也有操作系统的限制，如文件名的长度。
 */

#ifndef _LIMITS_H
#define _LIMITS_H

/* 机器字大小（以字节为单位），等于sizeof(int）的常量 */
#if __ACK__     /* 确定是不是Amsterdam Comiler Kit (ACK)编译器 */
#define _WORD_SIZE	_EM_WSIZE
#endif  //__ACK__

#if __GNUC__    /* 确定是不是GNU/GCC编译器 */
#if __i386__    /* 32位机器 */
#define _WORD_SIZE	4
#elif __x86_64__
#define _WORD_SIZE	8   /* 64位机器 */
#endif  // __i386__
#endif  // __GNUC__

/* Definitions about chars (8 bits in MINIX, and signed). */
#define CHAR_BIT           8	/* # bits in a char */
#define CHAR_MIN        -128	/* minimum value of a char */
#define CHAR_MAX         127	/* maximum value of a char */
#define SCHAR_MIN       -128	/* minimum value of a signed char */
#define SCHAR_MAX        127	/* maximum value of a signed char */
#define UCHAR_MAX        255	/* maximum value of an unsigned char */
#define MB_LEN_MAX         1	/* maximum length of a multibyte char */

/* Definitions about shorts (16 bits in MINIX). */
#define SHRT_MIN  (-32767-1)	/* minimum value of a short */
#define SHRT_MAX       32767	/* maximum value of a short */
#define USHRT_MAX     0xFFFF	/* maximum value of unsigned short */

/* 确定INT和UINT的限制 */
#if _WORD_SIZE == 2
#define INT_MIN   (-32767-1)	/* minimum value of a 16-bit int */
#define INT_MAX        32767	/* maximum value of a 16-bit int */
#define UINT_MAX      0xFFFF	/* maximum value of an unsigned 16-bit int */
#endif

#if _WORD_SIZE == 4
#define INT_MIN (-2147483647-1)	/* minimum value of a 32-bit int */
#define INT_MAX   2147483647	/* maximum value of a 32-bit int */
#define UINT_MAX  0xFFFFFFFF	/* maximum value of an unsigned 32-bit int */
#endif

/* 确定long的限制信息(32位flyanx). */
#define LONG_MIN (-2147483647L-1)/* minimum value of a long */
#define LONG_MAX  2147483647L	/* maximum value of a long */
#define ULONG_MAX 0xFFFFFFFFL	/* maximum value of an unsigned long */

/* Minimum sizes required by the POSIX P1003.1 standard (Table 2-3). */
#ifdef _POSIX_SOURCE		/* these are only visible for POSIX */
#define _POSIX_ARG_MAX    4096	/* exec() may have 4K worth of args */
#define _POSIX_CHILD_MAX     6	/* a process may have 6 children */
#define _POSIX_LINK_MAX      8	/* a file may have 8 links */
#define _POSIX_MAX_CANON   255	/* size of the canonical input queue */
#define _POSIX_MAX_INPUT   255	/* you can type 255 chars ahead */
#define _POSIX_NAME_MAX     14	/* a file name may have 14 chars */
#define _POSIX_NGROUPS_MAX   0	/* supplementary group IDs are optional */
#define _POSIX_OPEN_MAX     16	/* a process may have 16 files open */
#define _POSIX_PATH_MAX    255	/* a pathname may contain 255 chars */
#define _POSIX_PIPE_BUF    512	/* pipes writes of 512 bytes must be atomic */
#define _POSIX_STREAM_MAX    8	/* at least 8 FILEs can be open at once */
#define _POSIX_TZNAME_MAX    3	/* time zone names can be at least 3 chars */
#define _POSIX_SSIZE_MAX 32767	/* read() must support 32767 byte reads */

/* Values actually implemented by MINIX (Tables 2-4, 2-5, 2-6, and 2-7). */
/* Some of these old names had better be defined when not POSIX. */
#define _NO_LIMIT        100	/* arbitrary number; limit not enforced */

#define NGROUPS_MAX        0	/* supplemental group IDs not available */
#if _WORD_SIZE > 2
#define ARG_MAX        16384	/* # bytes of args + environ for exec() */
#else
#define ARG_MAX         4096	/* args + environ on small machines ：参数 + 小型计算机上的环境 */
#endif
#define CHILD_MAX  _NO_LIMIT    /* MINIX does not limit children */
#define OPEN_MAX          20	/* # open files a process may have */
#define LINK_MAX         127	/* # links a file may have */
#define MAX_CANON        255	/* size of the canonical input queue */
#define MAX_INPUT        255	/* size of the type-ahead buffer */
#define NAME_MAX          14	/* # chars in a file name */
#define PATH_MAX         255	/* # chars in a path name ：路径名称最大字符数 */
#define PIPE_BUF        7168	/* # bytes in atomic write to a pipe */
#define STREAM_MAX        20	/* must be the same as FOPEN_MAX in stdio.h */
#define TZNAME_MAX         3	/* maximum bytes in a time zone name is 3 */
#define SSIZE_MAX      32767	/* max defined byte count for read() */

#endif /* _POSIX_SOURCE */

#endif //_LIMITS_H
