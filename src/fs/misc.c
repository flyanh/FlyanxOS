/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含一些其他不宜分类的例程集合，其中有一些简单的系统调用，其他一些则
 * 执行大部分由内存管理器执行的系统调用。
 *
 * 该文件的入口点是：
 *  - do_fs_fork：   在MM执行FORK系统调用后调整FS的进程表
 *  - do_fs_exit：   进程已退出，同步FS的进程表
 *  - do_revive：    恢复挂起的进程（例如挂在一个终端设备上）
 */

#include "fs.h"
#include <fcntl.h>
#include <unistd.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "file.h"
#include "fsproc.h"
#include "inode.h"
#include "dev.h"
#include "param.h"

/*===========================================================================*
 *				do_fs_fork				     *
 *			   文件系统部分的fork
 *===========================================================================*/
PUBLIC int do_fs_fork(void){

    register FSProcess *child;
    int i;

    /* 只有MM才能使用此调用。 */
    if(who != MM_PROC_NR) return EGENERIC;

    /* 子进程继承父进程的结构 */
    fsproc[in_child] = fsproc[in_parent];

    /* 增加打开文件的计数器 */
    child = &fsproc[in_child];
    for(i = 0; i < OPEN_MAX; i++){
        if(child->open_file[i] != NIL_FILE){
            child->open_file[i]->count++;
            child->open_file[i]->inode->count++;
        }
    }

    /* 填写子进程的id */
    child->pid = in_pid;

    /* 子进程绝对不是首领 */
    child->ses_leader = FALSE;

    return OK;
}

/*===========================================================================*
 *				do_fs_exit				     *
 *			   文件系统部分的exit
 *===========================================================================*/
PUBLIC int do_fs_exit(void){
    /* 执行exit(status)系统调用的文件系统部分。 */

    int i;
    register FSProcess *exit;

    /* 只有MM才能使用此调用。 */
    if(who != MM_PROC_NR) return EGENERIC;

    exit = &fsproc[in_pid];
    for(i = 0; i < OPEN_MAX; i++){
        if(exit->open_file[i] != NIL_FILE){
            /* 调用close关闭所有退出进程已经打开的文件，假装调用来自用户进程。 */
            in_fd = i;
            (void) do_close();
        }
    }
    return OK;
}

/*===========================================================================*
 *				do_revive				     *
 *			   恢复挂起的进程
 *===========================================================================*/
PUBLIC int do_revive(void){
    /* 一个任务原先无法完成文件系统请求的工作，但是现在完成了该工作(例如为某用户进程提供输入数据)，
     * 此时调用本函数。文件系统唤醒用户进程并向它发送响应消息。
     * 现在，一项任务（通常是TTY）已获得上一次阅读所需的字符。进行调用时，该进程未得到答复。 相反，
     * 它被挂起了。现在，我们可以发送答复以将其唤醒。 该业务必须谨慎进行，因为传入的消息来自某个任
     * 务（无法将答复发送到该任务），并且答复必须进入较早被阻止的过程。 通过设置“need_reply”标志
     * 来禁止对调用者的答复，并且对被阻止进程的答复是在revive()中显式完成的。
     * 注意：这个函数并不是一个真正的系统调用，但我们将它按系统调用加以处理。
     */

    /* 用户进程不能做这件事！ */
    if(who >= LOW_USER && call_fp->pid != PID_SERVER) return EPERM;

    revive(fs_inbox.REPLY_PROC_NR, fs_inbox.REPLY_STATUS);
    need_reply = FALSE;     /* 不需要回复终端 */
    return OK;
}




