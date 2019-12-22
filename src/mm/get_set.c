/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件处理获取和设置用户进程号（uid）和组号（gid），以及各种获取、设置
 * 调用。因为它们完成的事情太简单了，所以我们就分为了两个函数处理就足够了！
 *
 * 它处理以下的系统调用：
 *  - GETPID        获取自己的进程号
 *  - GETPPID       获取父进程的进程号
 */


#include "mm.h"
#include <flyanx/callnr.h>
#include <signal.h>
#include "mmproc.h"
#include "param.h"

/*===========================================================================*
 *				do_get				     *
 *			处理进程信息获取相关的调用
 *===========================================================================*/
PUBLIC int do_get(){
    register MMProcess *proc = curr_mp;
    register int rs;

    switch (mm_call){
        /* 获取自己的进程号以及父进程的进程号 */
        case GETPID:
            rs = proc->pid;
            break;
        case GETPPID:
            rs = mmproc[proc->parent].pid;
            break;
        /* 调用号错误 */
        default:
            rs = EINVAL;
            break;
    }

    return rs;
}

/*===========================================================================*
 *				do_set				     *
 *			处理进程信息设置相关的调用
 *===========================================================================*/
PUBLIC int do_set(){
//    register MMProcess *proc = curr_mp;
    register int rs;

    switch (mm_call){
        default:
            rs = EINVAL;
            break;
    }

    return rs;
}


