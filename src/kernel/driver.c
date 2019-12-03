/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/3.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件存放机器所有块设备驱动程序的公共例程，这里面的例程
 * 可能会被许多不同的块设备驱动程序使用。
 */

#include "kernel.h"
#include <sys/ioctl.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "process.h"

PRIVATE Message msg;

/*===========================================================================*
 *				nop_task				     *
 *	未使用的控制器由该任务“服务”，即是一个设备的哑例程
 *===========================================================================*/
PUBLIC void nop_task(void)
{
    int rs, caller, proc_nr;

    while (TRUE){
        receive(ANY, &msg);

        /* 得到请求者以及需要服务的进程编号 */
        caller = msg.source;
        proc_nr = msg.PROC_NR;

        /* 检查请求者是否合法：只能是文件系统或者其他的系统任务 */
        if(caller != FS_PROC_NR && caller >= 0){
            printf("nop_task: got message form %d\n",  caller);
            continue;   /* 重新等待工作 */
        }

        switch(msg.type){
            case DEVICE_OPEN:
            case DEVICE_CLOSE:
            case DEVICE_IOCTL:
                rs = ENODEV;   break;
            default:
                rs = EIO;
        }

        /* 答复 */
        msg.type = TASK_REPLY;
        msg.REPLY_PROC_NR = proc_nr;
        msg.REPLY_STATUS = rs;          /* 传输的字节数或错误代码 */
        send(caller, &msg);
    }
}

/*===========================================================================*
 *				alarm_clock				     *
 *			   需要一个闹钟服务
 *===========================================================================*/
PUBLIC void alarm_clock(
        time_t ticks,       /* 多少滴答后闹钟响起 */
        WatchDog func       /* 响起后做什么事情？ */
){
    /* 有些磁盘设备需要延时，例如等待一个软驱的步进电机加速，本例程提供支持。
     * 该例程通过向时钟任务发送一个消息获得一个闹钟服务。
     */
    Message cmsg;

    cmsg.type = SET_ALARM;
    cmsg.CLOCK_PROC_NR = curr_proc->nr;
    cmsg.DELTA_TICKS = ticks;
    cmsg.FUNC_TO_CALL = (sighandler_t)func;
    send_receive(CLOCK_TASK, &cmsg);
}





