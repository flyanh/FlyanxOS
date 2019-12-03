/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含Flyanx的所有的系统调用号
 */

#ifndef _FLYANX_CALLNR_H
#define _FLYANX_CALLNR_H

#define NR_CALLS		    96  /* 允许的系统调用数量 */

#define EXIT            1
#define WAIT            2
#define FORK            3
#define EXEC            4
#define BLOCK           5


/* The following are not system calls, but are processed like them. */
/* 以下不是系统调用，但像它们一样进行处理。 */
#define KERNEL_SIG	    64	/* 内核检测到信号 */
#define UNPAUSE		    65	/* 给MM或FS：检查EINTR */
#define REVIVE	 	    67	/* 给FS：恢复睡眠进程 */
#define TASK_REPLY	    68	/* 给FS：终端/设备任务回复一个执行代码 */

/* Posix signal handling. */
/* Posix信号处理。 */
#define SIGACTION	  71
#define SIGSUSPEND	  72
#define SIGPENDING	  73
#define SIGPROCMASK	  74
#define SIGRETURN	  75

#define REBOOT		  76
#define SVRCTL		  77

#endif //_FLYANX_CALLNR_H
