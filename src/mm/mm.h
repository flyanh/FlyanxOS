/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内存管理器的主要头文件，内存管理器中的所有文件都需要包含它，它自己则包含
 * 了每个目标模块都需要的位于/usr/include和子目录中所有系统范围的头文件。
 * 大部分包含在/kernel/kernel.h中的文件也被包含到了这里，MM还需要/usr/include/fcntl.h
 * 和/usr/include/unistd.h中的定义。
 * 内存管理器是独立运行的一个用户进程，所以MM有自己版本的const.h、type.h、global.h。
 */

#define _POSIX_SOURCE      1	/* 告诉头包含POSIX内容 */
#define _FLYANX             1	/* 告诉头包含FLYANX内容 */
#define _SYSTEM            1	/* 告诉头这是内核（要被编译进内核的代码） */

/* 以下内容非常简单，所有*.c文件都会自动获得它们。 */
#include <flyanx/config.h>      /* 这个头文件必须第一个引入 */
#include <ansi.h>               /* 必须第二个 */
#include <sys/types.h>
#include <flyanx/const.h>
#include <flyanx/type.h>

#include <fcntl.h>
#include <unistd.h>

#include <limits.h>
#include <errno.h>

#include <flyanx/syslib.h>

#include "const.h"
#include "type.h"
#include "prototype.h"
#include "global.h"
