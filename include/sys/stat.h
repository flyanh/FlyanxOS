/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#ifndef _STAT_H
#define _STAT_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

/* 文件状态信息 */
typedef struct stat {
    dev_t dev;          /* 设备号：文件属于哪个设备？包含主次设备号 */
    ino_t ino_num;      /* 索引节点号 */
    mode_t mode;        /* 文件模式，保护位等... */
    nlink_t nr_link;    /* 链接数量 */
    uid_t uid;          /* 文件属主id */
    short int gid;      /* 文件属组id */
    dev_t rdev;         /* 真实设备号，只包含主设备号 */
    off_t size;         /* 文件大小 */
    time_t atime;       /* 最后访问时间 */
    time_t mtime;       /* 最后修改时间 */
    time_t ctime;       /* 最后文件状态信息改动时间 */
} Stat;

/* 文件模式的传统掩码定义。 */
/* 仅对某些定义进行丑陋的转换是为了避免出现符号扩展，例如int为32位时出现S_IFREG !=（mode_t）S_IFREG。 */
#define S_IFMT  ((mode_t) 0170000)	    /* type of file ：文件类型 */
#define S_IFREG ((mode_t) 0100000)	    /* regular ：规则 */
#define S_IFBLK 0060000		            /* block special ：超级块 */
#define S_IFDIR 0040000  	            /* directory ：目录 */
#define S_IFCHR 0020000		            /* character special ：特殊字符设备 */
#define S_IFIFO 0010000		            /* this is a FIFO ：这是一个先进先出的管道文件 */
#define S_ISUID 0004000		            /* set user id on execution ：在执行时设置用户ID */
#define S_ISGID 0002000		            /* set group id on execution ：在执行时设置组ID */
/* 下一个保留供将来使用 */
#define S_ISVTX   01000		            /* save swapped text even after use ：即使使用后也保存交换的文本 */

/* 文件模式的POSIX掩码定义 */
#define S_IRWXU   00700		/* owner:  rwx------ ：属主权限 = rwx------ */
#define S_IRUSR   00400		/* owner:  r-------- ：属主权限 = r-------- */
#define S_IWUSR   00200		/* owner:  -w------- ：属主权限 = -w------- */
#define S_IXUSR   00100		/* owner:  --x------ ：属主权限 = --x------ */

#define S_IRWXG   00070		/* group:  ---rwx--- ：属组权限 = ---rwx--- */
#define S_IRGRP   00040		/* group:  ---r----- ：属组权限 = ---r----- */
#define S_IWGRP   00020		/* group:  ----w---- ：属组权限 = ----w---- */
#define S_IXGRP   00010		/* group:  -----x--- ：属组权限 = -----x--- */

#define S_IRWXO   00007		/* others: ------rwx ：其他人的权限 = ------rwx */
#define S_IROTH   00004		/* others: ------r-- ：其他人的权限 = ------r--  */
#define S_IWOTH   00002		/* others: -------w- ：其他人的权限 = -------w- */
#define S_IXOTH   00001		/* others: --------x ：其他人的权限 = --------x */

/* 以下宏测试文件模式（来自POSIX第5.6.1.1节）。 */
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)	/* is a reg file ：是一个reg文件 */
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)	/* is a directory ：是一个目录 */
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)	/* is a char spec ：是一个特殊字符设备 */
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)	/* is a block spec ：是一个超级块 */
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)	/* is a pipe/FIFO ：是一个管道/FIFO */

/* 函数原型 */
_PROTOTYPE( int mkdir, (const char *_path, Mode_t _mode) );
_PROTOTYPE( int stat, (const char *_name, Stat *_buf) );
_PROTOTYPE( int fstat, (int _fd, Stat *_buf) );

#endif //_STAT_H
