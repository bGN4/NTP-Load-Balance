#pragma  comment(linker,"/OPT:NOWIN98")
#include "../struct.h"
struct   Server *psInfo = NULL;
unsigned int     ptr    = 0,   MAX = 0;
/*************************************************************************************************************
 * __declspec(dllexport) void __stdcall Balance(unsigned int *pre)
 * ������*pre : Server Info ѡ��ָ��
 * ���ܣ����������Ӧ�ĸ��ؾ����㷨
 *************************************************************************************************************/
__declspec(dllexport) void __stdcall Balance(unsigned int *pre) {
    *pre = psInfo[MAX].time;
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