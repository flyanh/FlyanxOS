/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 文件描述符定义及其文件表声明
 *
 * 文件表是文件描述符和索引节点之间的中介，如果FileDesc::count == 0，
 * 则说明插槽是空闲的。file表用于存放文件当前位置及其索引节点指针，除此
 * 外，它还给出了打开的文件是否可读写，以及有多少个文件描述符指向该项等。
 */

#ifndef _FS_FILE_H
#define _FS_FILE_H

#include "inode.h"

struct inode;

typedef struct file_desc {
    mode_t mode;        /* 读写位，指示文件是如何被打开的。 */
    off_t pos;          /* 文件当前读写位置 */
    int count;          /* 被几个人打开了？ */
    int flags;          /* 标志，暂时不知道可以标记什么，但总归有用的。 */
    Inode *inode;       /* 索引节点 */
} FileDesc;

EXTERN FileDesc file[NR_FILES];

#define NOT_OPEN                      0     /* 表示还没人人打开这个文件 */
#define NIL_FILE (struct file_desc *) 0	    /* 表示没有文件插槽 */

#endif //_FS_FILE_H
