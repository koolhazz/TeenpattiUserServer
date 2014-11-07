#ifndef BOYAA_MEMCACHE_CLIENT_H_20100127
#define BOYAA_MEMCACHE_CLIENT_H_20100127

#ifndef WIN32 
//#include <memcache.h>
#include <memcached.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
using namespace std;

class CMemcacheClient
{
public:
	CMemcacheClient(void);
	~CMemcacheClient(void);
public:
	bool Init(const char* pszServer);
	bool SetRecord(char* key, const char* value, const size_t bytes, const time_t expire = 0);
	bool Replace(char* key, const char* value, const size_t bytes, const time_t expire = 0);
	void GetRecord(const char* key, string &value);
	bool DeleteRecord(const char* key);
	bool increment(const char *key, int offset, uint64_t &value);
	bool decrement(const char *key, int offset, uint64_t &value);
	bool checkKey(const char *key);
private:
	//struct memcache *m_pMemcache;
	memcached_return rc;
	memcached_st *memc;

};
#endif
#endif
