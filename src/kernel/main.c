/* Copyright (C) 2007 Free Software Foundation, Inc.
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/9.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 该文件包含flyanx的主程序。
 *
 * 例程main()初始化系统，并通过设置进程表，中断向量和安排要运行的每个任务来初始化自身
 * 来开始运行flyanx内核。
 *
 * 该文件的入口点是：
 *  - main:         flyanx的主程序
 *  - panic:        由于致命错误而中止flyanx
 */

#include "kernel.h"
#include <signal.h>
#include <unistd.h>
#include <a.out.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "process.h"

void	hwint00();
void	hwint01();

/*===========================================================================*
 *                                   main                                    *
 *     Flyanx 内核的主体，从这里开始，我们的内核的90%可以由c语言开发了              *
 *===========================================================================*/
PUBLIC int main(){

    /* 主要做一些初始化工作，最重要的莫过于建立进程表 */

    disp_str("Flyanx Kernel started\n");

    /* 调用interrupt_init来初始化中断控制硬件
     * 该操作之所以放在这里是因为此前必须知道机器类型,因为完全依赖于硬件,所以该过程放在一个独立文件中。
     * 参数(1)，代表为Flyanx内核执行初始化，若是参数(0)则再次初始化硬件使其回到原始状态。
     *
     * 中断在这里初始化，那么则说明只有main函数真正执行起来，中断机制才能成功构建，如果在main函数之前
     * 产生了一个中断，那么将会没有效果。
     */
    interrupt_init(1);

    // 初始化内存


    while (TRUE){};
}

/*===========================================================================*
 *                                   panic                                   *
 *                              系统无法继续运行                               *
 *===========================================================================*/
PUBLIC void panic(msg, errno)
_CONST char *msg;
int errno;
{
    /* 当系统发现无法继续运行下去的故障时将调用它。典型的如无法读取一个很关键的数据块、
     * 检测到内部状态不一致、或系统的一部分使用非法参数调用系统的另一部分等。
     * 这里对printf的调用实际上是调用printk,这样当正常的进程间通信无法使用时核心仍能够
     * 在控制台上输出信息。
     * @TODO 还没有完成可以格式化输出的内核打印函数
     */
    if(msg != NULL){
        disp_str("\n");
        disp_str("Flyanx Kernel panic:");
        disp_str(msg);
    }

//    if(msg != NULL){
//        printf("\nFlyanx Kernel panic: %s", msg);
//        if(errno != NO_NUM) printf(" %d", errno);
//        printf("\n");
//    }
//    wreboot(RBT_PANIC);
}

/*===========================================================================*
 *                                   clear_screen                          *
 *                                     清屏                               *
 *===========================================================================*/
PUBLIC void clear_screen(){
    /* 现在屏幕实在是有点脏了，所以我们可以清理一下屏幕
     * 该例程仅限内核调试使用，用户自然有用户该使用的例程。
     */
    display_position = 0;
    int i = 0;
    for(i;i < 80 * 5; i++){
        disp_str(" ");
    }
    display_position = 0;
}


