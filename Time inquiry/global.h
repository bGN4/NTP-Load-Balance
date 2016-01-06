#ifndef __TIME_INQUIRY_GLOBAL__
#define __TIME_INQUIRY_GLOBAL__

#include "defines.h"
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include "struct.h"
#pragma  comment(lib,"ws2_32.lib")
#define  JAN_1970   0x83aa7e80                                // 1900-1970 in seconds(2208988800)
#define  NTPFRAC(x) (4294*(x)+((1981*(x))>>11))               // x*4294.967296
#define  USEC(x)    (((x)>>12)-759*((((x)>>10)+32768)>>16))   // x/4294.967296
#define  SEC2U(x)   ((x)*0.0000152587890625)
#define  QUIT(r)    DeleteCriticalSection(&CriticalSection);SetConsoleTextAttribute(hOut, bInfo.wAttributes);return r

volatile unsigned    int Direction, DETAIL;                             /* 方向指示、显示开关 */
struct   Packet      pkt,rpk,dpk;                                       /* 分别用于发送、接收、显示 */
struct   Stat        stat;                                              /* 统计信息 */
struct   sockaddr_in ser;                                               /*  */
int      addr_len;                                                      /*  */
unsigned int         LB_ID;                                             /* LB Server ID */
char     Title[64], dt[16], tm[16];                                     /* 标题、日期、时间缓冲区 */
SOCKET   sock;                                                          /* SOCKET */
HANDLE   hIn,hOut,DISP;                                                 /* 输入输出句柄、显示信号量 */
CRITICAL_SECTION            CriticalSection;                            /* 显示临界区 */
CONSOLE_SCREEN_BUFFER_INFO  bInfo;                                      /* SCREEN BUFFER INFO */

void displayPacket(struct Packet *pkt, struct sockaddr_in* ser, int rs);
int BindPort(SOCKET *s, struct sockaddr_in *saddr);
int NetworkInit(SOCKET *s);
int StartInit();
int cls(HANDLE hOut, CONSOLE_SCREEN_BUFFER_INFO *bInfo, unsigned int r);
DWORD WINAPI MoreInfo(void *display_stat);
BOOL WINAPI ConsoleHandler(DWORD Event);
DWORD WINAPI DispPacket(void *p);

#endif