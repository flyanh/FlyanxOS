/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 这个文件实现了进程相关的例程和服务，其通信结构参考自Andrew S. Tanenbaum教授的MINIX系统，
 * Flyanx最主要的进程间通信的高层代码就在这里。在这里，核心的任务是将一个硬件中断或软件中断转
 * 换为一条消息，前者由硬件产生，后者则是请求系统服务（系统调用）的途径。这两者很类似，以至可
 * 以用同一个函数处理，但将其分为两个专门的函数会更高效，且可读性会更好。
 *
 * 在这里，我们简单描述一下Flyanx中的进程间通信的过程。
 * Flyanx中的进程使用消息进行通信，这里使用到进程会合的原理。当一个进程执行send时，核心的最底层检查目
 * 标进程是否在等待从发送者（或任一发送者）发来的消息。如果是，则该消息从发送者的缓冲区拷贝到接收者的缓
 * 冲区，同时这两个进程都被标记为就绪态。如果目标进程未在等待消息，则发送者被标记为阻塞，并被挂入一个等
 * 待将消息发送到接收进程的进程队列中。
 * 当一个进程执行receive时，核心检查该队列中是否存在向它发送消息的进程。若有，则消息从被阻塞的发送进
 * 程拷贝到接收进程，并将两者均标记为就绪；若不存在这样的进程，则接收进程被阻塞，直到一条消息到达。
 *
 * 最后，我们概括一下Flyanx的进程调度
 * 调度算法维护三个优先级队列，分别对应I/O任务、服务器进程和用户进程。最高优先级队列的第一个进程总是被
 * 选中执行。系统任务和服务器进程被允许执行到堵塞位置，但时钟任务监视用户进程所使用的时间。若用户进程的
 * 时间片用完，它就被挂在其队列的尾部，这样便于在相互竞争的用户进程中达到了时间片轮转的调度效果。
 */
#include "kernel.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "process.h"

/* 进程正在切换则为TRUE，否则为FALSE；当为TRUE应该禁止硬件中断的产生，不然会
 * 产生一些严重的问题。
 */
PRIVATE u8_t switching;

#if (CHIP == INTEL)

#endif

/*===========================================================================*
 *				interrupt				     *
 *===========================================================================*/
PUBLIC void interrupt(task)
int task;           /* 要开始的任务号 */
{
    /* 在接受到一条硬件中断后，相应设备的底层中断服务例程将调用该函数。
     * 功能是将中断转换为向该设备所对应的系统任务发送一条消息，而且通常
     * 在调用interrupt之前几乎不进行什么操作。
     */
    register Process *proc; /* 进程指针，指向任务进程表项 */

    printf("interrupt(%d)\n", task);

    /* 得到目标任务的进程项 */


}

/*===========================================================================*
 *				     hunter   				     *
 *				     狩猎进程以用下次执行
 *===========================================================================*/
PRIVATE void hunter(){
    /* 从进程表中抓出一个作为下次运行的进程
     *
     * Flyanx有三个就绪进程队列，分别是任务、服务、进程(用户)
     * 我们的调度算法很简单：找到优先级最高的非空队列，并选择队首进程即可。
     * 如果所有队列均为空，则运行闲置进程IDLE。
     *
     * 选择最高优先级队列由本例程完成。
     * 该函数的主要作用是设置curr_proc(系统当前运行的进程)，任何影响到选择
     * 下一个运行进程的对这些队列的改变都要再次调用hunter。无论进程在什么时
     * 候阻塞，都调用curr_proc来重新调度CPU。
     */
    register Process *prey;      /* 准备运行的进程 */

    /* 就绪任务进程队列使我们狩猎的第一个目标 */
    if( (prey = ready_head[TASK_QUEUE]) != NIL_PROC){
        curr_proc = prey;
        return;
    }

    /* 寻找第二目标：就绪服务进程队列 */
    if( (prey = ready_head[SERVER_QUEUE]) != NIL_PROC ){
        curr_proc = prey;
        return;
    }

    /* 寻找第三目标：就绪用户进程队列
     * 这里多了一行代码：bill_proc = prey，这是标记当前正在运行的用户进程正在消费系统资源。
     * 这保证了对于一个即将运行的用户进程，将把系统为它所作的全部工作都记在它的账上。
     */
    if( (prey = ready_head[USER_QUEUE]) != NIL_PROC){
        curr_proc = prey;
        bill_proc  = prey;
        return;
    }
    /* 咳咳，本次狩猎失败了，那么只能狩猎IDLE闲置进程了，但这种情况较少发生 */
    prey = proc_addr(IDLE_TASK);
    bill_proc = curr_proc = prey;
//    printf("hunter --> %s\n", prey->name);
    /* 本例程只负责狩猎，狩猎到一个可以执行的进程，而进程执行完毕后的删除或更改在队列中的位置
     * 这种事情我们安排在其他地方去做。
     */
}

