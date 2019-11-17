/* The <a.out> header file describes the format of executable files. */

/* 定义了可执行文件在磁盘上的存储格式，包括启动文件执行的文件
 * 的头结构和编译器产生的符号表。它只由文件系统引用。
 *
 * 本文件从Minix2操作系统中导入，本人懒...
 */

#ifndef _AOUT_H
#define _AOUT_H

/* a.out标头 */
struct	exec {
  unsigned char	a_magic[2];	/* 魔数 */
  unsigned char	a_flags;	/* 标志，请看下面 */
  unsigned char	a_cpu;		/* cpu编号 */
  unsigned char	a_hdrlen;	/* 标头长度 */
  unsigned char	a_unused;	/* 保留以备将来使用 */
  unsigned short a_version;	/* 版本标记（目前已不使用） */
  long		a_text;		    /* 文本段的大小（以字节为单位） */
  long		a_data;		    /* 数据段的大小（以字节为单位） */
  long		a_bss;		    /* bss段的大小（以字节为单位） */
  long		a_entry;	    /* 入口点 */
  long		a_total;	    /* 分配的总内存 */
  long		a_syms;		    /* 符号表的大小 */

  /* SHORT FORM ENDS HERE */
  long		a_trsize;	    /* 正文重定位大小 */
  long		a_drsize;	    /* 数据重定位大小 */
  long		a_tbase;	    /* 正文重定位基地址 */
  long		a_dbase;	    /* 数据重定位基地址 */
};

#define A_MAGIC0      (unsigned char) 0x01
#define A_MAGIC1      (unsigned char) 0x03
#define BADMAG(X)     ((X).a_magic[0] != A_MAGIC0 ||(X).a_magic[1] != A_MAGIC1)

/* CPU Id of TARGET machine (byte order coded in low order two bits) */
#define A_NONE	0x00	/* unknown */
#define A_I8086	0x04	/* intel i8086/8088 */
#define A_M68K	0x0B	/* motorola m68000 */
#define A_NS16K	0x0C	/* national semiconductor 16032 */
#define A_I80386 0x10	/* intel i80386 */
#define A_SPARC	0x17	/* Sun SPARC */

#define A_BLR(cputype)	((cputype&0x01)!=0) /* TRUE if bytes left-to-right */
#define A_WLR(cputype)	((cputype&0x02)!=0) /* TRUE if words left-to-right */

/* Flags. */
#define A_UZP	0x01	/* unmapped zero page (pages) */
#define A_PAL	0x02	/* page aligned executable */
#define A_NSYM	0x04	/* new style symbol table */
#define A_EXEC	0x10	/* executable */
#define A_SEP	0x20	/* separate I/D */
#define A_PURE	0x40	/* pure text */		/* not used */
#define A_TOVLY	0x80	/* text overlay */	/* not used */

/* Offsets of various things. */
#define A_MINHDR	32
#define	A_TEXTPOS(X)	((long)(X).a_hdrlen)
#define A_DATAPOS(X)	(A_TEXTPOS(X) + (X).a_text)
#define	A_HASRELS(X)	((X).a_hdrlen > (unsigned char) A_MINHDR)
#define A_HASEXT(X)	((X).a_hdrlen > (unsigned char) (A_MINHDR +  8))
#define A_HASLNS(X)	((X).a_hdrlen > (unsigned char) (A_MINHDR + 16))
#define A_HASTOFF(X)	((X).a_hdrlen > (unsigned char) (A_MINHDR + 24))
#define A_TRELPOS(X)	(A_DATAPOS(X) + (X).a_data)
#define A_DRELPOS(X)	(A_TRELPOS(X) + (X).a_trsize)
#define A_SYMPOS(X)	(A_TRELPOS(X) + (A_HASRELS(X) ? \
  			((X).a_trsize + (X).a_drsize) : 0))

struct reloc {
  long r_vaddr;			/* virtual address of reference */
  unsigned short r_symndx;	/* internal segnum or extern symbol num */
  unsigned short r_type;	/* relocation type */
};

/* r_tyep values: */
#define R_ABBS		0
#define R_RELLBYTE	2
#define R_PCRBYTE	3
#define R_RELWORD	4
#define R_PCRWORD	5
#define R_RELLONG	6
#define R_PCRLONG	7
#define R_REL3BYTE	8
#define R_KBRANCHE	9

/* r_symndx for internal segments */
#define S_ABS		((unsigned short)-1)
#define S_TEXT		((unsigned short)-2)
#define S_DATA		((unsigned short)-3)
#define S_BSS		((unsigned short)-4)

struct nlist {			/* symbol table entry */
  char n_name[8];		/* symbol name */
  long n_value;			/* value */
  unsigned char	n_sclass;	/* storage class */
  unsigned char	n_numaux;	/* number of auxiliary entries (not used) */
  unsigned short n_type;	/* language base and derived type (not used) */
};

/* Low bits of storage class (section). */
#define	N_SECT		  07	/* section mask */
#define N_UNDF		  00	/* undefined */
#define N_ABS		  01	/* absolute */
#define N_TEXT		  02	/* text */
#define N_DATA		  03	/* data */
#define	N_BSS		  04	/* bss */
#define N_COMM		  05	/* (common) */

/* High bits of storage class. */
#define N_CLASS		0370	/* storage class mask */
#define C_NULL
#define C_EXT		0020	/* external symbol */
#define C_STAT		0030	/* static */

/* Function prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int nlist, (char *_file, struct nlist *_nl)			);

#endif /* _AOUT_H */
