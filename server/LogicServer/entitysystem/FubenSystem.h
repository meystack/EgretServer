#pragma once

typedef struct  tagTeamOutput
{
	unsigned int nActorid;
	char	sName[MAX_ACTOR_NAME_LENGTH];	 
	unsigned int nOutput;
}TeamOutput, *PTeamOutput;

class CFubenSystem:
	public CEntitySubSystem<enFubenSystemID,CFubenSystem,CActor>
{
public:

	enum EnFubenEvent{
    	enOnCheckEnter = 1,		//进入检查     [副本id，进入类型，玩家指针]
		enOnCreate = 2,    		//副本创建     [副本id，进入类型，副本指针]
    	enOnEnter = 3,    		//实体进入副本 [副本id，进入类型，副本指针，场景id，实体指针]
   		enOnExit = 4,     		//实体退出副本 [副本id，进入类型，副本指针，场景id，实体指针]
   		enOnUpdate = 5,   		//副本帧更新   [副本id，进入类型，副本指针，当前时间]
    	enOnDeath = 6,    		//实体死亡     [副本id，进入类型，副本指针，场景id，实体指针]
    	enOnGetAward = 7,   	//获取副本奖励 [副本id，进入类型，副本指针，场景id，玩家指针]
		enOnTimeout = 8,	//副本场景超时 [副本id，进入类型，副本指针，场景id]
    	enOnAttacked = 9,     	//实体收到攻击 [副本id，进入类型，副本指针，场景id，受击者，攻击者] 注意：怪物攻击怪物不触发
		enOnHurtDamage= 10,     //实体受到伤害 [副本id，进入类型，副本指针，场景id，受击者，攻击者,伤害值] 注意：怪物攻击怪物不触发
	};

public:

	CFubenSystem()
	{
	}
	~CFubenSystem() { }

public:

	//Comments: 初始化
	virtual bool Initialize(void *data,SIZE_T size);

	//Comments: 处理客户端的数据包 nCmd:帮派系统的消息id packet:消息包
	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	//Comments: 发送当前的副本或场景的剩余时间
	void SendRestTime();

	//Comments:玩家进入游戏的时候触发，用于发送某些数据给玩家初始化
	virtual void OnEnterGame();

	//Comments:玩家登出的时候触发
	void OnActorLoginOut();
  
public: //

	//Comments:申请进入副本 nFbid:副本id
	bool ReqEnterFuben(int nFbid);

	//Comments:退出副本 nFubenId:副本id
	bool ReqExitFuben( int nFubenId );

	//Comments:请求获取奖励 nFbid:副本id
	void GetFubenAward(int nFbid);

private:

};
