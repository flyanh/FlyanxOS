/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 包含管道文件的操作
 *
 * 该文件处理进程的暂停和恢复。进程可以挂起，因为它想从管道中读取或写入而不能，或者因为它想从特殊
 * 文件中读取或写入但暂时不能完成，这个时候一个进程无法继续，它将被挂起，并在以后能够继续时恢复。
 *
 * 注：flyanx1.0文件系统不实现管道，但是进程的挂起和恢复很适合放在这个文件中，也便于未来的扩展。
 *
 * 该文件的入口点是：
 *  - suspend：      暂停无法执行请求的读写的进程
 *  - revive：       恢复一个挂起进程
 */

#include "fs.h"
#include "file.h"
#include "fsproc.h"
#include "inode.h"
#include "param.h"


/*===========================================================================*
 *				suspend					     *
 *			   挂起一个用户进程
 *===========================================================================*/
PUBLIC void suspend(int task){
    /* 暂停无法执行读写请求的进程
     * 我们采取措施中止当前系统调用的处理，将恢复使用的参数存储在进程表中。
     */

    call_fp->suspended = SUSPENDED;
    call_fp->fd = in_fd << 8 | fs_call;
    call_fp->task = -task;
    call_fp->buffer = in_buffer;        /* 为了唤醒的读写 */
    call_fp->bytes = in_bytes;
    /* 禁止文件系统响应消息，这是挂起的关键，当进程收不到答复，它自然就被堵塞了，
     * 可以说这行使得调用进程被挂起了。
     */
    need_reply = FALSE;
}

/*===========================================================================*
 *				release					     *
 *          释放所有可以恢复的进程
 *===========================================================================*/
PUBLIC void release(){

}

/*===========================================================================*
 *				revive					     *
 *          恢复一个挂起进程
 *===========================================================================*/
PUBLIC void revive(int proc_nr, int bytes){
    /* 将已暂停的进程标记为能够再次运行，我们恢复先前堵塞的进程。
     * 当进程挂在终端上时，这是最终恢复它的方式。
     */

    register FSProcess *fp;
    register int task;

    if(proc_nr < 0 || proc_nr >= NR_PROCS) fs_panic("revive error", proc_nr);
    fp = &fsproc[proc_nr];
    if(fp->suspended == NOT_SUSPENDED) return;  /* 该进程已经被恢复了 */

    /* 恢复挂在任务上的进程 */
    task = -fp->task;
    fp->suspended = NOT_SUSPENDED;
    fp->bytes = bytes;
    fs_reply(proc_nr, bytes);

}
