; 加载FlyanxOS的第二步: Loader　跳入保护模式，并加载真正的系统内核
; 这个 加载文件　突破了引导扇区 512 字节的限制(但还是在64K的限制内，但对我们完全够用)，我们可以在这里完成很多事情，所以较为重要
; 本文件通过硬盘加载，同目录下的loader.asm是软盘引导

org 0100h
    jmp LOAD_START      ; loader开始运行

;;==================================================================================
;; 导入一些信息
%include "load.inc"     ;; 加载内核所时需要的信息
%include "pm.inc"       ;; 保护模式支持
;;==================================================================================

TRANS_SECT_NR       equ     2
SECT_BUFFER_SIZE    equ     TRANS_SECT_NR * 512

;; 对磁盘地址进行打包，然后送到超级块存储地址上
disk_address_packet:    db 0x10                 ;; [ 0] 磁盘地址大小（字节）
                        db 0                    ;; [ 1] 保留，必须为0
sect_count:             db TRANS_SECT_NR        ;; [ 2] 多少块要被传输
                        db 0                    ;; [ 3] 保留使用，必须为0
                        dw KERNEL_OFFSET   ;; [ 4] 传输偏移地址
                        dw KERNEL_SEG     ;; [ 6] 缓冲段地址，即要传输到的地方
lba_addr:               dd 0                    ;; [ 8] LBA低32位
                        dd 0                    ;; [12] LBA高32位

;;==================================================================================
; GDT全局描述符表相关信息和其他信息
; GDT全局描述符表--------------------------------------------------------------------------------------------
;                                   段基址           段界限           属性
LABEL_GDT:          Descriptor      0,              0,              0                               ; <- 空描述符，必要的!没有这个CPU无法识别GDT
LABEL_DESC_FLAT_C:  Descriptor      0,              0FFFFFh,        DA_CR | DA_32 | DA_LIMIT_4K     ; 0~4G，32位可读代码段
LABEL_DESC_FLAT_RW: Descriptor      0,              0FFFFFh,        DA_DRW | DA_32 | DA_LIMIT_4K    ; 0~4G，32位可读写数据段
LABEL_DESC_VIDEO:   Descriptor      0b8000h,        0FFFFFh,        DA_DRW | DA_DPL3              	; 显存首地址，特权级3
; GDT全局描述符表--------------------------------------------------------------------------------------------
GDTLen      equ $ - LABEL_GDT                       ; GDT长度
GDTPtr      dw  GDTLen - 1                          ; 段界限
            dd  LOADER_PHY_ADDR + LABEL_GDT     ; 基地址
; GDT选择子-------------------------------------------------------------------------------------------------
SelectorFlatC       equ LABEL_DESC_FLAT_C   -   LABEL_GDT               ; 32位可读代码段选择子
SelectorFlatRW      equ LABEL_DESC_FLAT_RW  -   LABEL_GDT               ; 32位可读写数据段选择子
SelectorVideo       equ LABEL_DESC_VIDEO    -   LABEL_GDT | SA_RPL3     ; 显存首地址选择子，特权级3
; GDT选择子-------------------------------------------------------------------------------------------------
BaseOfStack         equ 0100h                       ; 基栈
;;==================================================================================
error_handler:
    mov dh, 5
    call read_mode_print        ; print("ERROR 0   ");
    jmp $
;;==================================================================================
LOAD_START:                     ; <-- 从这里开始加载
    mov ax, cs
    mov ds, ax
    mov ds, ax
    mov ss, ax
    mov sp, BaseOfStack

    mov dh, 0
    call read_mode_print        ; print("Loading....");

    ; 得到内存大小
    mov ebx, 0                  ; ebx = 后续内存值，开始为0
    mov di, _MemChkBuf          ; es:di指向一个地址范围描述符结构(Address Range Descriptor Structure)
.mem_chk_loop:
    mov eax, 0xe820             ; eax = 0000e820
    mov ecx, 20                 ; ecx = 地址范围描述符结构大小
    mov edx, 0x534d4150         ; edx = 'SMAP'
    int 0x15                    ; int 0x15
    jc .mem_chk_fail		        ; 如果产生了进位，即CF = 1，跳转到.mem_chk_fail
    add di, 20
    inc dword [_dwMCRNumber]	; _dwMCRNumber = ARDS　的个数
    cmp ebx, 0
    jne .mem_chk_loop		; ebx != 0，继续进行循环
    jmp .mem_chk_ok		    ; ebx == 0，得到内存数OK
