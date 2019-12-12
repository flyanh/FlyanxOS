/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：取消一个文件的链接，等同于删除。
 */

#include <lib.h>
#define unlink  _unlink
#include <unistd.h>

PUBLIC int unlink(_CONST char *name){
    Message out;

    load_name(name, &out);
    return _syscall(FS, UNLINK, &out);
}



