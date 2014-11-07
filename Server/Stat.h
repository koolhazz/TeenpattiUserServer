#pragma once
#include "Singleton.h"
#include "noncopyable.h"
#include <map>

using namespace std;

typedef struct _DATAINFO
{
	int onlineCount;
	int onPlayCount;
	int onLookOnCount;
	_DATAINFO()
	{
		onlineCount = 0;
		onPlayCount = 0;
		onLookOnCount = 0;
	}
}DATAINFO;

typedef struct _LEVELINFO
{
	int playCount;
	int lookCount;
	_LEVELINFO()
	{
		playCount = 0;
		lookCount = 0;
	}
}LEVELINFO;

typedef struct _USERSTAT
{
	int terminalType;
	int tid;
}USERSTAT;

enum 
{
	TERMINAL_LOGINTABLE = 1,
	TERMINAL_LOGOUTTABLE = 2,
	TERMINAL_LOGINHALL = 3,
	TERMINAL_LOGOUTHALL = 4,
	TERMINAL_STANDTABLE = 5,
	TERMINAL_STANDLEAVETABLE = 6,
};

enum 
{
	LEVEL_LOGIN = 1,
	LEVEL_LOGOUT = 2,
	LEVEL_STANDIN = 3,
	LEVEL_STANDOUT = 4,
};

class CStat : private noncopyable
{
public:
	map<short,DATAINFO> m_terminalStatMap;
	map<short, LEVELINFO> m_LevelMgr;
public:
	static CStat * Instance (void);
	int UserLoginHall(int uid,int terminalType,USERSTAT * preStat);
	int UserLogoutTable(int uid,int terminalType);
	int UserLogoutHall(int uid,int terminalType);
	int UserLoginTable(int uid,int terminalType);
	int UserStandTable(int uid,int terminalType);
	int UserStandLeaveTable(int uid, int terminalType);
	int UpdateTerminalInfo(int uid,int type,int terminalType,void * privatedata);
	int UpdateLevelInfo(int uid,int type,int level,void * privatedata);

	void PrintStatData();
	
private:
	//nocopyright.
};
