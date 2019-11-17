;================================================================================================
; 文件：asmconst.inc
; 作用：内核文件所需的常量定义
; 作者： Flyan
; 联系QQ： 1341662010
;================================================================================================
;================================================================================================
; 自定义的一些常量
;================================================================================================
LEVEL0_VECTOR   equ 43              ; 将任务提权到最高特权级 - 0级 系统调用向量
;================================================================================================
; 保护模式相关
;================================================================================================
P_STACKBASE     equ 0               ; 栈基地址
GSREG           equ P_STACKBASE     ; GS段寄存器
FSREG           equ GSREG + 4       ; ES段寄存器
ESREG           equ FSREG + 4       ; FS段寄存器
DSREG           equ ESREG + 4       ; DS段寄存器(数据段寄存器)
EDIREG          equ DSREG + 4       ; EDI寄存器
ESIREG          equ EDIREG + 4      ; ESI寄存器
EBPREG          equ ESIREG + 4      ; EBP寄存器
KERNELESPREG    equ EBPREG + 4      ; 内核ESP寄存器
EBXREG          equ KERNELESPREG + 4
EDXREG          equ EBXREG + 4
ECXREG          equ EDXREG + 4
EAXREG          equ ECXREG + 4
RETADR          equ EAXREG + 4
EIPREG          equ RETADR + 4
CSREG           equ EIPREG + 4
EFLAGSREG       equ CSREG + 4
ESPREG          equ EFLAGSREG + 4
SSREG           equ ESPREG + 4
P_STACKTOP      equ SSREG + 4       ; 栈顶
P_LDT_SEL       equ P_STACKTOP      ; LDT选择子
P_LDT           equ P_LDT_SEL + 4   ; LDT

TSS3_S_SP0      equ 4

INT_M_CTL       equ 0x20            ; I/O port for interrupt controller <Master>
INT_M_CTLMASK   equ 0x21            ; setting bits in this port disable ints <Master>
INT_S_CTL       equ 0xA0            ; I/O port for second interrupt controller <Salve>
INT_S_CTLMASK   equ 0xA1            ; setting bits in this port disable ints <Slave>

EOI             equ 0x20

; 以下选择子值必须与 portect.h 中保持一致！
SELECTOR_FLAT_C equ 0x08                ; LOADER 里面已经确定了的
SELECTOR_FLAT_RW    equ 0x10            ; LOADER 里面已经确定了的，可读写数据段
SELECTOR_TSS    equ 0x20                ; TSS. 从外层调到内存时 SS 和 ESP 的值从里面获得

SELECTOR_KERNEL_CS  equ SELECTOR_FLAT_C ; 内核代码段
SELECTOR_KERNEL_DS  equ SELECTOR_FLAT_RW ; 内核代码段
;================================================================================================
; 关于消息通信的一些参数
CLICK_SHIFT     equ     10


MSG_SIZE		equ	    9		; 消息的大小（32位机器中）
;================================================================================================
