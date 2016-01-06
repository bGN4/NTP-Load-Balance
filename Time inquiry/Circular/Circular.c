#pragma  comment(linker,"/OPT:NOWIN98")
#include "../struct.h"
struct   Server *psInfo = NULL;
unsigned int     ptr    = 0,   MAX = 0;
/*************************************************************************************************************
 * __declspec(dllexport) void __stdcall Balance(unsigned int *pre)
 * ������*pre : Server Info ѡ��ָ��
 * ���ܣ���ת�㷨
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall Balance(unsigned int *pre) {
    ptr = *pre;
    do {
        *pre = (*pre < MAX-1) ? *pre+1 : 0;
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
 *       *m  : û��
 * ���ܣ���ʼ����Ϣ
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall setsInfo(unsigned int max, struct Server *p, int *m) {
    MAX    = max;
    psInfo = p;
}