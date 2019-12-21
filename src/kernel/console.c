/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/28.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 控制台
 */
#include "kernel.h"
#include <termios.h>
#include <flyanx/callnr.h>
#include <flyanx/common.h>
#include "protect.h"
#include "tty.h"
#include "process.h"
#include <stdarg.h>     /* 对可变参数列表操作 */

/* BIOS参数相关 */
#define BIOS_PARM_COLUMNS       0x44AL      /* 屏幕有多少列（按字符算） */
#define BIOS_PARM_CRTBASE       0x463L      /*  */
#define BIOS_PARM_ROWS          0x484L      /* 屏幕有多少行（按字符算） */
#define BIOS_PARM_FONTLINES     0x485L      /*  */

/* 控制台驱动程序使用的定义。 */
#define MONO_BASE       0xB0000L	    /* 单色视频存储器的基地址 */
#define COLOR_BASE      0xB8000L	    /* 彩色视频存储器的基地址 */
#define MONO_SIZE       0x1000	        /* 单色视频存储器所占大小->4K */
#define COLOR_SIZE      0x4000	        /* 彩色视频存储器所占大小->16K */
#define EGA_SIZE        0x8000	        /* EGA和VGA至少有32K */
#define BLANK_COLOR     0x0700	        /* 确定光标在空白屏幕上的颜色 */
#define SCROLL_UP       0	            /* 前滚，用于滚动屏幕 */
#define SCROLL_DOWN     1	            /* 后滚 */
#define BLANK_MEM       ((u16_t *) 0)	/* 告诉mem vid copy()清空屏幕 */
#define CONS_RAM_WORDS  80	            /* 视频ram缓冲区大小 */
#define MAX_ESC_PARMS   4	            /* 允许的转义序列参数数量 */

/* 与控制器芯片相关的常数。 */
#define MONO_6845           0x3B4	/* 6845单色端口 */
#define COLOR_6845          0x3D4	/* 6845彩色端口 */
#define EGA                 0x3C4	/* EGA卡端口 */
#define INDEX               0	    /* 6845控制器的索引寄存器 */
#define DATA                1	    /* 6845控制器的数据寄存器 */
#define VIDEO_ORG           12	    /* 6845控制器的段地址寄存器 */
#define CURSOR              14	    /* 6845控制器的光标寄存器 */

/* 蜂鸣器 */
#define BEEP_FREQ       0x0533	/* 将此值放入定时器以设置蜂鸣的频率 */
#define BEEP_TIME		    3	    /* CTRL-G蜂鸣声的时间长度（时钟滴答） */

/* 用于字体管理的定义 */
#define GA_SEQUENCER_INDEX	0x3C4
#define GA_SEQUENCER_DATA	0x3C5
#define GA_GRAPHICS_INDEX	0x3CE
#define GA_GRAPHICS_DATA	0x3CF
#define GA_VIDEO_ADDRESS	0xA0000L
#define GA_FONT_SIZE		8192

#define CONS_RAM_WORDS  80	                /* 视频ram缓冲区大小 */
#define MAX_ESC_PARMS   4	                /* 允许的转义序列参数数量 */
#define CONSOLE_IN_BYTES	  64	        /* 键盘输入缓冲区的大小 */

/* 为控制台驱动程序和程序集提供支持所使用的全局变量。 */
PUBLIC phys_bytes   video_base;     /* 视频基地址 */
PUBLIC unsigned     video_size;	    /* 0x2000表示彩色，0x0800表示单色 */
PUBLIC unsigned     video_mask;	    /* 0x1FFF表示彩色，0x07FF表示单色 */
PUBLIC unsigned     blank_color =   BLANK_COLOR; /* 空白显示代码 */

/* 控制台驱动程序使用的私有变量。 */
PRIVATE int video_port;		            /* 用于访问6845的I/O端口 */
PRIVATE bool wrap;			        /* 硬件可以重叠吗? */
PRIVATE bool software_scroll;       /* 1 = 软件滚动，0 = 硬件 */
PRIVATE bool beeping;		        /* 扬声器已经在蜂鸣？ */
PRIVATE unsigned font_lines;	        /* 每个字符占几行 */
PRIVATE unsigned screen_width;	        /* 屏幕一行的字符数 */
PRIVATE unsigned screen_lines;	        /* 屏幕的行数 */
PRIVATE unsigned screen_size;	        /* 屏幕的总字符数 */

typedef struct console_s {
    struct tty_s *tty;          /* 跟控制台关联的终端 */
    int column;                 /* 光标所在列 */
    int row;                    /* 光标所在行 */
    int ram_words;              /* 视频输出队列中的字（word）数 */
    unsigned start;             /* 显示内存的起始地址 */
    unsigned limit;         /* 显示内存的界限 */
    unsigned origin;        /* 由控制器芯片的基指针指向的内存地址 */
    unsigned cursor;        /* 在显存中光标的当前位置 */
    unsigned attr;          /* 字符属性，颜色以及是否闪烁等 */
    unsigned blank;         /* 空字符属性 */
    bool reverse;           /* 状态：反相显示 */
    char escape_state;      /* 0 = 正常,1 = ESC, 2 = ESC [] */
    char escape_intro;      /* 转义信息标识符，标识上次转义处理到了哪一步，0即ESC，[则是到了ESC [，以此类推 */
    int escape_parm[MAX_ESC_PARMS]; /* 转义参数列表 */
    int *p_escape_parm;         /* 指向当前转义参数的指针 */
    u16_t ram_queue[CONS_RAM_WORDS];/* 视频存储器缓冲队列 */
} Console;

