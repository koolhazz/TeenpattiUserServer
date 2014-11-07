#include "MemCacheClient.h"

CMemcacheClient::CMemcacheClient(void)
{
}

CMemcacheClient::~CMemcacheClient(void)
{
}

/* 
 * Adds a server to the list of available servers.  By default,
 * servers are assumed to be available.  Return codes:
 *
 * 0:	success
 * -1:	Unable to allocate a new server instance
 * -2:	Unable to strdup hostname
 * -3:	Unable to strdup port
 * -4:	Unable to Unable to resolve the host, server deactivated, but added to list
 * -5:	Unable to realloc(3) the server list, server list unchanged
*/
bool CMemcacheClient::Init(const char* pszServer)
{

	memc = memcached_create(NULL);
	int nPort = 11211;
	char * pszPort = strchr(pszServer, ':');
	if (pszPort) 
	{
		nPort = atoi(pszPort+1);
		*pszPort = 0;
	}
	memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1); 
	memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_POLL_TIMEOUT, 1000); 
	rc = memcached_server_add(memc, pszServer, nPort);
	if(rc != MEMCACHED_SUCCESS)
	{
		memcached_free(memc);
		return false;
	}
	return rc==MEMCACHED_SUCCESS;
}


bool CMemcacheClient::SetRecord(char* key, const char* value, const size_t bytes, const time_t expire)
{
	rc = memcached_set(memc, key, strlen(key), value, bytes, expire, 0);
	return rc==MEMCACHED_SUCCESS || rc==MEMCACHED_BUFFERED;
}

bool CMemcacheClient::Replace(char* key, const char* value, const size_t bytes, const time_t expire)
{
	rc = memcached_replace(memc, key, strlen(key), value, bytes, expire, 0);
	return rc==MEMCACHED_SUCCESS || rc==MEMCACHED_BUFFERED;
}

/* 
* mc_get() is the preferred method of accessing memcache.  It accepts
* multiple keys and lets a user (should they so choose) perform
* memory caching to reduce the number of mcMalloc(3) calls makes. 
*/
void CMemcacheClient::GetRecord(const char* key, std::string &value)
{
	size_t value_length = 0;
	uint32_t flags = 0;
	char *val = memcached_get(memc, key, strlen(key), &value_length, &flags, &rc);
	if(val !=NULL)
	{
		value = val;
		free(val);
	}
	else
	{
		value.clear();
	}
}

bool CMemcacheClient::DeleteRecord(const char* key)
{

	rc = memcached_delete(memc, key, strlen(key), 0);
	return rc==MEMCACHED_SUCCESS;
}

bool CMemcacheClient::increment(const char *key, int offset, uint64_t &value)
{
	rc = memcached_increment(memc, key, strlen(key), offset, &value);
	return rc==MEMCACHED_SUCCESS;
}

bool CMemcacheClient::decrement(const char *key, int offset, uint64_t &value)
{
	rc = memcached_decrement(memc, key, strlen(key), offset, &value);
	return rc==MEMCACHED_SUCCESS;
}

bool CMemcacheClient::checkKey(const char *key)
{
	size_t value_length = 0;
	uint32_t flags = 0;
	memcached_get(memc, key, strlen(key), &value_length, &flags, &rc);
	if (rc == MEMCACHED_NOTFOUND) {
		return false;
	}
	return true;
}
