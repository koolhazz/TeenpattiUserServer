#include <stdio.h>
#include "ICC_Timer_Handler.h"
#include "CCStreamDecoder.h"
#include "CCSocketHandler.h"
#include "CCSocketServer.h"
#include "CCReactor.h"
#include "ClientHandler.h"
#include "Packet.h"
#include <signal.h>
#include "Options.h"
#include "GameUserSHM.h"


using namespace comm::commu;

extern GameUserSHM gshm;

#include "clib_log.h"

clib_log	*g_pDebugLog = NULL;
clib_log	*g_pErrorLog = NULL;

#ifndef __KEY_BASE
#define __KEY_BASE 0x20111
#endif


void registerSignal()
{
	signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
} 

int readSysCfg(int argc, char *argv[])
{
    if (Options::instance()->parse_args(argc, argv) != 0)
        return -1;
	if (Options::instance()->read_conf("../conf/config.ini") != 0)
        return -2;
	
	Options::instance()->printConf();
	init_log(Options::instance()->szLogName, Options::instance()->strLogDir.c_str(), Options::instance()->nNum, Options::instance()->nSize);
	set_log_level(Options::instance()->nLogLevel);

	Options::instance()->ReadRandomConfig("../conf/config.ini");
		
    return 0;
}

int InitBackServer()
{
	if( Options::instance()->InitMeCached("../conf/config.ini") != 0)
		return -1;
	if( Options::instance()->InitBackServer("../conf/config.ini") != 0)
		return -2;
	return 0;
}



void InitLog(const char *szPrefixName)
{
	char szDebugLogFile[256] = {0};
	snprintf(szDebugLogFile, sizeof(szDebugLogFile), "%s.debug.%u", szPrefixName, getpid());
	g_pDebugLog = new clib_log();
    g_pDebugLog->set_file(szDebugLogFile);
    g_pDebugLog->set_level(5);
    g_pDebugLog->set_maxfile(5);
    g_pDebugLog->set_maxsize(10240000);
    g_pDebugLog->set_timeformat(CLIB_LOG_TFORMAT_0);

	char szErrorLogFile[256] = {0};
	snprintf(szErrorLogFile, sizeof(szErrorLogFile), "%s.error.%u", szPrefixName, getpid());
	g_pErrorLog = new clib_log();
    g_pErrorLog->set_file(szErrorLogFile);
    g_pErrorLog->set_level(5);
    g_pErrorLog->set_maxfile(5);
    g_pErrorLog->set_maxsize(10240000);
    g_pErrorLog->set_timeformat(CLIB_LOG_TFORMAT_0);
}


void ReportStartTJData()
{
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
		
		CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGINHALL, user.m_nTerminalType, NULL);
		
		if(user.m_nTid > 0)
		{
			CStat::Instance()->UpdateTerminalInfo(user.m_nUid, TERMINAL_LOGINTABLE, user.m_nTerminalType, NULL);
			CStat::Instance()->UpdateLevelInfo(user.m_nUid,LEVEL_LOGIN, user.m_nServerLevel, NULL);		
		}
		
		pNode = v.next(pNode);
	}

	Options::instance()->ReportData();
}


int InitSHM()
{
	THashPara hashPara;
	hashPara.bucket_size_ = 65537;									//hash Ͱ������ֵ65537Ч�ʱȽϸ�
	hashPara.node_total_ = Options::instance()->m_nSHMNodeCount; 		//�ڵ��������ٸ�����
	hashPara.chunk_total_ = Options::instance()->m_nSHMNodeCount; 		//CHUNK��Ƭ�����������ǵ�Ӧ�ã�ÿ�����ݴ�С��ͬ��������Ϊ�ͽڵ�����ͬ
	hashPara.chunk_size_ = sizeof(CGameUser);							//CHUNK��Ƭ��С	

	int shm_key = __KEY_BASE + Options::instance()->server_id;				//�����ڴ�key
	int sem_key = __KEY_BASE + Options::instance()->server_id;				//�ź���key

	gshm.SetHashInfo(hashPara, shm_key, sem_key);

	if(gshm.Init() != 0)
	{
		return -1;
	}

	ReportStartTJData();

	return 0;
}

int main(int argc, char *argv[])
{
	CCReactor::Instance()->Init();

	srand((unsigned int)time(0));
	registerSignal();

	if(readSysCfg(argc,argv) < 0)
	{
		return -1;
	}
	
	InitLog(argv[0]);

	
/**
*	������������SocketServerģ���࣬����һ������ļ���server����
*   ��ģ�����Ϊ���ǵڶ���������ɵ�SocketHandler��
**/
	SocketServer<ClientHandler> example_server("0.0.0.0", Options::instance()->listen_port);
	
/**
*	���Ĳ��� ע��example_server����Ӧ��CCReactorʵ����
**/
	if(CCReactor::Instance()->RegistServer(&example_server) == -1)
	{
		log_error("RegistServer fail");
		return -1;
	}
	if( InitBackServer() < 0)
	{
		log_error("InitBackServer fail");
		return -1;

	}

	if(InitSHM() != 0)
	{
		log_error("InitSHM fail.");
		return -1;
	}

	Options::instance()->InitTimer();
	
	log_boot("Server have been started, listen port:%d...",Options::instance()->listen_port);
	
/**
*	���岽�� ������Ӧ���¼�ѭ��
**/
	CCReactor::Instance()->RunEventLoop();
	
	return 0;
}
