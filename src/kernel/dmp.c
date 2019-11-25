/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/19.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 此文件包含一些用于调试的转储例程。
 */

#include "kernel.h"
#include <flyanx/common.h>
#include "process.h"

/*===========================================================================*
 *				proc_dmp    				     *
 *				进程转储
 *===========================================================================*/
PUBLIC void proc_dmp(void)
{
    /* 为所有的进程显示基本的处理信息，包括在按下F1键时显示的内存使用信息。 */

    int t, proc_count = 0;
    printf("#\tNAME\t\tPRIORITY\n");
    for(t = -NR_TASKS; t <= LOW_USER;++t){
        Process *proc =  proc_addr(t);
        printf("%d\t%s\t\t%s\n", proc->nr, proc->name, "SYSTEM TASK");
        proc_count++;
    }
    printf("process count: %d\n", proc_count);
    printf("current running process: %s, nr: %d.\n", curr_proc->name, curr_proc->nr);
}

/*===========================================================================*
 *				map_dmp    				     *
 *===========================================================================*/
PUBLIC void map_dmp(void){
    printf("You computer's memory size is %ldKB.\n", (total_memory_size / (1024)) );
//    ARDS p_ards;
//    int i;
//    for(i = 0; i < NR_MEMORY_CLICK; i++){
//        p_ards = ards[i];
//        printf("%lX  ", p_ards.base_addr_low);
//        printf("%lX  ", p_ards.base_addr_high);
//        printf("%lX  ", p_ards.size_low);
//        printf("%lX  ", p_ards.size_high);
//        printf("%lX\n", p_ards.type);
//    }
}

