/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/2.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 实现串处理函数，即头文件usr/include/string.h中的功能
 * 这是c语言实现的，有几个用汇编实现的在同级目录下的string.asm里。
 */


#include <string.h>

/*===========================================================================*
 *				strncmp					     *
 *			  比较size个字符
 *===========================================================================*/
int strncmp(
        register const char *s1,
        register const char *s2,
        register size_t n
){
    if (n) {
        do {
            if (*s1 != *s2++){
                break;
            }
            if (*s1++ == '\0'){
                return 0;
            }
        } while (--n > 0);

        if (n > 0) {
            if (*s1 == '\0') return -1;
            if (*--s2 == '\0') return 1;
            return (unsigned char) *s1 - (unsigned char) *s2;
        }
    }
    return 0;
}

/*=========================================================================*
 *				memcmp				   *
 *			  比较两个内存区域
 *=========================================================================*/
int memcmp(const void *s1, const void *s2, size_t n)
{
    if ((s1 == 0) || (s2 == 0)) { /* 健壮性 */
        return (s1 - s2);
    }

    register const char *p1 = s1;
    register const char *p2 = s2;
    int i;
    for (i = 0; i < n; i++,p1++,p2++) {
        if (*p1 != *p2) {
            return (*p1 - *p2);
        }
    }
    return 0;
}


