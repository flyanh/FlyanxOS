; 加载FlyanxOS的第二步: Loader　跳入保护模式，并加载真正的系统内核
; 这个 加载文件　突破了引导扇区 512 字节的限制(但还是在64K的限制内，但对我们完全够用)，我们可以在这里完成很多事情，所以较为重要

org 0100h
    jmp LABEL_START     ; 程序开始处

;================================================================================================
; 导入必要的头文件

; 下面是 FAT12 磁盘的头, 之所以包含它是因为下面用到了磁盘的一些信息
%include	"fat12hdr.inc"
; 加载内核所时需要的信息
%include	"load.inc"
; 主要的信息，各种宏定义和变量
%include	"pm.inc"

;================================================================================================
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

;================================================================================================
; 程序执行代码
LABEL_START:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, BaseOfStack

    ; 显示Loading字符串
    mov dh, 0          
    call DispStrOnRealModel

    ; 得到内存数
    mov ebx, 0          ; ebx = 后续值，开始时需为0
    mov di, _MemChkBuf  ; es:di 指向一个地址范围描述符结构(Address Range Descriptor Structure)
.MemChkLoop:
	mov eax, 0E820h		; eax = 0000E820h
	mov ecx, 20			; ecx = 地址范围描述符结构的大小
	mov edx, 0534D4150h	; edx = 'SMAP'
	int 15h				
	jc .MemChkFail		; 如果产生的进位，即CF = 1，跳转到.MemChkFail
	add di, 20
	inc dword [_dwMCRNumber]	; _dwMCRNumber = ARDS　的个数
	cmp ebx, 0
	jne .MemChkLoop		; ebx != 0，继续进行循环
	jmp .MemChkOK		; ebx == 0，得到内存数OK
.MemChkFail:
	mov dword [_dwMCRNumber], 0
.MemChkOK:
	; 下边将在A盘（我们的启动盘啦）根目录寻找 KERNEL.BIN 内核文件
	mov word [wSectorNo], SectorNoOfRootDirectory
	xor ah, ah	; @
	xor dl, dl	; #->　软驱复位
	int 13h		; @
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp word [wRootDirSizeForLoop], 0	; @
	jz LABEL_NO_KERNELBIN				; #-> 判断根目录是否已经读完，如果读完则表明没有找到内核文件
	dec word [wRootDirSizeForLoop]		; @
	mov ax, KERNEL_SEG
	mov es, ax							; es <- KERNEL_SEG
	mov bx, KERNEL_OFFSET			; bx <- KERNEL_OFFSET 于是，es:bx 存放的就是内核文件的应该存放的位置基地址
	mov ax, [wSectorNo]					; ax <- Root Directory 中的某　Sector 号
	mov cl, 1
	call ReadSector						; 开始读 ax 存放的　Sector

	mov si, KernelFileName		; ds:si -> "KERNEL  BIN"
	mov di, KERNEL_OFFSET	; es:di　－＞　KERNEL_SEG:??? = KERNEL_SEG * 10h + ???
	cld
	mov dx, 10h
LABEL_SEARCH_FOR_KERNELBIN:
	cmp dx, 0								; @
	jz LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR	; #-> 循环次数控制，如果已经读完了一个 Sector，就跳到下一个 Sector
	dec dx									; @
	mov cx, 11	; "KERNEL  BIN"字符串总共 11 个字节，当然就需要判断 11　次咯
LABEL_CMP_FILENAME:
	cmp cx, 0					; @
	jz LABEL_FILENAME_FOUND		; #-> 循环次数控制，比较了 11 个字符都相等，那么找到
	dec cx						; @
	lodsb						; ds:si -> al
	cmp al, byte [es:di]		; if al == es:di
	jz LABEL_GO_ON			
	jmp LABEL_DIFFERENT
LABEL_GO_ON:
	inc di
	jmp LABEL_CMP_FILENAME		;　继续循环
LABEL_DIFFERENT:
	and di, 0FFE0h						; @else 这时di的值不知道是什么，di　&= e0　是为了让它是 20h 的倍数
	add di, 20h							; #
	mov si, KernelFileName				; #-> di + 20h	到下一个目录条目
	jmp LABEL_SEARCH_FOR_KERNELBIN		; @
LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add word [wSectorNo], 1
	jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN
LABEL_NO_KERNELBIN:		; 如果循环比较完了，都没有找到，就会执行到这里，这表明内核文件不存在!
	; 显示未找到信息
	mov dh, 2	; "NO KERNEL!!!"
	call DispStrOnRealModel
	jmp $		; 没有找到内核文件，咱死循环在这