.mem_chk_fail:
    mov dword [_dwMCRNumber], 0
.mem_chk_ok:
    ;; 获取根目录'/'的扇区号（ROOT_INODE），它将存储到eax寄存器中
    mov eax, [fs:SB_ROOT_INODE]     ; eax = [fs:SB_ROOT_INODE]
    call get_inode                  ; get_inode(eax);

    ;; 读取'/'到ex:bx
    mov dword [disk_address_packet + 8], eax
    call read_sector

    ;; 现在我们开始搜索'/kernel.bin'文件
    mov si, kernel_file_name
    push bx     ; 保存bx
.str_cmp:
    ;; --> 表示指向什么； ==> 表示数据传向哪里
    ;; 比较之前：
    ;;  es:bx --> 硬盘的目录项
    ;;  es:si --> 我们想要的文件名
    add bx, [fs:SB_DIR_ENT_NAME_OFF]
.1:
    lodsb               ;; ds:si ==> al
    cmp byte al, [es:bx];; 比较当前文件名和我们想要的文件名（一个字符）
    jz .2
    jmp .different      ;; 我去，不一样
.2:                     ;; 很好，还没碰到不一样的字符
    cmp al, 0
    jz .found_it        ;; 前面都匹配了，而且到达了字符串结尾'\0'，我们找到它了
    inc bx              ;; 目录项的下一个字符
    jmp .1              ;; 继续比较
.different:
    pop bx              ;; -> 恢复bx
    add bx, [fs:SB_DIR_ENT_SIZE]
    sub ecx, [fs:SB_DIR_ENT_SIZE]
    jz .no_fount_it         ;; 没找到它

    push bx
    mov si, kernel_file_name
    jmp .str_cmp            ;; 问题不大，拿到硬盘下一个目录项，继续比较
.no_fount_it:               ;; 找不到，打印信息并死机
    mov dh, 3
    call read_mode_print    ; print("No KERNEL...");
    jmp $
.found_it:
    pop bx
    add bx, [fs:SB_DIR_ENT_INODE_OFF]
    mov eax, [es:bx]            ;; eax = Loader文件的索引节点号
    call get_inode              ;; get_inode(); //得到Loader文件的起始扇区，放在eax中
    mov dword [disk_address_packet + 8], eax
load_kernel2mem:                    ;; 找到内核文件了！我们将它载入内存
    call read_sector
    cmp ecx, SECT_BUFFER_SIZE
    jl .done
    sub ecx, SECT_BUFFER_SIZE   ; 载入剩余字节 -= 扇区缓冲区大小
    add word [disk_address_packet + 4], SECT_BUFFER_SIZE    ;; 传输缓冲区
    jc .1
    jmp .2
.1:
    add word [disk_address_packet + 6], 0x1000
.2:
    add dword [disk_address_packet + 8], TRANS_SECT_NR      ;; LBA
    jmp load_kernel2mem     ;; 还剩呢，继续加载
.done:
    mov dh, 2
    call read_mode_print        ; real_mode_print(“in HD LOADER”);

;;==================================================================================
; 通过上面的操作，我们已经将内核加载进入内存，接下来我们准备跳入保护模式

	; 1 首先，进入保护模式必须有GDT全局描述符表，我们加载GDTR
	lgdt [GDTPtr]

	; 2 由于保护模式中断处理的方式和实模式不一样，所以我们需要先关闭中断，否则会引发错误
	cli

	; 3 打开 A20 地址线，不打开也可以进入，但是会影响寻址能力，你也不希望自己的系统被限制吧？
	in al, 92h
	or al, 00000010b
	out 92h, al

	; 4 设置cr0寄存器上的保护模式标志为1，准备切换到保护模式
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	; 5 真正进入保护模式！前面的4步已经完成了保护模式所需的所有东西
	; 	现在只需要跳入到一个32位代码段就可以真正进入保护模式了！
	;	在保护模式下，你可以获得32位CPU给你带来的所有功能，不必要再忍受16位实模式的各种限制！
	jmp dword SelectorFlatC:(LOADER_PHY_ADDR + LAB_PM_START)

	jmp $       ; 如果前面的工作顺利，这行代码将永远不可能执行

