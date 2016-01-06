////////////////////////////////////////b)���ؾ������////////////////////////////////////////////////////////
/*
 * ����     ���w�w�w
 * �����ʼ� ��???@???.com
 * ���ʱ�� ��2013-07-08 23:58:00
 */
/*
 * ���ؾ������ֻҪ����һ�����̼��ɡ��˽���ӵ��һ��Ψһ��id���󶨵�������ͬ��UDP�˿��ϡ�
 * һ��UDP�˿ڣ����ĳ�Ϊclient_udp_port�������շ��ͻ��˵���Ϣ��
 * һ��UDP�˿ڣ����ĳ�Ϊserver_udp_port�������շ�����˵���Ϣ��
 * ���ؾ�����̵�id�Ƕ��٣��󶨵�����udp�˿ں��Ƕ��٣�֧�ֶ��ٸ�����ˣ�
 * ÿ������˵�id��udp�˿ڸ��Ƕ��٣���ͨ�������ļ��������õġ�
 * ���ؾ����������ʱ������Щ��Ϣ�����й����У�����ı䡣
 * ���ڿͻ����ж��ٸ���ÿ���ͻ��˵�id�Ƕ��٣�UDP�˿ں��Ƕ��٣�
 * ���ؾ�����̶���Щ��Ϣ��һ����֪�ģ�Ҳ���޷�Ԥ��ġ�
 *
 * ���ؾ������ͨ��client_udp_port���յ��ͻ��˵ġ�ʱ��������Ϣ��
 * �����Ϣ�е�dst_id�������Լ���id���Ͷ�������Ϣ��
 * ���򣬾Ͱ�����ת�㷨ѡ��һ������ˣ�
 * ��ʱ��������Ϣ�е�dst_id�ĳɴ˷���˵�id�󣬽���Ϣͨ��server_udp_port�ַ����÷���˴���
 * ���ؾ������ͨ��server_udp_port���յ��ͻ��˵ġ�ʱ��Ӧ����Ϣ��
 * ����Ϣ�е�src_id�ĳ��Լ���id��Ȼ����Ϣͨ��client_udp_port���͸���Ϣ�е�dst_id��ָʾ�Ŀͻ��ˡ�
 *
 * ���ؾ��������Ҫ�߱�һ�����Կ��ء������й����У����Դ�/�رյ��Կ��ء�
 * �����Կ��ش򿪺󣬳�����Ҫ���Լ�����/���͵�ÿһ����Ϣ����ʵʱ��ʾ���û�����
 *
 * ���ؾ��������Ҫ�߱�ͳ�ƹ��ܡ������й����У�������ʱ�ɿ���
 * �����̴ӿͻ��˽����˶�������Ϣ����ȷ�Ķ�����������Ķ�����������ͻ��˷����˶�������Ϣ��
 * �ӷ���˽����˶�������Ϣ����ȷ�Ķ�����������Ķ��������������˷����˶�������Ϣ��
 *
 * ���ؾ�����򣬻���Ҫ�߱���־���ܡ������й����У���������쳣�¼�����UDP���ա�����ʧ�ܵȣ���
 * ��Ҫ��¼��־�������������鿴����־�У����������ܰ�����ϸ����Ϣ�����쳣�¼�������ʱ�䡢�¼��������¼�ԭ��ȡ�
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../global.h"                                                /* ����ͷ�ļ� */
#include "function.h"
#include "LB.h"
HANDLE   Mutex_k,  Mutex_f,   *Mutex_c,  *hthread;                    /* ���������߳̾�� */
struct   Server    *sInfo     = NULL;                                 /* Server��Ϣ������ָ�� */
struct   Stat      stat_s     = {0, 0, 0};                            /* ��������ͳ����Ϣ */
unsigned short     Cport = 0, Sport = 0;                              /* ����Client��Server��ͨ�Ŷ˿� */
unsigned int       MAX_Ser    = 0;                                    /* Server���� */
extern   struct    Mail  *UpLink;                                     /* �������ݻ�����ָ�� */
extern   struct    Mail  *DownLink;                                   /* �������ݻ�����ָ�� */
extern   HANDLE    SR,RC,CS,CR,RS,SC;                                 /* �ź��� */
DWORD    (WINAPI *f[7])(void*);                                       /* �̺߳���ָ�� */
extern   void  (WINAPI* Balance)(unsigned int *);
extern   DWORD WINAPI Heartbeat(void *p);
/*************************************************************************************************************
 * __inline void Statistics()
 * ������
 * ���ܣ�ͳ����Ϣ��ʾ
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
 * ������
 * ���ܣ���ʼ���ź���������������ʼ���ϡ��������ݻ�����
 *       �˺������κ�һ��ĳ�ʼ��ʧ�ܶ������³����˳�����Դ�ɲ���ϵͳ����
 *       ����û�п���malloc�ڴ��free����
 *************************************************************************************************************/
