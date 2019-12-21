/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：退出当前进程。
 */

#include <lib.h>
#define exit    _exit
#include <unistd.h>

PUBLIC void exit(int status){
    Message out;
    out.m1_i1 = status;
    _syscall(MM, EXIT, &out);   /* void exit(int)，没有返回值 */
}

