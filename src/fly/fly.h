/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 飞彦拓展器的主要头文件，具体作用和内存管理器的头文件一样，可前往查看详细介绍。
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

#include <limits.h>
#include <errno.h>

#include <flyanx/syslib.h>

#include "const.h"
#include "type.h"
#include "prototype.h"
#include "global.h"
