////////////////////////////////////////c)�ͻ��˳���//////////////////////////////////////////////////////////
/*
 * ����     ���w�w�w
 * �����ʼ� ��???@???.com
 * ���ʱ�� ��2013-06-28 00:36:00
 */
/*
 * �ͻ��˳�����Ҫ�������̣�ÿ������ӵ��һ��id��һ��usr_id���󶨵�һ��Ĭ�Ϸ����UDP�˿��ϡ�
 * ÿ���ͻ��˽���������ͨ���Լ��󶨵�UDP�˿����ؾ���������n����ʱ��������Ϣ����������Ӧ��ʱ��Ӧ����Ϣ��
 * ʱ��������Ϣ�е�src_id��д�Լ���id��usr_id��д�Լ���usr_id��dst_id��д���ؾ��������̵�id��
 *
 * ÿ���ͻ��˽��̵�id��usr_id�����͵���Ϣ����n������ͨ�������в������룬����ͨ�������ļ����ã�
 * Ҳ�����ڽ�������ʱָ�������ַ�ʽ��ֻҪ֧������һ�־����ˡ�
 *
 * ע�⣬��ͬ�ͻ��˽��̵�id��usr_id������ͬ��
 *
 * �ͻ��˽��̣������й����У���Ҫ���Լ�����/���͵�ÿһ����Ϣ����ʵʱ��ʾ���û�����
 * �ͻ��˽�������Լ����������ʾһ����ص�ͳ����Ϣ�������˳���
 * ͳ����Ϣ���������̷����˶�������Ϣ�������˶�������Ϣ����ȷ�Ķ�����������Ķ���������
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../global.h"
HANDLE   hthread;
unsigned int MY_ID,USR_ID,COUNT;
/*************************************************************************************************************
 * void buildPacket(struct Packet *pkt, unsigned int msg_tp)
 * ������*pkt   : ����д�����ݰ�ָ��
 *       msg_tp : ��������
 * ���ܣ�����NTPЭ��ı�׼�������׼��ʱ���ѯ���ݰ�������ͷ����дID�����Ϣ
 *************************************************************************************************************/
void buildPacket(struct Packet *pkt, unsigned int msg_tp) {
#define LI   3
#define VER  3
#define MODE 3
    SYSTEMTIME st;
    memset(pkt, 0, sizeof(struct Packet));
    pkt->id.src_id                       = MY_ID;  /* Client id */
    pkt->id.dst_id                       = LB_ID;  /* ����Ŀ�� id */
    pkt->id.usr_id                       = USR_ID; /* Client usr_id */
    pkt->id.msg_tp                       = msg_tp; /* ʱ������(0) */
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
 * ������*s : SOCKET*
 * ���ܣ����ղ���ʾ���ݰ�
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
 * ������argc    : 7
 *       argv[1] : Client ID
 *       argv[2] : usr_id
 *       argv[3] : LB ID
 *       argv[4] : ��������
 *       argv[5] : Ŀ��IP
 *       argv[6] : Ŀ��˿�
 * ���ܣ�Client������
 *************************************************************************************************************/
int main(int argc, char* argv[]) {
    unsigned int i;             /* ѭ������ */
    StartInit();
    if(argc < 7) goto usage;
    if((MY_ID  = atoi(argv[1])) <= 0) goto usage; /* Client ID */
    if((USR_ID = atoi(argv[2])) <= 0) goto usage; /* Client usr_id */
    if((LB_ID  = atoi(argv[3])) <= 0) goto usage; /* ����Ŀ�� id */
    if((COUNT  = atoi(argv[4])) <= 0) goto usage; /* �������ݰ������� */
    /* ����Ŀ��IP */
    if(INADDR_NONE == (ser.sin_addr.s_addr = inet_addr(argv[5]))) goto usage;
    /* ����Ŀ��˿ں� */
    ser.sin_port = htons((unsigned short)atoi(argv[6]));
    if(htons(ser.sin_port)<=0) goto usage;
    sprintf(Title,"Client ID: %s    usr_id: %s",argv[1],argv[2]);
    SetConsoleTitle(Title);
	if(NetworkInit(&sock)) {QUIT(-1);}
    hthread = CreateThread(NULL,0,Recvpacket,(void*)&sock,0,NULL);
    printf("-------------------------------------------------------------------\n");
    for(i=0 ; i<COUNT ; i++) { /* �������ݰ� */
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