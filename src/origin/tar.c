/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/19.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 本文件可以对tar归档包进行解包操作，主要是为了给起源进程提供支持。
 * 在flyanx刚刚启动起来的时候，硬盘上有一个已经存在的cmd.tar归档包
 * ，里面有许多已经编译好的程序，现在只需要将它们解包出来到文件系统
 * 上即可。
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/dev.h>
#include <sys/stat.h>

/* A tar archive consists of 512-byte blocks.
   Each file in the archive has a header block followed by 0+ data blocks.
   Two blocks of NUL bytes indicate the end of the archive.  */

/* The fields of header blocks:
   All strings are stored as ISO 646 (approximately ASCII) strings.

   Fields are numeric unless otherwise noted below; numbers are ISO 646
   representations of octal numbers, with leading zeros as needed.

   linkname is only valid when typeflag==LNKTYPE.  It doesn't use prefix;
   files that are links to pathnames >100 chars long can not be stored
   in a tar archive.

   If typeflag=={LNKTYPE,SYMTYPE,DIRTYPE} then size must be 0.

   devmajor and devminor are only valid for typeflag=={BLKTYPE,CHRTYPE}.

   chksum contains the sum of all 512 bytes in the header block,
   treating each byte as an 8-bit unsigned value and treating the
   8 bytes of chksum as blank characters.

   uname and gname are used in preference to uid and gid, if those
   names exist locally.

   Field Name	Byte Offset	Length in Bytes	Field Type
   name		0		100		NUL-terminated if NUL fits
   mode		100		8
   uid		108		8
   gid		116		8
   size		124		12
   mtime	136		12
   chksum	148		8
   typeflag	156		1		see below
   linkname	157		100		NUL-terminated if NUL fits
   magic	257		6		must be TMAGIC (NUL term.)
   version	263		2		must be TVERSION
   uname	265		32		NUL-terminated
   gname	297		32		NUL-terminated
   devmajor	329		8
   devminor	337		8
   prefix	345		155		NUL-terminated if NUL fits

   If the first character of prefix is '\0', the file name is name;
   otherwise, it is prefix/name.  Files whose pathnames don't fit in that
   length can not be stored in a tar archive.  */
/* POSIX tar归档文件头（详细信息请看上面的英文）
 * 顾名思义，这个文件结构来自于POSIX标准，我们直接用就可以了。
 */
typedef struct posix_tar_header{
    /* 字节偏移量 */
    char name[100];		/*   0 */
    char mode[8];		/* 100 */
    char uid[8];		/* 108 */
    char gid[8];		/* 116 */
    char size[12];		/* 124 */
    char mtime[12];		/* 136 */
    char chksum[8];		/* 148 */
    char typeflag;		/* 156 */
    char linkname[100];	/* 157 */
    char magic[6];		/* 257 */
    char version[2];	/* 263 */
    char uname[32];		/* 265 */
    char gname[32];		/* 297 */
    char devmajor[8];	/* 329 */
    char devminor[8];	/* 337 */
    char prefix[155];	/* 345 */
                        /* 500 */
} TarHeader;

static char buf[SECTOR_SIZE * 16];
/*===========================================================================*
 *                            untar                                        *
 *                    提取一个tar归档文件并存储到文件系统上             *
 *===========================================================================*/
int untar(
        const char *filename,       /* 要提取的tar归档文件名称 */
        const char *parent_dir      /* 提取到哪个目录？ */
){
    /* 虽然这个函数支持给出解压目录，但flyanx1.0文件系统只有根目录，所以就算
     * 给了"/"以外的，只能导致失败，我只是在骗自己QAQ。
     */
    if(strcmp(parent_dir, "/") != 0) return -1;

    int fd;
    if((fd = open(filename, O_RDWR)) == -1){    /* 打开文件 */
        printf("extract file not exists.\n");
        return -1;
    }
    printf("[extract `%s'\n", filename);

    Stat fstat;
    int chunk = sizeof(buf);        /* 一块 = 8KB，用于读写 */
    int i = 0;
    int bytes = 0;

    while (1){
        bytes = read(fd, buf, SECTOR_SIZE);
        if(bytes != SECTOR_SIZE){
            return -1;      /* 一个TAR归档文件必须是512的字节的倍数 */
        }
        if(buf[0] == 0){
            if(i == 0){
                printf("    need not unpack the file.\n");
            }
            break;
        }
        i++;

        TarHeader *tar_hdr = (TarHeader *)buf;

        /* 计算文件大小 */
        char *p = tar_hdr->size;
        int file_size = 0;
        while (*p){
            file_size = (file_size * 8) + (*p++ - '0'); /* 以八进制算 */
        }
//        printf("file size: %d\n", file_size);
        int left = file_size;
        int out_fd = open(tar_hdr->name, O_RDWR | O_CREAT | O_TRUNC);

        if(out_fd == -1){       /* 打开并创建失败 */
            printf("    failed to extract file: %s\n", tar_hdr->name);
            printf(" aborted]\n");
            close(fd);
            return -1;
        }
        printf("    %s", tar_hdr->name);
        while (left){
            int io_bytes = chunk < left ? chunk : left;
            read(fd, buf,
                 ((io_bytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
            bytes = write(out_fd, buf, io_bytes);
            if(bytes != io_bytes){
                printf("untar faild\n");
                return -1;
            }
            left -= io_bytes;
            printf(".");
        }
        printf("\n");
        close(out_fd);
    }

    /* 标记这个文件已经被提取了 */
    if(i > 0){
        lseek(fd, 0, SEEK_SET);
        buf[0] = 0;
        bytes = write(fd, buf, 1);
        if(bytes != 1){
            return -1;
        }
    }

    close(fd);

    printf(" done, %d files extracted]\n", i);
    return i;
}

