/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * PC 和 AT键盘系统任务（驱动程序）
 */

#include "kernel.h"
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include <flyanx/keymap.h>
#include "tty.h"
#include "keymaps/us-std.src"

/* 标准键盘和AT键盘 */
#define KEYBORD_DATA     0x60    /* 键盘数据的I/O端口，用于和键盘控制器的底层交互。 */

/* AT键盘 */
#define KEYBOARD_COMMAND    0x64    /* AT上的命令I/o端口 */
#define KEYBOARD_STATUS     0x64    /* AT上的状态I/O端口 */
#define KEYBOARD_ACK        0xFA    /* 键盘相应确认 */

#define KEYBOARD_OUT_FULL   0x01    /* 字符按键按下时该状态位被设置 */
#define KEYBOARD_IN_FULL    0x02    /* 未准备接收字符时该状态位被设置 */
#define LED_CODE            0xED    /* 设置键盘灯的命令 */
#define MAX_KEYBOARD_ACK_RETRIES    0x1000  /* 等待键盘响应的最大等待时间 */
#define MAX_KEYBOARD_BUSY_RETRIES   0x1000  /* 键盘忙时循环的最大时间 */
#define KEY_BIT             0x80    /* 将字符打包传输到键盘的位 */

/* 其他用途 */
#define ESC_SCAN	        0x01	/* 重启键，当宕机时可用 */
#define SLASH_SCAN	        0x35	/* 识别小键盘区的斜杠 */
#define RSHIFT_SCAN	        0x36	/* 区分左移和右移 */
#define HOME_SCAN	        0x47	/* 数字键盘上的第一个按键 */
#define INS_SCAN	        0x52	/* INS键，为了使用CTRL-ALT-INS重启快捷键 */
#define DEL_SCAN	        0x53	/* DEL键，为了使用CTRL-ALT-DEL重启快捷键 */
#define CONSOLE		        0	    /* 控制台行号 */
#define MEMCHECK_ADR        0x472	/* 重启后的停止内存检查的地址 */
#define MEMCHECK_MAG        0x1234	/* 停止内存检查的魔数 */

#define KEYBOARD_IN_BYTES	  32	/* 键盘输入缓冲区的大小 */

PRIVATE char input_buffer[KEYBOARD_IN_BYTES];	/* 键盘输入缓冲区 */
PRIVATE char *input_free = input_buffer;	    /* 指向输入缓冲区的下一个空闲点 */
PRIVATE char *input_handle = input_buffer;	    /* 指向应该被处理并返回给终端的扫描码 */
PRIVATE unsigned int input_count;			    /* 多少扫描码在缓冲区中？ */

/* 当前键盘所处的各种状态，解释一个按键需要使用这些状态 */
PRIVATE int esc;		        /* 是一个转义扫描码？收到一个转义扫描码时，被置位 */
PRIVATE int alt_left;		    /* 左ALT键状态 */
PRIVATE int alt_right;		    /* 右ALT键状态 */
PRIVATE int alt;		        /* ALT键状态，不分左右 */
PRIVATE int ctrl_left;		    /* 左CTRL键状态 */
PRIVATE int ctrl_right;		    /* 右CTRL键状态 */
PRIVATE int ctrl;		        /* CTRL键状态，不分左右 */
PRIVATE int shift_left;		    /* 左SHIFT键状态 */
PRIVATE int shift_right;        /* 右SHIFT键状态 */
PRIVATE int shift;		        /* SHIFT键状态，不分左右 */
PRIVATE int num_down;		    /* 数字锁定键(数字小键盘锁定键)按下 */
PRIVATE int caps_down;		    /* 大写锁定键按下 */
PRIVATE int scroll_down;	    /* 滚动锁定键按下 */
PRIVATE int locks[NR_CONSOLES];	/* 每个控制台的锁定键状态 */

/* 锁定键激活位，应该要等于键盘上的LED灯位 */
#define SCROLL_LOCK	    0x01    /* 二进制：0001 */
#define NUM_LOCK	    0x02    /* 二进制：0010 */
#define CAPS_LOCK	    0x04    /* 二进制：0100 */

