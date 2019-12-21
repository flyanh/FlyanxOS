/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/19.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 服务器想知道某个进程的栈指针
 */

#include "syslib.h"

PUBLIC int sys_get_sp(
        int proc_nr,    /* 想要谁的？ */
        vir_bytes *sp   /* 拿到的堆栈指针放在哪？ */
){
    Message out;
    int rs;

    out.m1_i1 = proc_nr;
    rs = task_call(SYS_TASK, SYS_GET_SP, &out);
    *sp = (vir_bytes)out.STACK_PTR; /* 设置得到的栈指针 */
    return rs;
}


