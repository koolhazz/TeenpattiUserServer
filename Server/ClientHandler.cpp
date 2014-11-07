#include "ClientHandler.h"
#include "Global.h"
#include "GameCmd.h"
#include "Stat.h"


#include "HashMap.hpp"
#include "GameUserSHM.h"
#include "ShmHashMap.hpp"

using namespace comm::commu;

#include "Options.h"

using namespace std;

#include "clib_log.h"
extern clib_log *g_pDebugLog;
extern clib_log *g_pErrorLog;

static const int LevelArr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ,11, 12};

#define SYS_CTRL_LEVELCOUNT_RANDOM "sys_ctrl_level_count_random"


GameUserSHM gshm;


static string ToLower(const string & str) {
	if(str.length() == 0)
		return "";

	char buf[512] = {0};
	snprintf(buf, sizeof(buf), "%s", str.c_str());
	int len = strlen(buf);
	for(int i=0; i<len; ++i)
	{
		if(buf[i] >= 'A' && buf[i] <= 'Z')
			buf[i] += ('a'-'A');
	}

	return string(buf);
}


void GetTime(string& strTm)
{
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	char szTm[100] = {0};
	snprintf(szTm, sizeof(szTm), "[%04d-%02d-%02d %02d:%02d:%02d]", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	strTm = szTm;
}

static int split_str(const char* ps_str , char* ps_sp , vector<std::string> &v_ret)
{    
	char* ps_temp;    
	char* p;    
	int i_len = (int)strlen(ps_str);    
	std::string st_str;    
	ps_temp = new char[i_len+2];    
	snprintf(ps_temp , i_len+1 , "%s" , ps_str);    
	char *last = NULL;    
	p = strtok_r(ps_temp , ps_sp , &last);    
	if(NULL == p)
	{        
		delete ps_temp;        
		return 0;    
	}    
	st_str = (std::string)p;    
	v_ret.push_back(st_str);    
	while( NULL != ( p=strtok_r(NULL , ps_sp , &last) ) )
	{        
		st_str = (std::string)p;        
		v_ret.push_back(st_str);    
	}    
	delete ps_temp;    
	return 0;
}

static int m_nRandCount = 0;

static int GetRand()
{
	if(m_nRandCount > 1000000000)
		m_nRandCount = 0;
	srand((int)time(NULL)+m_nRandCount++);
	return rand() % Options::instance()->m_nRoundLimit;
}


static int CheckPacketComplete(const char * pData,const int & nLen)
{
	if( NULL == pData)
	{
		return 0;

	}

	int nHeadLen = sizeof(struct TPkgHeader);
	if(nLen < nHeadLen)
	{
		g_pErrorLog->logMsg("%s||Invalid Packet nLen[%d] HeadLen[%d]", __FUNCTION__, nLen, nHeadLen);
		return 0;
	}

	TPkgHeader *pHeader = (struct TPkgHeader*)pData;
	if(pHeader->flag[0]!='B' || pHeader->flag[1]!='Y')
	{
		g_pErrorLog->logMsg("%s||Invalid Packet", __FUNCTION__);

	}
	int pkglen = sizeof(short) + ntohs(pHeader->length);	//转换成大端
	if(pkglen<0 || pkglen>8*1024)
	{
		g_pErrorLog->logMsg("%s||Invalid packet, pkglen:[%d]", __FUNCTION__, pkglen);
		return 0;
	}
//	g_pDebugLog->logMsg("%s||Len[%d] pkgLen[%d]", __FUNCTION__, nLen, pkglen);
	if(nLen != pkglen)
	{
		g_pErrorLog->logMsg("%s||Len[%d] pkgLen[%d]", __FUNCTION__, nLen, pkglen);
		return 0;
	}
	return 1;
	
}


ClientHandler::ClientHandler()
{
	
}

ClientHandler::~ClientHandler()
{
	this->_decode = NULL;

}

int ClientHandler::OnClose()
{
	DebugMsg("ip[%s] port[%d] fd[%d] close", m_addrRemote.c_str(), m_nPort, netfd);
	return 0;

}


int ClientHandler::OnConnected(void)
{
	GetRemoteAddr();
	DebugMsg("ip[%s] port[%d] fd[%d] connect", m_addrRemote.c_str(), m_nPort, netfd);
	return 0;

}


void ClientHandler::GetRemoteAddr(void)
{
	struct in_addr sin_addr;
	sin_addr.s_addr = this->_ip;
	char * addr = inet_ntoa(sin_addr);
	m_nPort = this->_port;
	
	if(addr)
		m_addrRemote = string(addr);
	else
		m_addrRemote = "NULL";
}

void ClientHandler::SetServerUserTid0(short sid)
{
	std::list<int> tid0User;
	CShmHashMap<int>::vistor v(gshm.m_ShmHashMap);
	HashNode<int> * pNode = v.begin();
	
	while(NULL != pNode) {
		CGameUser user;
		int len = sizeof(CGameUser);

		int ret = v.GetData(pNode, (char *)&user, &len);
		if(ret == -1 || len != sizeof(CGameUser))
		{
			pNode = v.next(pNode);
			continue;
		}

		if(user.m_nTid > 0 && (user.m_nTid >> 16) == sid)
		{
			user.m_nTid = 0;
			CStat::Instance()->UpdateTerminalInfo(user.m_nUid,TERMINAL_LOGOUTTABLE, user.m_nTerminalType,NULL);

			if(user.m_nServerLevel != -1)
			{
				CStat::Instance()->UpdateLevelInfo(user.m_nUid,LEVEL_LOGOUT, user.m_nServerLevel,NULL);
				user.m_nServerLevel = -1;
			}

			tid0User.push_back(user.m_nUid);
		}
				
		pNode = v.next(pNode);
	}

	std::list<int>::iterator it = tid0User.begin();
	for(; it!=tid0User.end(); ++it) {
		CGameUser user;
		if(gshm.Find(*it, user) == 0) {
			user.m_nTid = 0;
			user.m_nServerLevel = -1;
			gshm.SetValue(user);
		}
	}

}


void ClientHandler::SetUserOfflineHallCore(int hid)
{
	std::list<int> userHallCore;
	CShmHashMap<int>::vistor v(gshm.m_ShmHashMap);
	HashNode<int> * pNode = v.begin();
	while(NULL != pNode) {
		CGameUser user;
		HashNode<int> * pTempNode = pNode;
		int len = sizeof(CGameUser);

		int ret = v.GetData(pNode, (char *)&user, &len);
		if(ret == -1 || len != sizeof(CGameUser))
		{
			pNode = v.next(pNode);
			continue;
		}

		if(user.m_nHallServerId == hid)
		{
			map<int, int>::iterator iter = Options::instance()->m_OfflineUserMgr.find(user.m_nUid);
			if(iter == Options::instance()->m_OfflineUserMgr.end())
			{
				Options::instance()->m_OfflineUserMgr[user.m_nUid] = (int)time(NULL);
			}	

			userHallCore.push_back(user.m_nUid);
		}
				
		pNode = v.next(pNode);
	}

	std::list<int>::iterator it = userHallCore.begin();
	for(; it!=userHallCore.end(); ++it) {
		CGameUser user;
		
		if(gshm.Find(*it, user) == 0) {
			user.m_nHallServerId = -1;
			gshm.SetValue(user);
		}
	}

}


int ClientHandler::OnPacketComplete(const char * data,int len)
{
	g_pErrorLog->logMsg("----------------- ClientHandler::OnPacketComplete Begin ----------------");
	g_pErrorLog->logMsg("len [%d]", len);
	pPacket.Copy(data,len);
	//*****处理包****/
	return ProcessPacket(&pPacket);

}

int ClientHandler::ProcessPacket(InputPacket * pPacket)
{
	g_pErrorLog->logMsg("----------------- ClientHandler::ProcessPacket Begin ----------------");

	short nCmd = pPacket->GetCmdType();
//	DebugMsg("CMD[%x]", nCmd);

	g_pErrorLog->logMsg("CMD [0x%x]",nCmd);

	switch(nCmd)
	{
	case ALLOC_REGISTER_PACKET:
		return ProcessAllocRegister(pPacket);
	case SYS_GET_USER_TABLE:
		return ProcessGetUserTable(pPacket);
	case CLIENT_PACKET2:
	case CLIENT_PACKET:
		return ProcessSubPacket(pPacket);
	case CLIENT_CLOSE_PACKET:
		return ProcessClientClose(pPacket);
	case SYS_USER_LEAVE_ROOM:
		return ProcessReportUserLeaveRoom(pPacket);
	case SYS_USER_ENTER_ROOM:
		return ProcessReportUserEnterRoom(pPacket);
	case SYS_USER_STAND_ROOM:
		return ProcessReportUserStandRoom(pPacket);
	case SYS_STAND_LEAVE_ROOM:
		return ProcessReportStandLeaveRoom(pPacket);
	case SYS_LOGIC_CORE:
		return ProcessLogicCore(pPacket);
	case SYS_HALL_CORE:
		return ProcessHallCore(pPacket);
	case CMD_CLEAR_SERVERINFO:
		return ProcessClearServerInfo(pPacket);
	case SYS_SET_TJ_SWITCH:
		return ProcessTJSwitch(pPacket);
	case GET_USER_HALLID:
		return ProcessGetUserHallID(pPacket);
	case SYS_SET_LEVEL_COUNT_RANDOM:
		return ProcessSetLevelCountRandom(pPacket);
	default:
		g_pErrorLog->logMsg("%s||Invalid cmd:[%d]", __FUNCTION__,nCmd);
		break;		
	}	
	return 0;

}

int ClientHandler::ProcessAllocRegister(InputPacket * pPacket)
{
	NOT_USE(pPacket);
	return 0;
}

int ClientHandler::ProcessGetUserTable(InputPacket * pPacket)
{
	int uid 	= pPacket->ReadInt();
	int svid 	= pPacket->ReadInt();		//大厅serverid
	short level = pPacket->ReadShort();		//用户请求场等级
	int sid 	= pPacket->ReadInt();		//游戏serverid
	int tid 	= 0;
	
	short serverId 		= 0;
	string ip 			= "";
	int port 			= 0;
	short serverLevel 	= -1;

	int ret = 0;

	CGameUser user;
	int shmRet = gshm.Find(uid, user);

	if(shmRet == 0 && user.m_nTid > 0)
	{
		ret = 1;
		user.m_nUpdateTime = (int)time(NULL);
		tid = user.m_nTid;
		serverId = (tid>>16);
		CLogicServer* pServer = Options::instance()->GetServer(serverId);
		if(pServer != NULL)
		{
			ip 			= pServer->m_strIp;
			port 		= pServer->m_nPort;
			serverLevel = pServer->m_nLevel;
		}
		else
		{
			if( 1 == Options::instance()->GetServerInfoFromMc(serverId,serverLevel,ip,port))
			{
				pServer = new CLogicServer(serverId, serverLevel, ip, port);
				if(pServer != NULL)
				{
					Options::instance()->AddLogicServer(serverId, pServer);
				}
			}
		}

		gshm.SetValue(user);
	}

	
	if(ret)
	{
		//之前就在房间
		NETOutputPacket resPacket;
		resPacket.Begin(SYS_GET_USER_TABLE);
		resPacket.WriteInt(uid);
		resPacket.WriteInt(svid);
		resPacket.WriteInt(ret);
		resPacket.WriteInt(tid);
		resPacket.WriteShort(serverId);
		resPacket.WriteString(ip.c_str());
		resPacket.WriteInt(port);
		resPacket.WriteShort(serverLevel);//2013-2-26，区分3人与4人地主新增加参数
		resPacket.End();
		Send(&resPacket);
	}
	else
	{
		NETOutputPacket resPacket;
		resPacket.Begin(SYS_GET_USER_TABLE);
		resPacket.WriteInt(uid);
		resPacket.WriteInt(svid);
		resPacket.WriteInt(ret);
		resPacket.WriteShort(level);
		resPacket.WriteInt(sid);
		resPacket.End();
		Send(&resPacket);	
	}
	
	return 0;
}



int ClientHandler::ProcessSubPacket(InputPacket * pPacket)
{
	g_pErrorLog->logMsg("----------------- ClientHandler::ProcessSubPacket Begin ----------------");
	short nCmd = pPacket->GetCmdType();
	char szTemp[10240];
	int uid, svid, nLen;
	int appIp = 0;
	short api;

	g_pErrorLog->logMsg("nCmd [0x%x]",nCmd);
	
	if(nCmd == CLIENT_PACKET)
	{
		uid  = pPacket->ReadInt();
		svid = pPacket->ReadInt();				//大厅 id
		nLen = pPacket->ReadBinary(szTemp, sizeof(szTemp));

		g_pErrorLog->logMsg("uid[%d],svid[%d]",uid, svid);
	}
	else if(nCmd == CLIENT_PACKET2)
	{
		uid  = pPacket->ReadInt();
		svid = pPacket->ReadInt();				//大厅 id	
		appIp = pPacket->ReadInt();
		api = pPacket->ReadShort();
		g_pErrorLog->logMsg("uid[%d],svid[%d],appIp[%d], api[%d]",uid, svid, appIp, api);
		nLen = pPacket->ReadBinary(szTemp, sizeof(szTemp));
	}

	if(nLen < 0)
	{
		g_pErrorLog->logMsg("%s||pPacket->ReadBinary error, nLen<0",__FUNCTION__);
		return 0;
	}
	if( !CheckPacketComplete(szTemp, nLen))
	{
		g_pErrorLog->logMsg("CheckPacketComplete error");
		return 0;
	}

	NETInputPacket tempPacket;
	if(!tempPacket.Copy(szTemp, nLen))
	{
		g_pErrorLog->logMsg("%s||tempPacket.Copy error",__FUNCTION__);
		return 0;
	}	
	return ProcessClientPacket(uid, svid, &tempPacket);

}

int ClientHandler::ProcessClientPacket(int nUid, int nSvid,InputPacket *pPacket)
{
	if(pPacket->CrevasseBuffer() < 0)
	{
		g_pErrorLog->logMsg("%s||CrevasseBuffer error", __FUNCTION__);
		return 0;

	}
	int cmd = pPacket->GetCmdType();
//	DebugMsg("Uid[%d] nSvid[%d] Cmd[%x]", nUid, nSvid, cmd);
	g_pErrorLog->logMsg("Uid[%d] nSvid[%d] Cmd[%x]", nUid, nSvid, cmd);

	switch(cmd)
	{
	case CLIENT_CMD_USER_LOGIN:
		return ProcessUserLogin(nUid, nSvid,pPacket);
	case CMD_GET_ROOM_LEVER_NUM:
		return ProcessGetRoomLevelNum(nUid, nSvid, pPacket);
	default:
		g_pErrorLog->logMsg("%s||Invalid cmd:[%d]", __FUNCTION__,cmd);
	}
	return 0;	

}

int ClientHandler::SendClientPacket(int uid,int svid,NETOutputPacket * pPacket)
{
	pPacket->EncryptBuffer();
	NETOutputPacket resPacket;
	resPacket.Begin(CLIENT_PACKET);
	resPacket.WriteInt(uid);
	resPacket.WriteInt(svid);
	resPacket.WriteBinary(pPacket->packet_buf(), pPacket->packet_size());
	resPacket.End();
	Send(&resPacket);
	return 0;


}

int ClientHandler::SendServerClosePacket(int uid,int svid)
{
	NETOutputPacket resPacket;
	resPacket.Begin(SERVER_CLOSE_PACKET);
	resPacket.WriteInt(uid);
	resPacket.WriteInt(svid);
	resPacket.End();
	Send(&resPacket);

	return 0;
}


int ClientHandler::ProcessUserLogin(int nUid, int nSvid,InputPacket *pPacket)
{
	g_pErrorLog->logMsg("--------------- ClientHandler::ProcessUserLogin begin------------");
	int 	userId 			= pPacket->ReadInt();
	short 	terminalType 	= pPacket->ReadShort();

	g_pErrorLog->logMsg("%s||Login uid:[%d], svid [%d],terminalType:[%hd]", __FUNCTION__,userId, nSvid, terminalType);

	if(userId <= 0)
	{
		g_pErrorLog->logMsg("%s||Login Invalid uid:[%d], terminalType:[%hd]", __FUNCTION__,userId, terminalType);
		return 0;
	}

	CGameUser user, * pUser(NULL);
	int shmRet = gshm.Find(userId, user);
	g_pErrorLog->logMsg("shmRet [%d]",shmRet);
	g_pErrorLog->logMsg("hallid [%d]",user.m_nHallServerId);
	if(shmRet == 0)
	{		
		if(nSvid != user.m_nHallServerId && -1 != user.m_nHallServerId)
		{
			NETOutputPacket resPacket;
			resPacket.Begin(SERVER_CMD_KICK_OUT);
			resPacket.End();
			SendClientPacket(nUid, user.m_nHallServerId, &resPacket);
			SendServerClosePacket(nUid, user.m_nHallServerId);
		}

		USERSTAT preStat;
		preStat.terminalType = user.m_nTerminalType;
		preStat.tid = user.m_nTid;

		CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGINHALL, terminalType, &preStat);
		user.m_nTerminalType = terminalType;
		user.m_nHallServerId = nSvid;
		user.m_nUpdateTime = (int)time(NULL);		

		pUser = new CGameUser(user.m_nUid, user.m_nHallServerId, user.m_nTerminalType, user.m_nUpdateTime);
		pUser->m_nTid = user.m_nTid;
		pUser->m_nServerLevel = user.m_nServerLevel;		

//		gshm.SetValue(user);
	}
	else
	{
		int updateTime = (int)time(NULL);
		pUser = new CGameUser(userId, nSvid, terminalType, updateTime);
		if(updateTime - Options::instance()->m_nStartTime <= TIME_INTEVAL)
		{
			int nTid = 0;
			if( 1 == Options::instance()->GetUserInfoFromMc(userId,nTid))
			{
				pUser->m_nTid = nTid;
			}
		}
				
		CStat::Instance()->UpdateTerminalInfo(pUser->m_nUid, TERMINAL_LOGINHALL, terminalType, NULL);
	}

	Options::instance()->DeleteOfflineUser(userId);
	g_pErrorLog->logMsg("Send login success");
	NETOutputPacket resPacket;
	resPacket.Begin(SERVER_CMD_LOGIN_SUCCESS);
	int levelCount = (int)sizeof(LevelArr)/sizeof(int);
	resPacket.WriteInt(levelCount);

	g_pErrorLog->logMsg("levelCount [%d]",levelCount);

	for(int i=0; i<levelCount; i++)
	{
		//int randCount = GetRand();
		//int nCOunt = Options::instance()->GetLevelCount(LevelArr[i]);
		resPacket.WriteInt(LevelArr[i]);	
		g_pErrorLog->logMsg("LevelArr[i] [%d]",LevelArr[i]);	
	}

	int 		tid = 0;
	string 		ip = "";
	int 		port = 0;
	short 		serverLevel = -1;
	
	if( NULL != pUser && pUser->m_nTid > 0)
	{
		tid 			= pUser->m_nTid;
		short serverId 	= (pUser->m_nTid>>16);
		CLogicServer* pServer = Options::instance()->GetServer(serverId);
		if(pServer != NULL)
		{
			ip 			= pServer->m_strIp;
			port 		= pServer->m_nPort;
			serverLevel = pServer->m_nLevel;
		}
		else
		{
			if( 1 == Options::instance()->GetServerInfoFromMc(serverId,serverLevel,ip,port))
			{
				pServer = new CLogicServer(serverId, serverLevel, ip, port);
				if(pServer != NULL)
				{
					Options::instance()->AddLogicServer(serverId, pServer);
				}
			}
		}

		pUser->m_nServerLevel = serverLevel;
	}

	gshm.SetValue(*pUser);

	resPacket.WriteInt(tid);
	g_pErrorLog->logMsg("tid [%d]",tid);	
	resPacket.WriteString(ip.c_str());
	g_pErrorLog->logMsg("ip [%s]",ip.c_str());	
	resPacket.WriteInt(port);
	g_pErrorLog->logMsg("port [%d]",port);
	resPacket.End();
	SendClientPacket(nUid, nSvid, &resPacket);

	delete(pUser);
		
	return 0;
}

