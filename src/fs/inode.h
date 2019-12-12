/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 索引节点定义及其索引节点表的声明
 *
 *
 */

#ifndef _FS_INODE_H
#define _FS_INODE_H

#include "fs.h"

/* 索引节点结构
 *
 */
typedef struct inode {
    u32_t mode;         /* 存取模式：文件类型，保护属性，等... */
    u32_t size;         /* 文件大小 */
    u32_t start_sect;   /* 数据的第一个扇区 */
    u32_t nr_sects;     /* 文件占用多少扇区 */
    u8_t _unused[16];   /* 为了对齐32位 */

    /* =============== 以下项目将仅存在于内存中 ================== */
    dev_t device;       /* 这是谁（设备）的索引节点？ */
    int count;          /* 多少个进程打开了此索引节点 */
    int num;            /* 索引节点号 */
} Inode;

EXTERN Inode inode[NR_INODES];

#define	inode_is_special(m)	((((m) & I_TYPE) == I_BLOCK_SPECIAL) ||	\
			 (((m) & I_TYPE) == I_CHAR_SPECIAL))

/* 其他 */
#define INODE_SIZE      (4 * 4 + 16)    /* 这是设备中结构的大小，不是内存中的 */
#define NIL_INODE (struct inode *) 0	/* 指示没有inode插槽 */
#define	ROOT_INODE		    1           /* 根目录的索引节点号 */


#endif //_FS_INODE_H
