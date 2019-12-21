/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：等待子进程运行完成。
 * 因为我使用的是NASM的原因，而wait是NASM汇编的一个关键字，导致了
 * 不能在lib/syscall/下编写wait.asm的系统调用汇编版本。
 */

#include <lib.h>

#define child_pid           out.type
#define child_exit_status   out.m2_i1

PUBLIC pid_t wait(int *status){

    Message out;
    if(_syscall(MM, WAIT, &out) < 0) return -1;         /* 没有找到子进程，返回-1 */
    if(status != 0) *status = child_exit_status;        /* 返回子进程的退出状态 */
    return child_pid;       /* 返回子进程的进程号 */
}

