#ifndef  _ICC_TCP_SERVER_H_
#define _ICC_TCP_SERVER_H_
#include <string.h>
#include "poller.h"
#include "net.h"
#include "ICC_TCP_Handler.h"

class ICC_TCP_Server : public CPollerObject 
{
	public:
		ICC_TCP_Server( ){};
		virtual ~ICC_TCP_Server( ){};

	protected:
		virtual ICC_TCP_Handler* CreateHandler(int netfd, struct sockaddr_in* peer) = 0;

};



#endif

