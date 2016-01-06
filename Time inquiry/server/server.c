////////////////////////////////////////a)服务端程序//////////////////////////////////////////////////////////
/*
 * 作者     ：╳╳╳
 * 电子邮件 ：???@???.com
 * 完成时间 ：2013-06-30 23:42:00
 */
/*
 * 服务端程序，需要起多个进程，每个进程拥有一个唯一的id，绑定到一个唯一的UDP端口上。
 * 每个服务端进程通过自己绑定的UDP端口接收“时间请求”消息，
 * 如果消息中的dst_id等于自己的id，就向对端发送“时间应答”消息。否则，就丢弃此消息。
 *
 * 每个服务端进程的id、udp端口号，可以通过命令行参数传入，
 * 可以通过配置文件配置，也可以在进程运行时指定。三种方式，只要支持任意一种就行了。
 *
 * 服务端程序，需要具备一个调试开关。在运行过程中，可以打开/关闭调试开关。
 * 当调试开关打开后，服务端进程需要将自己接收/发送的每一个消息，都实时显示给用户看。
 *
 * 服务端程序，需要具备统计功能。在运行过程中，可以随时可看，
 * 每个服务端进程接收了多少条消息（正确的多少条，错误的多少条），应答了多少条消息。
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../global.h"
HANDLE   hthread, hheart;
struct   Packet   hpk;
unsigned int      MY_ID;
/*************************************************************************************************************
 * void buildPacket(struct Packet *rpk, struct Packet *pkt, unsigned int msg_tp)
 * 参数：*rpk   : 接收到的时间请求数据包
 *       *pkt   : 被填写的数据包指针
 *       msg_tp : 数据包类型,其值只能为1(时间应答)
 * 功能：按照NTP协议的标准，构造标准的时间应答数据包，并在头部填写ID相关信息
 *************************************************************************************************************/
void buildPacket(struct Packet *rpk, struct Packet *pkt, unsigned int msg_tp) {
#define LI   0
#define VER  3
#define MODE 4
    SYSTEMTIME st;
    pkt->id.src_id                       = MY_ID;           /* Server id */
    pkt->id.dst_id                       = rpk->id.src_id;  /* copy from received src_id */
    pkt->id.usr_id                       = rpk->id.usr_id;  /* copy from received usr_id */
    pkt->id.msg_tp                       = msg_tp;          /* 时间应答(1) */
    pkt->ntpd.Flags                      = (LI<<6)|(VER<<3)|(MODE);
    pkt->ntpd.Peer_Clock_Stratum         = 5;
    pkt->ntpd.Peer_Polling_Interval      = 10; //
    pkt->ntpd.Peer_Clock_Precision       = -6;
    pkt->ntpd.Root_Delay                 = 1<<12; //htonl(1<<16)
    pkt->ntpd.Root_Dispersion            = 1<<12; //
    pkt->ntpd.Reference_Timestamp        = pkt->ntpd.Receive_Timestamp;
    pkt->ntpd.Origin_Timestamp           = rpk->ntpd.Transmit_Timestamp;
    pkt->ntpd.Transmit_Timestamp.coarse  = htonl(time(NULL) + JAN_1970); //
    GetSystemTime(&st);
    pkt->ntpd.Transmit_Timestamp.fine    = htonl(NTPFRAC(1000*st.wMilliseconds));
}
/*************************************************************************************************************
 * __inline void buildheart(struct Packet *hpk, unsigned int msg_tp)
 * 参数：*hpk   : 被填写的数据包指针
 *       msg_tp : 数据包类型,其值只能为3(心跳应答)
 * 功能：构造心跳应答数据包
 *************************************************************************************************************/
__inline void buildheart(struct Packet *hpk, unsigned int msg_tp) {
    memset(hpk, 0, sizeof(struct Packet));
    hpk->id.src_id = MY_ID;
    hpk->id.dst_id = LB_ID;
    hpk->id.msg_tp = msg_tp;
}
/*************************************************************************************************************
 * __inline void Statistics()
 * 参数：
 * 功能：显示统计信息
 *************************************************************************************************************/
