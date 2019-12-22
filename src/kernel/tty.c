/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/18.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 终端任务
 *
 * 主要的终端任务和设备无关的支持函数都在本文件定义。
 */

#include "kernel.h"
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#if (CHIP == INTEL)
#include <flyanx/keymap.h>
#endif
#include "process.h"
#include "tty.h"

#define tty_addr(line)	(&tty_table[line])  /* 宏：获得终端的地址 */

/* 由于终端任务支持许多不同的设备，所以在一个特定的调用中需要使用次设备号
 * 来辨别哪一种设备是被支持的，下面就是这些次设备号定义
 */
#define CONSOLE_MINOR   0       /* 系统控制台 */
#define LOG_MINOR       15      /* 日志设备 */
#define RS232_MINOR	    16		/* rs232终端 */
#define TTYPX_MINOR	    128
#define PTYPX_MINOR	    192

/* Macros for magic tty structure pointers. */
// 终端任务指针操作的宏
#define FIRST_TTY	tty_addr(0)												/* 第一个终端任务的地址 */
#define END_TTY		tty_addr(sizeof(tty_table) / sizeof(tty_table[0]))		/* 最后一个终端任务的地址 */

/* 宏：终端任务是否是系统控制台任务 */
#define is_console(tty)     ((tty) < tty_addr(NR_CONSOLES))

/* 宏：终端任务是否活动且可用 */
#define tty_active(tty)     ((tty)->device_read != NULL)

/* RS-232或伪终端设备的初始化函数在没有配置这些设备时都同样地调用一个空函数。 */
#if NR_RS_LINES == 0
#define rs_init(tty)	((void) 0)
#endif
#if NR_PTYS == 0
#define pty_init(tty)	((void) 0)
#define do_pty(tty, mp)	((void) 0)
#endif

PRIVATE Message rec_msg;            /* 终端任务接收到的消息缓冲区 */
PRIVATE bool reprint_backup;        /* 重新打印的备份 */

/* 默认的终端控制结构，初始化了默认参数 */
PRIVATE Termios default_termios = {
        TINPUT_DEF, TOUTPUT_DEF, TCTRL_DEF, TLOCAL_DEF, TSPEED_DEF, TSPEED_DEF,
        {
                TEOF_DEF, TEOL_DEF, TERASE_DEF, TINTR_DEF, TKILL_DEF, TMIN_DEF,
                TQUIT_DEF, TTIME_DEF, TSUSP_DEF, TSTART_DEF, TSTOP_DEF,
                TREPRINT_DEF, TLNEXT_DEF, TDISCARD_DEF,
        }
};

/* 默认的窗口框架，全部置都为0 */
PRIVATE WinFrame default_winframe;

/* 本地函数声明 */
FORWARD _PROTOTYPE( void do_read, (TTY *tty, Message *msg) );
FORWARD _PROTOTYPE( void do_write, (TTY *tty, Message *msg) );
FORWARD _PROTOTYPE( void do_open, (TTY *tty, Message *msg) );
FORWARD _PROTOTYPE( void do_close, (TTY *tty, Message *msg) );
FORWARD _PROTOTYPE( void do_ioctl, (TTY *tty, Message *msg) );
FORWARD _PROTOTYPE( void do_cancel, (TTY *tty, Message *msg) );
FORWARD _PROTOTYPE( void do_default, (void) );
FORWARD _PROTOTYPE( void dev_ioctl, (TTY *tty) );
FORWARD _PROTOTYPE( void raw_echo, (TTY *tty, int ch) );
FORWARD _PROTOTYPE( int back_over, (TTY *tty) );
FORWARD _PROTOTYPE( int echo, (TTY *tty, int ch) );
FORWARD _PROTOTYPE( void tty_init, (TTY *tty) );
FORWARD _PROTOTYPE( void reprint, (TTY *tty) );
FORWARD _PROTOTYPE( void tty_input_cancel, (TTY *tty) );
FORWARD _PROTOTYPE( void set_attr, (TTY *tty) );
FORWARD _PROTOTYPE( void in_transfer, (TTY *tty) );
FORWARD _PROTOTYPE( void set_alarm, (TTY *tty, int on) );

/*===========================================================================*
 *				tty_task				     *
 *				终端任务
 *===========================================================================*/
