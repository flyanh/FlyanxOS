; 加载FlyanxOS的第一步: boot　其做一些系统的初始化工作,然后寻找硬盘中的Loader,并加载它,使命完成
; 本文件通过硬盘引导，同目录下的boot.asm是软盘引导

org 0x7c00          ; Boot 状态, Bios 将把 Boot Sector 加载到 0:7C00 处并开始执行

    jmp boot_strap

;; 导入加载信息
%include "load.inc"

STACK_BASE          equ 0x7c00      ; boot的栈基地址
TRANS_SECT_NR       equ 2
SECT_BUFFER_SIZE    equ TRANS_SECT_NR * 512

;; 对磁盘地址进行打包，然后送到超级块存储地址上
disk_address_packet:    db 0x10             ;; [ 0] 磁盘地址大小（字节）
                        db 0                ;; [ 1] 保留，必须为0
                        db TRANS_SECT_NR    ;; [ 2] 多少块要被传输
                        db 0                ;; [ 3] 保留使用，必须为0
                        dw 0                ;; [ 4] 传输偏移地址
                        dw SUPER_BLOCK_SEG  ;; [ 6] 缓冲段地址，即要传输到的地方
                        dd 0                ;; [ 8] LBA低32位
                        dd 0                ;; [12] LBA高32位

;;==================================================================================
;;                       错误处理
;;==================================================================================
error_handler:
    mov dh, 3           ;; 字符串"ERROR 0   "
    call print          ;; 打印
    jmp $               ;; 死机

;;==================================================================================
;;                       开始引导
;;==================================================================================
boot_strap:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, STACK_BASE

    call clear_screen       ;; clear_screen();

    mov dh, 0               ;; "Booting..."
    call print              ;; print("Booting...");

    ;; 读超级块并将其信息存储到SUPER_BLOCK_SEG::0
    mov dword [disk_address_packet + 8], ROOT_BASE + 1
    call read_sector
    mov ax, SUPER_BLOCK_SEG
    mov fs, ax

    mov dword [disk_address_packet + 4], LOADER_OFFSET
    mov dword [disk_address_packet + 6], LOADER_SEG

    ;; 获取根目录'/'的扇区号（ROOT_INODE），它将存储到eax寄存器中
    mov eax, [fs:SB_ROOT_INODE]     ; eax = [fs:SB_ROOT_INODE]
    call get_inode                  ; get_inode(eax);

    ;; 读取'/'到ex:bx
    mov dword [disk_address_packet + 8], eax
    call read_sector

    ;; 现在我们开始搜索'/loader.bin'文件
    mov si, LoaderFileName
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

    mov dx, SECT_BUFFER_SIZE
    cmp bx, dx
    jge .no_fount_it        ;; 一样的结局

    push bx
    mov si, LoaderFileName
    jmp .no_fount_it

    push bx
    mov si, LoaderFileName
    jmp .str_cmp            ;; 问题不大，拿到硬盘下一个目录项，继续比较
.no_fount_it:               ;; 找不到，打印信息并死机
    mov dh, 2               ; print("No LOADER...");
    call print
    jmp $
.found_it:
    pop bx
    add bx, [fs:SB_DIR_ENT_INODE_OFF]
    mov eax, [es:bx]        ;; eax = Loader文件的索引节点号
    call get_inode          ;; get_inode(); //得到Loader文件的起始扇区，放在eax中
    mov dword [disk_address_packet + 8], eax
load_loader2mem:            ;; 现在开始将Loader载入内存中
    call read_sector        ;; read_sector();
    cmp eax, SECT_BUFFER_SIZE
    jl .done
    sub ecx, SECT_BUFFER_SIZE   ;; 载入剩余字节 -= 扇区缓冲区大小
    add word [disk_address_packet + 4], SECT_BUFFER_SIZE    ;; 传输缓冲区
    jc error_handler        ;; 传输失败，提示信息并死机，后期可改进为多试几次
    add dword [disk_address_packet + 8], TRANS_SECT_NR      ;; LBA
    jmp load_loader2mem                                     ;; 还剩呢，继续加载
.done:
    mov dh, 1
    call print      ; print("HD Boot.....");
    jmp LOADER_SEG:LOADER_OFFSET
    jmp $
;;==================================================================================
;;                       字符串
;;==================================================================================
LoaderFileName      db "hd_loader.bin", 0   ; LOADER的文件名
; 为简化代码，下面的字符串长度一致，都是MessageLength
MessageLength		equ	    12
BootMessage:        db	    "Booting....."  ; 12字节. 序号 0
Message1		    db	    "HD Boot....."  ; 12字节. 序号 1
Message2		    db	    "No LOADER..."  ; 12字节. 序号 2
Message3            db      "ERROR 0     "  ; 12字节，序号 3
Message4            db      "------------"  ; 4
;;==================================================================================
;;                       清屏
;;==================================================================================
clear_screen:
    mov ax, 0x600   ; AH = 6, AL = 0
    mov bx, 0x700   ; 黑底白字(BL = 0x7)
    mov cx, 0       ; 左上角：(0, 0)
    mov dx, 0x184f  ; 右下角：(80, 50)
    int 0x10
    ret
;;==================================================================================
;;                       打印一个字符串
;;==================================================================================
print:
    mov ax, MessageLength
    mul dh
    add ax, BootMessage
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
    mov edx, SECT_BUFFER_SIZE
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

times 510-($-$$)    db 0    ; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55			        ; 可引导扇区结束标志，必须是55aa，不然 BIOS 无法识别


