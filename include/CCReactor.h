#ifndef  _CCREACTOR_H_
#define  _CCREACTOR_H_

#include "timerlist.h"
#include "poller.h"
#include "Singleton.h"
#include "ICC_TCP_Server.h"
#include "ICC_TCP_Handler.h"
#define MAX_POLLER 102400
class ICC_Timer_Handler;
 
class CCReactor
{
	public:
		static CCReactor* Instance (void);
		static void Destroy (void);
		int SetLog(const char* logfile, int level);
		int Init(int maxpoller=MAX_POLLER);
		int RunEventLoop(); 
		int RegistServer(ICC_TCP_Server* server) ;		  
		int RegistClient(ICC_TCP_Handler* client);		
		int Connect(ICC_TCP_Handler* handler, const char* host, short port, int timeout=0);	
		int RegistTimer(ICC_Timer_Handler* timer, long t);		 
 	    int AttachPoller (CPollerObject* poller);  
	private:

		CCReactor();
		virtual ~CCReactor() ;

		CPollerUnit*    _pollerunit;
		CTimerUnit*    _timerunit;
		int _maxpoller;
		friend class  CreateUsingNew<CCReactor>;

};
#endif

