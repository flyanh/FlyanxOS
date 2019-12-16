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
#include "protect.h"
#include "assert.h"
INIT_ASSERT     /* 初始化断言 */


/* 程序状态字掩码 */
#define IF_MASK   0x00000200
#define IOPL_MASK 0x003000

PRIVATE Message msg_in;

_PROTOTYPE( int sprintf, (char *_buf, const char *_fmt, ...) );
FORWARD _PROTOTYPE( int do_fork, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_get_sp, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_exit, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_puts, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_find_proc, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_sudden, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_blue_screen, (void) );
FORWARD _PROTOTYPE( int do_copy, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_get_map, (Message *msg_ptr) );
FORWARD _PROTOTYPE( int do_new_map, (Message *msg_ptr) );


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
            case SYS_SUDDEN:    rs = do_sudden(&msg_in);    break;
            case SYS_BLUES:     rs = do_blue_screen();      break;
            case SYS_COPY:      rs = do_copy(&msg_in);      break;
            case SYS_GET_MAP:   rs = do_get_map(&msg_in);   break;
            case SYS_NEW_MAP:   rs = do_new_map(&msg_in);   break;
            default:            rs = ERROR_BAD_FCN;         break;
        }

        msg_in.type = rs;          /* 报告调用结果 */
        send(msg_in.source, &msg_in); /* 发送回复给调用者 */
    }
}

/*===========================================================================*
 *				do_fork					     *
 *			处理系统级调用sys_fork()
 *===========================================================================*/
