/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：获取自己的进程号。
 */

#include <lib.h>
#define getpid    _getpid
#include <unistd.h>

PUBLIC pid_t getpid(void){
    Message out;

    return _syscall(MM, GETPID, &out);
}

