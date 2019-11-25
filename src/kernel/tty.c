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

#define tty_addr(line)	(&tty_table[line])  /* 宏：获得终端任务在表中的位置 */

/* Macros for magic tty structure pointers. */
// 终端任务指针操作的宏
#define FIRST_TTY	tty_addr(0)												/* 第一个终端任务的地址 */
#define END_TTY		tty_addr(sizeof(tty_table) / sizeof(tty_table[0]))		/* 最后一个终端任务的地址 */

PRIVATE Message msg;            /* 发送和接收的消息缓冲区 */

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
FORWARD _PROTOTYPE( void do_default, (void) );
FORWARD _PROTOTYPE( void dev_ioctl, (TTY *tty) );
FORWARD _PROTOTYPE( void raw_echo, (TTY *tty, int ch) );
FORWARD _PROTOTYPE( int back_over, (TTY *tty) );
FORWARD _PROTOTYPE( int echo, (TTY *tty, int ch) );
FORWARD _PROTOTYPE( void tty_init, (TTY *tty) );
FORWARD _PROTOTYPE( void reprint, (TTY *tty) );

/*===========================================================================*
 *				tty_task				     *
 *				终端任务
 *===========================================================================*/
PUBLIC void tty_task(void){
    /* 终端任务 */

    register TTY *tty;
    int mess_type;      /* 消息类型，通过其判断用户需要什么服务 */

    /* 初始化所有启用的终端 */
    for(tty = FIRST_TTY; tty < END_TTY; tty++) tty_init(tty);

    /* 终端任务初始化工作后，暂时不需要再被唤醒了 */
    tty_wake_time = TIME_NEVER;

    /* 显示Flyanx启动信息
     * 现在已经有可以交互的终端任务了，所以我们打印系统启动信息。
     * 虽然这里看到的是调用printf，但在编译时使用了把对printf库例程的调用转换为对printk的调用的宏。
     * printk使用了控制台驱动程序中的一个叫作putk的例程，所以不涉及到文件系统。这条消息只发往主控制
     * 台显示器，不能被重定向。
     */
    printf("Flyanx %s.%s  Copyright 2019 flyan@Chenu, Inc.\n", OS_RELEASE, OS_VERSION);
    printf("You computer's memory size is %ldKB, ", (total_memory_size / (1024)) );
#if (CHIP == INTEL)
    /* 运行在实模式还是16/32位保护模式下? */
#if _WORD_SIZE == 4
    printf("executing in 32-bit protected mode.\n\n");
#else
    printf("executing in %s mode\n\n",
           protected_mode ? "16-bit protected" : "real");
#endif  /* _WORD_SIZE == 4 */
#endif  /* (CHIP == INTEL) */
    /* 显示欢迎信息 */
    printf("************  **             **        **\n");
    printf("************  **              **      **\n");
    printf("**            **                **  **\n");
    printf("**            **                  **\n");
    printf("************  **                  **\n");
    printf("************  **                  **\n");
    printf("**            **                  **\n");
    printf("**            **                  **\n");
    printf("**            ************        **\n");
    printf("**            ************        **\n");
    printf("          Author QQ: 1341662010\n");
    printf("****** QQ exchange group: 909830414 ******\n");
    printf("*********** Welcome to disturb ***********\n\n");
    /* 显示现在进程的运行情况 */
    proc_dmp();
    printf("You can click <F5> to clear the screen...\n");

    /* 时钟任务主循环，一直得到工作，处理工作，回复处理结果 */
    while (TRUE){
        /* 首先检查所有的终端，看看有没有挂起的事件，如果有则调用handle_events来处理
         * 未完成的事务。
         * 一般情况下，这个事件来自于用户的键盘输入而让终端的events置位。
         */
        for(tty = FIRST_TTY; tty < END_TTY; tty++){
            if(tty->events) handle_events(tty);
        }
        /* 从外界得到一条消息 */
        receive(ANY, &msg);

        /* 提取消息类型 */
        mess_type = msg.type;

        /* 如果接收到的消息来自于硬件中断，就跳过这一次循环，重新检查事件 */
        if(mess_type == HARD_INT) continue;

        /* 根据消息类型提供不同的功能服务 */
        switch (mess_type){
            case 0:    do_default();	        break;      /* 获取从启动开始后的时钟滴答数时间 */
            case 1:      do_default();              break;      /* 获取系统时间（时间戳） */
            case 3:	    do_default();	        break;      /* 设置系统时间 */
            case 4:	    do_default();	        break;      /* 设置定时器 */
            case 5:do_default();    break;         /* 设置同步闹钟 */
            default: panic("TTY task got bad message", msg.type);  /* 当然了，获取到不识别的操作就宕机 */
        }
    }

}

