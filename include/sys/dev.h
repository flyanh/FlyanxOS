/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件为IBM PC兼容机设备管理以及其磁盘分区提供支持。
 */

#ifndef _SYS_DEV_H
#define _SYS_DEV_H

#define	DIOCTL_GET_GEO	1
#define DIOCTL_SET_GEO  2

/* 硬盘驱动器 */
#define SECTOR_SIZE		512     /* 扇区大小（字节） */
#define SECTOR_BITS		(SECTOR_SIZE * 8)   /* 扇区大小（位） */
#define SECTOR_SIZE_SHIFT	9   /* 扇区大小移位 */
#define SECTOR_MASK     SECTOR_SIZE - 1 /* 扇区掩码（边界） */

#define	MAX_DRIVES		2
#define	NR_PART_PER_DRIVE	4
#define	NR_SUB_PER_PART		16
#define	NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define	NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)

/* 定义主分区的最大次要设备号。 如果有2个磁盘，prim_dev的范围为hd [0-9]，则此宏等于9。
 */
#define	MAX_PRIM		(MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)

#define	MAX_SUBPARTITIONS	(NR_SUB_PER_DRIVE * MAX_DRIVES)

/* 主要设备编号*/                 /* 0不存在 */
#define	DEV_FLOPPY		    1   /* 软盘驱动 */
#define	DEV_CDROM		    2   /* 光盘驱动 */
#define	DEV_HD			    3   /* AT硬盘驱动 */
#define	DEV_CHAR_TTY		4   /* 字符终端设备 */
#define	DEV_SCSI		    5   /* SCSI接口硬盘 */

/* 硬盘次设备号 */
#define	MINOR_hd1a		0x10
#define	MINOR_hd2a		(MINOR_hd1a + NR_SUB_PER_PART)
#define	MINOR_hd2c		(MINOR_hd1a + NR_SUB_PER_PART + 2)
/* boot的次设备号
 * 对应于src/boot/load.inc::ROOT_BASE，如果更改，请连同一起更改。
 */
#define	MINOR_BOOT			MINOR_hd2c

/* 根据主号和次号进行设备编号 */
#define	MAJOR_SHIFT		    8
#define	MAKE_DEV(a,b)		((a << MAJOR_SHIFT) | b)
#define	ROOT_DEV		MAKE_DEV(DEV_HD, MINOR_BOOT)

/* 这两个宏可以将主次设备号分开 */
#define	MAJOR(x)		((x >> MAJOR_SHIFT) & 0xFF)
#define	MINOR(x)		(x & 0xFF)

#define	P_PRIMARY	0
#define	P_EXTENDED	1

#define FLYANX_PART	102	    /* Flyanx的分区结构 */
#define NO_PART		    0x00	/* 未使用的条目 */
#define EXT_PART	    0x05	/* 扩展分区 */

#endif //_SYS_DEV_H