/* 数字键盘的转义字符映射 */
PRIVATE char numpad_map[] =
        {'H', 'Y', 'A', 'B', 'D', 'C', 'V', 'U', 'G', 'S', 'T', '@'};

FORWARD _PROTOTYPE( unsigned int key_make_break, (int scan_code) );
FORWARD _PROTOTYPE( void check_cad, (int scan_code) );
FORWARD _PROTOTYPE( void set_led, (void) );
FORWARD _PROTOTYPE( int function_key, (int scan_code) );
FORWARD _PROTOTYPE( int scan_keyboard, (void) );
FORWARD _PROTOTYPE(  void keyboard_read, (TTY *tty) );
FORWARD _PROTOTYPE( int keyboard_wait, (void) );
FORWARD _PROTOTYPE( int keyboard_ack, (void) );
FORWARD _PROTOTYPE( unsigned map_key, (int scan_code) );
FORWARD _PROTOTYPE( int keyboard_handler, (int irq) );


/*===========================================================================*
 *				keyboard_init					     *
 *				键盘驱动初始化
 *===========================================================================*/
PUBLIC void keyboard_init(void){
    /* 初始化一个键盘驱动程序，同时将该键盘和一个终端绑定 */

    /* 设置键盘灯 */
    set_led();

    /* 扫描键盘以确保没有残余的读入，将残余的键码都丢弃掉 */
    scan_keyboard();

    /* 设定键盘中断处理程序并打开键盘中断 */
    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);
}

/*===========================================================================*
 *				keyboard_bind_tty					     *
 *			 将键盘绑定到一个控制台终端上
 *===========================================================================*/
PUBLIC void keyboard_bind_tty(TTY *tty){
    /* 设置终端设备的读取函数，也是终端输入函数，即从哪里获取键码 */
    tty->device_read = &keyboard_read;
}

/*===========================================================================*
 *				keyboard_loadmap				     *
 *				加载一个新的按键映射
 *===========================================================================*/
PUBLIC int keyboard_loadmap(phys_bytes user_phys){
    phys_copy(user_phys, vir2phys(keymap), (phys_bytes) sizeof(keymap));
    return OK;
}

/*==========================================================================*
 *				wreboot					    *
 *			等待输入打印调试信息并重新启动。
 *==========================================================================*/
PUBLIC void wreboot(
        int how     /* 0 = halt, 1 = reboot, 2 = panic!, ... */
){
    int quiet, code;
    static u16_t magic = MEMCHECK_MAG;

    /* 屏蔽除了时钟外的所有中断 */
    int irq = CLOCK_IRQ + 1;
    for(;irq < NR_IRQ_VECTORS; irq++){
        disable_irq(irq);
    }

    /* 停止几个任务 */
    console_stop();     /* 停止控制台 */
    /* 停止用户进程的调度，也可以理解为停止所有用户进程的运行 */
    schedule_stop();

    if(how == RBT_HALT){
        printf("System Halted\n");
        if(!monitor_return) how = RBT_PANIC;      /* 是否能返回到监视器？ */
    }

    if(how == RBT_PANIC){
        /* 没办法了，只能强制宕机了。 */
        printf("Hit ESC to reboot, FUNC-keys for debug dumps\n");

        (void) scan_keyboard();     /* 清除所有旧的输出 */
        quiet = scan_keyboard();    /* 静态值（PC上为0，AT上为最后一个代码） */
        /* 即使系统错误非常严重，系统的时钟和键盘依然可以正常运行
         * 我们在这重复的读键盘，给用户一个分析错误的机会。
         */
        while (TRUE){
            milli_delay(100);   /* 读取间隔100ms，免得CPU一直空转 */
            code = scan_keyboard();
            if(code != quiet){
                /* 直到用户点击了ESC键，重启 */
                if(code == ESC_SCAN) break;
                /* FUNC-key（功能键）被调用以给用户提供用于分析系统崩溃原因 */
                (void) function_key(code);
                quiet = scan_keyboard();
            }
        }
        how = RBT_REBOOT;   /* 下一步进入重启 */
    }

    /* 重启中，打印信息 */
    if(how == RBT_REBOOT) printf("Rebooting...\n");

    /* 能返回到监视器，且不是重启RBT_RESET
     */
//    if(monitor_return && how != RBT_RESET) {
//        /* 将中断控制器重新初始化为BIOS默认值。 */
//        interrupt_init(0);
//        /* 打开BIOS中断 */
//        out_byte(INT_M_CTLMASK, 0);
//        out_byte(INT_S_CTLMASK, 0);
//    }

    /* 停止BIOS的内存测试。 */
    phys_copy(vir2phys(&magic), (phys_bytes)MEMCHECK_ADR, (phys_bytes)sizeof(magic));

    /* 通过跳转到reset地址（实模式）或强制处理器关闭（保护模式）来重置系统。
     */
//    milli_delay(1500);  /* 重启之前，等待1500ms，好让用户有个准备。 */
    level0(reset);

}

