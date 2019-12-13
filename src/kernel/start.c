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

/* 加载器（LOADER）传递的启动参数 */
PUBLIC BootParams bootParams;

/*==========================================================================*
 *				cstart					                                    *
 *			进入内核主函数前做一些准备工作                                  *
 *==========================================================================*/
PUBLIC void cstart(void)
{
    /* 记录内核的代码、数据段位置。 */
    code_base = seg2phys(SELECTOR_KERNEL_CS);
    data_base = seg2phys(SELECTOR_KERNEL_DS);

    /* 建立CPU的保护机制和中断表。 */
    protect_init();

    /* 得到引导（启动）参数 */
    get_boot_params(&bootParams);

    /* 根据引导参数确定机器的各种信息：显示器的型号、内存大小、机器类型、处理器操作模式
     * (实模式还是保护模式)，以及是否可能返回到引导监控程序等。这些所有的信息都保存在适
     * 当的全局变量中，这是为了使核心代码的所有部分在需要时都能访问到它们。
     */

}

/*==========================================================================*
 *				get_boot_params					    *
 *		        获取启动参数
 *==========================================================================*/
PUBLIC void get_boot_params(BootParams *bp)
{
    /* 得到引导参数 */
    u32_t *p_bp = (u32_t*)BOOT_PARAM_ADDR;
    /* 得到魔数 */
    u32_t magic = p_bp[BP_MAGIC];
    if(magic != BOOT_PARAM_MAGIC) return;   /* 坏的魔数 */

    bp->memory_size = p_bp[BP_MEMOARY_SIZE];
    bp->kernel_file = p_bp[BP_KERNEL_FILE];
}


