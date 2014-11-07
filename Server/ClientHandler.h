#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H


#include "CCSocketServer.h"
#include "CCSocketHandler.h"
#include "CCStreamDecoder.h"
#include "Packet.h"
#include "BoyaaDecoder.h"
#include <string>
#include <vector>


/*
#pragma pack(1)
struct TPkgHeader
{	
	 short length;
	 char  flag[2];	 	
	 char  cVersion;	
	 char  cSubVersion;
	 short cmd;
	 unsigned char  code;
};
#pragma pack()*/

using namespace std;

class ClientHandler: public CCSocketHandler
{

public:
	ClientHandler();
	virtual ~ClientHandler();

	string m_addrRemote;
	int    m_nPort;

	int Send(OutputPacket * packet, bool encode = false)
	{
		if( encode)
			packet->EncryptBuffer();

		return CCSocketHandler::Send(packet->packet_buf(), packet->packet_size())>0 ?0: -1;
	}

	int OnConnected(void); 
	int OnClose();

private:
	int OnPacketComplete(const char * data,int len);
	int ProcessPacket(InputPacket *pPacket);
	ICC_Decoder* CreateDecoder()
	{
		return BoyaaDecoder::getInstance();
	}

	void GetRemoteAddr(void);
	InputPacket	pPacket;

public:


private:
	//业务逻辑方面
	int ProcessAllocRegister(InputPacket *pPacket);
	int ProcessGetUserTable(InputPacket *pPacket);
	int ProcessSubPacket(InputPacket *pPacket);
	int ProcessClientPacket(int nUid, int nSvid,InputPacket *pPacket);
	int ProcessUserLogin(int nUid, int nSvid,InputPacket *pPacket);
	int ProcessGetRoomLevelNum(int nUid, int nSvid,InputPacket *pPacket);

	int ProcessClientClose(InputPacket *pPacket);
	int ProcessReportUserLeaveRoom(InputPacket *pPacket);
	int ProcessReportUserEnterRoom(InputPacket *pPacket);
	int ProcessReportUserStandRoom(InputPacket *pPacket);
	int ProcessReportStandLeaveRoom(InputPacket *pPacket);
	int ProcessLogicCore(InputPacket *pPacket);
	int ProcessHallCore(InputPacket *pPacket);
	int ProcessClearServerInfo(InputPacket *pPacket);
	int ProcessTJSwitch(InputPacket *pPacket);
	int ProcessSetLevelCountRandom(InputPacket *pPacket);
	int ProcessGetUserHallID(InputPacket *pPacket);

	int SendClientPacket(int uid, int svid, NETOutputPacket *pPacket);
	int SendServerClosePacket(int uid, int svid);

	void SetServerUserTid0(short sid);
	void SetUserOfflineHallCore(int hid);

public:
	
	
	
};



#endif


