/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 这个文件只被内核的驱动引用，不被文件系统和内存管理器等服务器使用。
 */

#ifndef _FLYANX_PARTITION_H
#define _FLYANX_PARTITION_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

/* 分区结构 */
typedef struct partition_s {
    u64_t base;             /* 分区开始的字节偏移 */
    u64_t size;             /* 分区中的字节数 */
    unsigned cylinders;     /* 硬盘硬件参数 */
    unsigned heads;
    unsigned sectors;       /* 扇区 */
} Partition;

#endif //_FLYANX_PARTITION_H
