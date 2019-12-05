/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 超级块定义，并声明出超级块表
 *
 * 根文件系统和每个已挂载的文件系统在此处都有一个条目。 该条目保存有关位图和索引节点大小的信息。
 *
 * 系统启动时，根设备的超级块被载入。而安装一个文件系统时，其超级块同样被载入。同其他表一样，
 * 超级块表也用EXTERN加以声明。
 */

#ifndef _FS_SUPER_H
#define _FS_SUPER_H

typedef struct super_block {
    u32_t magic;            /* 魔数，用于识别超级块 */
    u32_t nr_inodes;        /* 多少个索引节点 */
    u32_t nr_sects;         /* 多少个扇区 */
    u32_t nr_imap_sects;    /* 多少个<索引节点位图>扇区 */
    u32_t nr_smap_sects;    /* 多少个<扇区位图>扇区 */
    u32_t first_sect_nr;    /* 第一个数据扇区的编号 */
    u32_t nr_inode_sects;   /* 多少个<索引节点>扇区 */
    u32_t root_inode;       /* 根目录的索引节点 */
    u32_t inode_size;       /* 索引节点的长度 */
    u32_t inode_size_off;   /* 结构'Inode::size'的偏移量 */
    u32_t inode_start_off;  /* 结构'Inode::start_sect'的偏移量 */
    u32_t dir_ent_size;         /* 结构'DirectoryEntry'的大小 */
    u32_t dir_ent_inode_off;    /* 结构'DirectoryEntry::inode_nr'的偏移 */
    u32_t dir_ent_name_off;     /* 结构'DirectoryEntry::name'的偏移 */

    /* =============== 以下项目将仅存在于内存中 ================== */
    int super_dev;              /* 这是谁（设备）的超级块？ */
} SuperBlock;

EXTERN SuperBlock super_block[NR_SUPER_BLOCK];

/* 宏 */
#define SUPER_BLOCK_SIZE    (4 * 14)/* 这是设备中结构的大小，不是内存中的 */
#define NIL_SUPER_BLOCK     (struct super_block *) 0
#define INODE_MAP		    0	    /* 在索引节点位图上运行 */
#define SECT_MAP		    1	    /* 在扇区位图上操作 */

#endif //_FS_SUPER_H
