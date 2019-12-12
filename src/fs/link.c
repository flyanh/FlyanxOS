/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件处理LINK和UNLINK系统调用。
 *
 * 该文件的入口点是：
 *  do_link:	处理LINK系统调用
 *  do_unlink:	处理UNLINK系统调用
 */

#include "fs.h"
#include <sys/stat.h>
#include <string.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "dev.h"
#include "file.h"
#include "fsproc.h"
#include "inode.h"
#include "param.h"
#include "super.h"
#include <dir.h>


/*===========================================================================*
 *				do_link					     *
 *			处理LINK系统调用
 *===========================================================================*/
PUBLIC int do_link(void)
{
    /* flyanx1.0文件系统暂不支持此调用，但可以先放在这。 */
    return fs_no_sys();
}

/*===========================================================================*
 *				do_unlink				     *
 *			处理UNLINK系统调用
 *===========================================================================*/
PUBLIC int do_unlink(void) {
    /* 一般unlink是可以取消文件和目录的引用联系，所以unlink应该也能
     * 处理删除一个目录rmdir调用、但是flyanx1.0文件系统只有一个根目
     * 录，所以我们不能那么做，现在只能删除文件。
     *
     * 我们清除磁盘上索引节点数组中的文件索引节点，（实际上我们可以只
     * 是标记它是一个可用的节点），同时我们也不会清除数据的字节，因此
     * 文件是可恢复的。
     */

    char filename[NAME_MAX];
    memset(filename, 0, NAME_MAX);  /* 初始化一下，很重要，因为后面要跟磁盘中的文件进行比较！ */

    Inode *last_dir_inode, *file_inode;

    /* 获取路径名称 */
    if(get_pathname(pathname, pathname_length, M3) != OK) return err_code;
    if(strcmp(user_path, "/") == OK){
        return EPERM;       /* 想删除根目录？你在想屁吃。 */
    }

    /* 获取文件父目录的索引节点 */
    last_dir_inode = last_dir(user_path, filename);
    /* 获取文件的索引节点 */
    file_inode = step_dir(last_dir_inode, filename);
    if(file_inode == NIL_INODE){
        return ENOENT;      /* 没有此文件 */
    }

   if(file_inode->mode != I_REGULAR) {  /* 只能删除普通文件 */
       printf("{FS}-> cannot remove file %s, because it is not a regular file.\n", user_path);
       return EPERM;
   }

   if(file_inode->count > 1){           /* 不能删除被打开的文件，1表示被加载，1以上代表被打开多少次，例如2就是被打开了一次。 */
       printf("{FS}-> cannot remove file %s, because it has been opened(%d).\n", user_path, file_inode->count);
       return EPERM;
   }

   SuperBlock *sb = get_super_block(file_inode->device);

   /* ================================== */
   /* ======== 释放它的索引节点位图 ======= */
   /* ================================== */
   int byte_index = file_inode->num / 8;
   int bit_index = file_inode->num % 8;
   if(byte_index >= SECTOR_SIZE){      /* 我们只有一个扇区用于索引节点映射 */
       fs_print_info("inode nr over inode-map.");
       return ENOENT;
   }
   /* 读取扇区2（跳过引导区和超级块） */
   READ_SECT(file_inode->device, 2);
   if( !(fs_buffer[byte_index % SECTOR_SIZE] & (1 << bit_index)) ){
       return EPERM;
   }
   fs_buffer[byte_index % SECTOR_SIZE] &= ~(1 << bit_index);
   WRITE_SECT(file_inode->device, 2);

   /* ================================== */
   /* ========= 释放它的扇区位图 ========= */
   /* ================================== */
   bit_index = file_inode->start_sect - sb->first_sect_nr + 1;
   byte_index = bit_index / 8;
   int bits_left = file_inode->nr_sects;
   int byte_count = (bits_left - (8 - (bit_index % 8))) / 8;

   /* 当前的扇区号 */
   int curr_sect_nr = 2 +  /* 2:引导区 + 超级块 */
           sb->nr_imap_sects + byte_index / SECTOR_SIZE;

   READ_SECT(file_inode->device, curr_sect_nr);

   int i;
   /* 清除第一个字节 */
   for(i = bit_index % 8; (i < 8) && bits_left; i++,bits_left--){
       if((fs_buffer[byte_index % SECTOR_SIZE] >> i & 1) != 1){
           fs_panic("clear first byte is malfunction", (fs_buffer[byte_index % SECTOR_SIZE] >> i & 1));
       }
       fs_buffer[byte_index % SECTOR_SIZE] &= ~(1 << i);
   }

   int k;
   /* 清除第二个字节到倒数第二个字节 */
   i = (byte_index % SECTOR_SIZE) + 1;     /* 第二个字节 */
   for(k = 0; k < byte_count; k++, i++, bits_left -= 8){
       if(i == SECTOR_SIZE){
           i = 0;
           WRITE_SECT(file_inode->device, curr_sect_nr);
           curr_sect_nr++;
           READ_SECT(file_inode->device, curr_sect_nr);
       }
       if(fs_buffer[i] != 0xFF){
           fs_panic("clear second~second to last bytes is malfunction", NO_NUM);
       }
       fs_buffer[i] = 0;
   }

   /* 清除最后一个字节 */
   if(i == SECTOR_SIZE){
       i = 0;
       WRITE_SECT(file_inode->device, curr_sect_nr);
       curr_sect_nr++;
       READ_SECT(file_inode->device, curr_sect_nr);
   }
   unsigned char mask = ~((unsigned char)(~0) << bits_left);
   if((fs_buffer[i] & mask) != mask){
       fs_panic("clear last byte is malfunction", NO_NUM);
   }
   WRITE_SECT(file_inode->device, curr_sect_nr);
   /* ================================== */
   /* ========= 释放它的索引节点 ========= */
   /* ================================== */
   file_inode->mode = 0;
   file_inode->size = 0;
   file_inode->start_sect = 0;
   file_inode->nr_sects = 0;
   sync_inode(file_inode);
   /* 释放索引节点表插槽 */
   put_inode(file_inode);

   /* =============================================== */
   /* ========= 在目录条目中将索引节点号设置为0 ========= */
   /* =============================================== */
   int dir_blk0_nr = last_dir_inode->start_sect;
   int nr_dir_blks = (last_dir_inode->size + SECTOR_SIZE) / SECTOR_SIZE;
   int nr_dir_entries = last_dir_inode->size / sizeof(DirectoryEntry);  /* 包含了未使用的插槽
                                                                         * （文件已删除，但该插槽依旧存在）
                                                                         */
   int m = 0;
   DirectoryEntry *dep;
   bool file_found = FALSE;
   int dir_size = 0;

   for(i = 0; i < nr_dir_blks; i++){
       READ_SECT(last_dir_inode->device, dir_blk0_nr + i);

       dep = (DirectoryEntry*) fs_buffer;
       int j;
       for(j = 0; j < SECTOR_SIZE / sizeof(DirectoryEntry); j++, dep++){
           m++;
           if(m > nr_dir_entries) break;

           if(dep->inode_nr == file_inode->num){     /* 找到该索引节点了 */
               memset(dep, 0, sizeof(DirectoryEntry));
               WRITE_SECT(last_dir_inode->device, dir_blk0_nr + i);
               file_found = TRUE;
               break;
           }

           if(dep->inode_nr != 0){
               dir_size += sizeof(DirectoryEntry);
           }
       }
       if(m > nr_dir_entries ||    /* 该扇区目录项都已经被使用 或者 */
           file_found){            /* 文件已经被找到 */
           break;
       }
   }
   if(!file_found){
       fs_panic("file not found, but it on memory, it inode nr on code", file_inode->num);
   }
   if(m == nr_dir_entries){    /* 该文件是目录中的最后一个，那么我们需要将其更新到磁盘。 */
       last_dir_inode->size = dir_size;
       sync_inode(last_dir_inode);
   }
   printf("{FS}-> remove file %s success.\n", filename);
   return OK;
}




