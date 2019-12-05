/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/4.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 设备表
 *
 * 该表由主要设备号索引。它提供了主要设备编号和处理它们的例程之间的链接。
 */

#ifndef _FS_DEV_H
#define _FS_DEV_H

#include <sys/dev.h>

typedef struct dev_drv_map {
    int driver_task_nr;  /* 驱动任务的编号 */
} DDMap;

/* 表本身在table.c中说明并置初值。ddmap表的定义不能同时包含在几个文件中，
 * 这就是为什么使用dev.h文件的原因。这里，dmap用extern而不是EXTERN声明。
 * dmap表提供了主设备号与相应任务之间的映射。
 */
extern DDMap ddmap[];

/* 读写一个磁盘扇区
 * 因为在文件系统中对rw_sector()的调用很相似（大多数参数都相同），
 * 所以我们使用宏定义可以使代码可读性更好。
 */
#define READ_SECT(dev, sect_nr) dev_io(DEVICE_READ, \
				       dev,				\
				       FS_PROC_NR,     \
				       fs_buffer,       \
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE \
				       )
#define WRITE_SECT(dev, sect_nr) dev_io(DEVICE_WRITE, \
                        dev,				\
				        FS_PROC_NR,     \
				        fs_buffer,       \
				        (sect_nr) * SECTOR_SIZE,		\
				        SECTOR_SIZE \
				       )

#endif //_FS_DEV_H
