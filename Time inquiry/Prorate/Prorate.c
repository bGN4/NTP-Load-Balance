#pragma  comment(linker,"/OPT:NOWIN98")
#include "../struct.h"
struct   Server *psInfo = NULL;
volatile int    *weight = NULL;
unsigned int    MAX = 0, ptr = 0;
/*************************************************************************************************************
 * __declspec(dllexport) void __stdcall Balance(unsigned int *pre)
 * 参数：*pre : Server Info 选择指针
 * 功能：按比例分发的负载均衡算法
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall Balance(unsigned int *pre) {
    ptr = *pre;
    do {
        do {
            *pre = (*pre < MAX-1) ? *pre+1 : 0;
            if(ptr == *pre) {
                if( weight[*pre]>0 ) break;
                else {
                    for(ptr=0 ; ptr<MAX ; ptr++)
                        weight[ptr] += psInfo[ptr].busy;
                    ptr = *pre;
                }
            }
        } while( weight[*pre]<=0 );
        weight[*pre]--;
        if(ptr == *pre) {
            if( !psInfo[*pre].delay ) *pre = -1;
            break;
        }
    } while( !psInfo[*pre].delay );
}
/*************************************************************************************************************
 * __declspec(dllexport) void __stdcall setsInfo(unsigned int max, struct Server *p, int *m)
 * 参数：max : Server 数量
 *       *p  : Server Info 缓冲区地址
 *       *m  : 权值缓冲区
 * 功能：初始化信息
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall setsInfo(unsigned int max, struct Server *p, int *m) {
    MAX    = max;
    psInfo = p;
    weight = m;
    for(ptr=0 ; ptr<MAX ; ptr++)
        weight[ptr] = psInfo[ptr].busy;
}