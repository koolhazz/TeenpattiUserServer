#ifndef CLOGSERVER_H_
#define CLOGSERVER_H_


#include "ICC_Timer_Handler.h" 
#include "CCSocketHandler.h" 
#include "BoyaaDecoder.h"
#include "Packet.h"
#include "GameCmd.h"
#include "Stat.h"
#include "Data_PacketBase.h"




class CBackServer : public CCSocketHandler, public CCTimer
{
public:
	CBackServer(string strFlag);
	virtual ~CBackServer();

	virtual int Init();
	int OnConnected();
    int OnClose();
    int OnPacketComplete(const char* data, int len);
	ICC_Decoder* CreateDecoder()
    {
		return BoyaaDecoder1::getInstance();
    }
	virtual int ProcessOnTimerOut();

	int reConnect();

	int Send(DataNETOutputPacket  *pPakcet);
	DataNETInputPacket  pPacket;
private:
	string strServer_flag;
	bool   m_bIsConnected;

	int ProcessPacket(DataNETInputPacket  *pPacket);


public:
	int 	InitConnect(const char *ip, int port);
	void 	SynData(map<short, LEVELINFO>& levelMgr, map<short, DATAINFO>& terminalTypeMgr);
private:
	void	ProcessSynData(DataNETInputPacket  *pPacket);
};

#endif