PUBLIC int nr_console;		                        /* 控制台数量 */
PUBLIC Console console_table[NR_CONSOLES];     /* 控制台表 */
PRIVATE Console *curr_console;	                /* 当前可见的（在屏幕上显示的）控制台 */
PRIVATE char buffer[CONSOLE_IN_BYTES];              /* 控制台缓冲区 */

/* 从ANSI颜色映射到PC使用的属性 */
PRIVATE int ansi_colors[8] = {0, 4, 2, 6, 1, 5, 3, 7};

/* 如果使用彩色控制器，使用彩色端口。 */
#define color	(video_port == COLOR_6845)

/* 用于字体管理的结构 */
typedef struct sequence_s {
    unsigned short index;
    unsigned char port;
    unsigned char value;
} Sequence;


FORWARD _PROTOTYPE( void out_char, (Console *console_data, int ch) );
//FORWARD _PROTOTYPE( void scroll_screen, (unsigned direction) );
FORWARD _PROTOTYPE( void flush, (Console *console_data) );
FORWARD _PROTOTYPE( void parse_escape, (Console *console_data, char ch) );
FORWARD _PROTOTYPE( void beep, (void) );
FORWARD _PROTOTYPE( void console_origin0, (void) );
FORWARD _PROTOTYPE( void stop_beep, (void) );
FORWARD _PROTOTYPE( void do_escape, (Console *console_data, char ch) );
FORWARD _PROTOTYPE( void set_6845_video_start, (unsigned int start_addr) );
FORWARD _PROTOTYPE( void set_6845_cursor, (unsigned int position) );
FORWARD _PROTOTYPE( void memory2video_copy, (u16_t *src, unsigned int dst, unsigned int count) );
FORWARD _PROTOTYPE( void video2video_copy, (unsigned int src, unsigned int dest, unsigned int count) );
FORWARD _PROTOTYPE( void write, (TTY *tty)			);
FORWARD _PROTOTYPE( void echo, (TTY *tty, int ch)		);
FORWARD _PROTOTYPE( void ioctl, (TTY *tty) );

/*===========================================================================*
 *				console_init				     *
 *				控制台初始化
 * tty：跟控制台关联的终端
 * char_attribute：控制台显示字符的默认属性
 *===========================================================================*/
PUBLIC void console_init(TTY *tty){
    Console *console;
    int line;
    unsigned page_size;

    /* 关联控制台和终端 */
    line = tty - &tty_table[0];
    /* 如果初始化的控制台超出表，说明这不是一个有效可用的终端 */
    if(line >= nr_console) return;
    console = &(console_table[line]);
    console->tty = tty;             /* 关联控制台的终端 */
    tty->priv = console;            /* 关联终端的控制台 */

    /* 建立指向设备操作例程的指针 */
    tty->device_write = &write;      /* 终端写操作 */
    tty->echo = &echo;               /* 终端回显 */
    tty->ioctl = &ioctl;             /* 终端I/O控制操作 */

    page_size = video_size / nr_console;
    console->start = line * page_size;				        /* 计算控制台的显存开始地址 */
    console->limit = console->start + page_size;	        /* 计算控制台的显存界线 */
    console->cursor = console->origin = console->start;	    /* 初始化控制台的光标位置，基地址指针和显存开始地址 */
    console->attr = console->blank = BLANK_COLOR;	        /* 初始化字符属性，空属性为黑底白字 */

    /* 清空控制台的屏幕 */
    blank_color = BLANK_COLOR;
    memory2video_copy(BLANK_MEM, console->start, screen_size);
    display_position = 0;           /* 现在已经有终端了，这个值也就没用了，但我们还是重置它吧 */
    ioctl(tty);
}

/*===========================================================================*
 *				blue_screen				     *
 *				 蓝屏
 *===========================================================================*/
PUBLIC void blue_screen(void){
    /* 系统遇到不可恢复的错误，准备打印错误信息，进入蓝屏状态。
     * 这个功能十分有名，因为它来自于Windows，flyanx也简单实
     * 现了它。
     */
    TTY *tty = console_table[0].tty;
    Console *console = tty->priv;

    /* 切换到第一个控制台 */
    switch_to(0);
    /* 设置蓝屏属性 */
    console->attr = MAKE_COLOR(BLUE, WHITE) | BRIGHT;
    /* 清屏（改变属性） */
    blank_color = console->attr;
    memory2video_copy(BLANK_MEM, curr_console->origin, screen_size);
    blank_color = BLANK_COLOR;
    console->row = console->column = 0;
    flush(curr_console);
}

/*===========================================================================*
 *				    write				     *
 *                控制台写入
 *===========================================================================*/
