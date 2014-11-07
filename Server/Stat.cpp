#include "Stat.h"
#include "myglobal.h"

#include "Global.h"

#include "clib_log.h"
extern clib_log *g_pDebugLog;
extern clib_log *g_pErrorLog;


CStat * CStat::Instance()
{
	return CSingleton<CStat>::Instance();
}

int CStat::UserLoginHall(int uid,int terminalType ,USERSTAT * preStat)
{ 
	if(preStat == NULL)
	{
		//new user login
		DATAINFO & statinfo = m_terminalStatMap[terminalType];
		statinfo.onlineCount ++;
		g_pErrorLog->logMsg("User[UID:%d,TERM:%d] Login Hall,Now Stat Info [ONLINE:%d,PLAY:%d]",uid,terminalType,statinfo.onlineCount,statinfo.onPlayCount);
	}
	else
	{
		//reload.
		DATAINFO & statinfo = m_terminalStatMap[preStat->terminalType];
		DATAINFO & newstatinfo = m_terminalStatMap[terminalType];
		if(preStat->terminalType != terminalType)
		{
			
			if(preStat->tid != 0 && statinfo.onPlayCount > 0)
			{
				statinfo.onPlayCount--;
				//tid不为0时
				//先在新的终端统计中加上.
				newstatinfo.onPlayCount++;
			}
			if(statinfo.onlineCount > 0)
			{
				statinfo.onlineCount--;
			}
			newstatinfo.onlineCount++;
		}
		else
		{
			//重新在相同的终端登录.
		}
		g_pErrorLog->logMsg("User[UID:%d,TERM:%d] Re Load Hall,Now Stat Info [ONLINE:%d,PLAY:%d],Pre Stat Info[TERM:%d,TID:%d]",\
			uid,terminalType,newstatinfo.onlineCount,newstatinfo.onPlayCount,preStat->terminalType,preStat->tid);
	}
	return 0;
}

int CStat::UserLogoutHall(int uid,int terminalType)
{
	DATAINFO & statinfo = m_terminalStatMap[terminalType];
	if(statinfo.onlineCount> 0)
	{
		statinfo.onlineCount--;
	}
	g_pErrorLog->logMsg("User[UID:%d,TERM:%d] Logout Hall,Now Stat Info [ONLINE:%d,PLAY:%d]",uid,terminalType,statinfo.onlineCount,statinfo.onPlayCount);
	return 0;
}

int CStat::UserLogoutTable(int uid,int terminalType)
{
	DATAINFO & statinfo = m_terminalStatMap[terminalType];
	if(statinfo.onPlayCount> 0)
	{
		statinfo.onPlayCount--;
	}

	return 0;
}

int CStat::UserStandTable(int uid,int terminalType)
{
	DATAINFO & statinfo = m_terminalStatMap[terminalType];
	//if(statinfo.onLookOnCount> 0)
	//{
		statinfo.onLookOnCount++;
	//}
	//g_pErrorLog->logMsg("User[UID:%d,TERM:%d] Logout Table,Now Stat Info [ONLINE:%d,PLAY:%d,LOOK:%d]",uid,terminalType,statinfo.onlineCount,statinfo.onPlayCount,statinfo.onLookOnCount);
	return 0;
}

int CStat::UserStandLeaveTable(int uid, int terminalType)
{
	DATAINFO & statinfo = m_terminalStatMap[terminalType];
	if(statinfo.onLookOnCount> 0)
	{
		statinfo.onLookOnCount--;
	}
	//g_pErrorLog->logMsg("User[UID:%d,TERM:%d] Logout Table,Now Stat Info [ONLINE:%d,PLAY:%d,LOOK:%d]",uid,terminalType,statinfo.onlineCount,statinfo.onPlayCount,statinfo.onLookOnCount);
	return 0;
}

int CStat::UserLoginTable(int uid,int terminalType)
{
	DATAINFO & statinfo = m_terminalStatMap[terminalType];
	statinfo.onPlayCount++;
	g_pErrorLog->logMsg("User[UID:%d,TERM:%d] Login Table,Now Stat Info [ONLINE:%d,PLAY:%d]",uid,terminalType,statinfo.onlineCount,statinfo.onPlayCount);
	return 0;
}

