////////////////////////////////////////a)����˳���//////////////////////////////////////////////////////////
/*
 * ����     ���w�w�w
 * �����ʼ� ��???@???.com
 * ���ʱ�� ��2013-06-30 23:42:00
 */
/*
 * ����˳�����Ҫ�������̣�ÿ������ӵ��һ��Ψһ��id���󶨵�һ��Ψһ��UDP�˿��ϡ�
 * ÿ������˽���ͨ���Լ��󶨵�UDP�˿ڽ��ա�ʱ��������Ϣ��
 * �����Ϣ�е�dst_id�����Լ���id������Զ˷��͡�ʱ��Ӧ����Ϣ�����򣬾Ͷ�������Ϣ��
 *
 * ÿ������˽��̵�id��udp�˿ںţ�����ͨ�������в������룬
 * ����ͨ�������ļ����ã�Ҳ�����ڽ�������ʱָ�������ַ�ʽ��ֻҪ֧������һ�־����ˡ�
 *
 * ����˳�����Ҫ�߱�һ�����Կ��ء������й����У����Դ�/�رյ��Կ��ء�
 * �����Կ��ش򿪺󣬷���˽�����Ҫ���Լ�����/���͵�ÿһ����Ϣ����ʵʱ��ʾ���û�����
 *
 * ����˳�����Ҫ�߱�ͳ�ƹ��ܡ������й����У�������ʱ�ɿ���
 * ÿ������˽��̽����˶�������Ϣ����ȷ�Ķ�����������Ķ���������Ӧ���˶�������Ϣ��
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../global.h"
HANDLE   hthread, hheart;
struct   Packet   hpk;
unsigned int      MY_ID;
/*************************************************************************************************************
 * void buildPacket(struct Packet *rpk, struct Packet *pkt, unsigned int msg_tp)
 * ������*rpk   : ���յ���ʱ���������ݰ�
 *       *pkt   : ����д�����ݰ�ָ��
 *       msg_tp : ���ݰ�����,��ֵֻ��Ϊ1(ʱ��Ӧ��)
 * ���ܣ�����NTPЭ��ı�׼�������׼��ʱ��Ӧ�����ݰ�������ͷ����дID�����Ϣ
 *************************************************************************************************************/
void buildPacket(struct Packet *rpk, struct Packet *pkt, unsigned int msg_tp) {
#define LI   0
#define VER  3
#define MODE 4
    SYSTEMTIME st;
    pkt->id.src_id                       = MY_ID;           /* Server id */
    pkt->id.dst_id                       = rpk->id.src_id;  /* copy from received src_id */
    pkt->id.usr_id                       = rpk->id.usr_id;  /* copy from received usr_id */
    pkt->id.msg_tp                       = msg_tp;          /* ʱ��Ӧ��(1) */
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
 * ������*hpk   : ����д�����ݰ�ָ��
 *       msg_tp : ���ݰ�����,��ֵֻ��Ϊ3(����Ӧ��)
 * ���ܣ���������Ӧ�����ݰ�
 *************************************************************************************************************/
__inline void buildheart(struct Packet *hpk, unsigned int msg_tp) {
    memset(hpk, 0, sizeof(struct Packet));
    hpk->id.src_id = MY_ID;
    hpk->id.dst_id = LB_ID;
    hpk->id.msg_tp = msg_tp;
}
/*************************************************************************************************************
 * __inline void Statistics()
 * ������
 * ���ܣ���ʾͳ����Ϣ
 *************************************************************************************************************/
__inline void Statistics() {
    printf("\r%14c%-13u%-13u%-13u%-26u",32,stat.sent,stat.correct+stat.wrong,stat.correct,stat.wrong);
}
/*************************************************************************************************************
 * DWORD WINAPI Heartbeat(void *s)
 * ������*s   : SOCKET
 * ���ܣ����ڷ�������Ӧ�����ݰ�
 *       ��֪��LB Server��IP�Ͷ˿ںŵ�����¿���������LB Server������������
 *       ������LB Server���������󻷽ڣ�ʱ�������ʵ�����¿��Լ���LB Server����
 *       �˹����ݲ�����
 *************************************************************************************************************/
DWORD WINAPI Heartbeat(void *s) {
    unsigned int ret[2] = {0, 0};
    return 0; /* �˹����ݲ����� */
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
 * ������argc    : 4
 *       argv[1] : Server ID
 *       argv[2] : LB ID
 *       argv[3] : �󶨱��ض˿�
 * ���ܣ�Server������
 *************************************************************************************************************/
int main(int argc, char* argv[]) {
    SYSTEMTIME mt;
    time_t     st;
    StartInit();
    /* ������ */
    if(argc < 4) goto usage;
    if((MY_ID = atoi(argv[1])) <= 0) goto usage; /* Server ID */
    if((LB_ID = atoi(argv[2])) <= 0) goto usage; /* LB Server id */
    /* Server port */
    ser.sin_port = htons((unsigned short)atoi(argv[3]));
    if( htons(ser.sin_port) <= 0 ) goto usage;
	ser.sin_addr.s_addr = INADDR_ANY;
    sprintf(Title,"Server ID: %s",argv[1]);
    SetConsoleTitle(Title);
    /* ��������ϣ���ʼ������ */
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
        /* �������� */
        if(rpk.id.dst_id == MY_ID && rpk.id.src_id == LB_ID && rpk.id.msg_tp == 2) {
            sendto(sock,(char*)&hpk,sizeof(hpk),0,(struct sockaddr*)&ser,addr_len);
            continue;
        }
        /* Ŀ��ID�����Լ�����ʱ���ѯ�� */
        if(rpk.id.dst_id != MY_ID || rpk.id.msg_tp != 0) {
            stat.wrong++;
            if(1==DETAIL) Statistics();
            continue;
        }
        /* �Ϸ���ʱ���ѯ�� */
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