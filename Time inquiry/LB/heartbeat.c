#include "LB.h"
#include <time.h>
volatile int AllSerDown = 1;    /* 没有存活的服务器 *
/*************************************************************************************************************
 * extern __inline int PickPosition(unsigned int MAX, unsigned int ser_id)
 * 参数：MAX    : Server的总数量
 *       ser_id : Server ID
 * 功能：给出Server ID，找出对应的在sInfo中的位置
 *************************************************************************************************************/
extern __inline int PickPosition(unsigned int MAX, unsigned int ser_id) {
    unsigned int i = 0;
    for(i=0 ; i<MAX ; i++) {
        if(sInfo[i].ser_id == ser_id) return i;
    }
    PRINT(STD_ERROR, STD_TIME, "Pick position error!!!", "PickPosition", -1);
    return -1;
}
/*************************************************************************************************************
 * DWORD WINAPI Heartbeat(void *p)
 * 参数：*p   : Server Info pointer
 * 功能：周期发送心跳请求数据包
 *************************************************************************************************************/
DWORD WINAPI Heartbeat(void *p) {
    unsigned int i = 0, down = 0;
    struct Packet hpk;                 /* 心跳包 */
    memset(&hpk, 0, sizeof(hpk));
    hpk.id.msg_tp = 2;
    hpk.id.src_id = LB_ID;
    for(;;) {
        Sleep(HEARTBEAT_PERIOD);
        for(i=0 ; i<MAX_Ser ; i++) {
            hpk.id.dst_id = sInfo[i].ser_id;
            if( sInfo[i].time ) {                /* 上一个心跳包超时 */
                if( i == sInfo[MAX_Ser].time )   /* 超时的是响应最快的服务器 */
                    sInfo[MAX_Ser].time = (!i) ? MAX_Ser-1 : i-1;
                if( !sInfo[i].delay ) {          /* 连续4次超时 */
                    if( !AllSerDown ) {
                        if( down == i ) down++;  /* 掉线服务器累计 */
                        if( down == MAX_Ser ) {  /* 全掉线了 */
                            down = 0;
                            AllSerDown = 1;
                            PRINT(STD_WARN, STD_TIME, "All server is offline!!!", "Alive", 0);
                        }
                    }
                }
                else if( sInfo[i].delay < -2 ) { /* 第4次超时 */
                    sInfo[i].delay = 0;
                    PRINT("%s %s <WARN> Server %u lost!!!(%s:%hu)\n", STD_TIME, sInfo[i].ser_id,
                           inet_ntoa(sInfo[i].ser.sin_addr), htons(sInfo[i].ser.sin_port));
                }
                else if( sInfo[i].delay > 0 ) {  /* 第1次超时 */
                       sInfo[i].delay = -1;
                } else sInfo[i].delay--;         /* 第2,3次超时 */
            } else down = 0;
            sInfo[i].time = clock()*1000/CLOCKS_PER_SEC; //clocks_per_ms
            sendto(sock_s,(char*)&hpk,sizeof(hpk),0,(struct sockaddr*)&(sInfo[i].ser),addr_len);
        }
    }
}
/*************************************************************************************************************
 * void HealthMon(unsigned int w)
 * 参数：w : Server ID
 * 功能：
 *************************************************************************************************************/
void HealthMon(unsigned int w) {
    static unsigned int p;
    static unsigned long t;
    t = clock()*1000/CLOCKS_PER_SEC;
    if( AllSerDown ) AllSerDown = 0;
    p = PickPosition(MAX_Ser,w);
    if( p<0 ) return;
    t -= sInfo[p].time;
    sInfo[p].time  = 0;
    if( !sInfo[p].delay )
        PRINT("%s %s <INFO> Server %u joined!!!(%s:%hu)\n", STD_TIME, sInfo[p].ser_id,
               inet_ntoa(sInfo[p].ser.sin_addr), htons(sInfo[p].ser.sin_port));
    t = sInfo[p].delay = (t<=0) ? 1 : t;
    if( t<(unsigned long)sInfo[MAX_Ser].delay ) {
        sInfo[MAX_Ser].delay = t;   // min_delay
        sInfo[MAX_Ser].time = p;    // min_delay position
    }
    else if( p == sInfo[MAX_Ser].time ) {
        sInfo[MAX_Ser].delay = t;   // min_delay
    }
}
