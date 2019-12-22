/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件包含文件系统读写文件的处理
 *
 * 该文件包含用于读取/写入文件机制的核心，读写请求被分成不跨越块边界的块，然后依次处理每个块，
 * 同时它还可以检测并处理对特殊文件的读写。
 *
 * 该文件的入口点是：
 *  - do_read:      执行READ系统调用
 *  - do_write:     执行WRITE系统调用
 *  - read_write:   实际完成读写工作
 */

#include "fs.h"
#include <fcntl.h>
#include <flyanx/common.h>
#include "dev.h"
#include "file.h"
#include "fsproc.h"
#include "inode.h"
#include "param.h"

/*===========================================================================*
 *				do_read				     *
 *			执行READ系统调用
 *===========================================================================*/
PUBLIC int do_read(void){
    return read_write(READING);
}

/*===========================================================================*
 *				do_write				     *
 *			执行WRITE系统调用
 *===========================================================================*/
PUBLIC int do_write(void){
    return read_write(WRITING);
}

/*===========================================================================*
 *				read_write				     *
 *				读写处理
 *===========================================================================*/
PUBLIC int read_write(
        int rw_flag     /* 读/写标志？READING/WRITING */
){
    /* 执行read(fd，in_buffer，bytes)或write(fd，in_buffer，bytes)调用。
     * 文件读写的实际处理函数，其do_read和do_write最终都会调用本例程
     * 处理。
     */

    /* 为了健壮性 */
    if( (call_fp->open_file[in_fd] < &file[0]) || (call_fp->open_file[in_fd] >= &file[NR_FILES]) ){
        /* 无效的文件描述符 */
        return EINVAL;
    }
    if( ((rw_flag == READING) && (call_fp->open_file[in_fd]->mode & O_WRONLY))
        || ((rw_flag == WRITING) && (call_fp->open_file[in_fd]->mode & O_RDONLY)) ){
        /* 没有读或者写权限 */
        return EACCES;
    }

    off_t pos = call_fp->open_file[in_fd]->pos;         /* 文件位置 */
    Inode *inp = call_fp->open_file[in_fd]->inode;      /* 文件的索引节点 */

    /* 这个文件描述符的索引节点有问题？ */
    if(inp < &inode[0] || inp >= &inode[NR_INODES] ){
        fs_panic("this fd's inode bad, fd in code.", in_fd);
    }

    int oflags = call_fp->open_file[in_fd]->flags;
    if(fs_who < ORIGIN_PROC_NR){    /* 我们在这定死了规则，这不应该，但很有效，以后改进为应该根据用户的选择来 */
        oflags |= O_NONBLOCK;       /* 如果操作进程是服务器，那么它们不应该被堵塞。 */
    }

    int imode = inp->mode & I_TYPE;     /* 文件索引节点的存取模式 */

    if(imode == I_CHAR_SPECIAL){        /* 文件是一个特殊字符设备 */
        /* 发出设备io请求 */
        int dev = inp->start_sect;      /* 得到设备号 */
        if(MAJOR(dev) != DEV_CHAR_TTY){
            return ENODEV;      /* 不存在这个设备，因为索引节点的mode指示这个文件是一个字符设备，但索引节点拿到的设备号却对不上！ */
        }
        /* 执行设备io */
        dev_io(rw_flag == READING ? DEVICE_READ : DEVICE_WRITE,
               dev, fs_who, in_buffer, pos, in_bytes, oflags);
        return fs_outbox.REPLY_STATUS;  /* 返回io结果，如果成功则是传输的字节数，失败则是错误代码。 */
    } else {                            /* 普通文件/目录 ... */
        /* 计算读写文件位置的最大值 */
        int pos_end;
        if(rw_flag == READING){  /* 读 */
            pos_end = MIN(pos + in_bytes, inp->size);
        } else {                /* 写 */
            pos_end = MIN(pos + in_bytes, inp->nr_sects * SECTOR_SIZE);
        }
        /* 计算偏移和读写扇区参数的最大最小值 */
        int off = pos % SECTOR_SIZE;
        int rw_sect_min = inp->start_sect + (pos >> SECTOR_SIZE_SHIFT);
        int rw_sect_max = inp->start_sect + (pos_end >> SECTOR_SIZE_SHIFT);
        /* 计算读写的块大小 */
        int chunk = MIN(rw_sect_max - rw_sect_min + 1, FS_BUFFER_SIZE >> SECTOR_SIZE_SHIFT);
        /* 开始读写 */
        int bytes_rw = 0;
        int bytes_left = in_bytes;
        int i;
        for(i = rw_sect_min; i <= rw_sect_max; i += chunk) {
            /* 每次读写一个块的字节数，直到完成 */
            int bytes = MIN(bytes_left, chunk * SECTOR_SIZE - off);

            if(rw_flag == READING){      /* 读 */
                /* 先从磁盘中读取 */
                dev_io(DEVICE_READ, inp->device, FS_PROC_NR, fs_buffer,
                       i * SECTOR_SIZE, chunk * SECTOR_SIZE, oflags);
                /* 将读取的数据复制给用户 */
                sys_copy(FS_PROC_NR, DATA, (phys_bytes) (fs_buffer + off),
                         fs_who, DATA, (phys_bytes) (in_buffer + bytes_rw), bytes);
            } else {                    /* 写 */
                /* 先将用户要写入的数据拷贝到文件系统缓冲区中 */
                sys_copy(fs_who, DATA, (phys_bytes) (in_buffer + bytes_rw),
                        FS_PROC_NR, DATA, (phys_bytes) (fs_buffer + off), bytes);
                /* 再将数据写入磁盘中 */
                dev_io(DEVICE_WRITE, inp->device, FS_PROC_NR, fs_buffer,
                        i * SECTOR_SIZE, chunk * SECTOR_SIZE, oflags);
            }
            off = 0;            /* 偏移归零 */
            bytes_rw += bytes;  /* 更新读写字节数量 */
            call_fp->open_file[in_fd]->pos += bytes;    /* 更新文件位置 */
            bytes_left -= bytes;        /* 更新剩余需要传输的字节 */
        }

        if(call_fp->open_file[in_fd]->pos > inp->size){
//            printf("sync_inode\n");
            /* 更新文件大小 */
            inp->size = call_fp->open_file[in_fd]->pos;
            /* 将更新的索引节点写回到磁盘 */
            sync_inode(inp);
        }

        /* 返回读写字节量 */
        return bytes_rw;
    }
}

