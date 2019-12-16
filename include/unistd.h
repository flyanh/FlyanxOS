/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 这个头文件定义了许多常量，其中多数是POSIX需要的。
 * 它也包含了许多C函数的原型，其中包括所有用于进行系统调用的C函数原型。
 */

#ifndef _UNISTD_H
#define _UNISTD_H

#ifndef FLYANX_TYPES_H
#include <sys/types.h>
#endif

/* Values used by access().  POSIX Table 2-8. */
#define F_OK               0	/* test if file exists */
#define X_OK               1	/* test if file is executable */
#define W_OK               2	/* test if file is writable */
#define R_OK               4	/* test if file is readable */

/* Values used for whence in lseek(fd, offset, whence).  POSIX Table 2-9. */
#define SEEK_SET           0	/* offset is absolute  */
#define SEEK_CUR           1	/* offset is relative to current position */
#define SEEK_END           2	/* offset is relative to end of file */

/* This value is required by POSIX Table 2-10. */
#define _POSIX_VERSION 199009L	/* which standard is being conformed to */

/* These three definitions are required by POSIX Sec. 8.2.1.2. */
#define STDIN_FILENO       0	/* file descriptor for stdin */
#define STDOUT_FILENO      1	/* file descriptor for stdout */
#define STDERR_FILENO      2	/* file descriptor for stderr */

#ifdef _FLYANX
/* How to exit the system. */
/* 如何退出系统 */
#define RBT_HALT	   0	/* 正常关机 */
#define RBT_REBOOT	   1	/* 重启 */
#define RBT_PANIC	   2	/* for servers ：来自服务 */
#define RBT_MONITOR	   3	/* let the monitor do this ：让监控程序做这件事 */
#define RBT_RESET	   4	/* hard reset the system ：硬重置系统 */
#endif

/* NULL must be defined in <unistd.h> according to POSIX Sec. 2.7.1. */
#define NULL    ((void *)0)

/* The following relate to configurable system variables. POSIX Table 4-2. */
#define _SC_ARG_MAX		1
#define _SC_CHILD_MAX		2
#define _SC_CLOCKS_PER_SEC	3
#define _SC_CLK_TCK             3
#define _SC_NGROUPS_MAX		4
#define _SC_OPEN_MAX		5
#define _SC_JOB_CONTROL		6
#define _SC_SAVED_IDS		7
#define _SC_VERSION		8
#define _SC_STREAM_MAX		9
#define _SC_TZNAME_MAX         10

/* The following relate to configurable pathname variables. POSIX Table 5-2. */
#define _PC_LINK_MAX		1	/* link count */
#define _PC_MAX_CANON		2	/* size of the canonical input queue */
#define _PC_MAX_INPUT		3	/* type-ahead buffer size */
#define _PC_NAME_MAX		4	/* file name size */
#define _PC_PATH_MAX		5	/* pathname size */
#define _PC_PIPE_BUF		6	/* pipe size */
#define _PC_NO_TRUNC		7	/* treatment of long name components */
#define _PC_VDISABLE		8	/* tty disable */
#define _PC_CHOWN_RESTRICTED	9	/* chown restricted or not */

/* POSIX defines several options that may be implemented or not, at the
 * implementer's whim.  This implementer has made the following choices:
 *
 * _POSIX_JOB_CONTROL	    not defined:	no job control
 * _POSIX_SAVED_IDS 	    not defined:	no saved uid/gid
 * _POSIX_NO_TRUNC	    defined as -1:	long path names are truncated
 * _POSIX_CHOWN_RESTRICTED  defined:		you can't give away files
 * _POSIX_VDISABLE	    defined:		tty functions can be disabled
 */
#define _POSIX_NO_TRUNC       (-1)
#define _POSIX_CHOWN_RESTRICTED  1

/* 以下是POSIX标准系统调用的函数原型  */
_PROTOTYPE( int close, (int _fd) );
_PROTOTYPE( ssize_t read, (int _fd, void *_buffer, size_t _bytes) );
_PROTOTYPE( ssize_t write, (int _fd, const void *_buffer, size_t _bytes) );
_PROTOTYPE( int link, (const char *_name, const char *_name2) );
_PROTOTYPE( int unlink, (const char *_name) );
_PROTOTYPE( off_t lseek, (int _fd, off_t _offset, int _whence) );
_PROTOTYPE( pid_t fork, (void) );


_PROTOTYPE( int sleep, (time_t _mills) );




#endif //FLYANX_UNISTD_H
