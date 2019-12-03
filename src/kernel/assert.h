/* POSIX标准要求必须有assert函数,它可以用来进行运行时测试、终止一个程序同时打印出一条消息。事实上,
 * POSIX要求include/ 目录下必须有一个assert.h 文件。那么为什么此处有另一个版本呢?答案是当用户进程
 * 出错时,可以依靠操作系统提供某些服务,比如在控制台上打印一条消息。但如果核心本身出错,则正常的系统
 * 资源就未必靠得住。于是核心提供其自己的例程来处理assert并打印消息,它独立于通常的系统库中的版本。
 */

#ifndef NDEBUG	/* 8086必须在没有轮询的情况下进行。 */
#define NDEBUG	(_WORD_SIZE == 2)
#endif

#if !NDEBUG

#define INIT_ASSERT	static char *assert_file= __FILE__;

void bad_assertion(char *file, int line, char *what);
void bad_compare(char *file, int line, int lhs, char *what, int rhs);

#define assert(x)	(!(x) ? bad_assertion(assert_file, __LINE__, #x) \
								: (void) 0)
#define compare(a,t,b)	(!((a) t (b)) ? bad_compare(assert_file, __LINE__, \
				(a), #a " " #t " " #b, (b)) : (void) 0)
#else /* NDEBUG */

#define INIT_ASSERT	/* nothing */

#define assert(x)	(void)0
#define compare(a,t,b)	(void)0

#endif /* NDEBUG */


