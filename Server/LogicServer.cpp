#include "LogicServer.h"

CLogicServer::CLogicServer(short serverId, short level, string& ip, int port)
{
	m_nServerId = serverId;
	m_nLevel = level;
	m_strIp = ip;
	m_nPort = port;
}

CLogicServer::~CLogicServer()
{
}

