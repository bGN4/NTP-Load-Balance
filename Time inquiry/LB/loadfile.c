#include "luah.h"
#include "../struct.h"
#include "../defines.h"
#include "errorlevel.h"
extern FILE            *lg;
extern HANDLE          hOut;
extern char            dt[16], tm[16];
extern unsigned int    LB_ID, MAX_Ser, OffSet;
extern unsigned short  Cport, Sport;
extern struct   Server *sInfo;
char * __cdecl  _strdate(char *);
char * __cdecl  _strtime(char *);
void   (WINAPI* Balance)(unsigned int *);
void   (WINAPI* setsInfo)(unsigned int max, struct Server *p, int *m);
void   __stdcall Circular(unsigned int *);
void   __stdcall Prorate(unsigned int *);
void   __stdcall Fast(unsigned int *);
char      *dllfile  = NULL;
int       *weigh    = NULL;
HINSTANCE hInst     = NULL;
/*************************************************************************************************************
 * static void stackDump(lua_State *L)
 * 参数：*L  : Lua State
 * 功能：Lua Stack Dump
 *************************************************************************************************************/
static void stackDump(lua_State *L) {
    int i,top = lua_gettop(L);
    SetConsoleTextAttribute(hOut, 14);
    printf( "\n==========================Lua Stack Dump==========================\n");
    printf( "index: %d\n", top );
    for (i = 1; i <= top; i++) {
        int t = lua_type(L, i);
        switch (t) {
            case LUA_TSTRING:
                printf("'%s'\n", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                printf(lua_toboolean(L, i) ? "true\n" : "false\n");
                break;
            case LUA_TNUMBER:
                printf("%g\n", lua_tonumber(L, i));
                break;
            default:
                printf("%s\n", lua_typename(L, t));
                break;
        }
    }
    printf( "========================Lua Stack Dump END========================\n");
    SetConsoleTextAttribute(hOut, 7);
}
/*************************************************************************************************************
 * __inline int lua_err(lua_State *L, int err)
 * 参数：*L  : Lua State
 *       err : Error Code
 * 功能：输出Lua错误信息
 *************************************************************************************************************/
__inline int lua_err(lua_State *L, int err) {
    printf("\n");
    PRINT("%s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
    lua_close(L);
    return err;
}
/*************************************************************************************************************
 * void showconfig(struct Server * sInfo, FILE* f)
 * 参数：*sInfo : 指向 Server Info 的指针
 *       *f     : 输出文件流
 * 功能：输出配置信息
 *************************************************************************************************************/
void showconfig(struct Server * sInfo, FILE* f) {
    unsigned int i = 0;
    fprintf(f,"-------------------------%s %s-------------------------\n"
              "LB_id=%-12uMAX_server=%-12uOffSet=%-12d\nCport=%-12huSport=%-17hu\n",
              _strdate(dt),_strtime(tm),LB_ID,MAX_Ser,OffSet,Cport,Sport);
    for(i=0 ; i<MAX_Ser ; i++)
        fprintf(f,"Server%u: ID=%u IP=%s Port=%hu Weight=%u\n",i,sInfo[i].ser_id,
                inet_ntoa(sInfo[i].ser.sin_addr),htons(sInfo[i].ser.sin_port),sInfo[i].busy);
    fprintf(f,"-------------------------------------------------------------------\n");
    fflush(f);
}
/*************************************************************************************************************
 * int loadconfig(struct Server **psInfo)
 * 参数：**psInfo : 指向 Server Info 的二重指针
 * 功能：读取配置文件，初始化运行参数
 *************************************************************************************************************/
int loadconfig(struct Server **psInfo) {
    unsigned int i, t[5] = {0};
    char *global[] = {CONF_SERV, CONF_SPORT, CONF_CPORT, CONF_LBID};
    lua_State  *L  = luaL_newstate();
    fprintf(stderr, "Loading config file :%38s"," ");
    luaL_openlibs(L);
    if( luaL_dofile(L, CONFIG_FILE) ) return lua_err(L, -1);
    for(i=0 ; i<sizeof(*global) ; i++)
        lua_getglobal(L, global[i]);
    for(i=1 ; i<sizeof(*global) ; i++) {
        if( !lua_isnumber(L, -(int)i) ) return lua_err(L, i);
        t[i] = lua_tounsigned(L, -(int)i);
    }
    if( t[2]>65535 || t[3]>65535 ) return -2;
    LB_ID = t[1];
    Cport = t[2];
    Sport = t[3];
    lua_pop(L, 3);
	if( !lua_istable(L, -1) ) return lua_err(L, -3);
	MAX_Ser = lua_rawlen(L, -1);
    *psInfo  = (struct Server *)malloc((1+MAX_Ser)*sizeof(struct Server));
    memset(*psInfo, 0, (1+MAX_Ser)*sizeof(struct Server));
	for(i=0 ; i<MAX_Ser ; i++) {
		lua_pushnumber(L, i+1);
		lua_gettable(L, -2);
        lua_getfield(L, -1, CONF_S_WEI);
        lua_getfield(L, -2, CONF_S_PORT);
        lua_getfield(L, -3, CONF_S_IP);
        lua_getfield(L, -4, CONF_S_ID);
        if( !lua_isnumber(L, -1) ) return lua_err(L, sizeof(*global)+4*i);
        if( !lua_isstring(L, -2) ) return lua_err(L, sizeof(*global)+4*i+1);
        if( !lua_isnumber(L, -3) ) return lua_err(L, sizeof(*global)+4*i+2);
        if( !lua_isnumber(L, -4) ) return lua_err(L, sizeof(*global)+4*i+3);
        if( (t[0] = lua_tounsigned(L, -3)) > 65535 ) return -4-2*i;
        if( (t[4] = lua_tounsigned(L, -4)) > 10 )    return -5-2*i;
        if( 0 == t[4] ) return -5-2*i;
        (*psInfo)[i].ser_id              = lua_tounsigned(L, -1);
        (*psInfo)[i].ser.sin_family      = AF_INET;
        (*psInfo)[i].ser.sin_addr.s_addr = inet_addr( lua_tostring(L, -2) );
        (*psInfo)[i].ser.sin_port        = htons( (unsigned short)t[0] );
        (*psInfo)[i].busy                = lua_tounsigned(L, -4);
        //(*psInfo)[i].delay               = 0; //memset already 0
        //(*psInfo)[i].time                = 0; //memset already 0
        if( INADDR_NONE == (*psInfo)[i].ser.sin_addr.s_addr ) return -6-2*i;
		lua_pop(L, 5);
	}
	lua_pop(L, 1);
    lua_getglobal(L, CONF_CKEEP);
    lua_getglobal(L, CONF_LBDLL);
    if( !lua_isstring(L, -1) ) return lua_err(L, sizeof(*global)+4*MAX_Ser);
    if( !lua_isstring(L, -2) ) return lua_err(L, sizeof(*global)+4*MAX_Ser+1);
    dllfile = (char*)malloc( sizeof(char) * (1+lua_rawlen(L,-1)) );
    sprintf(dllfile, lua_tostring(L, -1));
    if( !strcmp(lua_tostring(L, -2), CONF_K_SRC) ) {
        OffSet = 0;
    }
    else if( !strcmp(lua_tostring(L, -2), CONF_K_USR) ) {
        OffSet = 2;
    }
    else {
        OffSet = -1;
    }
    lua_pop(L, 2);
    lua_close(L);
    INI_SUCCESS();
    return 0;
}
/*************************************************************************************************************
 * int LoadDllFunc(const char *dll)
 * 参数：*dll : DLL文件名
 * 功能：载入DLL文件并获取负载均衡函数地址
 *************************************************************************************************************/
int LoadDllFunc(const char *dll) {
    dll = dllfile;
    fprintf(stderr, "Loading balance function :%33s"," ");
    hInst = LoadLibrary(dll);
    if( NULL == hInst ) {
        fprintf(lg,STD_ERROR, STD_TIME, "Load DLL File Failed!!!", dll, GetLastError());
        INI_FAILED(GetLastError());
        free(dllfile);
        return 1;
    }
    Balance  = (void*)GetProcAddress(hInst, "_Balance@4");
    setsInfo = (void*)GetProcAddress(hInst, "_setsInfo@12");
    if( NULL == Balance || NULL == setsInfo ) {
        fprintf(lg,STD_ERROR, STD_TIME, "Get DLL Function Failed!!!", dll, GetLastError());
        INI_FAILED(GetLastError());
        FreeLibrary(hInst);
        free(dllfile);
        return -1;
    }
    weigh = (int *)malloc(MAX_Ser * sizeof(int));
    setsInfo(MAX_Ser, sInfo, weigh);
    if( !strcmp(dll, "Circular.dll") ) {
        Balance = Circular;
    }
    else if( !strcmp(dll, "Prorate.dll") ) {
        Balance = Prorate;
    }
    else if( !strcmp(dll, "Fast.dll") ) {
        Balance = Fast;
    }
    INI_SUCCESS();
    free(dllfile);
    return 0;
}