#ifndef OPTIONS_H
#define OPTIONS_H

#include <time.h>
#include <unistd.h>
#include <string>
#include "MemCacheClient.h"
#include "log.h"
#include "CBackServer.h"
#include <map>
#include <set>
#include <vector>
#include "GameUser.h"
#include "LogicServer.h"
#include "Timer.h"

using namespace std;

typedef map<int, CGameUser*> UserList;
typedef map<int, CLogicServer*> ServerList;

class Options
{
public:

	static Options* instance() {
		static Options options;
		return &options;
	}

	int parse_args(int argc, char *argv[]);

	int read_conf(char file[256]);

	int InitMeCached(char file[256]);

	int InitBackServer(char file[256]);

	int ReadRandomConfig(char file[256]);
	
	void printConf();

//***********************************************************
public:
	//Server ��Ϣ
	short	listen_port;			//�˿�
	int		server_id;				//sid
	
	//��־��Ϣ
	string	strLogDir;			//��ŵ�ַ
	int	   	nNum;				//��־����
	int	  	nSize;				//��־��С
	char	szLogName[256];		//��־�ļ���
	int		nLogLevel;			//��־�ȼ�

	//Mc
	CMemcacheClient  m_ServerMCClient;
	CMemcacheClient  m_OnlineMCClient;
	string strServerMcAddr;			//Mc��ַ
	string strOnlineMcAddr;

	//TJServer�����Ϣ
	string	m_strTjIp;
	int		m_nTjPort;
	int		m_nTJSwitch;
	int		m_nTJInteval;


	CBackServer	*m_pTJServer;

	//���ݽṹ
	ServerList		m_ServerList;
	map<int, int> 	m_OfflineUserMgr;		//���߱�  uid--time
	map<short, LEVELINFO> m_LevelTotalMgr;		//level ����

	int 		m_nStartTime;		//ϵͳ����ʱ��

	//ÿ�γ��μӵ�����������������������
	int			m_nRoundLimit;		//�����������
	int			m_nAddCount;		//�������

	int			m_nSHMNodeCount;
private:
	Options();
public:
	CGameUser*		GetUser(const int &nUid);
	int				AddUser(const int &nUid, CGameUser *pUser);
	int				DeleteUser(const int &nUid);
	
	CLogicServer*	GetServer(const int &nServerId);
	int				GetServerInfoFromMc(const short &nServerId, short &nLevel,string &strIp, int &Port);
	int				AddLogicServer(const int &nServerId, CLogicServer *pLogicServer);
	int				DeleteLogicServer(const int &nServerId);
	void			ClearServerList();
	int				AddOfflineUser(const int &nUid, const int &nTime);
	int 			DeleteOfflineUser(const int &nUid);
	int				GetLevelCount(const int &nLevel);
	int				SetLevelCount(const int &nLevel, const int &nPlayCount, const int &nLookCount);
	int				GetUserInfoFromMc(const int &nUid, int &nTid);

	CLogicServer*	GetServerFromMc(const short &nServerId);

	void			SetTjSwitch(const int &nTjSwitch, const int &nTjInteval);
	void 			ReportData();
	void 			InitTimer();
private:
	friend  class CTimer<Options>;

	CTimer<Options>	 m_CheckTimer;
	CTimer<Options>	 m_CheckOfflineTimer;
	CTimer<Options>	 m_CheckFlushTimer;

	int ProcessOnTimerOut(int nId);
	int ProcessTimeCheck();
	int ProcessOfflineCheck();
	int ProcessFlushCheck();
};


#endif