PUBLIC void tty_task(void){
    /* 终端任务 */

    TTY *tty;
    int mess_type;      /* 消息类型，通过其判断用户需要什么服务 */
    unsigned int line;

    if(NR_CONSOLES > 0){
        /* 如果有控制台终端，那么先初始化键盘驱动和屏幕 */
        keyboard_init();
        screen_init();
    }
    /* 初始化所有启用的终端 */
    for(tty = FIRST_TTY; tty < END_TTY; tty++) tty_init(tty);
    if(NR_CONSOLES > 0) switch_to(0);   /* 现在选择第一个控制台 */

    /* 终端任务初始化工作后，暂时不需要再被唤醒了 */
    tty_wake_time = TIME_NEVER;

    /* 显示Flyanx启动信息
     * 现在已经有可以交互的终端任务了，所以我们打印系统启动信息。
     * 虽然这里看到的是调用printf，但在const.h里已经用宏把对printf库例程的调用转换为对printk的调用。
     * printk使用了控制台驱动程序中的一个叫作k_putk的例程，所以不涉及到文件系统。这条消息只发往主控制
     * 台显示器，不能被重定向。
     */
    printf("Flyanx %s.%s  Copyright 2019 flyan@Chenu, Inc.\n", OS_RELEASE, OS_VERSION);
#if (CHIP == INTEL)
    /* 运行在实模式还是16/32位保护模式下? */
#if _WORD_SIZE == 4
    printf("executing in 32-bit protected mode.\n\n");
#else
    printf("executing in %s mode\n\n",
           protected_mode ? "16-bit protected" : "real");
#endif  /* _WORD_SIZE == 4 */
#endif  /* (CHIP == INTEL) */
    /* 显示现在进程的运行情况和它们的内存映像 */
//    proc_dmp();
//    map_dmp();
//    printf("kernel_base: %d, kernel_limit: %d, ORIGIN size: %ld\n", kernel_base, kernel_limit,
//            proc_addr(ORIGIN_PROC_NR)->map.size);

    /* 终端任务主循环，一直得到工作，处理工作，回复处理结果 */
    while (TRUE){
        /* 首先检查所有的终端，看看有没有挂起的事件，如果有则调用相应的处理函数来处理
         * 未完成的事务。
         * 一般情况下，这个事件来自于用户的键盘输入而让终端的events置位，他也可能来自
         * 用户进程的WRITE调用和io控制（IOCTL）调用。
         * 这里的处理在之前的版本是放在一个函数里一起完成的，但是这样做效率太低，即使用
         * 户只是按了下键盘，却要对三个操作都要进行检查和执行，而且这样分开代码的阅读性
         * 也会更好，所以，我分开他们了。
         */
        for(tty = FIRST_TTY; tty < END_TTY; tty++){
            if(tty->events & EVENTS_READ) handle_read(tty);
            if(tty->events & EVENTS_WRITE) handle_write(tty);
            if(tty->events & EVENTS_IOCTL) handle_ioctl(tty);
        }
        /* 从外界得到一条消息 */
        receive(ANY, &rec_msg);

        /* 提取消息类型 */
        mess_type = rec_msg.type;

        /* 如果接收到的消息来自于硬件中断，就跳过这一次循环，重新检查事件 */
        if(mess_type == HARD_INT) continue;

        /* 检查消息中的TTY_LINE终端线域，确定是哪个设备发出的，通过对其次设备号的一系列比较被解码 */
        line = rec_msg.TTY_LINE;
        if((line - CONSOLE_MINOR) < NR_CONSOLES){
            tty = tty_addr(line - CONSOLE_MINOR);
        } else if(line == LOG_MINOR){
            tty = tty_addr(0);
        } else {
            /* flyanx暂时不支持除了控制台外的其他终端 */
            tty = NULL;
        }

        /* 当设备不存在或没有配置时，产生一条ENXIO错误消息，并返回到主循环继续处理工作。 */
        if(tty == NULL || !tty_active(tty)){
            tty_reply(TASK_REPLY, rec_msg.source, rec_msg.PROC_NR, ENXIO);
            continue;
        }

        /* 根据消息类型提供不同的功能服务 */
        switch (mess_type){
            case DEVICE_READ:       do_read(tty, &rec_msg);     break;      /* 从终端读取数据 */
            case DEVICE_WRITE:      do_write(tty, &rec_msg);    break;      /* 写入数据到终端  */
            case DEVICE_IOCTL:	    do_ioctl(tty, &rec_msg);    break;      /* 终端io控制 */
            case DEVICE_OPEN:	    do_open(tty, &rec_msg);	    break;      /* 打开一个终端设备 */
            case DEVICE_CLOSE:      do_close(tty, &rec_msg);    break;      /* 关闭一个终端设备 */
            case CANCEL:            do_cancel(tty, &rec_msg);   break;      /* 强制取消当前终端正在处理的所有任务 */
            /* 如果消息类型无效，产生一条EINVAL错误消息并发送给调用者 */
            default:    tty_reply(TASK_REPLY, rec_msg.source, rec_msg.PROC_NR, EINVAL);
        }

    }

}

/*===========================================================================*
 *				do_read					     *
 *				读终端
 *===========================================================================*/
PRIVATE void do_read(TTY *tty, Message *msg){
    /* 一个程序想从终端设备中读取数据 */

    int rs;

    /* 检查当前设备情况，如果设备仍然在等待一个输入以应付上一次的读取请求时，或者
     * 请求参数无效时，返回一个错误。
     */
    if(tty->in_left > 0){           /*仍然在等待一个输入  */
        rs = EIO;
    } else if(msg->COUNT <= 0){     /* 不读？ */
        rs = EINVAL;
    } else if(numap(msg->PROC_NR, (vir_bytes)msg->ADDRESS, msg->COUNT) == 0) {
        /* 读取的缓冲区地址有问题 */
        rs = EFAULT;
    } else {                        /* 没问题 */
        /* 将请求的信息放入到设备的中 */
        tty->in_reply_code = TASK_REPLY;
        tty->in_caller = msg->source;
        tty->in_proc = msg->PROC_NR;
        tty->in_vir = (vir_bytes) msg->ADDRESS;
        /* 至关重要，这个变量决定读请求何时得到满足。
         * 在规范模式中，in_left随着每个返回的字符逐渐减少，直到接收到一行的结束时，
         * 这个值突然减少为0。在非规范模式中，处理有所不同，但在任何情况下，只要调用
         * 被满足in_left就被置为0，不论是由于超时还是接收到了要求的最小字符数。
         * in_left达到0时，一条回答消息被送出。如同我们将要看到的那样，回答消息可以
         * 在好几个地方产生。有时还需要检查一个读进程是否仍在等待一个回答，非0时的in_left
         * 可以用作这个目的。
         */
        tty->in_left = msg->COUNT;

        /* 通过tty::termios.lflag标志判断当前终端的运行模式，ICANON置位则处于规范模式，
         * 如果这一位没有置位，处于非规范模式，也称为原始模式，则需要检查termios.VMIN和
         * termios.VTIME以决定需要采取什么动作。
         */
        if(!(tty->termios.lflag & ICANON)){     /* 原始模式 */
            if(tty->termios.c_cc[VTIME] > 0){   /* VTIME > 0，没有else，VTIME == 0将在默认在下面和规范模式一起处理 */
                if(tty->termios.c_cc[VMIN] == 0){
                    /* VMIN == 0，设置一个闹钟，即使没有收到一个字节，在闹钟响起后
                     * 也结束这次的读取请求。这里min被置为1，保证超时前收到字节就立
                     * 即终止这次读取请求，因为原始模式是一个一个传输的。
                     */
                    tty->min = 1;
                    set_alarm(tty, TRUE);
                } else {
                    /* VTIME和VMIN都不为0，那么闹钟将赋予不同的含义，在这种情况下，
                     * 闹钟用来作为字符间的计时器，它只有在收到第一个字符后启动，并在
                     * 接收到每一个后继字符之后再次重新启动。
                     * tty::eot_count字符行数计算在原始模式下含义也不相同，它对字符
                     * 计数，如果为0的话，那么就还没有接收到字符，那么闹钟将被关闭。
                     */
                    if(tty->eot_count == 0){
                        set_alarm(tty, FALSE);
                        tty->min = tty->termios.c_cc[VMIN];
                    }
                }
            }
        }

        /* 现在将现在输入队列中的字节传输到读取进程中 */
        in_transfer(tty);

        /* 完成一次传输后，我们再次调用handle_read。
         * 这里需要解释一下这个显得重复的调用。虽然到目前为止所有的讨论都从键盘输入的角度来考虑，
         * 但do_read是在代码的设备无关部分，也要为通过串行线连接的远程终端的输入服务。上一次的输
         * 入可能已经填满了设备的输入缓冲区从而导致输入被禁止，例如RS-232设备。对in_transfer的
         * 第一次调用不能重新启动输入流，但对handle_read的调用可以起到这个作用。由此引起的对
         * in_transfer的第二次调用并没有实质性的作用，主要是为了确保远程终端再次被允许发送。
         */
        handle_read(tty);

        /* tty::in_left是一个标志，它可以判断完成传输后回答是否已发送，如果这里它为0，说明已经
         * 在上面的处理中完成了消息的回复，这里就没必要了。
         */
        if(tty->in_left == 0) return;

        /* 不能指望每次处理都能完美的完成并回复，所以如果到还没有完成，我们也需要回复调用者。 */
        if(msg->TTY_FLAGS & O_NONBLOCK){
            /* 如果调用进程指定了这次读取请求不能堵塞自己，那么我们通知FS回复给调用者一个错误
             * 代码，保证它能被及时唤醒。
             */
            rs = EAGAIN;
            /* 这次请求已经完成（虽然不完美），重置读取参数。 */
            tty->in_left = tty->in_cum = 0;
        } else {
            /* 如果此次请求是普通的可堵塞读取请求，那么我们返回一个SUSPEND代码，告诉FS不要解除
             * 调用者的堵塞状态，在这种情况下，终端的下次读取请求回复代码in_reply_code将被设置
             * 为REVIVE。如果READ请求在之后被可以完成，那么这个值将被回复给FS，告诉它调用者现在
             * 可以被恢复唤醒了。
             */
            rs = SUSPEND;
            tty->in_reply_code = REVIVE;
        }
    }
    tty_reply(TASK_REPLY, msg->source, msg->PROC_NR, rs);
}

