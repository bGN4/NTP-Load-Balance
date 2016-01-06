#include "LB.h"
HANDLE   SR,RC,CS;
struct   Mail   *DownLink  = NULL;                                 /* 下行数据缓冲区指针 */
unsigned int    d_push_ptr = 0, d_build_ptr = 0, d_pop_ptr = 0;    /* 下行数据控制游标 */
void     HealthMon(unsigned int w);
int      find_c(struct Packet *, struct sockaddr_in *, struct sockaddr_in *);
/*************************************************************************************************************
 * extern __inline void PickBuffer(unsigned int *previous)
 * 参数：*previous : 游标
 * 功能：确定游标的下一个指向
 *************************************************************************************************************/
extern __inline void PickBuffer(unsigned int *previous) {
    *previous = (*previous < BUFFER_MAX-1) ? *previous+1 : 0;
}
/*************************************************************************************************************
 * __inline int push_d(struct Packet *pkt,struct sockaddr_in *src)
 * 参数：*pkt : 从Server接收到的数据包的指针
 *       *src : Server地址的指针
 * 功能：将从Server接收到的数据包、Server的地址信息写入下行缓冲区
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
 * 参数：**ppt : 要发送的数据包的二重指针
 *       **pdr : 目的地址的二重指针
 * 功能：返回要发送的数据包的指针，和发送地址的指针
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
 * 参数：
 * 功能：将d_pop_ptr所指向的下行缓冲区已经发送过的空间设置为可写状态
 *************************************************************************************************************/
__inline int erase_d() {
    if( 3!=DownLink[d_pop_ptr].stat && -1!=DownLink[d_pop_ptr].stat )
        return DownLink[d_pop_ptr].stat+1;
    DownLink[d_pop_ptr].stat = 0;
    return 0;
}
/*************************************************************************************************************
 * __inline int rebuilder_d(int src_id)
 * 参数：src_id : LB Server 的ID
 * 功能：从会话缓冲区中定位与之匹配的Client地址，并将数据包中的src_id改成自己的id
 *************************************************************************************************************/
__inline int rebuilder_d(int src_id) {
    PickBuffer(&d_build_ptr);
    if( 1 != DownLink[d_build_ptr].stat ) return DownLink[d_build_ptr].stat+1;
    /* 从会话缓冲区中定位与之匹配的Client地址 */
    if( find_c( &(DownLink[d_build_ptr].pkt), &(DownLink[d_build_ptr].src), &(DownLink[d_build_ptr].dst)) ) {
        DownLink[d_build_ptr].stat = -1;
        return 0;
    }
    /* 将消息中的src_id改成自己的id(节选自题目要求) */
    DownLink[d_build_ptr].pkt.id.src_id = src_id;
    DownLink[d_build_ptr].stat++;
    return 0;
}
/*************************************************************************************************************
 * DWORD WINAPI ServerRecever(void *p)
 * 参数：*p : 没有任何作用
 * 功能：只负责从Server接收数据包(时间应答、心跳包)的线程函数，并做简单的验证
 *************************************************************************************************************/
DWORD WINAPI ServerRecever(void *p) {
    int ret = 0;
    struct sockaddr_in ser_s = {AF_INET,htons(Sport),INADDR_ANY,'\0'};
    for(;;) {
        if(sizeof(rpk)!=recvfrom(sock_s,(char*)&rpk,sizeof(rpk),0,(struct sockaddr*)&ser_s,&addr_len)) continue;
        /* 心跳应答 */
        if( rpk.id.msg_tp == 3 ) {
            if( rpk.id.dst_id == LB_ID ) {
                HealthMon(rpk.id.src_id);
            }
            continue;
        }
        /* 时间应答 */
        if( rpk.id.msg_tp != 1 ) {
            stat_s.wrong++;
            continue;
        }
        /* 等待下行缓冲区的空闲空间 */
        ret = WaitForSingleObject(CS, 50);
        if(WAIT_TIMEOUT == ret) continue;
        else if(0 != ret) {
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore CS error!!!", "ServerRecever", GetLastError());
            continue;
        }
        /* 添加到下行缓冲区中 */
        if( push_d(&rpk, &ser_s) ) {
            PRINT(STD_ERROR, STD_TIME, "Push Packet failed!!!", "ServerRecever", 1);
            continue;
        }
        if( DETAIL == 2 ) displayPacket(&rpk,&ser_s,0);
        /* 将处理好的数据包交给S2C_rebuilder(SR++) */
        if( !ReleaseSemaphore(SR,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore SR error!!!", "ServerRecever", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI S2C_rebuilder(void *p)
 * 参数：*p : 没有任何作用
 * 功能：S2C_rebuilder线程函数，负责将ServerRecever线程处理过的数据包修改src_id，并定位与之匹配的Client地址
 *************************************************************************************************************/
DWORD WINAPI S2C_rebuilder(void *p) {
    int ret;
    for(;;) {
        /* 等待ServerRecever处理过的数据包(SR--) */
        if( WaitForSingleObject(SR, INFINITE) )
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore SR error!!!", "S2C_rebuilder", GetLastError());
        /* 重新打包并从会话缓冲区中定位与之匹配的Client */
        if( ret = rebuilder_d(LB_ID) ) {
            if( ret<0 )
                PRINT(STD_ERROR, STD_TIME, "Rebuild Packet failed!!!", "S2C_rebuilder", --ret);
            stat_s.wrong++;
            continue;
        }
        stat_s.correct++;
        /* 将处理好的数据包交给ClientSender(RC++) */
        if( !ReleaseSemaphore(RC,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore RC error!!!", "S2C_rebuilder", GetLastError());
    }
}
/*************************************************************************************************************
 * DWORD WINAPI ClientSender(void *p)
 * 参数：*p : 没有任何作用
 * 功能：只负责向Client发送数据包的线程函数
 *************************************************************************************************************/
DWORD WINAPI ClientSender(void *p) {
    int    ret;
    struct Packet      *ppt;
    struct sockaddr_in *pdr;
    for(;;) {
        /* 等待S2C_rebuilder处理过的数据包(RC--) */
        if( WaitForSingleObject(RC, INFINITE) )
            PRINT(STD_ERROR, STD_TIME, "WaitSemaphore RC error!!!", "ClientSender", GetLastError());
        /* 从下行缓冲区中取得要发送的数据包的信息 */
        if( !( ret = get_d(&ppt,&pdr) ) ) {
            /* 发送数据包 */
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
        /* 释放已经发送完毕的缓冲区 */
        if( ret = erase_d() )
            PRINT(STD_ERROR, STD_TIME, "Pop Packet failed!!!", "ClientSender", --ret);
        /* 下行缓冲区空闲空间+1 */
        if( !ReleaseSemaphore(CS,1,NULL) )
            PRINT(STD_ERROR, STD_TIME, "ReleaseSemaphore CS error!!!", "ClientSender", GetLastError());
    }
}
