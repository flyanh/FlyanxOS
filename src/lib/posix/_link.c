/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 *
 * 提供给用户进程的posix调用：链接两个文件。
 */

#include <lib.h>
#define link    _link
#include <string.h>
#include <unistd.h>

PUBLIC int link(_CONST char *name, _CONST char *name2){
    Message out;

    out.m1_i1 = strlen(name) + 1;
    out.m1_i2 = strlen(name2) + 1;
    out.m1_p1 = (char *)name;
    out.m1_p2 = (char *)name2;
    return _syscall(FS, LINK, &out);
}

