#pragma  comment(linker,"/OPT:NOWIN98")
#include "../struct.h"
struct   Server *psInfo = NULL;
unsigned int     ptr    = 0,   MAX = 0;
/*************************************************************************************************************
 * __declspec(dllexport) void __stdcall Balance(unsigned int *pre)
 * 参数：*pre : Server Info 选择指针
 * 功能：基于最快响应的负载均衡算法
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall Balance(unsigned int *pre) {
    *pre = psInfo[MAX].time;
}
/*************************************************************************************************************
 * __declspec(dllexport) void __stdcall setsInfo(unsigned int max, struct Server *p, int *m)
 * 参数：max : Server 数量
 *       *p  : Server Info 缓冲区地址
 *       *m  : 没用
 * 功能：初始化信息
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall setsInfo(unsigned int max, struct Server *p, int *m) {
    MAX    = max;
    psInfo = p;
}