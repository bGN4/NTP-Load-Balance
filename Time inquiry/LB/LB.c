////////////////////////////////////////b)负载均衡程序////////////////////////////////////////////////////////
/*
 * 作者     ：╳╳╳
 * 电子邮件 ：???@???.com
 * 完成时间 ：2013-07-08 23:58:00
 */
/*
 * 负载均衡程序，只要启动一个进程即可。此进程拥有一个唯一的id，绑定到两个不同的UDP端口上。
 * 一个UDP端口（下文称为client_udp_port）用于收发客户端的消息，
 * 一个UDP端口（下文称为server_udp_port）用于收发服务端的消息。
 * 负载均衡进程的id是多少，绑定的两个udp端口号是多少，支持多少个服务端，
 * 每个服务端的id、udp端口各是多少，均通过配置文件进行配置的。
 * 负载均衡进程启动时读入这些信息，运行过程中，不会改变。
 * 对于客户端有多少个，每个客户端的id是多少，UDP端口号是多少，
 * 负载均衡进程对这些信息是一无所知的，也是无法预测的。
 *
 * 负载均衡进程通过client_udp_port接收到客户端的“时间请求”消息后，
 * 如果消息中的dst_id不等于自己的id，就丢弃此消息。
 * 否则，就按照轮转算法选出一个服务端，
 * 将时间请求消息中的dst_id改成此服务端的id后，将消息通过server_udp_port分发给该服务端处理。
 * 负载均衡进程通过server_udp_port接收到客户端的“时间应答”消息后，
 * 将消息中的src_id改成自己的id，然后将消息通过client_udp_port发送给消息中的dst_id所指示的客户端。
 *
 * 负载均衡程序，需要具备一个调试开关。在运行过程中，可以打开/关闭调试开关。
 * 当调试开关打开后，程序需要将自己接收/发送的每一个消息，都实时显示给用户看。
 *
 * 负载均衡程序，需要具备统计功能。在运行过程中，可以随时可看，
 * 本进程从客户端接收了多少条消息（正确的多少条，错误的多少条），向客户端发送了多少条消息，
 * 从服务端接收了多少条消息（正确的多少条，错误的多少条），向服务端发送了多少条消息。
 *
 * 负载均衡程序，还需要具备日志功能。在运行过程中，如果出现异常事件（如UDP接收、发送失败等），
 * 需要记录日志，供后续分析查看。日志中，尽当尽可能包含详细的信息，如异常事件发生的时间、事件描述、事件原因等。
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../global.h"                                                /* 公共头文件 */
#include "function.h"
#include "LB.h"
HANDLE   Mutex_k,  Mutex_f,   *Mutex_c,  *hthread;                    /* 互斥锁、线程句柄 */
struct   Server    *sInfo     = NULL;                                 /* Server信息缓冲区指针 */
struct   Stat      stat_s     = {0, 0, 0};                            /* 下行数据统计信息 */
unsigned short     Cport = 0, Sport = 0;                              /* 绑定与Client和Server的通信端口 */
unsigned int       MAX_Ser    = 0;                                    /* Server数量 */
extern   struct    Mail  *UpLink;                                     /* 上行数据缓冲区指针 */
extern   struct    Mail  *DownLink;                                   /* 下行数据缓冲区指针 */
extern   HANDLE    SR,RC,CS,CR,RS,SC;                                 /* 信号量 */
DWORD    (WINAPI *f[7])(void*);                                       /* 线程函数指针 */
extern   void  (WINAPI* Balance)(unsigned int *);
extern   DWORD WINAPI Heartbeat(void *p);
/*************************************************************************************************************
 * __inline void Statistics()
 * 参数：
 * 功能：统计信息显示
 *************************************************************************************************************/
__inline void Statistics() {
    static COORD home = {0, 0};
    SetConsoleCursorPosition(hOut, home);
    printf("Statistics :  Sent         Receive      Correct      Wrong                     \n"
           "    UpLink    %-13u%-13u%-13u%-26u\n  DownLink    %-13u%-13u%-13u%-26u\n",
           stat.sent,stat.correct+stat.wrong,stat.correct,stat.wrong,
           stat_s.sent,stat_s.correct+stat_s.wrong,stat_s.correct,stat_s.wrong);
}
/*************************************************************************************************************
 * int syncInit()
 * 参数：
 * 功能：初始化信号量、互斥锁，初始化上、下行数据缓冲区
 *       此函数中任何一项的初始化失败都将导致程序退出，资源由操作系统回收
 *       所以没有考虑malloc内存的free问题
 *************************************************************************************************************/
