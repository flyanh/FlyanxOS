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

PUBLIC bool break_point;

/*==========================================================================*
 *				delay				    *
 *	    休眠一段时间，使用空循环完成，所以时间在不同机器上能休眠的时间也不同
 *==========================================================================*/
PUBLIC void delay_by_loop(int ltime){

    int i, j, k;
    for (k = 0; k < ltime; k++) {
        for (i = 0; i < 1000; i++) {
            for (j = 0; j < 10000; j++) {}
        }
    }
}

/*==========================================================================*
 *				  simple_brk_point				    *
 *	              打一个简单的断点
 *==========================================================================*/
PUBLIC void simple_brk_point(int code){
    /* 一个最简单的断点实现，可以打印一个自定义的运行代码信息
     */

    break_point = TRUE;     /* 断点打开 */
//    printf("break point here, code: %X | ", code);
//    printf("please press any key to continue...\n");
    /* 死循环堵塞在这 */
    while (break_point){ }  /* 如果break_point值为FALSE，说明用户点击了任意键，退出断点堵塞 */
}