__inline void Statistics() {
    printf("\r%14c%-13u%-13u%-13u%-26u",32,stat.sent,stat.correct+stat.wrong,stat.correct,stat.wrong);
}
/*************************************************************************************************************
 * DWORD WINAPI Heartbeat(void *s)
 * 参数：*s   : SOCKET
 * 功能：周期发送心跳应答数据包
 *       在知道LB Server的IP和端口号的情况下可以主动向LB Server发送心跳包，
 *       避免了LB Server的心跳请求环节，时间间隔合适的情况下可以减轻LB Server负担
 *       此功能暂不启用
 *************************************************************************************************************/
DWORD WINAPI Heartbeat(void *s) {
    unsigned int ret[2] = {0, 0};
    return 0; /* 此功能暂不启用 */
    for(;;) {
        Sleep(HEARTBEAT_PERIOD);
        ret[1] = stat.sent;
        if( ret[1]-ret[0] ) {
            ret[0] = ret[1];
            continue;
        }
        buildheart(&hpk, 3);
        sendto(*(SOCKET*)s,(char*)&hpk,sizeof(hpk),0,(struct sockaddr*)&ser,sizeof(ser));
    }
}
/*************************************************************************************************************
 * int main(int argc, char* argv[])
 * 参数：argc    : 4
 *       argv[1] : Server ID
 *       argv[2] : LB ID
 *       argv[3] : 绑定本地端口
 * 功能：Server主函数
 *************************************************************************************************************/
int main(int argc, char* argv[]) {
    SYSTEMTIME mt;
    time_t     st;
    StartInit();
    /* 检查参数 */
    if(argc < 4) goto usage;
    if((MY_ID = atoi(argv[1])) <= 0) goto usage; /* Server ID */
    if((LB_ID = atoi(argv[2])) <= 0) goto usage; /* LB Server id */
    /* Server port */
    ser.sin_port = htons((unsigned short)atoi(argv[3]));
    if( htons(ser.sin_port) <= 0 ) goto usage;
	ser.sin_addr.s_addr = INADDR_ANY;
    sprintf(Title,"Server ID: %s",argv[1]);
    SetConsoleTitle(Title);
    /* 检查参数完毕，初始化网络 */
    if(NetworkInit(&sock)) {QUIT(-1);}
    if(BindPort(&sock,&ser)) {QUIT(-2);}
    hthread = CreateThread(NULL,0,MoreInfo,(void*)Statistics,0,NULL);
    hheart  = CreateThread(NULL,0,Heartbeat,(void*)&sock,0,NULL);
    printf("-------------------------------------------------------------------\n");
    buildheart(&hpk, 3);
    for(;;) {
        if(sizeof(rpk)!=recvfrom(sock,(char*)&rpk,sizeof(rpk),0,(struct sockaddr*)&ser,&addr_len)) continue;
        st = time(NULL);
        GetSystemTime(&mt);
        /* 心跳请求 */
        if(rpk.id.dst_id == MY_ID && rpk.id.src_id == LB_ID && rpk.id.msg_tp == 2) {
            sendto(sock,(char*)&hpk,sizeof(hpk),0,(struct sockaddr*)&ser,addr_len);
            continue;
        }
        /* 目标ID不是自己或不是时间查询包 */
        if(rpk.id.dst_id != MY_ID || rpk.id.msg_tp != 0) {
            stat.wrong++;
            if(1==DETAIL) Statistics();
            continue;
        }
        /* 合法的时间查询包 */
        stat.correct++;
        if(2==DETAIL) displayPacket(&rpk,&ser,0);
        else if(1==DETAIL) Statistics();
        memset(&pkt, 0, sizeof(struct Packet));
        pkt.ntpd.Receive_Timestamp.coarse = htonl(st + JAN_1970);
        pkt.ntpd.Receive_Timestamp.fine   = htonl(NTPFRAC(1000*mt.wMilliseconds));
        buildPacket(&rpk,&pkt,1);
        if(sizeof(pkt)==sendto(sock,(char*)&pkt,sizeof(pkt),0,(struct sockaddr*)&ser,addr_len)) {
            stat.sent++;
            if(2==DETAIL) displayPacket(&pkt,&ser,1);
            else if(1==DETAIL) Statistics();
        }
    }
	closesocket(sock);
	WSACleanup();
    QUIT(0);
usage:
    fprintf(stderr,"\nUsage: %s [server_id] [LB_id] [port]\n",argv[0]);
    QUIT(-255);
}