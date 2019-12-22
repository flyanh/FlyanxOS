/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 服务器和用户有时候必须打印一些消息。它使用系统库中的printf()的简单版本，该版本使用
 * 本例程putk()例程输出字符。
 * 打印是通过执行SYS_PUTS通过系统任务完成的，而不是通过文件系统完成。
 *
 * 此例程只能由MM，FS和FLY之类的服务器使用。内核必须定义自己的putk()。
 */

#include "syslib.h"
#include <flyanx/callnr.h>
#include <flyanx/flylib.h>


/*===========================================================================*
 *				putk					     *
 *===========================================================================*/
void putk(ch)
int ch;      /* 字符，因为包含字符属性，所以不能为char */
{
    /* 一直积累字符。 如果为0或缓冲区已满，打印出来。 */

    static size_t buffer_count;         /* 输出缓冲区字符数量 */
    static char print_buffer[80];       /* 输出缓冲区 */
    static Message msg;

    if((ch == 0 && buffer_count > 0) || buffer_count == sizeof(print_buffer)){
        /* 已经达到字符串结尾或输出缓冲区已满，可以输出了。
         * 我们向系统任务发送一条消息，如果此进程不是系统服务器，
         * 则转发到标准错误。
         */
        msg.m1_i1 = buffer_count;
        msg.m1_p1 = print_buffer;
        msg.type = SYS_PUTS;
        /* 在这里count清零，而不是在发送并接收到消息后，因为A的请求还没有得到答复，
         * A处于堵塞状态，这时候，B又想打印字符串，这时候B累积count，然后发送消息，
         * 这将会导致B的请求打印的字符串包含了A的字符串，这不是我们想看到的。所以我
         * 们应该在发消息之前就将缓冲区长度清零。
         */
        buffer_count = 0;
        send_receive(SYS_TASK, &msg);   /* 成功与否对我们并不重要 */
    }
    /* 将字符放入输出缓冲区 */
    if(ch != 0) print_buffer[buffer_count++] = ch;

}