PRIVATE void write(TTY *tty){
    /* 控制台写入
     * 这个函数的地址存放在每个控制台的device_write入口处。它只从一个地方被调用，即tty.c的handle_events
     * 中。priv.c中的其他大部分函数的存在都是为了支持这个函数。当它在一个客户进程进行了一个WRITE调用后
     * 被第一次调用时，待输出的数据位于客户进程的缓冲区中，可以用tty结构中的out_proc和out_vir_addr找到。
     * out_left域记录了还有多少字符需要传送，out_cum被初始化为0，表示什么还没有传送。这是在进入console_write
     * 时的一般情况，因为在正常情况下，一旦该函数被调用，它就传送原始调用请求的所有数据。不过，如果用户希望进
     * 程能慢下来，以便在屏幕上察看数据，那么他可以键入一个STOP（CTRL-S）字符，这将引起status_inhibited标
     * 志置位。console_write在该标志置位时立即返回， 即使WRITE还没有结束。在这种情况下，handle_events将
     * 继续调用console_write，当status_inhibited最终被重置时，用户通过键入START（CTRL-Q）可以让console_write
     * 继续中断的传送。
     */
    char *temp_buffer;
    phys_bytes user_phys;
    unsigned int count;
    Console *console = tty->priv;
    /* 进来第一件事就是快速检查是否真的有事可做，因此可以经常调用本例程，因为没事做就立马返回
     * 了，而不需要在其他地方进行费模块化测试。
     */
    count = tty->out_left;      /* 得到终端想要写入的数据量 */
    if(count == 0 || tty->status_inhibited) return;

    do{
        /* 如果用户要向控制台写入的数据过多，降低count到和控制台缓冲区一致，分多次传送 */
        if(count > sizeof(buffer)) count = CONSOLE_IN_BYTES;
        /* 得到用户的输出缓冲区，即用户准备写入控制台的数据 */
        user_phys = proc_vir2phys(proc_addr(tty->out_proc), tty->out_vir);
        phys_copy(user_phys, vir2phys(buffer), (phys_bytes)count);
        temp_buffer = buffer;
        /* 更新终端数据结构 */
        tty->out_vir += count;
        tty->out_cum += count;
        tty->out_left -= count;

        /* 将拷贝的每个字节输出到屏幕。避免对“简单”的字符调用out_char()，直接将它们放入缓冲区。 */
        do{
            /* 如果字符是一个不可见字符 或 有一个转义序列 或 超过了屏幕宽度
			 * 或 视频存储器缓冲队列满了
             * 上面的四种情况都需要执行out_char输出这个字符到控制台，因为其
             * 可能需要一些奇怪的特殊操作。
             */
            if((unsigned) *temp_buffer < ' ' || console->escape_state != FALSE
               || console->column >= screen_width
               || console->ram_words >= buffer_len(console->ram_queue)){
                out_char(console, *temp_buffer);
            } else {
                /* 当然了，如果这个字符的非常“简单”，我们直接将该字符连同属性字节直接放入到视频缓冲队列中。 */
                console->ram_queue[console->ram_words] = console->attr | (*temp_buffer & BYTE);
                console->ram_words++;   /* 指向视频输出的下一个字（word） */
                console->column++;      /* 指向下一列 */
            }
            temp_buffer++;  /* 指向下一个要处理的数据 */

        } while (--count != 0);

        /* 只要out_left指示仍有字符等待传送且inhibited仍然没有置位，这个从
         * 用户进程缓冲区到局部缓冲区再到视频缓存队列的传送就重复进行。直到处
         * 理完成所有请求量。
         */
        count = tty->out_left;      /* 上一轮的数据已经处理完成，重新得到还剩下的处理量 */
    } while (count != 0 && !tty->status_inhibited);

    /* ok。将现在控制台中缓冲队列的所有字符传送到真正的屏幕存储器。 */
    flush(console);

    /* 如果操作真的完成了，终端任务就发送一条回答消息。 */
    if(tty->out_left == 0){
        tty_reply(tty->out_reply_code, tty->out_caller, tty->out_proc,
                  tty->out_cum);
        tty->out_cum = 0;       /* 写入完毕 */
    }
}

/*===========================================================================*
 *				    echo				     *
 *				 控制台回显
 *===========================================================================*/
PRIVATE void echo(
        TTY *tty,       /* 回显的终端 */
        int ch          /* 回显的字符 */
){
    /* 控制台回显：回显键盘的输入(打印并刷新)。
     *
     * 关于这个函数，待显示的字符还由终端任务的硬件无关部分的echo或rawecho并立即送往
     * 控制台。如果控制台是当前的输出设备，就由tp->echo指针调用下一个本函数。本函
     * 数通过调用out_char然后调用flush完成所有的工作。来自键盘的输入逐个字符地到达，而
     * 输入的人希望看到回显没有可察觉的延迟，所以把字符送入输出队列并不能满足要求。
     */
    Console *console = tty->priv;

    /* 输出字符并将其刷洗到显存 */
    out_char(console, ch);
    flush(console);
}

/*===========================================================================*
 *				 ioctl				     *
 *				设置窗口框架
 *===========================================================================*/
PRIVATE void ioctl(TTY *tty){
    tty->win_frame.row = screen_lines;
    tty->win_frame.col = screen_width;
    tty->win_frame.x_pixel = screen_width * 8;
    tty->win_frame.y_pixel = screen_lines * font_lines;
    unsigned short s;
}

/*===========================================================================*
 *				out_char				     *
 *			向控制台输出一个字符
 *===========================`================================================*/
