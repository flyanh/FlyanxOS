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
#include <unistd.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>

PRIVATE Message msg_in;

/* 本地函数声明 */
FORWARD _PROTOTYPE( void power_test, (void) );
FORWARD _PROTOTYPE(  void clock_test, (void) );
FORWARD _PROTOTYPE( void tty_test, (void) );


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
//    ok_print("Test task", "START TEST");

    /* 好的，我们的单元测试代码从这里开始！ */

    /* 电源模块测试 */
//    power_test();

    /* 时钟模块测试 */
//    clock_test();

    /* 终端任务模块测试 */
//    tty_test();

//    ok_print("Test task", "TEST PASS");
    /* 好了，现在可以阻塞自己了，不然系统将找不到运行下去的理由 */
    receive(ANY, &msg_in);
}

/*===========================================================================*
 *				tty_test				     *
 *			  终端任务模块测试
 *===========================================================================*/
PRIVATE void tty_test(void){
    int status;
    /* 测试终端打开 */
    msg_in.type = DEVICE_OPEN;
    status = send_receive(TTY_TASK, &msg_in);
    if(status == OK){
        printf("device open success: %d\n", msg_in.type);
    } else {
        printf("device open failed.\n");
    }

    /* 测试终端读取数据 */
    msg_in.type = DEVICE_READ;
    status = send_receive(TTY_TASK, &msg_in);
    if(status == OK){
        printf("device read success: %d\n", msg_in.type);
    } else {
        printf("device read failed.\n");
    }

    /* 测试终端写入数据 */
    msg_in.type = DEVICE_WRITE;
    status = send_receive(TTY_TASK, &msg_in);
    if(status == OK){
        printf("device write success: %d\n", msg_in.type);
    } else {
        printf("device write failed.\n");
    }


    /* 测试终端io控制 */
    msg_in.type = DEVICE_IOCTL;
    status = send_receive(TTY_TASK, &msg_in);
    if(status == OK){
        printf("device ioctl success: %d\n", msg_in.type);
    } else {
        printf("device ioctl failed.\n");
    }

    /* 测试终端关闭 */
    msg_in.type = DEVICE_CLOSE;
    status = send_receive(TTY_TASK, &msg_in);
    if(status == OK){
        printf("device close success: %d\n", msg_in.type);
    } else {
        printf("device close failed.\n");
    }

    /* 测试取消终端正在处理的任务 */
    msg_in.type = CANCEL;
    status = send_receive(TTY_TASK, &msg_in);
    if(status == OK){
        printf("device cancel success: %d\n", msg_in.type);
    } else {
        printf("device cancel failed.\n");
    }

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
    msg_in.type = GET_UPTIME;
    status = send_receive(CLOCK_TASK, &msg_in);
    if(status == OK){
        printf("get uptime success: %ld\n", msg_in.CLOCK_TIME);
    } else {
        printf("get uptime failed.\n");
    }

    /* 测试设置时钟运行时间（s）
     * 期望：status = OK
     */
    msg_in.type = SET_TIME;
    msg_in.CLOCK_TIME = 5;
    status = send_receive(CLOCK_TASK, &msg_in);
    if(status == OK){
        printf("set time success\n");
    } else {
        printf("set time failed.\n");
    }

    /* 测试得到时钟运行时间（s）
     * 期望：打印>get time success: 5
     */
    msg_in.type = GET_TIME;
    status = send_receive(CLOCK_TASK, &msg_in);
    if(status == OK){
        printf("get time success: %ld\n", msg_in.CLOCK_TIME);
    } else {
        printf("get time failed.\n");
    }

    /* 测试闹钟功能好不好使
     * 期望：alarm_handler在3s后被调用
     */
    msg_in.type = SET_ALARM;
    msg_in.CLOCK_PROC_NR = TEST_TASK;
    msg_in.DELTA_TICKS = second2ms(3) / 10;
    msg_in.FUNC_TO_CALL = alarm_handler;
    status = send_receive(CLOCK_TASK, &msg_in);
    if(status == OK){
        printf("set alarm success: %ld\n", msg_in.SECONDS_LEFT);
    } else {
        printf("set alarm failed.\n");
    }

    /* 测试给时钟任务一个错误的消息
     * 期望：Flyanx系统宕机报错
     */
//    msg.type = 66;
//    status = send(CLOCK_TASK, &msg);

}

/*===========================================================================*
 *				power_test				     *
 *			    电源模块测试
 *===========================================================================*/
PRIVATE void power_test(void){
    /* 测试：内核发生故障 */
//    panic("hello panic", NO_NUM);

    /* 测试关机，即关闭电源 */
//    printf("The system will shutdown after 3s...\n");
//    milli_delay(second2ms(3));
//    wreboot(RBT_HALT);
}




