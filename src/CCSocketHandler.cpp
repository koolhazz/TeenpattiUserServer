#include <assert.h>
#include "memcheck.h"
#include "CCReactor.h"
#include "CCSocketHandler.h"

extern CMemPool*	_memPool;
CCSocketHandler::CCSocketHandler():
	_stage(CONN_IDLE),
	_r(*_memPool),
	_w(*_memPool)
{
	
}

CCSocketHandler::~CCSocketHandler()
{
	_w.skip( _w.data_len() );
	_r.skip( _r.data_len() );
	if(_decode)
		DELETE(_decode);
}
 
int CCSocketHandler::Init()
{
	if( _decode == NULL){
		this->_decode = this->CreateDecoder();
	}
	assert(this->_decode);
	CPollerObject::EnableInput ();
	return 0;
}

int CCSocketHandler::InputNotify(void)
{
	int stage = handle_input();
	switch (stage)
	{
		case CONN_DATA_ERROR:
			log_warning("input decode error, netfd[%d], stage[%d]", netfd, stage);
			this->OnClose();
			if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
				delete this;
			}
			else{
				Reset();
			}
			return POLLER_COMPLETE;

		case CONN_DISCONNECT:
			log_notice("input disconnect by user, netfd[%d], stage[%d]", netfd, stage);
			this->OnClose();
			if(this->GetReconnectFlag()== false)
			{
				delete this;
			}
			else{
				Reset();
			}
			return POLLER_COMPLETE;

		case CONN_DATA_RECVING:
		case CONN_IDLE:
		case CONN_RECV_DONE:			
			return POLLER_SUCC;

		case CONN_FATAL_ERROR:
			log_error("input fatal error, netfd[%d], stage[%d]", netfd, stage);
			this->OnClose();
			if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
				delete this;
			}
			else{
				Reset();
			}
			return POLLER_COMPLETE;

		default:
			log_error("input unknow status, netfd[%d], stage[%d]", netfd, stage);
			this->OnClose();
			if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
				delete this;
			}
			else{
				Reset();
			}
			return POLLER_COMPLETE;
		}
}

int CCSocketHandler::OutputNotify ()
{
    int stage = handle_output();
    switch (stage)
	{
		case CONN_SEND_DONE:
			log_debug ("reponse data completed, netfd[%d]", netfd);
		    return POLLER_COMPLETE;
		case CONN_DATA_SENDING:
			log_debug ("reponse data sending, netfd[%d]", netfd);
			return POLLER_SUCC;
		case CONN_FATAL_ERROR:
			this->OnClose();
			if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
				delete this;
			}
			else{
				Reset();
			}
			return POLLER_COMPLETE;
		default:
			log_debug ("response data to client failed, netfd[%d],stage[%d]", netfd, stage);
			this->OnClose();
			if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
				delete this;
			}
			else{
				Reset();
			}
			return POLLER_COMPLETE;
	}
}

int CCSocketHandler::HangupNotify ()
{
	this->OnClose();
	if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
		delete this;
	}
	else{
		Reset();
	}
	return POLLER_COMPLETE;
}
 
