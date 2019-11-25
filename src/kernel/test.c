/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/25.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 系统单元测试任务
 */

#include "kernel.h"
#include <signal.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>

PRIVATE Message msg;

/* 本地函数声明 */
FORWARD _PROTOTYPE(  void clock_test, (void) );


/*===========================================================================*
 *				test_task				     *
 *			    系统测试任务
 *===========================================================================*/
PUBLIC void test_task(void){
    /* 单元测试任务入口，系统初始化完毕将自动执行这里的代码。
     * 执行完毕后测试任务将会堵塞自己，你不必担心它会持续消
     * 耗系统资源。
     * 当然了，你可以把一个功能的多个测试封装到一个函数里，然
     * 后在这里调用，封装的函数放在本文件是一个不错的选择。
     */
    ok_print("Test task", "START TEST");

    /* 好的，我们的单元测试代码从这里开始！ */
    clock_test();

    ok_print("Test task", "TEST PASS");
    /* 好了，现在可以阻塞自己了，不然系统将找不到运行下去的理由 */
    receive(ANY, &msg);
}

/*===========================================================================*
 *				clock_test				     *
 *			    时钟模块测试
 *===========================================================================*/
PRIVATE void alarm_handler(int sig_no){
    printf("alarm up!\n");
}
PRIVATE void clock_test(void){
    int status;
    /* 测试延迟函数好不好使
     * 期望：延迟3s
     */
    milli_delay(second2ms(3));
    /* 测试得到时钟运行时间（滴答） */
    msg.type = GET_UPTIME;
    status = send_receive(CLOCK_TASK, &msg);
    if(status == OK){
        printf("get uptime success: %ld\n", msg.CLOCK_TIME);
    } else {
        printf("get uptime failed.\n");
    }
    /* 测试设置时钟运行时间（s）
     * 期望：status = OK
     */
    msg.type = SET_TIME;
    msg.CLOCK_TIME = 5;
    status = send_receive(CLOCK_TASK, &msg);
    if(status == OK){
        printf("set time success\n");
    } else {
        printf("set time failed.\n");
    }
    /* 测试得到时钟运行时间（s）
     * 期望：打印>get time success: 5
     */
    msg.type = GET_TIME;
    status = send_receive(CLOCK_TASK, &msg);
    if(status == OK){
        printf("get time success: %ld\n", msg.CLOCK_TIME);
    } else {
        printf("get time failed.\n");
    }
    /* 测试闹钟功能好不好使
     * 期望：alarm_handler在3s后被调用
     */
    msg.type = SET_ALARM;
    msg.CLOCK_PROC_NR = TEST_TASK;
    msg.DELTA_TICKS = second2ms(3) / 10;
    msg.FUNC_TO_CALL = alarm_handler;
    status = send_receive(CLOCK_TASK, &msg);
    if(status == OK){
        printf("set alarm success: %ld\n", msg.SECONDS_LEFT);
    } else {
        printf("set alarm failed.\n");
    }

}




