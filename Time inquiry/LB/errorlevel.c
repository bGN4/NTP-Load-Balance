#include <time.h>
#include <stdio.h>
#include <stdarg.h>
extern   FILE  *lg;
extern   char  dt[16], tm[16];
/*************************************************************************************************************
 * extern __inline void PRINT(char *fmt, ...)
 * 参数：*fmt : 参数格式同printf
 * 功能：打印异常信息到日志和屏幕
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