PRIVATE void out_char(
        Console *console,
        int ch
){
    /* 首先检查转义序列。 */
    if(console->escape_state != FALSE){
        /* 存在转义序列，调用转义处理函数，立即返回。 */
        parse_escape(console, ch);
        return;
    }

    switch (ch){
        case 000:               /* null字符 */
            break;              /* null字符通常用于填充，所以我们处理null字符最好的方式就是什么都不做 */

        case 007:               /* '\a': 响铃符号 */
            flush(console);     /* 刷洗输出缓冲区中等待的字符 */
            beep();             /* 蜂鸣：发出哔哔声 */
            break;

        case '\b':              /* 退格符号 */
            /* 列数-1，即退格 */
            console->column--;
            /* 如果退格完成后，发现超出屏幕左外边，处理一下下 */
            if(console->column < 0){
                /* 到达上一行（行数-1），如果还在屏幕中，那么，将控制台列数增加一行的总字符量，这样就正常了 */
                console->row--;
                if(console->row >= 0) console->column += screen_width;
            }
            flush(console);
            break;

        case '\n':              /* 换行符 */
            if((console->tty->termios.oflag & (OPOST | ONLCR)) == (OPOST | ONLCR)){
                /* 如果输出模式是执行输出处理，且输出时将NL映射到CR_NL，才可以进行换行 */
                console->column = 0;    /* 列数直接归零 */
            }
            /* 陷入下面向上滚屏的代码，可复用
            */
        case 013:               /* CTRL-K : 向上滚屏 */
        case 014:               /* CTRL-L : 效果等同CTRL-K */
            if(console->row == screen_lines - 1){
                /* 如果光标行在屏幕最底部最后一行，那么，进行向上滚屏 */
                scroll_screen(SCROLL_UP);
            } else{
                /* 如果光标在屏幕中间，那么行数+1即可 */
                console->row++;
            }
            flush(console);
            break;

        case '\r':              /* 回车符 */
            /* 控制台列数变为0，OK */
            console->column = 0;
            flush(console);
            break;

        case '\t':              /* 制表符 */
            /* 列数加上制表符的长度 */
            console->column = (console->column + TAB_SIZE) & ~TAB_MASK;
            /* 处理一下控制台列数到屏幕下外边的情况 */
            if(console->column > screen_width){
                /* 列数减掉屏幕宽度，就得到了最新的下一行的列数 */
                console->column -= screen_width;
                if(console->row == screen_lines - 1){
                    /* 如果行数处于最底下最后一行，向上滚屏 */
                    scroll_screen(SCROLL_UP);
                } else{
                    /* 还在屏幕的中间，行数+1即可 */
                    console->row++;
                }
            }
            flush(console);
            break;

        case 033:               /* 转义符号：标志着即将开始一个转义序列 */
            flush(console);     /* 前面或许还有在等待输出的字符，冲洗它们 */
            console->escape_state = TRUE;   /* 将转义状态置位 */
            break;

        default:                /* 能到这里的，将全部是可打印的普通字符，将其存储到视频缓冲队列中 */
            /* 超出了屏幕宽度 */
            if(console->column >= screen_width){
                /* 如果用户在config.h中配置不需要换行，我们直接返回结束 */
                if(!LINE_WARP) return;
                if(console->row == screen_lines - 1){
                    /* 如果行数已经到底，向上滚屏 */
                    scroll_screen(SCROLL_UP);
                } else {
                    /* 还在屏幕中间，正常情况，行数 + 1 */
                    console->row++;
                }
                /* 列置0 */
                console->column = 0;
                flush(console);
            }
            /* 如果控制台的缓存队列中已满，刷洗更新到视频存储 */
            if(console->row == buffer_len(console->ram_queue)) flush(console);
            /* 上面的工作做完了，我们可以处理这次要输出的字符了 */
            console->ram_queue[console->ram_words] = console->attr | (ch & BYTE); /* 记得设置上字符属性 */
            console->ram_words++;   /* 指向视频输出的下一个字（word） */
            console->column++;      /* 指向下一列 */
            break;
    }
}

/*===========================================================================*
 *				scroll_screen				     *
 *			        滚屏
 * direction: 上滚（SCROLL_UP）或下滚（SCROLL_DOWN）
 *===========================================================================*/
PUBLIC void scroll_screen(unsigned direction){
    /* 滚屏（只能对当前正在使用的控制台生效）
     * 处理在屏幕最后一行已满时的向上滚屏，以及向下滚屏，主要导致原因是其在光标定位命令试图
     * 把光标移到屏幕之外。对每个方向的滚动，有三种可能的处理方法。它们要用来支持不同类型的
     * 视频卡。
     */
    unsigned int new_line, new_origin , chars;

    flush(curr_console);    /* 滚屏前可能有字符正在排队，先保证它们更新到当前控制台 */
    chars = screen_size - screen_width;

    /* 滚动屏幕是一个真正的麻烦，因为各种不兼容的视频卡。这个驱动程序支持软件滚动，
    * 硬件滚动(单色卡和CGA卡)和硬件滚动无包装(EGA卡)。在后一种情况下，我们必须确保
    * 		c_start <= c_org && c_org + scr_size <= c_limit
    * 保持不变，因为EGA没有缠绕在视频存储器的末端。
    */
    if(direction == SCROLL_UP){             /* 向上滚屏 */
        if(software_scroll) {               /* 软件滚屏 */
            /* 软件滚屏，将chars个字符向内存低端移动，移动的距离是一行的字符数。
             * 软件卷屏由一个单独的对vid_vid_copy的调用完成，把chars个字符向内存低端移动，
             * 移动的距离是一行中的字符数。vid_vid_copy是可回卷的，即如果请求移动的内存块
             * 溢出了赋予视频显示内存的上边界，就从内存块的低端取出溢出部分，把它移动到高于
             * 被移到低端部分的地址处，即把整个块看作一个环形数组。这个调用看起来简单，但执
             * 行却相当慢。即使vid_vid_copy是定义在klib386.s中的汇编语言例程，这个调用也需
             * 要CPU移动3840个字节，就算对于汇编语言也是一项繁重的工作。
             * 软件卷屏永远不会作为默认方式；只有在硬件卷屏不能工作或由于某种原因不能采用时
             * 才选择软件方式。一个原因可能是因为希望使用screendump命令把屏幕内存存入一个文
             * 件。当使用硬件卷屏时，screendump可能会产生不可预知的结果，因为屏幕内存的起始
             * 地址和显示器可见部分的开始位置可能不一致。
             */
            video2video_copy(curr_console->start + screen_width, curr_console->start, chars);

        } else if(!wrap && curr_console->origin + screen_size + screen_width >= curr_console->limit){
            /* wrap变量被检测，它是一系列复合检测的第一部分，如果为FLASE，说明存在多各控制台
             * 不允许视频内存重叠，复合检测将继续测试滚屏操作将要移动的内存块是否溢出了分配给
             * 该控制台的内存界线，如果是，就再次调用video2video_copy执行一个回滚移动把内存块移动
             * 到控制台分配内存的起始处，并且原始指针被修改，通过软件辅助完成滚屏。如果没有重叠
             * ，控制就被传递给旧的视频控制器一直使用的简单硬件滚屏方法（在下面）。
             */
            video2video_copy(curr_console->origin + screen_width, curr_console->start, chars);
            curr_console->origin = curr_console->start;

        } else {                            /* 硬件滚屏 */
            /* 对于支持硬件滚屏的显示器wrap为真，那么这里进行简单的硬件滚屏，
             * 视频控制芯片使用的原始指针被更新为显示器左上角显示的第一个字符。
             */
            curr_console->origin = (curr_console->origin + screen_width) & video_mask;
        }
        /* 得到出现的新行，它从屏幕底部出现。 */
        new_line = (curr_console->origin + chars) & video_mask;
    } else {                                /* 向下滚屏 */
        /* 和上滚屏完全一样，没什么不同和可注意的。 */
        if(software_scroll) {               /* 软件滚屏 */
            video2video_copy(curr_console->start, curr_console->start + screen_width , chars);

        } else if(!wrap && curr_console->origin < curr_console->start + screen_width){
            new_origin = curr_console->limit - screen_size;
            video2video_copy(curr_console->origin, new_origin + screen_width , chars);
            curr_console->origin = new_origin;
        } else {                            /* 硬件滚屏 */
            curr_console->origin = (curr_console->origin - screen_width) & video_mask;
        }
        /* 得到出现的新行，它从屏幕顶部出现。 */
        new_line = curr_console->origin;
    }
    /* 清空新出现的行（最后或顶部） */
    blank_color = curr_console->blank;
    memory2video_copy(BLANK_MEM, new_line, screen_width);

    /* 设置新的原点位置 */
    set_6845_video_start(curr_console->origin);
    flush(curr_console);
}