int ClientHandler::ProcessGetRoomLevelNum(int nUid,int nSvid,InputPacket * pPacket)
{
	short levelCount = pPacket->ReadShort();
	NETOutputPacket resPacket;
	resPacket.Begin(CMD_GET_ROOM_LEVER_NUM);
	resPacket.WriteShort(levelCount);

//	DebugMsg("levelCount[%d]", levelCount);
	
	for(int i=0; i<levelCount; i++)
	{
		int randCount = GetRand();
		short level = pPacket->ReadShort();
		resPacket.WriteShort(level);
		int userCount = Options::instance()->GetLevelCount(level);
		resPacket.WriteShort(userCount + randCount + Options::instance()->m_nAddCount);
//		DebugMsg("level[%d] Count[%d]", level, userCount);
	}
	resPacket.End();
	SendClientPacket(nUid, nSvid, &resPacket);
	return 0;
}

int ClientHandler::ProcessClientClose(InputPacket * pPacket)
{
	int nUid = pPacket->ReadInt();

	CGameUser user;
	int shmRet = gshm.Find(nUid, user);
	if(shmRet == 0)
	{
		if(user.m_nTid>0)
		{
			int nTime = (int)time(NULL);
			Options::instance()->AddOfflineUser(nUid, nTime);
			user.m_nHallServerId = -1;					//表示与大厅断开连接
			user.m_nUpdateTime = nTime;
			gshm.SetValue(user);
			return 0;
		}
		
		Options::instance()->DeleteOfflineUser(nUid);
		
		CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGOUTHALL, user.m_nTerminalType, NULL);

		gshm.Delete(nUid);
	}
	
	return 0;
}


