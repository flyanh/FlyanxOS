/* The <errno.h> header defines the numbers of the various errors that can
 * occur during program execution.  They are visible to user programs and 
 * should be small positive integers.  However, they are also used within 
 * MINIX, where they must be negative.  For example, the READ system call is 
 * executed internally by calling do_read().  This function returns either a 
 * (negative) error number or a (positive) number of bytes actually read.
 *
 * To solve the problem of having the error numbers be negative inside the
 * the system and positive outside, the following mechanism is used.  All the
 * definitions are are the form:
 *
 *	#define EPERM		(_SIGN 1)
 *
 * If the macro _SYSTEM is defined, then  _SIGN is set to "-", otherwise it is
 * set to "".  Thus when compiling the operating system, the  macro _SYSTEM
 * will be defined, setting EPERM to (- 1), whereas when when this
 * file is included in an ordinary user program, EPERM has the value ( 1).
 */

/**
 * 包含了系统调用失败时从全局变量errno返回给用户程序的错误码。
 * errno也用，来标识一些内部错误，如试图向一个不存在的任务发送消息。
 * 在MINIX中,若返回值为负表示它是一个错误码,但在返回用户程序之前必须使它们为正值，
 * 这里的技巧在于每个错误码都在类似下面一行语句中定义:
 * #define EPERM         (_SIGN  1)
 * 操作系统各部分的主控头文件定义了宏 _SYSTEM,但在一个用户程序被编译后 _SYSTEM 将并未定义。如果
 * _SYSTEM 被定义了,则_SIGN 定义为“-”,否则 _SIGN无定义。
 *
 * 从Minix中引入...个人有些许修改
 */

#ifndef _ERRNO_H		/* check if <errno.h> is already included */
#define _ERRNO_H		/* it is not included; note that fact */

/* Now define _SIGN as "" or "-" depending on _SYSTEM. */
#ifdef _SYSTEM
#   define _SIGN         -
#   define OK            0
#else
#   define _SIGN         
#endif

extern int errno;		  /* place where the error numbers go */

/* Here are the numerical values of the error numbers. */
#define _NERROR               70  /* number of errors */  

#define EGENERIC      (_SIGN 99)  /* generic error */
#define EPERM         (_SIGN  1)  /* operation not permitted */
#define ENOENT        (_SIGN  2)  /* no such file or directory */
#define ESRCH         (_SIGN  3)  /* no such process */
#define EINTR         (_SIGN  4)  /* interrupted function call ：函数调用中断 */
#define EIO           (_SIGN  5)  /* input/output error */
#define ENXIO         (_SIGN  6)  /* 没有这样的设备或地址 */
#define E2BIG         (_SIGN  7)  /* arg list too long */
#define ENOEXEC       (_SIGN  8)  /* exec format error */
#define EBADF         (_SIGN  9)  /* bad file descriptor */
#define ECHILD        (_SIGN 10)  /* no child process */
#define EAGAIN        (_SIGN 11)  /* resource temporarily unavailable */
#define ENOMEM        (_SIGN 12)  /* not enough space */
#define EACCES        (_SIGN 13)  /* permission denied */
#define EFAULT        (_SIGN 14)  /* bad address */
#define ENOTBLK       (_SIGN 15)  /* Extension: not a block special file */
#define EBUSY         (_SIGN 16)  /* resource busy */
#define EEXIST        (_SIGN 17)  /* file exists */
#define EXDEV         (_SIGN 18)  /* improper link */
#define ENODEV        (_SIGN 19)  /* no such device */
#define ENOTDIR       (_SIGN 20)  /* not a directory */
#define EISDIR        (_SIGN 21)  /* is a directory */
#define EINVAL        (_SIGN 22)  /* 无效的参数 */
#define ENFILE        (_SIGN 23)  /* too many open files in system */
#define EMFILE        (_SIGN 24)  /* too many open files */
#define ENOTTY        (_SIGN 25)  /* inappropriate I/O control operation */
#define ETXTBSY       (_SIGN 26)  /* no longer used */
#define EFBIG         (_SIGN 27)  /* file too large */
#define ENOSPC        (_SIGN 28)  /* no space left on device */
#define ESPIPE        (_SIGN 29)  /* invalid seek */
#define EROFS         (_SIGN 30)  /* read-only file system */
#define EMLINK        (_SIGN 31)  /* too many links */
#define EPIPE         (_SIGN 32)  /* broken pipe */
#define EDOM          (_SIGN 33)  /* domain error    	(from ANSI C std) */
#define ERANGE        (_SIGN 34)  /* result too large	(from ANSI C std) */
#define EDEADLK       (_SIGN 35)  /* resource deadlock avoided */
#define ENAMETOOLONG  (_SIGN 36)  /* file name too long */
#define ENOLCK        (_SIGN 37)  /* no locks available */
#define ENOSYS        (_SIGN 38)  /* function not implemented */
#define ENOTEMPTY     (_SIGN 39)  /* directory not empty */