/*===========================================================================*
 *				ready					     *
 *				进程就绪
 *===========================================================================*/
PRIVATE void ready(proc)
register Process *proc;      /* 就绪的进程 */
{
    /* 将一个可运行的进程挂入就绪队列，它直接将进程追加到队列的尾部 */

    if(is_task_proc(proc)){     /* 系统任务？ */
        // 首先我们得知道，就绪队列已经有进程否？
        if(ready_head[TASK_QUEUE] != NIL_PROC){
            /* 就绪队列非空，挂到队尾 */
            ready_tail[TASK_QUEUE]->next_ready = proc;
        } else{
            /* 就绪队列是空的，那么这个进程直接就可以运行，并挂在就绪队列头上 */
            curr_proc = ready_head[TASK_QUEUE] = proc;
        }
        // 队尾指针指向新就绪的进程
        ready_tail[TASK_QUEUE] = proc;      /* 队尾指针-->新就绪的进程 */
        proc->next_ready = NIL_PROC;        /* 新条目没有后继就绪进程 */
        return;
    }

    if(is_serv_proc(proc)){     /* 系统服务？ */
        /* 同上 */
        if(ready_head[SERVER_QUEUE] != NIL_PROC){
            ready_tail[SERVER_QUEUE]->next_ready = proc;
        } else{
            curr_proc = ready_head[SERVER_QUEUE] = proc;
        }
        ready_tail[SERVER_QUEUE] = proc;
        proc->next_ready = NIL_PROC;
        return;
    }

    /* 用户进程的处理稍微有所不同
     * 我们将用户进程添加到队列的最前面。（对于受I/O约束的进程来说更公平一些。）
     */
    if(ready_head[USER_QUEUE] != NIL_PROC){
        ready_tail[USER_QUEUE] = proc;
    }
    proc->next_ready = ready_head[USER_QUEUE];
    ready_head[USER_QUEUE] = proc;
}

/*===========================================================================*
 *				unready					     *
 *===========================================================================*/
PRIVATE void unready(proc)
register Process *proc;     /* 未就绪的进程 */
{
    /**
     * 将一个不再就绪的进程从其队列中删除，即堵塞。
     * 通常它是将队列头部的进程去掉，因为一个进程只有处于运行状态才可被阻塞。
     * unready在返回之前要一般要调用hunter。一个未在运行的用户进程若收到一个信
     * 号也可能进入非就绪状态，若该进程没在队首，则将遍历整个USER_Q队列来查找它，一旦找到则将其删除。
     */
     register Process *xp;

     if(is_task_proc(proc)){        /* 系统任务？ */
         /* 如果系统任务的堆栈已经不完整，内核出错。 */
         if(*proc->stack_guard_word != SYS_TASK_STACK_GUARD){
             panic("stack over run by task", proc->nr);
         }

         xp = ready_head[TASK_QUEUE];   /* 得到就绪队列头的进程 */
         if(xp == NIL_PROC) return;     /* 并无就绪的系统任务 */
         if(xp == proc){
             /* 如果就绪队列头的进程就是我们要让之堵塞的进程，那么我们将它移除出就绪队列 */
             ready_head[TASK_QUEUE] = xp->next_ready;
             if(proc == curr_proc) {
                 /* 如果堵塞的进程就是当前正在运行的进程，那么我们需要重新狩猎以得到一个新的运行进程 */
                 hunter();
             }
             return;
         }
         /* 如果这个进程不在就绪队列头，那么搜索整个就绪队列寻找它 */
         while (xp->next_ready != proc){
             xp = xp->next_ready;
             if (xp == NIL_PROC) return;   /* 到边界了，说明这个进程根本就没在就绪队列内 */
         }
         /* 找到了，一样，从就绪队列中移除它 */
         xp->next_ready = xp->next_ready->next_ready;
         /* 如果之前队尾就是要堵塞的进程，那么现在我们需要重新调整就绪队尾指针（因为它现在指向了一个未就绪的进程） */
         if (ready_tail[TASK_QUEUE] == proc) ready_tail[TASK_QUEUE] = xp;   /* 现在找到的xp进程是队尾 */
     } else if(is_serv_proc(proc)){     /* 系统服务 */
         /* 所作操作同上的系统任务一样 */
         xp = ready_head[SERVER_QUEUE];
         if(xp == NIL_PROC) return;
         if(xp == proc){
             ready_head[SERVER_QUEUE] = xp->next_ready;
             /* 这里注意，因为不是系统任务，我们不作那么严格的判断了 */
             hunter();
             return;
         }
         while (xp->next_ready != proc){
             xp = xp->next_ready;
             if (xp == NIL_PROC) return;
         }
         xp->next_ready = xp->next_ready->next_ready;
         if (ready_tail[SERVER_QUEUE] == proc) ready_tail[SERVER_QUEUE] = xp;
     } else {                           /* 用户进程 */
         xp = ready_head[USER_QUEUE];
         if(xp == NIL_PROC) return;
         if(xp == proc){
             ready_head[USER_QUEUE] = xp->next_ready;
             /* 这里注意，因为不是系统任务，我们不作那么严格的判断了 */
             hunter();
             return;
         }
         while (xp->next_ready != proc){
             xp = xp->next_ready;
             if (xp == NIL_PROC) return;
         }
         xp->next_ready = xp->next_ready->next_ready;
         if (ready_tail[USER_QUEUE] == proc) ready_tail[USER_QUEUE] = xp;
     }
}

