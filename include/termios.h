/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/18.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 定义了控制终端类型的I/O所用到的常量、宏和函数原型。
 * 其中最重要的数据结构是termios结构，包含的内容有:标识操作模式的标志位、设置输入输出
 * 速率的变量、以及放置特殊字符的数组(比如 INTR 和KILL)。termios是POSIX所需要的,同
 * 样地,该文件中定义的许多宏和函数原型也是POSIX需要的。
 *
 * 然而,正如POSIX标准始终遵循的那样,它并未提供可能用到的全部内容。在文件的后半部分的
 * “#ifdef _FLYANX”中,提供了POSIX并不包含的扩展部分。其中有些扩展的意义是很明显的。
 * 比如定义57,600或以上的波特率,以及对终端显示窗口的支持。正如任何合理的标准都是开放
 * 的那样,POSIX标准并不排斥扩展。但在FLYANX环境下想写一个可移植到其他环境的程序时,就
 * 必须注意避免使用那些局限于FLYANX的特征。这一点很容易做到。在该文件和其他定义面向
 * FLYANX的扩展的文件中,这些扩展是通过以下语句控制的。
 * # ifdef _FLYANX
 * 如果 _FLYANX未被定义,编译器将根本不会感知到FLYANX扩展。
 */

#ifndef FLYANX_TERMIOS_H
#define FLYANX_TERMIOS_H

typedef unsigned short  tcflag_t;
typedef unsigned char   cc_t;
typedef unsigned int    speed_t;

#define NCCS		20	/* size of cc_c array, some extra space
				 	* for extensions. */

/* 终端控制结构 */
typedef struct termios_s {
    tcflag_t iflag;			/* 输入模式 */
    tcflag_t oflag;			/* 输出模式 */
    tcflag_t cflag;			/* 控制模式 */
    tcflag_t lflag;			/* 本地方式 */
    speed_t  ispeed;		/* 输入速度 */
    speed_t  ospeed;		/* 输出速度 */
    cc_t c_cc[NCCS];		/* 控制字符数组 */
} Termios;

/* 终端控制结构iflag位图的值. 来自 POSIX Table 7-2. 所以下面的原英文注释我应保留为好 */
#define BRKINT		0x0001	/* signal interrupt on break */
#define ICRNL		0x0002	/* map CR to NL on input */
#define IGNBRK		0x0004	/* ignore break */
#define IGNCR		0x0008	/* ignore CR */
#define IGNPAR		0x0010	/* ignore characters with parity errors */
#define INLCR		0x0020	/* map NL to CR on input */
#define INPCK		0x0040	/* enable input parity check */
#define ISTRIP		0x0080	/* mask off 8th bit > 屏蔽掉第8位 */
#define IXOFF		0x0100	/* enable start/stop input control */
#define IXON		0x0200	/* enable start/stop output control > 启用开始/停止输入控制 */
#define PARMRK		0x0400	/* mark parity errors in the input queue */

/* 终端控制结构oflag位图的值. 来自 POSIX Sec. 7.1.2.3. */
#define OPOST		0x0001	/* perform output processing > 执行输出处理 */

/* 终端控制结构cflag位图的值. 来自 POSIX Table 7-3. */
#define		CLOCAL		0x0001	/* ignore modem status lines */
#define 	CREAD		0x0002	/* enable receiver */
#define 	CSIZE		0x000C	/* number of bits per character */
#define		CS5			0x0000	/* if CSIZE is CS5, characters are 5 bits */
#define		CS6			0x0004	/* if CSIZE is CS6, characters are 6 bits */
#define		CS7			0x0008	/* if CSIZE is CS7, characters are 7 bits */
#define		CS8			0x000C	/* if CSIZE is CS8, characters are 8 bits */
#define 	CSTOPB		0x0010	/* send 2 stop bits if set, else 1 */
#define 	HUPCL		0x0020	/* hang up on last close */
#define 	PARENB		0x0040	/* enable parity on output */
#define 	PARODD		0x0080	/* use odd parity if set, else even */

