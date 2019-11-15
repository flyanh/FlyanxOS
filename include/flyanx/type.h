/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含了许多重要的类型定义以及相关的数量值
 * 该文件最重要的定义是Message，其定义了以消息传递的系统体系中最重要的
 * 消息结构体。
 */

#ifndef _FLYANX_TYPE_H
#define _FLYANX_TYPE_H

#ifndef _FLYANX_TYPES_H
#include <sys/types.h>
#endif

/* 虚拟内存块，一个块在INTEL的MINIX中是256个字节 */
typedef unsigned int vir_clicks;
/* 物理地址（字节长度） */
typedef unsigned long phys_bytes;
/* 物理内存块 */
typedef unsigned int phys_clicks;

#if (CHIP == INTEL)
/* 虚拟地址（字节长度） */
typedef unsigned int vir_bytes;
#endif

#if (CHIP == M68000)
typedef unsigned long vir_bytes;/* virtual addresses and lengths in bytes */
#endif

#if (CHIP == SPARC)
typedef unsigned long vir_bytes;/* virtual addresses and lengths in bytes */
#endif

/* 与消息有关的类型。 */
#define M1                 1
#define M3                 3
#define M4                 4
#define M3_STRING         14

/* 定义6种消息域将使得更易于在不同的体系结构上重新编译。 */
typedef struct {int m1i1, m1i2, m1i3; char *m1p1, *m1p2, *m1p3;} mess_union1;
typedef struct {int m2i1, m2i2, m2i3; long m2l1, m2l2; char *m2p1;} mess_union2;
typedef struct {int m3i1, m3i2; char *m3p1; char m3ca1[M3_STRING];} mess_union3;
typedef struct {long m4l1, m4l2, m4l3, m4l4, m4l5;} mess_union4;
typedef struct {char m5c1, m5c2; int m5i1, m5i2; long m5l1, m5l2, m5l3;}mess_union5;
typedef struct {int m6i1, m6i2, m6i3; long m6l1; sighandler_t m6f1;} mess_union6;

/* 消息，MINIX中的进程通信的根本，同时也是客户端和服务端通信的根本 */
typedef struct message_s{
    int source;         /* 谁发送的消息 */
    int type;           /* 消息的类型（例如，发给时钟任务的GET_TIME） */
    union {             /* 消息域，一共可以是六种消息域类型之一 */
        mess_union1 u1;
        mess_union2 u2;
        mess_union3 u3;
        mess_union4 u4;
        mess_union5 u5;
        mess_union6 u6;
    } u;
} Message;

#endif //_FLYANX_TYPE_H
