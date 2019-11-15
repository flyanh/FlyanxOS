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
PUBLIC void cstart()
//U16_t kss;                      /* 内核堆栈段 */
//U16_t parm_offset, parm_size;	/* boot parameters offset and length */
{
//    clear_screen();

    /* 记录内核的位置。 */
    code_base = seg2phys(SELECTOR_KERNEL_CS);
    data_base = seg2phys(SELECTOR_KERNEL_DS);

    // 调用prot_init来建立CPU的保护机制和中断表。
    protect_init();


    //@TODO 得到引导参数

    /* 根据引导参数确定机器的各种信息：显示器的型号、内存大小、机器类型、处理器操作模式
     * (实模式还是保护模式)，以及是否可能返回到引导监控程序等。这些所有的信息都保存在适
     * 当的全局变量中，这是为了使核心代码的所有部分在需要时都能访问到它们。
     */

}

/*==========================================================================*
 *				get_env					    *
 *		    在核心环境中查找数据项,该环境是引导参数的拷贝。
 *==========================================================================*/
PUBLIC char *get_env(name)
_CONST char *name;
{
    register _CONST char *namep;
    register char *envp;

    for (envp = k_environment; *envp != 0;) {
        for (namep = name; *namep != 0 && *namep == *envp; namep++, envp++)
            ;
        if (*namep == '\0' && *envp == '=') return(envp + 1);
        while (*envp++ != 0)
            ;
    }
    return(NIL_PTR);
}


