/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含创建，打开，关闭和查找文件的过程。
 *
 * 该文件的入口点是：
 * 	do_creat:	    执行CREAT系统调用
 *  do_open:	    执行OPEN系统调用
 *  do_mknod:	    执行MKNOD系统调用
 *  do_mkdir:	    执行MKDIR系统调用
 *  do_close:	    执行CLOSE系统调用
 *  do_lseek:  	    执行LSEEK系统调用
 */
#include "fs.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include "dev.h"
#include "dir.h"
#include "file.h"
#include "param.h"
#include "fsproc.h"

FORWARD _PROTOTYPE( int common_open, (register int oflag, mode_t omode) );
FORWARD _PROTOTYPE( Inode *create_file, (char *path, mode_t bits) );
FORWARD _PROTOTYPE( void new_dir_entry, (Inode *dir_inode, ino_t inode_nr, char *filename) );

/*===========================================================================*
 *				do_create					     *
 *				创建一个新文件
 *===========================================================================*/
PUBLIC int do_creat(void){
    /* 执行creat(name，mode)系统调用。
     * 在老版本的UNIX中，CREAT和OPEN调用用于不同的目的。打开一个并不存在的文件会出错，而一个新文件必须用
     * CREAT创建，CREAT也可以把一个存在文件的长度删除为0。可是POSIX系统不再需要两种不同的调用。在POSIX
     * 中，OPEN调用可以用于创建一个新文件和截断一个老文件，因此CREAT调用实现的功能只是OPEN的一个子集，仅
     * 用于与原有程序相兼容。
     */
    if(get_pathname(pathname, pathname_length, M3) != OK) return err_code;
    return common_open(O_WRONLY | O_CREAT | O_TRUNC, (mode_t) fmode);   /* 调用公共的打开方法 */
}

/*===========================================================================*
 *				do_open					     *
 *===========================================================================*/
PUBLIC int do_open(void){
    /* 执行open(name，flags，...)系统调用。
     * do_open做的事跟do_create差不多，但是多了更多的功能。本例程的主要作用是
     * 打开一个文件并返回一个文件描述符。
     */

    int flags = 0, rs;

    /* 获取用户要打开的文件全路径
     * 如果设置了O_CREAT标志，则open具有三个参数，否则是两个，我们处理一下
     * 因为打开的文件可能会很长，所以这时使用M3类型的消息传送，文件路径名有
     * 可能不在消息中传送，它可能还在用户缓冲区中。
     */
    if(fmode & O_CREAT){
        flags = f_cmode1;
        rs = get_pathname(f_cpathname, pathname1_length, M1);
    } else {
        rs = get_pathname(pathname, pathname_length, M3);
    }
    if(rs != OK) return err_code;       /* 文件名是错误的！ */
    rs = common_open(fmode, flags);
    return rs;
}

/*===========================================================================*
 *				common_open				     *
 *		来自do_creat和do_open的通用代码
 *===========================================================================*/