int syncInit() {
    unsigned int i = 0;
    if(NULL == (Mutex_c  = (HANDLE*)malloc(MAX_Ser * sizeof(HANDLE)))) return 1;
    if(NULL == (hthread  = (HANDLE*)malloc(10 * sizeof(HANDLE)))) return 2;
    if(NULL == (UpLink   = (struct Mail *)malloc(BUFFER_MAX * sizeof(struct Mail)))) return 3;
    if(NULL == (DownLink = (struct Mail *)malloc(BUFFER_MAX * sizeof(struct Mail)))) return 4;
    if(NULL == (SC = CreateSemaphore(NULL,BUFFER_MAX,BUFFER_MAX,NULL)) ) return 5; /* ���л������� */
    if(NULL == (CS = CreateSemaphore(NULL,BUFFER_MAX,BUFFER_MAX,NULL)) ) return 6; /* ���л������� */
    if(NULL == (CR = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 7;   /* ����C2S_rebuilder */
    if(NULL == (RS = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 8;   /* ����ServerSender */
    if(NULL == (SR = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 9;   /* ����S2C_rebuilder */
    if(NULL == (RC = CreateSemaphore(NULL,0,BUFFER_MAX,NULL)) ) return 10;  /* ����ClientSender */
    if(NULL == (Mutex_k = CreateMutex(NULL,FALSE,NULL)) ) return 11;
    if(NULL == (Mutex_f = CreateMutex(NULL,FALSE,NULL)) ) return 12;
    for(i=0 ; i<MAX_Ser ; i++)
        if( NULL == (Mutex_c[i] = CreateMutex(NULL,FALSE,NULL)) ) return 13+i; /* �Ự������������ */
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
 * ������argc : û���κ�����
 *       argv : û���κ�����
 * ���ܣ�LB Server ��������
 *************************************************************************************************************/
int main(int argc, char* argv[]) {
    int i, ret = 0;
    /* ��ʼ����Ļ��ʾ����ʾ�ٽ��� */
    if( ret = StartInit() ) return ret;
    /* ������־�ļ�LB.log */
    if(NULL == (lg=fopen("LB.log","w"))) {
        SetConsoleTextAttribute(hOut, 12);
		printf("\nCan't open file \"LB.log\"!!!\n");
		QUIT(1);
	}
    /* ��װ�¼����� */
    if( SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE ) {
        PRINT(STD_ERROR, STD_TIME, "Unable to install event handler!!!", "ErrorCode", GetLastError());
        QUIT(2);
    }
    /* ��ȡ�����ļ�LB.cfg */
    if( ret = loadconfig(&sInfo) ) {
        fprintf(lg,STD_ERROR, STD_TIME, "Read config file error!!!", "ErrorCode", ret);
        INI_FAILED(ret);
        QUIT(3);
    }
    /* ����ȡ����������Ϣ����LOG�ļ� */
    showconfig(sInfo, lg);
    /* ���ñ����� */
    sprintf(Title,"LB ID: %u",LB_ID);
    SetConsoleTitle(Title);
    /* ����DLL�ļ� */
    if( LoadDllFunc("B.dll") ) {
        Balance = Circular;
    }
    /* ��ʼ���ź����ͻ��������������ڴ���� */
    if( ret = syncInit()  )    {QUIT(5);}
    /* ��client_udp_port */
    ser.sin_port = htons(Cport);
    ser.sin_addr.s_addr = INADDR_ANY;
    if(NetworkInit(&sock))     {QUIT(6);}
    if(BindPort(&sock,&ser))   {QUIT(7);}
    /* ��server_udp_port */
    ser.sin_port = htons(Sport);
    if(NetworkInit(&sock_s))   {QUIT(8);}
    if(BindPort(&sock_s,&ser)) {QUIT(9);}
    /* �����߳� */
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
    /* �������̹߳���״�� */
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