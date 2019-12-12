/* Description of entry in partition table.  */
#ifndef _PARTITION_H
#define _PARTITION_H

/* 定义了IBM兼容机上使用的硬盘分区表和相关的常量。
 * 该文件有助于将MINIX移植到其他硬件平台上。
 * 
 * 对于不同的硬件，include/ibm/partition.h必须可被替换为通常位于另一个相应目录
 * 下的partition.h文件。但在include/flyanx/partition.h中定义的结构是flyanx
 * 内部使用的，它应对各种硬件都保持不变。
 *
 * 这个文件是从固件设计人员那继承过来的。
 */

typedef struct part_entry {
  unsigned char bootind;	/* boot indicator 0/ACTIVE_FLAG	 */
  unsigned char start_head;	/* head value for first sector	 */
  unsigned char start_sec;	/* sector value + cyl bits for first sector */
  unsigned char start_cyl;	/* track value for first sector	 */
  unsigned char sysind;		/* system indicator		 */
  unsigned char last_head;	/* head value for last sector	 */
  unsigned char last_sec;	/* sector value + cyl bits for last sector */
  unsigned char last_cyl;	/* track value for last sector	 */
  unsigned long lowsec;		/* logical first sector		 */
  unsigned long size;		/* size of partition in sectors	 */
} PartEntry;

#endif /* _PARTITION_H */
