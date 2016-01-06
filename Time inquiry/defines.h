#ifndef __GLOBAL_CONSTANT__
#define __GLOBAL_CONSTANT__

#define _WIN32_WINNT            0x0403
#define BUFFER_MAX              1024                           /* 上下行数据缓冲区大小 */
#define CKEEP_TIMEOUT           30000                          /* 会话保持超时时长 */
#define HEARTBEAT_PERIOD        500                            /* 心跳检测间隔时长 */
#define TIMEOUT_COUNT           2                              /* 会话信息超时 */
#define CONFIG_FILE             "LB.cfg"                       /* 配置文件名 */
#define CONF_LBID               "LB_id"
#define CONF_CPORT              "client_udp_port"
#define CONF_SPORT              "server_udp_port"
#define CONF_LBDLL              "Balance"
#define CONF_CKEEP              "Keep"
#define CONF_K_SRC              "src_id"
#define CONF_K_USR              "usr_id"
#define CONF_SERV               "servers"
#define CONF_S_ID               "id"
#define CONF_S_IP               "ip"
#define CONF_S_PORT             "port"
#define CONF_S_WEI              "weight"
#define STD_ERROR               "%s %s <ERROR> %s(%s:%d)\n"
#define STD_WARN                "%s %s <WARN> %s(%s:%d)\n"
#define STD_INFO                "%s %s <INFO> %s(%s:%d)\n"
#define STD_TIME                _strdate(dt),_strtime(tm)
#define INI_SUCCESS()           do { printf("[      ]\b\b\b\b\b");      \
                                     SetConsoleTextAttribute(hOut, 10); \
                                     printf("OK\n");                    \
                                     SetConsoleTextAttribute(hOut, 7); } while(0)
#define INI_FAILED(e)           do { printf("[      ]\b\b\b\b\b\b\b");  \
                                     SetConsoleTextAttribute(hOut, 12); \
                                     printf("FAILED\n");                \
                                     SetConsoleTextAttribute(hOut, 7);  \
                                     printf(STD_ERROR,STD_TIME,"","ErrorCode",e);} while(0)
#define USE_SAMPLE_CLIENT_SPOT  0

#endif