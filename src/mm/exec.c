/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件处理EXEC系统调用，它执行的工作如下：
 *  - 读取文件头得到各段长度和总长度
 *  - 从调用者处取得参数和环境
 *  - 分配新内存和释放旧内存
 *  - 把堆栈复制到新的内存映像中
 *  - 设置进程表项
 *  - 告诉内核进程现在是可运行的了
 *
 * 本文件的入口点是：
 *  - do_exec：      执行EXEC系统调用
 */

#include "mm.h"
#include <sys/stat.h>
#include <flyanx/callnr.h>
#include <elf.h>
#include <signal.h>
#include <string.h>
#include "mmproc.h"
#include "param.h"

PRIVATE char stack_buf[ARG_MAX];       /* MM的堆栈缓冲区 */
PRIVATE char name_buf[PATH_MAX];    /* 要执行的文件名称 */

FORWARD _PROTOTYPE( Elf32_Ehdr *read_elf32_header, (int fd, unsigned file_size) );

/*===========================================================================*
 *				do_exec					     *
 *			执行EXEC系统调用
 *===========================================================================*/
PUBLIC int do_exec(void){
    /* 执行execve(name, argv, envp)调用
     * 用户调用时，用户库会构建一个完整的堆栈映像，包括指针，参数指针，环境变量指针等等，
     * 我们应该拿到堆栈将其复制到MM的缓冲区中，然后再复制到新的核心映像中。
     * 这个调用执行的很好，花了我好长时间 : (
     */

    register MMProcess *proc;
    int rs;
    vir_bytes src, dest, tds_bytes, stk_bytes;
    Stat f_state;                   /* 文件状态信息 */

    /* 有效性检查 */
    proc = curr_mp;
    stk_bytes = (vir_bytes)m_stack_bytes;
    if(stk_bytes > ARG_MAX) return ENOMEM;      /* 堆栈太大 */
    if(m_exec_nlen <= 0 || m_exec_nlen > PATH_MAX) return EINVAL;

    /* 获取文件名称 */
    src = (vir_bytes)m_exec_name;
    dest = (vir_bytes)name_buf;
    rs = sys_copy(mm_who, DATA, (phys_bytes)src, /* 将文件名称复制到缓冲区 */
            MM_PROC_NR, DATA, (phys_bytes)dest,
            (phys_bytes)m_exec_nlen);
    if(rs != OK) return rs;     /* 文件名称可能不在用户数据段中 */

    /* 获取用户堆栈 */
    src = (vir_bytes)m_stack_ptr;
    dest = (vir_bytes)stack_buf;
    rs = sys_copy(mm_who, DATA, (phys_bytes)src, /* 将用户堆栈复制到缓冲区 */
                  MM_PROC_NR, DATA, (phys_bytes)dest,
                  (phys_bytes)stk_bytes);
    if(rs != OK) return EACCES;     /* 无法获取堆栈，可能是因为错误的虚拟地址导致的 */
//    printf("exec new stack on: %ld\n", src);
//    printf("exec new stack size: %d\n", stk_bytes);

    /* 获取文件信息，主要是需要知道文件的大小 */
    if(stat(name_buf, &f_state) != 0){
        printf("{MM} can't get exec file state, filename is %s\n", name_buf);
        return -1;
    }
    printf("exec file name: %s\n", name_buf);
    printf("exec file size: %d\n", f_state.size);
    if(f_state.size >= MM_BUFFER_SIZE){
        printf("exec file too big, {file: %s}\n", name_buf);
        return EFBIG;
    }

    /* 先打开该文件 */
    int fd = open(name_buf, O_RDWR);
    if(fd < 0){
        printf("exec file open failed. {fd: %d}\n", fd);
        return -1;
    }

    /* 然后读取该文件的可执行ELF32文件头 */
    Elf32_Ehdr *ehdr = read_elf32_header(fd, f_state.size);
    close(fd);  /* 文件使用完别忘了关闭文件，后面用不到了 */
    /* 文件是一个可执行文件吗？该文件不是一个特殊文件且文件大小不为空同时ELF32
     * 头也存在，那么它就是一个可执行文件。
     */
    if(f_state.mode != I_REGULAR || f_state.size == 0 || (ehdr == (Elf32_Ehdr*)0)){
        printf("{MM}-> the file is not an executable\n");
        return -1;
    }

    /* 现在用新的内存映像覆盖调用者的内存映像 */
    int i;
    for(i = 0; i < ehdr->e_phnum; i++){     /* 一次拷贝一个程序头表项 */
        /* 手动为prog_hdr在高速缓冲区中分配空间，这里分配的位置跟ELF32头指出的程序
         * 挂载点偏移一样，为了后面的拷贝。
         */
        Elf32_Phdr *prog_hdr = (Elf32_Phdr*)(mm_buffer + ehdr->e_phoff + (i * ehdr->e_phentsize));
        if(prog_hdr->p_type == PT_LOAD){        /* 它是一个可加载程序段 */
            tds_bytes = prog_hdr->p_vaddr + prog_hdr->p_memsz;  /* 计算程序这个段的总大小 */
            if(tds_bytes >= proc->map.size){
                /* 加载前先判断当前进程能否装得下这个新程序，不能我们也到此为止 */
                printf("exec file too big, {file: %s}\n", name_buf);
                return EFBIG;
            }
            /* 好的，一切顺利，我们现在可以拷贝它到内存中了。 */
            sys_copy(MM_PROC_NR, ABSOLUTE, (phys_bytes) (mm_buffer + prog_hdr->p_offset), /* 从MM高速缓冲区中存放的程序偏移 */
                     mm_who, TEXT, (phys_bytes)prog_hdr->p_vaddr,  /* 拷贝到内存中程序的虚拟地址 */
                     (phys_bytes)prog_hdr->p_filesz);              /* 拷贝大小 */
        }
    }

    /* 现在，我们应用新的堆栈到内存映像中 */
    char *new_stack = (char*)(proc->map.size - ARG_MAX);
    int delta = (int)new_stack - (int)m_stack_ptr;

    int argc = 0;
    if(stk_bytes){  /* 有参数和环境变量 */
        char **q = (char**)stack_buf;
        for (; *q != 0; q++, argc++){
            *q += delta;
        }
    }
    printf("argc = %d\n", argc);
    /* 将高速缓冲里的新堆栈数据拷贝到新堆栈地址上 */
    src = (vir_bytes)stack_buf;
    dest = (vir_bytes)new_stack;
    rs = sys_copy(MM_PROC_NR, DATA, (phys_bytes)src,
                  mm_who, DATA, (phys_bytes)dest,
                  (phys_bytes)stk_bytes);
    if(rs != OK) mm_panic("new stack apply failed", (int)new_stack);

    /* 好的，现在通知内核，设置新程序的运行框架，暂时不支持环境变量的指定，
     * 给个0就好。
     */
    sys_set_prog_frame(mm_who, argc, (u32_t)new_stack, 0);

    /* 万事俱备，现在通知内核，一个程序已经加载进内存，
     * 请设置它的一些信息让它跑起来。
     */
    sys_exec(mm_who, new_stack, name_buf, ehdr->e_entry);

    return ERROR_NO_MESSAGE;        /* 不需要回复，新程序已经覆盖旧程序的内存，默默运行就好了。 */
}

/*===========================================================================*
 *				read_elf_header					     *
 *			  读取一个可执行文件的ELF32头
 *===========================================================================*/
PRIVATE Elf32_Ehdr *read_elf32_header(int fd, unsigned file_size){
    /* 读取该文件到高速缓冲区中 */
    read(fd, mm_buffer, file_size);
    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)mm_buffer;
    /* 判断该文件头存在否 */
    if(ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
            ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3){
        /* 魔数匹配不上，这是个坏的ELF32文件头，我们返回空表示文件头是坏的 */
        ehdr = (Elf32_Ehdr*)0;
    }

    return ehdr;    /* 返回得到的FLF32头 */
}