;;==================================================================================
; 变量
w_secotr_nr         dw  0       ; 要读取的扇区号
bOdd                db  0       ; 奇数还是偶数
dw_kernel_size      dd  0       ; kernel.bin 内核文件的大小
;;==================================================================================
; 字符串
kernel_file_name    db  "kernel.bin", 0 ; 内核文件名
; 为简化代码, 下面每个字符串的长度均为 MessageLength
MessageLength		equ	12
LoadMessage:	    db	    "Loading....."  ; 12字节. 序号 0
Message1		    db	    "            "  ; 12字节. 序号 1
Message2		    db	    "in HD LOADER"  ; 12字节. 序号 2
Message3            db      "No KERNEL..."  ; 12字节，序号 3
Message4            db      "Too Large..."  ; 12字节，序号 4
Message5            db      "ERROR 0     "  ; 12字节，序号 5
;;==================================================================================
;;==================================================================================
;;                       实模式下清屏
;;==================================================================================
clear_screen:
    mov ax, 0x600   ; AH = 6, AL = 0
    mov bx, 0x700   ; 黑底白字(BL = 0x7)
    mov cx, 0       ; 左上角：(0, 0)
    mov dx, 0x184f  ; 右下角：(80, 50)
    int 0x10
    ret
;;==================================================================================
;;                       实模式下打印一个字符串
;;==================================================================================
read_mode_print:
    mov ax, MessageLength
    mul dh
    add ax, LoadMessage
    mov bp, ax      ;;
    mov ax, ds      ;; es:bp指向串地址
    mov es, ax      ;;
    mov cx, MessageLength   ; cx = 串长度
    mov ax, 0x1301          ; ah = 0x13, al = 0x1
    mov bx, 0x7             ; 页号为0（bh = 0） 黑底白字(bl = 0x7)
    mov dl, 0
    int 0x10                ; int 0x10
    ret
;;==================================================================================
;;                       读取一个扇区
;;==================================================================================
;; @param entry 扇区条目
;;  - 字段disk_address_packet应该在调用本例程之前填写它
;; @return es:bx
;;  - 将要读取的数据
;; @change
;;  - eax, ebx, dl, si, es
read_sector:
    xor ebx, ebx

    mov ah, 0x42
    mov dl, 0x80
    mov si, disk_address_packet
    int 0x13

    mov ax, [disk_address_packet + 6]
    mov es, ax
    mov bx, [disk_address_packet + 4]

    ret
;;==================================================================================
;;                       得到一个索引节点
;;==================================================================================
;; @params eax
;;  - 索引节点号
;; @return
;;  - eax   : 扇区号
;;  - ecx   : 文件大小
;;  - es:ebx: 索引节点扇区缓冲区
;; @change
;;  - eax, ebx, ecx, edx
get_inode:
    dec eax             ; eax = 索引节点号 - 1
    mov bl, [fs:SB_INODE_SIZE]
    mul bl              ; eax = (索引节点号 - 1) * INODE_SIZE
    mov eax, SECT_BUFFER_SIZE
    sub edx, dword [fs:SB_INODE_SIZE]
    cmp eax, edx
    jg error_handler
    push eax

    mov ebx, [fs:SB_NR_IMAP_SECTS]
    mov edx, [fs:SB_NR_SMAP_SECTS]
    lea eax, [ebx + edx + ROOT_BASE + 2]
    mov dword [disk_address_packet + 8], eax
    call read_sector

    pop eax         ; [es:ebx + eax] --> 操作的索引节点

    mov edx, dword [fs:SB_INODE_SIZE_OFF]
    add edx, ebx
    add edx, eax        ; [es:edx] --> inode.size
    mov ecx, [es:edx]   ; ecx = inode.size

    ;; es:[ebx+eax] --> inode.start_sect
    add ax, word [fs:SB_INODE_START_OFF]

    add bx, ax
    mov eax, [es:bx]
    add eax, ROOT_BASE  ; eax = inode.start_sect
    ret

