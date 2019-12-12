/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：创建一个目录（文件夹）。
 */

#include <lib.h>
#define mkdir   _mkdir
#include <sys/stat.h>
#include <string.h>

PUBLIC int mkdir(_CONST char *name, Mode_t mode){
    Message out;

    out.m1_i1 = strlen(name) + 1;
    out.m1_i2 = mode;
    out.m1_p1 = (char *) name;
    return _syscall(FS, MKDIR, &out);
}

