/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/20.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 包含了一些C实用程序，是核心公共的例程
 */


#include "kernel.h"
#include "assert.h"
#include <stdlib.h>
#include <flyanx/common.h>

#if (CHIP == INTEL)

/*=========================================================================*
 *				mem_init				   *
 *=========================================================================*/
PUBLIC void memory_init(void){
    /* 内存区域初始化
     * 这个函数仅在MINIX首次启动时被main调用。在一台IBMPC兼容机上可能存在两个或三个不连续
     * 的内存区域。PC用户称为“常规”内存的最低端内存的大小，以及从PC ROM之上开始的内存区域（
     *  “扩展”内存）由BIOS告知引导监控程序，引导监控程序随后又将其作为引导参数传递。该参数由
     * cstart进行解释并在引导时写入low_memsize和 ext_memsize。第三个区域是“影子”内存，BIOS ROM
     * 可被拷贝到该区域以提高性能，因为ROM通常比RAM的访问速度慢。由于MINIX一般不使用BIOS，所以
     * mem_init力图定位该片内存并将其加入它的可用内存区中，这是通过调用check_mem来完成的。
     */

    /* 复制地址范围描述符数组到内核中 */
    phys_copy(ards_phys, vir2phys(data_base, &ards), sizeof(ARDS) * NR_MEMORY_CLICK);
}

#endif

