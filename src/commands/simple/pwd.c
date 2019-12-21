/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/19.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 打印当前工作目录
 */

#include <stdio.h>
//#include <unistd.h>

static char* work_directory;

int main(int argc, char *argv[]){
    /* 真不好意思...flyanx1.0文件系统只有根目录... */
    work_directory = "/";
    printf("%s\n", work_directory);
//    exit(0);
    return 0;
}

