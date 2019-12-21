/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 此文件包含处理FORK、EXIT、和WAIT系统调用的实现
 * 本文件的入口点是：
 *  - do_fork：      处理FORK系统调用
 *  - do_exit：      处理EXIT系统调用
 *  - do_wait：      处理WAIT系统调用
 *  - do_waitpid：   处理WAITPID系统调用
 */

#include "mm.h"
#include <sys/wait.h>
#include <flyanx/callnr.h>
#include <signal.h>
#include "mmproc.h"
#include "param.h"

#define LAST_FEW        3   /* 为超级用户保留的最后几个进程插槽 */

PRIVATE pid_t next_pid = ORIGIN_PID;    /* 下一个要分配的pid，我们初始化为源进程的pid，因为分配时会进行+1操作 */

FORWARD _PROTOTYPE( void exit_cleanup, (MMProcess *exit_proc, MMProcess *wait_parent) );
FORWARD _PROTOTYPE( int mm_waitpid, (int pid, int options) );

/*===========================================================================*
 *				do_fork					     *
 *			处理FORK系统调用
 *===========================================================================*/
PUBLIC int do_fork(void){
    /* 这个调用很重要，也很复杂。当一个进程想要创建一个新进程的时候，
     * 就需要使用FORK调用生成一个新的进程分支，这个新进程是调用进程
     * 的子进程，可以共享父进程的一些资源，例如：打开的文件。
     * 该文件要完成的事情很多，因为一个新进程的产生需要为它在进程表中
     * 找到一个插槽，同时还需要设置它新的正文段等等...
     */

    register MMProcess *parent;     /* 指向父进程，即调用者 */
    register MMProcess *child;      /* 指向fork出来的子进程 */
    MMProcess *search;
    int child_nr, cp_rs;
    bool pid_in_use;
    phys_bytes child_base;

    /* 如果在FORK的时候，进程表已经满了，那么我们就没必要开始了，
     * 避免一些不必要的麻烦，因为如果失败了，恢复过程麻烦的你不敢
     * 想象，而且非常容易出错。
     */
    parent = curr_mp;
    if(procs_in_use == NR_PROCS) return EAGAIN;
    /* 为超级用户保留的最后几个进程表项，也不能使用，对于普通用户。 */
    if(parent->effective_uid != SUPER_USER &&
            procs_in_use >= NR_PROCS - LAST_FEW) return EAGAIN;

    /* ========== 好了，通过了上面的检查，可以开始创建子进程的操作了 ========== */
    /* 子进程需要继承父进程的：正文段、数据段和堆栈段。
     * 我们先进行分配内存，分配大小是父进程的内存大小。
     * 先进行内存分配是为了防止内存不足导致失败可以没
     * 有任何后果，因为新的进程表项还没有进行分配。
     */
    child_base = alloc_mem(parent->map.size >> CLICK_SHIFT) << CLICK_SHIFT;
    if(child_base == NO_MEM) return ENOMEM;     /* 空间分配失败... */

    /* 子进程是父进程的一个拷贝，我们拷贝父进程的数据到新分配的地址。*/
    if(parent->pid == ORIGIN_PID){
        /* 如果要进行FORK操作的是起源进程，那么复制可以从内核的基地址（挂载点）开始，
         * 这样可以节约时间，因为内核挂载点前面的数据对于起源进程是没有用的。
         */
        cp_rs = sys_copy(ABSOLUTE, 0, KERNEL_BASE,    /* 从这 */
                         ABSOLUTE, 0, child_base + KERNEL_BASE,    /* 到这 */
                         parent->map.size);                    /* 拷贝多少字节？ */
    } else {
        /* 正常情况 */
        cp_rs = sys_copy(ABSOLUTE, 0, parent->map.base,    /* 从这 */
                         ABSOLUTE, 0, child_base,         /* 到这 */
                         parent->map.size);                         /* 拷贝多少字节？ */
    }
    if(cp_rs < 0) mm_panic("do_fork can't copy to child", cp_rs);

    /* 现在，我们必须找到一个空的进程插槽给子进程 */
    for(child = &mmproc[0]; child < &mmproc[NR_PROCS]; child++){
        if(!(child->flags & IN_USE)){
            /* 找到了，停止寻找 */
            break;
        }
    }

    /* 得到子进程索引号 */
    child_nr = child - &mmproc[0];  /* 得到子进程的进程索引号 */
    procs_in_use++;     /* 一个新的进程被使用了 */
    /* 设置子进程信息及其内存映像，子进程继承父进程的结构（MM中的） */
    *child = *parent;
    child->parent = mm_who;     /* 不要忘了父亲是谁 */
    child->flags &= IN_USE;     /* 这个进程插槽已经被使用了，这很重要。 */

    /* 为子进程找到一个可用的进程号，并将其放入进程表中 */
    do{
        pid_in_use = FALSE;     /* 每次寻找前嘉定pid是可用状态 */
        /* 理论上可能发生以下的问题：
         * 在把一个进程号，例如17，赋值给了一个非常长寿的进程之后，可能会有将近很多很多
         * 的进程被它创建和撤销，我就认为可能有50000个吧，next_pid可能又再次回到回到17。
         * 指定一个我们仍然在使用的进程号是一场灾难（想想，随后某个进程向进程17发送信号的
         * 情形，有多少个进程会收到这个信号呢？）所以我们需要搜索整个进程表以确定被指定的
         * 进程号有没有被使用。虽然有点耗费时间，但是还好的是进程表并不大，只有32个用户进
         * 程能同时存在。
         */
        next_pid = (next_pid < 50000 ? next_pid + 1 : ORIGIN_PID + 1);
        for(search = &mmproc[0]; search < &mmproc[NR_PROCS]; search++){
            if(search->pid == next_pid || search->proc_group == next_pid){
                pid_in_use = TRUE;
                break;  /* 结束这次查找 */
            }
        }
        child->pid = next_pid;      /* 我们分配给子进程。 */
    } while (pid_in_use == TRUE);   /* 找到的进程号依然被使用，就继续 */

    /* 更新子进程的内存映像，子进程的正文段，数据段和堆栈段基址必须引用新分配的 */
    child->map.base = child_base;
    child->exit_status = 0;     /* 退出状态置位 */

    /* 现在告诉内核，一个新进程出现了，内核将会更新内核中的进程信息 */
    sys_fork(child_nr, mm_who, child->pid);
    /* 好了，通知文件系统已经有一个新进程成功创建了! */
    mm_tell_fs(FORK, child_nr, mm_who, child->pid);

    /* 万事具备，就差最后一步，新进程还需要设置自己的内存映像，
     * 它虽然继承自父进程，但它应该有自己的LDT。但是MM自己做不
     * 到，所以需我们需要通知内核来完成。
     */
    sys_new_map(child_nr, &child->map);

    /* 子进程的生日！我们给它发一条消息唤醒它。 */
    set_reply(child_nr, 0);

    /* 返回子进程的进程号 */
    return child->pid;
}