/* 终端控制结构lflag位图的值. 来自  POSIX Table 7-4. */
#define ECHO		0x0001	/* enable echoing of input characters */
#define ECHOE		0x0002	/* echo ERASE as backspace > 回显ERASE(擦除)作为退格 */
#define ECHOK		0x0004	/* echo KILL > 回显终止字符 */
#define ECHONL		0x0008	/* echo NL */
#define ICANON		0x0010	/* canonical input (erase and kill enabled) */
#define IEXTEN		0x0020	/* enable extended functions > 启用扩展功能 */
#define ISIG		0x0040	/* enable signals > 启用信号 */
#define NOFLSH		0x0080	/* disable flush after interrupt or quit */
#define TOSTOP		0x0100	/* send SIGTTOU (job control, not implemented*/

/* Indices into c_cc array.  Default values in parentheses. POSIX Table 7-5. */
#define VEOF               0	/* cc_c[VEOF] = EOF char (^D) */
#define VEOL               1	/* cc_c[VEOL] = EOL char (undef) */
#define VERASE             2	/* cc_c[VERASE] = ERASE char (^H) */
#define VINTR              3	/* cc_c[VINTR] = INTR char (DEL) */
#define VKILL              4	/* cc_c[VKILL] = KILL char (^U) */
#define VMIN               5	/* cc_c[VMIN] = MIN value for timer */
#define VQUIT              6	/* cc_c[VQUIT] = QUIT char (^\) */
#define VTIME              7	/* cc_c[VTIME] = TIME value for timer */
#define VSUSP              8	/* cc_c[VSUSP] = SUSP (^Z, ignored) */
#define VSTART             9	/* cc_c[VSTART] = START char (^S) */
#define VSTOP             10	/* cc_c[VSTOP] = STOP char (^Q) */

#define _POSIX_VDISABLE	  (cc_t)0xFF	/* 您甚至无法使用“普通”键盘生成此字符。
                                         * 但是某些语言特定的键盘扫描会生成它。
                                         * 现在看来所有256个字符都已使用，因此
                                         * cc_t数组应该不大...
                                         */

/* Values for the baud rate settings.  POSIX Table 7-6. */
#define B0		    0x0000	/* hang up the line > 挂线 */
#define B50		    0x1000	/* 50 baud */
#define B75		    0x2000	/* 75 baud */
#define B110		0x3000	/* 110 baud */
#define B134		0x4000	/* 134.5 baud */
#define B150		0x5000	/* 150 baud */
#define B200		0x6000	/* 200 baud */
#define B300		0x7000	/* 300 baud */
#define B600		0x8000	/* 600 baud */
#define B1200		0x9000	/* 1200 baud */
#define B1800		0xA000	/* 1800 baud */
#define B2400		0xB000	/* 2400 baud */
#define B4800		0xC000	/* 4800 baud */
#define B9600		0xD000	/* 9600 baud */
#define B19200		0xE000	/* 19200 baud */
#define B38400		0xF000	/* 38400 baud */

/* Optional actions for tcsetattr().  POSIX Sec. 7.2.1.2. */
#define TCSANOW            1	/* changes take effect immediately */
#define TCSADRAIN          2	/* changes take effect after output is done */
#define TCSAFLUSH          3	/* wait for output to finish and flush input */

/* Queue_selector values for tcflush().  POSIX Sec. 7.2.2.2. */
#define TCIFLUSH           1	/* flush accumulated input data */
#define TCOFLUSH           2	/* flush accumulated output data */
#define TCIOFLUSH          3	/* flush accumulated input and output data */

/* Action values for tcflow().  POSIX Sec. 7.2.2.2. */
#define TCOOFF             1	/* suspend output */
#define TCOON              2	/* restart suspended output */
#define TCIOFF             3	/* transmit a STOP character on the line */
#define TCION              4	/* transmit a START character on the line */


