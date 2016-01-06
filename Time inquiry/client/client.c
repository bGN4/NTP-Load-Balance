////////////////////////////////////////c)客户端程序//////////////////////////////////////////////////////////
/*
 * 作者     ：╳╳╳
 * 电子邮件 ：???@???.com
 * 完成时间 ：2013-06-28 00:36:00
 */
/*
 * 客户端程序，需要起多个进程，每个进程拥有一个id，一个usr_id，绑定到一个默认分配的UDP端口上。
 * 每个客户端进程启动后，通过自己绑定的UDP端口向负载均衡器发送n条“时间请求”消息，并接收相应的时间应答消息。
 * 时间请求消息中的src_id填写自己的id，usr_id填写自己的usr_id，dst_id填写负载均衡器进程的id。
 *
 * 每个客户端进程的id、usr_id，发送的消息条数n，可以通过命令行参数传入，可以通过配置文件配置，
 * 也可以在进程启动时指定。三种方式，只要支持任意一种就行了。
 *
 * 注意，不同客户端进程的id、usr_id可以相同。
 *
 * 客户端进程，在运行过程中，需要将自己接收/发送的每一个消息，都实时显示给用户看。
 * 客户端进程完成自己的任务后，显示一下相关的统计信息，即可退出。
 * 统计信息包括本进程发送了多少条消息，接收了多少条消息（正确的多少条，错误的多少条）。
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../global.h"
HANDLE   hthread;
unsigned int MY_ID,USR_ID,COUNT;
/*************************************************************************************************************
 * void buildPacket(struct Packet *pkt, unsigned int msg_tp)
 * 参数：*pkt   : 被填写的数据包指针
 *       msg_tp : 请求类型
 * 功能：按照NTP协议的标准，构造标准的时间查询数据包，并在头部填写ID相关信息
 *************************************************************************************************************/
void buildPacket(struct Packet *pkt, unsigned int msg_tp) {
#define LI   3
#define VER  3
#define MODE 3
    SYSTEMTIME st;
    memset(pkt, 0, sizeof(struct Packet));
    pkt->id.src_id                       = MY_ID;  /* Client id */
    pkt->id.dst_id                       = LB_ID;  /* 发送目标 id */
    pkt->id.usr_id                       = USR_ID; /* Client usr_id */
    pkt->id.msg_tp                       = msg_tp; /* 时间请求(0) */
    pkt->ntpd.Flags                      = (LI<<6)|(VER<<3)|(MODE);
    pkt->ntpd.Peer_Clock_Stratum         = 0;
    pkt->ntpd.Peer_Polling_Interval      = 10; //
    pkt->ntpd.Peer_Clock_Precision       = -6;
    pkt->ntpd.Root_Delay                 = 1<<12; //htonl(1<<16)
    pkt->ntpd.Root_Dispersion            = 1<<12; //
    pkt->ntpd.Reference_Timestamp.coarse = htonl(JAN_1970);
    pkt->ntpd.Origin_Timestamp.coarse    = htonl(JAN_1970);
    pkt->ntpd.Receive_Timestamp.coarse   = htonl(JAN_1970);
    pkt->ntpd.Transmit_Timestamp.coarse  = htonl(time(NULL) + JAN_1970); //
    GetSystemTime(&st);
    pkt->ntpd.Transmit_Timestamp.fine    = htonl(NTPFRAC(1000*st.wMilliseconds));
}
/*************************************************************************************************************
 * DWORD WINAPI Recvpacket(void *s)
 * 参数：*s : SOCKET*
 * 功能：接收并显示数据包
 *************************************************************************************************************/
DWORD WINAPI Recvpacket(void *s) {
    for(;;){
        if(sizeof(rpk)!=recvfrom(*(SOCKET*)s,(char*)&rpk,sizeof(rpk),0,(struct sockaddr*)&ser,&addr_len)) continue;
        if(rpk.id.src_id==LB_ID && rpk.id.dst_id==MY_ID && rpk.id.usr_id==USR_ID && rpk.id.msg_tp==1) stat.correct++;
        else stat.wrong++;
        displayPacket(&rpk,&ser,0);
	}
}
/*************************************************************************************************************
 * int main(int argc, char* argv[])
 * 参数：argc    : 7
 *       argv[1] : Client ID
 *       argv[2] : usr_id
 *       argv[3] : LB ID
 *       argv[4] : 发送数量
 *       argv[5] : 目标IP
 *       argv[6] : 目标端口
 * 功能：Client主函数
 *************************************************************************************************************/
int main(int argc, char* argv[]) {
    unsigned int i;             /* 循环变量 */
    StartInit();
    if(argc < 7) goto usage;
    if((MY_ID  = atoi(argv[1])) <= 0) goto usage; /* Client ID */
    if((USR_ID = atoi(argv[2])) <= 0) goto usage; /* Client usr_id */
    if((LB_ID  = atoi(argv[3])) <= 0) goto usage; /* 发送目标 id */
    if((COUNT  = atoi(argv[4])) <= 0) goto usage; /* 发送数据包的数量 */
    /* 发送目标IP */
    if(INADDR_NONE == (ser.sin_addr.s_addr = inet_addr(argv[5]))) goto usage;
    /* 发送目标端口号 */
    ser.sin_port = htons((unsigned short)atoi(argv[6]));
    if(htons(ser.sin_port)<=0) goto usage;
    sprintf(Title,"Client ID: %s    usr_id: %s",argv[1],argv[2]);
    SetConsoleTitle(Title);
	if(NetworkInit(&sock)) {QUIT(-1);}
    hthread = CreateThread(NULL,0,Recvpacket,(void*)&sock,0,NULL);
    printf("-------------------------------------------------------------------\n");
    for(i=0 ; i<COUNT ; i++) { /* 发送数据包 */
        //Sleep(10);
        buildPacket(&pkt, 0);
        if(sizeof(pkt)!=sendto(sock,(char*)&pkt,sizeof(pkt),0,(struct sockaddr*)&ser,addr_len)) continue;
        stat.sent++;
        displayPacket(&pkt,&ser,1);
    }
    Sleep(3000);
    fprintf(stderr, "Statistics:\tSent = %u, Receive = %u, Correct = %u, Wrong = %u\n",
                     stat.sent,stat.correct+stat.wrong,stat.correct,stat.wrong);
    //getchar();
    closesocket(sock);
	WSACleanup();
    QUIT(0);
usage:
    fprintf(stderr,"\nUsage: %s [client_id] [usr_id] [LB_id] [count] [dst_ip] [dst_port]\n",argv[0]);
    QUIT(-255);
}