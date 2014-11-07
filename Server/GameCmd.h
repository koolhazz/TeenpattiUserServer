#ifndef _GAME_CMD_H_
#define _GAME_CMD_H_

#define SYS_CTRL_KEY "sys_ctrl_key_alloc"


#define CLIENT_PACKET  		 0x0001//�û������������ݰ�����server�����û��İ�
#define CLIENT_PACKET2		 0x0004

#define CLIENT_CLOSE_PACKET  0x0002//�û��Ͽ����ӣ�hall���߼�server���İ�
#define SERVER_CLOSE_PACKET  0x0003//�߼�server�����Ͽ����ӵİ�
#define ALLOC_REGISTER_PACKET 0x0005//Allocע���
#define SYS_GET_USER_TABLE  0x0020

const int CLIENT_CMD_USER_LOGIN = 0x0101;//�û���¼
const int SERVER_CMD_LOGIN_SUCCESS = 0x0201;//�û���¼�ɹ�
const int SERVER_CMD_KICK_OUT = 0x0203;  //�޳��û�
const int CMD_GET_ROOM_LEVER_NUM = 0x0311;//��ȡ���ȼ�������  

const int SYS_USER_ENTER_ROOM = 0x0505;
const int SYS_USER_LEAVE_ROOM = 0x0506;
const int SYS_USER_STAND_ROOM = 0x050A;
const int SYS_STAND_LEAVE_ROOM = 0x050B;
const int SYS_HALL_CORE = 0x0507;
const int SYS_LOGIC_CORE = 0x0508;

const int SYS_SYN_DATA = 0x050D;//ͬ������

const int CMD_CLEAR_SERVERINFO = 0x0905;//���server��Ϣ������server�����ʱ��ʹ�ã���UserServer���´�mc�ж�ȡserver��Ϣ
const int SYS_SET_TJ_SWITCH = 0x0511;
const int SYS_SET_LEVEL_COUNT_RANDOM = 0x0513;
const int GET_USER_HALLID	= 0x00a1;	//ȥuserserverȡ�û��Ĵ���ID

enum
{
	ERROR_ROOM_NOT_EXIST = 1,
	ERROR_MAX_USERCOUNT,
};


const int SERVER_ATTR_COUNT = 3;
const int ONLINE_ATTR_COUNT = 3;

const int RANDOM_COUNT = 100;
const int TIME_INTEVAL = 301;//ʱ����5����
const int FLUSH_CHECK_TIME = (10*60+1);

#define TIME_CHECK 1
#define OFFLINE_CHECK 2
#define	FLUSH_CHECK	3

#endif

