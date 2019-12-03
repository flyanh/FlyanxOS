/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/11.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含flyanx的消息通信机制
 * 这是实现微内核的核心，应当仔细研究。同时，微内核一直以来被诟病的性能问题。
 * 大部分性能消耗都来自于消息通信，因为发送消息需要在内存中复制消息，而这项
 * 工作需要消耗很多的系统资源，所以如果想优化flyanx内核，第一步就是解决消息
 * 的资源消耗问题。
 *
 * 该文件的入口点：
 *  - flyanx_send              向某个进程发送一条消息
 *  - flyanx_receive           接收来自某个进程的消息
 *
 * 注意：这两个调用都会导致进程进入堵塞状态。
 */

#include "kernel.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "protect.h"
#include "process.h"
#include "message.h"

#ifdef INTEL
/* 复制消息的宏，就是简单的调用了phys_copy例程，它通过物理地址复制，
 * 速度较慢。后期将会改进。
 */
#define CopyMsg(src, src_msg, dest,  dest_msg) src_msg->source = src; \
    phys_copy(proc_vir2phys(proc_addr(src), (vir_bytes)src_msg), proc_vir2phys(proc_addr(dest), (vir_bytes)dest_msg), MESSAGE_SIZE)
#endif

FORWARD _PROTOTYPE( void copy_msg, (int src, Message *src_msg, Message *dest_msg) );

/*===========================================================================*
 *				flyanx_send	    		     *
 *				向一个进程发送一条消息             *
 *===========================================================================*/
PUBLIC int flyanx_send(caller_ptr, dest, message_ptr)
register struct process_s *caller_ptr;	/* 调用进程，即谁想发消息？ */
int dest;			                    /* 目标进程号，即谁将接收这条消息？或说这条消息将发给谁？ */
Message *message_ptr;          /* 消息 */
{
    /* 发送一条消息从发送进程到接收进程，消息在发送进程的数据空间中，所以我们
     * 需要将其复制到接收进程的数据空间的消息缓冲中。
     *
     * 由于消息机制要考虑的东西过多，以下我将可能用拟人手法进行描述，它更容易让人理解。
     */
    register Process *dest_proc, *next_proc;
    vir_bytes v_bytes;      /* 消息缓冲区指针的虚拟地址 */
    vir_clicks vlo, vhi;    /* 虚拟内存块包含要发送的消息 */

    /* 如果用户试图绕过系统服务直接发送消息给系统任务，返回错误，这是禁止的操作 */
    if(is_user_proc(caller_ptr) && !is_sys_server(dest)) return ERROR_BAD_DEST;
    dest_proc = proc_addr(dest);       /* 得到目标进程实例 */

    /* 如果目标进程已经不是一个活跃进程，出错返回 */
    if(is_empty_proc(dest_proc)) return ERROR_BAD_DEST;

//    printf("%s want to send message to %s\n", caller_ptr->name, dest_proc->name);

    /* 检查消息位置@TODO */

    /* 通过‘调用进程’和‘目标进程’相互发送消息来检查是否存在死锁
     * 它确保消息的目标进程没有正在试图向调用进程发送一条反向的
     * 消息，如果出现了，将会发生一个死链，大家互相等待对方的消
     * 息，但是永远等不到结果，形成了死锁。
     */
    if(dest_proc->flags & SENDING){     /* 对方正在发送一条消息 */
        next_proc = proc_addr(dest_proc->send_to);  /* 得到对方想要发送消息给的目标 */
        while (TRUE){
            /* 巧了这不，发送链上果真有人也想发送消息给我，为了避免循环死锁，这次消息我主动放弃发送 */
            if(next_proc == caller_ptr) return (ERROR_LOCKED);
            if(next_proc->flags & SENDING){     /* 只要发送链上的人是处于发消息的状态，继续往下找 */
                /* 如果发送链上的人没想发消息给我，那么得到他的下一个目标，继续查找 */
                next_proc = proc_addr(next_proc->send_to);
            } else break;       /* 很好，整个发送链上都没有人会再反向发送一条消息给我，说明这次的消息发送很安全 */
        }
    }

    /* 开始最关键的测试
     * 我们首先看一下对方是不是在接收消息的状态上，如果他在等待，我们就问一下他：“你在等谁啊？”
     * 如果他正好在等待我或者是任何人，那么我们就可以给它发送消息，将消息拷贝给它。
     */
    if((dest_proc->flags & (RECEIVING | SENDING)) == RECEIVING  /* RECEIVING|SENDING是为了保证对方不处于SEND_REC调用上 */
        && (dest_proc->get_form == ANY || dest_proc->get_form == caller_ptr->nr)){
        /* 调用CopyMsg复制消息给对方 */
        CopyMsg(caller_ptr->nr, message_ptr, dest_proc->nr, dest_proc->message);
        /* 好了，拿到了消息，解除对方接收消息堵塞的状态 */
        dest_proc->flags &= ~RECEIVING;

        /* 如果对方收到消息后不再堵塞，那么让对方可以就绪了 */
        if(dest_proc->flags == 0) ready(dest_proc);
    } else {
        /* 如果对方并没有堵塞，或者他被堵塞但不是在等待我
         * 那么堵塞我自己（发送消息的人）并开始排队。
         */
//        printf("unready self\n");
        caller_ptr->message = message_ptr;  /* 保存我没送成功的消息 */
        if(caller_ptr->flags == 0) unready(caller_ptr); /* 堵塞我自己 */
        caller_ptr->flags |= SENDING;   /* 进入状态：发送消息中 */
        caller_ptr->send_to = dest;     /* 保存对方的信息 */

        /* 现在我被堵塞了，加入对方的排队队列。 */
        next_proc = dest_proc->caller_head;
        if( next_proc ==  NIL_PROC){
            /* 如果排队队列为空，那么我就是队头 */
            dest_proc->caller_head = caller_ptr;
        } else {
            /* 排队队列不为空，那么我加入队尾 */
            while (next_proc->caller_link != NIL_PROC){
                next_proc = next_proc->caller_link;
            }
            next_proc->caller_link = caller_ptr;
        }
        caller_ptr->caller_link = NIL_PROC;
    }
    return OK;
}

