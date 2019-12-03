/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/30.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 */

#include "fly.h"
#include <flyanx/callnr.h>
#include <flyanx/common.h>

PRIVATE Message msg;

/*===========================================================================*
 *				fly_main					     *
 *===========================================================================*/
PUBLIC void fly_main(void){

    fly_print_info("working...\n");
    while (TRUE){
        receive(ANY, &msg);
    }
}

/*===========================================================================*
 *				fly_print_info					     *
 *				飞彦拓展管理器输出信息
 *===========================================================================*/
PUBLIC void fly_print_info(char *info){
    printf("{FLY}-> %s", info);
}
