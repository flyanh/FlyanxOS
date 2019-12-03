/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件为IBM PC兼容机以及其磁盘分区提供支持。
 */

#include <ibm/partition.h>
#include <flyanx/partition.h>

#define	DIOCTL_GET_GEO	1
#define DIOCTL_SET_GEO  2

/* 硬盘驱动器 */
#define SECTOR_SIZE		512     /* 扇区大小（字节） */
#define SECTOR_BITS		(SECTOR_SIZE * 8)   /* 扇区大小（位） */
#define SECTOR_SIZE_SHIFT	9   /* 扇区大小移位 */
#define SECTOR_MASK     SECTOR_SIZE - 1 /* 扇区掩码（边界） */


/* 主要设备编号*/
#define	NO_DEV			    0
#define	DEV_FLOPPY		    1
#define	DEV_CDROM		    2
#define	DEV_HD			    3
#define	DEV_CHAR_TTY		4
#define	DEV_SCSI		    5
/* 根据主号和次号进行设备编号 */
#define	MAJOR_SHIFT		8
#define	MAKE_DEV(a,b)		((a << MAJOR_SHIFT) | b)
/* separate major and minor numbers from device number */
#define	MAJOR(x)		((x >> MAJOR_SHIFT) & 0xFF)
#define	MINOR(x)		(x & 0xFF)

#define	INVALID_INODE		0
#define	ROOT_INODE		1

#define	MAX_DRIVES		2
#define	NR_PART_PER_DRIVE	4
#define	NR_SUB_PER_PART		16
#define	NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define	NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)