/*===========================================================================*
 *				do_write					     *
 *				写终端
 *===========================================================================*/
PRIVATE void do_write(TTY *tty, Message *msg){
    /* 一个程序想要写入数据到终端设备中，这个例程和do_read一毛一样，但是更简单，
     * 考虑的事情更少，这里的注释如果不够细致，请移步do_read。
     */

    int rs;

    /* 检查是否有进程挂在写操作上，然后检查参数是否正确，执行io */
    if(tty->out_left > 0){
        /* 终端还在完成上一次的写操作，拒绝这次请求 */
        rs = EIO;
    } else if(msg->COUNT <= 0) {
        /* 没打算写入？ */
        rs = EINVAL;
    } else if(numap(msg->PROC_NR, (vir_bytes)msg->ADDRESS, msg->COUNT) == 0){
        /* 写入缓冲区地址有问题 */
        rs = EFAULT;
    } else {
        /* 通过检查，现在将消息参数拷贝到终端结构中 */
        tty->out_reply_code = TASK_REPLY;
        tty->out_caller = msg->source;
        tty->out_proc = msg->PROC_NR;
        tty->out_vir = (vir_bytes) msg->ADDRESS;
        tty->out_left = msg->COUNT;

        /* 现在可以去写了 */
        handle_write(tty);
        if(tty->out_left == 0) return;     /* 工作完成了 */

        /* 不能写入全部的字节，如果该进程未堵塞，则挂起调用方或者中止写入。 */
        if(msg->TTY_FLAGS & O_NONBLOCK){    /* 取消写入 */
            rs = tty->out_cum > 0 ? tty->out_cum : EAGAIN;
            tty->out_left = tty->out_cum = 0;
        } else {
            rs = SUSPEND;
            tty->out_reply_code = REVIVE;
        }
    }
    /* 答复 */
    tty_reply(TASK_REPLY, msg->source, msg->PROC_NR, rs);
}

/*===========================================================================*
 *				do_ioctl					     *
 *				终端io控制
 *===========================================================================*/
PRIVATE void do_ioctl(TTY *tty, Message *msg){
    raw_echo(tty, 'i');
}

/*===========================================================================*
 *				do_open					     *
 *				打开终端
 *===========================================================================*/
PRIVATE void do_open(TTY *tty, Message *msg){
    /* 本例程只执行一个简单的基本动作
     *  - 增加设备的tty->open_count变量的值以便能够检测是否被打开
     */

    int rs = OK;
    if(msg->TTY_LINE == LOG_MINOR) {
        /* 日志设备是一个只写设备，所以如果师徒以读方式打开它，返回一个访问错误。 */
        if(msg->FLAGS & R_BIT) rs = EACCES;
    } else {
        if(!(msg->FLAGS & O_NOCTTY)){
            /* 如果O_NOCTTY标志未被置位，且不是日志设备，则该终端成为一个进程组的控制终端。
		     * 这是通过把调用方的进程号放入到p_group域完成的。
             */
            tty->p_group = msg->PROC_NR;
            rs = 1;
        }
        tty->open_count++;
    }
    /* 通知进程 */
    tty_reply(TASK_REPLY, msg->source, msg->PROC_NR, rs);
}

/*===========================================================================*
 *				do_close					     *
 *				关闭终端
 *===========================================================================*/
PRIVATE void do_close(TTY *tty, Message *msg){
    /* 关闭一个终端设备，同时会重置终端设备的属性。
     *
     * 一个终端设备可能被打开不止一次，所以这里如果该终端设备不是最后一次被关闭，
     * 那么，就只是减少open_count的值，不执行其他动作。
     * 但是如果是最后一次被关闭，那么得做一些善后的工作，即判断里面的内容。
     * 最后需要注意：msg->TTY_LINE != LOG_MINOR 这一句防止了函数试图关闭日志设备(dev/log)。
     */
    tty->open_count--;
    if(msg->TTY_LINE != LOG_MINOR && tty->open_count == 0){
        /* 善后工作开始，现在没有人使用这个终端，自然就没有控制进程了，直接赋0 */
        tty->p_group = 0;
        /* 取消设备的所有输入工作 */
        tty_input_cancel(tty);
        /* 执行设备指定的关闭例程 */
        tty->close(tty);
        /* 最后，终端和屏幕窗口两个域被重置回它们的默认值 */
        tty->termios = default_termios;
        tty->win_frame = default_winframe;
        set_attr(tty);
    }
    tty_reply(TASK_REPLY, msg->source, msg->PROC_NR, OK);
}

/*===========================================================================*
 *				do_cancel					     *
 *			取消终端正在处理的任务
 *===========================================================================*/
