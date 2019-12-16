/* The <signal.h> header defines all the ANSI and POSIX signals.
 * MINIX supports all the signals required by POSIX. They are defined below.
 * Some additional signals are also supported.
 */
/* 这里定义了标准信号名，同时包含一些与信号相关的函数原型。
 * 
 * 我发现，暂时没有时间实现信号了，这个功能被我放到了未来，但是
 * 这个文件它就在这，哪也不去。
 */

#ifndef _SIGNAL_H
#define _SIGNAL_H

#ifndef _ANSI_H
#include <ansi.h>
#endif
#ifdef _POSIX_SOURCE
#ifndef FLYANX_TYPES_H
#include <sys/types.h>
#endif
#endif

/* Here are types that are closely associated with signal handling. */
/*  */
typedef int sig_atomic_t;

#ifdef _POSIX_SOURCE
#ifndef _SIGSET_T
#define _SIGSET_T
typedef unsigned long sigset_t;
#endif
#endif

#define _NSIG             17	/* number of signals used ：被使用的信号数量 */

#define SIGHUP             1	/* hangup */
#define SIGINT             2	/* interrupt (DEL) */
#define SIGQUIT            3	/* quit (ASCII FS) */
#define SIGILL             4	/* illegal instruction */
#define SIGTRAP            5	/* trace trap (not reset when caught) ：跟踪陷阱（捕获时不重置） */
#define SIGABRT            6	/* IOT instruction */
#define SIGIOT             6	/* SIGABRT for people who speak PDP-11 */
#define SIGUNUSED          7	/* spare code */
#define SIGFPE             8	/* floating point exception */
#define SIGKILL            9	/* kill (cannot be caught or ignored) */
#define SIGUSR1           10	/* user defined signal # 1 */
#define SIGSEGV           11	/* segmentation violation */
#define SIGUSR2           12	/* user defined signal # 2 */
#define SIGPIPE           13	/* write on a pipe with no one to read it */
#define SIGALRM           14	/* alarm clock */
#define SIGTERM           15	/* software termination signal from kill */
#define SIGCHLD           17	/* child process terminated or stopped */

#define SIGEMT             7	/* obsolete */
#define SIGBUS            10	/* obsolete */

/* POSIX requires the following signals to be defined, even if they are
 * not supported.  Here are the definitions, but they are not supported.
 */
#define SIGCONT           18	/* continue if stopped */
#define SIGSTOP           19	/* stop signal */
#define SIGTSTP           20	/* interactive stop signal */
#define SIGTTIN           21	/* background process wants to read */
#define SIGTTOU           22	/* background process wants to write */

/* The sighandler_t type is not allowed unless _POSIX_SOURCE is defined. */
typedef void _PROTOTYPE( (*__sighandler_t), (int) );

/* Macros used as function pointers. */
#define SIG_ERR    ((__sighandler_t) -1)	/* error return */
#define SIG_DFL	   ((__sighandler_t)  0)	/* default signal handling */
#define SIG_IGN	   ((__sighandler_t)  1)	/* ignore signal */
#define SIG_HOLD   ((__sighandler_t)  2)	/* block signal */
#define SIG_CATCH  ((__sighandler_t)  3)	/* catch signal */

#ifdef _POSIX_SOURCE
/* 信号动作结构定义 */
struct sigaction {
  __sighandler_t sa_handler;	/* SIG_DFL, SIG_IGN, or pointer to function ：按照缺省的方式处理、忽略、还是由专门的处理过程处理。 */
  sigset_t sa_mask;		        /* signals to be blocked during handler ：处理程序期间要阻塞的信号 */
  int sa_flags;			          /* special flags ：信号处理用的标志 */
};

/* Fields for sa_flags. */
#define SA_ONSTACK   0x0001	/* deliver signal on alternate stack */
#define SA_RESETHAND 0x0002	/* reset signal handler when signal caught ：捕获信号时重置信号处理程序 */
#define SA_NODEFER   0x0004	/* don't block signal while catching it ：捕获信号时不要阻塞信号（同类型） */
#define SA_RESTART   0x0008	/* automatic system call restart */
#define SA_SIGINFO   0x0010	/* extended signal handling */
#define SA_NOCLDWAIT 0x0020	/* don't create zombies */
#define SA_NOCLDSTOP 0x0040	/* don't receive SIGCHLD when child stops */

/* POSIX requires these values for use with sigprocmask(2). */
#define SIG_BLOCK          0	/* for blocking signals */
#define SIG_UNBLOCK        1	/* for unblocking signals */
#define SIG_SETMASK        2	/* for setting the signal mask */
#define SIG_INQUIRE        4	/* for internal use only */
#endif	/* _POSIX_SOURCE */

/* POSIX and ANSI function prototypes. */
_PROTOTYPE( int raise, (int _sig)					);
_PROTOTYPE( __sighandler_t signal, (int _sig, __sighandler_t _func)	);

#ifdef _POSIX_SOURCE
_PROTOTYPE( int kill, (pid_t _pid, int _sig)				);
_PROTOTYPE( int sigaction,
    (int _sig, const struct sigaction *_act, struct sigaction *_oact)	);
_PROTOTYPE( int sigaddset, (sigset_t *_set, int _sig)			);
_PROTOTYPE( int sigdelset, (sigset_t *_set, int _sig)			);
_PROTOTYPE( int sigemptyset, (sigset_t *_set)				);
_PROTOTYPE( int sigfillset, (sigset_t *_set)				);
_PROTOTYPE( int sigismember, (const sigset_t *_set, int _sig)		);
_PROTOTYPE( int sigpending, (sigset_t *_set)				);
_PROTOTYPE( int sigprocmask,
	    (int _how, const sigset_t *_set, sigset_t *_oset)		);
_PROTOTYPE( int sigsuspend, (const sigset_t *_sigmask)			);
#endif

#endif /* _SIGNAL_H */