/*===========================================================================*
 *				map_key0				     *
 *===========================================================================*/
/* 返回一个扫描码对应的ASCII码，忽略修饰符 */
#define map_key0(scan_code)	 \
	    ((unsigned) keymap[(scan_code) * MAP_COLS])

/*===========================================================================*
 *				map_key					     *
 *				映射按键
 *===========================================================================*/
PRIVATE unsigned map_key(int scan_code){
    /* 返回扫描码对应映射文件中的ascii码，完全映射，包括处理和普通字符同时按下的（多重）修饰组合键。 */

    int caps, column, lock;
    u16_t *key_row;

    /* 不要映射数字小键盘上的斜杠 */
    if(scan_code == SLASH_SCAN && esc) return '/';
    /* 得到扫描码在映射表中的行 */
    key_row = &keymap[scan_code * MAP_COLS];

    /* 如果用户按下了shift，那么大写锁定打开 */
    caps = shift;
    /* 得到当前控制台的所有锁定键状态 */
    lock = locks[current_console_nr];
    if((lock & NUM_LOCK) && HOME_SCAN <= scan_code && scan_code <= DEL_SCAN) caps = !caps;
    if((lock & CAPS_LOCK) && (key_row[0] & HASCAPS)) caps = !caps;


    /* 按下了alt */
    if(alt){
        /* 列数变为2（从0算即是第3列），即alt + scan_code的那一列 */
        column = 2;
        /* ctrl + alt，列数变为3（4） */
        if(ctrl || alt_right) column = 3;
        /* 大写锁定，到第4（5）列 */
        if(caps) column = 4;
    } else {
        column = 0;
        // shift + scan_code，列1（2）
        if (caps) column = 1;
        // ctrl + scan_code，列5（6）
        if (ctrl) column = 5;
    }
    /* 返回ASCII码 */
    return key_row[column] & ~HASCAPS;

}

/*===========================================================================*
 *				keyboard_read					     *
 *			     键盘读取
 *===========================================================================*/
PRIVATE void keyboard_read(TTY *tty){
   /* 从键盘读取一个键盘扫描码并处理 */

    char buffer[3];		/* 局部缓冲区 */
    int scan_code;
    unsigned int ch;
    /* 首先，拿到当前用户所使用的控制台，我们始终使用当前控制台 */
    TTY *ctty = &tty_table[current_console_nr];

    /* 只有输入缓冲区中有字符，才能读 */
    while (input_count > 0){
        /* 很好，我们先拿到要处理的字符 */
        scan_code = *input_handle;
        input_handle++;
        /* 越界检查 */
        if(input_handle == input_buffer + KEYBOARD_IN_BYTES) input_handle = input_buffer;
        /* 锁中断防止字符计数被一个可能在同一时刻到达的键盘中断减少。 */
        interrupt_lock();
        input_count--;
        interrupt_unlock();

        /* 如果是局部动作键，执行其函数动作并返回TRUE，不再继续；普通按键，返回FALSE，继续处理。 */
        if(function_key(scan_code)) continue;

        /* 检查CTRL-ALT-DEL强制重启组合键，以确定用户是否要重启。 */
        check_cad(scan_code);

        /* 得到ASCII码 */
        ch = key_make_break(scan_code);

        if(ch <= 255){      /* ascii码的最大值 */
            /* 是一个普通的字符 */
            buffer[0] = ch;
            (void) input_handler(ctty, buffer, 1);
        } else if (HOME <= ch && ch <= INSERT){
            /* 由数字键盘产生的ASCII转义序列
             * 其转义序列格式如：ESC [ A
             */
            buffer[0] = ESCAPE;
            buffer[1] = '[';
            buffer[2] = numpad_map[ch - HOME];
            (void) input_handler(ctty, buffer, 3);
        } else if(ch == ALEFT) {
            /* ALT-LEFT-ARROW：切换到上一个控制台 */
            switch_to(current_console_nr - 1);
            set_led();
        } else if(ch == ARIGHT) {
            /* ALT-RIGHT-ARROW：切换到下一个控制台 */
            switch_to(current_console_nr + 1);
            set_led();
        } else if(AF1 <= ch && ch <= AF12){
            /* Alt-F1是控制台，Alt-F2是ttyc1，等等... */
            switch_to(ch - AF1);
            set_led();
        }
    }
}

