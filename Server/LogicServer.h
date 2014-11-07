#ifndef __LOGIC_SERVER_H
#define __LOGIC_SERVER_H

#include <string>
using std::string;

class SocketHandler;

class CLogicServer
{
public:
	CLogicServer(short serverId, short level, string& ip, int port);
	~CLogicServer(void);
public:
	short m_nServerId;
	short m_nLevel;
	string m_strIp;
	int m_nPort;
};
#endif
