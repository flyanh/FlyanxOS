/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#include "fly.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "param.h"


FORWARD _PROTOTYPE( void get_work, (void) );

/*===========================================================================*
 *				fly_main					     *
 *===========================================================================*/
PUBLIC void fly_main(void){
    int rs;

    fly_print_info("Working...\n");
    while (TRUE){
        get_work();

        /* 如果调用号有效，则执行调用 */
        if((unsigned) fly_call >= NR_CALLS){
            rs = ENOSYS;
        } else {
            rs = (fly_call_handlers[fly_call])();
        }

        if(dont_reply) continue;
        fly_reply(fly_who, rs);
    }
}


/*===========================================================================*
 *				get_work				     *
 *			   等待外界工作
 *===========================================================================*/
PRIVATE void get_work(void){
    int rec_state;

    /* 等待一条消息 */
    rec_state = receive(ANY, &flmsg_in);
    /* 接收消息错误 */
    if(rec_state != OK) fly_panic("MM receive msg error", NO_NUM);
    /* 得到调用者和功能调用号 */
    fly_who = flmsg_in.source;
    fly_call = flmsg_in.type;
    dont_reply = FALSE;     /* 默认是需要回复的。 */
//    /* 得到调用的进程实例，如果调用者插槽号<0，则可能是系统任务在调用，设置为MM的插槽号 */
//    curr_proc = &mmproc[who < 0 ? MM_PROC_NR : who];
}

/*===========================================================================*
 *				fly_reply					     *
 *			    回复结果
 *===========================================================================*/
PUBLIC void fly_reply(int whom, int rs){
    /* 向用户进程发送回复。 它可能会失败（如果该进程刚刚被信号杀死），因此不需要检查返回码。
     * 如果发送失败，则忽略它。
     */
    reply_type = rs;
    send(whom, &flmsg_out);
}

/*===========================================================================*
 *				fly_print_info					     *
 *				飞彦拓展管理器输出信息
 *===========================================================================*/
PUBLIC void fly_print_info(char *info){
    printf("{FLY}-> %s", info);
}
