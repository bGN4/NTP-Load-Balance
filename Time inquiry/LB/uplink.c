#include "LB.h"
HANDLE   CR,RS,SC;
struct   Mail   *UpLink    = NULL;                                 /* �������ݻ�����ָ�� */
unsigned int    u_push_ptr = 0; u_build_ptr = 0, u_pop_ptr = 0;    /* �������ݿ����α� */
unsigned int    OffSet;
extern   int    *weigh;
extern   struct Mail  *DownLink;
extern volatile int   AllSerDown;
extern unsigned int   d_pop_ptr;
extern          void  (WINAPI* Balance)(unsigned int *);
__inline void   PickBuffer(unsigned int *previous);
         int    push_back_c(struct ID *, struct sockaddr_in *, struct sockaddr_in *, unsigned int);
unsigned int    Select_k(unsigned int id);
         int    Insert_k(unsigned int id, unsigned int value);
/*************************************************************************************************************
 * __inline int push_u(struct Packet *pkt,struct sockaddr_in *src)
 * ������*pkt : ��Client���յ������ݰ���ָ��
 *       *src : Client��ַ��ָ��
 * ���ܣ�����Client���յ������ݰ���Client�ĵ�ַ��Ϣд�����л�����
 *************************************************************************************************************/
__inline int push_u(struct Packet *pkt,struct sockaddr_in *src) {
    PickBuffer(&u_push_ptr);
    if( 0 != UpLink[u_push_ptr].stat )
        return UpLink[u_push_ptr].stat;
    UpLink[u_push_ptr].pkt = *pkt;
    UpLink[u_push_ptr].src.sin_port = src->sin_port;
    UpLink[u_push_ptr].src.sin_addr.s_addr = src->sin_addr.s_addr;
    UpLink[u_push_ptr].stat++;
    return 0;
}
/*************************************************************************************************************
 * __inline int get_u(struct Packet **ppt,struct sockaddr_in **pdr)
 * ������**ppt : Ҫ���͵����ݰ��Ķ���ָ��
 *       **pdr : Ŀ�ĵ�ַ�Ķ���ָ��
 * ���ܣ�����Ҫ���͵����ݰ���ָ�룬�ͷ��͵�ַ��ָ��
 *************************************************************************************************************/
__inline int get_u(struct Packet **ppt,struct sockaddr_in **pdr) {
    PickBuffer(&u_pop_ptr);
    if( 2 != UpLink[u_pop_ptr].stat ) return UpLink[u_pop_ptr].stat+1;
    *ppt = &(UpLink[u_pop_ptr].pkt);
    *pdr = &(UpLink[u_pop_ptr].dst);
    UpLink[u_pop_ptr].stat++;
    return 0;
}
/*************************************************************************************************************
 * __inline int erase_u()
 * ������
 * ���ܣ���u_pop_ptr��ָ������л������Ѿ����͹��Ŀռ�����Ϊ��д״̬
 *************************************************************************************************************/
__inline int erase_u() {
    if( 3 != UpLink[u_pop_ptr].stat && -1!=DownLink[d_pop_ptr].stat )
        return UpLink[u_pop_ptr].stat+1;
    UpLink[u_pop_ptr].stat = 0;
    return 0;
}
/*************************************************************************************************************
 * __inline int rebuilder_u(struct Server *psInfo, unsigned int offset)
 * ������*psInfo : ָ�� Server Info ��ָ��
 *       *pick   : Server���
 * ���ܣ���ʱ��������Ϣ�е�dst_id�ĳɴ˷���˵�id������Client��Ϣ�����ڻỰ��������
 *************************************************************************************************************/
__inline int rebuilder_u(struct Server *psInfo, unsigned int *pick) {
    static unsigned int pos = 0, *key;
    PickBuffer(&u_build_ptr);
    if( 1 != UpLink[u_build_ptr].stat ) return UpLink[u_build_ptr].stat+3;
    /* Server �����㷨 */
    if( OffSet>3 ) {
        Balance(pick);
        if( *pick >= MAX_Ser ) *pick = 0;
        pos = *pick;
    } else {
        key = (unsigned int *)&(UpLink[u_build_ptr].pkt.id) + OffSet;
        pos = Select_k(*key);
        if( pos>MAX_Ser ) {
            Balance(pick);
            if( *pick >= MAX_Ser ) *pick = 0;
            pos = *pick;
            Insert_k(*key, pos);
        }// else weigh[pos]--;
    }
    /* ��ʱ��������Ϣ�е�dst_id�ĳɴ˷���˵�id(��ѡ����ĿҪ��) */
    UpLink[u_build_ptr].pkt.id.dst_id = psInfo[pos].ser_id;
    UpLink[u_build_ptr].dst = psInfo[pos].ser;
    UpLink[u_build_ptr].stat++;
    /* ��Client��Ϣ�����ڻỰ�������� */
    return push_back_c(&(UpLink[u_build_ptr].pkt.id), &(UpLink[u_build_ptr].src), &(UpLink[u_build_ptr].dst), pos);
}
/*************************************************************************************************************
 * DWORD WINAPI ClientRecever(void *p)
 * ������*p : û���κ�����
 * ���ܣ�ֻ�����Client�������ݰ�(ʱ������)���̺߳����������򵥵���֤
 *************************************************************************************************************/
