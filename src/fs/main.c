/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#include "fs.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>

/*===========================================================================*
 *				fs_main					     *
 *===========================================================================*/
PUBLIC void fs_main(void){
    Message msg;

    fs_print_info("working...\n");
    /* 文件系统开始工作了 */
    while (TRUE){
        receive(ANY, &msg);
    }
}


/*===========================================================================*
 *				fs_print_info					     *
 *				文件系统输出信息
 *===========================================================================*/
PUBLIC void fs_print_info(char *info){
    printf("{FS}-> %s", info);
}
