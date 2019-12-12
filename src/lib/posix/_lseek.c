/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/12.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 提供给用户进程的posix调用：设置文件位置。
 */

#include <lib.h>
#define lseek   _lseek
#include <unistd.h>


PUBLIC off_t lseek(
        int fd,         /* 哪个文件？ */
        off_t offset,   /* 设置的相对偏移量 */
        int whence      /* 从哪开始设置？ */
){
    Message out;

    out.m2_i1 = fd;
    out.m2_l1 = offset;
    out.m2_i2 = whence;
    if(_syscall(FS, LSEEK, &out) < 0){
        return -1;  /* 失败 */
    }
    return (off_t)out.m2_l1;    /* 成功，返回新的文件位置 */
}
