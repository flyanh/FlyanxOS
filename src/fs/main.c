/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 文件系统
 * 本文件入口点是：
 *  - main:         文件系统的主程序
 *  - reply:        完成工作后，向进程发送回复
 */

#include "fs.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include <flyanx/partition.h>
#include <string.h>
#include "dev.h"
#include "file.h"
#include "fsproc.h"
#include "inode.h"
#include <dir.h>
#include "super.h"
#include "param.h"

FORWARD _PROTOTYPE( void fs_init, (void) );
FORWARD _PROTOTYPE( void view_inbox, (void) );
FORWARD _PROTOTYPE( void mkfs_flyanx, (void) );
FORWARD _PROTOTYPE( void load_super_block, (SuperBlock *sb) );

/*===========================================================================*
 *				fs_main					     *
 *			  文件系统主程序
 *===========================================================================*/
PUBLIC void fs_main(void){
    int rs;

    /* 文件系统初始化 */
    fs_init();

    fs_print_info("Working...");
    /* 文件系统开始工作了 */
    while (TRUE){
        view_inbox();   /* 查看收件箱，等待来信 */

        call_fp = &fsproc[who]; /* 得到调用进程 */
        super_user = (call_fp->eff_uid == SU_UID ? TRUE : FALSE);   /* su超级用户？ */
        need_reply = TRUE;      /* 默认情况下的调用 有回复 */

        /* 如果调用号有效，则执行调用完成工作 */
        /* 如果调用号有效，则执行调用 */
        if((unsigned) fs_call >= NR_CALLS){
            rs = ENOSYS;
        } else {
            rs = (fs_call_handlers[fs_call])();
        }

        /* 将结果拷贝给用户并发送回复。 */
        if(!need_reply) continue;       /* 不需要回复，这次的工作结束 */
        fs_reply(who, rs);
    }
}

/*===========================================================================*
 *				view_inbox					     *
 *             查看收件箱，等待消息
 *===========================================================================*/
PRIVATE void view_inbox(void){
    /* 查看收件箱，等待一条消息得到工作
     */

    /* 正常情况下，没有人会被管道挂起，也没有人会被恢复 */
    if(receive(ANY, &fs_inbox) != OK) fs_panic("FS receive error", NO_NUM);
    who = fs_inbox.source;
    fs_call = fs_inbox.type;
}

/*===========================================================================*
 *				fs_reply					     *
 *			    回复结果
 *===========================================================================*/
PUBLIC void fs_reply(int whom, int rs){
    /* 向用户进程发送回复。 它可能会失败（如果该进程刚刚被信号杀死），因此不需要检查返回码。
     * 如果发送失败，则忽略它。
     */
    reply_type = rs;
    send(whom, &fs_outbox);
}

/*===========================================================================*
 *				fs_init					     *
 *			文件系统初始化
 *===========================================================================*/
PRIVATE void fs_init(void){
    /* 做一些准备工作 */
    int i, istat = TRUE;
    SuperBlock *sb;
    FSProcess *fp;

    /* 初始化进程表 */
    for(i = 0; i <= LOW_USER; i++){
        if(i == FS_PROC_NR) continue;   /* 不初始化自己 */
        fp = &fsproc[i];

        /* id */
        fp->real_uid = (uid_t) SYS_UID;
        fp->eff_uid = (uid_t) SYS_UID;
        fp->real_gid = (uid_t) SYS_GID;
        fp->eff_gid = (uid_t) SYS_GID;
        fp->umask = ~0;
        fp->pid = i < LOW_USER ? PID_SERVER : 1;
    }

    /* 打开硬盘 */
    if (dev_open(ROOT_DEV, FS_PROC_NR, (RWX_MODES)) == OK) {
        /* 先尝试读取根设备超级块 */
        if(READ_SECT(ROOT_DEV, 1)){
            sb = (SuperBlock *) fs_buffer;
            if(sb->magic != SUPER_MAGIC) {
                /* 如果文件系统不是我们flyanx v1.0的，那么我们创建一个新的文件系统 */
                mkfs_flyanx();
                sb = NIL_SUPER_BLOCK;
            }
            /* 现在，为根设备加载超级块 */
            load_super_block(sb);
            /* 加载完成超级块后，获取它然后看下魔数，看是否真的成功加载了。 */
            sb = get_super_block(ROOT_DEV);
            if(sb->magic != SUPER_MAGIC){
                fs_panic("super block not support! it magic bad.(in code).", sb->magic);
            }
            /* 最后，得到根索引节点 */
            root_inode = get_inode(ROOT_DEV, ROOT_INODE);
        } else {
            istat = FALSE;  /* 初始化失败 */
        }
    } else {
        istat = FALSE;  /* 初始化失败 */
    }
    /* 初始化失败，宕机 */
    if(istat == FALSE) {
        fs_panic("A serious error occurred in initializing.", NO_NUM);
    }
}

