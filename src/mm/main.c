/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 内存管理器的主程序文件，是内存管理器的入口。
 */

#include "mm.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mmproc.h"
#include "param.h"

#define click2round_kb(n)   \
    ((unsigned) ((((unsigned long) (n) << CLICK_SHIFT) + 512) / 1024))

FORWARD _PROTOTYPE( void mm_init, (void) );

/*===========================================================================*
 *				mm_main					     *
 *			内存管理器入口
 *===========================================================================*/
PUBLIC void mm_main(void){
    int rs, proc_nr;
    int rec_state, call;
    MMProcess *mp;

    /* 初始化 */
    mm_init();

    mm_print_info("working...");
    /* 内存管理器开始工作了 */
    while (TRUE){
        /* 等待一条消息 */
        rec_state = receive(ANY, &mmsg_in);
        /* 接收消息错误 */
        if(rec_state != OK) mm_panic("MM receive msg error", NO_NUM);
        /* 得到调用者和功能调用号 */
        who = mmsg_in.source;
        call = mmsg_in.type;
        /* 得到调用的进程实例，如果调用者插槽号<0，则可能是系统任务在调用，设置为MM的插槽号 */
        mp = &mmproc[who < 0 ? MM_PROC_NR : who];

        /* 如果调用号有效，则执行调用 */
        if((unsigned) call >= NR_CALLS){
            rs = ENOSYS;
        } else {
            rs = (mm_call_handlers[call])();
        }

        /* 将结果告诉用户以指示此次调用完成 */
        if(rs != ERROR_NO_MESSAGE) set_reply(who, rs);

        /* 发送所有未处理的回复消息，包括上述调用的结果。不能换出进程。 */
        for(proc_nr = 0, mp = mmproc; proc_nr < NR_PROCS; proc_nr++, mp++){
            if(mp->flags & REPLY){      /* 存在回复标记 */
                if(send(proc_nr, &mp->reply) != OK){    /* 回答它并检查回复成功状态 */
                    mm_panic("MM can't reply a message to any", proc_nr);
                }
                mp->flags &= ~REPLY;    /* 回复过了，解除标记 */
            }
        }
    }

}

/*===========================================================================*
 *				set_reply				     *
 *			   设置回复消息
 *===========================================================================*/
PUBLIC void set_reply(
        int proc_nr,    /* 回复的进程 */
        int rs          /* 调用结果（通常是OK或错误号） */
){
    /* 通过调用的结果填写回复消息，以便稍后发送给用户进程。
     * 系统调用有时会填写消息的其他字段。这仅仅适用于主要返
     * 回值以及用于设置“必须发送回复”标志。
     */
    register MMProcess *rmp = &mmproc[proc_nr];
    rmp->reply_type = rs;
    rmp->flags |= REPLY;    /* 挂起了一个回复，等待处理 */
}

/*===========================================================================*
 *				mm_init					     *
 *		    内存管理器初始化
 *===========================================================================*/
PRIVATE void mm_init(void){
    register int proc_nr;
    register MMProcess *rmp;
    phys_clicks total_clicks;    /* 总物理内存大小 */
    phys_clicks free_clicks;     /* 空闲的物理内存大小 */

    /* 初始化内存管理器所有的进程表项 */
    for(proc_nr = 0; proc_nr <= ORIGIN_PROC_NR; proc_nr++){
        rmp = &mmproc[proc_nr];
        rmp->flags |= IN_USE;
    }

    /* 得到机器的内存总量（包含了不可用的） */
    total_clicks = bootParams.memory_size >> CLICK_SHIFT;
    /* 得到剩余可用的空闲内存，总内存减去程序可以使用的空间即可 */
    free_clicks = total_clicks - (PROCS_BASE >> CLICK_SHIFT);
    /* 初始化内存空洞表，将表初始化为所有可用的物理内存 */
    mem_init(total_clicks, free_clicks);

    /* 准备ORIGIN进程表项 */
    mmproc[ORIGIN_PROC_NR].pid = ORIGIN_PID;
    procs_in_use = LOW_USER + 1;

    /* 打印内存信息：内存总量、核心内存的使用和空闲内存情况 */
    printf("You computer's total memory size = %uKB, Available = %uKB.\n\n",
            click2round_kb(total_clicks), click2round_kb(free_clicks) );
}

/*===========================================================================*
 *				mm_print_info					     *
 *				内存管理器输出信息
 *===========================================================================*/
PUBLIC void mm_print_info(char *info){
    printf("{MM}-> %s\n", info);
}



