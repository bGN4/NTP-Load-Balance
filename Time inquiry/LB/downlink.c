#include "LB.h"
HANDLE   SR,RC,CS;
struct   Mail   *DownLink  = NULL;                                 /* �������ݻ�����ָ�� */
unsigned int    d_push_ptr = 0, d_build_ptr = 0, d_pop_ptr = 0;    /* �������ݿ����α� */
void     HealthMon(unsigned int w);
int      find_c(struct Packet *, struct sockaddr_in *, struct sockaddr_in *);
/*************************************************************************************************************
 * extern __inline void PickBuffer(unsigned int *previous)
 * ������*previous : �α�
 * ���ܣ�ȷ���α����һ��ָ��
 *************************************************************************************************************/
extern __inline void PickBuffer(unsigned int *previous) {
    *previous = (*previous < BUFFER_MAX-1) ? *previous+1 : 0;
}
/*************************************************************************************************************
 * __inline int push_d(struct Packet *pkt,struct sockaddr_in *src)
 * ������*pkt : ��Server���յ������ݰ���ָ��
 *       *src : Server��ַ��ָ��
 * ���ܣ�����Server���յ������ݰ���Server�ĵ�ַ��Ϣд�����л�����
 *************************************************************************************************************/
__inline int push_d(struct Packet *pkt,struct sockaddr_in *src) {
    PickBuffer(&d_push_ptr);
    if( 0 != DownLink[d_push_ptr].stat )
        return DownLink[d_push_ptr].stat;
    DownLink[d_push_ptr].pkt  = *pkt;
    DownLink[d_push_ptr].src.sin_port = src->sin_port;
    DownLink[d_push_ptr].src.sin_addr.s_addr = src->sin_addr.s_addr;
    DownLink[d_push_ptr].stat++;
    return 0;
}
/*************************************************************************************************************
 * __inline int get_d(struct Packet **ppt,struct sockaddr_in **pdr)
 * ������**ppt : Ҫ���͵����ݰ��Ķ���ָ��
 *       **pdr : Ŀ�ĵ�ַ�Ķ���ָ��
 * ���ܣ�����Ҫ���͵����ݰ���ָ�룬�ͷ��͵�ַ��ָ��
 *************************************************************************************************************/
__inline int get_d(struct Packet **ppt,struct sockaddr_in **pdr) {
    PickBuffer(&d_pop_ptr);
    if(  2 != DownLink[d_pop_ptr].stat ) return DownLink[d_pop_ptr].stat+2;
    *ppt = &(DownLink[d_pop_ptr].pkt);
    *pdr = &(DownLink[d_pop_ptr].dst);
    DownLink[d_pop_ptr].stat++;
    return 0;
}
/*************************************************************************************************************
 * __inline int erase_d()
 * ������
 * ���ܣ���d_pop_ptr��ָ������л������Ѿ����͹��Ŀռ�����Ϊ��д״̬
 *************************************************************************************************************/
__inline int erase_d() {
    if( 3!=DownLink[d_pop_ptr].stat && -1!=DownLink[d_pop_ptr].stat )
        return DownLink[d_pop_ptr].stat+1;
    DownLink[d_pop_ptr].stat = 0;
    return 0;
}
/*************************************************************************************************************
 * __inline int rebuilder_d(int src_id)
 * ������src_id : LB Server ��ID
 * ���ܣ��ӻỰ�������ж�λ��֮ƥ���Client��ַ���������ݰ��е�src_id�ĳ��Լ���id
 *************************************************************************************************************/
__inline int rebuilder_d(int src_id) {
    PickBuffer(&d_build_ptr);
    if( 1 != DownLink[d_build_ptr].stat ) return DownLink[d_build_ptr].stat+1;
    /* �ӻỰ�������ж�λ��֮ƥ���Client��ַ */
    if( find_c( &(DownLink[d_build_ptr].pkt), &(DownLink[d_build_ptr].src), &(DownLink[d_build_ptr].dst)) ) {
        DownLink[d_build_ptr].stat = -1;
        return 0;
    }
    /* ����Ϣ�е�src_id�ĳ��Լ���id(��ѡ����ĿҪ��) */
    DownLink[d_build_ptr].pkt.id.src_id = src_id;
    DownLink[d_build_ptr].stat++;
    return 0;
}
/*************************************************************************************************************
 * DWORD WINAPI ServerRecever(void *p)
 * ������*p : û���κ�����
 * ���ܣ�ֻ�����Server�������ݰ�(ʱ��Ӧ��������)���̺߳����������򵥵���֤
 *************************************************************************************************************/
