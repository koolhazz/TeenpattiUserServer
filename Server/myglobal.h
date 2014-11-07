#pragma once
#include "clib_log.h"

//config and globle module.



//#define LogMsg(format,...) TGlobal::g_pFlowLog->logMsg("[%s %u][%s]"format,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define LogMsg(format,...)  


class TGlobal
{
	public:
		static clib_log *	g_pFlowLog;
	private://no impl
		TGlobal();
		~TGlobal();
		TGlobal (const TGlobal&);
    	const TGlobal& operator= (const TGlobal&);
};
