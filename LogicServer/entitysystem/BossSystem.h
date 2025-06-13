
//BOSS子系统

#pragma  once

class CBossSystem:
	public CEntitySubSystem<enBossSystemID, CBossSystem,CActor>
{
public:

	enum EnBossEvent{
			enOnInit = 1,           //boss初始化
			enOnEnterFuben = 2,          //玩家进入Boss副本
			enOnExitFuben = 3,       //玩家离开Boss副本
			enOnEntityDeath = 4,       //副本实体死亡
			enOnEntityAttacked = 5,  //--副本实体收到伤害[副本指针，受击者，攻击者]
			enOnReqData     = 6,     //--boss数据
			enOnCancelBL   = 7,//--取消归属
			enOnSetBossBL   = 8,//--设置归属
			enCheckEnterFuben   = 9,//--进入副本检查
			enFubenFinish = 10,     //--活动副本结束    [, 副本指针，结果, pOwner] 1为完成，0为失败，nil则结果未知（需要对应副本设置结果）
    		enReqEnterFuben = 11,     //--请求进入副本    [玩家指针, bossid]
		};
	typedef CEntitySubSystem<enBossSystemID, CBossSystem, CActor> Inherit;

//最多记录boss条数
#define MAXBOSSGROWROW 10
	
	CBossSystem();
	~CBossSystem();

	//覆盖父类函数

	bool Initialize(void *data,SIZE_T size);

	VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	void OnTimeCheck(TICKCOUNT nTickCount);

	void Save(PACTORDBDATA pData);


	//请求boss信息
	void RequestBossInfo(CDataPacketReader & packet);

	//请求一个boss信息
	void RequestOneBossInfo(CDataPacketReader & packet);

	/*
	* Comments:BOSS副本操作
	* Param CDataPacketReader & packet:
	* @Return void:
	*/
	void BossFubenOpt(CDataPacketReader & packet);

	//静态方法

	//获取怪的成长信息
	static int GetMonsGrowInfo(int nMonsterId, int & nGrowLv, int & nCurExp);
	//发送神装boss 剩余次数
	void SendSZBossTimes(int nLevel = 0);
	void OnEnterGame();


	/*
	* Comments:更新boss的成长，添加击杀人
	* Param int nMonsterId:怪的id
	* Param LPCTSTR sName:击杀者名称
	* Param PMONSTERCONFIG pConfig:怪的配置
	* @Return int:返回成长等级
	*/
	static int OnKillGrowBoss(int nMonsterId, LPCTSTR sName, PMONSTERCONFIG pConfig = NULL);
	//进入boss系统快传+1


private:
	void SendWildBossSts();
	void SendWorldBossSts();
	void SendBossLastKillTime( WORD nBossId );		//下发BOSS最近一次被击杀的时间（服务器启动后）
	void TeleportToBoss(int nBossId);
	void AnheiBossInfo(unsigned char nIndex);
	
};