int syncInit() {
    unsigned int i = 0;
    if(NULL == (Mutex_c  = (HANDLE*)malloc(MAX_Ser * sizeof(HANDLE)))) return 1;
    if(NULL == (hthread  = (HANDLE*)malloc(10 * sizeof(HANDLE)))) return 2;
    if(NULL == (UpLink   = (struct Mail *)malloc(BUFFER_MAX * sizeof(struct Mail)))) return 3;
    if(NULL == (DownLink = (struct Mail *)malloc(BUFFER_MAX * sizeof(struct Mail)))) return 4;
    if(NULL == (SC = CreateSemaphore(NULL,BUFFER_MAX,BUFFER_MAX,NULL)) ) return 5; /* 上行缓冲区空 */
    if(NULL == (CS = CreateSemaphore(NULL,BUFFER_MAX,BUFFER_MAX,NULL)) ) return 6; /* 下行缓冲区空 */
    if(NULL == (CR = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 7;   /* 唤醒C2S_rebuilder */
    if(NULL == (RS = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 8;   /* 唤醒ServerSender */
    if(NULL == (SR = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 9;   /* 唤醒S2C_rebuilder */
    if(NULL == (RC = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 10;  /* 唤醒ClientSender */
    if(NULL == (Mutex_k = CreateMutex(NULL,FALSE,NULL)) ) return 11;
    if(NULL == (Mutex_f = CreateMutex(NULL,FALSE,NULL)) ) return 12;
    for(i=0 ; i<MAX_Ser ; i++)
        if( NULL == (Mutex_c[i] = CreateMutex(NULL,FALSE,NULL)) ) return 13+i; /* 会话缓冲区互斥锁 */
    if( Alloc_Conversation(MAX_Ser) ) return -1;
    memset(UpLink  , 0, BUFFER_MAX * sizeof(struct Mail));
    memset(DownLink, 0, BUFFER_MAX * sizeof(struct Mail));
    for(i=0 ; i<BUFFER_MAX ; i++)
        UpLink[i].dst.sin_family   = UpLink[i].src.sin_family   = \
        DownLink[i].dst.sin_family = DownLink[i].src.sin_family = AF_INET;
    return 0;
}
/*************************************************************************************************************
 * int main(int argc, char* argv[])
 * 参数：argc : 没有任何作用
 *       argv : 没有任何作用
 * 功能：LB Server 的主函数
 *************************************************************************************************************/
int main(int argc, char* argv[]) {
    int i, ret = 0;
    /* 初始化屏幕显示、显示临界区 */
    if( ret = StartInit() ) return ret;
    /* 创建日志文件LB.log */
    if(NULL == (lg=fopen("LB.log","w"))) {
        SetConsoleTextAttribute(hOut, 12);
		printf("\nCan't open file \"LB.log\"!!!\n");
		QUIT(1);
	}
    /* 安装事件处理 */
    if( SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE ) {
        PRINT(STD_ERROR, STD_TIME, "Unable to install event handler!!!", "ErrorCode", GetLastError());
        QUIT(2);
    }
    /* 读取配置文件LB.cfg */
    if( ret = loadconfig(&sInfo) ) {
        fprintf(lg,STD_ERROR, STD_TIME, "Read config file error!!!", "ErrorCode", ret);
        INI_FAILED(ret);
        QUIT(3);
    }
    /* 将读取到的配置信息记入LOG文件 */
    showconfig(sInfo, lg);
    /* 设置标题栏 */
    sprintf(Title,"LB ID: %u",LB_ID);
    SetConsoleTitle(Title);
    /* 载入DLL文件 */
    if( LoadDllFunc("B.dll") ) {
        Balance = Circular;
    }
    /* 初始化信号量和互斥锁、缓冲区内存分配 */
    if( ret = syncInit()  )    {QUIT(5);}
    /* 绑定client_udp_port */
    ser.sin_port = htons(Cport);
    ser.sin_addr.s_addr = INADDR_ANY;
    if(NetworkInit(&sock))     {QUIT(6);}
    if(BindPort(&sock,&ser))   {QUIT(7);}
    /* 绑定server_udp_port */
    ser.sin_port = htons(Sport);
    if(NetworkInit(&sock_s))   {QUIT(8);}
    if(BindPort(&sock_s,&ser)) {QUIT(9);}
    /* 创建线程 */
    hthread[0] = CreateThread(NULL, 0, MoreInfo     , (void*)Statistics, 0, NULL);
    hthread[1] = CreateThread(NULL, 0, Cleaner      , (void*)0, 0, NULL);
    hthread[2] = CreateThread(NULL, 0, ClientRecever, (void*)0, 0, NULL);
    hthread[3] = CreateThread(NULL, 0, C2S_rebuilder, (void*)0, 0, NULL);
    hthread[4] = CreateThread(NULL, 0, ServerSender , (void*)0, 0, NULL);
    hthread[5] = CreateThread(NULL, 0, ServerRecever, (void*)0, 0, NULL);
    hthread[6] = CreateThread(NULL, 0, S2C_rebuilder, (void*)0, 0, NULL);
    hthread[7] = CreateThread(NULL, 0, ClientSender , (void*)0, 0, NULL);
    hthread[8] = CreateThread(NULL, 0, Heartbeat    , (void*)0, 0, NULL);
    printf("-------------------------------------------------------------------\n");
    /* 监视子线程工作状况 */
    for(;;) {
        Sleep(50);
        if( DETAIL == 1 ) Statistics();
        for(i=0 ; i<9 ; i++) {
            GetExitCodeThread(hthread[i],&ret);
            if( ret != STILL_ACTIVE ) {
                PRINT("%s %s <ERROR> hthread[%d] exit code:%d.\n", STD_TIME, i, ret);
                QUIT(-i);
            }
        }
    }
    QUIT(0);
}