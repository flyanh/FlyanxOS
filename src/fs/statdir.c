/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含状态和目录有关的系统调用代码。
 *
 * 该文件的入口点是：
 *  - do_stat:	    处理STAT系统调用
 *  - do_fstat:	    处理FSTAT系统调用
 */

#include "fs.h"
#include <sys/stat.h>
#include "file.h"
#include "inode.h"
#include "param.h"

FORWARD _PROTOTYPE( int stat_inode, (Inode *indp, char *user_buf) );

/*===========================================================================*
 *				do_stat					     *
 *			处理STAT系统调用
 *===========================================================================*/
PUBLIC int do_stat(void){
    /* 得到文件的索引节点信息。 */

    Inode *indp;
    int rs;

    if(get_pathname(pathname1, pathname1_length, M1) != OK) return err_code;    /* 参数不对，导致无法获取文件路径 */
    indp = track_path(user_path);
    if(indp == NIL_INODE) return err_code;      /* 文件不存在 */
    rs = stat_inode(indp, pathname2);
    put_inode(indp);        /* 使用完后，释放索引节点 */
    return rs;
}

/*===========================================================================*
 *				do_fstat					     *
 *			处理FSTAT系统调用
 *===========================================================================*/
PUBLIC int do_fstat(void){
    /* 同STAT一样，区别在于STAT给出文件的路径名，而FSTAT给出的是打开的文件描述符。 */

    FileDesc *fp;
    /* 文件描述符有效吗？ */
    fp = get_file(in_fd);
    if(fp == NIL_FILE) return err_code;

    return stat_inode(fp->inode, in_buffer);
}

/*===========================================================================*
 *				stat_inode					     *
 *			stat和fstat的通用代码
 *===========================================================================*/
PRIVATE int stat_inode(Inode *indp, char *user_buf){
    /* 从索引节点中提取信息，将它拷贝到调用者的缓冲区。 */

    /* 填写文件状态信息 */
    Stat stat_buf;
    stat_buf.dev = indp->device;
    stat_buf.ino_num = indp->num;
    stat_buf.mode = indp->mode;
    stat_buf.rdev = inode_is_special(indp->mode) ? indp->start_sect : NO_DEV;
    stat_buf.size = indp->size;

    int rs = sys_copy(FS_PROC_NR, DATA, (phys_bytes)&stat_buf,  /* 从这 */
                      fs_who, DATA, (phys_bytes)user_buf,                /* 到这 */
                (phys_bytes)sizeof(Stat));                      /* 拷贝多少？ */

    return rs;
}
