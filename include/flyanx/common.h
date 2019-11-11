/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 主要包含从MM(内存管理器)和FS(文件系统)发送到I/O任务的消息中所使用的公共定义。
 * 其中也包括了任务序号。在这里为了与进程号分开，任务号是负数的。
 *
 * 该文件还定义了可被发送到每个系统任务的消息类型(函数代码)，例如,
 * 时钟任务接收代码SET_ALARM(用于设置定时器)、CLOCK_TICK(当时钟中断发生时)、 GET_TIME(取真实时间)、
 * SET_TIME(设置一天中的当前时间)、 REAL_TIME 的值就是对GET_TIME请求所返回的消息类型。
 */

#ifndef FLYANX_COMMON_H
#define FLYANX_COMMON_H

/* System calls. */
#define SEND		   1	/* function code for sending messages */
#define RECEIVE		   2	/* function code for receiving messages */
#define BOTH		   3	/* function code for SEND + RECEIVE */
#define ANY		0x7ace	/* a magic, invalid process number.
				 * receive(ANY, buf) accepts from any source
				 */

/* 任务号，函数索引号(消息类型)和回复代码，将在下面开始定义 */

/* 用作中断生成消息的源 */
#define HARDWARE          -1	/* used as source on interrupt generated msgs */

#endif //FLYANX_COMMON_H
