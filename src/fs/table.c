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

/* 当一个请求到达时，其中的系统调用号将被取出作为fs_call_handlers数组的索引，以找到
 * 处理这个系统调用的过程。不是合法调用的系统调用号都会引起执行fs_no_sys，它只
 * 是返回一个错误代码。
 * 在这里值得注意的是输入_PROTOTYPE宏用在了call_handlers的定义中，这个定义
 * 在这并不是作为一个原型定义，而是一个初始化的数组的定义。但是因为它是一个函
 * 数的数组，使用_PROTOTYPE宏是使程序与经典C(Kernighan & Ritchie)和标准C
 * 都兼容的最简单的办法。
 */
_PROTOTYPE( int (*fs_call_handlers[NR_CALLS]), (void) ) = {
    &fs_no_sys,       /* 0 = 没有使用的调用 */
    &do_fs_exit,     /* 1 = exit::退出一个进程 */
    &fs_no_sys,     /* 2 = wait:: */
    &do_fs_fork,     /* 3 = fork::派生一个新进程 */
    &fs_no_sys,     /* 4 = exec::执行一个文件 */
    &fs_no_sys,      /* 5 = block::堵塞自己 */
    &do_open,       /* 6 = open::打开一个文件 */
    &do_close,     /* 7 = close::关闭一个文件 */
    &do_read,     /* 8 = read::读取一个文件 */
    &do_write,     /* 9 = write::写入内容到一个文件 */

    &do_unlink,     /* 10 = unlink::取消一个文件的链接 */
    &do_link,     /* 11 = link::为文件设置一个链接 */
    &do_lseek,     /* 12 = lseek::调整文件读写的指针位置 */
    &do_fstat,     /* 13 = fstat::查看文件状态 */
    &do_creat,     /* 14 = creat::创建一个新文件 */
    &do_mkdir,     /* 15 = mkdir::创建一个新目录 */
    &do_stat,     /* 16 = stat::查看文件状态 */
    &fs_no_sys,     /* 17 = 未实现 */
    &fs_no_sys,     /* 18 = 未实现 */
    &fs_no_sys,     /* 19 = 未实现 */

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
    &fs_no_sys,    /* 39 = 未实现 */

    &fs_no_sys,    /* 40 = 未实现 */
    &fs_no_sys,    /* 41 = 未实现 */
    &fs_no_sys,    /* 42 = 未实现 */
    &fs_no_sys,    /* 43 = 未实现 */
    &fs_no_sys,    /* 44 = 未实现 */
    &fs_no_sys,    /* 45 = 未实现 */
    &fs_no_sys,    /* 46 = 未实现 */
    &fs_no_sys,    /* 47 = 未实现 */
    &fs_no_sys,    /* 48 = 未实现 */
    &fs_no_sys,    /* 49 = 未实现 */

    &fs_no_sys,    /* 50 = 未实现 */
    &fs_no_sys,    /* 51 = 未实现 */
    &fs_no_sys,    /* 52 = 未实现 */
    &fs_no_sys,    /* 53 = 未实现 */
    &fs_no_sys,    /* 54 = 未实现 */
    &fs_no_sys,    /* 55 = 未实现 */
    &fs_no_sys,    /* 56 = 未实现 */
    &fs_no_sys,    /* 57 = kernel_sig::内核发出信号 */
    &fs_no_sys,    /* 58 = unpause::检查EINTR */
    &do_revive,    /* 59 = revive::恢复挂起的进程 */

    &fs_no_sys,    /* 60 = task_reply::任务回复 */
    &fs_no_sys,    /* 61 = 未实现 */
    &fs_no_sys,    /* 62 = 未实现 */
    &fs_no_sys,    /* 63 = 未实现 */
    &fs_no_sys,    /* 64 = 未实现 */
    &fs_no_sys,    /* 65 = 未实现 */
    &fs_no_sys,    /* 66 = 未实现 */
    &fs_no_sys,    /* 67 = 未实现 */
    &fs_no_sys,    /* 68 = 未实现 */
    &fs_no_sys,    /* 69 = 未实现 */

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


