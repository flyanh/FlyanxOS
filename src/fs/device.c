/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/5.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 设备管理
 * 当所需的块不在高速缓存中时，必须从磁盘中获取它。特殊字符文件也需要I/O，这些文件系统
 * 与这些任务的接口包含在这里。
 *
 * 该文件的入口点是：
 *  dev_open:       FS打开设备
 *  dev_close:      FS关闭设备
 *  dev_io:	        FS在设备上进行读取或写入
 *  do_ioctl:	    执行IOCTL系统调用
 */


#include "fs.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include <fcntl.h>
#include "dev.h"
#include "param.h"


/*===========================================================================*
 *				do_ioctl					     *
 *				处理设备io控制
 *===========================================================================*/
PUBLIC int do_ioctl(){
    return dev_ioctl(fs_inbox.DEVICE, who, in_buffer);
}

/*===========================================================================*
 *				dev_ioctl					     *
 *              设备io控制
 *===========================================================================*/
PUBLIC int dev_ioctl(int device, int proc_nr, void *buf){
    /* 单独提供本函数是为了文件系统内部的调用 */
    int status;
    fs_outbox.DEVICE = MINOR(device);
    fs_outbox.PROC_NR = proc_nr;
    fs_outbox.REQUEST = DIOCTL_GET_GEO;
    fs_outbox.ADDRESS = buf;
    int task_nr = ddmap[MAJOR(device)].driver_task_nr;
    if(task_nr == NO_EXIST_TASK){
        fs_panic("ioctl: driver no exist", NO_NUM);
    }
    if( (status = task_call(task_nr, DEVICE_CLOSE, &fs_outbox)) == TASK_REPLY){
        return fs_outbox.REPLY_STATUS;
    } else {
        /* 竟然调用（发送并接收消息除了问题）失败了？ */
        fs_panic("ioctl: task call failed.", status);
    }
}

/*===========================================================================*
 *				dev_open					     *
 *				打开设备
 *===========================================================================*/
PUBLIC int dev_open(
        dev_t device,   /* 设备号 */
        int proc,       /* 打开设备的进程 */
        int flags       /* 以什么方式打开？ */
){
    int status;
    fs_outbox.DEVICE = MINOR(device);
    fs_outbox.PROC_NR = proc;
    fs_outbox.FLAGS = flags;
    int task_nr = ddmap[MAJOR(device)].driver_task_nr;
    if(task_nr == NO_EXIST_TASK){
        fs_panic("open: driver no exist", NO_NUM);
    }
    if( (status = task_call(task_nr, DEVICE_OPEN, &fs_outbox)) == TASK_REPLY){
        return fs_outbox.REPLY_STATUS;
    } else {
        /* 竟然调用（发送并接收消息除了问题）失败了？ */
        fs_panic("open: task call failed.", status);
    }
}

/*===========================================================================*
 *				dev_close					     *
 *				关闭设备
 *===========================================================================*/
PUBLIC int dev_close(dev_t device, int proc){
    int status;
    fs_outbox.DEVICE = MINOR(device);
    fs_outbox.PROC_NR = proc;
    int task_nr = ddmap[MAJOR(device)].driver_task_nr;
    if(task_nr == NO_EXIST_TASK){
        fs_panic("close: driver no exist", NO_NUM);
    }
    task_call(task_nr, DEVICE_CLOSE, &fs_outbox);
    if( (status = task_call(task_nr, DEVICE_CLOSE, &fs_outbox)) == TASK_REPLY){
        return fs_outbox.REPLY_STATUS;
    } else {
        /* 竟然调用（发送并接收消息除了问题）失败了？ */
        fs_panic("close: task call failed.", status);
    }
}

/*===========================================================================*
 *				dev_io					     *
 *				设备读写
 *===========================================================================*/
PUBLIC int dev_io(
        int op,             /* 读还是写？ */
        dev_t device,       /* 设备号 */
        int proc_nr,        /* 谁想读写？ */
        void *buf_addr,     /* 读写到的缓冲区虚拟地址 */
        off_t pos,          /* 读写字节偏移量。 */
        int bytes,          /* 读写多少？（字节） */
        int flags           /* 特殊设备标志，例如：O_NONBLOCK */
){
    fs_outbox.DEVICE = MINOR(device);
    fs_outbox.PROC_NR = proc_nr;
    fs_outbox.ADDRESS = buf_addr;
    fs_outbox.POSITION = pos;
    fs_outbox.COUNT = bytes;
    fs_outbox.TTY_FLAGS = flags;
    int task_nr = ddmap[MAJOR(device)].driver_task_nr;
    if(task_nr == NO_EXIST_TASK){
        fs_panic("io: driver no exist", NO_NUM);
    }
    task_call(task_nr, op, &fs_outbox);
    /* 任务已经完成，检查结果。 */
    if(fs_outbox.REPLY_STATUS == SUSPEND){
        if(flags & O_NONBLOCK){
            /* 用户不想被堵塞 */
            fs_outbox.PROC_NR = proc_nr;
            fs_outbox.DEVICE = MINOR(device);
            task_call(task_nr, CANCEL, &fs_outbox);
            if(fs_outbox.REPLY_STATUS == EINTR) fs_outbox.REPLY_STATUS = EAGAIN;
        } else {
            /* 默认情况，我们挂起用户 */
            suspend(task_nr);
        }
    }
    return fs_outbox.REPLY_STATUS;
}


