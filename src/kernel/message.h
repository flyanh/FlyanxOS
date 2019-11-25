/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/16.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 消息机制的头文件，主要是显式声明了就绪(ready)和堵塞(unready)和其他一些重要的进程调度例程。
 * 因为消息机制需要它们，但它们并不需要让所有文件都看到，这是message.c文件独占时刻。
 */

#ifndef FLYANX_MESSAGE_H
#define FLYANX_MESSAGE_H

/* 进入进程结构 */
struct process_s;

_PROTOTYPE(void ready, (Process *process) );
_PROTOTYPE(void unready, (Process *process) );

#endif //FLYANX_MESSAGE_H
