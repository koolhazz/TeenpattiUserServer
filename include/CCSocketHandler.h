#ifndef  _CCSocketHandler_H_
#define _CCSocketHandler_H_
#include "cache.h"
#include "ICC_Decoder.h"
#include "ICC_TCP_Handler.h"
using namespace comm::sockcommu;

#define MAX_WEB_RECV_LEN            102400

enum CConnState
{
    CONN_IDLE,
    CONN_FATAL_ERROR,
    CONN_DATA_ERROR,
    CONN_CONNECTING,
    CONN_DISCONNECT,
    CONN_CONNECTED,
    CONN_DATA_SENDING,
    CONN_DATA_RECVING,
    CONN_SEND_DONE,
    CONN_RECV_DONE,
    CONN_APPEND_SENDING,
    CONN_APPEND_DONE,
	CONN_XML_POLICY
};

class CCSocketHandler  : public ICC_TCP_Handler  
{
	public:
		CCSocketHandler();
		virtual ~CCSocketHandler();
 
	public:
		virtual int Init();
		virtual int InputNotify();
		virtual int OutputNotify ();
		virtual int HangupNotify ();
		int Send(const char * buff, int len);
	
	protected:		
		virtual int OnClose() ;
		virtual int OnConnected() ;
		virtual int OnPacketComplete(const char * data, int len);
		virtual ICC_Decoder*  CreateDecoder()=0;
		void Reset();
	public:
		int Connect();
	protected:
		ICC_Decoder* _decode;
		CConnState	_stage;

	private:
		int handle_input();
		int handle_output();

		CRawCache       _r;
		CRawCache       _w;

};

#endif

