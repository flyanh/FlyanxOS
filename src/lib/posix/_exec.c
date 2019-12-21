/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/18.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：执行一个文件，无参数。
 */

#include <lib.h>
#define execve  _execve
#define exec   _exec
#include <unistd.h>


PUBLIC int exec(
        const char *path        /* 执行文件的全路径 */
){
    char *v[] = {};
    char *e[] = {"/"};
    return _execve(path, v, e);
}

