#ifndef _GLOBAL_H_

#define DEBUG_LOG 1

#ifdef DEBUG_LOG
#define DebugMsg(format,...)	g_pDebugLog->logMsg("[%s %u]%s||"format,__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);
#define ErrorMsg(format,...)	g_pErrorLog->logMsg("[%s %u]%s||"format,__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);
#else
#define DebugMsg(format,...)
#define ErrorMsg(format,...)
#endif


#define NOT_USE(x) (void)x

#endif




