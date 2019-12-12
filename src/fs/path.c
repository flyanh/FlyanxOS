/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 路径名和目录的处理
 *
 * 该文件的入口点是：
 *  - track_path:       跟踪一个全路径，得到其索引节点并返回
 *  - last_file:	    得到给定路径上最终文件的目录项，返回其索引节点
 *  - step_dir:         解析路径名的一个目录项
 *  - search_dir:       在目录中搜索字符串(文件名字)并返回其索引节点号
 */

#include "fs.h"
#include <string.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "file.h"
#include "fsproc.h"
#include "inode.h"
#include "dev.h"
#include "super.h"
#include <dir.h>

PUBLIC char dot[2] = ".";
PUBLIC char dotdot[3] = "..";   /* . 和 .. */


/*===========================================================================*
 *				track_path				     *
 *===========================================================================*/
PUBLIC Inode *track_path(char *path){
    /* 过程对路径名进行分析，并把它的索引节点调入内存，然后返回指向这个索引节点的指针。
     * 如果分析失败，返回NIL_INODE作为函数值，并在'err_code'中返回错误代码。
     */
    register Inode *last_inode, *np;
    char filename[NAME_MAX];    /* 最终目录项的名称 */
    memset(filename, 0, NAME_MAX);  /* 初始化一下，很重要，因为后面要跟磁盘中的文件进行比较！ */

    /* 首先得到路径中最后一个目录项 */
    last_inode = last_dir(path, filename);
    if(last_inode == NIL_INODE){
        return NIL_INODE;       /* 无法打开最终目录 */
    }

    /* "/"是一种特殊情况 */
    if(filename[0] == '\0') return last_inode;
    /* 获取最终的文件 */
    np = step_dir(last_inode, filename);
    return np;
}

/*===========================================================================*
 *				last_dir				     *
 *		得到给定路径上最终文件的目录项名称，返回其父目录的索引节点
 *===========================================================================*/
PUBLIC Inode *last_dir(
        char *path,                 /* 要解析的路径名 */
        char last_name[NAME_MAX]    /* 最后一个目录项的名称在这返回 */
){
    /* 有一点需要注意：目前flyanx文件系统v1.0中，所有文件都只存储到根目录中，
     * 没有文件夹这一说法，为了简单起见我这么做，但是其拓展也是很容易的，但现在
     * 不在我的考虑范围内。
     *
     * 关于调用本例程，例如在执行last_dir("/hello.c", name)后，我们将得到：
     *  - 文件名：hello.c
     *  - 返回值（索引节点）：成功则是"/"的索引节点，失败则为NIL_INODE
     *
     * 当前文件系统，文件名前面只能是根目录"/"开头（后面如果还有"/"，跳过），文件
     * 名可以包含"/"和"\\0"的任何字符，包括特殊字符。
     *
     */
    const char *pp = path;
    char *lnp = last_name;

    /* 路径地址不正确？ */
    if(pp == NIL_PTR){
        return NIL_INODE;
    }

    /* 找到了"/"，如果后面还有'/'，我们跳过 */
    while (*pp == '/'){
        pp++;
    }

    /* 解析文件名 */
    while (*pp) {
        /* 解析到的文件名给last_name */
        *lnp = *pp;
        lnp++;
        pp++;
        /* 如果长度要越界了，停止解析 */
        if(lnp - last_name >= NAME_MAX){
            break;
        }
    }
    *lnp = 0; /* 文件名结束 */

    /* 只能是根节点"/" */
    return root_inode;
}

/*===========================================================================*
 *				step_dir				     *
 *		解析路径名的一个目录项，返回其索引节点
 *===========================================================================*/
PUBLIC Inode *step_dir(
        Inode *dir,                 /* 在哪个索引节点（目录）下搜索？ */
        char name[NAME_MAX]         /* 目录下的哪个文件？ */
){
    /* 以目录指针和一个字符串为参数，在该目录中查找该字符串。如果找到，
     * 将其打开并返回指向相应索引节点的指针。如果无法完成，返回NIL_INODE。
     */

    register int i;
    register int j;
    int m = 0;      /* 记录读取到扇区中第几个目录项了 */

    /* 在目录中搜索该文件 */
    int dir_blk0_nr = dir->start_sect;
    int nr_dir_blks = (dir->size + SECTOR_MASK) / SECTOR_SIZE;
    int nr_dir_entries = dir->size / sizeof(DirectoryEntry);    /* 包含了未使用的插槽
                                                                 * （文件已删除，但该插槽依旧存在）
                                                                 */

//    printf("on '%s' search '%s'.\n", "/", name);
//    printf("inode info: num:%d, dev:%d, start_sect:%d, size:%d\n", dir->num, dir->device, dir->start_sect, dir->size);
    DirectoryEntry *dp;

    for(i = 0; i < nr_dir_blks; i++){
        READ_SECT(dir->device, dir_blk0_nr + i);
        dp = (DirectoryEntry*)fs_buffer;
        /* 读取一个扇区中的所有目录项 */
        for(j = 0; j < SECTOR_SIZE / sizeof(DirectoryEntry); j++, dp++){
            if(memcmp(name, dp->name, NAME_MAX ) == 0){
                /* 找到了该目录 */
                return get_inode(dir->device, dp->inode_nr);
            }
            m++;
            /* 如果要读取的目录项超出了这个块，结束。 */
            if(m > nr_dir_entries){
                break;
            }
        }
        /* 如果要读取的目录项超出了这个块，结束。 */
        if(m > nr_dir_entries){
            break;
        }
    }
    return NIL_INODE;
}

/*===========================================================================*
 *				search_dir				     *
 *===========================================================================*/
PUBLIC int *search_dir(char *path){

}