/*===========================================================================*
 *				    flush				     *
 *		        将控制台缓冲刷到显存
 *===========================================================================*/
PRIVATE void  flush(Console *console){
    /* 将缓存队列更新到视频内存中，并更新变量保证行和列的数值是合理的。 */
    unsigned int cursor;
    TTY *tty = console->tty;

    /* 将缓冲队列中的字符传输到屏幕上。 */
    if(console->ram_words > 0){
        memory2video_copy(console->ram_queue, console->cursor, console->ram_words);
        console->ram_words = 0;     /* 传输完毕，清零 */

        /* 终端想知道当前的列 以及 是否有错误的回调（回显数据已经混乱） */
        tty->position = console->column;
        tty->reprint = TRUE;
    }

    /* 检查并更新光标位置。 */
    if(console->column < 0) console->column = 0;    /* 列在屏幕左外边，拉回来 */
    if(console->column > screen_width) console->column = screen_width;  /* 在屏幕右外边 */
    if(console->row < 0) console->row = 0; /* 行在屏幕上外边 */
    if(console->row >= screen_lines) console->row = screen_lines - 1;    /* 行在屏幕下外边 */
    cursor = console->origin + console->row * screen_width + console->column; /* 计算光标位置 */
    if(cursor != console->cursor){
        /* 当新光标跟当前控制台光标位置不一致时，且是当前正在使用的控制器时，更新视频光标位置 */
        if(console == curr_console) set_6845_cursor(cursor);
        /* 无论如何，光标不一致都需要更新控制台的光标位置 */
        console->cursor = cursor;
    }
}

/*===========================================================================*
 *				    beep				     *
 *			      发出蜂鸣声
 *===========================================================================*/
PRIVATE void beep(void){
    /* 扬声器发出蜂鸣声
     *
     * 本例程在必须输出CTRL-G字符时被调用，它利用了PC内建的对声音的支持，向扬声器发送方波
     * 发出声音，即这个程序通过打开驱动扬声器的8255芯片B端口的0和1位来工作。
     */
//    Message msg;
    /* 如果电脑扬声器已经正在发出蜂鸣声，不再继续 */
    if(beeping) return;
    out_byte(TIMER_MODE, 0xB6);         /* 设置定时器通道2 */
    out_byte(TIMER2, BEEP_FREQ & BYTE); /* 加载频率的低位 */
    out_byte(TIMER2, (BEEP_FREQ >> 8) & BYTE);  /* 高位 */
    interrupt_lock();       /* 保护PORT_B端口的数据写入 */
    out_byte(PORT_B, in_byte(PORT_B) | 3);  /* 打开蜂鸣位 */
    interrupt_unlock();
    beeping = TRUE;

    /* 延迟BEEP_TIME滴答 */
    milli_delay(BEEP_TIME * 10);
    stop_beep();        /* 关闭蜂鸣 */


    /* 这里可以改进为使用闹钟的方式来定时关闭蜂鸣，以后的版本会改进 */
}

/*===========================================================================*
 *				    stop_beep				     *
 *			        停止蜂鸣声
 *===========================================================================*/
PRIVATE void stop_beep(void){
    /* 通过关闭PORT_B端口中的0和1位来关闭蜂鸣器。 */

    interrupt_lock();       /* 保护PORT_B端口的数据写入 */
    out_byte(PORT_B, in_byte(PORT_B) & ~3);
    beeping = FALSE;
    interrupt_unlock();
}

/*===========================================================================*
 *				parse_escape				     *
 *			    转义处理
 *===========================================================================*/
