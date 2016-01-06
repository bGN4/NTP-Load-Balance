#ifndef __TIME_INQUIRY_LBH__
#define __TIME_INQUIRY_LBH__

#include "../global.h"
#include "errorlevel.h"

#ifdef  __cplusplus
extern "C" {
#endif

FILE     *lg;                                 /* LOG文件句柄 */
SOCKET   sock_s;                              /* 用来与Server端通信 */
struct   Stat      stat_s;                    /* 下行数据统计信息 */
struct   Server    *sInfo;                    /* Server信息缓冲区指针 */
unsigned short     Cport, Sport;              /* 绑定与Client和Server的通信端口 */
unsigned int       MAX_Ser;                   /* Server数量 */

#ifdef  __cplusplus
}
#endif

#endif