PRIVATE int common_open(register int oflag, mode_t omode){
    /* 这个函数会被do_creat和do_open调用，若调用函数指定创建新文件(调用时设置O_CREAT位)。
     * common_open的代码，同文件系统的其他许多过程一样，含有大量检查各种错误和非法组合的代码，
     * 这些代码对于设计一个无错的、健壮的文件系统是必不可少的。如果出现错误，以前分配的文件描述
     * 符和file项被收回，i-节点被释放。在这种情况下，common_open过程返回一个负数以表示出错。
     * 若运行成功，返回一个正数，即文件描述符。
     */

    int fd = -1, rs;
    FileDesc *free_slot;
    /* 为文件找一个空闲的文件插槽 */
    rs = get_fd(0, oflag, &fd, &free_slot);
//    printf("new file: %s, fd: %d, &free_slot: %p\n", user_path, fd, &free_slot);
    if(rs != OK) return rs;

    /* 解析路径得到文件的索引节点 */
    Inode *inp = track_path(user_path);

    /* 该文件并不存在，看看用户是否指定了创建标志
     * 如果指定了我们创建它，否则打印一条信息。
     */
    if(inp == NIL_INODE){
        if(oflag & O_CREAT){
            inp = create_file(user_path, oflag);
        } else {
            printf("{FS}-> file not exists: %s\n", user_path);
            return -1;
        }
    } else {
        /* 文件存在 */
        if(oflag & O_RDWR) {    /* 用户想以读写的方式打开 */
            /* 在这判断一下用户是否给了正确的打开参数，要么以读写打开，要么就读写打开并截断文件，
             * 又或者是读写打开且不存在并创建并截断，如果不是这些参数，关闭打开的文件，打印错误
             * 信息并返回错误。
             */
            if(!((oflag == O_RDWR) || (oflag == (O_RDWR | O_TRUNC)) || (oflag == (O_RDWR | O_CREAT | O_TRUNC)))){
                printf("FS flags Internal inconsistency, you should eg. {'O_RDWR', 'O_RDWR | O_TRUNC', 'O_RDWR | O_CREAT | O_TRUNC'}\n");
                put_inode(inp);
                return -1;
            }
            /* 好了，文件已经被打开了，后面没事了。 */
        } else {                /* 并不想读写 */
            printf("{FS}-> file exists: %s\n", user_path);
            return -1;
        }
    }

    if(oflag & O_TRUNC){
        /* 截断一个文件？将其文件大小变为0即可。 */
        if(inp == NIL_INODE) fs_panic("FS open inode is NIL", NO_NUM);
        inp->size = 0;
        sync_inode(inp);
    }

    /* 文件存在 */
    if(inp != NIL_INODE){
        /* 将进程与文件描述符连接 */
        call_fp->open_file[fd] = free_slot;

        /* 将文件描述符和索引节点连接 */
        free_slot->inode = inp;

        free_slot->mode = oflag;
        free_slot->count = 1;
        free_slot->pos = 0;

        int imode = inp->mode & I_TYPE;

        if(imode == I_CHAR_SPECIAL){    /* 字符设备？ */
            int dev = inp->start_sect;
//            printf("dev: %d\n", MAJOR(inp->device));
            dev_open(dev, fs_who, oflag);    /* 打开这个字符设备 */
        } else if(imode == I_DIRECTORY){
            /* 目录？现在只能是"/" */
            if(inp->num != ROOT_INODE){
                fs_panic("FS open a directory not ROOT", NO_NUM);
            }
        } else {
            /* 打开一个普通文件 */
            if(inp->mode != I_REGULAR){
                fs_panic("FS open a file, but not a regular", NO_NUM);
            }
        }
    } else {
        return -1;
    }
    return fd;
}

/*===========================================================================*
 *				create_file				     *
 *			 创建一个新文件并返回它的索引节点
 *===========================================================================*/
PRIVATE Inode *create_file(char *path, mode_t bits){
    char filename[NAME_MAX];
    memset(filename, 0, NAME_MAX);  /* 初始化一下，很重要，因为后面要跟磁盘中的文件进行比较！ */
    Inode *dir_inode = last_dir(path, filename);
    /* 路径不正确不能继续 */
    if(dir_inode == NIL_INODE){
        return NIL_INODE;
    }

    int inode_nr = alloc_imap_bit(dir_inode->device);
    int free_sect_nr = alloc_smap_bit(dir_inode->device, NR_DEFAULT_FILE_SECTS);

    Inode *new_ind = new_inode(dir_inode->device, inode_nr, free_sect_nr);
    /* 建立目录项 */
    new_dir_entry(dir_inode, new_ind->num, filename);
//    printf("creat a file '%s'\n", filename);
//    printf("inode:{num: %d, start_sect: %d, nr_sects: %d}\n", new_ind->num, new_ind->start_sect, new_ind->nr_sects);
    return new_ind;
}

/*===========================================================================*
 *				do_mknod					     *
 *			执行mknod(name，mode，addr)系统调用。
 *===========================================================================*/
PUBLIC int do_mknod(void){
    /* 本例程跟do_creat类似，只是本例程仅仅创建索引节点并为其分配一个目录项。 */

    /* @TODO */
    return fs_no_sys();
}