PRIVATE void parse_escape(
        Console *console,  /* 哪个控制台产生了转义？ */
        char ch                     /* 转义序列中的下一个字符，将会陆续进来 */
){
    /* 解析转义字符
     *
     * 目前支持下列ANSI转义序列。
     * 如果n或m（n和m是常数）被省略，它们默认为1。
     * ESC [nA 				向上移动n行
     * ESC [nB 				向下移动n行
     * ESC [nC 				向右移动n个空格
     * ESC [nD 				向左移动n个空格
     * ESC [m;nH]" 			将光标移动到(m,n)
     * ESC [J 				清除屏幕显示
     * ESC [K				清除光标处的行
     * ESC [nL				在光标处插入n行
     * ESC [nM				在光标处删除n行
     * ESC [nP				在光标处删除n个字符
     * ESC [n@				在光标处插入n个字符
     * ESC [nm				启用渲染n(0=正常，4=粗体，5=闪烁，7=反向)
     * ESC M 				如果光标在最上面一行，向后滚动屏幕
     *
     * 来自Flyan的话：它们都来自MINIX2，这个函数可以被更改或扩展，以支持更多的转义序列，需要更多
     * 参数的转义序列，可以直接更改 MAX_ESC_PARMS 的值，不需要改动太多这个函数。
     */

    if(console->escape_state == 1){     /* 转义序列处理第一步，只处理'ESC'开头的序列 */
        /* 往转义信息字符中放入0，用于标识ESC开头的处理正在进行 */
        console->escape_intro = '\0';
        /* 将转义参数列表的界限放入到其指针中 */
        console->p_escape_parm = buffer_end(console->escape_parm);
        do {
            /* 转义参数列表初始化为0 */
            console->p_escape_parm--;
            *console->p_escape_parm = 0;
        } while (console->p_escape_parm > console->escape_parm);
        /* 开始处理 */
        switch (ch){
            case '[':           /* 控制字符开头 */
                /* 放入'['到转义信息字符中，用于标识ESC [开头的处理正在进行 */
                console->escape_intro = ch;
                /* 转义状态设置为状态2，准备进行第二步处理 */
                console->escape_state = 2;
                break;
            case 'M':           /* ESC M转义序列，向后滚动屏幕 */
                /* 真正的工作我们交给do_escape()去做。 */
                do_escape(console, ch);
                break;
            default:            /* 不属于转义序列，或者flyanx暂时无法识别序列，重置转义状态 */
                console->escape_state = FALSE;
                break;
        }
    } else if(console->escape_state == 2){      /* 转义序列处理第二步，只处理'ESC ['开头的序列 */
        if(ch >= '0' && ch <= '9'){             /* ESC [ 常数 */
            if(console->p_escape_parm < buffer_end(console->escape_parm)){
                /* 常数 = 常数 * 10 + 字符的数值（不是ASCII码的值）
                 * 这里这样理解：我输入 '45'字符串，那么得到的不是该字符串的ASCII码，而是其真实数值，就是45！ */
                *console->escape_parm = *console->p_escape_parm * 10 + (ch - '0');
            } else if(ch == ';') {              /* ESC [ 常数; */
                if(console->p_escape_parm < buffer_end(console->escape_parm)){
                    /* 转义义参数向前移动，这样后继的数字值就可以在第二个参数中累积。 */
                    console->p_escape_parm++;
                }
            } else{
                /* 所有参数已经载入参数列表，可以做事情了 */
                do_escape(console, ch);
            }
        }
    }

}

/*===========================================================================*
 *				do_escape				     *
 *		识别转义序列，并执行其需要的动作
 *===========================================================================*/
PRIVATE void do_escape(Console *console, char ch){
    int value, n;
    unsigned int src, dest, count;
//    int *parmp;

    /* 在处理转义序列时，可能会有字符需要添加到视频中，所以最好保证视频显示应该是最新的 */
    flush(console);

    if(console->escape_intro == 0){     /* 处理 ESC ? 转义序列 */
        switch (ch){
            case 'M':                   /* ESC M : 向后滚动屏幕 */
                if(console->row == 0){
                    /* 如果行数是第一列，向后滚屏 */
                    scroll_screen(SCROLL_DOWN);
                } else {
                    /* 在屏幕中间，直接减少行数即可 */
                    console->row--;
                }
                flush(console);
                break;
            default:                    /* 未识别的 ESC ? 转义序列 */
                break;
        }
    } else if(console->escape_intro == '['){    /* 处理 ESC [ ? 转义序列 */
        value = console->p_escape_parm[0];      /* 得到第一个参数 */
        switch (ch){
            case 'A':		/* ESC [nA moves up n lines ：向上移动n行 */
                break;

            case 'B':		/* ESC [nB moves down n lines ：向下移动n行 */
                break;

            case 'C':		/* ESC [nC moves right n spaces ：屏幕向右移动 */
                break;

            case 'D':		/* ESC [nD moves left n spaces ：屏幕向左移动 */
                break;

            case 'H':		/* ESC [m;nH" moves cursor to (m,n) ：将光标移动到(m,n) */
                break;

            case 'J':		/* ESC [sJ clears in display ：清除屏幕显示 */
                switch (value) {
                    case 0:	/* Clear from cursor to end of screen ：清除从光标到屏幕末端 */
                        break;
                    case 1:	/* Clear from start of screen to cursor ：清除从屏幕开始到光标 */
                        break;
                    case 2:	/* Clear entire screen ：清屏 */
                        break;
                    default:	/* Do nothing ：啥也不做 */
                        count = 0;
                        break;
                }
                break;

            case 'K':		/* ESC [sK clears line from cursor ：清除光标处的行 */
                switch (value) {
                    case 0:	/* Clear from cursor to end of line ：从光标清除到行尾 */
                        break;
                    case 1:	/* Clear from beginning of line to cursor ：从行首清除到光标 */
                        break;
                    case 2:	/* Clear entire line ：清除整行，这行是光标所指行 */
                        break;
                    default:	/* Do nothing ： 啥也不做 */
                        break;
                }
                break;

            case 'L':		/* ESC [nL inserts n lines at cursor ：在光标处插入n行 */
                break;

            case 'M':		/* ESC [nM deletes n lines at cursor ：在光标处删除n行 */
                break;

            case '@':		/* ESC [n@ inserts n chars at cursor ：在光标处插入n个字符 */
                break;

            case 'P':		/* ESC [nP deletes n chars at cursor ：在光标处删除n个字符 */
                break;

            case 'm':		/* ESC [nm enables rendition n ：启用渲染n(0=正常，4=粗体，5=闪烁，7=反向) */
                break;
        }
    }
    /* 无效序列将会到这 */
    console->escape_state = FALSE;
}