/*===========================================================================*
 *				do_exit				     *
 *			 处理系统调用EXIT
 *===========================================================================*/
PUBLIC int do_exit(void){
    /* 这个例程接收EXIT调用，但全部工作都是mm_exit()做的。
     * 这样划分是因为POSIX要求应该实现信号，但我们还没有实
     * 现，当以后信号被支持以后，被信号终止运行的进程也需要
     * 做进程退出工作，但它们给出的参数不同，所以我可以先将
     * 其摆在这里为以后信号的实现提供便利。
     */
    mm_exit(curr_mp, m_status);
    return ERROR_NO_MESSAGE;    /* 死人不能再讲话 */
}

/*===========================================================================*
 *				mm_exit					     *
 *				进程退出
 *===========================================================================*/
PUBLIC void mm_exit(
        MMProcess *exit_proc,   /* 退出进程 */
        int exit_status         /* 退出状态 */
){
    /* 本例程首先将释放大部分退出进程的资源，如果它的父进程还在等待它，
     * 释放余下的所有部分；否则我们保留退出进程的插槽，它称为一个僵死的
     * 进程。
     */

    MMProcess *wait_parent; /* 可能在等待退出进程完成退出的父进程 */
    register int proc_nr;   /* 退出进程的进程索引号 */

    /* 检查一些问题 */
    proc_nr = exit_proc - &mmproc[0];
    if(exit_proc == NIL_MPROC) { /* 空进程，大问题，空进程如何调用的exit()？ */
        mm_panic("a NIL Proc try to exit, proc nr in code", proc_nr);
        return;     /* 让编译器闭嘴 */
    }


    /* 好了，在这我们已经可以确定这个进程是一个正常调用exit()的进程了。
     * 我们现在报告领导（内核）和同事（FS、FLY）该进程退出了。
     * 报告参数：退出进程的进程索引号以及父亲的进程索引号
     */
    mm_tell_fs(EXIT, proc_nr, exit_proc->parent, 0);
    sys_exit(proc_nr, exit_proc->parent);

    /* 释放退出进程所占的内存 */
    free_mem(exit_proc->map.base >> CLICK_SHIFT, exit_proc->map.size >> CLICK_SHIFT);

    /* 设置退出状态 */
    exit_proc->exit_status = exit_status;

    /* 检查父进程是否在等待子进程退出，如果在等待，请解除父进程的
     * 等待状态，使父进程能继续运行。
     */
    wait_parent = &mmproc[exit_proc->parent];
    if(wait_parent->flags & WAITING){       /* 父进程在等待退出进程 */
        /* 退出清理工作，告诉父进程一个子进程已经退出并且进程插槽已经被释放。 */
        exit_cleanup(exit_proc, wait_parent);
    } else {                                /* 父进程并不等待 */
        exit_proc->flags = IN_USE | ZOMBIE; /* 僵尸进程 */
    }

    /* 寻找进程表，如果退出进程还存在子进程，那么设置这些子进程的父亲为源进程，随后
     * 完成这些进程的退出工作。
     */
    for(exit_proc = &mmproc[0]; exit_proc < &mmproc[NR_PROCS]; exit_proc++){
        if(exit_proc->parent == proc_nr){   /* 找到了！ */
            exit_proc->parent = ORIGIN_PROC_NR;
            wait_parent = &mmproc[ORIGIN_PROC_NR];

            if(wait_parent->flags & WAITING){ /* 源进程正在等待子进程 */
                /* 如果某个子进程进入了僵死状态，做退出清理工作。 */
                if(exit_proc->flags & ZOMBIE) exit_cleanup(exit_proc, wait_parent);
            }
        }
    }
}

