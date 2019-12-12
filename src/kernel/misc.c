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
#include <elf.h>            /* elf可执行文件头 */

/*=========================================================================*
 *				get_kernel_map				   *
 *			    获取内核映像
 *=========================================================================*/
PUBLIC int get_kernel_map(phys_clicks *base, phys_clicks *limit){
    /* 解析内核文件，获取内核映像的内存范围。 */

    /* 得到内核文件的elf32文件头 */
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)(bootParams.kernel_file);

    /* 内核文件必须为ELF32格式 */
    if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0){
        return ERROR_BAD_ELF;
    }

    *base = ~0;
    phys_clicks t = 0;
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

#if !NDEBUG
/*=========================================================================*
 *				bad_assertion				   *
 *				坏断言处理
 *=========================================================================*/
PUBLIC bool assert_panic;           /* 断言宕机标志 */
PUBLIC void bad_assertion(
        char *file,     /*  */
        int line,
        char *what
){
    /* 本例程只有在宏DEBUG被定义为TRUE时才对其进行编译，支持assert.h中的宏。 */
    assert_panic = TRUE;
    blue_screen();          /* 蓝屏 */
    printf("panic at file://%s(%d): assertion \"%s\" failed\n", file, line, what);
    panic(NULL, NO_NUM);
}

/*=========================================================================*
 *				bad_compare				   *
 *				坏比较处理
 *=========================================================================*/
PUBLIC void bad_compare(file, line, lhs, what, rhs)
        char *file;
        int line;
        int lhs;
        char *what;
        int rhs;
{
    /* 本例程只有在宏DEBUG被定义为TRUE时才对其进行编译，支持assert.h中的宏。 */
    printf("panic at %s(%d): compare (%d) %s (%d) failed\n",
           file, line, lhs, what, rhs);
    panic(NULL, NO_NUM);
}
#endif /* !NDEBUG */