/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int tcsendbreak, (int _fildes, int _duration)		     );
_PROTOTYPE( int tcdrain, (int _filedes)				   	     );
_PROTOTYPE( int tcflush, (int _filedes, int _queue_selector)		     );
_PROTOTYPE( int tcflow, (int _filedes, int _action)			     );
_PROTOTYPE( speed_t cfgetispeed, (const struct termios_s *_termios_p)	     );
_PROTOTYPE( speed_t cfgetospeed, (const struct termios_s *_termios_p)	     );
_PROTOTYPE( int cfsetispeed, (struct termios_s *_termios_p, speed_t _speed)    );
_PROTOTYPE( int cfsetospeed, (struct termios_s *_termios_p, speed_t _speed)    );
_PROTOTYPE( int tcgetattr, (int _filedes, struct termios_s *_termios_p)        );
_PROTOTYPE( int tcsetattr,
        (int _filedes, int _opt_actions, const struct termios_s *_termios_p)   );

#define cfgetispeed(termios_p)		((termios_p)->c_ispeed)
#define cfgetospeed(termios_p)		((termios_p)->c_ospeed)
#define cfsetispeed(termios_p, speed)	((termios_p)->c_ispeed = (speed), 0)
#define cfsetospeed(termios_p, speed)	((termios_p)->c_ospeed = (speed), 0)

#ifdef _FLYANX

/* Here are the local extensions to the POSIX standard for Minix. Posix
 * conforming programs are not able to access these, and therefore they are
 * only defined when a Minix program is compiled.
 */

/* Extensions to the termios c_iflag bit map.  */
#define IXANY		0x0800	/* allow any key to continue output ： 允许任何键继续输出 */

/* Extensions to the termios c_oflag bit map. They are only active iff
 * OPOST is enabled. */
#define ONLCR		0x0002	/* Map NL to CR-NL on output ：在输出时将NL映射到CR-NL */
#define XTABS		0x0004	/* Expand tabs to spaces */
#define ONOEOT		0x0008	/* discard EOT's (^D) on output) */

/* Extensions to the termios c_lflag bit map.  */
#define LFLUSHO		0x0200	/* Flush output. */

/* c_cc数组的扩展位 */
#define VREPRINT	  	  11	/* cc_c[VREPRINT]  = (^R) */
#define VLNEXT            12    /* cc_c[VLNEXT]    = (^V) */
#define VDISCARD          13    /* cc_c[VDISCARD]  = (^O) */

/* Extensions to baud rate settings. */
#define B57600		0x0100	/* 57600 baud */
#define B115200		0x0200	/* 115200 baud */

/* 这些是内核和一些终端设备使用的默认设置。 */
#define TCTRL_DEF	(CREAD | CS8 | HUPCL)
#define TINPUT_DEF	(BRKINT | ICRNL | IXON | IXANY)
#define TOUTPUT_DEF	(OPOST | ONLCR)
#define TLOCAL_DEF	(ISIG | IEXTEN | ICANON | ECHO | ECHOE)
#define TSPEED_DEF	B9600

#define TEOF_DEF	'\4'	/* ^D */
#define TEOL_DEF	_POSIX_VDISABLE
#define TERASE_DEF	'\10'	/* ^H */
#define TINTR_DEF	'\177'	/* ^? */
#define TKILL_DEF	'\25'	/* ^U */
#define TMIN_DEF	1
#define TQUIT_DEF	'\34'	/* ^\ */
#define TSTART_DEF	'\21'	/* ^Q */
#define TSTOP_DEF	'\23'	/* ^S */
#define TSUSP_DEF	'\32'	/* ^Z */
#define TTIME_DEF	0
#define	TREPRINT_DEF	'\22'	/* ^R */
#define	TLNEXT_DEF	'\26'	/* ^V */
#define	TDISCARD_DEF	'\17'	/* ^O */

/*
 * 窗口框架结构
 * 描述一台电脑使用终端显示信息时的屏幕信息
 */
typedef struct win_frame_s
{
    int	    row;	    /* 行，以字符为单位 */
    int 	col;	    /* 列，以字符为单位 */
    int 	x_pixel;	/* 水平尺寸，像素 */
    int  	y_pixel;	/* 垂直尺寸，像素 */
} WinFrame;

#endif /*_ FLYANX */

#endif //FLYANX_TERMIOS_H
