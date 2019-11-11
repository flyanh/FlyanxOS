/* The <ansi.h> header attempts to decide whether the compiler has enough
 * conformance to Standard C for Minix to take advantage of.  If so, the
 * symbol _ANSI is defined (as 31415).  Otherwise _ANSI is not defined
 * here, but it may be defined by applications that want to bend the rules.
 * The magic number in the definition is to inhibit unnecessary bending
 * of the rules.  (For consistency with the new '#ifdef _ANSI" tests in
 * the headers, _ANSI should really be defined as nothing, but that would
 * break many library routines that use "#if _ANSI".)

 * If _ANSI ends up being defined, a macro
 *
 *	_PROTOTYPE(function, params)
 *
 * is defined.  This macro expands in different ways, generating either
 * ANSI Standard C prototypes or old-style K&R (Kernighan & Ritchie)
 * prototypes, as needed.  Finally, some programs use _CONST, _VOIDSTAR etc
 * in such a way that they are portable over both ANSI and K&R compilers.
 * The appropriate macros are defined here.
 */

/* 此文件作用是测试编译器是否符合ISO规定的标准C语言要求
 * 其标准C也被称之为ANSI C
 *
 * 整个头文件ansi.h的内容被包含在
 * #ifndef _ANSI_H
 * 和
 * #endif
 * 之间。在#ifndef _ANSI_H下面一行，_ANSI_H紧接着就被定义。
 * 这告诉编译器在一次编译中一个头文件应该仅被包含一次，这种写法保证了一个文件被包含多次时内容的正确性。
 * 我们将看到在 include/ 目录下的所用头文件都使用这种技术。
 */

#ifndef _ANSI_H
#define _ANSI_H

#if __STDC__ == 1
#define _ANSI		31459	/* compiler claims full ANSI conformance */
#endif

#ifdef __GNUC__
#define _ANSI		31459	/* gcc conforms enough even in non-ANSI mode */
#endif

#ifdef _ANSI

/* Keep everything for ANSI prototypes. */
/* 保持一切的函数原型声明 */
#define	_PROTOTYPE(function, params)	function params
#define	_ARGS(params)			params

#define	_VOIDSTAR	void *
#define	_VOID		void
#define	_CONST		const
#define	_VOLATILE	volatile
#define _SIZET		size_t

#else

/* Throw away the parameters for K&R prototypes. */
/* 不需要参数的函数原型声明 */
#define	_PROTOTYPE(function, params)	function()
#define	_ARGS(params)			()

#define	_VOIDSTAR	void *
#define	_VOID		void
#define	_CONST
#define	_VOLATILE
#define _SIZET		int

#endif /* _ANSI */

/* Setting any of _MINIX, _POSIX_C_SOURCE or _POSIX2_SOURCE implies
 * _POSIX_SOURCE.  (Seems wrong to put this here in ANSI space.)
 */
#if defined(_MINIX) || _POSIX_C_SOURCE > 0 || defined(_POSIX2_SOURCE)
#undef _POSIX_SOURCE
#define _POSIX_SOURCE	1
#endif

#endif /* ANSI_H */
