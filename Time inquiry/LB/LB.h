#ifndef __TIME_INQUIRY_LBH__
#define __TIME_INQUIRY_LBH__

#include "../global.h"
#include "errorlevel.h"

#ifdef  __cplusplus
extern "C" {
#endif

FILE     *lg;                                 /* LOG�ļ���� */
SOCKET   sock_s;                              /* ������Server��ͨ�� */
struct   Stat      stat_s;                    /* ��������ͳ����Ϣ */
struct   Server    *sInfo;                    /* Server��Ϣ������ָ�� */
unsigned short     Cport, Sport;              /* ����Client��Server��ͨ�Ŷ˿� */
unsigned int       MAX_Ser;                   /* Server���� */

#ifdef  __cplusplus
}
#endif

#endif