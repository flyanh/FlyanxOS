/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/13.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#include <stdio.h>
#include <stdarg.h>

/* 只是简单的格式化字符串，但不进行输出 */
int sprintf(char *buf, const char *fmt, ...){
    va_list ap;

    va_start(ap, fmt);

    int rs = vsprintf(buf, fmt, ap);

    va_end(ap);
    return rs;
}
