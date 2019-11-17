/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/10.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 对Intel 8259中断控制器芯片进行初始化。
 */

#include "kernel.h"
#include "protect.h"

#define ICW1_AT         0x11	/* edge triggered, cascade, need ICW4 */
#define ICW1_PC         0x13	/* edge triggered, no cascade, need ICW4 */
#define ICW1_PS         0x19	/* level triggered, cascade, need ICW4 */
#define ICW4_AT         0x01	/* not SFNM, not buffered, normal EOI, 8086 */
#define ICW4_PC         0x09	/* not SFNM, buffered, normal EOI, 8086 */

/*==========================================================================*
 *				interrupt_init				    *
 *				中断初始化
 *==========================================================================*/
PUBLIC void interrupt_init(mine)
int mine;           /* 中断初始化的模式 */
{
    /* interrupt_init通过两个步骤来保证在初始化完成之前的任何中断都不会生效。第一步向每个中断控制器芯
     * 片中写入一个字节以使其无法响应外部中断。随后,用来访问设备相关的中断处理程序的表格中所有表项都填入一
     * 个例程的地址,该例程在收到一个伪中断时将打印出一条信息,它绝对没有任何副作用。其后,在每个I/O任务运行
     * 其自己的初始化代码时,这些表项逐个地被重填,以指向相应的中断处理例程。最后,每个系统任务复位中断控制
     * 器芯片中的一个二进制位以激活其自己的中断信号输入。
     */

    interrupt_lock();
    if(protected_mode){
        /* 对第一个端口写数据，以适应不同型号的计算机。 */
//        out_byte(INT_M_CTL, ps_mca ? ICW1_PS : ICW1_AT);

        /* 对参数mine进行测试，并将对Flyanx在BIOS ROM合适的数值写入该端口
         * 当退出内核时，可调用interrupt_init来恢复BIOS向量，这样便可以平滑的
         * 退回到引导监控程序。而mine参数是选择所使用的模式。
         * @TODO 未保存BIOS的中断向量，被自定义的中断向量覆盖了，暂实现不了该功能
         */
        // Master and Slave 8259, ICW1
        out_byte(INT_M_CTL, 0x11);
        out_byte(INT_S_CTL, 0x11);
        // Master 8259, ICW2. 设置 '主8259'　的中断入口地址为 0x20.
        out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0);
        // Slave 8259, ICW2. 设置 '从8259'　的中断入口地址为 0x28.
        out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8);
        // Master 8259, ICW3. IR2 对应 '从8259'.
        out_byte(INT_M_CTLMASK,	0x4);
        // Slave  8259, ICW3. 对应 '主8259' 的 IR2.
        out_byte(INT_S_CTLMASK,	0x2);
        // Master and Slave 8259, ICW4.
        out_byte(INT_M_CTLMASK,	0x1);
        out_byte(INT_S_CTLMASK,	0x1);

        // Master 8259, OCW1.  将所有中断关闭
        out_byte(INT_M_CTLMASK,	0xFF);
        // Slave  8259, OCW1.  将所有中断关闭
        out_byte(INT_S_CTLMASK,	0xFF);
    }

    /* 在这里，我们初始化所有的中断请求处理程序到中断请求处理程序表中 */
    int i;
    for(i = 0; i < NR_IRQ_VECTORS; i++){
        int_request_table[i] = spurious_irq;	// 默认的
    }

    ok_print("Open interrupt mechanism", "OK");
}

/*=========================================================================*
 *				put_irq_handler				   *
 *=========================================================================*/
PUBLIC void put_irq_handler(irq, handler)
int irq;    /* 请求 */
irq_handler_t handler;
{
    /* 注册一个中断处理例程
     *
     * 在初始化期间那些必须响应中断的任务调用它以将其自己的
     * 处理程序地址装入中断表，覆盖掉spurious_irq的地址。
     */

    // 如果中断请求irq不在正常范围内，内核报错并停止。
    if(irq < 0 || irq >= NR_IRQ_VECTORS){
        panic("invalid call to put_irq_handler", irq);
    }
    // 如果已经注册过了，不做任何事
    if(int_request_table[irq] == handler) return;

    // 如果中断请求处理表中对应位置未被初始化过，内核报错并停止
    if(int_request_table[irq] != spurious_irq){
        panic("attempt to register second irq handler for ieq", irq);
    }

    // 先关闭对应的中断
    disable_irq(irq);
    // 开始注册安装中断处理例程
    if(!protected_mode) {
        /* 不在保护模式，内核报错并停止，以后再让Flyanx内核支持没有保护模式的CPU */
        panic("your computer no support protect mode!", NO_NUM);
    }
    int_request_table[irq] = handler;
    int_request_used |= 1 << irq;       /* 设置这个中断请求的使用位图 */
}

/*=========================================================================*
 *				spurious_irq				   *
 *=========================================================================*/
PUBLIC int spurious_irq(irq)
int irq;
{
/* 默认的中断处理例程，，可能从未被调用。 */

    // 简单的显示一下即可
    printf("I am interrupt %d, i have appeared!\n", irq);
    return 0;	/* Leave interrupt masked */
}

