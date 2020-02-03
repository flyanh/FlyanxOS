/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/12/19.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 列出当前工作目录下的所有文件
 */

#include <sys/stat.h>
#include <stdio.h>

/* 文件列表
 *
 */
typedef struct file_list_s {
    char *name;
    struct file_list_s *next;
} FileList;

static char* work_directory;

static char getopt(int argc, char *argv[], char*opt, int nr_opt);
static void list(char *dir);
static void list_all(char *dir);
static void list_line(char *dir);

int main(int argc, char *argv[]){
    if(argc < 2){
        return -1;
    }
    work_directory = "/";
    char opt[10] = {'l', 'a'};    /* 现在支持的所有选项 */
    opt[2] = 0;
    char c;

    while (1){
        c = getopt(argc, argv, opt, 2); /* 得到选项 */
        switch (c){
            case 'l':
                list_line(work_directory);      /* 以行列出文件 */
                break;
            case 'a':
                list_all(work_directory);       /* 列出所有包括隐藏文件 */
                break;
            default:
                list(work_directory);           /* 普通列出 */
                break;
        }
    }

}

static void list(char *dir){

}

static void list_all(char *dir){

}

static void list_line(char *dir){
    Stat fstat;
}

/* 得到文件选项 */
static char getopt(int argc, char *argv[], char*opt, int nr_opt){
    int i = 0, j;
    char *str;

    for(i = 1; i < argc; i++){  /* 从1开始，为了跳过文件名 */
        str = argv[i];
        while (*str){           /* 找出带-的选项 */
            if(*str == '-'){
                str++;
                /* 判断选项我们支持吗？ */
                for (j = 0;j < nr_opt; j++){
                    if(*str == opt[j]) return *str; /* 返回选项 */
                }
                return -1;      /* 找不到，那么返回-1 */
            }
            str++;              /* 没找到之前都跳过 */
        }

    }
}