int ClientHandler::ProcessReportUserLeaveRoom(InputPacket * pPacket)
{
	int nUid = pPacket->ReadInt();

	CGameUser user;
	int shmRet = gshm.Find(nUid, user);
	if(shmRet == 0)
	{
		if(user.m_nTid > 0)
		{
			CStat::Instance()->UpdateTerminalInfo(user.m_nUid,TERMINAL_LOGOUTTABLE,user.m_nTerminalType,NULL);
		}
		user.m_nTid = 0;
		user.m_nUpdateTime = (int)time(NULL);
		if(user.m_nServerLevel != -1)
		{
			CStat::Instance()->UpdateLevelInfo(user.m_nUid,LEVEL_LOGOUT,user.m_nServerLevel,NULL);
			user.m_nServerLevel = -1;
		}

		gshm.SetValue(user);
	}
	
	return 0;
}


int ClientHandler::ProcessReportUserEnterRoom(InputPacket * pPacket)
{
	int nUid = pPacket->ReadInt();
	int nTid = pPacket->ReadInt();

	CGameUser user;
	int shmRet = gshm.Find(nUid, user);
	if(shmRet == 0)
	{
		if(user.m_nTid == 0)
		{
			CStat::Instance()->UpdateTerminalInfo(user.m_nUid,TERMINAL_LOGINTABLE, user.m_nTerminalType,NULL);
		}

		user.m_nTid = nTid;
		user.m_nUpdateTime = (int)time(NULL);
		
		short nServerId = (nTid>>16);
		
		CLogicServer *pServer = Options::instance()->GetServer(nServerId);
		if( NULL == pServer )
		{

			pServer = Options::instance()->GetServerFromMc(nServerId);
		}

		if( NULL != pServer)
		{
			if(pServer->m_nLevel != user.m_nServerLevel)
			{				
				if(user.m_nServerLevel != -1)
				{
					CStat::Instance()->UpdateLevelInfo(user.m_nUid,LEVEL_LOGOUT,user.m_nServerLevel,NULL);
				}
				CStat::Instance()->UpdateLevelInfo(user.m_nUid,LEVEL_LOGIN,pServer->m_nLevel,NULL);
				user.m_nServerLevel = pServer->m_nLevel;
			}

		}

		gshm.SetValue(user);
	}
	return 0;	
}