/* The following errors relate to networking. */
#define EPACKSIZE     (_SIGN 50)  /* invalid packet size for some protocol */
#define EOUTOFBUFS    (_SIGN 51)  /* not enough buffers left */
#define EBADIOCTL     (_SIGN 52)  /* illegal ioctl for device */
#define EBADMODE      (_SIGN 53)  /* badmode in ioctl */
#define EWOULDBLOCK   (_SIGN 54)
#define EBADDEST      (_SIGN 55)  /* not a valid destination address */
#define EDSTNOTRCH    (_SIGN 56)  /* destination not reachable */
#define EISCONN	      (_SIGN 57)  /* all ready connected */
#define EADDRINUSE    (_SIGN 58)  /* address in use */
#define ECONNREFUSED  (_SIGN 59)  /* connection refused */
#define ECONNRESET    (_SIGN 60)  /* connection reset */
#define ETIMEDOUT     (_SIGN 61)  /* connection timed out */
#define EURG	      (_SIGN 62)  /* urgent data present */
#define ENOURG	      (_SIGN 63)  /* no urgent data present */
#define ENOTCONN      (_SIGN 64)  /* no connection (yet or anymore) */
#define ESHUTDOWN     (_SIGN 65)  /* a write call to a shutdown connection */
#define ENOCONN       (_SIGN 66)  /* no such connection */

/* 以下不是POSIX的错误，但仍然可能发生。 */
#define ERROR_LOCKED      (_SIGN 101)   /* 可能会出现死锁，不能发送一条消息 */
#define ERROR_BAD_CALL     (_SIGN 102)  /* 发送/接收错误 */

/* 以下错误代码由内核本身生成。 */
#ifdef _SYSTEM
#define ERROR_BAD_ELF      -6552    /* 错误的ELF文件头 */
#define ERROR_BAD_DEST     -1001	/* 目的地址非法 */
#define ERROR_BAD_SRC      -1002	/* 源地址非法 */
#define ERROR_TRY_AGAIN    -1003	/* can't send-- tables full */
#define ERROR_OVERRUN      -1004	/* interrupt for task that is not waiting */
#define ERROR_BAD_BUF      -1005	/* message buf outside caller's addr space */
#define ERROR_TASK         -1006	/* 无法发送到任务，任务可能是错误的 */
#define ERROR_NO_MESSAGE   -1007	/* RECEIVE failed: no message present */
#define ERROR_NO_PERM      -1008	/* 普通用户无法直接发送消息到任务 */
#define ERROR_BAD_FCN      -1009	/* only valid fcns are SEND, RECEIVE, BOTH */
#define ERROR_BAD_ADDR     -1010	/* bad address given to utility routine */
#define ERROR_BAD_PROC     -1011	/* bad proc number given to utility */
#endif /* _SYSTEM */

#endif /* _ERRNO_H */
