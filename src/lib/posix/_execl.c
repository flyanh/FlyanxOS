/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/18.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：执行一个文件，给出可变的命令行参数，但至少给一个。
 */

#include <lib.h>
#define execve  _execve
#define execl   _execl
#include <unistd.h>

PUBLIC int execl(const char *path, const char *arg1, ...){
    char *e[] = {"/"};
    return execve(path, (char **) &arg1, e);
}

