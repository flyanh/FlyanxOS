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




