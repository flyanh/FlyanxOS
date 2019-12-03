/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 系统任务
 *
 * 该文件负责和服务器进行通信，协助它们完成POSIX系统调用。因为
 * 服务器在特权级2下，有些事情它自己不能办到，而系统任务处于特
 * 权级1下，可以使用内核的全部功能。
 */

#include "kernel.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sigcontext.h>
#include <sys/ptrace.h>
#include <sys/svrctl.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "process.h"
#if (CHIP == INTEL)
#include "protect.h"
#endif
#include "assert.h"
INIT_ASSERT     /* 初始化断言 */


/* 程序状态字掩码 */
#define IF_MASK   0x00000200
#define IOPL_MASK 0x003000

PRIVATE Message msg_in;

FORWARD _PROTOTYPE( int do_fork, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_get_sp, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_exit, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_puts, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_find_proc, (Message *msg_ptr) );


/*===========================================================================*
 *				system_task				     *
 *===========================================================================*/
PUBLIC void system_task(void){
    /* 系统任务入口，获取消息并根据类型分配调用不同的例程实现。 */

    register int rs;

    while (TRUE){
        receive(ANY, &msg_in);
//        printf("source: %d, type: %d\n", msg_in.source, msg_in.type);

        switch (msg_in.type){      /* 想要什么系统功能服务？ */
            case SYS_FORK:      rs = do_fork(&msg_in);      break;
            case SYS_GET_SP:    rs = do_get_sp(&msg_in);    break;
            case SYS_EXIT:      rs = do_exit(&msg_in);      break;
            case SYS_PUTS:      rs = do_puts(&msg_in);      break;
            case SYS_FIND_PROC: rs = do_find_proc(&msg_in); break;
            default:            rs = ERROR_BAD_FCN;         break;
        }

        msg_in.type = rs;          /* 报告调用结果 */
        send(msg_in.source, &msg_in); /* 发送回复给调用者 */
    }
}

/*===========================================================================*
 *				do_fork					     *
 *===========================================================================*/
PRIVATE int do_fork(Message *msg_ptr){

}

/*===========================================================================*
 *				do_get_sp					     *
 *===========================================================================*/
PRIVATE int do_get_sp(Message *msg_ptr){

}

/*===========================================================================*
 *				do_exit					     *
 *===========================================================================*/
PRIVATE int do_exit(Message *msg_ptr){

}

/*===========================================================================*
 *				do_puts					     *
 *				输出字符串
 *===========================================================================*/
PRIVATE int do_puts(Message *msg_ptr){
    /* 为一个服务器打印一个字符串 */

    char ch;
    vir_bytes src;
    int count;

    src = (vir_bytes) msg_ptr->m1_p1;   /* 数据 */
    count = msg_ptr->m1_i1;             /* 字符数量 */
    for(; count > 0; count--){
        if(vir_copy(msg_ptr->source, src,
                SYS_TASK, (vir_bytes) &ch, 1) != OK) return EFAULT;
        src++;
        k_putk(ch);
    }
    k_putk(0);    /* 字符串结束 */
    return OK;
}

/*===========================================================================*
 *				do_find_proc					     *
 *			通过名称寻找一个进程
 *===========================================================================*/
PRIVATE int do_find_proc(Message *msg_ptr){
    /* 根据任务的名称确定任务的任务编号。这允许像internet这样的延迟启动的服务器
     * 不知道任何任务号，因此它可以与一个精确配置(任务在哪里?)未知的内核一起使用。
     */

    Process *p;

    /* 得到参数 */
    char *name = msg_ptr->m3_ca1;

    for(p = BEG_PROC_ADDR; p < END_PROC_ADDR; p++){
        /* 只查找任务和服务 */
        if(!is_task_proc(p) && !is_serv_proc(p)) continue;
        if(strncmp(p->name, name, M3_STRING) == 0){
            msg_ptr->m3_i1 = p->nr;
            return OK;
        }
    }
    return (ERROR_SEARCH);
}

/*==========================================================================*
 *				vir_copy					    *
 *			虚拟地址间拷贝数据
 *==========================================================================*/
PUBLIC int vir_copy(
        int src_proc,       /* 源进程 */
        vir_bytes src_vir,  /* 源的虚拟地址 */
        int dest_proc,      /* 目标进程 */
        vir_bytes dest_vir, /* 目标的虚拟地址 */
        vir_bytes bytes     /* 要复制多少字节？ */
){
    /* 按字节从一个进程拷贝数据到另一个进程。用于简单的情况，没有速度要求。 */
    phys_bytes src_phys, dest_phys;

    /* 得到物理地址 */
    src_phys = umap(proc_addr(src_proc), DATA, src_vir, bytes);
    dest_phys = umap(proc_addr(dest_proc), DATA, dest_vir, bytes);
    if(src_phys == 0 || dest_phys == 0) return (EFAULT);
    phys_copy(src_phys, dest_phys, (phys_bytes) bytes);
    return OK;
}

/*===========================================================================*
 *				umap					     *
 *		计算给定虚拟地址的物理地址
 *===========================================================================*/
PUBLIC phys_bytes umap(
        Process *proc,      /* 指向虚拟地址待映射的进程或任务的进程表表目的指针 */
        int seg_index,      /* 指定正文、数据、或堆栈段的标志 */
        vir_bytes vir_addr, /* 虚拟地址 */
        vir_bytes bytes     /* 字节计数器 */
){
    /* 本例程是一个通用的把虚拟地址映射为物理地址的过程。我们已经从上面的代码注意到，它由
     * 为SYS_UMAP消息服务的do_umap调用。它的参数是一个指向虚拟地址待映射的进程或任务的
     * 进程表表目的指针，一个指定正文、数据、或堆栈段的标志，虚拟地址本身，以及一个字节计
     * 数器。这个计数器很有用因为umap要检查以确保从虚拟地址开始的全部缓冲区都处于进程的
     * 地址空间中。为此，它必须知道缓冲区的大小。字节计数器并不用于映射本身，而只用于这个
     * 检查。所有从/向用户进程空间拷贝数据的任务都使用umap计算缓冲区的物理地址。
     */

    vir_clicks vc;              /* 虚拟地址块 */
    phys_bytes vir_phys;        /* 转换完成的物理地址 */

    /* 检查@TODO */
    if(bytes <= 0) return ( (phys_bytes) 0 );

    if(seg_index == DATA){  /* 如果要的是数据段，那么直接使用proc_vir2phys即可实现 */
        return proc_vir2phys(proc, vir_addr);
    } else {
        /* 首先得到该进程的段物理地址 */
        phys_bytes seg_base = ldt_seg_phys(proc, seg_index);
        /* 该虚拟地址对应的物理地址 = 进程段地址 + 虚拟地址 */
        vir_phys = seg_base + vir_addr;
    }
    return vir_phys;
}

/*===========================================================================*
 *				numap					     *
 *		计算给定虚拟地址的物理地址
 *===========================================================================*/
PUBLIC phys_bytes numap(
        int proc_nr,        /* 指向虚拟地址待映射的进程或任务的编号 */
        vir_bytes vir_addr, /* 虚拟地址 */
        vir_bytes bytes     /* 字节计数器 */
){
    /* 对于设备驱动程序如果能够使用进程号作为参数而不是指向进程表表目的指针，
     * 那么numap则更为方便。它调用proc_addr来转换它的第一个参数，然后调用
     * umap。为了节省时间，没有'seg'参数。总是DATA（数据段）。
     */
    return (umap(proc_addr(proc_nr), DATA, vir_addr, bytes));
}



