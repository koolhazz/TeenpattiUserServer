#include <sys/stat.h> 
#include <signal.h> 
#include "IniFile.h"
#include "Options.h"
#include "GameCmd.h"
#include "GameUserSHM.h"

using namespace comm::commu;

#ifndef WIN32
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <getopt.h>
#endif


#include "clib_log.h"
extern clib_log *g_pDebugLog;
extern clib_log *g_pErrorLog;

extern GameUserSHM gshm;

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



void daemonize()
{
#ifndef WIN32
	pid_t pid, sid;

	/* already a daemon */
	if ( getppid() == 1 ) return;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* At this point we are executing as the child process */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Redirect standard files to /dev/null */
	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	freopen( "/dev/null", "w", stderr);
#endif 
}  

Options::Options()
{
	listen_port = 0;
	server_id	= 0;
	strServerMcAddr = "";
	strOnlineMcAddr = "";
	m_strTjIp = "";
	m_nTjPort = 0;
	m_nTJInteval = 60;
	m_nTJSwitch = 0;
	m_pTJServer = NULL;

	m_ServerList.clear();
	m_OfflineUserMgr.clear();
	m_LevelTotalMgr.clear();

	m_nRoundLimit = 100;
	m_nAddCount   = 0;

	m_nSHMNodeCount = 0;

}

const char* const short_options = "p:s:l:u:m:f:vdh";  
struct option long_options[] = {  
     { "port",		1,   NULL,    'p'     },  
     { "sid",		1,   NULL,    's'     }, 
     {"shm",		1,   NULL,    'm'    },
     { "daemon",	0,   NULL,    'd'     },
	 { "version",   0,   NULL,    'v'     },
     { "help",		0,   NULL,    'h'     },  
     {      0,     0,     0,     0        },  
}; 

void printHelp()
{
        printf("-%s %-10s %-15s %s\n",                "p","--port","<port>","tcp port number to listen on");
        printf("-%s %-10s %-15s %s\n",                "s","--sid","<sid>","server id ");
      	 printf("-%s %-10s %-15s %s\n",                "m","--shm","","shm node count");
        printf("-%s %-10s %-15s %s\n",                "d","--daemonize","","run as a daemon");
  	 printf("-%s %-10s %-15s %s\n",                "v","--version","","print version info");
        printf("-%s %-10s %-15s %s\n",                "h","--help","","print this help and exit");
        printf("\n");
}

