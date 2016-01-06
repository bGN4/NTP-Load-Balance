#ifndef __STRUCT_CUSTOM__
#define __STRUCT_CUSTOM__

#include <winsock2.h>

struct ID {
    unsigned int src_id;    /* ��Ϣ���ͷ���id */
    unsigned int dst_id;    /* ��Ϣ���շ���id */
    unsigned int usr_id;    /* ���͡�ʱ������ʱ��д���ظ���ʱ��Ӧ����Ϣʱ����ֵҪ��������Ϣ����һ�� */
    unsigned int msg_tp;    /* ��Ϣ���ͣ�0, ʱ������1, ʱ��Ӧ��2, ��������3, ����Ӧ�� */
};
struct ntptime {                           /* NTP 64λ UTCʱ�� */
	unsigned int coarse;                   /* ��ȷ���� */
	unsigned int fine;                     /* ������ */
};
struct NTP_DATA{                           /* ����NTP���ݽṹ */
	unsigned char  Flags;                  /* LI(2bit),VERSION(3bit),MODE(3bit) */
    unsigned char  Peer_Clock_Stratum;     /* ����UTCԴ��Զ�� */
             char  Peer_Polling_Interval;  /* */
             char  Peer_Clock_Precision;   /* */
             long  Root_Delay;             /* */
    unsigned long  Root_Dispersion;        /* */
    unsigned long  Reference_Identifier;   /* �ο�Դ */
    struct ntptime Reference_Timestamp;    /* ������һ�α�У׼��ʱ�� */
    struct ntptime Origin_Timestamp;       /* request�뿪Client��ʱ�� */
    struct ntptime Receive_Timestamp;      /* request����Server��ʱ�� */
    struct ntptime Transmit_Timestamp;     /*   reply�뿪Server��ʱ�� */
};
struct Packet {                            /* �������ݰ� */
    struct ID       id;                    /* ID�������  (16 bytes) */
    struct NTP_DATA ntpd;                  /* NTPЭ������ (48 bytes) */
};
struct Server {                            /* Server Info */
    unsigned int           ser_id;         /* Server ID */
    unsigned int           busy;           /* Server��æ�̶� */
             int           delay;          /* �ӳ� */
    struct   sockaddr_in   ser;            /* Server ��ַ */
    volatile unsigned long time;
};
struct Stat {                              /* ͳ����Ϣ */
    unsigned int sent;
    unsigned int correct;
    unsigned int wrong;
};
struct Mail{                               /* ���ݰ��ڴ�����ˮ���ϵ�������Ϣ */
    struct   Packet        pkt;            /* ����������ݰ� */
    struct   sockaddr_in   src;            /* ������ */
    struct   sockaddr_in   dst;            /* ����ȥ */
    volatile int           stat;           /* ���е����� */
};

#endif