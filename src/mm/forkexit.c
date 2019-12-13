/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 此文件包含处理FORK、EXIT、和WAIT系统调用的实现
 * 本文件的入口点是：
 *  - do_fork：      处理FORK系统调用
 *  - do_exit：      处理EXIT系统调用
 *  - do_wait：      处理WAIT系统调用
 */

#include "mm.h"
#include <sys/wait.h>
#include <flyanx/callnr.h>
#include <signal.h>
#include "mmproc.h"
#include "param.h"

#define LAST_FEW        3   /* 为超级用户保留的最后几个进程插槽 */

PRIVATE pid_t next_pid = ORIGIN_PID + 1;    /* 下一个要分配的pid */

/*===========================================================================*
 *				do_fork					     *
 *			处理FORK系统调用
 *===========================================================================*/
PUBLIC int do_fork(void){
    /* 这个调用很重要，也很复杂。当一个进程想要创建一个新进程的时候，
     * 就需要使用FORK调用生成一个新的进程分支，这个新进程是调用进程
     * 的子进程，可以共享父进程的一些资源，例如：打开的文件。
     * 该文件要完成的事情很多，因为一个新进程的产生需要为它在进程表中
     * 找到一个插槽，同时还需要设置它新的正文段等等...
     */

    register MMProcess *parent = curr_mp;   /* 指向父进程，即调用者 */
    register MMProcess *child;              /* 指向fork出来的子进程 */
    int i, child_nr, t;

    /* 如果在FORK的时候，进程表已经满了，那么我们就没必要开始了，
     * 避免一些不必要的麻烦，因为如果失败了，恢复过程麻烦的你不敢
     * 想象，而且非常容易出错。
     */
    if(procs_in_use == NR_PROCS) return EAGAIN;
    /* 为超级用户保留的最后几个进程表项，也不能使用，对于普通用户。 */
    if(parent->effective_uid != SUPER_USER &&
            procs_in_use >= NR_PROCS - LAST_FEW) return EAGAIN;

    /* ========== 好了，通过了上面的检查，可以开始创建子进程的操作了 ========== */
    /* 子进程需要继承父进程的：正文段、数据段和堆栈段。
     * 第一步首先我们要知道父进程段的基地址和大小，由于这一步的信息只有内核才知道，
     * 我们通知内核，让内核去完成这件事情，完成后内核将会返回父进程的这些信息。MM
     * 才能继续下一步，如果失败了，那么这次的fork操作就不能继续进行了。
     */



    /* 首先，我们必须找到一个空的进程插槽给子进程 */
    for(child = &mmproc[0]; child < &mmproc[NR_PROCS]; child++){
        if(!(child->flags & IN_USE)){
            /* 找到了，停止寻找 */
            break;
        }
    }

    /* 子进程继承父进程的结构（MM中的） */
    *child = *parent;
    child->parent = who;    /* 不要忘了父亲是谁 */


}

/*===========================================================================*
 *				do_exit				     *
 *			 处理系统调用EXIT
 *===========================================================================*/
PUBLIC int do_exit(void){
    /* 这个例程接收EXIT调用，但全部工作都是mm_exit()做的。这样划分是因为mm_exit()也
     * 被用来处理被信号终止运行的进程。两者工作相同，但参数不同，所以这样划分是很方便的。
     */

    mm_exit(curr_mp, m_status);
    return (ERROR_NO_MESSAGE);      /* 死人不能再讲话 */
}

/*===========================================================================*
 *				mm_exit					     *
 *===========================================================================*/
PUBLIC void mm_exit(MMProcess *rmp, int exit_status){

}

/*===========================================================================*
 *				do_wait				     *
 *===========================================================================*/
PUBLIC int do_wait(void){

}