int CCSocketHandler::handle_input()
{
	int	ret = 0 ;
	int	packet_len = 0 ;
	int	curr_recv_len   = 0;
	char	curr_recv_buf[MAX_WEB_RECV_LEN] = {'\0'};

	curr_recv_len = ::recv (netfd, curr_recv_buf, MAX_WEB_RECV_LEN, 0);
	log_debug ("*STEP: receiving data, length[%d]", curr_recv_len);
 
	if(-1 == curr_recv_len)//æ¥æ”¶æ•°æ®æœ‰è¯¯
	{
		if(errno != EAGAIN && errno != EINTR && errno != EINPROGRESS)
		{
			DisableInput();
			_stage = CONN_FATAL_ERROR;
			log_boot ("recv failed from fd[%d], msg[%s]", netfd, strerror(errno));
		}
		else
			_stage = CONN_DATA_RECVING;
	}
	else if( 0 == curr_recv_len )//å®¢æˆ·ç«¯å…³é—­è¿æ¥
	{
		DisableInput ();
		_stage = CONN_DISCONNECT;
		log_boot ("connection disconnect by user fd[%d], msg[%s]", netfd, strerror(errno));
	}
	else
	{
		if(curr_recv_len==23 && curr_recv_buf[0]=='<' && curr_recv_buf[1]=='p')		
		{	
			std::string policy = "<policy-file-request/>";			
			for(int i=0; i<23; ++i)			
			{				
				if(curr_recv_buf[i] != policy[i])				
				{					
					_stage = CONN_DATA_ERROR;	
					return _stage;
				}
			}
			std::string resPolicy ="<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>\0";
			this->Send(resPolicy.c_str(), resPolicy.size());
			_stage = CONN_XML_POLICY;		
			log_notice ("flash policy-file-request");
			return _stage;
		}
		log_debug ("1111111111111111111");
		_r.append(curr_recv_buf, curr_recv_len);
		while(_r.data_len() > 0)
		{
			log_debug ("2222222222222");
			packet_len = this->_decode->ParsePacket(_r.data(), _r.data_len());
			log_debug ("333333333333");
			if(packet_len == -1) //æ•°æ®é”™è¯¯
			{
				DisableInput ();
				_stage = CONN_DATA_ERROR;
				break ;
			}
			else if(packet_len == 0) //åŒ…å¤´è§£æå®Œï¼Œç­‰å¾…åŒ…ä½“
			{
				_stage = CONN_DATA_RECVING;
				break;
			}
			else //è§£æåˆ°å®Œæ•´åŒ… inputRet=å®Œæ•´åŒ…é•¿åº¦
			{
				log_debug ("55555555555555");
				ret = this->OnPacketComplete(_r.data(), packet_len);
				log_debug ("66666666666666    ret = %d",ret);	    
				if( ret < 0 )           
				{ 
					_stage = CONN_FATAL_ERROR; 
					break;
				}
				_stage = CONN_RECV_DONE; 
				_r.skip(packet_len);
			}
		}
	}
 
	return _stage;
}

int CCSocketHandler::handle_output()
{
	log_debug("*STEP: send data, len:[%d] netfd[%d]", _w.data_len(), netfd);	
	if (_w.data_len() != 0)
	{		
		int ret = ::send (netfd, _w.data(), _w.data_len(), 0);
		if(-1 == ret)
		{
			if(errno == EINTR || errno == EAGAIN || errno == EINPROGRESS)
			{
				log_boot("sending,INTR|EAGAIN|EINPROGRESS,errno:[%d]", errno);
				EnableOutput ();
				ApplyEvents ();
				_stage = CONN_DATA_SENDING;
				return _stage;
			}
		
			log_boot ("sending package to client failed, ret[%d]",  ret);	
			DisableInput ();
			DisableOutput ();
			ApplyEvents ();
			_stage = CONN_FATAL_ERROR;
			return _stage;
		}

		if(ret == (int)_w.data_len())
		{
			log_debug("send complete, send len=[%d]",ret);
			DisableOutput();
			ApplyEvents ();
			_w.skip(ret);	
			_stage = CONN_SEND_DONE;
			return _stage;
		}
		else if (ret < (int)_w.data_len())
		{
			log_debug("had sent part of data, send len=[%d]",ret);
			EnableOutput ();
			ApplyEvents ();
			_w.skip(ret);
			_stage = CONN_DATA_SENDING;
			return _stage;
		}
	}

	DisableOutput();
	ApplyEvents ();	
	_stage = CONN_FATAL_ERROR;
	log_debug("send process failure");
	return _stage;
}

