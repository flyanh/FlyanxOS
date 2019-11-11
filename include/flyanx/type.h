/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含了许多重要的类型定义以及相关的数量值
 * 该文件最重要的定义是message，其定义了以消息传递的系统体系中最重要的消息结构体。
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

#endif //_FLYANX_TYPE_H