/*===========================================================================*
 *				flyanx_receive	    		     *
 *				接收来自某个进程的消息             *
 *===========================================================================*/
PUBLIC int flyanx_receive(
        register Process *caller_ptr,	    /* 准备获取(接收)消息的进程 */
        int src,			                /* 准备从哪个源进程接收消息（可以是任何），也就是发送消息的进程 */
        Message *message_ptr			    /* 消息 */
)
{
    /* 一个进程想要接收一条消息
     *
     * 如果对方已经在排队了，那么就接收他的消息并唤醒对方。如果没有所需来源的消息，那么
     * 堵塞自己，告诉外界我正在等待一条消息。我们无需检查参数的有效性，因为用户调用只能
     * 是send_receive()，所以在flyanx_send()中我们已经检查，而来自任务和服务器（MM、
     * FS和FLY）的调用是受信任的。
     */

    register Process *sender_proc;
    register Process *previous_proc;

//    if(src == ANY){
//        printf("%s want to receive message from %s\n", caller_ptr->name, "ANY ONE");
//    } else {
//        printf("%s want to receive message from %d\n", caller_ptr->name, src);
//    }


    /* 检查要有没有要发消息过来的对方且符合我的要求 */
    if(!(caller_ptr->flags & SENDING)){ /* 只要我没处于发送消息状态就行 */
        /* 遍历查看给我发消息的排队队列 */
        for(sender_proc = caller_ptr->caller_head; sender_proc != NIL_PROC;
            previous_proc = sender_proc, sender_proc = sender_proc->caller_link){
            /* 我如果接收任何人的消息 或者 找到了我期望发送消息给我的对方，那么可以拿到对方的消息了 */
            if(src == ANY || src == sender_proc->nr){
                CopyMsg(sender_proc->nr, sender_proc->message, caller_ptr->nr,  message_ptr);
                if(sender_proc == caller_ptr->caller_head){
                    /* 如果对方是排队队列的第一个（头），那么排队队列的头更改为下一个 */
                    caller_ptr->caller_head = sender_proc->caller_link;
                } else {
                    /* 如果对方不是队头，那么对方出队，然后下一个排队的人顶替在对方原来的位置 */
                    previous_proc->caller_link = sender_proc->caller_link;
                }
                sender_proc->flags &= ~SENDING;
                if(sender_proc->flags == 0){
                    /* 取消对方的发送状态，如果对方不再堵塞，那么就绪他 */
                    ready(sender_proc);
                }
                return OK;
            }
        }

        /* 如果排队队列中未找到合适的发送者，则检查我自己的int_blocked标志是否标明了自己
         * 有一条中断消息没有处理被堵塞了，如果是则构造一条硬件消息-因为来自HARDWARE硬件
         * 的消息无非就是其目标进程域为HARDWARE，类型域位HARD_INT，没有更多，所以没必要
         * 复制消息。
         *
         * 这里刚好跟interrupt中断留下的问题前后呼应，这里解决这个问题。因为我还没准备好
         * 中断而进入堵塞导致无法收发消息的问题。这里解决后，我就可以继续正常接收消息了。
         */
        if(caller_ptr->int_blocked && is_any_hardware(src)){
            message_ptr->source = HARDWARE;
            message_ptr->type = HARD_INT;
            caller_ptr->int_blocked = FALSE;
            return OK;
        }
    }

    /* 如果既没有找到我期望的对方，我也没有因为未处理的硬件中断而导致被堵塞，那么现在
     * 我们等的对方根本就没发消息过来给我！
     * 我们现在将所期望的对方信息和我的收件地址（消息地址）保存起来，并且堵塞自己，堵
     * 塞状态是RECEIVING接收消息中。
     * 如果我的flags标志位还有其他标志被置位了，那么一个信号可能被挂起了，于是我应该
     * 很快就得处理该信号，所以我们就没必要堵塞自己了，这样可以提高进程响应性能。
     */
    caller_ptr->get_form = src;
    caller_ptr->message = message_ptr;
    if(caller_ptr->flags == 0) unready(caller_ptr);
    caller_ptr->flags |= RECEIVING;

    /* 处理信号 @TODO */

    return OK;
}

