#include <time.h>
#include <stdio.h>
#include <stdarg.h>
extern   FILE  *lg;
extern   char  dt[16], tm[16];
/*************************************************************************************************************
 * extern __inline void PRINT(char *fmt, ...)
 * ������*fmt : ������ʽͬprintf
 * ���ܣ���ӡ�쳣��Ϣ����־����Ļ
 *************************************************************************************************************/
extern __inline void PRINT(char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(lg, fmt, argp);
    fflush(lg);
    va_end(argp);
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
}