/*===========================================================================*
 *				do_wait				     *
 *			处理WAIT系统调用
 *===========================================================================*/
PUBLIC int do_wait(void){
    /* 如果进程执行了wait()，那么进程A将会被堵塞等待直到一个子进程完成运行（或被终止）。
     * WAIT会完成以下操作：
     *  1 - 遍历进程表，寻找进程A的子进程B，如果找到有僵尸进程。
     *      - 清理掉子进程并回复A使其恢复运行。
     *      - 释放该子进程的进程表条目。
     *  2 - 如果没有找到任何子进程是僵尸
     *      - 进程A进入等待，设置标志位WAITING位。
     *  3 - 如果进程A没有任何子进程
     *      - 回复一个错误给进程A（没有子进程你瞎调用干啥）。
     * 当然了，上面这些事情将会在mm_waitpid里实现。
     */

    /* pid：-1，代表等待所有子进程
     * options：0，这个参数wait并没有，所以无所谓，随便给就好，你给个666也行。
     */
    return mm_waitpid(-1, 0);
}

/*===========================================================================*
 *				do_waitpid				     *
 *			    处理WAITPID调用
 *===========================================================================*/
PUBLIC int do_waitpid(void){
    /* 它和wait的区别就是本调用可以可以非常精准的等待某一个
     * 子进程，而不是所有的。同时，waitpid还允许设置等待选
     * 项，如果等待的子进程还在运行，也可以不堵塞自己，继续
     * 运行。
     */
    return mm_waitpid(m_pid, m_sig_nr);
}

