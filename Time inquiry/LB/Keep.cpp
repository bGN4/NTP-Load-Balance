#pragma warning(disable : 4786)
#include <set>
#include <time.h>
#include "../struct.h"
#include "../defines.h"
#include "errorlevel.h"
struct Keep {
    unsigned int   key;
    unsigned int   value;
    unsigned int   Timeout;                /* 会话超时 */
    bool operator< (const Keep &p) const { /* 重载<运算符 */
		return key < p.key;
	};
}Keep;
struct Keep ki = {0, 0, 30}, ks = {0, 0, 30};
std::set<struct Keep> ConKeep;
std::set<struct Keep>::iterator CKis, CKid;
std::pair<std::set<struct Keep>::iterator,bool> pair;
extern "C" HANDLE   Mutex_k;
extern "C" char     dt[16], tm[16];
extern "C" struct   Server  *sInfo;
/*************************************************************************************************************
 * extern "C" unsigned int Select_k(unsigned int id)
 * 参数：*id : key
 * 功能：通过KEY找VALUE
 *************************************************************************************************************/
extern "C" unsigned int Select_k(unsigned int id) {
    ks.key = id;
    if( WaitForSingleObject(Mutex_k, INFINITE) )
        PRINT(STD_ERROR, STD_TIME, "Wait Mutex_k error!!!", "Select", GetLastError());
    CKis = ConKeep.find(ks);
    if( CKis == ConKeep.end() ) {
        id = -1;
    } else {
        CKis->Timeout = 30;
        id = CKis->value;
    }
    if( !ReleaseMutex(Mutex_k) )
        PRINT(STD_ERROR, STD_TIME, "Release Mutex_k error!!!", "Select", GetLastError());
    return id;
}
/*************************************************************************************************************
 * extern "C" int Insert_k(unsigned int id, unsigned int value)
 * 参数：id    : key
 *       value : 位置
 * 功能：插入KEY-VALUE对
 *************************************************************************************************************/
extern "C" int Insert_k(unsigned int id, unsigned int value) {
    ki.key   = id;
    ki.value = value;
    if( WaitForSingleObject(Mutex_k, INFINITE) )
        PRINT(STD_ERROR, STD_TIME, "Wait Mutex_k error!!!", "Insert", GetLastError());
    pair = ConKeep.insert(ki);
    if( !ReleaseMutex(Mutex_k) )
        PRINT(STD_ERROR, STD_TIME, "Release Mutex_k error!!!", "Insert", GetLastError());
    return pair.second;
}
/*************************************************************************************************************
 * extern "C" void Delete_k()
 * 参数：
 * 功能：删除超时的KEY-VALUE对
 *************************************************************************************************************/
extern "C" void Delete_k() {
    static unsigned int j,size;
    j = WaitForSingleObject(Mutex_k, 100);
    if( WAIT_TIMEOUT == j ) return;
    else if( j ) {
        PRINT(STD_ERROR, STD_TIME, "Wait Mutex_k error!!!", "Delete", GetLastError());
    }
    size = ConKeep.size();
    CKid = ConKeep.begin();
    for(j=0 ; j<size ; j++) {
        if( !sInfo[CKid->value].delay ) {
            CKid = ConKeep.erase(CKid);
            continue;
        }
        if(CKid->Timeout-- >= 0) CKid++; /* 没超时，继续减 */
        else CKid = ConKeep.erase(CKid); /* 已超时，删除 */
    }
    if( !ReleaseMutex(Mutex_k) )
        PRINT(STD_ERROR, STD_TIME, "Release Mutex_k error!!!", "Delete", GetLastError());
}