LABEL_FILENAME_FOUND:	; 如果找到的内核文件，将会跳转到这里继续
	mov ax, RootDirSectors
	and di, 0FFF0h			; di -> 当前条目的开始

	push eax	; 保存eax的值
	mov eax, [es:di + 01Ch]			; @
	mov dword [dwKernelSize], eax	; @-> 保存内核文件的大小
	cmp eax, KERNEL_HAVE_SPACE      ; 看看内核文件大小有没有超过我们为其保留的大小
	ja .KERNEL_FILE_TOO_LARGE       ; 超过了！
	pop eax		;恢复eax
    jmp .KERNEL_FILE_START_LOAD
.KERNEL_FILE_TOO_LARGE:
    mov dh, 3
    call DispStrOnRealModel         ; 相当于printf("Too Large!!!\n");
    jmp $                           ; 死机，因为内核太大，我们加载不了
.KERNEL_FILE_START_LOAD:            ; 准备开始加载内核文件
	add di, 01Ah	; di -> 首　Sector
	mov cx, word [es:di]
	push cx			; 保存此　Sector　在　FAT　中的序号
	add cx, ax
	add cx, DeltaSectorNo	; 这时 cl 里面是　内核文件　的起始扇区(从 0 开始计数哦) 
	mov ax, KERNEL_SEG
	mov es, ax		; es <- KERNEL_SEG
	mov bx, KERNEL_OFFSET	;　不想多解释了，这是偏移地址
	mov ax, cx		; ax <- Sector 号

LABEL_GOON_LOADING_FILE:
	push ax			; @
	push bx			; #
	cmp byte [bOdd], 0
	jz .2
.1:
	mov ah, 0Eh		; # 每读一个扇区就在　"Loading   "加符号
	mov al, ')'		; # 如果读取扇区奇数,加':-' ; 偶数加') '
	mov bl, 0Fh
    int 10h
    mov ah, 0Eh
    mov al, ' '
	jmp .3
.2:
	mov ah, 0Eh
	mov al, ':'
	mov bl, 0Fh
	int 10h
	mov ah, 0Eh
	mov al, '-'
.3:
	mov bl, 0Fh		    ; # 效果：Loading :-) :-) :-) :-)
	int	10h			    ; ┃
	pop	bx			    ; ┃
	pop	ax			    ; ┛

	mov cl, 1
	call ReadSector		; 读取　内核文件　所在的 Sector
	pop ax				; 取出 Sector 在 FAT 中的序号
	call GetFATEntry	; 获取读取的 Sector序号 的 FAT 条目
	cmp ax, 0FFFh
	jz LABEL_FILE_LOADED	; if ax != 0FFFh ; 跳转到 LABEL_FILE_LOADED，读写完毕
	push ax					; 保存 Sector 在　FAT　中的序号
	mov dx, RootDirSectors	
	add ax, dx
	add ax, DeltaSectorNo
	add bx, [BPB_BytsPerSec]
	jc .4               ; 如果bx重新变成0了，说明内核文件大于64KB
	jmp .5              ; 加载完毕
.4:
    push ax             ; es += 0x1000  <-- es指向下一个段，准备继续加载
    mov ax, es
    add ax, 1000h
    mov es, ax
    pop ax
.5:
	jmp LABEL_GOON_LOADING_FILE		; 继续读写　内核文件　下一个　Sector
LABEL_FILE_LOADED:					;　读　内核文件　完毕
	call KillMotor					;　关闭软驱马达
	mov dh, 1						; "Enter KERNEL"
	call DispStrOnRealModel			; 显示字符串

; 通过上面的操作，我们已经将内核加载进入内存，接下来我们准备跳入保护模式 -------------------------------------------
	
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

;================================================================================================
;变量
;----------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors	; Root Directory 占用的扇区数
wSectorNo		    dw	0		        ; 要读取的扇区号
bOdd			    db	0		        ; 奇数还是偶数
dwKernelSize		dd	0		        ; KERNEL.BIN 文件大小

;================================================================================================
;字符串
;----------------------------------------------------------------------------
KernelFileName		db	"KERNEL  BIN", 0	; KERNEL.BIN 之文件名(在FAT12中)
; 为简化代码, 下面每个字符串的长度均为 MessageLength
MessageLength		equ	12
LoadMessage:		db	"Loading....."
Message1		    db	"Enter KERNEL"
Message2		    db	"No KERNEL!!!"
Message3            db  "Too Large!!!"
;================================================================================================
;----------------------------------------------------------------------------
; 函数名: DispStrOnRealModel
;----------------------------------------------------------------------------
; 运行环境:
;	实模式（保护模式下显示字符串由函数 DispStr 完成）
; 作用:
;	显示一个字符串, 函数开始时 dh 中应该是字符串序号(0-based)
DispStrOnRealModel:
	mov	ax, MessageLength
	mul	dh
	add	ax, LoadMessage
	mov	bp, ax			; ┓
	mov	ax, ds			; ┣ ES:BP = 串地址
	mov	es, ax			; ┛
	mov	cx, MessageLength	; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0
	add	dh, 2			; 从第 3 行往下显示
	int	10h			; int 10h
	ret

