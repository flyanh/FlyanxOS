/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/22.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 回显一个字符串
 */

#include <stdio.h>

int main(int argc, char *argv[]){

    int i, len = 0;
    char *str, out[128];


    for(i = 1; i < argc; i++){  /* 从1开始，为了跳过文件名 */
        str = argv[i];
        while (*str){           /* 不要""和''号 */
            if(*str == '\'' || *str == '"'){
                str++;
                continue;
            }
            out[len++] = *str;
            str++;
        }
        out[len] = 0;       /* 字符串结束符 */
        printf("%s%s", i == 1 ? "" : " ", out); /* 输出 */
        len = 0;
    }
    printf("\n");

    return 0;
}

