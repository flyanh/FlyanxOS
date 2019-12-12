/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/1.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * table.c为所有的全局变量进行声明并为其保留空间
 * 除了本文件中可以看到的变量声明，本文件的编译还会为我们在global.c和可以看到
 * 的各种头文件中用EXTERN声明的变量声明空间。
 */

#define _TABLE

#include "fly.h"
#include <flyanx/callnr.h>

/* 当一个请求到达时，其中的系统调用号将被取出作为call_vec数组的索引，以找到
 * 处理这个系统调用的过程。不是合法调用的系统调用号都会引起执行fly_no_sys，它只
 * 是返回一个错误代码。
 * 在这里值得注意的是输入_PROTOTYPE宏用在了call_handlers的定义中，这个定义
 * 在这并不是作为一个原型定义，而是一个初始化的数组的定义。但是因为它是一个函
 * 数的数组，使用_PROTOTYPE宏是使程序与经典C(Kernighan & Ritchie)和标准C
 * 都兼容的最简单的办法。
 */
_PROTOTYPE( int (*fly_call_handlers[NR_CALLS]), (void) ) = {
        &fly_no_sys,     /* 0 = 没有使用的调用 */
        &fly_no_sys,    /* 1 = exit::退出一个进程 */
        &fly_no_sys,       /* 2 = wait:: */
        &fly_no_sys,       /* 3 = fork::派生一个新进程 */
        &fly_no_sys,       /* 4 = exec::执行一个文件 */
        &do_sleep,      /* 5 = sleep::休眠 */
        &fly_no_sys,     /* 6 = open::打开一个文件 */
        &fly_no_sys,     /* 7 = close::关闭一个文件 */
        &fly_no_sys,     /* 8 = read::读取一个文件 */
        &fly_no_sys,     /* 9 = write::写入内容到一个文件 */

        &fly_no_sys,     /* 10 = unlink::取消一个文件的链接 */
        &fly_no_sys,     /* 11 = link::为文件设置一个链接 */
        &fly_no_sys,     /* 12 = lseek::调整文件读写的指针位置 */
        &fly_no_sys,     /* 13 = fstat::查看文件状态 */
        &fly_no_sys,     /* 14 = creat::创建一个新文件 */
        &fly_no_sys,     /* 15 = 未实现 */
        &fly_no_sys,     /* 16 = 未实现 */
        &fly_no_sys,     /* 17 = 未实现 */
        &fly_no_sys,     /* 18 = 未实现 */
        &fly_no_sys,     /* 19 = 未实现 */

        &fly_no_sys,    /* 20 = 未实现 */
        &fly_no_sys,    /* 21 = 未实现 */
        &fly_no_sys,    /* 22 = 未实现 */
        &fly_no_sys,    /* 23 = 未实现 */
        &fly_no_sys,    /* 24 = 未实现 */
        &fly_no_sys,    /* 25 = 未实现 */
        &fly_no_sys,    /* 26 = 未实现 */
        &fly_no_sys,    /* 27 = 未实现 */
        &fly_no_sys,    /* 28 = 未实现 */
        &fly_no_sys,    /* 29 = 未实现 */

        &fly_no_sys,    /* 30 = 未实现 */
        &fly_no_sys,    /* 31 = 未实现 */
        &fly_no_sys,    /* 32 = 未实现 */
        &fly_no_sys,    /* 33 = 未实现 */
        &fly_no_sys,    /* 34 = 未实现 */
        &fly_no_sys,    /* 35 = 未实现 */
        &fly_no_sys,    /* 36 = 未实现 */
        &fly_no_sys,    /* 37 = 未实现 */
        &fly_no_sys,    /* 38 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */

        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */

        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */

        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */

        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */

        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */

        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
        &fly_no_sys,    /* 5 = 未实现 */
};

/* 审计call_vec数组是否未分配到应有的内存 */
extern int dummy[sizeof(fly_call_handlers) == NR_CALLS * sizeof(fly_call_handlers[0]) ? 0 : -1];


