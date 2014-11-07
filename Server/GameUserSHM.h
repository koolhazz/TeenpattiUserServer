#pragma once
//#include "inet.h"
#include "GameUser.h"
#include "HashMap.hpp"
#include "ShmHashMap.hpp"

class GameUserSHM
{
protected:
	comm::commu::THashInfo m_HashInfo;
	
public:
	void SetHashInfo(comm::commu::THashPara hashPara,int shm_key,int sem_key);

	int Init();
	int Init(comm::commu::THashInfo hashInfo);
	
	int Insert(const CGameUser & user);
	int Find(int uid, CGameUser & user);
	int Delete(int uid);
	int SetValue(const CGameUser & user);

public:
	GameUserSHM();
	~GameUserSHM();
	GameUserSHM(const GameUserSHM &);
	const GameUserSHM & operator = (const GameUserSHM &);
	
public:
	comm::commu::CShmHashMap<int> m_ShmHashMap;
};
