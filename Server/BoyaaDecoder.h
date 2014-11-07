#ifndef BOYAA_DECODER_H
#define BOYAA_DECODER_H 

#include "CCSocketHandler.h"
#include "CCStreamDecoder.h"
#include "Packet.h"
#include "Global.h"
#include "clib_log.h"
extern clib_log * g_pDebugLog;
extern clib_log * g_pErrorLog;

class BoyaaDecoder :  public CCStreamDecoder
{
public:
	virtual ~BoyaaDecoder()	{}
	static BoyaaDecoder* getInstance()
	{
		static BoyaaDecoder *instance = NULL;
		if( NULL == instance)
		{
			instance = new BoyaaDecoder();
		}
		return instance;
	}

protected:
	virtual inline int GetHeadLen(const char * data,int len)
	{
		return sizeof(TPkgHeader);
	}

	virtual inline int GetPacketLen(const char * data,int len)
	{
		struct TPkgHeader* pHeader = (struct TPkgHeader*)data;
		int pkgLen = sizeof(short) + ntohs(pHeader->length);
		if( pkgLen <= 0 || pkgLen > 10240)
		{
			log_error("pkgLen[%d] error", pkgLen);
			return -1;
		}
		else
			return pkgLen;
	}

	virtual inline bool CheckHead(const char * data,int len)
	{
		struct TPkgHeader* pHeader = (struct TPkgHeader*)data;
		if( pHeader->flag[0] != 'B' || pHeader->flag[1] != 'Y')
		{
			log_error("flag error");
			return false;
		}
			
		int pkgLen = sizeof(short) + ntohs(pHeader->length);
		if( pkgLen <= 0 || pkgLen > 10240)
		{
			log_error("PkgLen[%d] Error", pkgLen);
			return false;
		}
		
		return true;
	}

private:
	BoyaaDecoder()	{}
	
};



class BoyaaDecoder1 :  public CCStreamDecoder
{
public:
	virtual ~BoyaaDecoder1()	{}
	static BoyaaDecoder1* getInstance()
	{
		static BoyaaDecoder1 *instance = NULL;
		if( NULL == instance)
		{
			instance = new BoyaaDecoder1();
		}
		return instance;
	}

protected:
	virtual inline int GetHeadLen(const char * data,int len)
	{
		return sizeof(TPkgHeader);
	}

	virtual inline int GetPacketLen(const char * data,int len)
	{
		struct TPkgHeader* pHeader = (struct TPkgHeader*)data;
		int pkgLen = sizeof(short) + (pHeader->length);
		if( pkgLen <= 0 || pkgLen > 10240)
		{
			log_error("pkgLen[%d] error", pkgLen);
			return -1;
		}
		else
			return pkgLen;
	}

	virtual inline bool CheckHead(const char * data,int len)
	{
		struct TPkgHeader* pHeader = (struct TPkgHeader*)data;
		if( pHeader->flag[0] != 'B' || pHeader->flag[1] != 'Y')
		{
			log_error("flag error");
			return false;
		}
			
		int pkgLen = sizeof(short) + (pHeader->length);
		if( pkgLen <= 0 || pkgLen > 10240)
		{
			log_error("PkgLen[%d] Error", pkgLen);
			return false;
		}
		
		return true;
	}

private:
	BoyaaDecoder1()	{}
	
};


#endif


