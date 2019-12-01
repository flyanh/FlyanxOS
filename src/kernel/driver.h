/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * driver.h为所有的块设备驱动程序提供支持。
 */

#ifndef _KERNEL_DRIVER_H
#define _KERNEL_DRIVER_H

/* 我们需要一些头文件支持 */
#include <flyanx/common.h>
#include <flyanx/callnr.h>
#include "process.h"
#include <flyanx/partition.h>
#include <flyanx/u64.h>

/* 块设备驱动程序的结构定义
 * 保存了各驱动程序执行具体I/O操作的函数地址。
 */
typedef struct driver_s {
    _PROTOTYPE( char *(*name), (void) );
    _PROTOTYPE( int (*open), (struct driver_s *dp, struct message_s *m_ptr) );
    _PROTOTYPE( int (*close), (struct driver_s *dp, struct message_s *m_ptr) );
    _PROTOTYPE( int (*ioctl), (struct driver_s *dp, struct message_s *m_ptr) );
    _PROTOTYPE( struct driver_s *(*prepare), (int device) );
    _PROTOTYPE( int (*transfer), (int proc_nr, int opcode, off_t position,
            iovec_t *iov, unsigned nr_req) );
    _PROTOTYPE( void (*cleanup), (void) );
    _PROTOTYPE( void (*geometry), (struct partition_s *entry) );
} Driver;

/* 在达到64K边界之前可以DMA操作的字节数: */
#define dma_bytes_left(phys)    \
   ((unsigned) (sizeof(int) == 2 ? 0 : 0x10000) - (unsigned) ((phys) & 0xFFFF))

/* 块设备的结构定义
 * 保存了与分区相关的最主要信息：基地址和长度，它们都以字节为单位。
 * 采用这种格式使得对基于内存的设备（RAM盘）无需任何转换，由此最大
 * 程度地提高了响应速度。而对于真正的磁盘，由于有很多因素影响其存取
 * 速度，因而转换到扇区地址并不增添很多麻烦。
 */
typedef struct device {
    u64_t base;     /* 基地址 */
    u64_t size;     /* 长度 */
} Device;

/* driver.c驱动实现的公用函数原型 */

/* 磁盘驱动器的参数 */
#define SECTOR_SIZE      512	/* 物理扇区大小（以字节为单位） */
#define SECTOR_SHIFT       9	/* 用于划分 */
#define SECTOR_MASK      511	/* and remainder */

/* DMA缓冲区的大小（以字节为单位）。 */
#define DMA_BUF_SIZE    ( DMA_SECTORS * SECTOR_SIZE)
extern u8_t *tmp_buf;			/* DMA缓冲区 */

#endif //_KERNEL_DRIVER_H
