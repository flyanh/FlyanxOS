/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/18.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 标准函数：有符号十进制整数转化为ASCII字符整数。
 * 实现了两个，一个可以根据进制转化，另一个则是最经典的转十进制。
 */

#include <lib.h>

PRIVATE char str[8];    /* int至少要六位 */

_PROTOTYPE(char *itoa, (int n));
_PROTOTYPE(char *itoap, (int n, int radix));

char *itoa(int num){
    return itoap(num, 10);
}

/* 整数转字符串,可以转不同进制 */
char* itoap(
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

