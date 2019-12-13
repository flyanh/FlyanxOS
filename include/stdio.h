/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/25.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 标准输入/输出 定义
 * 这个文件基于POSIX标准，所以你没必要更改和在意它原有的内容，但是添加新内容是
 * 完全可以的。
 */

#ifndef _STDIO_H
#define _STDIO_H

#ifndef _ANSI_H
#include <ansi.h>
#endif


/* 标准输入/输出流（文件描述符） */
#define STDIN       0
#define STDOUT      1

/* 以下定义也在<unistd.h>中，它们不应该发生冲突！ */
/* "fseek"更改文件指针位置的三种操作，这些值不应被更改！  */
#define SEEK_SET	0	/* 从文件起点开始 */
#define SEEK_CUR	1	/* 从当前位置开始  */
#define SEEK_END	2	/* 从文件末尾开始  */

#define STR_DEFAULT_LEN 512

_PROTOTYPE( int vsprintf, (char *_buf, const char *_fmt, char *_args) );
_PROTOTYPE( int printf, (const char *_fmt, ...) );
_PROTOTYPE( int sprintf, (char *_buf, const char *_fmt, ...) );


#endif //STDIO_H