PRIVATE void do_cancel(TTY *tty, Message *msg){
    raw_echo(tty, ';');
}

/*===========================================================================*
 *				echo					     *
 *				回显
 *===========================================================================*/
PRIVATE int echo(
        TTY *tty,      /* 回显到哪个终端 */
        int ch         /* 回显的字符 */
/* return: 返回一个包含回显字符和其所用屏幕上的空格数，例如TAB字符，可以占用空格数最大到8。 */
){
    /* 回显输入的字符，这个例程很短，但也略有些复杂性。
     * echo对一些字符进行特殊处理，但对大多数字符只是在和输入使用的同一个设备的输入方显示出来。
     */
    int len, rch;

    /* rch = 只要字符的ascii码 */
    rch = ch & IN_CHAR;
    /* 将字符的长度位复位 */
    ch &= ~IN_LENGTH;
    /* 如果不在回显模式下，但是规范模式下的可回显换行符，回显换行符 */
    if(!(tty->termios.lflag & ECHO)){
        if(ch == ('\n' | IN_EOT) && (tty->termios.lflag & (ICANON|ECHONL)) == (ICANON|ECHONL) ){
            tty->echo(tty, '\n');
        }
        return ch;
    }
    /* 在输入被回显的同时，一个进程可能正好要向同一个设备输出，这时如果用户正试图从键盘退格，
     * 就有可能引起混乱。为了处理这种情况，在产生正常的输出时，设备指定输出例程总是把
     * tp->tty_reprint标志设置为TRUE，这样处理退格的函数就能够判断出是否产生了混合输出。
     * 由于echo也使用设备输出例程，所以tp->tty_reprint的当前值在回显时由局部变量rp保存起来。
     * 不过，如果刚开始了一个新的输入行，rp将被置为FALSE而不是保持原值以保证tp->tty_reprint
     * 在echo终止时被重置。
     */
    reprint_backup = tty->input_count == 0 ? FALSE : tty->reprint;

    /* 字符是控制字符？0~31是控制字符，32是空格符，所以这里取小于空格符的ascii码 */
    if(rch < ' '){
        /* 根据字符判断是何控制字符 */
        switch (ch & (IN_ESCAPE|IN_EOF|IN_EOT|IN_CHAR)){
            /* 制表符：我们打印制表符长度的空格符 */
            case '\t':
                len = 0;
                do {
                    tty->echo(tty, ' ');
                    len++;
                } while (len < TAB_SIZE && (tty->position & TAB_MASK) != 0);
                break;
            /* 回车和换行符：长度0，将其交由设备回显例程回显。 */
            case '\r' | IN_EOT:
            case '\n' | IN_EOT:
                tty->echo(tty, rch);
                len = 0;
                break;
            /* 其他控制字符：长度2，回显 '^' + char(ch + 64) 的形式，例如 ^C */
            default:
                tty->echo(tty, '^');
                tty->echo(tty, '@' + rch);
                len = 2;
                break;
        }

    } else if(rch == '\177'){    /* DEL符号？ */
        /* DEL打印为"^?" */
        tty->echo(tty, '^');
        tty->echo(tty, '?');
        len = 2;
    } else {                                 /* 以上都不是，说明是可回显的普通字符 */
        /* 直接回显即可 */
        tty->echo(tty, rch);
        len = 1;
    }

    /* 如果该字符标志着文件结束，则光标回退该字符的长度次 */
    if(ch & IN_EOF){
        while (len > 0) {
            tty->echo(tty, '\b');
            len--;
        }
    }
    tty->reprint = reprint_backup;
    ch |= (len << IN_LSHIFT);
    return ch;
}

/*==========================================================================*
 *				raw_echo					    *
 *				原始回显
 *==========================================================================*/
PRIVATE void raw_echo(TTY *tty, int ch){
    /* 不进行任何特殊处理的回显，如果设置了ECHO（启用回显），
     * 则进行回显，否则，本例程等于什么都没做。
     */
    reprint_backup = tty->reprint;
    if(tty->termios.lflag & ECHO) {
        tty->echo(tty, ch);
    }
    tty->reprint = reprint_backup;
}

/*==========================================================================*
 *				back_over				    *
 *				 退格
 *==========================================================================*/
PRIVATE int back_over(TTY *tty){
    /* 回退到到屏幕上的前一个字符并擦除它 */
    u32_t *free;
    int len;

    if(tty->input_count == 0) return 0;     /* 终端输入队列为空，即屏幕是空的。 */
    free = tty->input_free;
    /* 如果空闲区是输入缓冲区的第一个，那么将其设置为缓冲区界限，这样---下一步就可以得到缓冲区的最后一个元素 */
    if(free == tty->input_buffer) free = buffer_end(tty->input_buffer);
    /* 上一个字符是换行符，不能退格 */
    if(*--free & IN_EOT) return 0;
    /* 如果视频内存已经被改变，那么需要重新打印该输出行 */
    if(tty->reprint) reprint(tty);
    /* ok，空闲区等于上一个输入的字符 */
    tty->input_free = free;
    tty->input_count--;
    /* 现在检查终端配置是否要回显擦除字符作为退格
     */
    if(tty->termios.lflag & ECHOE){
        /* 查询上一个显示字符的长度来确定需要在显示器上擦除几个字符 */
        len = (*free & IN_LENGTH) >> IN_LSHIFT;
        while (len > 0){
            /* 由rawecho发出一个退格字符从屏幕上删除不需要的字符。 */
            raw_echo(tty, '\b');
            raw_echo(tty, ' ');
            raw_echo(tty, '\b');
            len--;
        }
    }

    return 1;       /* 一个字符已擦除成功 */
}

/*==========================================================================*
 *				reprint					    *
 *				重新打印
 *==========================================================================*/
PRIVATE void reprint(TTY *tty){
    /* 重新打印之前回显到屏幕上的内容
     * 如果用户的输入已经被输出弄乱了，或者输入了REPRINT(^R)，这个
     * 例程将被调用。
     */

//    int count;
//    u16_t *free;
//
//    /* 处理了，复位 */
//    tty->reprint = FALSE;
//
//    free = tty->input_free;
//    count = tty->input_count;
//    /* 逆向查找输入中的最后一个换行符。 */
//    while (count > 0) {
//        /* 越界检查 */
//        if(free == tty->input_buffer) free = buffer_end(tty->input_buffer);
//        /* 找到换行符 */
//        if((*free - 1) & IN_EOT) break;
//        free--;
//        count--;
//    }
//    /* 没有找到上一个换行符，没有理由去重新打印 */
//    if(count == tty->input_count) return;
//
//    /* 回显CTRL-R（^R），然后移动到下一行 */
//    (void) echo(tty, tty->termios.c_cc[VREPRINT] | IN_ESCAPE);
//    raw_echo(tty, '\r');
//    raw_echo(tty, '\n');
//
//    /* 重新打印队列中从上一行终止符到末尾的部分 */
//    do{
//        /* 越界检查 */
//        if(free == buffer_end(tty->input_buffer)) free = tty->input_buffer;
//        *free = echo(tty, *free);
//        free++;
//        count++;
//    } while (count < tty->input_count);

}

