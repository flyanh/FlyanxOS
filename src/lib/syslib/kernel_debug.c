/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内核调试库
 */

#include "syslib.h"

/*==========================================================================*
 *				itoa				    *
 *				整数转字符串,可以转不同进制
 *==========================================================================*/
PUBLIC char* itoa(char *str, int num,u8_t radix){ //num：int型原数;str:需转换成的string;radix,原进制,即要转换的进制

    /* 索引表 */
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;/* 中间变量 */
    int i=0,j,k;

    /* 确定unum的值 */
    if(radix==10&&num<0){/* 十进制负数 */
        unum=(unsigned)-num;
        str[i++]='-';
    }else
        unum=(unsigned)num;/* 其他情况 */
    /* 逆序 */
    do{
        str[i++]=index[unum%(unsigned)radix];
        unum/=radix;
    }while(unum);

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


    str[i]='\0';
    /* 转换 */
    if(str[0]=='-')
        k=1;/* 十进制负数 */
    else
        k=0;
    /* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */
    for(j=k;j<(i-1)/2.0+k;j++){
        num=str[j];
        str[j]=str[i-j-1+k];
        str[i-j-1+k]=num;
    }
    return str;
}

/*==========================================================================*
 *				disp_num				    *
 *				显示一个整形数字
 *==========================================================================*/
PUBLIC void disp_inum(num, radix)
int num;        /* 显示的整形数字 */
u8_t radix;      /* 显示进制 */
{
    char output[16];
    // 进制限制为 2 8 10 16
    if(radix != 2 & radix != 8 & radix != 10 & radix != 16){
        radix = 16;
    }
    // 将数字 --> 16位数字字符串
    itoa(output, num, radix);
    // 显示
    disp_str(output);
}

/*==========================================================================*
 *				delay				    *
 *	    休眠一段时间，使用空循环完成，所以时间在不同机器上能休眠的时间也不同
 *==========================================================================*/
PUBLIC void delay_by_loop(int looptime){
    int i, j, k;
    for (k = 0; k < looptime; k++) {
        for (i = 0; i < 1000; i++) {
            for (j = 0; j < 10000; j++) {}
        }
    }
}



