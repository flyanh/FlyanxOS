/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#include "fly.h"
#include "param.h"

/*===========================================================================*
 *				do_sleep					     *
 *			 处理系统调用SLEEP
 *===========================================================================*/
PUBLIC int do_sleep(void){
    /* 一个用户进程想堵塞自己，停止自己的运行，但不退出，
     * 直到再被MM唤醒自己。它可以设置一个时间(s)，告诉
     * 内存管理器什么时候去唤醒自己，如果时间为FOREVER
     * 将永久堵塞自己成为一个僵尸进程，以后可能会被系统
     * 清理掉。
     * 虽然这个调用和设置一个闹钟很像，但它们有本质区别，
     * 那就是设置闹钟不会堵塞自己，而SLEEP会。
     */
#define FOREVER -1      /* 永久堵塞 */
    time_t daze = in_daze_time;        /* 什么时候唤醒自己(ms)？ */

    /* 不是用户进程或则唤醒时间无效则不行，返回一个OK，让他们继续运行 */
    if(fly_who < ORIGIN_PROC_NR || daze < FOREVER || daze == 0) return OK;


    dont_reply = TRUE;
//    if(daze == FOREVER){
//        /* 用户进程想永久堵塞，只要别回答消息给用户就可以了 */
//        return dont_reply;
//    } else {
//        /* 我们设置一个闹钟等一会唤醒它 @TODO */
////    mmsg_in.type = SET_ALARM;
////    mmsg_in.DELTA_TICKS = daze / 10;
////    mmsg_in.FUNC_TO_CALL = (WatchDog)&block_proc_wakeup;
////    send_receive(CLOCK_TASK, &mmsg_in);
//        return dont_reply;
//    }

}