int ClientHandler::ProcessReportUserStandRoom(InputPacket *pPacket)
{
	int nUid = pPacket->ReadInt();
	int nTid = pPacket->ReadInt();

	CGameUser user;
	int shmRet = gshm.Find(nUid, user);
	if(shmRet == 0)
	{
		CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_STANDTABLE, user.m_nTerminalType,NULL);
		
		short nServerId = (nTid>>16);
		
		CLogicServer *pServer = Options::instance()->GetServer(nServerId);
		if( NULL == pServer )
		{

			pServer = Options::instance()->GetServerFromMc(nServerId);
		}

		if( NULL != pServer)
		{

					CStat::Instance()->UpdateLevelInfo(user.m_nUid, LEVEL_STANDIN, user.m_nServerLevel, NULL);


		}
	}
	return 0;
}

int ClientHandler::ProcessReportStandLeaveRoom(InputPacket *pPacket)
{
	int nUid = pPacket->ReadInt();

	CGameUser user;
	int shmRet = gshm.Find(nUid, user);
	if(shmRet == 0)
	{
		CStat::Instance()->UpdateTerminalInfo(user.m_nUid,TERMINAL_STANDLEAVETABLE,user.m_nTerminalType,NULL);

		if(user.m_nServerLevel != -1)
		{
			CStat::Instance()->UpdateLevelInfo(user.m_nUid,LEVEL_STANDOUT,user.m_nServerLevel,NULL);
		}
	}
	return 0;
}