/*===========================================================================*
 *				in_transfer				     *
 *			     输入传输
 *===========================================================================*/
PRIVATE void in_transfer(TTY *tty){
    /* 将数据从任务存储空间中的输入队列移动到请求输入的用户进程的缓冲区中。然而，直接拷贝
     * 是不行的，输入队列是一个环形缓冲区，需要检查字符以判断是否已经达到文件末尾，或者检查
     * 是否处于规范模式下，传送只持续到一行结束。另外，输入队列中各单元大小为16位，而接收缓
     * 冲区是一个8位字符的数组。这样就需要一个中间局部缓冲区。字符在放入局部缓冲区时被逐个检
     * 查，当缓冲区被填满或者输入队列被移空时，就调用phys_copy把局部缓冲区中的内容移到接收
     * 进程的缓冲区中。
     */

    int ch;
    int count;
    phys_bytes buffer_phys, user_base;
    char buffer[64], *pb;

    /* 如果终端线路已经挂断，那么强制让读取成功，对于读取方就像遇到了一个EOF文件结束符 */
    if(tty->termios.ospeed == B0) tty->min = 0;

    /* 首先判断tty->inleft == 0，那么说明已经处理完毕并发送了一条回发消息，所以这里直接
     * 可以返回了，不需要再继续往下进行处理并回发消息了。
     *
     * 而检测的下一部分比较tty->eot_count和tty->min两个变量。在规范模式中，这两个变量与
     * 完整的输入行有关；而在非规范模式中，它们与字符有关。只要一个“行中止”字符或一个字节
     * 被放入输入队列，就增加tty->eot_count的值。这样tty->eot_count就可以对已被终端任
     * 务接收但还未传给一个读进程的行或字节计数。tty->min则指示了满足一个读请求所必须传送
     * 的最小的行数（规范模式）或字符数（非规范模式）。在规范模式中它的值总是为1，在非规范
     * 模式中，它可以是从0到MAX_INPUT（Flyanx中被设置为255）之间的任何一个值。第二部分
     * 的检测使得在规范模式下如果没有接收到一个整行，本例程就立即返回。传送并不马上执行，
     * 而是等到一行结束之后，这样队列的内容就可以被修改。例如，如果用户在按下ENTER之前接着
     * 输入ERASE或KILL字符，那么将丢弃队列中的内容。在非规范模式中，如果无法取得所需的最小
     * 字符数，就立即返回。
     */
    if(tty->in_left == 0 || tty->eot_count < tty->min) return;

    buffer_phys = vir2phys(buffer);
    user_base = proc_vir2phys(proc_addr(tty->in_proc), 0);
    pb = buffer;

    /* 开始传输 */
    while (tty->in_left > 0 && tty->eot_count > 0){
        /* 拿到要处理的字符 */
        ch = *tty->input_handle;
        /* 输入队列中的字符大小为16位，实际传给用户进程的是低8位。
         * 而各个位的使用方法如下(15~0)：
         * 	15：不做任何用途
         * 	14：V、IN_ESC，标明是否转义（CTRL-V）
         * 	13：D、IN_EOF，标明文件结束符（CTRL-D）
         * 	12：N：IN_EOT，标明换行符(NL 和 others)
         * 	11~8：cccc、标明计数字符回显时占据的屏幕空间
         * 	7：掩码，有可能会被置为0，在ISTRIP位被设置的时候。
         * 	6~0：ascll码
         */

        /* 检测到文件结束符 */
        if(!(ch & IN_EOF)){
            /* 得到该字符的ascii码 */
            *pb = ch & IN_CHAR;
            tty->in_left--;
            pb++;       /* 指向缓冲区下一个位置 */
            if(pb == buffer_end(buffer)){
                /* 如果处理的字符到达缓冲区的边界，那么现在复制到用户空间，因为实在放不下了 */
                phys_copy(buffer_phys, user_base + tty->in_vir,
                        (phys_bytes) buffer_len(buffer));
                tty->in_vir += buffer_len(buffer);
                tty->in_cum += buffer_len(buffer);
                pb = buffer;        /* 重新指回缓冲区的头部 */
            }
        }

        /* 从输入队列中删除已经处理了的字符 */
        tty->input_handle++;
        if(tty->input_handle == buffer_end(tty->input_buffer)){
            tty->input_handle = tty->input_buffer;
        }
        tty->input_count--;

        /* 检测到换行符 */
        if(ch & IN_EOT){
            tty->eot_count--;       /* 字符行数 - 1 */
            /* 判断当前是否是否是规范模式，是的话，inleft直接归零，退出循环，一次传输结束。 */
            if(tty->termios.lflag & ICANON) tty->in_left = 0;
        }
    }

    if(pb > buffer){
        /* 处理临时缓冲区中剩下的字符，它们少于一个缓冲区的最大量 */
        count = pb - buffer;
        phys_copy(buffer_phys, user_base + tty->in_vir, (phys_bytes) count);
        tty->in_vir += count;
        tty->in_cum += count;
    }

    /* 如果这次的传输已经完成，那么发送一条回答消息，在规范模式下总是这种情况。
     * 但如果处于非规范模式并且传送的字符少于全部的请求，那么将不发送回答。这
     * 时候的回答将在这个函数调用外面去发送。
     */
    if(tty->in_left == 0) {
        tty_reply(tty->in_reply_code, tty->in_caller, tty->in_proc, tty->in_cum);
        tty->in_left = tty->in_cum = 0;
    }
}

/*===========================================================================*
 *				dev_ioctl				     *
 *			    设备io控制
 *===========================================================================*/