/*===========================================================================*
 *				    check_cad					     *
 *			  检查CTRL-ALT-DEL强制重启组合键             *
 *===========================================================================*/
PRIVATE void check_cad(int scan_code){
    /* 检测众所周知的用来在MS-DOS下强制重新启动的神奇的CTRL-ALT-DEL组合，当然CTRL-ALT-INS也是同样的效果。
     * 我们希望是顺序的关机，所以要向包括origin（所有进程的父进程）发送一个SIGABRT信号，而不应该是试图启动
     * PC BIOS例程。origin应该在返回到可以完全重启系统或重启Flyanx的启动监控程序之前接收这个信号，把它解释
     * 为开始一个按次序关机的进程的命令。
     * 当然了，希望这个过程每次都能实现是不现实的。大部分的用户都不可能知道下一刻电脑会怎样，要在危险之前让用
     * 户点击CTRL-ALT-DEL去顺序关机是不可能的。这时系统会变得非常混乱以致于按次序向另一个进程发送信号是不可
     * 能的。这就是在本例程中为什么有一个静态变量CAP_count的原因。大多数系统崩溃时，中断系统仍能工作，所以键
     * 盘输入仍然可以被接收，时钟任务仍可以保持终端运行。这里我们利用了计算机用户的可预期行为，即在电脑看起来
     * 不能正常工作时重复地重击键盘（他提前知道了CTRL-ALT-DEL可以重启）。如果试图向ORIGIN发送SIGABRT失败，
     * 并且用户按了CTRL-ALT-DEL三次，就直接调用wreboot返回到监控程序而不经过调用ORIGIN。
     */

    static int CAD_count = 0;   /* 用户点击CTRL-ALT-DEL的次数 */

    if(ctrl && alt && (scan_code == DEL_SCAN || scan_code == INS_SCAN)){
        /* 如上所说，当CTRL-ALT-DEL被点了三次，直接wreboot强制重启 */
        if(++CAD_count == 3) {
            printf("you try restart the computer forced.\n");
            wreboot(RBT_HALT);
        }
        /* 发送SIGABRT信号给origin进程，进行按次序的重启。如果CAD_count不为3，则正常重启 */
        printf("you try restart the computer normally.\n");
//        cause_sig(ORIGIN_PROC_NR, SIGABRT);
    }
}

/*===========================================================================*
 *				    key_make_break					     *
 *			      执行按键按下/松开处理
 *===========================================================================*/
