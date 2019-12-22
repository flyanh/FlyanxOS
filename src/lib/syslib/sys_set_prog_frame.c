/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/19.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 一个EXEC即将完成，需要为程序设置执行框架，包括程序的命令行参数以及运行环境变量。
 */

#include "syslib.h"

PUBLIC int sys_set_prog_frame(
        int proc_nr,        /* 哪个进程？ */
        int argc,           /* 该程序的命令行参数计数 */
        u32_t argv,         /* 该程序的命令行参数数组地址 */
        u32_t envp          /* 该程序的环境变量数组地址 */
){
    Message out;

    out.PROC_NR3 = proc_nr;
    out.ARGC = argc;
    out.ARGV = argv;
    out.ENVP = envp;
    return task_call(SYS_TASK, SYS_SET_PROG_FRAME, &out);
}

