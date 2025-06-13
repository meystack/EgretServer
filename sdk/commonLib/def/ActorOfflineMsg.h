#pragma once
#pragma  pack(push,4)

/*
*玩家的离线消息
*/
struct ActorOfflineMsg
{
	char name[32];				//玩家的名字
	char sGuildName[32];		//行会名称
	unsigned int nActorId;		//玩家ID
	unsigned int nLevel;    	//玩家的等级
	BYTE nSex;					//玩家的性别
	BYTE nVocation;  			//玩家的职业
	BYTE nCircle;				//玩家的转数
	BYTE nReserve;				//保留

	CActorOfflineProperty actordata;

	ActorOfflineMsg()
	{
		memset(this,0,sizeof(*this));
	}
};


#pragma  pack(pop)


/*
*玩家英雄的离线消息
*/
struct HeroOfflineMsg
{
	BYTE         bCircle;	 //转数 
	BYTE         bLevel;     //等级
	BYTE         bVocation;  //职业
	BYTE		 bSex;		 //性别
	unsigned int nScore;     //评分

	int      nModelId;      //玩家的外观
	int      nWeaponAppear; //玩家武器的，模型ID
	int		 nWingAppear;	//玩家的翅膀外观ID

	HeroOfflineMsg()
	{
		memset(this,0,sizeof(*this));
	}
};
