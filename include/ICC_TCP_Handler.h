#ifndef  _ICC_TCP_HANDLE_H_
#define  _ICC_TCP_HANDLE_H_

#include "poller.h"
#include <string>
using namespace std;

class ICC_TCP_Handler : public CPollerObject 
{
	public:
		ICC_TCP_Handler(){_enableReconnect = false;};
		virtual ~ICC_TCP_Handler(){};
 
	public:
		virtual int OnClose()=0;									// ���ӶϿ������ ע��:�������1�򲻻�ɾ������ֻ��ر�����
		virtual int OnConnected()=0;								// ���ӳɹ����������
		virtual int OnPacketComplete(const char * data, int len)=0; // ������������ݰ�ʱ����
		
	public:
		inline int GetIP(){return this->_ip;}; 
		inline string GetSIP(){return this->_sip;}
		inline void SetIP(int ip){ this->_ip = ip;}; 
		inline void SetSIP(const string & ip){this->_sip = ip;};
		inline uint16_t GetPort(){return this->_port;}; 
		inline void SetPort(uint16_t port){ this->_port = port;}; 

		inline int GetNetfd(){return this->netfd;}; 
		inline void SetNetfd(int netfd){ this->netfd = netfd;}; 
		inline void EnableReconnect(){_enableReconnect = true;}
		inline void DisableReconnect(){_enableReconnect = false;}
		inline bool GetReconnectFlag(){return _enableReconnect;}
	protected:
		in_addr_t	_ip;
		string _sip;
		uint16_t	_port;
		bool _enableReconnect;//
};
#endif

