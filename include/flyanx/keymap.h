/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2019/11/17.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * 定义了若干结构，它们用来实现不同语言所需的字符集对应的特效键盘布局。
 * 它也被那些生成和加载表格的程序使用。
 * 此文件参考自MINIX。
 */

#ifndef FLYANX_KEYMAP_H
#define FLYANX_KEYMAP_H

#define	C(c)	((c) & 0x1F)	/* Map to control code		*/
#define A(c)	((c) | 0x80)	/* Set eight bit (ALT)		*/
#define CA(c)	A(C(c))		/* Control-Alt			*/
#define	L(c)	((c) | HASCAPS)	/* Add "Caps Lock has effect" attribute */

#define EXT	0x0100		/* Normal function keys		*/
#define CTRL	0x0200		/* Control key			*/
#define SHIFT	0x0400		/* Shift key			*/
#define ALT	0x0800		/* Alternate key		*/
#define EXTKEY	0x1000		/* extended keycode		*/
#define HASCAPS	0x8000		/* Caps Lock has effect ：大写锁定生效 */

/* Numeric keypad */
#define HOME	(0x01 + EXT)
#define END	(0x02 + EXT)
#define UP	(0x03 + EXT)
#define DOWN	(0x04 + EXT)
#define LEFT	(0x05 + EXT)
#define RIGHT	(0x06 + EXT)
#define PGUP	(0x07 + EXT)
#define PGDN	(0x08 + EXT)
#define MID	(0x09 + EXT)
#define NMIN	(0x0A + EXT)
#define PLUS	(0x0B + EXT)
#define INSERT	(0x0C + EXT)

/* Alt + Numeric keypad */
#define AHOME	(0x01 + ALT)
#define AEND	(0x02 + ALT)
#define AUP	(0x03 + ALT)
#define ADOWN	(0x04 + ALT)
#define ALEFT	(0x05 + ALT)
#define ARIGHT	(0x06 + ALT)
#define APGUP	(0x07 + ALT)
#define APGDN	(0x08 + ALT)
#define AMID	(0x09 + ALT)
#define ANMIN	(0x0A + ALT)
#define APLUS	(0x0B + ALT)
#define AINSRT	(0x0C + ALT)

/* Ctrl + Numeric keypad */
#define CHOME	(0x01 + CTRL)
#define CEND	(0x02 + CTRL)
#define CUP	(0x03 + CTRL)
#define CDOWN	(0x04 + CTRL)
#define CLEFT	(0x05 + CTRL)
#define CRIGHT	(0x06 + CTRL)
#define CPGUP	(0x07 + CTRL)
#define CPGDN	(0x08 + CTRL)
#define CMID	(0x09 + CTRL)
#define CNMIN	(0x0A + CTRL)
#define CPLUS	(0x0B + CTRL)
#define CINSRT	(0x0C + CTRL)

/* 锁定键 */
#define CALOCK	(0x0D + EXT)	/* 大写锁定	*/
#define	NLOCK	(0x0E + EXT)	/* 数字锁定	*/
#define SLOCK	(0x0F + EXT)	/* 滚动锁定	*/

/* Function keys */
#define F1	(0x10 + EXT)
#define F2	(0x11 + EXT)
#define F3	(0x12 + EXT)
#define F4	(0x13 + EXT)
#define F5	(0x14 + EXT)
#define F6	(0x15 + EXT)
#define F7	(0x16 + EXT)
#define F8	(0x17 + EXT)
#define F9	(0x18 + EXT)
#define F10	(0x19 + EXT)
#define F11	(0x1A + EXT)
#define F12	(0x1B + EXT)

/* Alt+Fn */
#define AF1	(0x10 + ALT)
#define AF2	(0x11 + ALT)
#define AF3	(0x12 + ALT)
#define AF4	(0x13 + ALT)
#define AF5	(0x14 + ALT)
#define AF6	(0x15 + ALT)
#define AF7	(0x16 + ALT)
#define AF8	(0x17 + ALT)
#define AF9	(0x18 + ALT)
#define AF10	(0x19 + ALT)
#define AF11	(0x1A + ALT)
#define AF12	(0x1B + ALT)

/* Ctrl+Fn */
#define CF1	(0x10 + CTRL)
#define CF2	(0x11 + CTRL)
#define CF3	(0x12 + CTRL)
#define CF4	(0x13 + CTRL)
#define CF5	(0x14 + CTRL)
#define CF6	(0x15 + CTRL)
#define CF7	(0x16 + CTRL)
#define CF8	(0x17 + CTRL)
#define CF9	(0x18 + CTRL)
#define CF10	(0x19 + CTRL)
#define CF11	(0x1A + CTRL)
#define CF12	(0x1B + CTRL)

/* Shift+Fn */
#define SF1	(0x10 + SHIFT)
#define SF2	(0x11 + SHIFT)
#define SF3	(0x12 + SHIFT)
#define SF4	(0x13 + SHIFT)
#define SF5	(0x14 + SHIFT)
#define SF6	(0x15 + SHIFT)
#define SF7	(0x16 + SHIFT)
#define SF8	(0x17 + SHIFT)
#define SF9	(0x18 + SHIFT)
#define SF10	(0x19 + SHIFT)
#define SF11	(0x1A + SHIFT)
#define SF12	(0x1B + SHIFT)


/* Alt+Shift+Fn */
#define ASF1	(0x10 + ALT + SHIFT)
#define ASF2	(0x11 + ALT + SHIFT)
#define ASF3	(0x12 + ALT + SHIFT)
#define ASF4	(0x13 + ALT + SHIFT)
#define ASF5	(0x14 + ALT + SHIFT)
#define ASF6	(0x15 + ALT + SHIFT)
#define ASF7	(0x16 + ALT + SHIFT)
#define ASF8	(0x17 + ALT + SHIFT)
#define ASF9	(0x18 + ALT + SHIFT)
#define ASF10	(0x19 + ALT + SHIFT)
#define ASF11	(0x1A + ALT + SHIFT)
#define ASF12	(0x1B + ALT + SHIFT)

#define MAP_COLS	    6	    /* 映射表的列数，列数越多，说明按键组合越多  */
#define NR_SCAN_CODES	0x80	/* 键盘扫描码数量，也是映射表的行数 */

typedef u16_t Keymap[NR_SCAN_CODES * MAP_COLS];   /* 键码映射表类型 */

#define KEY_MAGIC	"KMAZ"	/* 键码映射文件的模式 */

#endif //FLYANX_KEYMAP_H
