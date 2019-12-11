/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/25.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 标准输入/输出 定义
 */

#ifndef _STDIO_H
#define _STDIO_H

#ifndef _ANSI_H
#include <ansi.h>
#endif

/* 以下定义也在<unistd.h>中，它们不应该发生冲突！ */


/* 关于文件的一些信息 */
#define FILENAME_LEN    17      /* 文件名最大长度 */

_PROTOTYPE( int printf, (const char *_fmt, ...)				);

#endif //STDIO_H