;;==================================================================================
;; 32位代码段！！！运行在保护模式下
;;==================================================================================
[SECTION .s32]
ALIGN	32
[BITS	32]

LAB_PM_START:	; 程序开始
	; 寄存器归位
	mov ax, SelectorVideo
	mov gs, ax				; 我们用 gs 寄存器放置显存首地址，以便使用
	mov ax, SelectorFlatRW
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov esp, TopOfStack

	; 打印显示内存信息
	call DispMemInfo
	; 启动分页机制，分页机制能让我们将所有的物理地址看做是一维的线性空间
    call SetupPaging
	; 初始化内核
	call InitKernel

    ; 进入内核代码前，我们将一些启动参数保存好，内核可以很方便的获取到它们
    mov dword [BOOT_PARAM_ADDR], BOOT_PARAM_MAGIC ; 魔数
    mov eax, [dwMemSize]
    mov [BOOT_PARAM_ADDR + 4], eax  ; 内存大小
    mov eax, KERNEL_SEG
    shl eax, 4
    add eax, KERNEL_OFFSET
    mov [BOOT_PARAM_ADDR + 8], eax  ; 内核所在的物理地址

    ;*********************************************************************************
	; 正式进入内核，Loader将CPU控制权转交给内核，至此，Loader的使命也结束了！比Boot厉害吧！
	; 从这里的函数运行成功后，我们才真正算是进入编写操作系统的门槛
	jmp SelectorFlatC:KERNEL_ENTRY_POINT_PHY_ADDR
	;*********************************************************************************
	; 而在此时，内存看上去是这样的：
	;              ┃                                    ┃
    ;              ┃                 .                  ┃
    ;              ┃                 .                  ┃
    ;              ┃                 .                  ┃
    ;      501000h ┃                                    ┃ <- 其他进程的数据，应该从501000以上开始
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;      500FFFh ┃■■■■■■■■■■■■■■■■■■┃  4GB ram needs 4MB  for page tables: [101000h, 501000h)
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┣■■■■■■■■■■■■■■■■■■┫
    ;      108FFFh ┃■■■■■■■■■■■■■■■■■■┃ 32MB ram needs 32KB for page tables: [101000h, 109000h)
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■Page  Tables■■■■■■┃
    ;              ┃■■■■■(大小由LOADER决定)■■■■┃
    ;    00101000h ┃■■■■■■■■■■■■■■■■■■┃ PAGE_TBL_BASE
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;    00100000h ┃■■■■Page Directory Table■■■■┃ PAGE_DIR_BASE  <- 1M
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃□□□□□□□□□□□□□□□□□□┃
    ;       F0000h ┃□□□□□□□System ROM□□□□□□┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃□□□□□□□□□□□□□□□□□□┃
    ;       E0000h ┃□□□□Expansion of system ROM □□┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃□□□□□□□□□□□□□□□□□□┃
    ;       C0000h ┃□□□Reserved for ROM expansion□□┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃□□□□□□□□□□□□□□□□□□┃ B8000h ← gs
    ;       A0000h ┃□□□Display adapter reserved□□□┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃□□□□□□□□□□□□□□□□□□┃
    ;       9FC00h ┃□□extended BIOS data area (EBDA)□┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;       90000h ┃■■■■■■■LOADER.BIN■■■■■■┃ somewhere in LOADER ← esp
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;       70000h ┃■■■■■■■KERNEL.BIN■■■■■■┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃ 7C00h~7DFFh : BOOT 向量, 将会被内核覆盖
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;              ┃■■■■■■■■■■■■■■■■■■┃
    ;        1000h ┃■■■■■■■■KERNEL■■■■■■■┃ 1000h ← KERNEL 入口 (KERNEL_ENTRY_POINT_PHY_ADDR)
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;        700h  ┃Boot Params                         ┃
    ;              ┃                                    ┃
    ;         500h ┃              F  R  E  E            ┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃□□□□□□□□□□□□□□□□□□┃
    ;         400h ┃□□□□ROM BIOS parameter area □□┃
    ;              ┣━━━━━━━━━━━━━━━━━━┫
    ;              ┃◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇┃
    ;           0h ┃◇◇◇◇◇◇Int  Vectors◇◇◇◇◇◇┃
    ;              ┗━━━━━━━━━━━━━━━━━━┛ ← cs, ds, es, fs, ss
    ;
    ;
    ;		┏━━━┓		┏━━━┓
    ;		┃■■■┃ 我们使用 	┃□□□┃ 不能使用的内存
    ;		┗━━━┛		┗━━━┛
    ;		┏━━━┓		┏━━━┓
    ;		┃      ┃ 未使用空间	┃◇◇◇┃ 可以覆盖的内存
    ;		┗━━━┛		┗━━━┛
    ;
    ; 注：KERNEL 的位置实际上是很灵活的，可以通过同时改变 LOAD.INC 中的 KERNEL_ENTRY_POINT_PHY_ADDR 和 MAKEFILE 中参数 -Ttext 的值来改变。
    ;     比如，如果把 KERNEL_ENTRY_POINT_PHY_ADDR 和 -Ttext 的值都改为 0x400400，则 KERNEL 就会被加载到内存 0x400000(4M) 处，入口在 0x400400。
    ;

