/* PC (and AT) BIOS structure to hold disk parameters.  Under Minix, it is
 * used mainly for formatting.
 */

/**
 * 该文件被软盘系统任务使用
 * 注意：软盘任务和硬盘任务很相似，所以可以重点看硬盘任务即可。
 * 
 * 中文注释添加者：Flyan
 **/

#ifndef _DISKPARM_H
#define _DISKPARM_H
struct disk_parameter_s {
  char spec1;
  char spec2;
  char motor_turnoff_sec;
  char sector_size_code;
  char sectors_per_cylinder;
  char gap_length;
  char dtl;
  char gap_length_for_format;
  char fill_byte_for_format;
  char head_settle_msec;
  char motor_start_eigth_sec;
};
#endif /* _DISKPARM_H */
