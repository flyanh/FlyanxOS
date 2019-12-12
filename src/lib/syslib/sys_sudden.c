/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/5.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 系统停止运行，这个库例程只能被服务器调用。它被起了这么个奇怪的名字
 * 是因为系统停止的时候会蓝屏，那很突然(sudden)，令人不知所措。
 */

#include "syslib.h"
#include <stdarg.h>
#include <unistd.h>

PUBLIC int sys_sudden(int how, ...){
    /* 一些古怪出现了，虽然很突然，但现在必须放弃系统的继续运行。 */

    Message out;
    va_list ap;

    va_start(ap, how);

    out.m1_i1 = how;
    if(how == RBT_MONITOR){
        out.m1_i2 = how;
        out.m1_p1 = va_arg(ap, char*);
        out.m1_i3 = va_arg(ap, size_t);
    }

    va_end(ap);

    return task_call(SYS_TASK, SYS_SUDDEN, &out);
}

