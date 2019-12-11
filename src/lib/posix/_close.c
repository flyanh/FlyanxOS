/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：关闭一个已使用的文件。
 */

#include <lib.h>
#define close   _close
#include <unistd.h>

PUBLIC int _close(int fd){
    Message out;

    out.m1_i1 = fd;
    return _syscall(FS, CLOSE, &out);
}