/*===========================================================================*
 *				do_mkdir					     *
 *				创建新目录
 *===========================================================================*/
PUBLIC int do_mkdir(void){
    /* flyanx1.0文件系统不支持该调用，所以我们什么都不做，调用fs_no_sys即可。 */

    return fs_no_sys();
}

/*===========================================================================*
 *				do_close					     *
 *			  关闭一个文件
 *===========================================================================*/
PUBLIC int do_close(void){
    /* 执close(fd)系统调用。 */

    int fd = in_fd;
    /* 释放该文件的索引节点 */
    put_inode(call_fp->open_file[fd]->inode);
    call_fp->open_file[fd]->count--;
    /* 如果该文件已经不再被需要，按次序关闭它 */
    if(call_fp->open_file[fd]->count == 0){
        call_fp->open_file[fd]->inode = NIL_INODE;
    }
    call_fp->open_file[fd] = NIL_FILE;
//    printf("close file(%d) success!\n", fd);

    return OK;
}

/*===========================================================================*
 *				do_lseek					     *
 *			   设置文件位置
 *===========================================================================*/
PUBLIC int do_lseek(void){
    /* 执行lseek(ls_fd，offset，whence)系统调用。
     * 进行文件读/写时，可以调用本例程设置新的文件位置。
     */

    /* 检查文件描述符是否有效 */
    if(get_file(in_ls_fd) == NIL_FILE) return err_code;

    off_t pos = call_fp->open_file[in_ls_fd]->pos;    /* 当前文件位置 */
    int file_size = call_fp->open_file[in_ls_fd]->inode->size;  /* 文件大小 */

    switch (in_whence){
        /* 从文件头开始*/
        case SEEK_SET:  pos = 0 + in_offset;         break;
        /* 从文件当前位置开始 */
        case SEEK_CUR:  pos += in_offset;            break;
        /* 从文件末尾开始 */
        case SEEK_END:  pos = file_size + in_offset; break;
        /* 参数不对？ */
        default:    return EINVAL;
    }
    /* 看看文件置位是否溢出了？ */
    if((pos > file_size) || (pos < 0)){
        return EINVAL;
    }
    /* 设置新的文件位置 */
    call_fp->open_file[in_ls_fd]->pos = pos;
    reply_l1 = pos;     /* 返回文件新的位置 */
    return OK;
}

/*===========================================================================*
 *				new_dir_entry					     *
 *				建立新的目录项
 *===========================================================================*/
PRIVATE void new_dir_entry(
        Inode *dir_inode,
        ino_t inode_nr,
        char *filename
){
    int dir_blk0_nr = dir_inode->start_sect;
    int nr_dir_blks = (dir_inode->size + SECTOR_SIZE) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode->size / sizeof(DirectoryEntry);  /* 包含了未使用的插槽
                                                                     * （文件已删除，但该插槽依旧存在）
                                                                     */
    int m = 0;       /* 记录读取到扇区中第几个目录项了 */
    DirectoryEntry *dep;
    DirectoryEntry *new_de = 0;

    int i, j;
    for(i = 0; i < nr_dir_blks; i++){
        READ_SECT(dir_inode->device, dir_blk0_nr + i);

        dep = (DirectoryEntry*) fs_buffer;
        for(j = 0; j < SECTOR_SIZE / sizeof(DirectoryEntry); j++, dep++){
            m++;
            if(m > nr_dir_entries) break;

            if(dep->inode_nr == 0){     /* 找到空的目录项插槽了 */
                new_de = dep;
                break;
            }
        }
        if(m > nr_dir_entries ||    /* 该扇区目录项都已经被使用 或者 */
                new_de){            /* 找到了空闲插槽 */
            break;
        }
    }
    if(!new_de){
        new_de = dep;
        dir_inode->size += sizeof(DirectoryEntry);
    }
    new_de->inode_nr = inode_nr;
    strcpy(new_de->name, filename);
    /* 写目录块-->根目录块 */
    WRITE_SECT(dir_inode->device, dir_blk0_nr + i);
    /* 更新目录索引节点 */
    sync_inode(dir_inode);
}