/*===========================================================================*
 *				schedule					     *
 *			     进程调度
 *===========================================================================*/
PRIVATE void schedule(){
    /* 这个调度程序只针对于用户进程
     * 尽管多数调度决策实在一个进程阻塞或解除阻塞时作出的，但调度仍要考虑
     * 到当前用户进程时间片用完的情况。这种情况下，时钟任务调度schedule来将
     * 就绪用户进程队首的进程移到队尾。
     * 该算法的结果是将用户进程按时间片轮转方式运行。文件系统、内存管理器
     * 和I/O任务绝不会被放在队尾，因为它们肯定不会运行得太久。这些进程可以
     * 被认为是非常可靠的，因为它们是我们编写的，而且在完成要做的工作后将堵塞。
     */

    /* 空队列... */
    if(ready_head[USER_QUEUE] == NIL_PROC) return;

    /* 将队首的用户进程移到队尾 */
    Process *temp;
    temp = ready_head[USER_QUEUE]->next_ready;
    ready_tail[USER_QUEUE]->next_ready = ready_head[USER_QUEUE];
    ready_tail[USER_QUEUE] = ready_tail[USER_QUEUE]->next_ready;
    ready_head[USER_QUEUE] = temp;
    ready_tail[USER_QUEUE]->next_ready = NIL_PROC;  /* 队尾没有后继进程 */
    /* 汉特儿 */
    hunter();
}

/*==========================================================================*
 *				lock_flyan_send 			    *
 *				加锁的，安全的flyanx_send例程
 *==========================================================================*/
PUBLIC int lock_flyan_send(caller_ptr, dest, message_ptr)
register struct process_s *caller_ptr;	/* 调用进程，即谁想发消息？ */
int dest;			                    /* 目标进程号，即谁将接收这条消息？或说这条消息将发给谁？ */
struct message_s *message_ptr;	        /* 消息 */
{
    int rs;
    switching = TRUE;
    rs = flyanx_send(caller_ptr, dest, message_ptr);
    switching = FALSE;
    return rs;
}

/*==========================================================================*
 *				    lock_hunter				    *
 *				    加锁的，安全的进程狩猎例程
 *==========================================================================*/
PUBLIC void lock_hunter(){
    switching = TRUE;
    hunter();
    switching = FALSE;
}

/*==========================================================================*
 *				    lock_ready				    *
 *				    加锁的，安全的进程就绪例程
 *==========================================================================*/
PUBLIC void lock_ready(proc)
Process *proc;
{
    printf("process %s ready.\n",proc->name);
    switching = TRUE;
    ready(proc);
    switching = FALSE;
}

/*==========================================================================*
 *				lock_unready				    *
 *				加锁的，安全的进程堵塞例程
 *==========================================================================*/
PUBLIC void lock_unready(proc)
Process *proc;
{
    switching = TRUE;
    unready(proc);
    switching = FALSE;
}

/*==========================================================================*
 *				lock_schedule				    *
 *				加锁的进程调度方法
 *==========================================================================*/
PUBLIC void lock_schedule()
{
    switching = TRUE;
    schedule();
    switching = FALSE;
}



