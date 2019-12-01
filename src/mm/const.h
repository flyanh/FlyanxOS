/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内存管理器所需要的常量
 */
#ifndef _MM_CONST_H
#define _MM_CONST_H

/* 由alloc_mem()函数返回，用于告诉调用者，这块内存已经被分配了 */
#define NO_MEM  ((phys_clicks) 0)

/* 起源进程的进程号 */
#define ORIGIN_PID      0

#endif //_MM_CONST_H
