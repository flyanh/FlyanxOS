/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含操纵文件描述符的例程。
 *
 * 该文件的入口点是：
 *  get_fd:	        寻找空闲的文件描述符和空闲的文件插槽
 *  get_file:	    查找给定文件描述符的文件条目
 *  find_file:	    找到指向给定inode的文件插槽
 */

#include "fs.h"
#include "file.h"
#include "fsproc.h"
#include "inode.h"

/*===========================================================================*
 *				get_fd					     *
 *		寻找空闲的文件描述符和空闲的文件插槽
 *===========================================================================*/
PUBLIC int get_fd(
        int start,              /* 从插槽处哪里开始搜索？这个参数可以减少寻找的时间 */
        mode_t mode,            /* 要创建的文件的模式（RWX位） */
        int *fd,                /* 返回文件描述符的地方 */
        FileDesc **fd_slot      /* 返回文件插槽的地方 */
){
    /* 创建或打开文件时，需要分配一个空闲的文件描述符和一个空闲的filp项，
     * 这时可调用本例程。但是分配到的文件描述符和文件项并不标志为使用，因
     * 为在确信CREATE和OPEN成功之前，需要进行许多检查。
     */

    register FileDesc *f;
    register int i = -1;

    *fd = -1;       /* 默认-1，不存在的文件描述符号，如果作为找没找到的判断依据。 */

    if(start < 0) start = 0;    /* 禁止越界 */

    /* 在调用者的打开文件中查找空闲的文件描述符插槽 */
    for(i = start; i < OPEN_MAX; i++){
        if(call_fp->open_file[i] == NIL_FILE){
            /* 已经找到 */
            *fd = i;
            break;
        }
    }
    if(*fd < 0) return EMFILE;  /* 是否找到文件描述符了？ */

    /* 现在已经找到文件描述符，寻找一个空闲的插槽为它 */
    for(f = &file[0]; f < &file[NR_FILES]; f++){
        if(f->count == NOT_OPEN){
            f->mode = mode;
            f->pos = 0L;
            f->flags = 0;
            *fd_slot = f;
            return OK;
        }
    }

    /* 如果到了这里，说明系统的文件表已经满了，返回一个打开文件过多错误 */
    return ENFILE;
}

/*===========================================================================*
 *				get_file				     *
 *				获取文件
 *===========================================================================*/
PUBLIC FileDesc *get_file(
        int fd      /* 文件描述符 */
){
    /* 查找给定文件描述符的文件条目 */

    err_code = EBADF;
    if(fd < 0 || fd >= OPEN_MAX) return NIL_FILE;
    return call_fp->open_file[fd];
}





