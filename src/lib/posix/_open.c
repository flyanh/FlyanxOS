/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：打开一个文件。
 * 注意：如果一个文件名前面以_打头，那么这个文件是一个专门为用户进程
 * 编写的系统调用程序，以后我们将会看到更多。
 */

#include <lib.h>
#define open    _open
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>

#if _ANSI
PUBLIC int _open(const char *name, int flags, ...)
#else
PUBLIC int _open(
        _CONST char *name,
        int flags
)
#endif
{
    va_list argp;
    Message out;

    va_start(argp, flags);
    if(flags & O_CREAT) {
        /* 不存在需要创建，那么我们使用M1消息类型，过户的路径字符串始终放在了用户空间。 */
        out.m1_i1 = strlen(name) + 1;
        out.m1_i2 = flags;
        out.m1_i3 = va_arg(argp, Mode_t);
        out.m1_p1 = (char *) name;
    } else {
        /* 不需要就比较麻烦，我们使用M3消息类型，如果可能的话，将用户的路径字符串加载到消息中。 */
        load_name(name, &out);
        out.m3_i2 = flags;
    }
    va_end(argp);
    return _syscall(FS, OPEN, &out);
}


