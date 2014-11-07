#include "GameUserSHM.h"


using namespace comm::commu;



GameUserSHM::GameUserSHM()
{
}

GameUserSHM::~GameUserSHM()
{
}

void GameUserSHM::SetHashInfo(THashPara hashPara,int shm_key,int sem_key)
{
	m_HashInfo.hash_para_.bucket_size_ = hashPara.bucket_size_; 	//hash Ͱ������ֵ65537Ч�ʱȽϸ�
    m_HashInfo.hash_para_.node_total_ = hashPara.node_total_; 		//�ڵ��������ٸ�����
    m_HashInfo.hash_para_.chunk_total_ = hashPara.chunk_total_; 	//CHUNK��Ƭ�����������ǵ�Ӧ�ã�ÿ�����ݴ�С��ͬ��������Ϊ�ͽڵ�����ͬ
    m_HashInfo.hash_para_.chunk_size_ = sizeof(CGameUser);		//CHUNK��Ƭ��С

    m_HashInfo.shm_key_ = shm_key;						//�����ڴ�key��Server�����ú�port��ͬ
    m_HashInfo.sem_key_ = sem_key;						//�ź���key��Server�����ú�port��ͬ
}

int GameUserSHM::Init()
{
	return m_ShmHashMap.Init(m_HashInfo);
}

int GameUserSHM::Init(THashInfo hashInfo)
{
	return m_ShmHashMap.Init(hashInfo);
}


int GameUserSHM::Insert(const CGameUser & user)
{
	return m_ShmHashMap.Insert(user.m_nUid , (void*)&user , sizeof(user));
}

int GameUserSHM::Find(int uid, CGameUser & user)
{
	int len = sizeof(CGameUser);
	int ret=m_ShmHashMap.Find(uid, (char*)&user, &len);

/*	
	if( ret != 0 || len!=sizeof(user))
    {
		printf("Find failed|uid:%d	ret:%d|readLen:%d \n",UID	, ret , len);
    } else 
    {
        printf("uid:%d \n" , mo_GameUser.uid);
    }
*/    
	return ret;
}
	
int GameUserSHM::Delete(int uid)
{
	return m_ShmHashMap.Delete(uid);
}

int GameUserSHM::SetValue(const CGameUser & user)
{
	return m_ShmHashMap.SetValue(user.m_nUid , (void*)&user, sizeof(CGameUser));
}

