/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * printf，这是最经典的函数，在标准输出流上写入数据。
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



