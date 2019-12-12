/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件是低特权级格式化打印函数，只能被内核和服务器使用（MM，FS和FLY）
 *
 * 该文件的入口点是：
 *  - printl            格式化打印字符串
 */

#include <stdarg.h>     /* 对可变参数列表操作 */
#include <stddef.h>
#include <limits.h>

#define isdigit(c)    ((unsigned) ((c) - '0') <  (unsigned) 10)

#if  !__STDC__
/* 经典C语言的东西，忽略。 */
void putk();
int printf(fmt) char *fmt;
#else

typedef void(*putk_func_t)(int ch);
void putk(int ch);      /* 引入通用的putk */

int redirect_printf(const char *fmt, va_list argp, putk_func_t rp_putk);

/*=========================================================================*
 *				        printl				   *
 *				    打印一个格式化字符串      *
 *=========================================================================*/
int printl(const char *fmt, ...)
#endif
{
    va_list argp;

    va_start(argp, fmt);

    /* redirect_printf去做真正的事情 */
    redirect_printf(fmt, argp, &putk);

    va_end(argp);
    return 0;
}

/*=========================================================================*
 *				    redirect_printf				   *
 *				    可重定向的printf函数      *
 *=========================================================================*/
int redirect_printf(const char *fmt, va_list argp, putk_func_t rp_putk){
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

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            /* 普通字符 */
            rp_putk(c);
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
                    *--p= x2c[(ptrdiff_t) (u % base)];
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
                if (fill == '0' && i < 0) rp_putk('-');
                if (adjust == RIGHT) {
                    while (width > 0) { rp_putk(fill); width--; }
                }
                if (fill == ' ' && i < 0) rp_putk('-');
                while (len > 0) { rp_putk((unsigned char) *p++); len--; }
                while (width > 0) { rp_putk(fill); width--; }
                break;

                /* 无法识别的格式键，将其回显。 */
            default:
                rp_putk('%');
                rp_putk(c);
        }
    }

    /* 将结尾标记为空（可以是其他值，例如-1）。 */
    rp_putk(0);
    va_end(argp);
    return 0;
}