int ClientHandler::ProcessLogicCore(InputPacket * pPacket)
{
	short nServerId = pPacket->ReadShort();
	g_pDebugLog->logMsg("%s||GameServer core, serverId:[%hd]", __FUNCTION__, nServerId);
	CLogicServer* pServer = Options::instance()->GetServer(nServerId);
	if( NULL != pServer)
	{
		delete pServer;
		pServer = NULL;

	}
	Options::instance()->DeleteLogicServer(nServerId);

	SetServerUserTid0(nServerId);

	return 0;	

}

int ClientHandler::ProcessHallCore(InputPacket * pPacket)
{
	int nSvid = pPacket->ReadInt();


	SetUserOfflineHallCore(nSvid);
	return 0;
}


int ClientHandler::ProcessClearServerInfo(InputPacket * pPacket)
{
	NOT_USE(pPacket);
	g_pDebugLog->logMsg("%s||Clear server info", __FUNCTION__);
	Options::instance()->ClearServerList();
	return 0;

}

int ClientHandler::ProcessTJSwitch(InputPacket * pPacket)
{
	int nTJSwitch = pPacket->ReadInt();
	int nTJInteval = pPacket->ReadInt();
	Options::instance()->SetTjSwitch(nTJSwitch, nTJInteval);
	g_pDebugLog->logMsg("%s||flag:[%d], inteval:[%d]", __FUNCTION__,nTJSwitch, nTJInteval);
	return 0;


}

int ClientHandler::ProcessSetLevelCountRandom(InputPacket * pPacket)
{
	string key = pPacket->ReadString();
	if(strcmp(key.c_str(), SYS_CTRL_LEVELCOUNT_RANDOM) != 0)
	{
		g_pErrorLog->logMsg("%s||Strcmp [%s]--[%s]", __FUNCTION__,key.c_str(), SYS_CTRL_LEVELCOUNT_RANDOM);
		return 0;
	}
	
	Options::instance()->ReadRandomConfig("../conf/config.ini");
	g_pDebugLog->logMsg("%s||Set RandomCOunt", __FUNCTION__);
	return 0;
}

int ClientHandler::ProcessGetUserHallID(InputPacket * pPacket)
{
	int nUid = pPacket->ReadInt();
	CGameUser user;
	int shmRet = gshm.Find(nUid, user);
	NETOutputPacket pkg;
	pkg.Begin(GET_USER_HALLID);
	pkg.WriteInt(nUid);
	if(shmRet == 0) 
	{
		pkg.WriteInt(user.m_nHallServerId);
	} 
	else 
	{
		pkg.WriteInt(-1);	
	}

	pkg.End();	
	Send(&pkg);
	return 0;
}

