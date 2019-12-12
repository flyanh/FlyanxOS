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
 *  - alloc_imap_bit:       有人想要分配一个索引节点
 *  - alloc_smap_bit:       有人想要分配一个扇区位图
 */

#include "fs.h"
#include <flyanx/common.h>
#include "super.h"
#include "dev.h"

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

/*===========================================================================*
 *                  alloc_imap_bit
 *				为一个新的索引节点分配位图			     *
 *===========================================================================*/
PUBLIC ino_t alloc_imap_bit(dev_t dev){
    /* 返回的是新分配的索引节点号 */

    int i, j, k;    /* i：扇区索引
                     * j：字节索引
                     * k：位索引
                     */
    ino_t inode_nr = 0;

    SuperBlock *sb = get_super_block(dev);
    int imap_blk0_nr = 1 + 1;   /* 1个引导扇区和1个超级块 */

    /* 开始在超级块中查找可用的位 */
    for(i = 0; i < sb->nr_imap_sects; i++){
        READ_SECT(dev, imap_blk0_nr + i);

        for(j = 0; j < SECTOR_SIZE; j++){
            /* 跳过'11111111'这样的字节，即跳过已经被分配字节  */
            if(fs_buffer[j] & 0xFF) {
                continue;
            }
            /* 跳过所有已经分配的位 */
            for(k = 0; ((fs_buffer[j] >> k) & 1) != 0; k++) {}

            /* 现在，k指向的就是没有被使用的索引节点位了，可以进行分配了 */
            inode_nr = (i * SECTOR_SIZE + j) * 8 + k;
            fs_buffer[j] |= (1 << k);
            /* 将分配好的位写入到超级块中 */
            WRITE_SECT(dev, imap_blk0_nr + i);
            return inode_nr;
        }
    }
    /* 位图已经满了，不能分配 */
    return NO_BIT;
}

/*===========================================================================*
 *                  alloc_imap_bit
 *				为一段已经使用的扇区分配位图			     *
 *===========================================================================*/
PUBLIC int alloc_smap_bit(
        dev_t dev,      /* 哪个设备？ */
        int nr_sects    /* 要分配几个扇区？ */
){
    /* nr_sects这里如果需要分配的话我们固定为NR_DEFAULT_FILE_SECTS，
     * 可能有点浪费空间，但是这是实现的最简单的方法，以后通过一些改动可以
     * 使它不那么死板。
     *
     * 返回值：分配的第一个扇区
     */

    int i, j, k;    /* i：扇区索引
                     * j：字节索引
                     * k：位索引
                     */
    if(nr_sects <= 0) return NO_BIT;        /* 没打算分配？ */
    nr_sects = NR_DEFAULT_FILE_SECTS;       /* 管它呢，就这么大！ */

    SuperBlock *sb = get_super_block(dev);
    int smap_blk0_nr = 1 + 1 + sb->nr_imap_sects;   /* 1个引导扇区和1个超级块以及索引节点位图 */
    int free_sect_nr = 0;

    /* 开始在超级块中查找可用的位 */
    for(i = 0; i < sb->nr_imap_sects; i++){
        READ_SECT(dev, smap_blk0_nr + i);

        for(j = 0; j < SECTOR_SIZE; j++){
            /* 跳过'11111111'这样的字节，即跳过已经被分配字节  */
            if(fs_buffer[j] & 0xFF) {
                continue;
            }
            /* 跳过所有已经分配的位 */
            for(k = 0; ((fs_buffer[j] >> k) & 1) != 0; k++) {}
            free_sect_nr = (i * SECTOR_SIZE + j) * 8 +
                    k - 1 + sb->first_sect_nr;

            /* 重复置位，一直到分配的扇区数量为0 */
            for(; k < 8; k++){
                fs_buffer[j] |= (1 << k);
                nr_sects--;
                if(nr_sects == 0) break;
            }
        }
        /* 将分配好的位写入到超级块中 */
        if(free_sect_nr){
            WRITE_SECT(dev, smap_blk0_nr + i);
        }
        if(nr_sects == 0) break;
    }

    /* 分配失败，空间可能已经满了 */
    if(nr_sects >= 0){
        return NO_BIT;
    }

    /* 返回分配的第一个扇区号 */
    return free_sect_nr;
}