/*===========================================================================*
 *				proc_vir2phys				     *
 *			进程的虚拟地址转化为物理地址
 *===========================================================================*/
PUBLIC phys_bytes proc_vir2phys(Process *proc, vir_bytes vir){
    /* 这个函数和vir2phys宏的区别就是，本例程的虚拟地址是针对于
     * 一个进程的段地址转换的，而vir2phys只能转换内核和任务的物
     * 理地址。
     */

    /* 如果该进程是系统任务，那么它的数据段就是内核数据段，
     * 所以直接用vir2phys宏去计算即可
     */
    if(proc->priority == PROC_PRI_TASK) {
        return vir2phys(vir);
    }

    /* ===== 进程是服务或用户进程，需要我们计算一下 ===== */

    /* 首先得到该进程数据段的物理地址 */
    phys_bytes seg_base = ldt_seg_phys(proc, DATA);
    /* 该虚拟地址对应的物理地址 = 进程段地址 + 虚拟地址 */
    phys_bytes vir_phys = seg_base + vir;

    return vir_phys;
}

/*===========================================================================*
 *				ldt_seg_phys				     *
 *		得到一个进程本地段描述符的物理地址
 *===========================================================================*/
PUBLIC phys_bytes ldt_seg_phys(Process *proc, int seg_index){
    SegDescriptor *d = &proc->ldt[seg_index];
    return d->base_high << 24 | d->base_middle << 16 | d->base_low;
}

/*===========================================================================*
 *				sys_call				     *
 *				系统调用
 *===========================================================================*/
PUBLIC int sys_call(function, src_dest, message_ptr)
int function;	        /* 发送，接收，或者发送接收 */
int src_dest;	        /* 既是目标进程，也是源进程 */
Message *message_ptr;   /* 消息地址 */
{
    /* 系统调用与interrupt类似：它将一个软件中断（即用来激发一条系统调用的INT_VECTOR_SYS386_CALL指令）转换为一条消息。
     * 但由于在这种情况下源和目的的可能范围很宽，并且该调用可能需要发送、或接收、或既发送又接收一条消息，
     * 所以系统调用要做的事情更多。但sys_call的代码比较简短，因为它的工作通过调用其他过程来完成。
     */

    register Process *caller;
    int n;

    /* 检查并保证该消息指定的源进程目标地址合法，不合法直接返回错误代码E_BAD_SRC，即错误的源地址 */
    if(!is_ok_src_dest(src_dest)) return ERROR_NO_PERM;
    caller = curr_proc;     /* 通过检查,那么当前运行的进程就是发起一个合法的系统调用的消息发送方 */

    /* 我们继续检查，检查该调用是否从一个用户进程发出
     * 如果是用户进程，但它请求了一个非SEND_REC的请求，那么返回一个E_NO_PERM的错误码，表示用户
     * 不能越过服务发送消息给任务。因为SEND和RECEIVE是给任务间消息通信设置的。
     * 用户只能请求SEND_REC，首先申请发出一条消息，随后接收一个应答，对于用户进程而言，这是唯一
     * 一种被系统允许的系统调用方式。
     */
    if(is_user_proc(caller) && function != SEND_REC) return ERROR_NO_PERM;

    /*==============================*/
    /* 下面开始真正处理消息通信调用机制 */
    /*==============================*/

    /* 处理发送消息（包括SEND_REC里的SEND操作）操作 */
    if(function & SEND){
        n = flyanx_send(caller, src_dest, message_ptr); /* 发送消息 */

        /* 如果是发送操作，不管成功与否，返回代码 */
        if(function == SEND){
            return n;       /* 发送操作完成， */
        }

        /* 如果是发送并接收操作，发送失败，返回错误代码，不再有下文 */
        if(n != OK){
            return n;       /* 发送失败 */
        }

        /* flyan的话：
         * 上面的两个if判断可以合二为一，变为以下的方式
         * if (function == SEND || n != OK)
		 *      return(n);
         * 我不这么做的原因是这样可读性太差了，不信你可以试着去理解一下上面
         * 合二为一的代码...我希望写一些通俗易懂的代码，即使他们不够优雅...
         */
    }

    /* 处理接收消息操作，同样的，也包括SEND_REC里的REC操作
     * 直接调用接收消息的函数，并返回操作代码，例程结束
     */
    return (flyanx_receive(caller, src_dest, message_ptr));
}



