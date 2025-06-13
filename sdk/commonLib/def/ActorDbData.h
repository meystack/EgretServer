#pragma once
#pragma  pack(push,4)

/*
    玩家需要存盘的数据结构体
*/

#define MAX_ACHIEVE_ID_SIZE 64   			//成就的最大ID
#define MAX_ACHIEVE_DATA_SIZE 750   		//成就的最大的BYTE数量
#define MAX_ACHIEVE_EVENT_DATA_SIZE 256     //成就事件最大的BYTE 数量
#define MAX_TITLE_DATA_SIZE 32              //称号的存储的BYTE数量
#define QUESTBTYE   256

//这个是和db通信的包
typedef struct tagActorDbData:
	public ENTITYDATA,
	public ACTORDBPARTDATA
{
	unsigned  nLevel ; 			//等级
	ACTORNAME name;  			//名字
	unsigned nFbHandle; 		//副本的句柄，如果是0表示是普通的场景
	unsigned nScenceID; 		//场景id
	unsigned nHp;   			//当前的血
	unsigned nMp;  				//当前的魔
	unsigned  nEnterFbScenceID;
	int       nEnterFbPos; 		//X 和y放在了一起
	char      quest[QUESTBTYE]; //任务的数据
	
	unsigned int  nCreateTime ; 		//创建的时间
	unsigned long long lLastLoginIp ;	//上次登录的ip
	unsigned int nLastLoginTime;		//上次登录时间
	unsigned int nLastLogoutTime ; 		//上次的下线时间,这个表示上次在逻辑服务器的下线时间,minidate格式
	unsigned int nDeport2ExpiredTime;
	unsigned int nDeport3ExpiredTime; 	//仓库3的过期时间
	unsigned int nDeport4ExpiredTime; 	//仓库4的过期时间
	ACCOUNT  accountName;       		//账户名字
	char	 sGuildname[32];			//行会名字（只保存方便查询，不读）

	int nActorDir;    //方向，以及其他的一些信息，复用
	unsigned int nAccountId; //账户的ID
	char achieveFinishState[MAX_ACHIEVE_DATA_SIZE];			//成就的完成状态
	char achieveEventData[MAX_ACHIEVE_DATA_SIZE];		//成就事件的完成状态
	char titles[MAX_TITLE_DATA_SIZE];						//称号数据
	long long csrevivepoint; //普通场景的复活点，是场景id,x,y的组合
	long long fbrevivepoint; //副本场景的复活点，是场景id,x,y的组合
	long long cityrevivepoint; //城镇的复活点，是场景id,x,y的组合
	unsigned int nExpToday;    //角色今天获取的经验的数量
	unsigned int nZyContrToday;    //今天获得的阵营贡献值
	
	unsigned int      nFashionBattle; //时装战力（元宝的第1次的种植的数据）
	int      nAlmirahLv;  // 衣橱等级
	unsigned int  nEquipScore;   //玩家的装备的总分
	unsigned int  nConsumeYb;    //消费的gm
	int		 nSwingLevel;				//翅膀档次
	int		 nSwingId;					//翅膀id
	int		 nLoginDays;				//登陆的天数
	//////////////////////////////////////////////////////////////////////////
	// 存DB，不用下发给客户端。不作为玩家属性
	int		nLastLoginServerId; // 玩家最后一次登录的服务器Id.
	int		nSalary;		//工资点【高16位保存上周工资点|低16为保存本周工资点】
	int		nOffice;
	int		nDepotCoin;
	int		nPlayerMaxAttack;	//玩家最大攻击力
	int     nShutUpTime; //禁言时间
	int		nNextChangeVocTime;//下次可转职的时间戳
	int		nOldSrvId;//原区服id
	tagActorDbData()
	{
		memset(this,0,sizeof(tagActorDbData));
	}
}ACTORDBDATA,*PACTORDBDATA;

#pragma  pack(pop)
