/* Copyright (C) 2007 Free Software Foundation, Inc. 
 * See the copyright notice in the file /usr/LICENSE.
 * Created by flyan on 2020/2/3.
 * QQ: 1341662010
 * QQ-Group:909830414
 * gitee: https://gitee.com/flyanh/
 *
 * Flyanx默认的Shell，fly_shell，简称Flsh。
 * 它包含一个简单的登录器以及命令解释器。
 * 输入的命令先会看看是不是内键命令，是则执行。
 * 不是内建命令则当做一个可执行文件打开，如果文件不存在提示命令不存在，否则打开并执行该文件。
 */
#define _POSIX_SOURCE   /* 需要POSIX标准的一些支持 */

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h>

#define USER_FILE "users"
#define DEFAULT_UNAME "admin"
#define DEFAULT_PWD "flyanx"

#define BUF_SIZE 256        /* 缓冲区大小 */
#define UNAME_LEN  21     /* 用户名最大长度 */
#define PWD_LEN  32       /* 密码最大长度 */
#define CIPHERTEXT_LEN (PWD_LEN * 2)  /* 密码密文最大长度 */

/* 用户信息：密码存放密文 */
typedef struct user_s{
    struct user_s *next;
    char *uname;
    char *encode_pwd;
} User;

typedef struct login_state_s{
    bool ok;            /* 登录成功否？ */
    char *login_user;   /* 登录用户 */
} LoginState ;

static LoginState loginTable[8];  /* 控制台登录校验表 */

//static char buf[SECTOR_SIZE * 16];

static void login(int tty_index);
static void command(int tty_index);
static char *itoap(int num, int radix);
static char *encryption(char *text);

/*===========================================================================*
 *                            fly_shell                                        *
 *                           Fly命令行Shell             *
 *===========================================================================*/
void fly_shell(const char * tty_name, int tty_index){
    /* 打开要使用的终端，分别打开输入输出流 */
    int fd_stdin = open(tty_name, O_RDWR);
    int fd_stout = open(tty_name, O_RDWR);

    /* 进行登录验证 */
    login(tty_index);

    /* 打开命令解释器 */
    command(tty_index);
}

/*===========================================================================*
 *                            command                                        *
 *                           简单的命令解释器             *
 *===========================================================================*/
static void command(int tty_index){
    /* 读写缓冲区 */
    char rdwt_buf[BUF_SIZE];
    while (1){
        /* 打印提示信息 */
        printf("%s@flyanx # ", loginTable[tty_index].login_user);
        /* 接收命令 */
        int rlen = read(STDIN, rdwt_buf, BUF_SIZE);
        rdwt_buf[--rlen] = 0;     /* 长度-1：回车键会产生一个换行符，我们不需要这个换行符！ */
        /* 得到命令参数，调整用户堆栈，为运行做准备 */
        int argc = 0;
        char * argv[_POSIX_ARG_MAX];
        char * p = rdwt_buf;
        char * s;
        int word = 0;
        char ch;
        do {
            ch = *p;
            if (*p != ' ' && *p != 0 && !word) {
                s = p;
                word = 1;
            }
            if ((*p == ' ' || *p == 0) && word) {
                word = 0;
                argv[argc++] = s;
                *p = 0;
            }
            p++;
        } while(ch);
        argv[argc] = 0;

        /* 处理内建命令 */
        /* exit：退出shell */
        if(strcmp(argv[0], "exit") == 0){
            loginTable[tty_index].ok = 0;   /* 消除登录状态 */
            printf("bye~see you agin.\n");
            close(STDIN);
            close(STDOUT);
            exit(0);
        }
        /* strlen: 检查字符串长度 */
        if(strcmp(argv[0], "strlen") == 0){
            int diff = 7;    /* 差值："strlen "的长度 */
            printf("string length --> %d\n", rlen - diff);
            continue;
        }

        /* 处理可执行文件 */
        int fd = open(argv[0], O_RDWR);
        int pid;
        if(fd == -1){   /* 可执行文件不存在：提示用户命令不存在 */
            if(rdwt_buf[0]){
                printf("flsh: commond not found --> %s\n", argv[0]);
            }
        } else {        /* 存在可执行文件：执行这个文件 */
            close(fd);  /* 关闭文件 */
            if((pid = fork()) != 0){    /* 产生分支子进程 */
                /* 父进程执行的代码 */
                int s;
                wait(&s);               /* 等待子进程结束 */
            } else {
                /* 分支子进程执行代码 */
                execv(argv[0], argv);   /* 执行文件 */
            }
        }
    }
}

/*===========================================================================*
 *                            login                                        *
 *                           简单的登录模块             *
 *===========================================================================*/
