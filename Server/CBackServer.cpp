#include "CBackServer.h"
#include "CCReactor.h"
#include "ClientHandler.h"
#include "GameCmd.h"
#include "Options.h"


#include "clib_log.h"
extern clib_log *g_pDebugLog;
extern clib_log *g_pErrorLog;


CBackServer::CBackServer(string strFlag)
{
	EnableReconnect();

	strServer_flag = strFlag;
	m_bIsConnected = false;
}

CBackServer::~CBackServer()
{
	

}

int CBackServer::Init()
{
	CCSocketHandler::Init();

	return 0;
}

int CBackServer::OnConnected()
{
	if( m_bIsConnected == false)
	{
		m_bIsConnected = true;
		log_boot("Connect %s[%s:%d] Success Fd[%d]", strServer_flag.c_str(),GetSIP().c_str(), GetPort(),netfd);
	}
	Options::instance()->ReportData();			//连接上就得发
	StopTimer();
	return 0;
}

int CBackServer::OnClose()
{
	m_bIsConnected = false;
	log_boot("%s[%s:%d] Closed", strServer_flag.c_str(),GetSIP().c_str(), GetPort());
	StartTimer(5000,true);
	return 0;
	
}

int CBackServer::OnPacketComplete(const char * data,int len)
{
	//DebugMsg("%s len[%d]",data, len);
	pPacket.Copy(data,len);
	//*****处理包****/
	return ProcessPacket(&pPacket);
}

int CBackServer::ProcessPacket(DataNETInputPacket * pPacket)
{
	short cmd = pPacket->GetCmdType();
	switch(cmd)
	{
	case SYS_SYN_DATA:
		ProcessSynData(pPacket);
		break;
	default:
		break;
	}
	return 0;	
}

int CBackServer::ProcessOnTimerOut()
{
	log_boot("TimeOut Reconncet %s", strServer_flag.c_str());
	reConnect();
	return 0;
}

int CBackServer::reConnect()
{
	log_boot("try connect %s[%s:%d]", strServer_flag.c_str(),GetSIP().c_str(), GetPort());
	Connect();
	return 0;	
}

int CBackServer::InitConnect(const char * ip,int port)
{
	SetSIP(ip);
	SetPort(port);
	if( Connect()< 0)
	{
		return -1;
	}
	return 0;		
}


int CBackServer::Send(DataNETOutputPacket* pPacket)
{
	return CCSocketHandler::Send(pPacket->packet_buf(), pPacket->packet_size());
}

void CBackServer::SynData(map<short, LEVELINFO>& levelMgr, map<short, DATAINFO>& terminalTypeMgr)
{
	g_pErrorLog->logMsg("---------------  CBackServer::SynData begin  -------------");
	if( false == m_bIsConnected)
	{
		g_pErrorLog->logMsg("---------------  CBackServer::SynData end 1  -------------");
		return ;
	}

	DataNETOutputPacket  reqPacket;
	reqPacket.Begin(SYS_SYN_DATA);
	reqPacket.WriteInt(Options::instance()->server_id);
	int levelCount = (int)levelMgr.size();
	reqPacket.WriteInt(levelCount);
	g_pDebugLog->logMsg("ServerId [%d],levelCount[%d]",Options::instance()->server_id,levelCount);
	map<short, LEVELINFO>::iterator iterLevel = levelMgr.begin();
	for(; iterLevel!=levelMgr.end(); iterLevel++)
	{
		LEVELINFO& info = iterLevel->second;
		reqPacket.WriteShort(iterLevel->first);
		reqPacket.WriteInt(info.playCount);
		reqPacket.WriteInt(info.lookCount);
		g_pDebugLog->logMsg("iterLevel->first [%d],levelCount[%d]",iterLevel->first,iterLevel->second);
	}
	int typeCount = (int)terminalTypeMgr.size();
	reqPacket.WriteInt(typeCount);
	g_pDebugLog->logMsg("itypeCount [%d]",typeCount);
	map<short, DATAINFO>::iterator iterType = terminalTypeMgr.begin();
	for(; iterType!=terminalTypeMgr.end(); iterType++)
	{
		DATAINFO& info = iterType->second;
		reqPacket.WriteShort(iterType->first);
		reqPacket.WriteInt(info.onlineCount);
		reqPacket.WriteInt(info.onPlayCount);
		reqPacket.WriteInt(info.onLookOnCount);
		g_pDebugLog->logMsg("iterType->first[%d],info.onlineCount[%d],info.onPlayCount[%d]",iterType->first,info.onlineCount,info.onPlayCount);
	}
	reqPacket.End();
	Send(&reqPacket);

	g_pErrorLog->logMsg("---------------  CBackServer::SynData end -------------");
}


void CBackServer::ProcessSynData(DataNETInputPacket * pPacket)
{
	g_pErrorLog->logMsg("---------------  CBackServer::ProcessSynData begin -------------");
	int size = pPacket->ReadInt();
	for(int i=0; i<size; i++)
	{
		short level = pPacket->ReadShort();
		int playcount = pPacket->ReadInt();
		int lookcount = pPacket->ReadInt();
		g_pDebugLog->logMsg("level [%d], count [%d]",level,lookcount);
		Options::instance()->SetLevelCount(level, playcount,lookcount);
	}	

	g_pErrorLog->logMsg("---------------  CBackServer::ProcessSynData end -------------");
}
