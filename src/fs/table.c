/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * table.c为所有的全局变量进行声明并为其保留空间
 * 除了本文件中可以看到的变量声明，本文件的编译还会为我们在global.c和可以看到
 * 的各种头文件中用EXTERN声明的变量声明空间。
 */

#define _TABLE

#include "fs.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "dev.h"
#include "file.h"
#include "fsproc.h"
#include "inode.h"
#include "super.h"

/* 当一个请求到达时，其中的系统调用号将被取出作为call_vec数组的索引，以找到
 * 处理这个系统调用的过程。不是合法调用的系统调用号都会引起执行fs_no_sys，它只
 * 是返回一个错误代码。
 * 在这里值得注意的是输入_PROTOTYPE宏用在了call_handlers的定义中，这个定义
 * 在这并不是作为一个原型定义，而是一个初始化的数组的定义。但是因为它是一个函
 * 数的数组，使用_PROTOTYPE宏是使程序与经典C(Kernighan & Ritchie)和标准C
 * 都兼容的最简单的办法。
 */
_PROTOTYPE( int (*fs_call_handlers[NR_CALLS]), (void) ) = {
    &fs_no_sys,    /* 0 = 没有使用的调用 */
    &fs_no_sys,    /* 1 = exit::退出一个进程 */
    &fs_no_sys,    /* 2 = wait:: */
    &fs_no_sys,    /* 3 = fork::派生一个新进程 */
    &fs_no_sys,    /* 4 = exec::执行一个文件 */
    &fs_no_sys,    /* 5 = block::堵塞自己 */
    &fs_no_sys,    /* 6 = 未实现 */
    &fs_no_sys,    /* 7 = 未实现 */
    &fs_no_sys,    /* 8 = 未实现 */
    &fs_no_sys,    /* 9 = 未实现 */

    &fs_no_sys,    /* 10 = 未实现 */
    &fs_no_sys,    /* 11 = 未实现 */
    &fs_no_sys,    /* 12 = 未实现 */
    &fs_no_sys,    /* 13 = 未实现 */
    &fs_no_sys,    /* 14 = 未实现 */
    &fs_no_sys,    /* 15 = 未实现 */
    &fs_no_sys,    /* 16 = 未实现 */
    &fs_no_sys,    /* 17 = 未实现 */
    &fs_no_sys,    /* 18 = 未实现 */
    &fs_no_sys,    /* 19 = 未实现 */

    &fs_no_sys,    /* 20 = 未实现 */
    &fs_no_sys,    /* 21 = 未实现 */
    &fs_no_sys,    /* 22 = 未实现 */
    &fs_no_sys,    /* 23 = 未实现 */
    &fs_no_sys,    /* 24 = 未实现 */
    &fs_no_sys,    /* 25 = 未实现 */
    &fs_no_sys,    /* 26 = 未实现 */
    &fs_no_sys,    /* 27 = 未实现 */
    &fs_no_sys,    /* 28 = 未实现 */
    &fs_no_sys,    /* 29 = 未实现 */

    &fs_no_sys,    /* 30 = 未实现 */
    &fs_no_sys,    /* 31 = 未实现 */
    &fs_no_sys,    /* 32 = 未实现 */
    &fs_no_sys,    /* 33 = 未实现 */
    &fs_no_sys,    /* 34 = 未实现 */
    &fs_no_sys,    /* 35 = 未实现 */
    &fs_no_sys,    /* 36 = 未实现 */
    &fs_no_sys,    /* 37 = 未实现 */
    &fs_no_sys,    /* 38 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */

    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */

    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */

    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */

    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */

    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */

    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
    &fs_no_sys,    /* 5 = 未实现 */
};

/* 审计call_vec数组是否未分配到应有的内存 */
extern int dummy[sizeof(fs_call_handlers) == NR_CALLS * sizeof(fs_call_handlers[0]) ? 0 : -1];

/* 设备-驱动映射表
 * 对于ddmap[n]，n是一个设备号，ddmap[n].driver_task_nr是该
 * 设备的驱动任务号，通过它可以知道驱动任务。
 *
 * 如果要更改该表，请查阅usr/include/flyanx/common.h下的各驱
 * 动任务编号。
 */
PUBLIC DDMap ddmap[] = {
        /* driver nr.		major device nr.
	   ----------		---------------- */
        {NO_EXIST_TASK},	/* 0 : 未使用 */
        {NO_EXIST_TASK},	/* 1 : 保留用于软盘驱动程序 */
        {NO_EXIST_TASK},	/* 2 : 保留用于光盘驱动程序 */
        {CONTROLLER(1)},	/* 3 : 硬盘驱动程序 */
        {TTY_TASK},		    /* 4 : TTY */
        {NO_EXIST_TASK}	/* 5 : 保留用于SCSI磁盘驱动程序 */
};


