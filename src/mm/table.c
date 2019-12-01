/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * table.c为所有的全局变量进行声明并为其保留空间
 * 除了本文件中可以看到的变量声明，本文件的编译还会为我们在global.c和可以看到
 * 的各种头文件中用EXTERN声明的变量声明空间。
 */

#define _TABLE

#include "mm.h"
#include <flyanx/callnr.h>
#include <signal.h>     /* 导入它只是因为mmproc.h需要 */
#include "mmproc.h"

char *core_name = "core";   /* 生成核心映像的文件名 */

/* 当一个请求到达时，其中的系统调用号将被取出作为call_vec数组的索引，以找到
 * 处理这个系统调用的过程。不是合法调用的系统调用号都会引起执行no_sys，它只
 * 是返回一个错误代码。
 * 在这里值得注意的是输入_PROTOTYPE宏用在了call_handlers的定义中，这个定义
 * 在这并不是作为一个原型定义，而是一个初始化的数组的定义。但是因为它是一个函
 * 数的数组，使用_PROTOTYPE宏是使程序与经典C(Kernighan & Ritchie)和标准C
 * 都兼容的最简单的办法。
 */
_PROTOTYPE( int (*call_handlers[NR_CALLS]), (void) ) = {
        &no_sys,    /* 0 = 没有使用的调用 */
};

