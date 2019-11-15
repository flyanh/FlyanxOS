/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内核的主头文件
 *
 * 该文件包含内核所需要的所有定义
 */

/*
 * _POSIX_SOURCE是POSIX标准自行定义的一个特征检测宏。
 * 作用是保证所有POSIX要求的符号和那些显式地允许但并不要求的符号将是可见的，
 * 同时隐藏掉任何POSIX非官方扩展的附加符号。
 */
#define _POSIX_SOURCE      1

/* 宏_FLYANX将为FLYANX所定义的扩展而"重载_POSIX_SOURCE"的作用 */
#define _FLYANX             1

/* 在编译系统代码时，如果要作与用户代码不同的事情，比如改变错误码的符号，则可以对_SYSTEM宏进行测试 */
#define _SYSTEM            1	/* tell headers that this is the kernel */

/* 以下内容非常简单，所有*.c文件都会自动获得它们。 */
#include <flyanx/config.h>      /* 这个头文件必须第一个引入 */
#include <ansi.h>               /* 必须第二个 */
#include <sys/types.h>
#include <flyanx/const.h>
#include <flyanx/type.h>
#include <flyanx/syslib.h>

#include <string.h>
#include <limits.h>
#include <errno.h>

#include "const.h"
#include "type.h"
#include "prototype.h"
#include "global.h"