PRIVATE void dev_ioctl(TTY *tty){
    /* 可以对终端设备进行io控制（设置设备的io速率等等）
     * 当用TCSADRAIN或TCSAFLUSH选项调用dev_ioctl时，它都支持执行tcdrain函数和tcsetattr函数。
     */
    phys_bytes user_phys;

    /* 输出处理未完成，不能对设备进行控制 */
    if(tty->out_left > 0) return;

    if(tty->ioc_request != TCDRAIN){
        /* tcsetattr（TCSETSF请求）在TCSAFLUSH选项下调用终端的tty_input_cancel例程取消输入 */
        if(tty->ioc_request == TCSETSF) tty_input_cancel(tty);
        /* 得到调用程序的termios结构所在的物理地址 */
        user_phys = proc_vir2phys(proc_addr(tty->ioc_proc), tty->ioc_vir_addr);
        /* 拷贝其到该设备的termios中 */
        phys_copy(user_phys, vir2phys(&tty->termios), (phys_bytes) sizeof(tty->termios));
        set_attr(tty);  /* 配置生效 */
    }
    /* 为tcdrain服务的唯一动作：重置tp->ioc_request域并向FS发送回答消息
     * 当然，tcsetattr服务也需要这一步动作。
     */
    tty->ioc_request = 0;
    tty_reply(REVIVE, tty->ioc_caller, tty->ioc_proc, OK);  /* 回复调用者 */
}

/*===========================================================================*
 *				tty_init				     *
 *			  初始化一个终端
 *===========================================================================*/
PRIVATE void tty_init(TTY *tty) {
    /* 初始化终端结构并调用设备初始化例程 */

    /* 初始化终端基本数据结构 */
    tty->input_handle = tty->input_free = tty->input_buffer;
    tty->min = 1;
    tty->termios = default_termios;
    /* input_cancel、output_cancel、ioctl、close指向一个什么都不做的空函数 */
    tty->input_cancel = tty->output_cancel = tty->ioctl = tty->close = tty_dev_nop;

    /* 根据不同类型的终端（系统控制台、串行线、伪终端）调用设备指定的初始化函数，
     * 它们再间接的设置调用设备指定函数实际的指针。
     */
    if(tty < tty_addr(NR_CONSOLES)){
        /* 将控制台和键盘输入绑定 */
        keyboard_bind_tty(tty);
        /* 初始化控制台，并将其和终端绑定 */
        console_init(tty);
    }
    /* 其他的，Flyanx虽然已经定义，但暂未支持 */

}

/*===========================================================================*
 *				handle_read				     *
 *			处理挂起的终端设备读操作
 *===========================================================================*/
PUBLIC void handle_read(TTY *tty){
    do{
        tty->events &= ~EVENTS_READ;
        /* 调用终端的读操作例程 */
        tty->device_read(tty);
    } while (tty->events & EVENTS_READ);    /* 如果处理中间标志又被用户的输入置位，那么，接着处理。 */

    /* 把字符从输入队列中传送到调用一个读操作的进程的内部缓冲区中。
     * 传输完成后，无论是传输了请求的最大字符数还是达到了一行的末尾（规范模式中），
     * in_transfer本身就会发送一条回答消息，如果是这样的情况，那么在返回到本例程
     * 时，tty->in_left的值就为0。
     */
    in_transfer(tty);

    /* 传输字符数达到了用户请求的最小数目，则发送一条回答消息，
     * 后面的tty->in_left > 0 的检测是为了防止即使完成后，
     * 已经发送了一条回复消息，但再次进来，即使用户没有请求，
     * 却还是再次发送一条消息。
     */
    if(tty->in_cum >= tty->min && tty->in_left > 0){
        tty_reply(tty->in_reply_code, tty->in_caller, tty->in_proc, tty->in_cum);
        /* 完成，in_left和in_cum还原为0， */
        tty->in_left = tty->in_cum = 0;
    }
}

/*===========================================================================*
 *				handle_write				     *
 *			 处理挂起的终端设备写操作
 *===========================================================================*/
PUBLIC void handle_write(TTY *tty){
    do{
        tty->events &= ~EVENTS_WRITE;
        /* 调用终端的写操作例程 */
        (*tty->device_write)(tty);
    } while (tty->events & EVENTS_WRITE);   /* 如果处理中间标志又被用户的操作置位，那么，接着处理。 */
}

/*===========================================================================*
 *				handle_ioctl    		     *
 *			 处理挂起的终端设备io控制操作
 *===========================================================================*/
PUBLIC void handle_ioctl(TTY *tty){
    do{
        tty->events &= ~EVENTS_IOCTL;
        /* 如果有一个等待的终端io控制请求，调用dev_ioctl函数处理它 */
        if(tty->ioc_request != FALSE) dev_ioctl(tty);
    } while (tty->events & EVENTS_IOCTL);   /* 如果处理中间标志又被用户的操作置位，那么，接着处理。 */
}

/*===========================================================================*
 *				input_handler				     *
 *				  输入处理
 *===========================================================================*/
