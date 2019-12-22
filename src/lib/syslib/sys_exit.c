/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 一个进程已经退出，内核也需要做一些工作
 */

#include "syslib.h"

PUBLIC int sys_exit(
        int proc_nr,    /* 退出的进程索引号 */
        int parent      /* 上面那个的父亲 */
){
    Message out;

    out.m1_i1 = proc_nr;
    out.m1_i2 = parent;
    return task_call(SYS_TASK, SYS_EXIT, &out);
}

