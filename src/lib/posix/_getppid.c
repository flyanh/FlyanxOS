/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：获取父亲的进程号。
 */

#include <lib.h>
#define getppid    _getppid
#include <unistd.h>

PUBLIC pid_t getppid(void){
    Message out;
    int rs = _syscall(MM, GETPPID, &out);
    return rs;
}

