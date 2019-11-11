/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含启动内核主函数前的准备工作的函数
 *
 * 该文件的入口点是：
 *  - cstart            进入内核主函数前做一些准备工作
 *  - get_env           得到内核环境，即引导参数
 */

#include "kernel.h"
#include <stdlib.h>
#include "protect.h"

/* 加载器传递的环境字符串。 */
PRIVATE char k_environment[128 * sizeof(char *)];

/*==========================================================================*
 *				cstart					                                    *
 *				进入内核主函数前做一些准备工作                                  *
 *==========================================================================*/
PUBLIC void cstart(){
    disp_str("<--\"cstart\" ---\n");

    // 调用prot_init来建立CPU的保护机制和中断表。
    protect_init();


    //@TODO 得到引导参数

}


