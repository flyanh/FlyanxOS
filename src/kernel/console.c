/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/13.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * IBM控制台驱动程序的代码和数据。
 */

#include "kernel.h"
#include <termios.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "protect.h"
#include "tty.h"
#include "process.h"

/*===========================================================================*
 *				putk					     *
 *			输出一个字符到当前控制台
 *===========================================================================*/
PUBLIC void putk(c)
int c;      /* 输出的字符 */
{
    static size_t buffer_count;         /* 输出缓冲区字符数量 */
    static char print_buffer[80];       /* 输出缓冲区 */

    if((c == 0 && buffer_count > 0) || buffer_count == sizeof(print_buffer)){
        /* 达到字符串结尾或输出缓冲区已满，直接输出 */
        print_buffer[buffer_count++] = (char)c;
        disp_str(print_buffer);
        buffer_count = 0;
    }
    /* 将字符放入输出缓冲区 */
    if(c != 0) print_buffer[buffer_count++] = (char)c;
}