;================================================================================================
; 函数名: ReadSector
;----------------------------------------------------------------------------
; 作用:
;	从序号(Directory Entry 中的 Sector 号)为 ax 的的 Sector 开始, 将 cl 个 Sector 读入 es:bx 中
ReadSector:
	; -----------------------------------------------------------------------
	; 怎样由扇区号求扇区在磁盘中的位置 (扇区号 -> 柱面号, 起始扇区, 磁头号)
	; -----------------------------------------------------------------------
	; 设扇区号为 x
	;                          ┌ 柱面号 = y >> 1
	;       x           ┌ 商 y ┤
	; -------------- => ┤      └ 磁头号 = y & 1
	;  每磁道扇区数       │
	;                   └ 余 z => 起始扇区号 = z + 1
	push	bp
	mov	bp, sp
	sub	esp, 2				; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]

	mov	byte [bp-2], cl
	push	bx				; 保存 bx
	mov	bl, [BPB_SecPerTrk]	; bl: 除数
	div	bl					; y 在 al 中, z 在 ah 中
	inc	ah					; z ++
	mov	cl, ah				; cl <- 起始扇区号
	mov	dh, al				; dh <- y
	shr	al, 1				; y >> 1 (其实是 y/BPB_NumHeads, 这里BPB_NumHeads=2)
	mov	ch, al				; ch <- 柱面号
	and	dh, 1				; dh & 1 = 磁头号
	pop	bx					; 恢复 bx
	; 至此, "柱面号, 起始扇区, 磁头号" 全部得到 ^^^^^^^^^^^^^^^^^^^^^^^^
	mov	dl, [BS_DrvNum]		; 驱动器号 (0 表示 A 盘)
.GoOnReading:
	mov	ah, 2				; 读
	mov	al, byte [bp-2]		; 读 al 个扇区
	int	13h
	jc	.GoOnReading		; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止

	add	esp, 2
	pop	bp

	ret

;================================================================================================
; 函数名: GetFATEntry
;----------------------------------------------------------------------------
; 作用:
;	找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
;	需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax, KERNEL_SEG	; ┓
	sub	ax, 0100h				; ┣ 在 KERNEL_SEG 后面留出 4K 空间用于存放 FAT
	mov	es, ax					; ┛
	pop	ax
	mov	byte [bOdd], 0
	mov	bx, 3
	mul	bx						; dx:ax = ax * 3
	mov	bx, 2
	div	bx						; dx:ax / 2  ==>  ax <- 商, dx <- 余数
	cmp	dx, 0
	jz	LABEL_EVEN
	mov	byte [bOdd], 1
LABEL_EVEN:;偶数
	xor	dx, dx					; 现在 ax 中是 FATEntry 在 FAT 中的偏移量. 下面来计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
	mov	bx, [BPB_BytsPerSec]
	div	bx						; dx:ax / BPB_BytsPerSec  ==>	ax <- 商   (FATEntry 所在的扇区相对于 FAT 来说的扇区号)
								;				dx <- 余数 (FATEntry 在扇区内的偏移)。
	push	dx
	mov	bx, 0				; bx <- 0	于是, es:bx = (KERNEL_SEG - 100):00 = (KERNEL_SEG - 100) * 10h
	add	ax, SectorNoOfFAT1	; 此句执行之后的 ax 就是 FATEntry 所在的扇区号
	mov	cl, 2
	call	ReadSector		; 读取 FATEntry 所在的扇区, 一次读两个, 避免在边界发生错误, 因为一个 FATEntry 可能跨越两个扇区
	pop	dx
	add	bx, dx
	mov	ax, [es:bx]
	cmp	byte [bOdd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4
LABEL_EVEN_2:
	and	ax, 0FFFh

LABEL_GET_FAT_ENRY_OK:

	pop	bx
	pop	es
	ret

;================================================================================================
;----------------------------------------------------------------------------
; 函数名: KillMotor
;----------------------------------------------------------------------------
; 作用:
;	关闭软驱马达，有时候软驱读取完如果不关闭马达，马达会持续运行且发出声音
KillMotor:
	push	dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret

;================================================================================================
;================================================================================================
;================================================================================================
; 32位代码段！！！运行在保护模式下
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

	; 现在为简化处理，所有线性地址对应相等的物理地址，并且暂不考虑内存空洞

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