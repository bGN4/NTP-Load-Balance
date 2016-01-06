#ifndef __STRUCT_CUSTOM__
#define __STRUCT_CUSTOM__

#include <winsock2.h>

struct ID {
    unsigned int src_id;    /* 消息发送方的id */
    unsigned int dst_id;    /* 消息接收方的id */
    unsigned int usr_id;    /* 发送“时间请求”时填写，回复“时间应答”消息时，其值要与请求消息保持一致 */
    unsigned int msg_tp;    /* 消息类型：0, 时间请求；1, 时间应答；2, 心跳请求；3, 心跳应答 */
};
struct ntptime {                           /* NTP 64位 UTC时间 */
	unsigned int coarse;                   /* 精确到秒 */
	unsigned int fine;                     /* 秒以下 */
};
struct NTP_DATA{                           /* 定义NTP数据结构 */
	unsigned char  Flags;                  /* LI(2bit),VERSION(3bit),MODE(3bit) */
    unsigned char  Peer_Clock_Stratum;     /* 距离UTC源的远近 */
             char  Peer_Polling_Interval;  /* */
             char  Peer_Clock_Precision;   /* */
             long  Root_Delay;             /* */
    unsigned long  Root_Dispersion;        /* */
    unsigned long  Reference_Identifier;   /* 参考源 */
    struct ntptime Reference_Timestamp;    /* 本地上一次被校准的时间 */
    struct ntptime Origin_Timestamp;       /* request离开Client的时间 */
    struct ntptime Receive_Timestamp;      /* request到达Server的时间 */
    struct ntptime Transmit_Timestamp;     /*   reply离开Server的时间 */
};
struct Packet {                            /* 完整数据包 */
    struct ID       id;                    /* ID相关数据  (16 bytes) */
    struct NTP_DATA ntpd;                  /* NTP协议数据 (48 bytes) */
};
struct Server {                            /* Server Info */
    unsigned int           ser_id;         /* Server ID */
    unsigned int           busy;           /* Server繁忙程度 */
             int           delay;          /* 延迟 */
    struct   sockaddr_in   ser;            /* Server 地址 */
    volatile unsigned long time;
};
struct Stat {                              /* 统计信息 */
    unsigned int sent;
    unsigned int correct;
    unsigned int wrong;
};
struct Mail{                               /* 数据包在处理流水线上的完整信息 */
    struct   Packet        pkt;            /* 待处理的数据包 */
    struct   sockaddr_in   src;            /* 从哪来 */
    struct   sockaddr_in   dst;            /* 到哪去 */
    volatile int           stat;           /* 进行到哪了 */
};

#endif