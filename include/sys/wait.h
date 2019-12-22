/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 定义了WAIT和WAITPID系统调用所使用的宏,它们本身在内存管理器中实现。
 */

#ifndef _WAIT_H
#define _WAIT_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

#define WNOHANG         1   /* 不需要等待子进程退出 */
#define WUNTRACED       2   /* 为了任务控制；但我还未实现 */

/* 函数原型 */
_PROTOTYPE( pid_t wait, (int *_status) );
_PROTOTYPE( pid_t waitpid, (pid_t _pid, int *_status, int _options) );

#endif //_WAIT_H
