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
#include <elf.h>        /* 使用elf可执行文件头 */
#include "string.h"

#if (CHIP == INTEL)

/*=========================================================================*
 *				memory_init				   *
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
    

}

/*=========================================================================*
 *				get_kernel_map				   *
 *			    获取内核映像
 *=========================================================================*/
PUBLIC int get_kernel_map(vir_bytes *base, vir_bytes *limit){
    /* 解析内核文件，获取内核映像的内存范围。 */

    /* 得到内核文件的elf32文件头 */
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)(bootParams.kernel_file);

    /* 内核文件必须为ELF32格式 */
    if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0){
        return ERROR_BAD_ELF;
    }

    *base = ~0;
    vir_bytes t = 0;
    int i;
    for(i = 0; i < elf_header->e_shnum; i++){
        Elf32_Shdr *section_header = (Elf32_Shdr*)(bootParams.kernel_file +
                            elf_header->e_shoff + i * elf_header->e_shentsize);
        if(section_header->sh_flags & SHF_ALLOC) {
            int bottom = section_header->sh_addr;
            int top = section_header->sh_addr + section_header->sh_size;

            if(*base > bottom) *base = bottom;
            if(t < top) t = top;
        }
    }
    if(*base >= t) {
        return -1;
    }
    *limit = t - *base - 1;

    return OK;
}

/*=========================================================================*
 *				memcmp				   *
 *			  比较两个内存区域
 *=========================================================================*/
PUBLIC int memcmp(const void * s1, const void *s2, size_t n)
{
    if ((s1 == 0) || (s2 == 0)) { /* for robustness */
        return (s1 - s2);
    }

    const char * p1 = (const char *)s1;
    const char * p2 = (const char *)s2;
    int i;
    for (i = 0; i < n; i++,p1++,p2++) {
        if (*p1 != *p2) {
            return (*p1 - *p2);
        }
    }
    return 0;
}

#endif