;================================================================================================
; 显示 AL 中的数字
; ------------------------------------------------------------------------
DispAL:
	push ecx
	push edx
	push edi

	mov edi, [dwDispPos]	; 得到显示位置

	mov ah, 0Fh		; 0000b: 黑底	1111b: 白字
	mov dl, al
	shr al, 4
	mov ecx, 2
.begin:
	and al, 01111b
	cmp al, 9
	ja	.1
	add al, '0'
	jmp	.2
.1:
	sub al, 0Ah
	add al, 'A'
.2:
	mov [gs:edi], ax
	add edi, 2

	mov al, dl
	loop .begin

	mov [dwDispPos], edi	; 显示完毕后，设置新的显示位置

	pop edi
	pop edx
	pop ecx

	ret

;================================================================================================
; ------------------------------------------------------------------------
; 显示一个整形数
; ------------------------------------------------------------------------
DispInt:
    mov	ah, 0Fh			; 0000b: 黑底    1111b: 白字
    mov	al, '0'
    push	edi
    mov	edi, [dwDispPos]
    mov	[gs:edi], ax
    add edi, 2
    mov	al, 'x'
    mov	[gs:edi], ax
    add	edi, 2
    mov	[dwDispPos], edi	; 显示完毕后，设置新的显示位置
    pop edi

	mov	eax, [esp + 4]
	shr	eax, 24
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 16
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 8
	call	DispAL

	mov	eax, [esp + 4]
	call	DispAL

	ret

;================================================================================================
; 显示一个字符串
; ------------------------------------------------------------------------
DispStr:
	push ebp
	push ebx
	push esi
	push edi

	mov esi, [ebp + 8]		; 显示字符串的基地址
	mov edi, [dwDispPos]	; 得到显示位置
	mov ah, 0Fh				; 0000b: 黑底	1111b: 白字
.1:
	lodsb
	test al, al
	jz	.2
	cmp al, 0Ah		; 是回车符吗？
	jnz .3
	push eax
	mov eax, edi
	mov bl, 160
	div bl
	and eax, 0FFh
	inc eax
	mov bl, 160
	mul bl
	mov edi, eax
	pop eax
	jmp .1
.3:
	mov [gs:di], ax
	add edi, 2
	jmp .1
.2:
	mov [dwDispPos], edi	; 显示完毕后，设置新的显示位置

	pop edi
	pop esi
	pop ebx
	pop ebp
	ret

;================================================================================================
; ------------------------------------------------------------------------
; 换行
; ------------------------------------------------------------------------
DispReturn:
	push	szReturn
	call	DispStr			;printf("\n");
	add	esp, 4
	ret
;================================================================================================
; ------------------------------------------------------------------------
; 打印三个空格
; ------------------------------------------------------------------------
DispThreeSpace:
	push	szThreeSpace
	call	DispStr			;printf("   ");
	add	esp, 4
	ret

;================================================================================================
; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
MemCpy:
	push ebp
	mov ebp, esp

	push esi
	push edi
	push ecx

	mov edi, [ebp + 8]	; edi -> 目的地地址
	mov esi, [ebp + 12]	; esi -> 源地址（要复制的数据首地址）
	mov ecx, [ebp + 16]	; ecx <- 计数器（复制的数据大小）
