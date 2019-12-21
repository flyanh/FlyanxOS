/* The <stdarg.h> header is ANSI's way to handle variable numbers of params.
 * Some programming languages require a function that is declared with n
 * parameters to be called with n parameters.  C does not.  A function may
 * called with more parameters than it is declared with.  The well-known
 * printf function, for example, may have arbitrarily many parameters.
 * The question arises how one can access all the parameters in a portable
 * way.  The C standard defines three macros that programs can use to
 * advance through the parameter list.  The definition of these macros for
 * MINIX are given in this file.  The three macros are:
 *
 *	va_start(ap, parmN)	prepare to access parameters
 *	va_arg(ap, type)	get next parameter value and type
 *	va_end(ap)		access is finished
 *
 * Ken Thompson's famous line from V6 UNIX is equally applicable to this file:
 *
 *	"You are not expected to understand this"
 *
 * 本头文件是ANSI处理可变数量参数的方法。某些编程语言要求n个参数声明的函数需要用n个
 * 参数调用。但是c语言却没有，一个函数所调用的参数可能比声明的函数多。
 * 例如，众所周知的printf函数可以具有任意多个参数。出现了一个问题，即如何以一种可移
 * 植的方式访问所有参数？C标准定义了三个宏，程序可以使用它们来遍历参数列表。
 *
 * 这些宏的定义在此文件中给出。这三个宏是：
 *  va_start(ap, parmN)	    准备访问参数
 *	va_arg(ap, type)	    获取下一个参数值和类型
 *	va_end(ap)		        访问完成
 */

#ifndef _STDARG_H
#define _STDARG_H


#ifdef __GNUC__
/* GNU C编译器使用自己的但类似的varargs机制。 */

typedef char *va_list;

/* 参数列表中TYPE类型的arg所需的空间量。TYPE也可以是使用其类型的表达式。 */

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#if __GNUC__ < 2

/* GNU C编译器版本低于2 */

#ifndef __sparc__
#define va_start(AP, LASTARG)                                           \
 (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#else
#define va_start(AP, LASTARG)                                           \
 (__builtin_saveregs (),                                                \
  AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#endif

void va_end (va_list);          /* Defined in gnulib */
#define va_end(AP)

#define va_arg(AP, TYPE)                                                \
 (AP += __va_rounded_size (TYPE),                                       \
  *((TYPE *) (AP - __va_rounded_size (TYPE))))

#elif __GNUC__ >= 2 && __GNUC__ < 4	/* __GNUC__ >= 2 && __GNUC__ < 4  */

/* GNU C编译器版本2和3 */

#ifndef __sparc__
#define va_start(AP, LASTARG) 						\
 (AP = ((char *) __builtin_next_arg ()))
#else
#define va_start(AP, LASTARG)					\
  (__builtin_saveregs (), AP = ((char *) __builtin_next_arg ()))
#endif

void va_end (va_list);		/* 已经定义在libgcc.a中 */
#define va_end(AP)

#define va_arg(AP, TYPE)						\
 (AP = ((char *) (AP)) += __va_rounded_size (TYPE),			\
  *((TYPE *) ((char *) (AP) - __va_rounded_size (TYPE))))

#else   /* __GNUC__ >= 4 */

/* GNU C编译器版本高于4 */

/* Define __gnuc_va_list.  */

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#endif

/* Define the standard macros for the user,
   if this invocation was from the user program.  */
#ifdef _STDARG_H

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L \
    || __cplusplus + 0 >= 201103L
#define va_copy(d,s)	__builtin_va_copy(d,s)
#endif
#define __va_copy(d,s)	__builtin_va_copy(d,s)
#endif /* _STDARG_H */

#endif	/* __GNUC__ <= 2 */


#else	/* not __GNUC__ */

/* 未使用GNU C编译器，我们自己定义这些宏 */

typedef char *va_list;

#define __vasz(x)		((sizeof(x)+sizeof(int)-1) & ~(sizeof(int) -1))

#define va_start(ap, parmN)	((ap) = (va_list)&parmN + __vasz(parmN))
#define va_arg(ap, type)      \
  (*((type *)((va_list)((ap) = (void *)((va_list)(ap) + __vasz(type))) \
						    - __vasz(type))))
#define va_end(ap)

#endif /* __GNUC__ */

#endif /* _STDARG_H */
