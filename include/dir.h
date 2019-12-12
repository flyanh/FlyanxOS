/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件定义了Flyanx的目录项结构
 * 这个文件只被直接引用了几次，但包含它的文件在整个文件系统
 * 中广为使用。它之所以重要是因为它定义了一个文件名最多有多
 * 少个字符，因为文件名的使用贯穿整个文件系统。
 */

#ifndef FLYANX_DIR_H
#define FLYANX_DIR_H

#include "../src/fs/fs.h"

#define DIR_BLOCK_SIZE  512 /* 目录块的大小 */

#ifndef NAME_LEN
#define NAME_LEN	18    /* 目录/文件名长度 */
#endif

typedef struct directory_entry {
    ino_t inode_nr;         /* 索引节点号，它告诉我们应该从哪里找到这个文件 */
    char name[NAME_LEN];    /* 文件名 */
} DirectoryEntry;

#endif //FLYANX_DIR_H
