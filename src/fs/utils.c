/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含了文件系统的通用的管理例程。
 *
 * 该文件的入口点是：
 *  - clock_time:       询问时钟任务得到实时时间
 *  - copy:	            拷贝一块数据
 *  - get_pathname:     从用户空间获取路径名
 *  - no_sys:           拒绝FS无法处理的系统调用
 *  - fs_panic:         FS发生了可怕的事情导致flyanx无法继续
 */

#include "fs.h"
#include <unistd.h>
#include <flyanx/common.h>
#include "param.h"

PRIVATE Message clock_msg;

/*===========================================================================*
 *				clock_time				     *
 *			    得到实时时间
 *===========================================================================*/
PUBLIC time_t clock_time(void){
    int rs;

    clock_msg.type = GET_TIME;
    rs = send_receive(CLOCK_TASK, &clock_msg);
    if(rs != OK){
        fs_panic("clock_time error", rs);
    }
    return (time_t)clock_msg.CLOCK_TIME;
}

/*===========================================================================*
 *				get_pathname				     *
 *			从用户空间获取路径名
 *===========================================================================*/
PUBLIC int get_pathname(
        char *path,     /* 指向用户空间路径的指针 */
        int len,        /* 路径长度，包括0个字节 */
        int flag        /* M3表示路径（可能）在消息中 */
){
    /* 许多系统调用都以文件名作为参数，所以我们定义本例程。如果文件名较短，它被包含在用户
     * 发送给文件系统的消息之中。若文件名很长，就把指向用户空间中名字的一个指针放在消息中。
     * 我们检查这两种情况，获得文件名。
     */

    register char *upp, *mpp;
    int rs;

    /* 检查名称长度 */
    if(len <= 0){
        err_code = EINVAL;
        return EGENERIC;
    }
    if(len >= PATH_MAX){
        err_code = ENAMETOOLONG;
        return EGENERIC;
    }

    if(flag == M3 && len < M3_STRING){
        /* 好的，路径名很短，他被包含在了消息中，我们只需要复制它到"user_path"全局变量即可。 */
        upp = &user_path[0];
        mpp = pathname_in_msg;     /* 在输入的消息中 */
        do{
            *upp = *mpp;
            upp++;
            mpp++;
            len--;
        } while (len);
        rs = OK;
    } else {
        /* 路径名称太长了，不适合放在消息中，它现在在用户的缓冲区中，我们复制过来 */
        rs = sys_copy(who, DATA, (phys_bytes) path,
                FS_PROC_NR, DATA, (phys_bytes) user_path, (phys_bytes) len);
    }
    return rs;
}

/*===========================================================================*
 *				no_sys					     *
 *			  无效调用处理
 *===========================================================================*/
PUBLIC int fs_no_sys()
{
    /* 请求了FS未实现的系统调用号。
     * 本过程应该永远不被调用，提供它只是为了处理用户用非法的或不是由内存管理器处理的
     * 系统调用号调用内存管理器的情况。
     */
    return EINVAL;
}

/*===========================================================================*
 *                         fs_panic                                   *
 *                       系统无法继续运行                               *
 *===========================================================================*/
extern bool assert_panic;
PUBLIC void fs_panic(msg, err_no)
        const char *msg;
        int err_no;
{
    /* 只有在内存管理器检测到一个它无法恢复的严重错误时才会被调用。它向系统任务报告错误，系统任务
     * 紧急停止系统。它不该被轻易调用。例如当检测到内部不一致（例如，编程错误或定义的常数的非法值）时
     * ，会引起恐慌然后宕机。
     */

    /* OK，蓝屏吧（致敬XP）*/
    sys_blues();

    if(msg != NULL){
        printf("File system panic: %s", msg);
        if(err_no != NO_NUM) printf(" %d", err_no);
        printf("\n");
    }
    sys_sudden(RBT_PANIC);  /* 突然死机 */
}

