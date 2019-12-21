/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/19.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 一个进程已经完成EXEC调用，通知内核完成后续工作。
 */

#include "syslib.h"

PUBLIC int sys_exec(
        int proc,           /* 哪个进程完成了EXEC调用？ */
        char *sp,           /* 它新的栈指针在哪？ */
        char *prog_name,    /* 进程名称 */
        vir_bytes init_pc   /* 初始程序计数器，指向程序的开始地址 */
){
    Message out;

    out.PROC_NR1 = proc;
    out.STACK_PTR = sp;
    out.NAME_PTR = prog_name;
    out.PC_PTR = (char*)init_pc;
    return task_call(SYS_TASK, SYS_EXEC, &out);
}


