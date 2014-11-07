#ifndef BOYAA_GAME_USER_H
#define BOYAA_GAME_USER_H

#include "Global.h"
#include <vector>

using namespace std;

#pragma pack(1)
class CGameUser
{
public:
	CGameUser(int uid, int svid, short terminalType, int updateTime);
	CGameUser();
	~CGameUser(void);
public:
	int m_nUid;				//用户id
	int m_nHallServerId;	//用户所在Hall server id
	int m_nTid;				//用户桌子id
	short m_nTerminalType;	//用户终端类型
	short m_nServerLevel;	//用户所在server等级
	int m_nUpdateTime;		//用户信息更新时间
};
#pragma pack()
#endif