PUBLIC int input_handler(
        TTY *tty,
        char *buffer,
        int count
){
    /* 本例程是设备指定软件调用来执行对所有输入都要进行的共同处理。
     * 对刚刚输入的字符进行处理、保存并回显它们。返回处理的字符数。
     */

    int ch, sig, i;
    int time_set = FALSE;
    static unsigned char csize_mask[] = { 0x1F, 0x3F, 0x7F, 0xFF };
    /* 遍历输入的数据 */
    for(i = 0; i < count; i++){
        /* 得到缓冲区的字符 */
        ch = *buffer & BYTE;
        buffer++;

        /* 如果ISTRIP置位，那么字符第8位将会被屏蔽。（POSIX标准要求） */
        if(tty->termios.cflag & ISTRIP) ch &= 0x7F;

        /* 启用了输入扩展功能？（POSIX标准要求） */
        if(tty->termios.lflag & IEXTEN){
            /* 前一个字符是字符转义吗？ */
            if(tty->status_escaped){
                tty->status_escaped = NOT_ESCAPED;  /* 转义状态复位 */
                ch |= IN_ESCAPE;            /* 将字符中的转义标志置位 */
            }

            /* 下一个字符需要转义吗？根据字符中的转义标志判断 */
            if(ch == tty->termios.c_cc[VLNEXT]){
                tty->status_escaped = ESCAPED;      /* 转义状态置位 */
                raw_echo(tty, '^');
                raw_echo(tty, '\b');
                continue;
            }

            /* 重新打印回显字符？ */
            if(ch == tty->termios.c_cc[VREPRINT]){
                reprint(tty);
                continue;
            }
        }

        /* _POSIX_VDISABLE是一个普通的字符值，所以最好转义它。 */
        if (ch == _POSIX_VDISABLE) ch |= IN_ESCAPE;

        /* 根据终端控制结构的值，确定映射CR到LF，忽略CR，或映射LF到CR。 */
        if(ch == '\r'){
            if(tty->termios.iflag & IGNCR) continue;
            if(tty->termios.iflag & ICRNL) ch = '\n';
        } else if(ch == '\n'){
            if(tty->termios.iflag & INLCR) ch = '\r';
        }

        /* 终端处在规范模式？ */
        if(tty->termios.lflag & ICANON){
            /* 擦除处理(擦除最后一个字符)。 */
            if(ch == tty->termios.c_cc[VERASE]){    /* ch == 擦除字符 */
                (void) back_over(tty);
                if(!(tty->termios.lflag & ECHOE)){  /* 要回显擦除字符吗？ */
                    (void) echo(tty, ch);
                }
                continue;
            }

            /* 终止处理（删除当前整行） */
            if(ch == tty->termios.c_cc[VKILL]) {        /* ch == 终止字符 */
                while (back_over(tty)){}
                if(!(tty->termios.lflag & ECHOE)) {     /* 要回显擦除字符吗？ */
                    (void) echo(tty, ch);
                    if( tty->termios.lflag & ECHOK){    /* 回显终止字符? */
                        raw_echo(tty, '\n');
                    }
                    continue;
                }
            }

            /* EOF (^D)表示文件结束，一个不可见的“换行符”。 */
            if(ch == tty->termios.c_cc[VEOF]) ch |= (IN_EOT | IN_EOF);

            /* 行可以在换行符之后返回给用户。 */
            if(ch == '\n') ch |= IN_EOT;

            /* EOL和EOT一样，不管它在什么情况下。 */
            if (ch == tty->termios.c_cc[VEOL]) ch |= IN_EOT;
        }

        /* 启用了 开始/停止 输入控制？ */
        if(tty->termios.iflag & IXON){
            /* 输出停止符号(^S)在停止输入后 */
            if(ch == tty->termios.c_cc[VSTOP]){     /* ch == 停止输入 */
                tty->status_inhibited = STOPPED;
                tty->events |= EVENTS_WRITE;
                continue;
            }

            /* 如果在允许任何键继续输出的情况下 输出^Q 或者 任何字符 */
            if(tty->status_inhibited){      /* 停止输入状态 */
                if(ch == tty->termios.c_cc[VSTART] || (tty->termios.iflag & IXANY)){    /* 重新开始了输入或允许任何键输出 */
                    tty->status_inhibited = RUNNING;
                    tty->events |= EVENTS_WRITE;
                    if(ch == tty->termios.c_cc[VSTART]) /* ch == 开始输入 */
                        continue;
                }
            }
        }

        /* 启用信号？ @TODO 信号机制未实现 */
//        if(tty->termios.lflag & ISIG){
//            /* 检查INTR(^?)和QUIT(^\)字符。 */
//            if (ch == tty->termios.c_cc[VINTR]
//                || ch == tty->termios.c_cc[VQUIT]) {
//                sig = SIGINT;
//                if (ch == tty->termios.c_cc[VQUIT]) sig = SIGQUIT;
//                sigchar(tty, sig);
//                (void) echo(tty, ch);
//                continue;
//            }
//        }

        /* 输入缓冲区满了？ */
        if(tty->input_count == buffer_len(tty->input_buffer)){
            if(tty->termios.lflag & ICANON){    /* 规范模式下，丢弃该字符 */
                continue;
            } else break;   /* 原始模式（非规范模式）下，保留该字符 */
        }

        /* 在原始模式？ */
        if(!(tty->termios.lflag & ICANON)){
            /* 原始模式下，所有字符都被当做换行符来看待。 */
            ch |= IN_EOT;

            /* 启动一个字节间计时器？ @TODO 设置定时器功能未实现 */
//            if(!time_set && tty->termios.c_cc[VMIN] > 0
//                && tty->termios.c_cc[VTIME] > 0){
//                interrupt_lock();
//                set_timer(tty, TRUE);
//                interrupt_unlock();
//                time_set = TRUE;
//            }
        }

        /* 执行复杂的回显功能。 */
        if(tty->termios.lflag & (ECHO|ECHONL)){
            ch = echo(tty, ch);
        }

        /* 将字符保存在输入队列中。 */
        *tty->input_free = ch;
        tty->input_free++;
        /* 如果输入队列已经满了，丢弃这个字符，下次将从队列头存放字符 */
        if(tty->input_free == buffer_end(tty->input_buffer)){
            tty->input_free = tty->input_buffer;
        }
        tty->input_count++;
        /* 如果该字符有换行符，则换行计数器eot_count + 1 */
        if(ch & IN_EOT) tty->eot_count++;

        /* 如果刚送入队列的字符填满了队列，那么我们调用in_transfer处理它们，将他们传输给设备 */
        if(tty->input_count == buffer_len(tty->input_buffer)) in_transfer(tty);
    }
    return i    ;
}

/*===========================================================================*
 *				tty_reply				     *
 *			    终端任务回复
 *===========================================================================*/
PUBLIC void tty_reply(
        int code,               /* TASK_REPLY（任务回复） 或 REVIVE（恢复进程） */
        int reply_dest,         /* 直接回复的目标地址，是一个进程索引号，一般是服务 */
        int proc_nr,            /* 告诉服务应该回复给谁？一般是一个用户进程 */
        int status              /* 回复代码 */
){
    /* 构造并发送一条消息。如果因为某种原因回答失败，就会出错停止内核运行。 */

    Message tty_msg;

    /* 设置回复代码 */
    tty_msg.type = code;
    /* 设置回复 */
    tty_msg.REPLY_PROC_NR = proc_nr;
    tty_msg.REPLY_STATUS = status;
    status = send(reply_dest, &tty_msg);
    if(status != OK){
        panic("tty_reply failed, status\n", status);
    }
}

/*===========================================================================*
 *				tty_wakeup				     *
 *				终端任务唤醒
 *===========================================================================*/
