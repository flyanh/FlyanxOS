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

#define NCALLS		  78	/* 允许的系统调用数量 */

#define EXIT		   1
#define FORK		   2
#define READ		   3
#define WRITE		   4
#define OPEN		   5
#define CLOSE		   6
#define WAIT		   7
#define CREAT		   8
#define LINK		   9
#define UNLINK		  10
#define WAITPID		  11
#define CHDIR		  12
#define TIME		  13
#define MKNOD		  14
#define CHMOD		  15
#define CHOWN		  16
#define BRK		      17
#define STAT		  18
#define LSEEK		  19
#define GETPID		  20
#define MOUNT		  21
#define UMOUNT		  22
#define SETUID		  23
#define GETUID		  24
#define STIME		  25
#define PTRACE		  26
#define ALARM		  27
#define FSTAT		  28
#define PAUSE		  29
#define UTIME		  30
#define ACCESS		  33
#define SYNC		  36
#define KILL		  37
#define RENAME		  38
#define MKDIR		  39
#define RMDIR		  40
#define DUP		      41
#define PIPE		  42
#define TIMES		  43
#define SETGID		  46
#define GETGID		  47
#define SIGNAL		  48
#define IOCTL		  54
#define FCNTL		  55
#define EXEC		  59
#define UMASK		  60
#define CHROOT		  61
#define SETSID		  62
#define GETPGRP		  63

/* The following are not system calls, but are processed like them. */
/* 以下不是系统调用，但像它们一样进行处理。 */
#define KERNEL_SIG	    64	/* 内核检测到信号 */
#define UNPAUSE		    65	/* 给MM或FS：检查EINTR */
#define REVIVE	 	    67	/* 给FS：恢复睡眠进程 */
#define TASK_REPLY	    68	/* 给FS：终端任务回复一个执行代码 */

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
