/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/14.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 *  提供给用户进程的posix调用：创建一个新进程，用于分支完成新任务。
 */

#include <lib.h>
#define fork    _fork
#include <unistd.h>

PUBLIC pid_t fork(void){
    Message out;
    return _syscall(MM, FORK, &out);
}

