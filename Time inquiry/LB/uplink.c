#include "LB.h"
HANDLE   CR,RS,SC;
struct   Mail   *UpLink    = NULL;                                 /* 上行数据缓冲区指针 */
unsigned int    u_push_ptr = 0; u_build_ptr = 0, u_pop_ptr = 0;    /* 上行数据控制游标 */
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
 * 参数：*pkt : 从Client接收到的数据包的指针
 *       *src : Client地址的指针
 * 功能：将从Client接收到的数据包、Client的地址信息写入上行缓冲区
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
 * 参数：**ppt : 要发送的数据包的二重指针
 *       **pdr : 目的地址的二重指针
 * 功能：返回要发送的数据包的指针，和发送地址的指针
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
 * 参数：
 * 功能：将u_pop_ptr所指向的上行缓冲区已经发送过的空间设置为可写状态
 *************************************************************************************************************/
__inline int erase_u() {
    if( 3 != UpLink[u_pop_ptr].stat && -1!=DownLink[d_pop_ptr].stat )
        return UpLink[u_pop_ptr].stat+1;
    UpLink[u_pop_ptr].stat = 0;
    return 0;
}
/*************************************************************************************************************
 * __inline int rebuilder_u(struct Server *psInfo, unsigned int offset)
 * 参数：*psInfo : 指向 Server Info 的指针
 *       *pick   : Server编号
 * 功能：将时间请求消息中的dst_id改成此服务端的id，并将Client信息保存在会话缓冲区中
 *************************************************************************************************************/
__inline int rebuilder_u(struct Server *psInfo, unsigned int *pick) {
    static unsigned int pos = 0, *key;
    PickBuffer(&u_build_ptr);
    if( 1 != UpLink[u_build_ptr].stat ) return UpLink[u_build_ptr].stat+3;
    /* Server 分配算法 */
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
    /* 将时间请求消息中的dst_id改成此服务端的id(节选自题目要求) */
    UpLink[u_build_ptr].pkt.id.dst_id = psInfo[pos].ser_id;
    UpLink[u_build_ptr].dst = psInfo[pos].ser;
    UpLink[u_build_ptr].stat++;
    /* 将Client信息保存在会话缓冲区中 */
    return push_back_c(&(UpLink[u_build_ptr].pkt.id), &(UpLink[u_build_ptr].src), &(UpLink[u_build_ptr].dst), pos);
}
/*************************************************************************************************************
 * DWORD WINAPI ClientRecever(void *p)
 * 参数：*p : 没有任何作用
 * 功能：只负责从Client接收数据包(时间请求)的线程函数，并做简单的验证
 *************************************************************************************************************/
DWORD WINAPI ClientRecever(void *p) {
    int ret = 0;
    struct sockaddr_in ser_c = {AF_INET,htons(Cport),INADDR_ANY,'\0'};
    for(;;) {
        if(sizeof(pkt)!=recvfrom(sock,(char*)&pkt,sizeof(pkt),0,(struct sockaddr*)&ser_c,&addr_len)) continue;
        /* 目标ID不是自己或不是时间请求数据包 */
        if(pkt.id.dst_id != LB_ID || pkt.id.msg_tp != 0) {
            stat.wrong++;
            continue;
        }
        /* 判断Server是否存活 */
        if( AllSerDown ) continue;
        /* 等待上行缓冲区的空闲空间 */
        ret = WaitForSingleObject(SC, 50);
        if(WAIT_TIMEOUT == ret) continue;
        else if(0 != ret) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore SC error!!!", "ClientRecever", GetLastError());
            continue;
        }
        stat.correct++;
        /* 添加到上行缓冲区中 */
        if( push_u(&pkt, &ser_c) ) {
            PRINT(STD_ERROR, STD_TIME, "Push Packet failed!!!", "ClientRecever", 1);
            continue;
        }
        if( DETAIL == 2 ) displayPacket(&pkt,&ser_c,0);
        /* 将处理好的数据包交给C2S_rebuilder(CR++) */
        if( !ReleaseSemaphore(CR,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore CR error!!!", "ClientRecever", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI C2S_rebuilder(void *p)
 * 参数：*p : 没有任何作用
 * 功能：C2S_rebuilder线程函数，负责将ClientRecever线程处理过的数据包修改dst_id，
 *       选择目标Server，并保存Client的地址信息
 *************************************************************************************************************/
DWORD WINAPI C2S_rebuilder(void *p) {
    unsigned int pick = 0;
    int ret;
    for(;;) {
        /* 等待ClientRecever处理过的数据包(CR--) */
        if( WaitForSingleObject(CR, INFINITE) ) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore CR error!!!", "C2S_rebuilder", GetLastError());
            continue;
        }
        /* 重新打包并保存会话信息 */
        if( ret = rebuilder_u(sInfo, &pick) ) {
            if( ret > 1) ret -= 3;
            PRINT(STD_ERROR, STD_TIME, "Rebuild Packet failed!!!", "C2S_rebuilder", ret);
            continue;
        }
        /* 将处理好的数据包交给ServerSender(RS++) */
        if( !ReleaseSemaphore(RS,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore RS error!!!", "C2S_rebuilder", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI ServerSender(void *p)
 * 参数：*p : 没有任何作用
 * 功能：只负责向Server发送数据包的线程函数
 *************************************************************************************************************/
DWORD WINAPI ServerSender(void *p) {
    int    ret;
    struct Packet      *ppt;
    struct sockaddr_in *pdr;
    for(;;) {
        /* 等待C2S_rebuilder处理过的数据包(RS--) */
        if( WaitForSingleObject(RS, INFINITE) ) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore RS error!!!", "ServerSender", GetLastError());
            continue;
        }
        /* 从上行缓冲区中取得要发送的数据包的信息 */
        if( !( ret = get_u(&ppt,&pdr) ) ) {
            /* 发送数据包 */
            if(sizeof(pkt)==sendto(sock_s,(char*)ppt,sizeof(pkt),0,(struct sockaddr*)pdr,addr_len)) {
                stat.sent++;
                if( DETAIL == 2 ) displayPacket(ppt,pdr,1);
            } else {
                PRINT(STD_ERROR, STD_TIME, "Send Packet failed!!!", "ServerSender", WSAGetLastError());
            }
        } else {
            PRINT(STD_ERROR, STD_TIME, "Get Packet failed!!!", "ServerSender", --ret);
        }
        /* 释放已经发送完毕的缓冲区 */
        if( ret = erase_u() )
            PRINT(STD_ERROR, STD_TIME, "Pop Packet failed!!!", "ServerSender", --ret);
        /* 上行缓冲区空闲空间+1 */
        if( !ReleaseSemaphore(SC,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore SC error!!!", "ServerSender", GetLastError());
    }
}