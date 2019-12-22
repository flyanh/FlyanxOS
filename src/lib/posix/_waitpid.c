/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：等待某一个子进程运行完成。
 */

#include <lib.h>
#define waitpid _waitpid
#include <sys/wait.h>

#define out_pid             out.m1_i1
#define out_options         out.m1_i2
#define child_pid           out.type
#define child_exit_status   out.m2_i1

PUBLIC pid_t waitpid(
        pid_t pid,      /* 等待哪个子进程？ */
        int *status,    /* 子进程退出状态存放在哪？ */
        int options     /* 等待选项 */
){
    Message out;

    out_pid = pid;
    out_options = options;
    if(_syscall(MM, WAITPID, &out) < 0) return -1;      /* 没有找到等待的子进程，返回-1 */
    if(status != 0) *status = child_exit_status;        /* 返回子进程的退出状态 */
    return child_pid;       /* 返回子进程的进程号 */
}
