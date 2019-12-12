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

/* 整数转字符串,可以转不同进制 */
static char* itoa(
        char *str,  /* 转换完放在这 */
        int num,    /* 整型数 */
        int radix   /* 转换进制 */
){
    /* 索引表 */
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;  /* 中间变量 */
    int i = 0, j, k;

    /* 确定unum的值 */
    if(radix == 10 && num < 0){/* 十进制负数 */
        unum = (unsigned) - num;
        str[i++] = '-';
    } else
        unum = (unsigned)num;/* 其他情况 */
    /* 逆序 */
    do{
        str[i++] = index[unum % (unsigned)radix];
        unum /= radix;
    } while(unum);

    /* 如果不是十进制,加前缀 */
    if(radix == 2){
        str[i++] = 'b';
        str[i++] = '0';
    } else if (radix == 8)
    {
        str[i++] = 'o';
        str[i++] = '0';
    } else if (radix == 16)
    {
        str[i++] = 'x';
        str[i++] = '0';
    }


    str[i] = '\0';
    /* 转换 */
    if(str[0] == '-')
        k = 1;/* 十进制负数 */
    else
        k = 0;
    /* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */
    for(j = k;j < (i-1) / 2.0 + k; j++){
        num = str[j];
        str[j] = str[i - j - 1 + k];
        str[i - j - 1 + k] = num;
    }
    return str;
}

int vsprintf(char *buffer, const char *format, va_list args){
    char 	*p;
    char 	tmp[STR_DEFAULT_LEN];
    int 	len;

    // 对格式化字符串format进行遍历,碰到'%'停止,并判断后边的符号,进行字符串格式化
    for(p = buffer; *format; format++){
        if(*format != '%' ){
            *p++ = *format;
            continue;
        }

        format++;

        // 判断需要进行怎样的转化
        switch(*format){

            case 'a': case 'A':		/* 浮点数,十六进制数字和p-记数法(C99) */
                break;

            case 'c':				/* 一个标准字符 */
                tmp[0] = *((char*)args);
                tmp[1] = '\0';
                strcpy(p, tmp);
                args += sizeof(int);
                p += strlen(tmp);
                break;

            case 'C':				/* ISO标准宽字符 */
                break;

            case 'd': case 'i':		/* 有符号十进制整数(int) */
                itoa(tmp, *((int*)args), 10);
                strcpy(p, tmp);
                args += sizeof(int);
                p += strlen(tmp);
                break;

            case 'e': 				/* 浮点数、e-记数法 */
                break;

            case 'E':				/* 浮点数、Ｅ-记数法 */
                break;

            case 'f':				/* 单精度浮点数(默认float)、十进制记数法（%.nf  这里n表示精确到小数位后n位.十进制计数） */
                break;

            case 'g': case 'G':		/* 根据数值不同自动选择％f或％e． */
                break;

            case 'o':				/* 无符号八进制整数 */
                break;

            case 'p':				/* 指针 */
                // 我们打印指针的地址
                itoa(tmp, (int)&args, 16);
                strcpy(p, tmp);

                args += sizeof(int);
                p += strlen(tmp);
                break;

            case 's':				/* 显示字符串 */
                len = strlen(*((char**)args));
                memcpy(tmp, *((char**)args), len );
                tmp[len] = '\0';
                strcpy(p, tmp);

                args += sizeof(int);
                p += strlen(tmp);
                break;

            case 'x': case 'X':		/* 使用十六进制数字0f的无符号十六进制整数　 */
                // 首先,先将得到的int参数转化为16进制字符串
                itoa(tmp, *((int*)args), 16);
                // 将转化的字符串放入buffer中
                strcpy(p, tmp);
                // 参数列表指向下一个
                args += sizeof(int);
                // buffer长度增长
                p += strlen(tmp);
                break;

            case '%':				/* 打印一个百分号 */
                // % 放入
                strcpy(p, "%");

                args += sizeof(int);
                p++;
                break;

            default:
                break;
        }

    }

    *p = 0;             /* 最后在转换好的字符串结尾处添上null。 */
    return (p - buffer);	/* 终址 - 起址 --> 字符串长度 */
}