void login(int tty_index){
    char uname[UNAME_LEN];
    char pwd[PWD_LEN];
    ssize_t uname_len, pwd_len;

    /* 看看当前控制台的登录状态 */
    if(loginTable[tty_index].ok == 1) return;

    /* 打印登录提示信息 */
    while (1){
        printf("Flyanx 0.01 tty%d\n",  tty_index);
        printf("Login: ");

        /* 输入用户登录信息 */
        uname_len = read(STDIN, uname, UNAME_LEN);  /* 输入用户名 */
        uname[uname_len - 1] = 0;    /* 长度-1：回车键会产生一个换行符，我们不需要这个换行符！ */
        printf("Password: ");
        pwd_len = read(STDIN, pwd, PWD_LEN);     /* 输入密码 */
        pwd[pwd_len - 1] = 0;

        /* 登录信息是否正确？ */
        if(strcmp(uname, DEFAULT_UNAME) != 0 || strcmp(pwd, DEFAULT_PWD) != 0){
            /* 错误 */
            printf("Login incorrect, retry!\n\n");
            continue;
        }
        /* 正确，打印登录成功信息，返回登录用户 */
        printf("Welcome %s!\n\n", DEFAULT_UNAME);
        loginTable[tty_index].ok = 1;
        loginTable[tty_index].login_user = DEFAULT_UNAME;
        return;
    }

    /* 读取登录文件中的所有用户信息 @TODO */
//    int fd;
//
//    if((fd = open(USER_FILE, O_RDWR | O_CREAT | O_TRUNC)) == -1){    /* 打开文件，不存在则创建 */
//        /* 打开并创建失败 */
//        printf("failed to open file: %s\n", USER_FILE);
//        close(fd);
//    }
//    Stat uf_stat;   /* 得到登录文件的信息 */
//    fstat(fd, &uf_stat);
//    int left = uf_stat.size;    /* 读取剩余 */
//    int chunk = sizeof(buf);        /* 一块 = 8KB，用于读写 */
//    int i = 0;
//    int bytes = 0;
//    while(left){
//        int io_bytes = chunk < left ? chunk : left;
//        read(fd, buf,
//             ((io_bytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
//        left -= io_bytes;
//    }




}

/*===========================================================================*
 *                            encryption                                        *
 *                          一个简单的加密             *
 *===========================================================================*/
static char *encryption(char *text){
    char ciphertext[CIPHERTEXT_LEN], prior[CIPHERTEXT_LEN];
    int clen = 0;   /* 加密密文长度 */

    int i;
    char c;
    char *encode16;
    for(i = 0; i < PWD_LEN; i++) {
        c = text[i];
        if(c == 0){
            /* 结束 */
            prior[clen] = 0;
            break;
        }
        if (c != ' ') {
            encode16 = itoap(c, 16);
            prior[clen] = encode16[2]; /* 16禁止的0x我们不需要，所以索引是从2开始的 */
            prior[++clen] = encode16[3];
            if (i != PWD_LEN - 1) {
                prior[++clen] = ' ';
            }
        }
    }

    /* 反转 */
    for(i = clen - 1; i >= 0; i--){
        ciphertext[clen - i - 1] = prior[i];
    }
    ciphertext[clen] = 0;

    return ciphertext;
}

/* 整数转字符串,可以转不同进制 */
static char* itoap(
        int num,    /* 整型数 */
        int radix   /* 转换进制 */
){
    char str[8];
    /* 索引表 */
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;  /* 中间变量 */
    int i = 0, j, k;

    /* 确定unum的值 */
    if(radix == 10 && num < 0){/* 十进制负数 */
        unum = (unsigned) - num;
        str[i++] = '-';
    } else
        unum = (unsigned)num;/* 其他情况 */
    /* 逆序 */
    do{
        str[i++] = index[unum % (unsigned)radix];
        unum /= radix;
    } while(unum);

    /* 如果不是十进制,加前缀 */
    if(radix == 2){
        str[i++] = 'b';
        str[i++] = '0';
    } else if (radix == 8)
    {
        str[i++] = 'o';
        str[i++] = '0';
    } else if (radix == 16)
    {
        str[i++] = 'x';
        str[i++] = '0';
    }


    str[i] = '\0';
    /* 转换 */
    if(str[0] == '-')
        k = 1;/* 十进制负数 */
    else
        k = 0;
    /* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */
    for(j = k;j < (i-1) / 2.0 + k; j++){
        num = str[j];
        str[j] = str[i - j - 1 + k];
        str[i - j - 1 + k] = num;
    }
    return str;
}


