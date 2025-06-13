// 机器人模块一些公共的定义文件
#pragma  once

// 场景名称最大字节数
#define MAX_SCENE_NAME 128


//服务器的ip和端口
typedef struct tagServerConfig
{
	char ip[32];  //服务器的ip
	int  nPort;   //服务器最小端口
	int	 nMaxPort;	//服务器最大端口号
}SERVERCONFIG,*PSERVERCONFIG;

//账户的名字和密码
typedef struct tagLoginAccount
{
	char name[32];  //名字
	char pass[16];     //密码
	byte bDoWord;		//是否做任务
}LOGINACCOUNT,*PLOGINACCOUNT;

// 进入游戏后地图分布
typedef struct tagSceneMapPoint
{
	INT_PTR nSceneId;						// 场景Id
	char szSceneName[MAX_SCENE_NAME];		// 场景名称
	INT_PTR nX;								// 地图点X坐标
	INT_PTR nY;								// 地图点Y坐标
	int nLevel;								// 场景推荐等级
	tagSceneMapPoint()
	{
		ZeroMemory(this, sizeof(*this));
	}
}MapScenePoint;