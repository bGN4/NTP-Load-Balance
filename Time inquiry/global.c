#include "global.h"

volatile unsigned    int  DETAIL = 0;                                   /* ����ָʾ����ʾ���� */
struct   Stat        stat = {0, 0, 0};                                  /* ͳ����Ϣ */
struct   sockaddr_in ser  = {AF_INET,0,INADDR_ANY,'\0'};                /*  */
int      addr_len         = sizeof(struct sockaddr_in);                 /*  */

/*************************************************************************************************************
 * void displayPacket(struct Packet *pkt, struct sockaddr_in *ser, int rs)
 * ������*pkt : Packet
 *       *ser : ��ַ��Ϣ
 *        rs  : Sendto(1)��Receivefrom(0)
 * ���ܣ���ʾPacket����ϸ��Ϣ
 *************************************************************************************************************/
void displayPacket(struct Packet *pkt, struct sockaddr_in* ser, int rs) {
    time_t t = htonl(pkt->ntpd.Reference_Timestamp.coarse) - JAN_1970;
    struct tm *tm = gmtime(&t);
    EnterCriticalSection(&CriticalSection);
    if(rs) printf("SEND TO:\t\t%s:%u\n",inet_ntoa(ser->sin_addr),htons(ser->sin_port));
    else printf("RECEIVE FROM:\t\t%s:%u\n",inet_ntoa(ser->sin_addr),htons(ser->sin_port));
    printf("id_src:\t\t\t%u\n"             "id_dst:\t\t\t%u\n"
           "id_usr:\t\t\t%u\n"             "msg_type:\t\t%u\n"
           "Leap Indicator:\t\t%u\n"       "Version number:\t\t%u\n"
           "Mode:\t\t\t%u\n"               "Peer Clock Stratum:\t%u\n"
           "Peer Polling Interval:\t%d\n"  "Peer Clock Precision:\t%f\n"
           "Root Delay:\t\t%f sec\n"       "Root Dispersion:\t%f sec\n"
           "Reference Identifier:\t%u.%u.%u.%u\n"
           "Reference Timestamp:\t%04u-%02u-%02u %02u:%02u:%02u.%.6u\n",
           pkt->id.src_id,                 pkt->id.dst_id,
           pkt->id.usr_id,                 pkt->id.msg_tp,
           (pkt->ntpd.Flags&0xc0)>>6,      (pkt->ntpd.Flags&0x38)>>3,
           pkt->ntpd.Flags&0x7,            pkt->ntpd.Peer_Clock_Stratum,
           1<<pkt->ntpd.Peer_Polling_Interval,
           1.0/(1<<~pkt->ntpd.Peer_Clock_Precision),
           SEC2U(htonl(pkt->ntpd.Root_Delay)),
           SEC2U(htonl(pkt->ntpd.Root_Dispersion)),
           ((unsigned char*)&pkt->ntpd.Reference_Identifier)[0],
           ((unsigned char*)&pkt->ntpd.Reference_Identifier)[1],
           ((unsigned char*)&pkt->ntpd.Reference_Identifier)[2],
           ((unsigned char*)&pkt->ntpd.Reference_Identifier)[3],
           tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,USEC(pkt->ntpd.Reference_Timestamp.fine));
    t = htonl(pkt->ntpd.Origin_Timestamp.coarse) - JAN_1970;
    tm = gmtime(&t);
    printf("Origin Timestamp:\t%04u-%02u-%02u %02u:%02u:%02u.%.6u\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,USEC(pkt->ntpd.Origin_Timestamp.fine));
    t = htonl(pkt->ntpd.Receive_Timestamp.coarse) - JAN_1970;
    tm = gmtime(&t);
    printf("Receive Timestamp:\t%04u-%02u-%02u %02u:%02u:%02u.%.6u\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,USEC(pkt->ntpd.Receive_Timestamp.fine));
    t = htonl(pkt->ntpd.Transmit_Timestamp.coarse) - JAN_1970;
    tm = gmtime(&t);
    printf("Transmit Timestamp:\t%04u-%02u-%02u %02u:%02u:%02u.%.6u\n"
           "-------------------------------------------------------------------\n",
           tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,USEC(pkt->ntpd.Transmit_Timestamp.fine));
    LeaveCriticalSection(&CriticalSection);
}
/*************************************************************************************************************
 * NetworkInit(SOCKET *s)
 * ������*s : SOCKET
 * ���ܣ���ʼ��SOCKET
 *************************************************************************************************************/
int NetworkInit(SOCKET *s) {
    WSADATA  WSAData;
	int  ret = 0, TimeOut = 1000;
    printf("Initializing Network:%38s"," ");
    do {
        if(WSAStartup(MAKEWORD(2,2),&WSAData)!=0) {ret = 1; break;}
        if((*s=WSASocket(AF_INET,SOCK_DGRAM,IPPROTO_UDP,NULL,0,WSA_FLAG_OVERLAPPED))==INVALID_SOCKET) {ret = 2; break;}
        if(setsockopt(*s,SOL_SOCKET,SO_SNDTIMEO,(char*)&TimeOut,sizeof(int))==SOCKET_ERROR) {ret = 3; break;}
        if(setsockopt(*s,SOL_SOCKET,SO_RCVTIMEO,(char*)&TimeOut,sizeof(int))==SOCKET_ERROR) {ret = 4; break;}
        INI_SUCCESS();
	    return ret;
    } while(0);
    INI_FAILED(WSAGetLastError());
    return ret;
}
/*************************************************************************************************************
 * int BindPort(SOCKET *s, struct sockaddr_in *saddr)
 * ������*s     : SOCKET
 *       *saddr : struct sockaddr_in
 * ���ܣ��󶨶˿�
 *************************************************************************************************************/
int BindPort(SOCKET *s, struct sockaddr_in *saddr) {
    printf("Bind the port %u:\t%35s",htons(saddr->sin_port)," ");
    if(bind(*s, (struct sockaddr*)saddr, addr_len)==SOCKET_ERROR) {
        INI_FAILED(WSAGetLastError());
		return 1;
	}
    INI_SUCCESS();
    return 0;
}
/*************************************************************************************************************
 * int StartInit()
 * ������
 * ���ܣ���ʼ����Ļ��ʾ
 *************************************************************************************************************/
int StartInit() {
    hIn   = GetStdHandle(STD_INPUT_HANDLE);
    hOut  = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    SetConsoleTextAttribute(hOut, 7);
    /* ��ʼ�����ݰ���ʾ�ٽ��� */
    if(!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x4000)) return 1;
    return 0;
}
/*************************************************************************************************************
 * int cls(HANDLE hOut, CONSOLE_SCREEN_BUFFER_INFO *bInfo, unsigned int r)
 * ������hOut   : STD_OUTPUT_HANDLE
 *       *bInfo : CONSOLE_SCREEN_BUFFER_INFO
 *       r      : �ӵڼ��п�ʼ����
 * ���ܣ�����
 *************************************************************************************************************/
int cls(HANDLE hOut, CONSOLE_SCREEN_BUFFER_INFO *bInfo, unsigned int r) {
    static COORD home = {0, 0};
    static unsigned long NumberOfCharsWritten;
    static unsigned long size = 0;
    home.Y = r;
    size = bInfo->dwSize.X * bInfo->dwSize.Y;
    FillConsoleOutputAttribute(hOut,bInfo->wAttributes,size,home,&NumberOfCharsWritten);
    FillConsoleOutputCharacter(hOut,' ',size,home,&NumberOfCharsWritten);
    SetConsoleCursorPosition(hOut, home);
    return NumberOfCharsWritten;
}
/*************************************************************************************************************
 * DWORD WINAPI MoreInfo(void *display_stat)
 * ������*display_stat : ��ʾͳ����Ϣ�ĺ���ָ��
 * ���ܣ�����Alt+S��ʾͳ����Ϣ������Alt+D��ʾÿ�����ݰ�����ϸ��Ϣ
 *************************************************************************************************************/
DWORD WINAPI MoreInfo(void *display_stat) {
    unsigned long NumberOfEventsRead = 0;
    INPUT_RECORD keyRes;
    for(;;) {
        Sleep(1);
        /* ��ȡ������� */
        ReadConsoleInput(hIn, &keyRes, 1, &NumberOfEventsRead);
        /* �Ƿ��Ǽ��̲��� */
        if(KEY_EVENT != keyRes.EventType) continue;
        /* �Ƿ�����Alt�� */
        if(!(keyRes.Event.KeyEvent.dwControlKeyState & LEFT_ALT_PRESSED)) continue;
        /* Alt+S */
        if(keyRes.Event.KeyEvent.wVirtualKeyCode==0x53 && keyRes.Event.KeyEvent.bKeyDown) {
            if( 1 == DETAIL ) {
                DETAIL = 0;
                cls(hOut, &bInfo, 0);
            } else {
                DETAIL = 1;
                cls(hOut, &bInfo, 0);
                printf("Statistics :  Sent         Receive      Correct      Wrong                     \n");
                (*(void (*)())display_stat)();
            }
        }
        /* Alt+D */
        else if(keyRes.Event.KeyEvent.wVirtualKeyCode==0x44 && keyRes.Event.KeyEvent.bKeyDown) {
            DETAIL = (DETAIL==2) ? 0 : 2;
            cls(hOut, &bInfo, 0);
        }
    }
}
/*************************************************************************************************************
 * BOOL WINAPI ConsoleHandler(DWORD Event)
 * ������Event : ����̨�����¼�
 * ���ܣ�����̨�¼�����ص�����
 *************************************************************************************************************/
BOOL WINAPI ConsoleHandler(DWORD Event) {
    switch(Event) {
    case CTRL_C_EVENT:
        ;
    case CTRL_BREAK_EVENT:
        break;
    case CTRL_CLOSE_EVENT:
        ;
    case CTRL_SHUTDOWN_EVENT:
        fprintf(stderr,"quit\n");
        break;
    default:
        break;
    }
    return FALSE;
}
/*************************************************************************************************************
 * DWORD WINAPI DispPacket(void *p)
 * ������*p : û���κ�����
 * ���ܣ��ڵ������߳�����ʾPacket����ϸ��Ϣ����ʱ����Ҫ
 *************************************************************************************************************/
DWORD WINAPI DispPacket(void *p) {
    //time_t t;
    //struct tm *tm;
    for(;;) {
        if( WaitForSingleObject(DISP, INFINITE) ) Sleep(1000);
            //PRINT("WaitSemaphore DISP error!!!", "DispPacket", 1);
        /*
        ����
        displayPacket(&dpk, dsr, Direction);
        �ͷ���
        */
    }
}