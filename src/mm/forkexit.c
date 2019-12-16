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
 */

#include "mm.h"
#include <sys/wait.h>
#include <flyanx/callnr.h>
#include <signal.h>
#include <string.h>
#include "mmproc.h"
#include "param.h"

#define LAST_FEW        3   /* 为超级用户保留的最后几个进程插槽 */

PRIVATE pid_t next_pid = ORIGIN_PID;    /* 下一个要分配的pid */

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
        next_pid = (next_pid < 5000 ? next_pid + 1 : ORIGIN_PID + 1);
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
    /* 这个例程接收EXIT调用，但全部工作都是mm_exit()做的。这样划分是因为mm_exit()也
     * 被用来处理被信号终止运行的进程。两者工作相同，但参数不同，所以这样划分是很方便的。
     */
    mm_exit(curr_mp, m_status);
    return (ERROR_NO_MESSAGE);      /* 死人不能再讲话 */
}

/*===========================================================================*
 *				mm_exit					     *
 *===========================================================================*/
PUBLIC void mm_exit(MMProcess *rmp, int exit_status){

}

/*===========================================================================*
 *				do_wait				     *
 *===========================================================================*/
PUBLIC int do_wait(void){

}