DWORD WINAPI ServerRecever(void *p) {
    int ret = 0;
    struct sockaddr_in ser_s = {AF_INET,htons(Sport),INADDR_ANY,'\0'};
    for(;;) {
        if(sizeof(rpk)!=recvfrom(sock_s,(char*)&rpk,sizeof(rpk),0,(struct sockaddr*)&ser_s,&addr_len)) continue;
        /* ����Ӧ�� */
        if( rpk.id.msg_tp == 3 ) {
            if( rpk.id.dst_id == LB_ID ) {
                HealthMon(rpk.id.src_id);
            }
            continue;
        }
        /* ʱ��Ӧ�� */
        if( rpk.id.msg_tp != 1 ) {
            stat_s.wrong++;
            continue;
        }
        /* �ȴ����л������Ŀ��пռ� */
        ret = WaitForSingleObject(CS, 50);
        if(WAIT_TIMEOUT == ret) continue;
        else if(0 != ret) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore CS error!!!", "ServerRecever", GetLastError());
            continue;
        }
        /* ��ӵ����л������� */
        if( push_d(&rpk, &ser_s) ) {
            PRINT(STD_ERROR, STD_TIME, "Push Packet failed!!!", "ServerRecever", 1);
            continue;
        }
        if( DETAIL == 2 ) displayPacket(&rpk,&ser_s,0);
        /* ������õ����ݰ�����S2C_rebuilder(SR++) */
        if( !ReleaseSemaphore(SR,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore SR error!!!", "ServerRecever", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI S2C_rebuilder(void *p)
 * ������*p : û���κ�����
 * ���ܣ�S2C_rebuilder�̺߳���������ServerRecever�̴߳���������ݰ��޸�src_id������λ��֮ƥ���Client��ַ
 *************************************************************************************************************/
DWORD WINAPI S2C_rebuilder(void *p) {
    int ret;
    for(;;) {
        /* �ȴ�ServerRecever����������ݰ�(SR--) */
        if( WaitForSingleObject(SR, INFINITE) )
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore SR error!!!", "S2C_rebuilder", GetLastError());
        /* ���´�����ӻỰ�������ж�λ��֮ƥ���Client */
        if( ret = rebuilder_d(LB_ID) ) {
            if( ret<0 )
                PRINT(STD_ERROR, STD_TIME, "Rebuild Packet failed!!!", "S2C_rebuilder", --ret);
            stat_s.wrong++;
            continue;
        }
        stat_s.correct++;
        /* ������õ����ݰ�����ClientSender(RC++) */
        if( !ReleaseSemaphore(RC,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore RC error!!!", "S2C_rebuilder", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI ClientSender(void *p)
 * ������*p : û���κ�����
 * ���ܣ�ֻ������Client�������ݰ����̺߳���
 *************************************************************************************************************/
DWORD WINAPI ClientSender(void *p) {
    int    ret;
    struct Packet      *ppt;
    struct sockaddr_in *pdr;
    for(;;) {
        /* �ȴ�S2C_rebuilder����������ݰ�(RC--) */
        if( WaitForSingleObject(RC, INFINITE) )
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore RC error!!!", "ClientSender", GetLastError());
        /* �����л�������ȡ��Ҫ���͵����ݰ�����Ϣ */
        if( !( ret = get_d(&ppt,&pdr) ) ) {
            /* �������ݰ� */
            if(sizeof(pkt)==sendto(sock,(char*)ppt,sizeof(pkt),0,(struct sockaddr*)pdr,addr_len)) {
                stat_s.sent++;
                if( DETAIL == 2 ) displayPacket(ppt,pdr,1);
            } else {
                PRINT(STD_ERROR, STD_TIME, "Send Packet failed!!!", "ClientSender", WSAGetLastError());
            }
        } else if( 1 != ret ) {
            PRINT(STD_ERROR, STD_TIME, "Get Packet failed!!!", "ClientSender", ret-2);
            PRINT("d_push_ptr = %u, d_build_ptr = %u, d_pop_ptr = %u\n",
                   d_push_ptr,      d_build_ptr,      d_pop_ptr);
        }
        /* �ͷ��Ѿ�������ϵĻ����� */
        if( ret = erase_d() )
            PRINT(STD_ERROR, STD_TIME, "Pop Packet failed!!!", "ClientSender", --ret);
        /* ���л��������пռ�+1 */
        if( !ReleaseSemaphore(CS,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore CS error!!!", "ClientSender", GetLastError());
    }
}