/*===========================================================================*
 *				mkfs_flyanx					     *
 *			创建flyanx文件系统
 *===========================================================================*/
PRIVATE void mkfs_flyanx(void){
    /* 在磁盘中创建flyanx 0.1文件系统，主要完成工作如下：
     *  - 将超级块写入第一个扇区。
     *  - 创建三个特效文件：dev_tty0, dev_tty1. dev_tty2，它们用于终端。
     *  - 创建文件cmd.tar。
     *  - 创建索引节点映射。
     *  - 创建扇区映射。
     *  - 创建文件的索引节点。
     *  - 创建根目录'/'。
     */
    fs_print_info("mkfs.flyanx v1.0 file system for hard disk.");
    int i, j;
    /* ================================== */
    /* ========= 首先，写入超级块 ========= */
    /* ================================== */
    /* 获取根目录的 */
    Partition geo;
    if(dev_ioctl(ROOT_DEV, FS_PROC_NR, &geo) != OK){
        fs_panic("get hard disk geometry failed.", NO_NUM);
    }
    printf("{FS}-> dev size: 0x%x sectors.\n", geo.size);

    int bits_per_sect = SECTOR_SIZE * 8;    /* 一字节8位 */
    /* 生成一个超级块 */
    SuperBlock sb;
    sb.magic = SUPER_MAGIC;     /* flyanx文件系统魔数 */
    sb.nr_inodes = bits_per_sect;
    sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
    sb.nr_sects = geo.size;     /* 分区信息中的扇区数量 */
    sb.nr_imap_sects = 1;
    sb.nr_smap_sects = sb.nr_sects / bits_per_sect + 1;
    sb.first_sect_nr = 1 + 1 +  /* 引导扇区和超级块 */
            sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
    sb.root_inode = ROOT_INODE;
    sb.inode_size = INODE_SIZE;
    Inode root;
    sb.inode_size_off = (int)&root.size - (int)&root;
    sb.inode_start_off = (int)&root.start_sect - (int)&root;
    sb.dir_ent_size = sizeof(DirectoryEntry);
    DirectoryEntry root_dir;
    sb.dir_ent_inode_off = (int)&root_dir.inode_nr - (int)&root_dir;
    sb.dir_ent_name_off = (int)&root_dir.name - (int)&root_dir;

    /* 将超级块写入到磁盘第一个扇区中 */
    memset(fs_buffer, 0x90, SECTOR_SIZE);
    memcpy(fs_buffer, &sb, SUPER_BLOCK_SIZE);   /* 将超级块的内容放入高速缓冲区 */
    WRITE_SECT(ROOT_DEV, 1);               /* 写到第一个扇区中 */

    printf("{FS}-> devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
           "        inodes:0x%x00, 1st_sector:0x%x00\n",
           geo.base * 2,
           (geo.base + 1) * 2,
           (geo.base + 1 + 1) * 2,
           (geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
           (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
           (geo.base + sb.first_sect_nr) * 2);

    /* ================================== */
    /* ====== 其次，设置索引节点映射 ======= */
    /* ================================== */
    memset(fs_buffer, 0, SECTOR_SIZE);  /* 清空一个扇区的缓冲区大小 */
    /* dev_tty0, dev_tty1. dev_tty2 */
    for(i = 0; i < NR_CONSOLES + 3; i++){
        fs_buffer[0] |= 1 << i;
    }
    if(fs_buffer[0] != 0x3F){
        fs_panic("set inode map failed.", NO_NUM);
        /* 这里必须为0x3F，他们各位使用如下。
         * 0011 1111 :
         *   || ||||
         *   || |||`--- bit 0 : 保留待用
         *   || ||`---- bit 1 : 根节点
         *   || ||              它指向 `/' 根目录
         *   || |`----- bit 2 : /dev_tty0   终端1
         *   || `------ bit 3 : /dev_tty1   终端2
         *   |`-------- bit 4 : /dev_tty2   终端3
         *   `--------- bit 5 : /cmd.tar    系统初始自带的命令程序，是一个压缩包
         */
    }
    WRITE_SECT(ROOT_DEV, 2);        /* 第二个扇区 */

    /* ================================== */
    /* ========== 设置扇区映射 ============ */
    /* ================================== */
    memset(fs_buffer, 0, SECTOR_SIZE);
    int nr_sects = NR_DEFAULT_FILE_SECTS + 1;   /* 给根目录使用，+1是保留待用1个扇区。 */
    for(i = 0; i < nr_sects / 8; i++){
        fs_buffer[i] = 0xFF;                    /* 每个扇区都置位全1 */
    }
    for(j = 0; j < nr_sects % 8; j++){
        fs_buffer[i] |= (1 << j);
    }
    WRITE_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

    /* 用零填充剩余的扇区映射 */
    memset(fs_buffer, 0, SECTOR_SIZE);
    for(i = 1; i < sb.nr_smap_sects; i++){
        WRITE_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);
    }

    /* ================================== */
    /* ======== 创建文件cmd.tar ========== */
    /* ================================== */
    /* 要确保这个文件不会被磁盘日志给覆盖 */
    if(INSTALL_START_SECT + INSTALL_NR_SECTS >= sb.nr_sects - NR_SECTS_LOG){
        fs_panic("cmd.tar too little space available.", NO_NUM);
    }
    int bit_offset = INSTALL_START_SECT - sb.first_sect_nr; /* 扇区M偏移 = (M - sb.first_sect_nr + 1)  */
    int bit_off_in_sect = bit_offset % (SECTOR_SIZE << 3 /* == *8 */);
    int bit_left = INSTALL_NR_SECTS;
    int curr_sect = bit_offset / (SECTOR_SIZE << 3);
    READ_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + curr_sect);
    while (bit_left) {
        int byte_off = bit_off_in_sect >> 3;    /* == /8 */
        /* 这行在循环中效率低下，但无所谓，系统还没真正启动起来 */
        fs_buffer[byte_off] |= 1 << (bit_off_in_sect % 8);
        bit_left--;
        bit_off_in_sect++;
        if(bit_off_in_sect == (SECTOR_SIZE << 3)){
            WRITE_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + curr_sect);
            curr_sect++;
            READ_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + curr_sect);
            bit_off_in_sect = 0;
        }
    }
    WRITE_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + curr_sect);

    /* ================================== */
    /* ========== 设置索引节点 ============ */
    /* ================================== */
    /* 首先是根'/'的 */
    memset(fs_buffer, 0, SECTOR_SIZE);
    Inode *roo_inode = (Inode *)fs_buffer; /* 这相当于我们手动分配空间，不需要编译器给我们瞎分配 */
    roo_inode->mode = I_DIRECTORY;     /* '/'是一个目录 */
    roo_inode->size = sizeof(DirectoryEntry) * 5;  /* '/'下有五个文件：
                                                     * '.',
                                                     * 'dev_tty0', 'dev_tty1', 'dev_tty2',
                                                     * 'cmd.tar'
                                                     */
    roo_inode->start_sect = sb.first_sect_nr;      /* 根使用第一个数据扇区 */
    roo_inode->nr_sects = NR_DEFAULT_FILE_SECTS;   /* 占用多少个扇区？ */
    /* 然后是'/dev_tty0~2' */
    Inode *tty_inode;
    for(i = 0; i < NR_CONSOLES; i++){
        tty_inode = (Inode *) (fs_buffer + (INODE_SIZE * (i + 1))); /* +1防止把'/'的覆盖掉 */
        tty_inode->mode = I_CHAR_SPECIAL;
        tty_inode->size = 0;
        tty_inode->start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
        tty_inode->nr_sects = 0;
    }
    /* 最后是'/cmd,tar'的 */
    Inode *cmd_inode = (Inode *)(fs_buffer + (INODE_SIZE * (NR_CONSOLES + 1))); /* 后面这里应该要可以根据NR_CONSOLES动态变化 */
    cmd_inode->mode = I_REGULAR;
    cmd_inode->size = INSTALL_NR_SECTS * SECTOR_SIZE;
    cmd_inode->start_sect = INSTALL_START_SECT;
    cmd_inode->nr_sects = INSTALL_NR_SECTS;
    WRITE_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);  /* 将这些索引节点写进映射中 */

    /* ================================== */
    /* ========== 创建出'/'文件 =========== */
    /* ================================== */
    memset(fs_buffer, 0, SECTOR_SIZE);
    DirectoryEntry *p_root_dir = (DirectoryEntry *)fs_buffer;

    p_root_dir->inode_nr = ROOT_INODE;  /* '/'文件跟索引节点建立关联 */
    strcpy(p_root_dir->name, dot);

    /* 根目录下有'/dev_tty0~2'文件，设置它们的目录项 */
    for(i = 0; i < NR_CONSOLES; i++){
        p_root_dir++;
        p_root_dir->inode_nr = i + ROOT_INODE + 1;   /* dev_tty0文件的索引节点号是2（'/'索引号 + 1） */
        /* 文件名，有9个分支是因为flyanx允许编译开启最多九个终端。 */
        switch (i){
            case 0: strcpy(p_root_dir->name, "dev_tty0"); break;
            case 1: strcpy(p_root_dir->name, "dev_tty1"); break;
            case 2: strcpy(p_root_dir->name, "dev_tty2"); break;
            case 3: strcpy(p_root_dir->name, "dev_tty3"); break;
            case 4: strcpy(p_root_dir->name, "dev_tty4"); break;
            case 5: strcpy(p_root_dir->name, "dev_tty5"); break;
            case 6: strcpy(p_root_dir->name, "dev_tty6"); break;
            case 7: strcpy(p_root_dir->name, "dev_tty7"); break;
            case 8: strcpy(p_root_dir->name, "dev_tty8"); break;
        }
    }
    /* 最后是'cmd.tar'的 */
    (++p_root_dir)->inode_nr = NR_CONSOLES + 2;
    strcpy(p_root_dir->name, "cmd.tar");
    WRITE_SECT(ROOT_DEV, sb.first_sect_nr);

    /* 至此，硬盘上已经有了flyanx 1.0文件系统 */
}

/*===========================================================================*
 *				load_super_block					     *
 *				   加载超级块
 *===========================================================================*/
PRIVATE void load_super_block(SuperBlock *sb){
    /* 初始化超级块表 */
    SuperBlock *psb = super_block;
    for(; psb < &super_block[NR_SUPER_BLOCK]; psb++){
        psb->super_dev = NO_DEV;
    }

    if(sb == NIL_SUPER_BLOCK){
        /* 如果传入的超级块是坏的，我们读取第一块扇区，即超级块所在位置 */
        memset(fs_buffer, 0, SECTOR_SIZE);
        READ_SECT(ROOT_DEV, 1);
        sb = (SuperBlock *)fs_buffer;
    }

    /* 装载超级块 */
    super_block[0] = *sb;
    super_block[0].super_dev = ROOT_DEV;
}

/*===========================================================================*
 *				fs_print_info					     *
 *				文件系统输出信息
 *===========================================================================*/
PUBLIC void fs_print_info(char *info){
    printf("{FS}-> %s\n", info);
}
