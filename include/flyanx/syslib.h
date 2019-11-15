/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 包含了在操作系统内部调用以访问操作系统其他服务的C库函数原型。
 * 操作系统向外提供的系统调用，也是通过调用这些库函数去实现的。
 */

#ifndef _FLYANX_SYSLIB_H
#define _FLYANX_SYSLIB_H

#ifndef FLYANX_TYPES_H
#include <sys/types.h>
#endif

/* 隐藏名称以避免名称空间被污染 */
#define sendrec		_sendrec
#define receive		_receive
#define send		_send

/* Flyanx 用户和系统双用库 */
_PROTOTYPE( int printf, (const char *_fmt, ...)				);
_PROTOTYPE( void putk, (int c)						);
_PROTOTYPE(void disp_str, (char* string));   /* 显示一个字符串 */
_PROTOTYPE(void disp_color_str, (char *string, int color));   /* 显示一个带颜色的字符串 */



#endif //_FLYANX_SYSLIB_H
