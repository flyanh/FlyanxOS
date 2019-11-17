/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * tty.h为所有类型的终端设备提供支持
 */

#ifndef FLYANX_TTY_H
#define FLYANX_TTY_H

/* 声明终端结构 */
struct tty_s;

#define TTY_INPUT_BYTES     256     /* 终端输入队列大小 */
#define TAB_SIZE            4       /* 制表符间距 */
#define TAB_MASK            3       /* 计算制表符停止位置的掩码。 */

#define ESCAPE              '\33'   /* 转移字符 */

// 这两个标志在"include/fcntl.h"中已经定义过了。在这里再重写一次是为了避免需要再包含一个文件
#define O_NOCTTY            00400
#define O_NONBLOCK          04000

typedef _PROTOTYPE( void (*device_func_t), (struct tty_s *tp) );
typedef _PROTOTYPE( void (*device_func_arg_t), (struct tty_s *tp, int c) );

/* 终端结构 */
typedef struct tty_s {
    int events;     /* 标志：当某个中断引起变化需要终端任务来处理设备 */
} TTY;

#endif //FLYANX_TTY_H
