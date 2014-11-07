#ifndef _GAME_CMD_H_
#define _GAME_CMD_H_

#define SYS_CTRL_KEY "sys_ctrl_key_alloc"


#define CLIENT_PACKET  		 0x0001//用户发过来的数据包或者server发给用户的包
#define CLIENT_PACKET2		 0x0004

#define CLIENT_CLOSE_PACKET  0x0002//用户断开连接，hall给逻辑server发的包
#define SERVER_CLOSE_PACKET  0x0003//逻辑server主动断开连接的包
#define ALLOC_REGISTER_PACKET 0x0005//Alloc注册包
#define SYS_GET_USER_TABLE  0x0020

const int CLIENT_CMD_USER_LOGIN = 0x0101;//用户登录
const int SERVER_CMD_LOGIN_SUCCESS = 0x0201;//用户登录成功
const int SERVER_CMD_KICK_OUT = 0x0203;  //剔除用户
const int CMD_GET_ROOM_LEVER_NUM = 0x0311;//获取各等级场人数  

const int SYS_USER_ENTER_ROOM = 0x0505;
const int SYS_USER_LEAVE_ROOM = 0x0506;
const int SYS_USER_STAND_ROOM = 0x050A;
const int SYS_STAND_LEAVE_ROOM = 0x050B;
const int SYS_HALL_CORE = 0x0507;
const int SYS_LOGIC_CORE = 0x0508;

const int SYS_SYN_DATA = 0x050D;//同步数据

const int CMD_CLEAR_SERVERINFO = 0x0905;//清除server信息，启动server出错的时候使用，让UserServer重新从mc中读取server信息
const int SYS_SET_TJ_SWITCH = 0x0511;
const int SYS_SET_LEVEL_COUNT_RANDOM = 0x0513;
const int GET_USER_HALLID	= 0x00a1;	//去userserver取用户的大厅ID

enum
{
	ERROR_ROOM_NOT_EXIST = 1,
	ERROR_MAX_USERCOUNT,
};


const int SERVER_ATTR_COUNT = 3;
const int ONLINE_ATTR_COUNT = 3;

const int RANDOM_COUNT = 100;
const int TIME_INTEVAL = 301;//时间间隔5分钟
const int FLUSH_CHECK_TIME = (10*60+1);

#define TIME_CHECK 1
#define OFFLINE_CHECK 2
#define	FLUSH_CHECK	3

#endif

