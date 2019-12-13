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
    printf("\tprocess(all) information\n");
    printf("----------------------------------------------\n");
    printf("#\tpid\tname\t\tpriority\n");
    printf("----------------------------------------------\n");
    for(t = -NR_TASKS; t <= LOW_USER;++t){
        Process *proc =  proc_addr(t);
        printf("%d\t%d\t%s\t", proc->nr,proc->pid, proc->name);
        if(t < 0){  /* 任务 */
            printf("%s\n", "system task");
        } else {    /* 服务或用户进程 */
            if (t < LOW_USER) {
                /* 服务 */
                printf("\t%s\n", "system server");
            } else {
                /* 用户进程 */
                printf("\t%s\n", "user process");
            }
        }
        proc_count++;
    }
    printf("----------------------------------------------\n");

//    printf("why?!!!\n");
    printf("process count: %d\n", proc_count);
    printf("current running process: %s, nr: %d.\n", curr_proc->name, curr_proc->nr);
}

/*===========================================================================*
 *				map_dmp    				     *
 *===========================================================================*/
PUBLIC void map_dmp(void){
    //    SegDescriptor *sdp = &curr_proc->ldt[TEXT];
//    printf("*****TEXT*****\n");
//    printf("limit_low: %d, base_low: %d\n",
//          1, 2);
//    printf("base_middle: %d, access: %d, ", 3, 4);
//    printf("granularity: %d, base_high: %d.\n", 5, 6);

//    sdp = &curr_proc->ldt[DATA];
//    printf("*****DATA*****\n");
//    printf("limit_low: %d, base_low: %d, base_middle: %d, access: %d, granularity: %d, base_high: %d.\n",
//           sdp->limit_low, sdp->base_low, sdp->base_middle, sdp->access, sdp->granularity, sdp->base_high);
}

