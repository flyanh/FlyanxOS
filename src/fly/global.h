/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#ifndef _FLY_GLOBAL_H
#define _FLY_GLOBAL_H

#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif

/* 全局变量 */
//EXTERN

/* 外界的调用参数在这 */
EXTERN Message msg_in;     /* 传入的消息保存在这 */
EXTERN int who;         /* 调用进程的进程号 */

extern _PROTOTYPE( int (*call_handlers[]), (void) );    /* 系统调用处理函数在这里 */

#endif //_FLY_GLOBAL_H