int Options::parse_args(int argc, char *argv[]) 
{
	if(argc==1)
	{
		 printHelp();
		 return -1;
	}

	int opt;
	while ((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1)  
	{
		switch (opt) 
		{      
		case 'p':
			listen_port = atoi(optarg);
			break;
		case 's':
			server_id = atoi(optarg);
			break;
		case 'd': 
			{
				signal(SIGINT,  SIG_IGN);
				signal(SIGTERM, SIG_IGN);
				daemonize();
			}
			break;
		case 'm':
			{
				m_nSHMNodeCount = atoi(optarg);
				if(m_nSHMNodeCount <= 0) {
					fprintf(stderr, "%s\n", "err: Bad share memory node count!");
					m_nSHMNodeCount = 0;
					return -1;
				}
			}
			break;
		case 'h':
			printHelp();
			return -1; 
		default:
			printHelp();
			printf("Parse error.\n");
			return -1;
		}
	}
   
   if(listen_port==0)
	{
	    printf("Please Input server_port: -p port\n");
		printHelp();
		return -1;
   }

	if(server_id < 0)
	{
		printf("Please Input Server ID: -s id\n");
        printHelp();
        return -1;
	}	
	return 0;
}

int Options::read_conf(char file[256])
{
	IniFile iniFile(file);
	if(!iniFile.IsOpen())
	{ 
		printf("Open IniFile Error:[%s]\n",file);
		return -1;
	}
	strLogDir		= iniFile.ReadString("LOG", "LOGDIR", "./");
	nNum			= iniFile.ReadInt("LOG", "LOGNUM", 10);
	nSize			= iniFile.ReadInt("LOG", "LOGSIZE", 10 * 1024 * 1024);
	nLogLevel   	= iniFile.ReadInt("LOG", "LOGLEVEL", 0);
	snprintf(szLogName, sizeof(szLogName), "CCReactor.log.%d", getpid());
	m_nStartTime = time(NULL);			//启动时间

	return 0;
	
}

int Options::ReadRandomConfig(char file [ 256 ])
{
	IniFile iniFile(file);
	if(!iniFile.IsOpen())
	{ 
		printf("Open IniFile Error:[%s]\n",file);
		return -1;
	}
	m_nRoundLimit	= iniFile.ReadInt("LEVELCOUNT", "RANDCOUNT", 100);
	m_nAddCount		= iniFile.ReadInt("LEVELCOUNT", "ADDCOUNT", 0);
	log_boot("m_nRoundLimit[%d] m_nAddCount[%d]", m_nRoundLimit, m_nAddCount);
	return 0;
}


int Options::InitMeCached(char file[256])
{
	IniFile iniFile(file);
	if(!iniFile.IsOpen())
	{ 
		printf("Open IniFile Error:[%s]\n",file);
		return -1;
	}
	strServerMcAddr = iniFile.ReadString("MEMCACHED","SERVER_MC","0.0.0.0:0");
	string strServerMc = strServerMcAddr;
	if( !m_ServerMCClient.Init(strServerMc.c_str()))
	{
		log_error("Init serverMc[%s] Fail", strServerMcAddr.c_str());
		return -1;	
	}
	log_boot("Init Mc[%s] Success", strServerMcAddr.c_str());

	strOnlineMcAddr = iniFile.ReadString("MEMCACHED","ONLINE_MC","0.0.0.0:0");
	string strOnlineMc = strOnlineMcAddr;
	if( !m_OnlineMCClient.Init(strOnlineMc.c_str()))
	{
		log_error("Init onlineMc[%s] Fail", strOnlineMcAddr.c_str());
		return -1;	
	}
	log_boot("Init Mc[%s] Success", strOnlineMcAddr.c_str());
	return 0;
}

int Options::InitBackServer(char file [256])
{
	IniFile iniFile(file);
	if(!iniFile.IsOpen())
	{ 
		printf("Open IniFile Error:[%s]\n",file);
		return -1;
	}
	m_strTjIp = iniFile.ReadString("TJSvr", "HOST","0.0.0.0");
	m_nTjPort = iniFile.ReadInt("TJSvr", "PORT",0);
	m_nTJSwitch = iniFile.ReadInt("TJSvr", "SWITCH",0);
	m_nTJSwitch = iniFile.ReadInt("TJSvr", "SWITCH",0);

	m_pTJServer = new CBackServer("TJServer");
	if( NULL == m_pTJServer)
	{
		log_error("new CBackServe error");
		return -1;
	}
	m_pTJServer->InitConnect(m_strTjIp.c_str(), m_nTjPort);
	log_boot("UserServer[%s:%d]...",m_strTjIp.c_str(), m_nTjPort);
	
	return 0;	
}


void Options::printConf()
{
	printf("==============SysConfig=================\n");
	printf("listen_port   =[%u]\n",listen_port);
	printf("server_id     =[%u]\n",server_id);
	printf("Log Dir	      =[%s]\n",strLogDir.c_str());
	printf("Log Num       =[%d]\n",nNum);
	printf("Log Size      =[%d]\n",nSize);
	printf("Log Level     =[%d]\n",nLogLevel);
	printf("Log Name      =[%s]\n",szLogName);
	printf("=====================================\n");
}

CLogicServer* Options::GetServer(const int & nServerId)
{
	CLogicServer *pServer = NULL;
	ServerList::iterator iterServer = m_ServerList.find(nServerId);
	if(iterServer != m_ServerList.end())
	{
		pServer = iterServer->second;
	}
	return pServer;	
}

int Options::AddLogicServer(const int & nServerId,CLogicServer * pLogicServer)
{
	if( NULL != pLogicServer)
	{
		m_ServerList[nServerId] = pLogicServer;
	}
	return 0;
}

int Options::DeleteLogicServer(const int & nServerId)
{
	ServerList::iterator iterServer = m_ServerList.find(nServerId);
	if(iterServer != m_ServerList.end())
	{
		m_ServerList.erase(iterServer);
	}
	return 0;
}

void Options::ClearServerList()
{
	m_ServerList.clear();	
}

int Options::GetServerInfoFromMc(const short & nServerId,short & nLevel,string & strIp,int & nPort)
{
	char szKey[20]={0};
	snprintf(szKey, sizeof(szKey), "%d", nServerId);
	string value = "";
	m_ServerMCClient.GetRecord(szKey, value);
	if(value != "")
	{
		vector<string> vec;
		split_str(value.c_str(), ",", vec);
		if((int)vec.size() == SERVER_ATTR_COUNT)//读取数据不是level,ip,port
		{
			nLevel 	= (short)atoi(vec[0].c_str());
			strIp 	= vec[1];
			nPort 	= atoi(vec[2].c_str());
			return 1;
		}
		return 0;
	}
	return 0;
}

CLogicServer* Options::GetServerFromMc(const short &nServerId)
{
	CLogicServer* pServer = NULL;
	short 	nLevel 	= 0;
	string 	ip 		= "";
	int 	nPort 	= 0;
	if( 1 != GetServerInfoFromMc(nServerId, nLevel, ip, nPort))
	{
		return pServer;
	}
	pServer = new CLogicServer(nServerId, nLevel, ip, nPort);
	if( NULL != pServer)
	{
		AddLogicServer(nServerId, pServer);
	}
	return pServer;
}

int Options::GetUserInfoFromMc(const int & nUid,int & nTid)
{
	char szOnlineKey[20]={0};
	snprintf(szOnlineKey, sizeof(szOnlineKey), "%d", nUid);
	string onlineValue = "";
	m_OnlineMCClient.GetRecord(szOnlineKey, onlineValue);
	if(onlineValue != "")
	{
		vector<string> v;
		split_str(onlineValue.c_str(), ",", v);
		if((int)v.size() != ONLINE_ATTR_COUNT)//读取数据不是tid,serverId,level
		{
			g_pErrorLog->logMsg("%s||v.size()!=ONLINE_ATTR_COUNT", __FUNCTION__);
		}
		else
		{
			nTid = atoi(v[0].c_str());
			return 1;
		}
	}
	else
	{
		g_pErrorLog->logMsg("%s||mc get value=NULL", __FUNCTION__);
	}
	return 0;
}



int Options::AddOfflineUser(const int & nUid,const int & nTime)
{
	m_OfflineUserMgr[nUid] = nTime;
	return 0;
}

int Options::DeleteOfflineUser(const int & nUid)
{
	map<int, int>::iterator iteroffline = m_OfflineUserMgr.find(nUid);
	if(iteroffline != m_OfflineUserMgr.end())
	{
		m_OfflineUserMgr.erase(iteroffline);
	}
	return 0;
}

int Options::SetLevelCount(const int &nLevel, const int &nPlayCount, const int &nLookCount)
{
	LEVELINFO info;
	info.playCount = nPlayCount;
	info.lookCount = nLookCount;
	m_LevelTotalMgr[nLevel] = info;
	return 0;
}

int Options::GetLevelCount(const int & nLevel)
{
	map<short, LEVELINFO>::iterator iter = m_LevelTotalMgr.find(nLevel);
	if(iter != m_LevelTotalMgr.end())
	{
		return (iter->second).playCount;
	}
	return 0;
}

void Options::SetTjSwitch(const int & nTjSwitch,const int & nTjInteval)
{
	m_nTJSwitch  = nTjSwitch;
	m_nTJInteval = nTjInteval;
}

void Options::InitTimer()
{
	m_CheckTimer.SetTimeEventObj(instance(), TIME_CHECK);
	m_CheckTimer.StartTimer(m_nTJInteval);


	m_CheckFlushTimer.SetTimeEventObj(instance(), FLUSH_CHECK);
	m_CheckFlushTimer.StartTimer(FLUSH_CHECK_TIME);


	m_CheckOfflineTimer.SetTimeEventObj(instance(), OFFLINE_CHECK);
	m_CheckOfflineTimer.StartTimer(TIME_INTEVAL);	

}

int Options::ProcessOnTimerOut(int nId)
{
	switch(nId)
	{
	case TIME_CHECK:
//		DebugMsg("TIME_CHECK");
		return ProcessTimeCheck();
	case FLUSH_CHECK:
//		DebugMsg("FLUSH_CHECK");
		return ProcessFlushCheck();
	case OFFLINE_CHECK:
//		DebugMsg("OFFLINE_CHECK");
		return ProcessOfflineCheck();
	default:
		g_pErrorLog->logMsg("%s||Invalid TimerId[%d]", __FUNCTION__, nId);
	}

	return 0;
}

int Options::ProcessTimeCheck()
{
	m_CheckTimer.StopTimer();
	ReportData();
	m_CheckTimer.StartTimer(m_nTJInteval);
	return 0;
}

void Options::ReportData()
{
	if(m_pTJServer != NULL)
	{
		m_pTJServer->SynData(CStat::Instance()->m_LevelMgr, CStat::Instance()->m_terminalStatMap);
	}	
}

int Options::ProcessOfflineCheck()
{
	m_CheckOfflineTimer.StopTimer();
	int now = (int)time(NULL);
	
	map<int, int>::iterator iter = m_OfflineUserMgr.begin();
	for(; iter!=m_OfflineUserMgr.end(); )
	{
		int& offlineTime = iter->second;
		if((now-offlineTime) >= 5*60)
		{
			CGameUser user;
			if(gshm.Find(iter->first, user) == 0)
			{
				CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGOUTHALL, user.m_nTerminalType, NULL);
				//如果游戏server core了，无法上报用户退出房间，如果用户tid>0,这里做下修正处理				
				if(user.m_nTid > 0)
				{
					CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGOUTTABLE, user.m_nTerminalType, NULL);
					user.m_nTid = 0;
				}		

				//如果游戏server core了，无法上报用户退出房间，如果用户serverLevel!=-1,这里做下修正处理
				if(user.m_nServerLevel != -1)
				{
					CStat::Instance()->UpdateLevelInfo(user.m_nUid, LEVEL_LOGOUT, user.m_nServerLevel, NULL);
					user.m_nServerLevel = -1;
				}

				gshm.Delete(user.m_nUid);
			}
						
			m_OfflineUserMgr.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	
	m_CheckOfflineTimer.StartTimer(TIME_INTEVAL);
	return 0;
}

int Options::ProcessFlushCheck()
{
	m_CheckFlushTimer.StopTimer();
	int optTimes = 0;
	
	struct timeval tv1;
	gettimeofday(&tv1, NULL);	

	int now = (int)time(NULL);
	std::list<int> needDelUser;

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

		int timePass = now - user.m_nUpdateTime;
		if(user.m_nTid == 0)
		{
			if(timePass >= 1*60*60)
			{
				CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGOUTHALL, user.m_nTerminalType, NULL);
				
				if(user.m_nServerLevel != -1)
				{
					CStat::Instance()->UpdateLevelInfo(user.m_nUid, LEVEL_LOGOUT, user.m_nServerLevel, NULL);
				}
				
				map<int, int>::iterator itOffline = m_OfflineUserMgr.find(user.m_nUid);
				if(itOffline != m_OfflineUserMgr.end())
				{
					m_OfflineUserMgr.erase(itOffline);
				}	

				needDelUser.push_back(user.m_nUid);
				++ optTimes;
			} 
		}
		else 
		{
			if(timePass >= 2*60*60) 		
			{
				CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGOUTHALL, user.m_nTerminalType, NULL);
									
				CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGOUTTABLE, user.m_nTerminalType, NULL);
				user.m_nTid = 0;
				
				if(user.m_nServerLevel != -1)
				{
					CStat::Instance()->UpdateLevelInfo(user.m_nUid, LEVEL_LOGOUT, user.m_nServerLevel, NULL);
				}
				
				map<int, int>::iterator itOffline = m_OfflineUserMgr.find(user.m_nUid);
				if(itOffline != m_OfflineUserMgr.end())
				{				
					m_OfflineUserMgr.erase(itOffline);
				}	
				
				needDelUser.push_back(user.m_nUid);
				++ optTimes;
			}
		}

		pNode = v.next(pNode);
	}

	std::list<int>::iterator it = needDelUser.begin();
	while(it != needDelUser.end()) {
		gshm.Delete(*it++);
	}
	struct timeval tv2;
	gettimeofday(&tv2, NULL);	

	g_pDebugLog->logMsg("ProcessFlushCheck time cost calc begin stamp:[%d.%d], end stamp:[%d.%d] ", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec);	

	g_pDebugLog->logMsg("ProcessFlushCheck, user opt times:[%d]", optTimes);

	m_CheckFlushTimer.StartTimer(FLUSH_CHECK_TIME);	

	return 0;	
}