PRIVATE void do_default(){
    Message tty_msg;
    tty_msg.type = 66;
    send(msg.source, &tty_msg);
}

/*===========================================================================*
 *				echo					     *
 *				回显
 *===========================================================================*/
PRIVATE int echo(
        register TTY *tty,      /* 回显到哪个终端 */
        register int ch         /* 回显的字符 */
/* return: 返回一个包含回显字符和其所用屏幕上的空格数，例如TAB字符，可以占用空格数最大到8。 */
){
    /* 回显输入的字符，这个例程很短，但也略有些复杂性。
     * echo对一些字符进行特殊处理，但对大多数字符只是在和输入使用的同一个设备的输入方显示出来。
     */

    int len, rp;
    /* 将字符的长度位复位 */
    ch & ~IN_LENGTH;
    /* 如果不在回显模式下，但是规范模式下的可回显换行符，回显换行符 */
    if(!(tty->termios.lflag & ECHO)){
        if(ch == ('\n' | IN_EOT) && (tty->termios.lflag
                                     & (ICANON | ECHONL)) == (ICANON | ECHONL) ){
            (*tty->echo)(tty, '\n');
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
    rp = tty->input_count == 0 ? FALSE : tty->status_reprint;

    /* 字符是控制字符？0~31是控制字符，32是空格符，所以这里取小于空格符的ascii码 */
    if((ch & IN_CHAR) < ' '){
        /* 根据字符判断是何控制字符 */
        switch (ch & (IN_ESCAPE|IN_EOF|IN_EOT|IN_CHAR)){
            /* 制表符：我们打印制表符长度的空格符 */
            case '\t':
                for(len = 0; len < TAB_SIZE; len++){
                    if((tty->position & TAB_MASK) != 0){    /*  */
                        (*tty->echo)(tty, ' ');
                    }
                }
                break;
                /* 换行符：长度0，将其交由设备回显例程回显。 */
            case '\r' | IN_EOT:
            case '\n' | IN_EOT:
                (*tty->echo)(tty, ch & IN_CHAR);
                len = 0;
                break;
                /* 其他控制字符：长度2，回显 '^' + char(ch + 64) 的形式，例如 ^C */
            default:
                (*tty->echo)(tty, '^');
                (*tty->echo)(tty, '@' + (ch & IN_CHAR));
                len = 2;
                break;
        }
    } else if((ch & IN_CHAR) == '\177'){    /* DEL符号？ */
        /* DEL打印为"^?" */
        (*tty->echo)(tty, '^');
        (*tty->echo)(tty, '?');
        len = 2;
    } else{                                 /* 以上都不是，说明是可回显的普通字符 */
        /* 直接回显即可 */
        (*tty->echo)(tty, ch & IN_CHAR);
        len = 1;
    }

    /* 如果是文件结束符号，我们再删掉回显的字符，回到最初的状态 */
    if(ch & IN_EOF){
        while (len > 0) {
            (*tty->echo) (tty, '\b');
            len--;
        }
    }
    tty->status_reprint = rp;
    return (ch | (len << IN_LSHIFT));
}

/*==========================================================================*
 *				raw_echo					    *
 *				原始回显
 *==========================================================================*/
PRIVATE void raw_echo(register TTY *tty, int ch){
    /* 不进行任何特殊处理的回显，如果设置了ECHO（启用回显），
     * 则进行回显，否则，本例程等于什么都没做。
     */
    int rep = tty->status_reprint;
    if(tty->termios.lflag & ECHO) (*tty->echo)(tty, ch);
    tty->status_reprint = rep;
}

/*==========================================================================*
 *				back_over				    *
 *				 退格
 *==========================================================================*/
PRIVATE int back_over(register TTY *tty){
    /* 回退到到屏幕上的前一个字符并擦除它 */
    u16_t *head;
    int len;


    if(tty->input_count == 0) return 0;     /* 终端输入队列为空，即屏幕是空的。 */
    head = tty->input_free;
    /* 如果空闲区是输入缓冲区的第一个，那么将其设置为缓冲区界限，这样---下一步就可以得到缓冲区的最后一个元素 */
    if(head == tty->input_buffer) head = buffer_end(tty->input_buffer);
    head--;     /* 得到上一个输入的字符 */
    /* 上一个字符是换行符，不能退格 */
    if(*head & IN_EOT) return 0;
    /* 如果视频内存已经被改变，那么需要重新打印该输出行 */
    if(tty->status_reprint) reprint(tty);
    /* ok，空闲区等于上一个输入的字符 */
    tty->input_free = head;
    tty->input_count--;
    /* 现在检查终端配置是否要回显擦除字符作为退格
     */
    if(tty->termios.lflag & ECHOE){
        /* 查询上一个显示字符的长度来确定需要在显示器上擦除几个字符 */
        len = (*head & IN_LENGTH) >> IN_LSHIFT;
        while (len > 0){
            /* 由rawecho发出一个退格-空格-退格字符序列从屏幕上删除不需要的字符。 */
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
PRIVATE void reprint(register TTY *tty){
//    printf("reprint...\n");
}

/*===========================================================================*
 *				dev_ioctl				     *
 *			  初始化一个终端
 *===========================================================================*/
PRIVATE void dev_ioctl(TTY *tty){
    /* 可以对终端设备进行io控制（设置设备的io速率等等）
     * 当用TCSADRAIN或TCSAFLUSH选项调用dev_ioctl时，它都支持执行tcdrain函数和tcsetattr函数。
     */
//    printf("dev_ioctl()\n");
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
        /* 控制台类型的终端，初始化它的屏幕等参数 */
        screen_init(tty);
    }
    /* 其他的，Flyanx虽然已经定义，但暂未支持 */

}

/*===========================================================================*
 *				handle_events				     *
 *			    处理挂起事件
 *===========================================================================*/
PUBLIC void handle_events(register TTY *tty){
    /* 处理终端上挂起的任何事件。这些事件通常是设备中断。
     *
     * 	do_read和do_write也调用handle_events，这个例程必须工作的很快。
     */
    /* 测试消息发送功能 */

    char *buffer;
    unsigned count;

    do{
        /* 终端此次挂起的事件正在处理，复位events */
        tty->events = FALSE;

        /* 调用终端的读操作例程 */
        (*tty->device_read)(tty);

        /* 写 */
//        (*tty->device_write)(tty);

        /* 如果有一个等待的终端io控制请求，调用dev_ioctl函数处理它 */
//        if(tty->ioc_request != FALSE) dev_ioctl(tty);

    } while (tty->events);  /* 如果处理中间events标志又被用户的输入置位，那么，接着处理。 */
}

/*===========================================================================*
 *				input_handler				     *
 *				  输入处理
 *===========================================================================*/
PUBLIC int input_handler(register TTY *tty, char *buffer, int count){
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
        if(tty->termios.cflag & IEXTEN){
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
                if(!(tty->termios.iflag & ECHOE)){  /* 要回显擦除字符吗？ */
                    (void) echo(tty, ch);
                }
                continue;
            }

            /* 终止处理（删除当前整行） */
            if(ch == tty->termios.c_cc[VKILL]) {        /* ch == 终止字符 */
                while (back_over(tty)){}
                if(!(tty->termios.iflag & ECHOE)) {     /* 要回显擦除字符吗？ */
                    (void) echo(tty, ch);
                    if( tty->termios.iflag & ECHOK){    /* 回显终止字符? */
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

        /* 启用了 开始/停止输入控制？ */
        if(tty->termios.iflag & IXON){
            /* 输出停止符号(^S)在停止输入后 */
            if(ch == tty->termios.c_cc[VSTOP]){     /* ch == 停止输入 */
                tty->status_inhibited = STOPPED;
                tty->events = TRUE;
                continue;
            }

            /* 如果在允许任何键继续输出的情况下 输出^Q 或者 任何字符 */
            if(tty->status_inhibited){      /* 停止输入状态 */
                if(ch == tty->termios.c_cc[VSTART] || (tty->termios.iflag & IXANY)){    /* 重新开始了输入或允许任何键输出 */
                    tty->status_inhibited = RUNNING;
                    tty->events = TRUE;
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
        if(tty->termios.lflag & (ECHO | ECHONL)){
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
    }
    return i;
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
            /* events置位，告诉终端任务，这个终端有一些挂起的事情必须得到处理 */
            tty->events = TRUE;
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
 *				tty_devnop				    *
 *==========================================================================*/
PUBLIC void tty_dev_nop(TTY *tty) {
    /* 有些设备不需要服务，同时，这个函数也被用在初始化例程中。 */
}