/*===========================================================================*
 *				mm_waitpid				     *
 *			do_waitpid的真正处理函数
 *===========================================================================*/
PRIVATE int mm_waitpid(int pid, int options){
    /* 本例程实现waitpid的功能，将其拿出来是因为wait和waitpid本质上没有什么
     * 区别，可以这么说，wait只是waitpid的一个子集，放在这，可以服用这些可以
     * 重复的代码。
     */

    register MMProcess *proc;
    int child_count = 0;        /* 记录子进程数量和 */

    /* 等待的pid等于0>？那么说明进程要等待一个进程组运行完成。 */
    if(pid == 0) pid = -(curr_mp->proc_group);  /* 转化为组号 */

    /* 遍历所有进程
     * 这里需要注意一下，通过pid参数我们可以判断出进程具体要等什么：
     *  - pid  >  0：这意味着进程正在等待一个特定进程，pid是这个进程的进程号
     *  - pid == -1：这意味着进程等待自己的任何一个子进程
     *  - pid  < -1：这意味着进程等待一个进程组里的进程
     */
    for(proc = &mmproc[0]; proc < &mmproc[NR_PROCS]; proc++){
        if(proc->parent == mm_who){             /* 是调用者的子进程吗？ */
            if(pid > 0 && pid != proc->pid)continue;
            if(pid < -1 && -pid != proc->proc_group)continue;
            /* 只有 pid == -1 的情况才能到下面 */
            child_count++;                      /* 记录调用者的子进程数量 */
            if(proc->flags & ZOMBIE){           /* 找到了一个僵尸进程 */
                exit_cleanup(proc, curr_mp);    /* 清理掉这个僵尸进程并答复调用者 */
                return ERROR_NO_MESSAGE;        /* 已经设置了回复，所以不需要回复了 */
            }
        }
    }

    if(child_count > 0){                        /* 没有找到任何子进程是僵尸 */
        if(options & WNOHANG) return 0;         /* 如果进程表示不需要等待子进程，那么请直接回复结果 */
        curr_mp->flags |= WAITING;              /* 父进程希望等待子进程完成 */
        curr_mp->wait_pid = (pid_t)pid;         /* 保存起来等待的进程号 */
        return ERROR_NO_MESSAGE;                /* 没有回复，让调用者等待 */
    } else {
        return ECHILD;                          /* 没有任何子进程 */
    }
}

/*===========================================================================*
 *				exit_cleanup				     *
 *				进程退出清理工作
 *===========================================================================*/
PRIVATE void exit_cleanup(
        register MMProcess *exit_proc,  /* 退出的进程 */
        MMProcess *wait_parent          /* 上面那个的老爸 */
){
    /* 完成进程的退出，做一些清理等善后的工作
     * 当一个进程已经结束运行并且它的父进程在等待它的时候，不管这
     * 些事件发生的次序如何，本例程都将被调用执行完成最后的操作。
     * 这些操作包括：
     *  - 解除父进程的等待状态
     *  - 发送一条消息给父进程使其重新运行
     *  - 释放退出进程的进程槽位
     *  - 归还
     */
    int exit_status;

    /* 父进程解除等待状态 */
    wait_parent->flags &= ~WAITING;

    /* 唤醒父进程 */
    exit_status = exit_proc->exit_status;
    wait_parent->reply_rs2 = exit_status;
    set_reply(exit_proc->parent, exit_proc->pid);

    /* 释放进程槽位，减少计数 */
    exit_proc->flags = 0;
    procs_in_use--;
}