PRIVATE int do_fork(Message *msg_ptr){
    /* msg_ptr->PROC_NR1是新创建的进程，它的父进程在msg_ptr->PROC_NR2中，
     * msg_ptr->PID是新进程的进程号。
     */

    register Process *child;
    reg_t old_ldt_sel;
    Process *parent;

    child = proc_addr(msg_ptr->PROC_NR1);   /* 得到子进程 */
    assert(is_empty_proc(child));   /* 子进程一定要是一个空进程 */
    parent = proc_addr(msg_ptr->PROC_NR2);  /* 父进程 */
    assert(is_user_proc(parent));   /* 父进程必须是一个用户进程 */

    /* 将父进程拷贝给子进程（所有） */
    old_ldt_sel = child->ldt_sel;  /* 防止LDT选择子被覆盖，我们备份它 */
    *child = *parent;                   /* 拷贝进程结构体 */
    child->ldt_sel = old_ldt_sel;  /* 恢复LDT选择子 */
    /* 设置子进程一些独有的信息 */
    child->nr = msg_ptr->PROC_NR1;      /* 子进程要记住自己的索引号 */
    child->flags |= NO_MAP;             /* 禁止子进程运行，因为它刚刚出生 */
    child->flags &= ~(PENDING | SIG_PENDING | PROC_STOP);   /* 复位标志，它们不应该继承父进程的这些状态 */
    child->pid = msg_ptr->PID;          /* 记住自己的进程号 */
    child->regs.eax = 0;                /* 子进程看到ax是0.就知道自己是fork出来的了。 */
    sprintf(child->name, "%s_fk_%d",
            parent->name, child->pid);  /* 还是给它起个默认名字吧：父名_fk_子号 */

    /* 清零子进程的时间记账信息 */
    child->user_time = child->sys_time =
            child->child_user_time = child->child_sys_time = 0;

    return OK;  /* OK了 */
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

    /* 检查@TODO */
    if(bytes <= 0) return ( (phys_bytes) 0 );
    /* flyanx0.1暂时不区分三段，所以直接转换即可 */
    return proc_vir2phys(proc, vir_addr);
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


/*===========================================================================*
 *				do_sudden					     *
 *				系统突然终止
 *===========================================================================*/
PRIVATE int do_sudden(Message *msg_ptr){
    /* 处理sys_sudden，flyanx已经无法继续，终止系统。
     *
     * 存储管理器或文件系统都有发现一个错误以致于无法继续执行操作的可能。例如，如果在首次启动时文件系统
     * 发现跟设备的超级块有致命的损坏，它将陷入混乱状态并向内核发送一条SYS_ABORT消息。超级用户也可能强
     * 迫返回到启动监控程序或者使用reboot命令调用REBOOT系统调用重新启动。在任何一种情况下，系统任务执行
     * do_abort，把指令拷贝到监控程序中，然后调用wreboot完成处理。
     * @TODO
     */

//    phys_bytes src_phys;
//    vir_bytes len;

    /* 返回到监控器 */
    if(msg_ptr->m1_i1 == RBT_MONITOR){
        printf("RBT_MONITOR");
        /* 暂时没完成 */
    }
    wreboot(msg_ptr->m1_i1);
    return OK;              /* 额...其实这里已经不可能执行的到了，走个形式吧。 */
}


/*===========================================================================*
 *				do_blue_screen					     *
 *			它很简单，给服务器提供蓝屏功能
 *===========================================================================*/
PRIVATE int do_blue_screen(void){
    blue_screen();
    return OK;
}

/*===========================================================================*
 *				do_copy					     *
 *			它很简单，给服务器提供蓝屏功能
 *===========================================================================*/
PUBLIC int do_copy(Message *msg_ptr){
    /* 处理系统级调用sys_copy。 从MM或FS复制数据。
     *
     * SYS_COPY消息是最常用的一条消息。它被用来允许文件系统和存储管理器从用户进程拷贝信息。
     * 当一个用户进程执行READ调用时，文件系统查看它的缓存中是否有所需的块。如果没有，它就向
     * 适当的磁盘任务发送一条消息把该块装入缓存。然后文件系统向系统任务发送一条消息通知它把
     * 该块拷贝到用户进程。在最坏的情况下，读一个块需要7条消息；在最好的情况下，需要4条消息。
     * 这些消息是现在现在系统开销的主要来源，是高度模块化设计的代价。它参考了MINIX的实现。
     * 处理SYS_COPY请求是很直接的。它由本例程完成，由提取消息参数和调用phys_copy以及其他的
     * 一些动作组成。
     * 如果将phys_copy直接暴露给服务器，那么开销将会变得较小，因为少了几条消息的发送，但是
     * 如果这么做了，消息通信和高度模块化的意义也就不存在了，它就和Windows的实现一样，混合
     * 了微内核和宏内核的优点，但作为一个学习的操作系统，我还是高举纯微内核大旗。
     */

    int src_proc, dest_proc, src_space, dest_space;
    vir_bytes src_vir, dest_vir;
    phys_bytes src_phys, dest_phys, bytes;

    /* 得到消息中的参数 */
    src_proc = msg_ptr->SRC_PROC_NR;
    src_vir = (vir_bytes)msg_ptr->SRC_BUFFER;
    src_space = msg_ptr->SRC_SPACE;

    dest_proc = msg_ptr->DEST_PROC_NR;
    dest_vir = (vir_bytes)msg_ptr->DEST_BUFFER;
    dest_space = msg_ptr->DEST_SPACE;

    bytes = (phys_bytes)msg_ptr->COPY_BYTES;

    /* 计算数据源地址（物理）和目标地址并进行复制 */
    if(src_proc == ABSOLUTE){
        /* 如果源数据来自于系统任务或源进程，说明给出的虚拟地址就是物理地址，无需转换 */
        src_phys = (phys_bytes)src_vir;
    } else {
        if(bytes != (vir_bytes)bytes){
            /* 这将可能发生在64K段和16位vir_bytes上。这种情况在do_fork中经常发生，
             * 但是MM在这种情况下使用的是ABS副本。
             */
            panic("overflow in count in do_copy", NO_NUM);
        }
        /* 转换 */
        src_phys = umap(proc_addr(src_proc), src_space, src_vir, (vir_bytes)bytes);
    }

    /* 故技重施 */
    if(dest_proc == ABSOLUTE){
        dest_phys = (phys_bytes)dest_vir;
    } else {
        dest_phys = umap(proc_addr(dest_proc), dest_space, dest_vir, (vir_bytes)bytes);
    }

    /* 如果目的地的地址有效，执行拷贝；否则错误返回。 */
    if( src_phys == 0 || dest_phys == 0 ) return EFAULT;
    phys_copy(src_phys, dest_phys, bytes);
//    printf("%ld ---%d---> %ld\n", src_phys, bytes, dest_phys);
    return OK;
}

/*===========================================================================*
 *				do_get_map					     *
 *			报告某个进程的内存映像
 *===========================================================================*/
PRIVATE int do_get_map(Message *msg_ptr){
    /* 虽然这个系统级调用是提供给所有服务器的，但是一般只有MM需要，
     * 我们报告一个进程的内存映像给调用者，它不难实现。
     */

    register Process *proc;
    phys_bytes dest_phys;       /* 内存映像应该送到的物理地址 */
    int caller;                 /* 调用者索引号 */
    int who;                    /* 想要谁的内存映像？ */
    MemoryMap *map_ptr;

    /* 获取消息中的参数 */
    caller = msg_ptr->source;
    who = msg_ptr->PROC_NR1;
    map_ptr = (MemoryMap*)msg_ptr->MEM_MAP_PTR;

    assert(is_ok_proc_nr(who));   /* 我们断言：这是个正确的进程索引号，因为服务器不应该传一个错误的值 */

    proc = proc_addr(who);      /* 得到进程实例，里面有我们需要的内存映像 */

    /* 好了，可以复制给服务器了。 */
    dest_phys = umap(proc_addr(caller), DATA, (vir_bytes)map_ptr, sizeof(proc->map));
    assert(dest_phys != 0);     /* 它也不应该发生 */
    phys_copy(vir2phys(&proc->map), dest_phys, sizeof(proc->map));

    return OK;
}


/*===========================================================================*
 *				do_new_map					     *
 *		MM报告了一个新进程的内存映像
 *===========================================================================*/
PRIVATE int do_new_map(Message *msg_ptr){
    /* 在一个FORK调用之后，存储管理器为子进程分配内存。内核必须知道子进程位于内存何处以在运行子进程时能正确
     * 设置段寄存器。SYS_NEW_MAP消息允许存储管理器传给内核任何进程的存储映象。
     */

    register Process *proc;
    phys_bytes src_phys;    /* 内存映像所在的物理地址 */
    int callnr;             /* 调用者索引号 */
    int who;                /* 谁的内存映像？ */
    int old_flags;          /* 修改前标记的值 */
    MemoryMap *map_ptr;     /* 映像的虚拟地址 */

    /* 获取消息中的参数 */
    callnr = msg_ptr->source;
    who = msg_ptr->PROC_NR1;
    map_ptr = (MemoryMap*)msg_ptr->MEM_MAP_PTR;
    if(!is_ok_proc_nr(who)) return (ERROR_BAD_PROC);    /* 啊哈，这个进程索引号不正确 */
    proc = proc_addr(who);

    /* 将映像复制过来 */
    src_phys = umap(proc_addr(callnr), DATA, (vir_bytes)map_ptr, sizeof(proc->map));
    assert(src_phys != 0);  /* 不可思议，MM竟然发送了一个错误的地址 */
    phys_copy(src_phys, vir2phys(&proc->map), (phys_bytes) sizeof(proc->map));

    /* 现在根据新的内存映像设置进程的LDT信息
     *  limit = size - 1
     */
    init_seg_desc(&proc->ldt[CS_LDT_INDEX],
                  proc->map.base,
                  (proc->map.size - 1) >> LIMIT_4K_SHIFT,
                  DA_32 | DA_LIMIT_4K | DA_C | USER_PRIVILEGE << 5
    );
    init_seg_desc(&proc->ldt[DS_LDT_INDEX],
                  proc->map.base,
                  (proc->map.size - 1) >> LIMIT_4K_SHIFT,
                  DA_32 | DA_LIMIT_4K | DA_DRW | USER_PRIVILEGE << 5
    );
//    printf("%s(nr-%d) base: %d, size: %d | ldt_sel: (c-%d|p-%d)\n", proc->name, proc->nr,
//            proc->map.base, proc->map.size, proc->ldt_sel, proc_addr(proc->nr - 1)->ldt_sel);

    old_flags = proc->flags;        /* 保存标志 */
    proc->flags &= ~NO_MAP;         /* 解开封印！将NO_MAP复位，等同于YES_MAP！！！ */
    /* 最后一步：确定旧的flags位上是否还存在除了NO_MAP以外限制进程运行的堵塞位，
     * 如果没有了，那么就可以将这个新生儿加入就绪队列了！
     */
    if(old_flags != 0 && proc->flags == 0) lock_ready(proc);
    /* Over!!! */
    return OK;
}

