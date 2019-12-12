/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 文件系统的进程信息部分
 * 为每个潜在进程保留一个插槽，在这里不可能甚至没有必要告知插槽何时可用。
 */

#ifndef _FSPROC_H
#define _FSPROC_H


typedef struct fs_process_s {
    mode_t umask;                   /* 由umask系统调用设置的掩码 */
    Inode *work_dir;                /* 进程工作目录 */
    Inode *root_dir;                /* 根目录（参阅chroot调用） */
    FileDesc *open_file[OPEN_MAX];   /* 进程打开的所有文件，有限制... */

    uid_t real_uid;		            /* 真实用户ID */
    uid_t eff_uid;		            /* 有效用户ID */
    gid_t real_gid;		            /* 真实组ID */
    gid_t eff_gid;	                /* 有效组ID */
    dev_t tty;		                /* 控制终端的(主要/次要)设备号 */

    int fd;                         /* 如果读/写暂时无法完成，保存文件描述符 */
    char *buffer;                   /* 如果读/写暂时无法完成，保存缓冲区 */
    int bytes;                      /* 如果读/写暂时无法完成，保存读写字节数量 */
    int fp_cum_io_partial;          /* 如果读/写暂时无法完成，保存部分字节计数 */
    bool suspended;                 /* 进程是否被挂起？ */
    bool revived;                   /* 进程正在恢复？ */
    char task;                      /* 进程被哪个任务挂起了？ */
    bool ses_leader;                /* 是否是会话领导者？ */

    pid_t pid;                      /* 进程号 */
} FSProcess;

FSProcess fsproc[NR_PROCS];

/* 标志值 */
#define NOT_SUSPENDED      0	/* 进程未在管道或任务上挂起 */
#define SUSPENDED          1	/* 进程挂起在管道或任务上 */
#define NOT_REVIVING       0	/* 进程没有恢复 */
#define REVIVING           1	/* 进程从挂起中恢复 */
#define PID_FREE	       0	/* 无进程插槽 */
#define PID_SERVER	      (-0x328)/* 进程已成为服务器 */

#endif //_FSPROC_H