int CCSocketHandler::Send(const char * buff, int len)
{
	if(len>0)
	{
		if (GetReconnectFlag()==true){
			Connect();
		}
		const char* sendbuff = buff;
		int sendlen = len;

		if(this->_w.data_len()==0)
		{
			int ret = ::send (netfd,buff, len, 0);
			log_debug("Send Fd[%d]", netfd);
			if(-1 == ret)
			{
				if(errno == EINTR || errno == EAGAIN || errno == EINPROGRESS)
				{
					log_boot("sending,INTR|EAGAIN|EINPROGRESS,errno:[%d]", errno);
					this->_w.append(sendbuff, sendlen);
					EnableOutput ();
					ApplyEvents ();
					_stage = CONN_DATA_SENDING;
					return 0;
				}
				else
				{
					log_boot ("sending package to client failed, ret[%d]",  ret);	
					_stage = CONN_FATAL_ERROR;
					this->OnClose();
					if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
						delete this;
					}
					else{
						Reset();
					}
					return -1;
				}
			}
			else if(ret<len)
			{
				sendbuff += ret;
				sendlen -=  ret;
				this->_w.append(sendbuff, sendlen);
				EnableOutput ();
				ApplyEvents ();
				_stage = CONN_DATA_SENDING;
				log_debug("had sent part of data, send len=[%d]",ret);
				return ret;
			}
			else if(ret==len)
			{
				log_debug("send complete, send len=[%d]",len);
				_stage = CONN_SEND_DONE;
				return ret;
			}
		}
		else
		{
			this->_w.append(sendbuff, sendlen);
			if( handle_output() ==CONN_FATAL_ERROR )
			{
				this->OnClose();
				if(GetReconnectFlag()==false){//²»ĞèÒªÖØÁ¬
					delete this;
				}
				else{
					Reset();
				}
				return -1;
			}
			else
				return len;
		}
	}
	return len;
}

int CCSocketHandler::OnPacketComplete(const char* data, int len)
{
	return 0;
}	

int CCSocketHandler::OnClose()
{
	log_debug("client  OnClose: fd=[%d] ip=[%d:%d]",this->GetNetfd(),this->GetIP(),this->GetPort());
	return 0;
}

int CCSocketHandler::OnConnected()
{
	log_debug("client  OnConnected: fd=[%d] ip=[%d:%d]",this->GetNetfd(),this->GetIP(),this->GetPort());
	return 0;
}

void CCSocketHandler::Reset(){
	log_debug("reset socket handler");
	_w.skip( _w.data_len() );
	_r.skip( _r.data_len() );
	DisableInput();
	DisableOutput();
	ApplyEvents();
	CPollerObject::DetachPoller();
	if(netfd > 0)
	{
		::close(netfd);
	}
	netfd  = -1;
	_stage = CONN_IDLE;
	return;
}


int CCSocketHandler::Connect(){
	int ret = -1;
	if (_stage == CONN_IDLE){
		ret = CNet::tcp_connect(&netfd, GetSIP().c_str(), GetPort(), 0,500);
		log_debug("Connect...");
	}
	else
	{
		if (netfd < 0)
		{
			ret = CNet::tcp_connect(&netfd, GetSIP().c_str(), GetPort(), 0,500);
		}
		else
		{
			ret = 1;
		}
	}
	
	if(ret <0)
	{
	/*	if (ret == SOCKET_CREATE_FAILED)
		{
			log_error("*STEP: helper create socket failed, errno[%d], msg[%s]", errno, strerror(errno));
			return -1;
		}

		if(errno != EINPROGRESS)
		{
			log_error("*STEP: PROXY connect to logic failed, errno[%d], msg[%s]", errno , strerror(errno));
			return -1;
		}

		_stage = CONN_CONNECTING;
		
		log_debug("*STEP: PROXY connecting to logic, unix fd[%d]", netfd);
		goto exit;*/

		if( netfd > 0)
		{
			close(netfd);
		}
	
		return -1;
		
	}
	else if( 0 == ret)
	{

		_stage = CONN_CONNECTED;
		OnConnected();
	}
//exit:
	return CCReactor::Instance()->AttachPoller(this);
}


