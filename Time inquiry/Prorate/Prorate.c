#pragma  comment(linker,"/OPT:NOWIN98")
#include "../struct.h"
struct   Server *psInfo = NULL;
volatile int    *weight = NULL;
unsigned int    MAX = 0, ptr = 0;
/*************************************************************************************************************
 * __declspec(dllexport) void __stdcall Balance(unsigned int *pre)
 * ������*pre : Server Info ѡ��ָ��
 * ���ܣ��������ַ��ĸ��ؾ����㷨
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
 * ������max : Server ����
 *       *p  : Server Info ��������ַ
 *       *m  : Ȩֵ������
 * ���ܣ���ʼ����Ϣ
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall setsInfo(unsigned int max, struct Server *p, int *m) {
    MAX    = max;
    psInfo = p;
    weight = m;
    for(ptr=0 ; ptr<MAX ; ptr++)
        weight[ptr] = psInfo[ptr].busy;
}