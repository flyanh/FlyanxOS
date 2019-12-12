/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#ifndef _FS_GLOBAL_H
#define _FS_GLOBAL_H

/* 声明结构体 */
struct fs_process_s;
struct inode;

/* 当该文件被包含在定义了宏_TABLE的 table.c中时，宏EXTERN的定义被取消。 */
#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif

/* 文件系统全局变量 */
EXTERN struct fs_process_s *call_fp;        /* 一直指向调用者 */
EXTERN bool super_user;                     /* 超级用户？ */
EXTERN int suspended_count;                 /* 被挂起的进程数量（管道上） */
EXTERN int reviving;                        /* 正在恢复运行的进程数量（管道上） */
extern u8_t *fs_buffer;                     /* 文件系统使用的高速缓冲区首地址 */
extern int FS_BUFFER_SIZE;                  /* 高速缓冲区长度 */
EXTERN struct inode *root_inode;            /* 根索引节点 */

/* 通信 */
EXTERN Message fs_inbox;        /* 文件系统收件箱：收到的消息在这里 */
EXTERN Message fs_outbox;       /* 文件系统发件箱：发送的消息在这里 */
EXTERN int who;                 /* 谁发的信？ */
EXTERN int fs_call;             /* 系统调用号 */
EXTERN bool need_reply;         /* 此次调用需要回复吗? */
EXTERN char user_path[PATH_MAX]; /* 用户路径名称 */

/* 以下用于将结果返回给调用者 */
EXTERN int err_code;            /* 临时存储错误号 */
EXTERN int rdwt_stat;           /* 最后一个磁盘I/O请求的状态 */

/* 数据在其他地方初始化。 */
extern _PROTOTYPE (int (*fs_call_handlers[]), (void) ); /* 系统调用表 */
extern max_major;       /* 最大主设备（+1） */
extern char dot[2];   /* . 和 .. 具有特殊的意义 */
extern char dotdot[3];   /* 对search_dir的含义：无访问权限检查。 */


#endif //_FS_GLOBAL_H