/*===========================================================================*
 *				console_origin0				     *
 *	    滚动视频的内存，还原所有控制台的源地址为0
 *===========================================================================*/
PRIVATE void console_origin0(void){
    /* 滚动视频内存，使其原点为0
     * 本例程只有在F3键强制切换卷屏模式或准备关机时使用。
     * @TODO 有bug
     */

    int line;
    Console *console;
    unsigned int unused;

    for(line = 0; line < nr_console; line++){
        /* 得到控制台实例 */
        console = &console_table[line];
        while (console->origin > console->start){
            /* 未使用的内存量 */
            unused = video_size - screen_size;
            if(unused > console->origin - console->start){
                unused = console->origin - console->start;
            }
            video2video_copy(console->origin, console->origin - unused, screen_size);
            console->origin -= unused;
        }
        flush(console);
    }
    /* 激活控制台 */
    switch_to(current_console_nr);
}

/*===========================================================================*
 *				toggle_scroll				     *
 *				切换滚屏方式
 *===========================================================================*/
PUBLIC void toggle_scroll(void){
    /* 切换滚屏方式，硬件或者是软件。 */

    /* 重置控制台的基地址指针 */
//    console_origin0();
    software_scroll = !software_scroll;
    /* 打印一条提示信息 */
    printf("%sware scrolling enabled.\n", software_scroll ? "Soft" : "Hard");
}

/*===========================================================================*
 *				cons_stop				     *
 *	重新初始化控制台为重启监控程序指定的状态，优先于关机或重启
 *===========================================================================*/
PUBLIC void console_stop(void){
    console_origin0();
    software_scroll = TRUE;
    switch_to(0);
//    console_table[0].attr = console_table[0].blank = BLANK_COLOR;
}

/*===========================================================================*
 *				k_putk					     *
 *			输出一个字符到当前控制台
 *===========================================================================*/
PUBLIC void k_putk(int ch)
{
    /* 不需要通过文件系统而直接打印一个字符，内核和任务级别可使用
     * printk()使用这个过程，系统库中的printf()则需要向文件系统
     * 发送一条消息，这么低效的工作方式不是内核和任务所需要的，所
     * 以本例程只将输出的字符加入输出队列并输出。
     */

    if(ch != 0){
        /* 如果遇到了换行符，再输入一个'\r'，即输出'\n'时，实际上是输出"\r\n" */
        if(ch == '\n') out_char(&console_table[0], '\r');
        /* 只要还没到遇到字符串结束符号0，继续将字符加入到输出队列 */
        out_char(&console_table[0], ch);
    } else {
        /* 如果是一个字符串结束符号0，将输出队列冲洗到视频内存 */
        flush(&console_table[0]);
    }
}

/*===========================================================================*
 *				printk					     *
 *			格式打印一个字符串，只能被内核调用
 *===========================================================================*/
PUBLIC void printk(const char *fmt, ...){
    va_list argp;
    va_start(argp, fmt);
    redirect_printf(fmt, argp, &k_putk);      /* 实际的调用是库例程里的redirect_printf完成的 */
    va_end(argp);
}

/*===========================================================================*
 *				screen_init				     *
 *				屏幕初始化
 *===========================================================================*/
PUBLIC void screen_init(void){
    u16_t bios_columns, bios_crtbase, bios_font_lines;
    u8_t bios_rows;
    /* 获取描述视频显示单元的BIOS参数。 */
    phys_copy(BIOS_PARM_COLUMNS, vir2phys(&bios_columns), 2L);
    phys_copy(BIOS_PARM_CRTBASE, vir2phys(&bios_crtbase), 2L);
    phys_copy(BIOS_PARM_ROWS, vir2phys( &bios_rows), 1L);
    phys_copy(BIOS_PARM_FONTLINES, vir2phys(&bios_font_lines), 2L);

    /* 将得到的参数设置到全局变量，对于屏幕行数screen_lines需要注意：
     * 如果视频卡是EGA，则屏幕的行数就是检测到的bios_rows + 1，否则(VGA)是25。
     */
    video_port = bios_crtbase;
    screen_width = bios_columns;
    font_lines = bios_font_lines;
    screen_lines = ega ? bios_rows + 1 : 25;

    if(color) {
        video_base = COLOR_BASE;
        video_size = COLOR_SIZE;
    } else {
        video_base = MONO_BASE;
        video_size = MONO_SIZE;
    }
    if(ega) video_size = EGA_SIZE;
    /* 根据使用的视频控制器的类型设置wrap标志（用来确定如何滚屏），EGA不可以硬件重叠。 */
    wrap = !ega;
    /* 根据视频存储器基地得到并初始化段描述符 */
    video_size >>= 1;				/* 将按字节(byte)计数转换成按字(word)计数 */
    video_mask = video_size - 1;	/* 计算视频存储器的大小索引 */

    /* 计算屏幕大小（显示的总字符数） */
    screen_size = screen_lines * screen_width;
    /* 在视频内存允许的情况下，可以有尽可能多的控制台（但是受NR_CONSOLE配置选项的限制），我们分配它们。  */
    nr_console = video_size / screen_size;				    /* 首先，得到视频大小允许分配的最大的控制台数量 */
    if (nr_console > NR_CONSOLES) nr_console = NR_CONSOLES;	/* 当然了，配置项NR_CONSOLE限制其能开启的控制台数量 */
    if (nr_console > 1) wrap = FALSE;					    /* 如果有多个控制台被激活，那么不允许硬件重叠 */
}

