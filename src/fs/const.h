/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#ifndef _FS_CONST_H
#define _FS_CONST_H

/* 文件系统版本，伴随地flyanx v0.01发行，我将它命名为1.0，这是一个开始 */
#define FS_VERSION      "1.0"

/* 表的大小 */
//#define NR_

#define	NR_FILES	    128     /* 共享文件表中的插槽数量 */
#define	NR_INODES	    64      /* "在内核中"的索引节点表的插槽数量 */
#define	NR_SUPER_BLOCK	8       /* 超级块表的插槽数量 */
#define NR_LOCKS        8       /* 文件锁表的插槽数量 */

/* sizeof的类型可能是(unsigned)long。使用以下宏获取小对象的大小，
 * 这样就不会出现诸如将(small)long常量传递给需要int的例程的意外情况。
 */
#define u_sizeof(t)     ((unsigned) sizeof(t))

/* 文件系统魔数 */
#define SUPER_MAGIC     0x3EA   /* 超级块的魔数 */

/* 其他 */
#define SU_UID      ((uid_t) 0) /* 超级用户的uid */
#define SYS_UID     ((uid_t) 0) /* 系统uid，分配给其他服务器和起源进程 */
#define SYS_GID     ((gid_t) 0) /* 系统gid，分配给其他服务器和起源进程 */
//#define NORMAL              0   /* 强制get_block进行磁盘读取 */
//#define NO_READ             1   /* 禁止get_block进行磁盘读取 */
//#define PREFETCH            2   /*  */


#endif //_FS_CONST_H