PRIVATE unsigned int key_make_break(int scan_code){
    /* 将扫描码转换为ASCII码然后更新跟踪修饰键状态的变量。
     *
     * 这个例程可以处理只在按下键时中断的键盘，也可以处理在按下键和释放键时中断的键盘。
     * 为了提高效率，中断例程会过滤掉大多数键释放。
     */

    int ch, make, escape;

    /* 得到扫描码的按下状态 */
    make = (scan_code & 0200) == 0;
    /* 通过映射得到ASCII码 */
    ch = map_key(scan_code &= 0177);
    /* 判断字符需要转义？ */
    escape = esc;
    esc = 0;

    switch (ch){
        case CTRL:          /* 点击了CTRL控制键 */
            *(escape ? &ctrl_right : &ctrl_left) = make;
            ctrl = ctrl_left | ctrl_right;
            break;
        case SHIFT:         /* SHIFT键 */
            *(scan_code == RSHIFT_SCAN ? &shift_right : &shift_left) = make;
            shift = shift_left | shift_right;
            break;
        case ALT:		    /* AIT键 */
            *(escape ? &alt_right : &alt_left) = make;
            alt = alt_left | alt_right;
            break;
        case CALOCK:        /* 大写锁定 */
            if (caps_down < make) {
                /* 异或：切换点击的状态，1变为0,0变为1；下面也有使用 */
                locks[current_console_nr] ^= CAPS_LOCK;
                /* 锁定状态改变，重新设置led灯 */
                set_led();
            }
            caps_down = make;
            break;
        case NLOCK:		    /* 数字键盘锁定 */
            if (num_down < make) {
                locks[current_console_nr] ^= NUM_LOCK;
                set_led();
            }
            num_down = make;
            break;
        case SLOCK:		/* 滚动锁定 */
            if (scroll_down < make) {
                locks[current_console_nr] ^= SCROLL_LOCK;
                set_led();
            }
            scroll_down = make;
            break;
        case EXTKEY:		/* 转义码 */
            esc = 1;		/* 下一个键码将被转义 */
            return -1;
        default:		/* 普通字符 */
            if(make) return ch;
    }

    /* 释放键，或移位类型键，都被忽略，返回-1 */
    return -1;
}

/*===========================================================================*
 *				    function_key					     *
 *			        处理功功能键
 *===========================================================================*/
PRIVATE int function_key(int scan_code){
    /* 判断并处理意味着局部处理的特殊功能键 */
    unsigned int code;
    TTY *tty = &tty_table[current_console_nr];  /* 得到当前使用的控制台终端 */

    /* 如果是按键释放，不处理 */
    if(scan_code & 0200) return FALSE;
    /* 先忽略修饰符 */
    code = map_key0(scan_code);
    /* 不是F1~F12的功能键，不属于我们的工作 */
    if(code < F1 || code > F12) return FALSE;

    switch (map_key(scan_code)){        /* 现在需要修饰符了 */
        case F1:        /* 打印进程表 */
            proc_dmp();
            break;
        case F2:	    /* 打印内存映射 */
            map_dmp();
            break;
        case F3:	    /* 切换硬件/软件滚屏方式 */
            toggle_scroll();
            break;
        case F5:        /* 暂用作清屏功能 */
            clear_screen(tty);
            break;
        /* 一个小彩蛋 */
        case ASF7:          /* ALT + SHIFT + F7 */
            printf("Congratulations, find an egg, I LOVE YOU!!!\n");
            printf("************  **            ***        **\n");
            printf("************  **              **      ** \n");
            printf("**            **                **  **    \n");
            printf("**            **                  **      \n");
            printf("************  **                  **      \n");
            printf("************  **                  **      \n");
            printf("**            **                  **      \n");
            printf("**            **                  **      \n");
            printf("**            ************        **      \n");
            printf("**            ************        **      \n");
            printf("********* Author QQ: 1341662010 **********\n");
            printf("****** QQ exchange group: 909830414 ******\n");
            printf("*********** Welcome to disturb ***********\n");
            break;
        case CF11:
            scroll_screen(1);
            break;
        case CF12:
            scroll_screen(0);
            break;
        /* CF7、CF8和CF9扫描码将引起对sigchar的调用，发送信号不同 */
        case CF7:
//            sigchar(&tty_table[CONSOLE], SIGQUIT);
            break;
        case CF8:
//            sigchar(&tty_table[CONSOLE], SIGINT);
            break;
        case CF9:
//            sigchar(&tty_table[CONSOLE], SIGKILL);
            break;
        default: return FALSE;
    }
    return TRUE;
}

/*===========================================================================*
 *				    keyboard_wait					     *
 *			       键盘等待直到其不忙，即就绪状态
 *===========================================================================*/