int CStat::UpdateTerminalInfo(int uid,int type,int terminalType,void * privatedata)
{
	
	switch(type)
	{
		case TERMINAL_LOGINHALL:
		{
			USERSTAT * preStat = NULL;
			if(privatedata != NULL)
			{
				preStat = (USERSTAT *)privatedata;
			}
			UserLoginHall(uid,terminalType ,preStat);
			break;
		}
		case TERMINAL_LOGOUTHALL:
		{
			UserLogoutHall(uid,terminalType);
			break;
		}
		case TERMINAL_LOGINTABLE:
		{
			UserLoginTable(uid,terminalType);
			break;
		}
		case TERMINAL_LOGOUTTABLE:
		{
			UserLogoutTable(uid,terminalType);
			break;
		}
		case TERMINAL_STANDTABLE:
		{
			UserStandTable(uid,terminalType);
			break;
		}
		case TERMINAL_STANDLEAVETABLE:
		{
			UserStandLeaveTable(uid,terminalType);
			break;
		}
	}
//	PrintStatData();
	return 0;
}
int CStat::UpdateLevelInfo(int uid,int type,int level,void * privatedata)
{
	switch(type)
	{
		case LEVEL_LOGIN:
		{
			map<short,LEVELINFO>::iterator iterLevel = m_LevelMgr.find(level);
			if(iterLevel != m_LevelMgr.end())
			{
				(iterLevel->second).playCount++;
			}
			else
			{
				LEVELINFO levelinfo;
				levelinfo.playCount = 1;
				levelinfo.lookCount = 0;
				m_LevelMgr[level] = levelinfo;
				g_pErrorLog->logMsg("Add User[UID:%d] To New Level[%d]",uid,level);
			}
			break;
		}
		case LEVEL_LOGOUT:
		{
			map<short, LEVELINFO>::iterator iterLevel = m_LevelMgr.find(level);
			if(iterLevel != m_LevelMgr.end())
			{
				LEVELINFO & levelinfo = m_LevelMgr[level];
				if ( levelinfo.playCount > 0 ) 
				{
					levelinfo.playCount --;
				}
			}
			else
			{
				g_pErrorLog->logMsg("Remove User[UID:%d] from Level[%d] which not exist",uid,level);
			}
			break;
		}
		case LEVEL_STANDIN:
		{
			map<short, LEVELINFO>::iterator iterLevel = m_LevelMgr.find(level);
			if(iterLevel != m_LevelMgr.end())
			{
				//int& count = iterLevel->second;
				//if(count > 0)
				//	count--;
				LEVELINFO & levelinfo = m_LevelMgr[level];
				levelinfo.lookCount ++;
				//g_pErrorLog->logMsg("Remove User[UID:%d] from Level[%d],now levelinfo.lookCount:%d",uid,level,levelinfo.lookCount);
			}
			else
			{
				g_pErrorLog->logMsg("LEVEL_STANDIN Remove User[UID:%d] from Level[%d] which not exist",uid,level);
			}
			break;
		}
		case LEVEL_STANDOUT:
		{
			map<short, LEVELINFO>::iterator iterLevel = m_LevelMgr.find(level);
			if(iterLevel != m_LevelMgr.end())
			{
				//int& count = iterLevel->second;
				//if(count > 0)
				//	count--;
				LEVELINFO & levelinfo = m_LevelMgr[level];
				if ( levelinfo.lookCount > 0 ) 
				{
					levelinfo.lookCount --;
				}
				g_pErrorLog->logMsg("Remove User[UID:%d] from Level[%d],now levelinfo.lookCount:%d",uid,level,levelinfo.lookCount);
			}
			else
			{
				g_pErrorLog->logMsg("LEVEL_STANDOUT Remove User[UID:%d] from Level[%d] which not exist",uid,level);
			}
			break;
		}
	}

//	PrintStatData();
	return 0;
}

void CStat::PrintStatData()
{
	map<short,DATAINFO>::iterator it1 = m_terminalStatMap.begin();
	for(; it1 != m_terminalStatMap.end(); it1++)
	{
		DATAINFO	&data = it1->second;
		//g_pErrorLog->logMsg("terminal[%d] OnlineCount[%d] PlayeCount[%d]", it1->first, data.onlineCount, data.onPlayCount);
	}

	//map<short, int>::iterator it2 = m_LevelMgr.begin();
	//for(; it2 != m_LevelMgr.end(); it2++)
	//{
	//	g_pErrorLog->logMsg("Level[%d] LevelCount[%d] ", it2->first, it2->second);	
	//}
}

