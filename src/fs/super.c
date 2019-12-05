/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/6.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 超级块的管理，该文件管理超级块表和相关的数据结构，即位图，该位图跟踪哪些区域
 * 和哪些索引节点已分配以及哪些空闲。当需要新的索引节点或区域时，将在适当的位图
 * 中搜索一个空闲条目。
 *
 * 该文件的入口点是：
 *  - read_super_block:     读取一个
 *  - get_super_block:      获得一个超级块
 */

#include "fs.h"
#include "super.h"

/*===========================================================================*
 *                  read_super
 *					读取一个超级块			     *
 *===========================================================================*/
PUBLIC int read_super_block(SuperBlock *sp){

    return OK;
}

/*===========================================================================*
 *                  get_super_block
 *					获取一个超级块			     *
 *===========================================================================*/
PUBLIC SuperBlock *get_super_block(dev_t dev){
    register SuperBlock *sp;
    for(sp = super_block; sp < &super_block[NR_SUPER_BLOCK]; sp++){
        if(sp->super_dev == dev){
            return sp;
        }
    }
    /* 能运行到这里，说明没找到... */
    fs_panic("super block of device not found.\n", dev);

    return NIL_SUPER_BLOCK;     /* 让编译器闭嘴，别再漂黄了 */
}


