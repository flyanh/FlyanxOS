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
PRIVATE char *input_free = input_buffer;	    /* 指向输入输入缓冲区的下一个空闲点 */
PRIVATE char *input_handle = input_buffer;	    /* 指向应该被处理并返回给终端的扫描码 */
PRIVATE int input_count;			            /* 多少扫描码在缓冲区中？ */

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
PRIVATE int locks[NR_CONSOLE];	/* 每个控制台的锁定键状态 */

/* 锁定键激活位，应该要等于键盘上的LED灯位 */
#define SCROLL_LOCK	    0001b
#define NUM_LOCK	    0010b
#define CAPS_LOCK	    0100b

/* 数字键盘的转义字符映射 */
PRIVATE char numpad_map[] =
        {'H', 'Y', 'A', 'B', 'D', 'C', 'V', 'U', 'G', 'S', 'T', '@'};


