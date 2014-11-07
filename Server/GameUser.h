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
	int m_nUid;				//�û�id
	int m_nHallServerId;	//�û�����Hall server id
	int m_nTid;				//�û�����id
	short m_nTerminalType;	//�û��ն�����
	short m_nServerLevel;	//�û�����server�ȼ�
	int m_nUpdateTime;		//�û���Ϣ����ʱ��
};
#pragma pack()
#endif