PUBLIC void tty_wakeup(clock_t now){
    /* 这个例程虽然短，但在终端任务的功能中却十分重要。只要时钟中断处理程序运行，也就是说，对每个时钟滴答，
     * 全局变量tty_timeout（在global.h中定义）都被检查是否包含小于当前时间的值。如果是，那么就调用本例
     * 程唤醒终端任务。这保证了用户键盘输入数据后，终端任务能及时响应并处理。
     */

    TTY *tty;

    /* 把下次终端唤醒的时间置为TIME_NEVER，这是一个未来非常远的时间，以禁止下一次的唤醒。 */
    tty_wake_time = TIME_NEVER;
    /* 扫描按唤醒时间从小到大排序的终端链表，直到发现有终端的唤醒时间比当前时间还要迟的那个时
     * 间，这个终端就是我们想要下一次要唤醒的终端。然后将它的唤醒时间放入tty_wake_time中。
     */
    tty = tty_wake_list;
    while (tty != NULL){
        if(tty->wake_time > now){   /* 唤醒时间比当前时间迟 */
            /* 发现了下一次应该唤醒终端任务的时间 */
            tty_wake_time = tty->wake_time;
            break;
        } else {
            /* 唤醒时间比当前时间早（已经发生）
             * 在这，我们发现了即将会被唤醒的终端，因为它的唤醒时间早已过去。
             * 下面，我们对即将唤醒的终端做一些处理。
             */

            /* tty_min设置为0，以保证即使没有收到字符下一次用户的读终端操作也能成功 */
            tty->min = 0;
            /* events置位，告诉终端任务，这个终端有一些挂起的事情必须得到处理，好像没有必要... */
//            tty->events |= EVENTS_ALL;
            /* 把这个终端从唤醒终端链表中移除 */
            tty_wake_list = tty->next_wake;
            /* 继续往下寻找 */
            tty = tty_wake_list;
        }
    }

    /* 现在可以触发一个中断，唤醒终端任务了 */
    interrupt(TTY_TASK);
}

/*==========================================================================*
 *				tty_input_cancel				    *
 *				取消设备的所有输入
 *==========================================================================*/
PRIVATE void tty_input_cancel(register TTY *tty){
    /* 丢弃所有挂起的输入、终端缓冲区或设备。 */

    /* 删除已在排队的输入-接收到的字符或行的计数被置为0  */
    tty->input_count = tty->eot_count = 0;
    /* 队列的处理指针和空闲输入指针重合。 */
    tty->input_handle = tty->input_free;
    /* 所有输出任务停止 */
    tty->input_cancel(tty);
}

/*===========================================================================*
 *				set_attr					     *
 *			应用新的终端属性
 *===========================================================================*/
PRIVATE void set_attr(TTY *tty){
    /* 这些属性例如：标准模式/原始模式，传输速度等等... */

    u32_t *inp;
    int count;

    if (!(tty->termios.cflag & ICANON)) {
        /* 处于非规范模式：第一个动作就是将当前输入队列中的所有字符都标上IN_EOF位，
         * 如同在处于非规范模式下这些字符刚被输入到队列中时所要做的一样。直接这样做
         * 要比检测字符是否有该位容易一些。
         */
        count = tty->eot_count = tty->input_count;
        inp = tty->input_handle;
        while (count > 0) {
            *inp |= IN_EOT;
            inp++;
            if (inp == buffer_end(tty->input_buffer)) inp = tty->input_buffer;
            count--;
        }
    }

    /* 检测MIN和TIME值 */
    set_alarm(tty, FALSE);  /* 先将该终端设备的闹钟关闭 */

    if(tty->termios.cflag & ICANON){
        /* 规范模式下，没有这两个值，所以tty->min总是1 */
        tty->min = 1;
    } else {
        /* 非规范模式：两个值的组合允许四种不同的操作，需要判断 */

        /* 首先tty->min置为传过来的值 */
        tty->min = tty->termios.c_cc[VMIN];
        /* 然后，如果min为0且传过来的time值大于0，那么min置为1 */
        if(tty->min == 0 && tty->termios.c_cc[VTIME] > 0){
            tty->min = 1;
        }
    }

    if(!(tty->termios.cflag & IXON)){
        /* 如果关闭了开始/停止输入控制，输出就不停止 */
        tty->status_inhibited = RUNNING;
        tty->events = EVENTS_ALL;
    }

    /* 如果输出速度被设置为0，发送一个SIGUP信号给使用该终端的所有人@TODO */

    /* 执行一个由tty->ioctl指向的设备指定例程的间接调用，来完成只能在设备层完成的工作。 */
    tty->ioctl(tty);
}

/*==========================================================================*
 *				set_alarm				    *
 *		    为终端设备设置一个闹钟
 *==========================================================================*/
PRIVATE void set_alarm(
        TTY *tty,       /* 谁要设置闹钟？ */
        int on          /* 打开还是关闭？TRUE打开，FALSE关闭。 */
){
    /* 设置一个闹钟以便设备知道在原始模式中何时从一个READ读取调用
     * 中返回并结束。这里有一点需要注意，由于唤醒终端任务对硬件中
     * 断十分敏感，所以我们在操作时需要先关闭中断，操作完毕再打开
     * 中断。
     */

    TTY **tdp;

    /* 关中断 */
    interrupt_lock();
    /* 在唤醒列表中寻找要设置的设备是否已经存在 */
    for(tdp = &tty_wake_list; *tdp != NULL; tdp = &(*tdp)->next_wake){
        if(tty == *tdp){
            /* 找到了，删除，因为要重新设置了 */
            *tdp = tty->next_wake;
            break;
        }
    }

    /* 关闭闹钟？那么好了，没事情了。 */
    if(!on) return;

    /* 计算闹钟时：当前时间加上在设备的TIME（在termios结构中）值，
     * TIME需要换算成时钟滴答值，别忘了这点。
     */
    tty->wake_time = get_uptime() + tty->termios.c_cc[VTIME] * ONE_TICK_MILLISECOND;

    /* 在唤醒列表中找到一个合适的位置加入进去 */
    for(tdp = &tty_wake_list; *tdp != NULL; tdp = &(*tdp)->next_wake){
        /* 唤醒时间前面的，都跳过 */
        if(tty->wake_time <= (*tdp)->wake_time) break;
    }
    /* 找到了，放到这 */
    tty->next_wake = *tdp;
    *tdp = tty;
    /* 如果新设置的闹钟时间比下一次要响起的更早，那么下一次响起就是它了 */
    if(tty->wake_time < tty_wake_time) tty_wake_time = tty->wake_time;
    /* 开中断 */
    interrupt_unlock();
}

/*==========================================================================*
 *				tty_devnop				    *
 *		有些设备不需要服务，同时，这个函数也被用在初始化例程中。
 *==========================================================================*/
PUBLIC void tty_dev_nop(TTY *tty) {}