/*===========================================================================*
 *				switch_to				     *
 *				切换控制台
 *===========================================================================*/
PUBLIC void switch_to(int line){
    /* 切换到一个不存在的控制台？那可不行！ */
    if(line < 0 || line >= nr_console) return;
    /* 设置当前使用的控制台 */
    current_console_nr = line;
    /* 设置该控制台为当前控制台 */
    curr_console = &console_table[line];
    /* 设置控制台的光标 */
    set_6845_cursor(curr_console->cursor);
    /* 设置控制台基地址，即控制台视频显存开始地址 */
    set_6845_video_start(curr_console->start);
}

/*===========================================================================*
 *				set_video_start				     *
 *				设置6845的显存开始区域
 *===========================================================================*/
PRIVATE void set_6845_video_start(
        unsigned int start_addr   /* 要设置的显存位置 */
){
    interrupt_lock();
    out_byte(video_port + INDEX, VIDEO_ORG);
    out_byte(video_port + DATA, (start_addr >> 8) & BYTE);
    out_byte(video_port + INDEX, VIDEO_ORG + 1);
    out_byte(video_port+ DATA, start_addr & BYTE);
    interrupt_unlock();
}

/*===========================================================================*
 *				set_6845_cursor				     *
 *			    设置6845的光标位置
 *===========================================================================*/
PRIVATE void set_6845_cursor(
        unsigned int position      /* 屏幕显示的位置，光标将会跟随更新 */
){
    /* 本例程其实可以和set_video_start合并为一个函数，但是为了代码可读性更好，还是分开吧！
     */
    interrupt_lock();
    out_byte(video_port + INDEX, CURSOR);
    out_byte(video_port + DATA, (position >> 8) & BYTE);
    out_byte(video_port + INDEX, CURSOR + 1);
    out_byte(video_port+ DATA, position & BYTE);
    interrupt_unlock();
}

/*===========================================================================*
 *				clear_screen					     *
 *			  清除控制台的屏幕输出
 *===========================================================================*/
PUBLIC void clear_screen(TTY *tty) {
    Console *console = tty->priv;
    if(console != curr_console) return; /* 如果不是当前控制台，no！ */
    blank_color = BLANK_COLOR;
    memory2video_copy(BLANK_MEM, curr_console->origin, screen_size);
    curr_console->row = curr_console->column = 0;
    flush(curr_console);
}

/*===========================================================================*
 *				memory2video_copy					     *
 *			     内存到显存复制
 *===========================================================================*/
PRIVATE void memory2video_copy(
        register u16_t *src,            /* 要复制到显存的word字串 */
        register unsigned int dest,     /* 目标，是显存中的相对位置 */
        unsigned int count              /* 要复制多少个字？ */
){
    /* 将一个字串（不是字符串）从核心的内存区域拷贝到视频显示器的存储器中（通俗讲就是显存）。
     * 该字串中包含替换字符码和若干属性字节 *
     */
    u16_t *video_memory = (u16_t*)(video_base + dest * 2);  /* 得到目标显存 */
    unsigned int i = 0;

    /* 如果字串是BLANK_MEM，执行清空整个屏幕空间 */
    if(src == BLANK_MEM){
//        blank_color = BLANK_COLOR;
        for(; i < screen_size; i++){   /* 整个屏幕 */
            *video_memory = blank_color;
            video_memory++;
        }
    } else {                            /* 移动src字串到显存，从dest相对位置开始，复制count个字 */
        while (count != 0){             /* 只要count != 0，一直复制 */
            *video_memory = src[i];
            video_memory++;
            i++;
            count--;
        }
    }
}

/*===========================================================================*
 *				video2video_copy					     *
 *			     显存内复制
 *===========================================================================*/
PRIVATE void video2video_copy(
        register unsigned int src,       /* 源，是显存中的相对位置 */
        register unsigned int dest,      /* 目标，是显存中的相对位置 */
        unsigned int count      /* 要复制多少个字？ */
){
    /* 显示器存储器(显存)内部的数据块拷贝。
     * 这里例程较为复杂，因为目的数据块可能与源数据块之间有重叠，
     * 并且数据移动的方向非常关键。 *
     */
    u16_t *src_video_memory = (u16_t*)(video_base + src * 2);  /* 得到目标显存 */
    u16_t *dest_video_memory = (u16_t*)(video_base + dest * 2);  /* 得到目标显存 */

    if(src < dest) {
        /* 目标比源小，那么说明是向下移动，即向高地址移动
         * 这个非常复杂，因为它们发生了重叠！暂时未实现！@TODO
         */
        while (count != 0){             /* 只要count != 0，一直复制 */
            *dest_video_memory = *src_video_memory;
            dest_video_memory++;
            src_video_memory++;
            count--;
        }
    } else {
        /* 是向上移动，即向低地址移动，这个很简单 */
        while (count != 0){             /* 只要count != 0，一直复制 */
            *dest_video_memory = *src_video_memory;
            dest_video_memory++;
            src_video_memory++;
            count--;
        }
    }
}











