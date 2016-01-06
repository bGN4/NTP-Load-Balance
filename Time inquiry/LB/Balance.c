#include "../struct.h"
unsigned int ptr = 0;
extern   int *weigh;
extern unsigned int    MAX_Ser;
extern struct   Server *sInfo;
/*************************************************************************************************************
 * void __stdcall Circular(unsigned int *pre)
 * 参数：*pre : Server Info 选择指针
 * 功能：轮转算法
 *************************************************************************************************************/
void __stdcall Circular(unsigned int *pre) {
    ptr = *pre;
    do {
        *pre = (*pre < MAX_Ser-1) ? *pre+1 : 0;
        if(ptr == *pre) {
            if( !sInfo[*pre].delay ) *pre = -1;
            break;
        }
    } while( !sInfo[*pre].delay );
}
/*************************************************************************************************************
 * void __stdcall Fast(unsigned int *pre)
 * 参数：*pre : Server Info 选择指针
 * 功能：基于最快响应的负载均衡算法
 *************************************************************************************************************/
void __stdcall Fast(unsigned int *pre) {
    *pre = sInfo[MAX_Ser].time;
}
/*************************************************************************************************************
 * void __stdcall Prorate(unsigned int *pre)
 * 参数：*pre : Server Info 选择指针
 * 功能：按比例分发的负载均衡算法
 *************************************************************************************************************/
void __stdcall Prorate(unsigned int *pre) {
    ptr = *pre;
    do {
        do {
            *pre = (*pre < MAX_Ser-1) ? *pre+1 : 0;
            if(ptr == *pre) {
                if( weigh[*pre]>0 ) break;
                else {
                    for(ptr=0 ; ptr<MAX_Ser ; ptr++)
                        weigh[ptr] += sInfo[ptr].busy;
                    ptr = *pre;
                }
            }
        } while( weigh[*pre]<=0 );
        weigh[*pre]--;
        if(ptr == *pre) {
            if( !sInfo[*pre].delay ) *pre = -1;
            break;
        }
    } while( !sInfo[*pre].delay );
}