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

typedef void(*putk_func_t)(int ch);     /* 输出一个字符函数类型 */

/* Flyanx 用户和系统双用库 */
_PROTOTYPE( int send_receive, (int src_dest, Message *message_ptr) );
_PROTOTYPE( int printf, (const char *_fmt, ...)				);
_PROTOTYPE( int redirect_printf, (const char *fmt, char *argp, putk_func_t rp_putk) );
_PROTOTYPE( void putk, (int c)						);


/* Flyanx系统库 */
_PROTOTYPE( int receive, (int src, Message *message_ptr)			);
_PROTOTYPE( int send, (int dest, Message *message_ptr)			);
_PROTOTYPE( void k_putk, (int c)						);



#endif //_FLYANX_SYSLIB_H