DWORD WINAPI ClientRecever(void *p) {
    int ret = 0;
    struct sockaddr_in ser_c = {AF_INET,htons(Cport),INADDR_ANY,'\0'};
    for(;;) {
        if(sizeof(pkt)!=recvfrom(sock,(char*)&pkt,sizeof(pkt),0,(struct sockaddr*)&ser_c,&addr_len)) continue;
        /* Ŀ��ID�����Լ�����ʱ���������ݰ� */
        if(pkt.id.dst_id != LB_ID || pkt.id.msg_tp != 0) {
            stat.wrong++;
            continue;
        }
        /* �ж�Server�Ƿ��� */
        if( AllSerDown ) continue;
        /* �ȴ����л������Ŀ��пռ� */
        ret = WaitForSingleObject(SC, 50);
        if(WAIT_TIMEOUT == ret) continue;
        else if(0 != ret) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore SC error!!!", "ClientRecever", GetLastError());
            continue;
        }
        stat.correct++;
        /* ��ӵ����л������� */
        if( push_u(&pkt, &ser_c) ) {
            PRINT(STD_ERROR, STD_TIME, "Push Packet failed!!!", "ClientRecever", 1);
            continue;
        }
        if( DETAIL == 2 ) displayPacket(&pkt,&ser_c,0);
        /* ������õ����ݰ�����C2S_rebuilder(CR++) */
        if( !ReleaseSemaphore(CR,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore CR error!!!", "ClientRecever", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI C2S_rebuilder(void *p)
 * ������*p : û���κ�����
 * ���ܣ�C2S_rebuilder�̺߳���������ClientRecever�̴߳���������ݰ��޸�dst_id��
 *       ѡ��Ŀ��Server��������Client�ĵ�ַ��Ϣ
 *************************************************************************************************************/
DWORD WINAPI C2S_rebuilder(void *p) {
    unsigned int pick = 0;
    int ret;
    for(;;) {
        /* �ȴ�ClientRecever����������ݰ�(CR--) */
        if( WaitForSingleObject(CR, INFINITE) ) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore CR error!!!", "C2S_rebuilder", GetLastError());
            continue;
        }
        /* ���´��������Ự��Ϣ */
        if( ret = rebuilder_u(sInfo, &pick) ) {
            if( ret > 1) ret -= 3;
            PRINT(STD_ERROR, STD_TIME, "Rebuild Packet failed!!!", "C2S_rebuilder", ret);
            continue;
        }
        /* ������õ����ݰ�����ServerSender(RS++) */
        if( !ReleaseSemaphore(RS,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore RS error!!!", "C2S_rebuilder", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI ServerSender(void *p)
 * ������*p : û���κ�����
 * ���ܣ�ֻ������Server�������ݰ����̺߳���
 *************************************************************************************************************/
DWORD WINAPI ServerSender(void *p) {
    int    ret;
    struct Packet      *ppt;
    struct sockaddr_in *pdr;
    for(;;) {
        /* �ȴ�C2S_rebuilder����������ݰ�(RS--) */
        if( WaitForSingleObject(RS, INFINITE) ) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore RS error!!!", "ServerSender", GetLastError());
            continue;
        }
        /* �����л�������ȡ��Ҫ���͵����ݰ�����Ϣ */
        if( !( ret = get_u(&ppt,&pdr) ) ) {
            /* �������ݰ� */
            if(sizeof(pkt)==sendto(sock_s,(char*)ppt,sizeof(pkt),0,(struct sockaddr*)pdr,addr_len)) {
                stat.sent++;
                if( DETAIL == 2 ) displayPacket(ppt,pdr,1);
            } else {
                PRINT(STD_ERROR, STD_TIME, "Send Packet failed!!!", "ServerSender", WSAGetLastError());
            }
        } else {
            PRINT(STD_ERROR, STD_TIME, "Get Packet failed!!!", "ServerSender", --ret);
        }
        /* �ͷ��Ѿ�������ϵĻ����� */
        if( ret = erase_u() )
            PRINT(STD_ERROR, STD_TIME, "Pop Packet failed!!!", "ServerSender", --ret);
        /* ���л��������пռ�+1 */
        if( !ReleaseSemaphore(SC,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore SC error!!!", "ServerSender", GetLastError());
    }
}