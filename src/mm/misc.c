/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/3.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 其他一些不适合分类的系统调用
 *
 * 该文件的入口点是：
 *  - do_block
 */

#include "mm.h"
#include <signal.h>
#include "mmproc.h"
#include "param.h"

FORWARD _PROTOTYPE( void block_proc_wakeup, (void) );

/*===========================================================================*
 *				do_block					     *
 *			 处理系统调用BLOCK
 *===========================================================================*/
PUBLIC int do_block(void){
    /* 一个用户进程想堵塞自己，停止自己的运行，但不退出，
     * 直到再被MM唤醒自己。它可以设置一个时间(s)，告诉
     * 内存管理器什么时候去唤醒自己，如果时间为FOREVER
     * 将永久堵塞自己成为一个僵尸进程，以后可能会被系统
     * 清理掉。
     * 虽然这个调用和设置一个闹钟很像，但它们有本质区别，
     * 那就是设置闹钟不会堵塞自己，而BLOCK会。
     */
#define FOREVER -1      /* 永久堵塞 */

    int proc_nr = mmsg_in.source;       /* 谁想堵塞自己？ */
    time_t daze = mmsg_in.m6_l1;        /* 什么时候唤醒自己(ms)？ */

    /* 不是用户进程或则唤醒时间无效则不行，返回一个OK，让他们继续运行 */
    if(proc_nr < ORIGIN_PROC_NR || daze < FOREVER || daze == 0) return OK;

    if(daze == FOREVER){
        /* 用户进程想永久堵塞，只要别回答消息给用户就可以了 */
        return ERROR_NO_MESSAGE;
    } else {
        /* 我们设置一个闹钟等一会唤醒它 @TODO */
//    mmsg_in.type = SET_ALARM;
//    mmsg_in.DELTA_TICKS = daze / 10;
//    mmsg_in.FUNC_TO_CALL = (WatchDog)&block_proc_wakeup;
//    send_receive(CLOCK_TASK, &mmsg_in);
        return ERROR_NO_MESSAGE;
    }

}

PRIVATE void block_proc_wakeup(){

}



