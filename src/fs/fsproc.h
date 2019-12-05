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
    mode_t umask;               /* 由umask系统调用设置的掩码 */

    uid_t real_uid;		        /* 真实用户ID */
    uid_t eff_uid;		        /* 有效用户ID */
    gid_t real_gid;		        /* 真实组ID */
    gid_t eff_gid;	            /* 有效组ID */
    dev_t tty;		            /* 控制终端的(主要/次要)设备号 */
    
    pid_t pid;
} FSProcess;

FSProcess fsproc[NR_PROCS];

/* 标志值 */
#define NOT_SUSPENDED      0	/* process is not suspended on pipe or task ：进程未在管道或任务上挂起 */
#define SUSPENDED          1	/* process is suspended on pipe or task ：进程暂停在管道或任务上 */
#define NOT_REVIVING       0	/* process is not being revived ：进程没有恢复 */
#define REVIVING           1	/* process is being revived from suspension ：流程从暂停中恢复 */
#define PID_FREE	       0	/* process slot free ：无进程插槽 */
#define PID_SERVER	      (-0x328)/* process has become a server ：进程已成为服务器 */

#endif //_FSPROC_H
