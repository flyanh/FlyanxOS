/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * printf，这是最经典的函数，在标准输出流上写入数据。
 *
 * 现在它有一个问题，我暂时不知道怎么回事，当可变参里调用多个函数，
 * 输出它们的返回值时，第二个以及后面的都会输出不正常，但在函数外
 * 先接收再使用就不会有问题，看了好久也不明白，就先这样吧！
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

int printf(const char *format, ...){
    va_list ap;
    int len, rs;
    char buf[STR_DEFAULT_LEN];

    va_start(ap, format);

    len = vsprintf(buf, format, ap);
    rs = write(STDOUT, buf, len);   /* 输出到标准输出上 */

    va_end(ap);
    return rs;
}



