/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/14.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 一个新进程已经被创建，内核也需要做一些工作
 */

#include "syslib.h"

PUBLIC int sys_fork(
        int child,      /* 被创建的子进程 */
        int parent,     /* 谁FORK了子进程？ */
        int child_pid   /* 子进程的进程号 */
){
    Message out;

    out.m1_i1 = child;
    out.m1_i2 = parent;
    out.m1_i3 = child_pid;
    return task_call(SYS_TASK, SYS_FORK, &out);
}



