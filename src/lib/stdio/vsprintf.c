/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 格式化字符串，并拷贝到缓冲区
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#define isdigit(c)    ((unsigned) ((c) - '0') <  (unsigned) 10)

int vsprintf(char *buf, const char *fmt, va_list argp){
    int c;
    enum { LEFT, RIGHT } adjust;
    enum { LONG, INT } intsize;
    int fill;
    int width, max, len, base;
    static char X2C_tab[]= "0123456789ABCDEF";
    static char x2c_tab[]= "0123456789abcdef";
    char *x2c;
    char *p;
    long i;
    unsigned long u;
    char temp[8 * sizeof(long) / 3 + 2];
    int buf_len = 0;

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            /* 普通字符 */
            *buf = c;
            buf++;
            buf_len++;
            continue;
        }

        /* 格式说明符，格式为：
         * ％[adjust] [fill] [width] [.max]keys
         */
        c = *fmt++;

        adjust = RIGHT;
        if (c == '-') {
            adjust= LEFT;
            c= *fmt++;
        }

        fill = ' ';
        if (c == '0') {
            fill= '0';
            c= *fmt++;
        }

        width = 0;
        if (c == '*') {
            /* 宽度被指定为参数，例如 %*d。 */
            width = (int) va_arg(argp, int);
            c= *fmt++;
        } else
        if (isdigit(c)) {
            /* 数字表示宽度，例如 %10d。 */
            do {
                width= width * 10 + (c - '0');
            } while (isdigit(c= *fmt++));
        }

        max = INT_MAX;
        if (c == '.') {
            /* 就要到最大字段长度了 */
            if ((c = *fmt++) == '*') {
                max = (int) va_arg(argp, int);
                c = *fmt++;
            } else
            if (isdigit(c)) {
                max = 0;
                do {
                    max = max * 10 + (c - '0');
                } while (isdigit(c = *fmt++));
            }
        }

        /* 将一些标志设置为默认值 */
        x2c = x2c_tab;
        i = 0;
        base = 10;
        intsize = INT;
        if (c == 'l' || c == 'L') {
            /* “Long”键，例如 %ld。 */
            intsize = LONG;
            c = *fmt++;
        }
        if (c == 0) break;

        switch (c) {
            /* 十进制 */
            case 'd':
                i = intsize == LONG ? (long)va_arg(argp, long)
                                    : (long) va_arg(argp, int);
                u = i < 0 ? -i : i;
                goto int2ascii;

                /* 八进制 */
            case 'o':
                base= 010;
                goto getint;

                /* 指针，解释为%X 或 %lX。 */
            case 'p':
                if (sizeof(char *) > sizeof(int)) intsize= LONG;

                /* 十六进制。 %X打印大写字母A-F，而不打印%lx。 */
            case 'X':
                x2c = X2C_tab;
            case 'x':
                base = 0x10;
                goto getint;

                /* 无符号十进制 */
            case 'u':
            getint:
                u = intsize == LONG ? (unsigned long)va_arg(argp, unsigned long)
                                    : (unsigned long)va_arg(argp, unsigned int);
            int2ascii:
                p = temp + sizeof(temp) - 1;
                *p = 0;
                do {
                    *--p= x2c[(int) (u % base)];
                } while ((u /= base) > 0);
                goto string_length;

                /* 一个字符 */
            case 'c':
                p = temp;
                *p = (int)va_arg(argp, int);
                len = 1;
                goto string_print;

                /* 只是一个百分号 */
            case '%':
                p = temp;
                *p = '%';
                len = 1;
                goto string_print;

                /* 一个字符串，其他情况将加入这里。 */
            case 's':
                p = va_arg(argp, char *);

            string_length:
                for (len= 0; p[len] != 0 && len < max; len++) {}

            string_print:
                width -= len;
                if (i < 0) width--;
                if (fill == '0' && i < 0) {
                    *buf++ = '-';
                    buf_len++;
                }
                if (adjust == RIGHT) {
                    while (width > 0) {
                        *buf = fill;
                        buf++;
                        buf_len++;
                        width--;
                    }
                }
                if (fill == ' ' && i < 0) *buf++ = '-';
                while (len > 0) {
                    *buf = (unsigned char) *p++;
                    buf++;
                    buf_len++;
                    len--;
                }
                while (width > 0) {
                    *buf = fill;
                    buf++;
                    buf_len++;
                    width--;
                }
                break;

                /* 无法识别的格式键，将其回显。 */
            default:
                *buf = '%';
                *buf = c;
                buf += 2;
                buf_len += 2;
        }
    }

    /* 将结尾标记为空 */
    *buf++ = 0;
    return buf_len;
}