.1:
	cmp ecx, 0			; 判断计数器
	jz	.2				; if(计算器 == 0); jmp .2

	mov al, [ds:esi]		; @
	inc esi					; #
							; #-> 逐字节移动并复制
	mov byte [es:edi], al	; #
	inc edi					; @

	dec ecx				; 计数器--
	jmp	.1				; 循环
.2:
	mov eax, [ebp + 8]	; 设置返回值

	pop ecx
	pop edi
	pop esi
	mov esp, ebp
	pop ebp

	ret

;================================================================================================
; 显示内存信息 --------------------------------------------------------------
DispMemInfo:
	push esi
	push edi
	push ecx

	mov esi, MemChkBuf		; for(int i = 0; i < [MCRNumber]; i++)	// 每次得到一个ARDS(Address Range Descriptor Structure)结构
	mov ecx, [dwMCRNumber]	; {
.loop:						; 	for(int j = 0; j < 5; j++)	// 每次得到一个ARDS中的成员，共5个成员
	mov edx, 5				; 	{	// 依次显示：BaseAddrLow, BaseAddrHigh, LengthLow, LengthHigh, Type
	mov edi, ARDStruct		;
.1:							;
	push dword [esi]		;
	pop eax					;
	stosd					;		ARDStruct[j * 4] = MemChkBuf[j * 4];
	add esi, 4				;
	dec edx					;
	cmp edx, 0				;
	jnz	.1					;	}
	call DispReturn			;	printf("\n");
	cmp dword [dwType], 1	;	if(Type == AddressRangeMemory)	// AddressRangeMemory : 1, AddressRangeReserved : 2
	jne	.2					;	{
	mov eax, [dwBaseAddrLow];
	add eax, [dwLengthLow]	;
	cmp eax, [dwMemSize]	;		if(BaseAddrLow + LengthLow > MemSize)
	jb	.2					;		{
	mov [dwMemSize], eax	;			MemSize = BaseAddrLow + LengthLow;
.2:							;		}
	loop .loop				;	}
							; }
	mov dword [dwDispPos], (160 * 5 + 2 * 30)	; 第 5 行 第 30 列

	push szRAMSize			;
	call DispStr			;
	add esp, 4				; printf("RAM size:");
							;
	call dispMemSize		; dispMemSize(); // 将内存转换为KB显示并显示

	pop ecx
	pop edi
	pop esi
	ret
;================================================================================================
; 换算内存byte到MB --------------------------------------------------------------
dispMemSize:
    mov ebx, [dwMemSize]    ; ebx = 内存大小（字节）
    xor edx, edx
    mov eax, ebx
    mov ecx, 1024
    div ecx                  ; 内存大小 / 1024

    push eax
    call DispInt			; DispInt(MemSize);
    add esp, 4

    push szMSizeKb          ; 显示内存大小单位KB
    call DispStr
    add esp, 4

    ret
;================================================================================================
; 启动分页机制 --------------------------------------------------------------
SetupPaging:
	; 根据内存的大小来计算应初始化多少的PDE以及多少页表，我们给每页分4K大小(32位操作系统一般分4K，Windows 32 即如此)
	xor edx, edx
	mov eax, [dwMemSize]	; eax <- 内存总大小
	mov ebx, 400000h		; 400000h = 4M = 4096 * 1024, 一个页表对应的内存大小
	div ebx					; 内存总大小　/ 4M
	mov ecx, eax			; 此时 ecx 为页表的个数，页即 PDE 应该的页数
	test edx, edx
	jz	.no_remainder		; if(edx == 0) jmp .no_remainder
	inc ecx					; else ecx++
.no_remainder:
	push ecx				; 暂存页表个数

	; 现在为简化处理，所有线性地址对应相等的物理地址，并且暂不考虑内存空洞。

	; 首先，初始化页目录
	mov ax, SelectorFlatRW
	mov es, ax
	mov edi, PAGE_DIR_BASE	; 此段首地址为 PAGE_DIR_BASE
	xor eax, eax
	mov eax, PAGE_TABLE_BASE | PG_P | PG_US_U | PG_RW_W
