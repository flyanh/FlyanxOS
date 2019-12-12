/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：得到文件的状态信息。
 */

#include <lib.h>
#define fstat    _fstat
#include <sys/stat.h>

PUBLIC int fstat(int fd, Stat *buf){
    Message out;

    out.m1_i1 = fd;
    out.m1_p1 = (char*)buf;
    return _syscall(FS, FSTAT, &out);
}

