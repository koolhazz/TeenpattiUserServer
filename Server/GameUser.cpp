#include "GameUser.h"
#include <time.h>

CGameUser::CGameUser(int uid, int svid, short terminalType, int updateTime)
{
	m_nUid = uid;
	m_nHallServerId = svid;
	m_nTerminalType = terminalType;
	m_nTid = 0;
	m_nServerLevel = -1;
	m_nUpdateTime = updateTime;
}


CGameUser::CGameUser() :
	m_nUid(0), m_nHallServerId(0), m_nTerminalType(0), m_nTid(0), m_nServerLevel(-1), m_nUpdateTime(0)
{
	
}


CGameUser::~CGameUser(void)
{
}