PRIVATE int keyboard_wait(void){
    /* CPU等待8042控制器处于可以接收参数的就绪状态；如果超时，返回0 */

    int retries, status;

    retries = MAX_KEYBOARD_BUSY_RETRIES + 1;    /* 等待直到不忙 */
    while (--retries != 0
        && (status = in_byte(KEYBOARD_STATUS)) & (KEYBOARD_IN_FULL | KEYBOARD_OUT_FULL)) {
        if(status & KEYBOARD_OUT_FULL) {
            (void) in_byte(KEYBORD_DATA);       /* 丢弃 */
        }
    }
    return (retries);       /* 如果非0则说明未超时，说明准备好了 */
}

/*===========================================================================*
 *				    keyboard_ack					     *
 *			         键盘等待
 *===========================================================================*/
PRIVATE int keyboard_ack(void){
    /* CPU等待8042控制器确认最后一条命令响应；如果超时，返回0 */

    int retries, status;

    retries = MAX_KEYBOARD_ACK_RETRIES + 1;
    /* 循环一直等待响应 */
    while (--retries != 0 && in_byte(KEYBORD_DATA) != KEYBOARD_ACK){;}
    return retries;      /* 如果非0则说明未超时，说明控制器已经响应了最后一条命令 */
}

/*===========================================================================*
 *				    set_led					     *
 *			    设置键盘的LED灯
 *===========================================================================*/
PRIVATE void set_led(){
    /* 设置指示PC键盘上的Num Lock、Caps Lock或Scroll Lock键是否被按下的灯。 */

    if(!pc_at) return;  /* 因为PC/XT键盘没有LED灯 */

    /* 发出命令，通知键盘即将设置LED灯，下一个字节将会是参数。 */
    keyboard_wait();
    out_byte(KEYBORD_DATA, LED_CODE);
    keyboard_ack();

    /* 三个指示灯的状态用locks[current_console]中的三位进行编码。 */
    keyboard_wait();
    out_byte(KEYBORD_DATA, locks[current_console_nr]);
    keyboard_ack();
}

/*===========================================================================*
 *				 scan_keyboard					     *
 *			    扫描键盘获得键盘码
 *===========================================================================*/
PRIVATE int scan_keyboard(void){
    /* 从键盘硬件中取得键盘码，本例程只做这一件事 */

    int code;
    int value;

    /* 得到扫描键盘码 */
    code = in_byte(KEYBORD_DATA);
    /* 选通键盘以确认字符 */
    value = in_byte(PORT_B);
    out_byte(PORT_B, value | KEY_BIT);
    out_byte(PORT_B, value);
    return code;
}

/*===========================================================================*
 *				 keyboard_handler					     *
 *			     键盘中断处理程序
 *===========================================================================*/
PRIVATE int keyboard_handler(int irq){
    /* 键盘中断处理程序从键盘控制器缓冲区中获得扫描码并放入到输入缓冲区中。
     * 从另一个角度上来说，这个例程就是键盘的数据写入操作例程，即keyboard_write，
     * 因为键盘的特殊性，键盘写入只能通过用户的敲击键盘来完成。
     */

    int scan_code;
    /* 从键盘控制器芯片中获取扫描码 */
    scan_code = scan_keyboard();
    /* 用户点击了任意键，关闭断点，使打了断点的代码能够继续往下执行 */
    if(!(scan_code & 0200)) break_point = FALSE;

    /* 当键盘输入缓冲区还没满，将取出来的扫描码放入到缓冲区中 */
    if(input_count < KEYBOARD_IN_BYTES){
        /* 放入缓冲区空闲处，空闲处指针指向下一个区域 */
        *input_free = scan_code;
        input_free++;
        /* 如果缓冲区已经满了，空闲指针重新指向缓冲区的头，形成闭环队列，并丢弃原本缓冲区头的字符 */
        if(input_free == input_buffer + KEYBOARD_IN_BYTES) input_free = input_buffer;
        /* 字符计数器加1 */
        input_count++;
        /* 键盘触发的字符读入，控制台的events标志的READ就应该被置位，
         * 以便中断及时处理缓冲区的字符。
         */
        tty_table[current_console_nr].events |= (EVENTS_READ);
        /* 现在，该唤醒终端任务做事情了！ */
        tty_wake();
    }
    return ENABLE;  /* 使键盘中断再次打开 */
}

