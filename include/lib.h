/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本头文件是库使用的主标头。 lib子目录中的所有C文件都包括它。
 */

#ifndef FLYANX_LIB_H
#define FLYANX_LIB_H

/*
 * _POSIX_SOURCE是POSIX标准自行定义的一个特征检测宏。
 * 作用是保证所有POSIX要求的符号和那些显式地允许但并不要求的符号将是可见的，
 * 同时隐藏掉任何POSIX非官方扩展的附加符号。
 */
#define _POSIX_SOURCE      1

/* 宏_FLYANX将为FLYANX所定义的扩展而"重载_POSIX_SOURCE"的作用 */
#define _FLYANX

#include <flyanx/config.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <ansi.h>

#include <flyanx/const.h>
#include <flyanx/type.h>
#include <flyanx/callnr.h>

#define MM                  0       /* 内存管理器 */
#define FS                  1       /* 文件系统 */
#define FLY                 2       /* 飞彦拓展器，其他调用都在这处理 */

_PROTOTYPE( void panic, (const char *_message, int _errnum)		);



#endif //FLYANX_LIB_H
