/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 服务器有必须打印一些消息。它使用系统库中的printf()的简单版本，该版本使用
 * putk()例程输出字符。
 * 打印是通过执行SYS_PUTS完成的，而不是通过文件系统完成。
 *
 * 此例程只能由MM，FS和FLY之类的服务器使用。内核必须定义自己的putk()。
 */

#include "syslib.h"
#include <flyanx/callnr.h>
//#include <flyanx/minlib.h>


/*===========================================================================*
 *				putk					     *
 *				@TODO 消息通信机制还未完成
 *===========================================================================*/
void putk(ch)
int ch;      /* 字符，因为包含字符属性，所以不能为char */
{
    /* 一直积累字符。 如果为0或缓冲区已满，打印出来。 */


}