.1:
	stosd
	add eax, 4096			; 为简化，所有页表在内存中是连续的
	loop .1

	; 再初始化所有页表
	pop eax 				; 页表个数
	mov ebx, 1024			; 每个页表 1024 个 PTE
	mul ebx
	mov ecx, eax			; PTE个数 = 页表个数 * 1024
	mov edi, PAGE_TABLE_BASE	; 此段首地址为 PAGE_TABLE_BASE
	xor eax, eax
	mov eax,  PG_P | PG_US_U | PG_RW_W
.2:
	stosd
	add eax, 4096			; 每一页指向 4K 的空间
	loop .2

	; 最后，设置cr3和cr0，开启分页机制
	mov eax, PAGE_DIR_BASE
	mov cr3, eax
	mov eax, cr0
	or	eax, 80000000h
	mov cr0, eax
	jmp short .3
.3:
	nop
	ret

;================================================================================================
; InitKernel ---------------------------------------------------------------------------------
; 将 KERNEL.BIN 的内容经过整理对齐后放到新的位置
; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
; --------------------------------------------------------------------------------------------
InitKernel:
	xor esi, esi
	mov cx, word [KERNEL_PHY_ADDR + 2Ch]	; @
	movzx ecx, cx									; @-> ecx <- pELFHdr -> e_phnum
	mov esi, [KERNEL_PHY_ADDR + 1Ch]		; esi <- pELFHdr ->e_phoff
	add esi, KERNEL_PHY_ADDR				; esi <- OffsetOfKernel + pELFHdr -> e_phoff
.Begin:
	mov eax, [esi + 0]
	cmp eax, 0 				; PT_NULL
	jz	.NoAction
	push dword [esi + 010h]				; @
	mov eax, [esi + 04h]				; #
	add eax, KERNEL_PHY_ADDR	; # memcpy( (void*))(pPhdr->p_vaddr),
	push eax							; #			uchCode + pPHdr->p_offset,
	push dword [esi + 08h]				; #			pPHdr->p_filesz
	call MemCpy							; #		   );
	add esp, 12							; @
.NoAction:
	add esi, 020h						; esi += pELFHdr->e_phentsize
	dec ecx
	jnz .Begin

	ret

;================================================================================================
; 32位数据段
[SECTION .data1]
ALIGN	32
LABEL_DATA:
; 实模式下需要使用这些符号
; 字符串
_szRAMSize:			    db	"RAM size:", 0
_szMSizeKb:             db  "KB", 0
_szReturn:			    db	0Ah, 0
_szThreeSpace:          db  '   ', 0
;; 变量
_dwMCRNumber:			dd	0	                ; Memory Check Result
_dwDispPos:			    dd	(160 * 4 + 2 * 0)	; 屏幕第 4 行, 第 0 列。
_dwMemSize:			    dd	0
_ARDStruct:			    ; Address Range Descriptor Structure
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:		    dd	0
_MemChkBuf:	times	256	db	0
;
; 保护模式下需要使用这些符号
szRAMSize		    equ	LOADER_PHY_ADDR + _szRAMSize
szMSizeKb           equ LOADER_PHY_ADDR + _szMSizeKb
szReturn		    equ	LOADER_PHY_ADDR + _szReturn
szThreeSpace        equ LOADER_PHY_ADDR + _szThreeSpace
dwDispPos		    equ	LOADER_PHY_ADDR + _dwDispPos
dwMemSize		    equ	LOADER_PHY_ADDR + _dwMemSize
dwMCRNumber		    equ	LOADER_PHY_ADDR + _dwMCRNumber
ARDStruct		    equ	LOADER_PHY_ADDR + _ARDStruct
	dwBaseAddrLow	equ	LOADER_PHY_ADDR + _dwBaseAddrLow
	dwBaseAddrHigh	equ	LOADER_PHY_ADDR + _dwBaseAddrHigh
	dwLengthLow	    equ	LOADER_PHY_ADDR + _dwLengthLow
	dwLengthHigh	equ	LOADER_PHY_ADDR + _dwLengthHigh
	dwType		    equ	LOADER_PHY_ADDR + _dwType
MemChkBuf		    equ	LOADER_PHY_ADDR + _MemChkBuf


; 堆栈就在数据段的末尾
StackSpace:	times	1000h	db	0
TopOfStack	equ	LOADER_PHY_ADDR + $	    ; 栈顶

;================================================================================================

