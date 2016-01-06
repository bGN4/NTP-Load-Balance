#include <list>
#include "../global.h"
#include "errorlevel.h"
struct Chat {                       /* �Ự��Ϣ */
    unsigned int         usr_id;
    unsigned int         cli_id;
    unsigned int         Timeout;   /* �Ự��ʱ */
    unsigned long        s_ip;
    unsigned long        c_ip;
    unsigned short       s_port;
    unsigned short       c_port;
    bool operator== (const unsigned int &p) const { // ����==�����
        return usr_id==p;
	};
}Chat;
std::list<struct Chat> **Conversation = NULL;
std::list<struct Chat>::iterator Citf,Citd;
struct     Chat     chat;                                               /* �Ự��Ϣ */
extern "C" FILE     *lg;                                                /* LOG�ļ���� */
extern "C" HANDLE   *Mutex_c;                                           /* �ԻỰ������д�����Ļ����� */
extern "C" unsigned int     MAX_Ser;                                    /* Server ���� */
extern "C" struct   Server  *sInfo;                                     /* Server��Ϣ������ָ�� */
extern "C" inline   int  PickPosition(unsigned int MAX, unsigned int ser_id);
extern "C" void     Delete_k();
/*************************************************************************************************************
 * extern "C" int Alloc_Conversation(unsigned int MAX_Ser)
 * ������MAX_Ser : ����������
 * ���ܣ��Ự��������ʼ��
 *************************************************************************************************************/
extern "C" int Alloc_Conversation(unsigned int MAX_Ser) {
    unsigned int i = 0;
    Conversation = (std::list<struct Chat>**)malloc(MAX_Ser * sizeof(std::list<struct Chat>*));
    memset(Conversation, 0, MAX_Ser * sizeof(std::list<struct Chat>*));
    if( NULL == Conversation ) return 1;
    for(i=0 ; i<MAX_Ser ; i++) {
        Conversation[i] = new(std::list<struct Chat>);
        if( NULL == Conversation[i] ) return i+2;
    }
    return 0;
}
/*************************************************************************************************************
 * extern "C" int push_back_c(ID *pid, sockaddr_in *cli, sockaddr_in *ser, unsigned int pick)
 * ������*pid : struct ID
 *       *cli : Client ��ַ��Ϣ
 *       *ser : Server ��ַ��Ϣ
 *       pick :  sInfo ��λ��
 * ���ܣ����Ự��Ϣ����Ự������
 *************************************************************************************************************/
extern "C" int push_back_c(struct ID *pid, struct sockaddr_in *cli, struct sockaddr_in *ser, unsigned int pick) {
    chat.usr_id  = pid->usr_id;
    chat.cli_id  = pid->src_id;
    chat.Timeout = 0;
    chat.s_ip    = ser->sin_addr.s_addr;
    chat.s_port  = ser->sin_port;
    chat.c_ip    = cli->sin_addr.s_addr;
    chat.c_port  = cli->sin_port;
    if( WaitForSingleObject(Mutex_c[pick], INFINITE) )
        PRINT(STD_ERROR, STD_TIME, "Wait Mutex_c[pick] error!!!", "push_back_c", GetLastError());
    Conversation[pick]->push_back(chat);
    if( !ReleaseMutex(Mutex_c[pick]) )
        PRINT(STD_ERROR, STD_TIME, "Release Mutex_c[pick] error!!!", "push_back_c", GetLastError());
    return ( Conversation[pick]->begin() == Conversation[pick]->end() );
}
/*************************************************************************************************************
 * extern "C" int find_c(struct Packet *pkt, struct sockaddr_in *src, struct sockaddr_in *dst)
 * ������*pkt : Packet
 *       *src : Server�ĵ�ַ��Ϣ
 *       *dst : �ȴ���д��Client��ַ��Ϣ
 * ���ܣ��ӻỰ�������ж�λ��֮ƥ���Client��ַ������д��*dst��
 *************************************************************************************************************/
extern "C" int find_c(struct Packet *pkt, struct sockaddr_in *src, struct sockaddr_in *dst) {
    unsigned int i = 0, size = 0, ret = 1, pos = -1;
    pos = PickPosition(MAX_Ser, pkt->id.src_id);
    if( pos < 0 ) return -2;
    if( WaitForSingleObject(Mutex_c[pos], INFINITE) )
        PRINT(STD_ERROR, STD_TIME, "Wait Mutex_c[pos] error!!!", "find_c", GetLastError());
    size = Conversation[pos]->size();
    Citf = Conversation[pos]->begin();
    /* �ڻỰ�������в���ƥ���Client��Ϣ */
    for(i=0 ; i<size ; i++) {
        if( Citf == Conversation[pos]->end() ) return -1;
        if( Citf->usr_id == pkt->id.usr_id &&
            Citf->cli_id == pkt->id.dst_id &&
            Citf->s_ip   == src->sin_addr.s_addr &&
            Citf->s_port == src->sin_port) {       /* �ҵ��� */
            /* ��д*dst */
            dst->sin_port = Citf->c_port;
            dst->sin_addr.s_addr = Citf->c_ip;
            /* ɾ��Cit */
            Citf = Conversation[pos]->erase(Citf);
            ret  = 0;
            break;
        }
        if(Citf->Timeout++ <= MAX_Ser*TIMEOUT_COUNT) Citf++; /* ������ */
        else Citf = Conversation[pos]->erase(Citf);          /* �˻Ự��Ϣ��ʱ��ɾ�� */                                       
    }
    if( !ReleaseMutex(Mutex_c[pos]) )
        PRINT(STD_ERROR, STD_TIME, "Release Mutex_c[pos] error!!!", "find_c", GetLastError());
    return ret;
}
/*************************************************************************************************************
 * extern "C" void Delete_k()
 * ������
 * ���ܣ���������Ự�������еĳ�ʱ��Ϣ 
 *************************************************************************************************************/
extern "C" void Delete_p() {
    static unsigned int i,j,size;
    for(i=0 ; i<MAX_Ser ; i++) {
        j = WaitForSingleObject(Mutex_c[i], 100);
        if( WAIT_TIMEOUT == j ) continue;
        else if( j ) {
            PRINT(STD_ERROR, STD_TIME, "Wait Mutex_c[i] error!!!", "CCleaner", GetLastError());
        }
        size = Conversation[i]->size();
        Citd = Conversation[i]->begin();
        for(j=0 ; j<size ; j++) {
            if(Citd->Timeout++ <= MAX_Ser*TIMEOUT_COUNT) Citd++;
            else Citd = Conversation[i]->erase(Citd);
        }
        if( !ReleaseMutex(Mutex_c[i]) )
            PRINT(STD_ERROR, STD_TIME, "Release Mutex_c[i] error!!!", "CCleaner", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI Cleaner(void *p)
 * ������*p : û���κ�����
 * ���ܣ�����������еĳ�ʱ��Ϣ     
 *************************************************************************************************************/
extern "C" DWORD WINAPI Cleaner(void *p) {
    for(;;) {
        Sleep(1000);
        Delete_k();
        Sleep(1000);
        Delete_k();
        Delete_p();
    }
}