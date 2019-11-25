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

    int t, task_count = 0, server_count = 0, usr_proc_count = 0;
    for(t = -NR_TASKS; t <= LOW_USER;++t){
        if(t < 0){  /* 任务 */
            task_count++;
        } else {    /* 服务或用户进程 */
            if(t < LOW_USER){
                /* 服务 */
                server_count++;
            } else{
                /* 用户进程 */
                usr_proc_count++;
            }
        }
    }
    printf("System task count: %d\n", task_count);
    printf("System server count: %d\n", server_count);
    printf("User process count: %d\n", usr_proc_count);
    printf("Current running process: %s, nr: %d.\n", curr_proc->name, curr_proc->nr);
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

