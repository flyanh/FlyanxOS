/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/6.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 索引节点管理
 *
 * 有一些进程可以分配和取消分配inode，获取，清除和释放它们，并从磁盘进行读写。
 *
 * 该文件的入口点是：
 *  - get_inode:        在索引节点表中搜索给定的索引节点； 如果没有，读入它。
 *  - readwrite_inode:  读取/写入磁盘块并提取一个inode。
 */
#include "fs.h"
#include <flyanx/common.h>
#include "inode.h"
#include "dev.h"
#include "super.h"

/*===========================================================================*
 *				get_inode				     *
 *		在索引节点表中搜索给定的索引节点； 如果没有，读入它。
 *===========================================================================*/
PUBLIC Inode *get_inode(
        dev_t dev,      /* 索引节点所在的设备 */
        int numb        /* 索引节点号（ANSI标准：它可能不是unsigned short类型） */
)
{
    register Inode *ret_inode, *xp;

    if(numb == 0) {
        return NIL_INODE;
    }

    /* 在索引节点表中搜索（dev，numb）和一个空闲插槽。 */
    xp = NIL_INODE;
    for(ret_inode = inode; ret_inode < &inode[NR_INODES]; ret_inode++){
        if(ret_inode->count > 0){       /* 仅检查（dev，numb）的已用插槽  */
            if(ret_inode->device == dev && ret_inode->num == numb){
                /* 这正是我们正在寻找的 */
                ret_inode->count++;
                return ret_inode;
            }
        } else {
            xp = ret_inode;         /* 记住此空闲插槽以供以后使用 */
        }
    }
    /* 我们想要索引节点当前未使用，我们看看我们还有没有空位 */
    if(xp == NIL_INODE){        /* 满了 */
        err_code = ENFILE;
        return NIL_INODE;
    }

    /* 有空闲，加载它 */
    xp->device = dev;
    xp->num = numb;
    xp->count = 1;
    if(dev != NO_DEV) readwrite_inode(xp, READING);

    return xp;
}

/*===========================================================================*
 *				readwrite_inode				     *
 *===========================================================================*/
PUBLIC void readwrite_inode(Inode *np, int type){
    /* 读取磁盘块并提取一个inode/往磁盘中写入一个inode。
     */
    SuperBlock *sb = get_super_block(np->device);
    Inode *gp;
    int dev = np->device;
    int num = np->num;

    if(type == READING){
        int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
                ((num - 1) / (SECTOR_SIZE / INODE_SIZE));
        READ_SECT(dev, blk_nr);
        gp = (Inode *)((u8_t *)fs_buffer +
                       ((num - 1 ) % (SECTOR_SIZE / INODE_SIZE))
                       * INODE_SIZE);
        np->mode = gp->mode;
        np->size = gp->size;
        np->start_sect = gp->start_sect;
        np->nr_sects = gp->nr_sects;
    } else {
        /* 写 */
    }
}

