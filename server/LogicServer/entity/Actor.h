
#pragma once

/***************************************************************/
/*
/*                     玩家的实体
/*
/***************************************************************/

#include<string>
using namespace std;
//#include "DataPacket.hpp"
 

static const char *szExploitCount		= "ExploitCount";					//一天只能加10次功勋
static const char *szMaxActorCircle		= "MaxActorCircle";					//曾经最高的转数
static const char *szMaxActorLevel		= "MaxActorLevel";					//曾经最高的等级
static const char *szAchieveCount		= "nMeritoriousChestCountData";		//功勋的抽奖次数
static LPCTSTR  RechargeStatus			= "RechargeStatus";					//



//玩家附加的属性，默认初始化都是0
struct CActorBattleProp
{
	float                   m_zyContRate;      //阵营贡献度的增加速度(只用于玩家)
	float                   m_renowAddRate;   // 声望的获取速度加成(浮点型),+%n
	
	unsigned int            m_consumeyb;      //累计消耗的元宝的数目
	

	unsigned int            m_nZyContriGetToday;	    // 角色当天获取的阵营贡献（存db）
	unsigned int            m_nZyContriGetLastLogout;   // 角色上次下线那天阵营贡献

	unsigned int            m_nExpGetToday;	            // 角色当天获取的经验值（存db）
	unsigned int			m_nExpGetLastLogout;         // 角色上次下线那天杀怪获取的经验值

	float                   m_fExpRate;             //杀怪的经验倍率
	float                   m_fBindCoinRate;             //捡取金币的倍率
	int                     m_nSkillExpRate;     //练技能熟练度的加成
	int						m_nReincarnationTimes;		//转生次数
	unsigned int			m_nBase_MaxHp;	//基本生命值
	unsigned int			m_nBase_MaxMp;	//基本魔法值

	CActorBattleProp()
	{
		memset(this,0,sizeof(*this));
	}
};

typedef struct tagJoinMap
{
	int                     m_nJDJoinTime;               //玩家进入禁地时间
	int                     m_nJDJoinMapId;               //玩家进入禁地地图id
	int                     m_nMapGroup;               //玩家进入地图组
	tagJoinMap()
	{
		memset(this,0,sizeof(*this));
	}
}JoinMap;

class CCamp;
class CActor:
	public CAdvanceAnimal,
	public CActorBattleProp
{
public:
	typedef CAdvanceAnimal Inherited;
	friend struct CActorPacket;
	friend class CPropertySystem;
	// 传送类型
	enum enTransmitType
	{
		enTransmitAtLogin,	// 在登录游戏的时候传送
		enTransmitInGame,	// 在游戏中传送
	};

	// 传送状态
	enum enTransmitStatus
	{
		enTS_IDLE,				// 非传送状态
		enTS_QueryDestServer,	// 查询目标服务器是否准备好
		enTS_StartTransmit,		// 开始传送。开始之后要么是马上存盘，要么是直接执行传送
		enTS_WaitSaveActorData,	// 角色数据存盘阶段
		enTS_DoTransmit,		// 执行传送阶段，发消息给客户端开始传送
	};

	enum enActorStatus
	{
		enAS_Delete,
		enAS_Seal,
		enAS_Normal,
		enAS_InUsed,
	};

public:
	CActor();//构造函数
	bool Init( void * pData, size_t size) ;
	void Destroy(  );

	static int GETTYPE() {return enActor;}

	void UpdateUseDeopt() ;

	//玩家死亡后要触发一个事件
	//virtual void OnEntityDeath();
	
	virtual void OnKilledByEntity(CEntity * pKillerEntity);

	//获取玩家在网关的唯一的key
	inline Uint64 GetGateKey (){return m_lKey;}
	
	//设置网关的key
	inline void SetGateKey(UINT64 lKey)
	{
		m_lKey =lKey;
	}
	//处理跨天成就
	void dealNewDayAchieve(int nDiffDay = 1);
	//客户端需要 玩家是否处于活动地图
	void CrossActivityArea(BYTE nState = 0);
	//泡点地图 默认 1
	void AddPaoDianExp(int nType = 1, int nAtvId = 0);
	void AreaCostCoin();
	//设置创建角色登陆的新手村村索引
	inline void SetCreateIndex(int nIndex) {m_CreateIndex = nIndex;}

	inline int GetCreateIndex() {return m_CreateIndex;}

	//初始化实体的广播的mask，哪些属性是需要广播的
	static void InitActorBroadcastMask();
	static void DestroyActorBroadcastMask()
	{
		SafeDelete(CActor::s_actorBroadcastMask);
	}

	inline CQuestSystem * GetQuestSystem()  {return &m_questSystem;}  //任务子系统
	inline CGuildSystem * GetGuildSystem()	{return &m_guildSystem;}//帮派子系统
	inline CChatSystem * GetChatSystem() { return & m_chatSystem;}  //聊天子系统
	inline CGameStoreBuySystem& GetStoreSystem(){return m_GameStoreBuySystem;}
	inline CDealSystem& GetDealSystem(){ return m_DealSystem; }
	inline CUpdateMask * GetBroadCastMask() {return s_actorBroadcastMask;}
	virtual char * GetForbitMask() {return s_actorForbitMask;}
	virtual CPropertySet * GetPropertyPtr() {return &m_property ;}
	virtual const CPropertySet * GetPropertyPtr() const {return &m_property ;}

	virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}

	inline  CLootSystem& GetLootSystem(){return  m_lootSystem;}// 拾取子系统
	inline  CStrengthenSystem& GetStrengthenSystem() {return m_strengthenSystem; }//强化系统

	inline CGhostSystem& GetGhostSystem() {return m_nGhostSystem;} //神魔
	inline CGameSetsSystem& GetGameSetsSystem(){return  m_GameSets;}// 设置子系统
	inline CTeamSystem& GetTeamSystem(){return m_teamSystem;} //队伍子系统
	inline CFubenSystem& GetFubenSystem() { return m_fubenSystem;}				//副本子系统
	inline CMsgSystem& GetMsgSystem() { return m_msgSystem;}	//消息子系统
	inline CPkSystem& GetPkSystem() { return m_pkSystem;}	//pk子系统
	inline INT_PTR GetPropertySize() const {return sizeof(CActorProperty) -sizeof(CPropertySet);}
	inline CUserDepot&  GetDeportSystem() { return m_deportSystem;}	//仓库子系统
	inline CActorVarSubSystem& GetActorVarSystem(){ return m_varSystem; }//玩家变量系统
	inline CCLVariant& GetActorVar(){ return m_varSystem.GetVar(); }//获取玩家变量
	inline CAchieveSystem &GetAchieveSystem(){return m_achieveSystem;} //成就子系统
	inline CMiscSystem & GetMiscSystem() {return m_miscSystem;} //返回杂七杂八系统

	inline CTreasureSystem & GetTreasureSystem() {return m_treasureSystem;} //返回宝物

	inline CPetSystem& GetPetSystem(){return m_petSystem;} //宠物子系统
	//inline CStallSystem& GetStallSystem()	{ return m_stallSystem;}		//摆摊子系统
	inline CFriendSystem& GetFriendSystem() {return m_friendSystem;}		//好友子系统
	inline CHeroSystem& GetHeroSystem() {return m_heroSystem;}	//英雄子系统
	inline CUserEquipment& GetEquipmentSystem(){ return m_Equiptment; }//获取装备子系统
	inline CStaticCountSystem& GetStaticCountSystem() { return m_StaticCountSystem;} //获取计数器系统
	inline BasicFunctionsSystem& GetBasicFunctionsSystem() { return m_BasicFunctionSystem;} //获取计数器系统
	inline CUserBag& GetBagSystem(){ return m_Bag; }//获取背包子系统
	inline CActivitySystem& GetActivitySystem() { return m_activitySystem; }//获取活动系统
	
	inline CBossSystem & GetBossSystem() { return m_BossSystem;}				//boss系统
	inline CMailSystem & GetMailSystem() { return m_MailSystem;}				//邮件系统
	inline CCheckSpeedSystem& GetCheckSpeedSystem() {return m_CkSpeedSystem;}	//加速外挂子系统
	inline CNewTitleSystem& GetNewTitleSystem() {return m_NewTitleSystem;}//新头衔系统
	inline CLootPetSystem& GetLootPetSystem() {return m_LootPetSystem;}//宠物系统非 ai宠物
	inline CAlmirahSystem& GetAlmirahSystem() {return m_AlmirahSystem;} ///< 衣橱系统
	inline ConsignmentSystem& GetConsignmentSystem(){return m_ConsignmentSystem;}//寄卖
	inline CCombatSystem& GetCombatSystem() { return m_CombatSystem;}	//竞技子系统
	inline CHallowsSystem& GetHallowsSystem() { return m_nHallowsSystem;}	//圣物管理系统
	inline CCrossSystem& GetCrossSystem() { return m_nCCrossSystem;}	//圣物管理系统
	inline CReviveDurationSystem& GetReviveDurationSystem() { return m_ReviveDurationSystem;}	//复活特权系统
	inline CRebateSystem& GetRebateSystem() { return m_RebateSystem;}							//返利系统
	//获取实际头像图片编号（图片编号=头像*2+性别）
	inline int GetFaceImgIndex(){ return GetProperty<unsigned int>(PROP_ENTITY_ICON) * 2 + GetProperty<unsigned int>(PROP_ACTOR_SEX); }
 
	//防沉迷
	bool IsAdult(); 

	//检查于对象的距离是否满足给定的距离
	bool CheckTargetDistance(CActor *pTaget, int nDistance);
	
	//更新属性到好友服务器
	void UpdateFriendProperty();

	//添加飞鞋
	bool ChangeFlyShoesNum(int nValue);

	//添加喇叭数量
	bool ChangeBroatNum(int nValue);
	//改变积分
	bool ChangeActorRecyclePonit( int nValue );
	//改变声望
	bool ChangePrestige( int nValue ) ; 
	/*
	* Comments:初始化好友列表
	* Param CVector<int> & actorIdList: 好友actorid列表
	* @Return VOID:
	*/
	VOID InitFriendList(CVector<int> &actorIdList);

	/*
	* Comments: 好友变化
	* Param bool bAdd: 为true表面是新加了好友；否则是删除好友
	* Param int nActorId: 好友ActorID
	* @Return VOID:
	*/
	VOID OnFriendChanged(bool bAdd, unsigned int nActorId);

	/*
	* Comments: 获取好友列表
	* @Return CVector<int>&: 好友列表
	*/
	inline CVector<int>& GetFriendList() { return m_FriendList; }

	/*
	* Comments:得到角色当前正在对话的NPC
	* @Return : 无NPC则返回NULL
	*/
	CNpc* GetTalkNpc();
	/*
	* Comments: 获取发送缓冲,用于写入数据发送网络数据包，写完以后调用一下pack.flush()就写入发送缓冲
	*            典型的用法参考 SendData接口
	* Param CActorPacket & pack:局部的CActorPacket
	* @Return CDataPacket&: 返回pack里的datapack，可以直接写入数据
	*/
#ifdef _DEBUG
	CDataPacket& _AllocPacket(CActorPacket &pack, LPCSTR file, INT_PTR line);
#define AllocPacket(pack) _AllocPacket(pack, __FILE__, __LINE__)
#else
	CDataPacket& AllocPacket(CActorPacket &pack);

#endif
	CDataPacket& AllocPacketEx(CActorPacket &pack, BYTE nSubSysId, BYTE nMsgId);

	/* 
	* Comments: 初始化玩家数据包
	* Param CActorPacket & pack: 数据包
	* Param int nGateId:  网关的编号
	* Param int nGateSessionIndex: 网关的会话index
	* Param int nServerSessionIndex: 服务器的会话index
	* Param unsigned long long sock: socket
	* @Return CDataPacket: 数据包
	*/
	static CDataPacket& InitActorPacket(CActorPacket & pack,int nGateId, int nGateSessionIndex, int nServerSessionIndex,unsigned long long sock
		, LPCSTR file, INT_PTR line);

	void SendKeepAliveToGate();

	/*
	* Comments:获取玩家的门派
	* @Return INT_PTR: 返回门派的ID
	*/

	inline INT_PTR GetMenPai()
	{
		/*
		INT_PTR nVocation =m_property.nVocation; //职业
		switch(nVocation )
		{
		case enVocJianzong:
		case enVocQizong:
			return mpHuashan;
			
		case enVocLinghua:
		case enVocSiming:
			return mpShuiyuegong;
		
		case enVocYijin:
		case enVocXisui:
			return mpShaolin;
		
		case enVocJianshi:
		case enVocRenzhe:
			return mpYushenliu;
		
		default:
			return mpNoMenpai;
		}
		*/

		return 0;

	}

	/*
	* Comments: 获取角色上次下线那天获取的经验值
	* @Return unsigned int:
	*/
	inline unsigned int GetExpLastLogout()
	{
		return m_nExpGetLastLogout;
	}

	/*
	* Comments:给玩家的奖励
	* Param INT_PTR nType:奖励的类型
	* Param INT_PTR nId:奖励的ID 一般用于物品
	* Param INT_PTR nValue:奖励的数值
	* Param INT_PTR nLogId: 计入日志的ID
	* Param INT_PTR param:附加的参数，可以是整数也可以是指针
	* Param LPCTSTR pStr:如果需要存日志的话，用于存盘用的描述
	* Param int nQualityDataIndex:极品属性-库编号索引,1开始
	* @Return bool:
	*/
	bool GiveAward(INT_PTR nType,INT_PTR nId, INT_PTR nValue,
		INT_PTR nQuality=0, INT_PTR nStrong=0, INT_PTR nBindFlag=1, const INT_PTR param=0,INT_PTR nLogId=0,LPCTSTR pStr ="", int nQualityDataIndex = -1,const CUserItemContainer::ItemOPParam*  itemOPParam= nullptr);


	//GM用
	bool SetValueAward(int type, int value);
	//类型是否为金钱类型
	bool getMoneyType(int nType);

	void BroadRelive();

	/*
	* Comments:判断消耗值
	* Param INT_PTR nType:消耗的类型
	* Param int nId: 附加ID 如果是物品 则为物品ID 
	* Param int nValue:数量
	* Param int nStrong:强化值
	* Param int nQuality:品质
	* Param int nBindFlag:是否绑定,如果是-1表示绑定和非绑定都可以
	* Param INT_PTR param:附加的参数，可以是整数也可以是指针
	* @Return unsigned int:返回true消耗满足条件 false不满足
	*/
	bool CheckConsume(INT_PTR nType, INT_PTR nId, INT_PTR nValue,
		INT_PTR nQuality = -1, INT_PTR nStrong = -1, INT_PTR nBindFlag = -1, const INT_PTR nParam = 0);

	//获取消耗值
	Uint64 GetConsume(INT_PTR nType, INT_PTR nId, INT_PTR nValue = 0,
		INT_PTR nQuality = -1, INT_PTR nStrong = -1, INT_PTR nBindFlag = -1, const INT_PTR nParam = 0);
	/*
	* Comments:消耗
	* Param INT_PTR nType:消耗的类型
	* Param int nId: 附加ID 如果是物品 则为物品ID 
	* Param int nValue: 消耗值
	* Param int nQuality:品质
	* Param int nStrong:强化值
	* Param int nBindFlag:是否绑定,如果是-1表示绑定和非绑定都可以
	* Param INT_PTR param:附加的参数，可以是整数也可以是指针
	* Param INT_PTR nLogId: 计入日志的ID
	* Param LPCTSTR sComment:如果需要存日志的话，用于存盘用的描述
	* @Return INT_PTR:返回已消耗的值
	*/
	INT_PTR RemoveConsume(INT_PTR nType, INT_PTR nId, INT_PTR nValue,
		INT_PTR nQuality = -1, INT_PTR nStrong = -1, INT_PTR nBindFlag = -1,const INT_PTR nParam = 0,INT_PTR nLogId = 0,LPCTSTR sComment = "");

	
	/*
	* Comments:发送命令让客户端打开角色的属性窗口，主要用于脚本
	* Param int type:
	* param char：打开窗口所带的参数
	* @Return VOID:
	*/
	inline VOID OpenDialog(int type,const char* sParam = "")
	{
		if(sParam == NULL) 
		{
			OutputMsg(rmWaning,"OpenDialog string is NUll");
			return; 
		}
		CActorPacket pack;
		CDataPacket & data = AllocPacket(pack);
		data <<(BYTE)enDefaultEntitySystemID << (BYTE)sOpenDlg << (BYTE)type << (BYTE)0;
		data.writeString(sParam);
		pack.flush();
	}

	/*
	* Comments:发送命令让客户端关闭角色的属性窗口，主要用于脚本
	* Param int type:见tagOpenDialogID定义
	* @Return VOID:
	*/
	inline VOID CloseDialog(int type)
	{
		CActorPacket pack;
		CDataPacket & data = AllocPacket(pack);
		data <<(BYTE)enDefaultEntitySystemID << (BYTE)sOpenDlg << (BYTE)type << (BYTE)1;
		data.writeString("");
		pack.flush();
	}
	/*
	* Comments: 根据系统提示ID发送系统提示
	* Param INT_PTR nTipmsgID:系统提示ID
	* Param INT_PTR nType: 系统提示显示的类型
	* @Return void:
	*/
	void SendTipmsgWithId(INT_PTR nTipmsgID,INT_PTR nType = ttTipmsgWindow);
	void SendOldTipmsgWithId(INT_PTR nTipmsgID,INT_PTR nType = ttTipmsgWindow);

	/*
	* Comments: 根据系统提示ID定义，进行格式化后发送系统提示
	* Param INT_PTR nTipmsgID:系统提示ID
	* Param INT_PTR nType: 系统提示显示的类型
	* @Return void:
	*/
	void SendTipmsgFormatWithId(INT_PTR nTipmsgID,INT_PTR nType = ttTipmsgWindow, ...);
	void SendOldTipmsgFormatWithId(INT_PTR nTipmsgID,INT_PTR nType = ttTipmsgWindow, ...);
		
	/*
	* Comments:发送系统提示消息,自己组装好了，不需要组装就调用这个接口
	* Param LPCTSTR sTipmsg: 组装好的字符串
	* Param INT_PTR nType:  系统提示显示的类型
	* @Return VOID:
	*/
	inline VOID SendTipmsg(LPCTSTR sTipmsg,INT_PTR nType =ttTipmsgWindow)
	{
		if(sTipmsg ==NULL || strcmp(sTipmsg,"")==0 || IsInited() ==false) return;
		
		if(OnGetIsSimulator()
			|| OnGetIsTestSimulator())
		{ 
			return;
		}
		CActorPacket pack;
		CDataPacket & data = AllocPacket(pack);
		data <<(BYTE)enChatSystemID << (BYTE)sSendTipmsg << (WORD) nType;
		data.writeString(sTipmsg);
		pack.flush();
	}

	/*
	* Comments:具有参数格式化功能的发送系统提示消息
	* Param LPCTSTR sTipmsg: 组装好的字符串
	* Param INT_PTR nType:  系统提示显示的类型
	* @Return VOID:
	*/
	inline VOID  SendTipmsgFormat(LPCTSTR sFormat,INT_PTR nType =ttTipmsgWindow, ...)
	{
		va_list args;
		va_start(args, nType);
		SendTipmsgWithArgs(sFormat, args, nType);
		va_end(args);
	}

	/*
	* Comments:向客户端发送公共操作的结果
	* Param bool flag:成功或者失败
	* Param unsigned int nStep:对于移动操作，表示移动步长；否则为0
	* Param unsigned int nSendPkgTick:客户端发送数据包的时刻
	* Param bool bNeedBreakAction:是否需要打断客户端的动作，true表示需要打断动作,false表示不需要打断动作
	* @Return VOID:
	*/
	VOID SendOperateResult(bool flag, unsigned int nStep = 0, unsigned int nSendPkgTick = 0,bool bNeedBreakAction =true);

	/* 
	* Comments:能够实现参数格式化功能的发送系统提示消息
	* Param LPCTSTR sFormat: 消息格式
	* Param INT_PTR nType:  系统提示显示的类型
	* @Return VOID:
	*/
	inline VOID SendTipmsgWithArgs(LPCTSTR sFormat, va_list &args, INT_PTR nType);


	/*
	* Comments:向玩家发送数据，这个接口一般给广播使用，玩家一般的数据包请使用AllocPacket分配然后发送
	* Param char * pData: 数据指针
	* Param SIZE_T size: 数据大小
	* @Return VOID:
	*/
	VOID  SendData(LPCVOID pData, SIZE_T size)
	{
		if(IsInited() == false) 
		{
			return;
		}
		if (m_IsTestSimulator || m_IsSimulator)
		{
			return;
		}
		
		CActorPacket pack;
		CDataPacket & data = AllocPacket(pack);
		data.writeBuf(pData,size);
		pack.flush();
	}

	/*
	/* Comments: 玩家登陆
	/* Param UINT_PTR nSocket: socketID
	/* Param UINT_PTR nActorID: 玩家的角色ID
	/* Param UINT_PTR nGateSessionID: 在网关上的sessionID
	/* Param UINT_PTR nGateIndex: 网关编号
	/* Param UINT_PTR nServerSessionId: 在服务器上的sessionID
	/* Param UINT_PTR nAccountID:玩家的帐户ID
	/* Param INT_PTR  nFcmTime: 防沉迷的时间
	/* Param INT_PTR  nGmLevel: 玩家的GM等级
	/* Return void: 
	*/
	void Onlogin(UINT_PTR nSocket,UINT_PTR nActorID,
		UINT_PTR nGateSessionID, UINT_PTR nGateIndex,
		UINT_PTR nServerSessionId,UINT_PTR nAccountID,
		INT_PTR  nFcmTime,INT_PTR nGmLevel, int nRawServerId, LONGLONG nClientIpAddr);

	//服务器登录//不需要绑定socket
	void OnServerLogin(UINT_PTR nActorID, UINT_PTR nAccountID,
		INT_PTR  nFcmTime, INT_PTR nGmLevel, int nRawServerId, LONGLONG nClientIpAddr);
 
	//是否isGatewayLogout是否是网关关闭了玩家(正常关闭)，false表示是掉线网关关闭(正常关闭)
	VOID CloseActor(int nWayId, bool isGatewayLogout= false, int nLogoutServerId = 0, bool bDestroyEntiy = true, bool bNeedDBAck = false); // 关闭连接

	VOID LogicRun(TICKCOUNT nCurrentTime);


	// bLogout: 为true表明是下线、否则为定时存盘 
	// nLogoutServerI: 如果为0，上次登录的服务器Id就取当前逻辑服务器的Id；否则取此值
	// bNeedDBAck: 是否需要DB应答
	VOID SaveDb(bool bLogout = false, int nLogoutServerId = 0, bool bNeedDBAck = false); //存盘

	VOID SaveBasicData();

	/*
	* Comments: 接收到来自DB的Logout消息应答
	* @Return void:
	* @Remark: 角色在下线或者传送的时候会先保存db数据，发送dcLogout消息。在传送时要求DB会应答消息
	*/
	void OnActorLogoutAck();

	/*
	* Comments: 请求传送到指定的逻辑服务器
	* Param int nDestServerId:
	* Param enTransmitType transType:
	* @Return void:
	* @Remark:
	*/
	void RequestTransmitTo(int nDestServerId, enTransmitType transType = enTransmitInGame);

	/*
	* Comments:检查等级是否满足
	* Param int nMinLevel	:	最小等级
	* Param int nMinCircle	:	转生
	* Param int bCircleOnly	:	当转生大于时，是否只判断转生
	* @Return bool:判断成功返回true
	*/
	bool CheckLevel( int nMinLevel, int nMinCircle, bool bCircleOnly=false);

	/*
	* Comments:检查行会等级是否满足
	* Param int nMinLevel	:	最小行会等级
	* @Return bool:判断成功返回true
	*/
	bool CheckGuildLevel( int nMinLevel);

	/*
	* Comments:检查充值金额是否满足
	* Param int nMinLevel	:	最小充值额度
	* @Return bool:判断成功返回true
	*/
	bool CheckRecharge( int nMinRecharge);

	/*
	* Comments:检查官职是否满足
	* Param int nofficeId	:	官职id
	* @Return bool:判断成功返回true
	*/
	bool CheckOffice( int nofficeId = 0);

	/*
	* Comments:检查神魔等级是否满足
	* Param int nGhostLevel	:	神魔等级
	* @Return bool:判断成功返回true
	*/
	bool CheckGhostLevel( int nGhostLevel = 0);

	//获取玩家等级
	int GetLevel();	
	//获取玩家转生等级
	int GetCircleLevel();
	inline int GetOfficeLv()
	{
		return GetProperty<int>(PROP_ACTOR_OFFICE);
	}
	//检查活动/系统等的开启等级
	bool CheckOpenLevel(int nLevelConfigId);
	//能否传送到目标的服务器
	bool CanTransMit(int nDesServerId);
		
	/*
	* Comments: 接收到来自Session服务器的传送确认消息
	* Param CDataPacketReader & inPacket:
	* @Return void:
	* @Remark: 消息中携带传送目的服务器ServerId以及传送错误码（是否可以传送）
	*/
	void OnRecvTransmitToAck(CDataPacketReader &inPacket);

	/*
	* Comments: 判断玩家是否是在传送状态
	* @Return bool:
	* @Remark:
	*/
	bool IsInTransmitStatus() const { return m_nTransmitStatus != enTS_IDLE; }
		
	/*
	* Comments: 判断玩家是否在原始服务器
	* @Return bool: 如果玩家当前的服务器是原始服务器，返回true；否则返回false
	* @Remark:
	*/
	bool IsInRawServer() const;

	//获取玩家的帐户ID
	inline unsigned int GetAccountID(){return m_nAccountID;}

	//获取玩家的账户字符串
	inline LPCTSTR GetAccount(){ return m_sAccount; }
	
	/*
	* Comments: 向客户端发送心跳包
	* @Return void: 
	*/
	inline void SendHeartBeatPack()
	{
		CActorPacket pack;
		CDataPacket & data = AllocPacket(pack);
		data << (BYTE) enDefaultEntitySystemID << (BYTE) sHeartbeat;
		pack.flush();
	}
	
	//收到网络数据包
	VOID PostNetMsg(CDataPacket * data,INT_PTR nSize);
	
	/*
	* Comments: 如果消息中没有指定子系统ID，也就是nSystemID=0,那么将在这里进行处理
	* Param INT_PTR nCmd: 命令，1字节的
	* Param CDataPacket & pack: 数据
	* @Return void:
	*/
	void ProcessDefaultNetData( INT_PTR nCmd,CDataPacket &pack );
	
	
	/*
	* Comments:处理网络消息
	* Param INT_PTR nSystemID:子系统的ID,1个字节的
	* Param INT_PTR nCmd: 命令码，1个字节的
	* Param CDataPacket & pack: 数据包，读取了CMD命令后的数据
	* @Return void:
	*/
	void HandleNetworkData(INT_PTR nSystemID, INT_PTR nCmd,CDataPacket &pack); 
	

	//重设玩家的显示名字
	void ResetShowName( );


	/*
	* Comments: 玩家进入一个场景的时候向客户端发进入场景的消息
	* @Return void:
	*/
	void OnEnterScene(int nType = 0, int nValue = 0, int nParam=-1);
	
	
	/*
	* Comments: 在玩家离开一个场景前调用
	* @Return void:
	*/
	void OnPreExitScene();
	
	//设置在某个区域的标志
	void SetSceneAreaFlag(int nFlag);

	//返回是否在某个区域的标志
	int GetSceneAreaFlag()
	{
		return m_nEnterSceneFlag;
	}
	//设置翅膀档次
	void SetSwingLevel(int nValue) {m_nSwingLevel = nValue;}

	//获取翅膀档次
	int GetSwingLevel() {return m_nSwingLevel;}

	//设置翅膀id
	void SetSwingId(int nValue) {m_nSwingid = nValue;}

	//获取翅膀id
	int GetSwingId() {return m_nSwingid;}

	//获取登陆天数
	int GetLoginDaysValue() {return m_LonginDays;}

	//获取消耗元宝
	int GetConsumeYb() {return m_consumeyb;}

	/*
	* Comments:有人查看你的装备/宠物...
	* Param Actor * pActor:查看人的指针
	* @Return void:
	*/
	void PlayerViewNotice( CEntity * pActor);

	/*
	* Comments: 增加经验，这里要处理升级的处理
	* Param INT_PTR nExpValue: 增加经验的值
	* Param INT_PTR nExpWay: 经验的来源,比如任务，排名，工会等
	* Param INT_PTR nParam   如果是任务，这个就填写任务的ID，其他的话填关键的有意义的参数，如果没有就填写0
	* Param int nType: 广播类型 1为周围玩家广播 2自己广播
	* Param INT_PTR nParam2: 怪物转生等级
	* @Return void:
	*/
	void AddExp(INT_PTR nExpValue,INT_PTR nExpWay =0,INT_PTR nParam =0,  INT_PTR nType = 0, INT_PTR nParam2 =0);

	//预先检测添加经验后能到多少级
	int  AddExpReachLevel(INT_PTR nExpValue);

	/* 
	* Comments:改变活跃度
	* Param int nValue:
	* @Return bool:
	*/
	bool ChangeActivity(int nValue);

	/* 
	* Comments:改变转生灵力
	* Param int nValue:
	* @Return bool:
	*/
	bool ChangeCircleSoul(int nValue);

	/*
	* Comments: 消耗经验
	* Param INT_PTR nExpValue: 经验的值
	* @Return bool: 成功返回true，否则返回false
	*/
	inline bool DecExp(INT_PTR nExpValue)
	{
		if( (INT64)m_property.lExp < nExpValue) return false;
		
		//这里要处理升级判断
		SetProperty<Uint64>(PROP_ACTOR_EXP, m_property.lExp-nExpValue );
		return true;
	}
	//获取充值的RMB
	inline int GetRechargeNum() {
		return GetProperty<unsigned int>(PROP_ACTOR_DRAW_YB_COUNT); //玩家提取的元宝的数目
	}
	//职业
	inline int GetJob() {
		return GetProperty<unsigned>(PROP_ACTOR_VOCATION);
	}

	//行别
	inline int GetSex() {
		return GetProperty<unsigned>(PROP_ACTOR_SEX);
	}

	void OnFeeCmd(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);

	/*
	* Comments: 修改玩家的不可交易的金钱
	* Param INT_PTR nMentyType: 金钱的类型
	* Param INT_PTR nMoneyValue: 金钱的值
	* Param INT_PTR nWay:途径，比如任务，排名，工会等
	* Param INT_PTR nCount:消费的相关数量，通常是元宝消费购买物品
	* Param char* sComment: 备注，比如技能消耗里记录技能的ID等
	* Param bool needLog:是否需要记录日志
	bNotice 是否立即提示给前端
	* @Return bool: 成功返回true，否则返回false
	*/
	bool ChangeMoney(INT_PTR nMentyType,INT_PTR nMoneyValue,INT_PTR nWay ,INT_PTR nCount,LPCSTR sComment ,bool needLog=true, bool boForceLog = false,bool bNotice = true );

	/*
	* Comments: 转换职业
	* Param int nVocation: 职业ID
	*/
	void ChangeVocation(int nVocation);
	void ChangeSex(int nSex);

	void UseChangeVocationCard(int nVoc,int nSex);
	
	/*
	* Comments: 获取特定金钱的数量
	* Param INT_PTR nMentyType: 金钱的类型，定义如下
		//金钱的类型的定义
		enum eMoneyType
		{
		mtBindCoin =0,			//可交易的金钱，如任务等发送的金钱
		mtNonBindCoin=1,	    //不可交易的金钱，比如系统奖励的一些金钱
		mtBindYuanbao =2,		//可交易的元宝，玩家充值兑换的
		mtNonBindYuanbao=3,		//不可交易的元宝，一般是系统奖励的
		};
	* bBindAndUnBind:是否绑定+非绑一起算
	* @Return INT_PTR: 返回金钱的数量

	注意：在ZGame中，取mtNonBindYuanbao的数量，必须返回绑定元宝+元宝的数量
	扣除绑定元宝，必须先够绑元，不走再扣元宝；扣除元宝则不可逆
	*/
	inline unsigned int GetMoneyCount(eMoneyType nMentyType, bool bBindAndUnBind=false)
	{	
		switch(nMentyType)
		{
		case mtBindCoin:
			return m_property.nBindCoin;
			break;
		case mtCoin:
			return m_property.nNonBindCoin;
			break;
		case mtBindYuanbao:			//绑定元宝
            //绑元+元宝一起计算, 注意: 检查时，bBindAndUnBind 要为true, 消耗时为false
			if(bBindAndUnBind)
			{
			     return m_property.nBindYuanbao+m_property.nNonBindYuanbao;
			}else
			{
				 return m_property.nBindYuanbao;
			}
			break;
		case mtYuanbao:
			return m_property.nNonBindYuanbao;
			break;
		case mtWingSoul:		//羽魂
			return m_property.nWingPoint;
			break;
		case mtBossJiFen://boss积分
			return m_property.nPersonBossJifen;
			break;
		case mtFlyShoes://飞鞋
			return m_property.nFlyShoes;
			break;
		case mtJyQuota: //交易额度
			return m_property.nTradingQuota;
			break;
		default: 
			return 0;
		}
	}

	/*获取玩家拥有的奖励类型的数量
	包括货币、装备、物品
	*/
	 unsigned int GetAwardTypeCount( int nAwardType, INT_PTR nId, bool bBindAndUnBind=false );

	/*
	* Comments:修改绑定的钱（不可交易的金钱）
	* Param INT_PTR nMoneyValue:值
	* Param INT_PTR nWay::途径，比如任务，排名，工会等
	* Param char* sComment: 备注，比如技能消耗里记录技能的ID等
	* Param bool needLog:是否需要记录日志
	* @Return bool: 成功返回true，否则返回false
	*/
	inline bool ChangeBindCoin(INT_PTR nMoneyValue,INT_PTR nWay,INT_PTR nCount,LPCSTR sComment,bool needLog=true)
	{
		return ChangeMoney( mtBindCoin,nMoneyValue,nWay,nCount,sComment,needLog);
	}

	/*
	* Comments:修改非绑定的钱（可以交易的金钱）
	* Param INT_PTR nMoneyValue:值
	* Param INT_PTR nWay::途径，比如任务，排名，工会等
	* Param char* sComment: 备注，比如技能消耗里记录技能的ID等
	* Param bool needLog:是否需要记录日志
	* @Return bool:成功返回true，否则返回false
	*/
	inline bool ChangeCoin(INT_PTR nMoneyValue,INT_PTR nWay ,INT_PTR nCount,LPCSTR sComment,bool needLog=true)
	{
		return ChangeMoney( mtCoin,nMoneyValue,nWay,nCount,sComment,needLog);
	}
	//飞鞋
	inline bool ChangeFlyShoes(INT_PTR nMoneyValue, INT_PTR nWay, INT_PTR nCount, LPCSTR sComment, bool needLog = true)
	{
		return ChangeMoney( mtFlyShoes, nMoneyValue, nWay, nCount, sComment,needLog );
	}
	//交易额度
	inline bool ChangeJyQuota(INT_PTR nMoneyValue, INT_PTR nWay, INT_PTR nCount, LPCSTR sComment, bool needLog = true)
	{
		return ChangeMoney( mtJyQuota, nMoneyValue, nWay, nCount, sComment,needLog );
	}

	/*
	* Comments:修改绑定的元宝的数量(不可交易元宝)
	* Param INT_PTR nMoneyValue:值
	* Param INT_PTR nWay::途径，比如任务，排名，工会等
	* Param char* sComment: 备注，比如技能消耗里记录技能的ID等
	* Param bool needLog:是否需要记录日志
	* @Return bool: 成功返回true，否则返回false
	*/
	inline bool ChangeBindYuanbao(INT_PTR nMoneyValue,INT_PTR nWay ,INT_PTR nCount,LPCSTR sComment,bool needLog=true)
	{
		//扣元宝和绑定元宝，要走替代流程
		if( nMoneyValue < 0 )
		{
			if( DeleteBindYuanbaoAndYuanbao( -1*nMoneyValue, (WORD)nWay,(int)nCount, sComment,needLog) < 0 )  //扣除失败
			{
				return  false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return ChangeMoney( mtBindYuanbao,nMoneyValue,nWay,nCount,sComment,needLog);
		}
	}

		/*
	* Comments:是否有这么多银两，绑定银两 + 非绑定银两
	* Param INT_PTR nCount: 银两的数量
	* @Return bool: 有足够的返回true，否则返回false
	*/
	inline bool HasEnoughCoin(INT_PTR nCount)
	{
		if(nCount < 0) return false;
		return (INT_PTR)m_property.nNonBindCoin >= nCount;
	}

		/*
	* Comments:是否有这么多绑定银两 
	* Param INT_PTR nCount: 绑定银两 的数量
	* @Return bool: 有足够的返回true，否则返回false
	*/
	inline bool HasEnoughBindCoin(INT_PTR nCount)
	{
		if(nCount < 0) return false;
		return (INT_PTR)m_property.nBindCoin >= nCount;
	}


	/*
	* Comments:改变帮派共吸纳
	* Param int nValue:改变的数值，整数表示增加，负数表示减少
	* Param int nLogId:改变的logid
	* @Return bool:成功返回true，否则返回false
	*/
	bool ChangeGuildContri(int nValue,int nLogId=0);



	/*
	* Comments:消耗银两(绑定的，非绑定的都可以)
	* Param int64 nCount:消耗的数目，是正数，比如消耗100银两就是传入100
	* Param int nlogId:日志的od
	* Param LPCTSTR comment:日志里的备注
	* Param bool needLog: 是否需要日志
	* Param bool bindFirst: true表示绑定的优先消耗，否则表示非绑定的优先消耗
	* @Return bool:成功消耗返回true，否则不够的话不会扣除并返回false
	*/
	bool ConsumeCoin(INT_PTR nCount,int nlogId, LPCTSTR comment,bool needLog= true,bool bindFirst =true);


	/*
	* Comments:修改非绑定的元宝(可交易元宝)
	* Param INT_PTR nMoneyValue:值
	* Param INT_PTR nWay::途径，比如任务，排名，工会等
	* Param char* sComment: 备注，比如技能消耗里记录技能的ID等
	* Param bool needLog:是否需要记录日志
	* @Return bool: 成功返回true，否则返回false
	*/
	inline bool ChangeYuanbao(INT_PTR nMoneyValue,INT_PTR nWay,INT_PTR nCount,LPCSTR sComment,bool needLog=true)
	{
		return ChangeMoney( mtYuanbao,nMoneyValue,nWay,nCount,sComment,needLog);
	}

	/*
	* Comments:与NPC交谈以及执行脚本
	* Param EntityHandle & NpcHandle:
	* Param LPCTSTR sFunctionName:脚本要执行的函数名和参数信息
	* @Return VOID:
	*/
	//VOID NpcTalk(EntityHandle& NpcHandle,LPCTSTR sFunctionName);

	/*
	* Comments:新版与npc交谈接口
	* Param EntityHandle & hNpcHandle:
	* Param LPCTSTR sFuncName:
	* @Return void:
	*/
	void OnNpcTalk(const EntityHandle & hNpcHandle, short nFuncId, CDataPacket &pack);

	/*
	* Comments:本函数用于当事件发生时调用的函数，函数内会调用全局的脚本执行
	* Param int CScriptValueList & paramList:参数列表
	* Param int CScriptValueList & retParamList:返回参数列表
	* @Return VOID:
	*/
	VOID OnEvent(int nEntityType, CScriptValueList & paramList,CScriptValueList & retParamList);
	

	/* 
	* Comments:使用物品时，物品npc去处理回调
	* Param CScriptValueList & paramList:参数列表
	* Param CScriptValueList & retParamList:返回参数列表
	* @Return VOID:
	*/
	VOID OnItemEvent(CScriptValueList & paramList, CScriptValueList & retParamList);

	/*
	* Comments:本函数用于调用脚本的保存数据（luahlp）的函数，类似OnEvent函数
	* Param int nDataId:要保存的数据的id，对应tagDataEventID的定义
	* Param T * pParaList:要保存的数据内容
	* Param int nCount:pParaList的长度
	* @Return VOID:
	*/
	/*
	template<typename T>
	VOID OnSaveData(int nDataId,T* pParaList,int nCount)
	{
		CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		if (pNpc == NULL) return;
		CScriptValueList paramList;
		paramList << nDataId;
		paramList << GetProperty<unsigned int>(PROP_ENTITY_ID);
		//if( ! pNpc->GetScript().Call<T>("OnSaveData",paramList,pParaList,nCount,paramList ) )
		//{
		//	//脚本错误，将以模态对话框的形式呈现给客户端
		//	const RefString &s = pNpc->GetScript().getLastErrorDesc();
		//	SendTipmsg((LPCSTR)s,ttDialog);
		//}

	}
	*/

	/*
	* Comments:读取脚本的数据
	* Param int nDataId:数据id，对应tagDataEventID的定义
	* Param T * pParaList:
	* Param int & nCount:
	* @Return VOID:
	*/
	/*
	template<typename T>
	VOID OnLoadData(int nDataId,T* pParaList,size_t nCount,size_t& nRetCount)
	{
		CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		if (pNpc == NULL) return;
		CScriptValueList paramList;
		paramList << nDataId;
		paramList << GetProperty<unsigned int>(PROP_ENTITY_ID);
	}
	*/

	/*
	* Comments: 初始化一个事件调用的CScriptValueList，主要
	  把自己的指针和事件的指针压入
	* Param CScriptValueList & paramList: 参数列表
	* Param INT_PTR nEventID: 事件ID
	* @Return void:
	*/
	inline void InitEventScriptVaueList(CScriptValueList &paramList,INT_PTR nEventID )
	{
		paramList << this;
		paramList << nEventID;
	}
	/*
	* Comments: 获取一个社会关系的位
	* Param INT_PTR nMaskID: maskID,定义在SystemParamDef.h里，如
		enum tagSocialMaskDef
		{
		smGuildCommon=0,    //帮会普通成员
		smGuildTangzhu=1,    //堂主
		smGuildAssistLeader=2, //副帮主
		smGuildLeader=3,     //帮主
		};
	* @Return bool: 如果设置了就返回true,否则返回false
	*/
	inline bool GetSocialMask(INT_PTR nMaskID)
	{
		return (GetProperty<unsigned int >(PROP_ACTOR_SOCIALMASK) & (1 << nMaskID) )? true:false;
	}

	/*
	* Comments: 设置社会关系的位
	* Param INT_PTR nMaskID: maskID，同上
	* Param bool flag:如果设置是就传入true，否则传入false
	* @Return void:
	*/
	inline bool SetSocialMask(INT_PTR nMaskID,bool flag)
	{
		unsigned int nMask = GetProperty<unsigned int >(PROP_ACTOR_SOCIALMASK);
		if(flag)
		{
			nMask |= (1<< nMaskID);
		}
		else
		{
			nMask &= ~( 1<< nMaskID ); //取反
		}
		SetProperty<unsigned int >(PROP_ACTOR_SOCIALMASK,nMask );

		return true;
	}

	inline void SetWholeAwards(int nMaskId, bool flag = true)
	{
		if (flag)
		{
			m_nWholeAwards |= (1 << nMaskId);
		}
		else
		{
			m_nWholeAwards &= ~(1 << nMaskId);
		}
	}

	byte GetWholeAwards(int nMaskId)
	{
		if((m_nWholeAwards & (1 << nMaskId)) > 0)
		{
			return 1;
		}
		return 0;
	}

	int  GetStaticVarValue(LPCTSTR szVarName);					//获取静态变量的值

	void SetStaticVarValue(LPCTSTR szVarName, int nValue);		//设置静态变量的值

	/* 
	* Comments:设置十天活动标记
	* Param int nFlag:
	* @Return void:
	*/
	void SetWholeAwardsFlag(int nFlag)
	{
		m_nWholeAwards = nFlag;
	}

	/* 
	* Comments:获取十天活动标记
	* @Return int:
	*/
	int GetWholeAwardsFlag()
	{
		return m_nWholeAwards;
	}

	/*
	* Comments: 获取一个玩家的队伍的指针
	* @Return CTeam *: 队伍的指针，如果不在队伍里，或者队伍已经被删除，那么就返回NULL
	*/
	CTeam * GetTeam();

	/*
	* Comments: 获取上次的登出时间
	* @Return unsigned int: 返回登出时间
	*/
	inline unsigned int GetLastLogoutTime()
	{
		return m_nLastLogOutTime;
	}



	/*
	* Comments: 获取本次的登录时间
	* @Return unsigned int: 返回登录时间
	*/
	inline unsigned int GetLoginTime()
	{
		return m_nLoginTime;
	}
	void SetLoginTime(unsigned int t)
	{
		m_nLoginTime.tv = t;
	}
	inline unsigned int GetCreateTime()
	{
		return m_nCeateTime;
	}
	//获取CActorPacket
	inline CActorPacket & GetFastPacket()
	{
		return m_FastPacket;
	}
	
	/*
	* Comments:让角色退出副本，回到常规场景，位置是进入副本前保存的常规场景的位置，
	* 如果角色已经在常规场景中，则不做处理
	* @Return void:
	*/
	void ReturnToStaticScene(int nSceneId=-1, int nPosX=-1, int nPosY=-1);

		/*
	* Comments:flyshose ,进入特定场景特定位置
	* @Return void:
	*/
	bool FlyShoseToStaticScene(INT_PTR nScenceID,INT_PTR nPosX, INT_PTR nPosY, INT_PTR nWeight = 0, INT_PTR nHeight = 0, int nType = 0,
		int nValue = 0, int nParam=-1);

	/*
	* Comments:增加一个消息框并且发送到客户端
	* Param EntityHandle hNpc:将执行这些脚本的npc,如果为0，则使用全局的npc
	* Param UINT nActorId:接收这个消息的角色id，如果是0，则表示发送给自己
	* Param char* title:对话框的标题
	* Param char * * sFnName:脚本（按钮）的内容，文本内容为包含了按钮要显示的文字，以及点击后要执行的函数名，中间用 “/”隔开，比如"确定/commonAcceptMissions,1"
	//其中“确定”是客户端要显示的按钮的文字，commonAcceptMissions,1是要执行的脚本函数和参数
	* Param INT_PTR nButtonCount:按钮的数量
	* Param unsigned int nTimeOut:对话框存在的时间
	* Param int msgType:0:直接弹出	1：停在屏幕右侧，用户点击才弹出
	* Param int nTimeOutBtn:对话框倒计时结束执行按钮ID 第1个按钮 1,第2个按钮 2....
	* Param char*sTip:鼠标移上去的tip，弹出类型是1才有用
	* Param WORD wIcon:图标ID
	* Param int nTimeOutBtn:倒数时间结束默认执行的按钮ID
	* Param int msgId:消息ID。一般为角色ID。用于msgType=1时叠加同个消息 为0时客户端自动分配 
	* Param int showId:是否弹出提示框系统ID 在SystemParamDef.h(enum tagMsgBoxShowId)定义
	* @Return int:返回错误码
	*/
	int AddAndSendMessageBox(EntityHandle hNpc,UINT nActorId,const char* sTitle,char sFnName[][MAX_MSG_COUNT],INT_PTR nButtonCount,
		unsigned int nTimeOut,int msgType,const char* sTip,WORD wIcon=0,int nTimeOutBtn=0, int msgId = 0, int showId=mbsDefault);

	/* 
	* Comments:提示购买物品的窗口
	* Param EntityHandle hNpc:npc句柄
	* Param WORD nItemId:物品id
	* Param byte nCount:需要的物品数量
	* Param LPCSTR sNotice:提示信息
	* Param LPCSTR sCallFunc:回调函数
	* @Return bool:
	*/
	bool AddBuyItemDialog(EntityHandle hNpc, WORD nItemId, byte nCount, LPCSTR sNotice, LPCSTR sDesc, LPCSTR sCallFunc);

	/*
	* Comments:处理玩家离线的消息通知
	* Param INT_PTR nMsgType:消息类型
	* Param char * pMsgBuf:消息数据的缓冲区
	* Param SIZE_T size:数据缓冲区的大小
	* @Return bool:是否处理成功，如果成功，这条信息将会被删除
	*/
	bool ProcessOfflineMsg(INT_PTR nMsgType,char* pMsgBuf,SIZE_T size, INT64 nMsgId);

	/*
	* Comments:给玩家发送离线的文字信息
	* Param BYTE nMsgType:消息类型，对应tagTipmsgType
	* Param char * sMsg:文字内容,UTF编码，以0结尾
	* Param INT_PTR nSrcActorId:发出这条消息的角色id，比如GM，当添加消息不成功，比如角色名称错误，需要通知GM,如果是0，表示系统发送
	* @Return bool:是否成功，即AddOfflineMsg的返回值
	*/
	static bool AddTxtMsg(unsigned int nActorId,const char* sMsg,unsigned int nSrcActorId);

	static bool AddTxtMsgByActorName(const char *pActorName, const char *sMsg,unsigned int nSrcActorId);

	static bool AddGmTxtMsgByActorName(const char *pActorName, const char *sMsg,unsigned int nSrcActorId);

	static bool AddPaTaAwardMsgByName( const char * pActorName, const char * sMsg,const char *sTitle, byte bType,int nFubenId, int nDeadline);
	
	static bool AddCommonMsgByActorName(int nMsgId, const char *pActorName, const char *sMsg,unsigned int nSrcActorId, int nParam = 0, int nParam2=0,const char * sParam = NULL);

	static bool AddFestivalMsgByActorName(int nMsgId, LPCSTR pActorName, LPCSTR sMsg, unsigned int nSrcId,LPCSTR sParam1 = NULL, LPCSTR sParam2 =NULL, int nParam1 = 0, int nParam2 = 0);
	/*
	* Comments:改变玩家的金钱
	* Param INT_PTR nActorId:目标角色
	* Param LPCSTR sTitle:显示在客户端的标题文字
	* Param LPCSTR sBtnTxt:显示在客户端的按钮文字
	* Param INT_PTR nMoneyType:金钱类型，见eMoneyType的定义
	* Param INT_PTR nMoneyValue:金钱的数量，如果是负数，表示是减少的
	* Param INT_PTR nWay:来源，见GameLog的定义
	* Param LPCSTR sComment:备注，比如技能消耗里记录技能的ID等
	* Param bool needLog:是否需要记录日志
	* @Return bool:
	*/
	static bool AddChangeMoneyMsg(unsigned int nActorId,unsigned int nSrcActorId,LPCSTR sTitle,LPCSTR sBtnTxt,BYTE nMoneyType,int nMoneyValue,
		int nWay =0,LPCSTR sComment ="",bool needLog=true );

	static bool AddChangeMoneyMsgByName(LPCSTR sActorName,unsigned int nSrcActorId,LPCSTR sTitle,LPCSTR sBtnTxt,BYTE nMoneyType,int nMoneyValue,
		int nWay =0,LPCSTR sComment ="",bool needLog=true );

	/*
	* Comments:给玩家发离线信息，信息发物品
	* Param LPCSTR sActorName:目的玩家的名字
	* Param INT_PTR nSrcActorId:发送的玩家，通常是GM，如果是系统程序发的，则为0
	* Param CUserItem * pUserItem:物品
	* Param LPCSTR sTitle:给玩家看的文本
	* Param LPCSTR sBtnTxt:按钮的文本
	* Param INT_PTR nWay:来源，见GameLog的定义
	* Param LPCSTR sComment:备注，比如GM的名字
	* Param bool needLog:是否记录日志
	* @Return bool:
	*/
	static bool AddItemMsgByName(LPCSTR sActorName, unsigned int nSrcActorId, const CUserItemContainer::ItemOPParam& param, LPCSTR sTitle,LPCSTR sBtnTxt,INT_PTR nWay, LPCSTR sComment ="",bool needLog=true);

	/*
	* Comments:给玩家发离线信息，信息发物品
	* Param INT_PTR nActorId:目标角色ID
	* Param INT_PTR nSrcActorId:发送的玩家，通常是GM，如果是系统程序发的，则为0
	* Param CUserItem * pUserItem:物品
	* Param LPCSTR sTitle:给玩家看的文本
	* Param LPCSTR sBtnTxt:按钮的文本
	* Param INT_PTR nWay:来源，见GameLog的定义
	* Param LPCSTR sComment:备注，比如GM的名字
	* Param bool needLog:是否记录日志
	* Param INT_PTR nMsgType:消息类型 (必须为添加道具的消息，且在ProcessOfflineMsg必须做相应处理)
	* Param INT_PTR nParam:额外参数
	* @Return bool:
	*/
	static bool AddItemMsg(unsigned int nActorId, unsigned int nSrcActorId, const CUserItemContainer::ItemOPParam& param, LPCSTR sTitle,
		LPCSTR sBtnTxt,INT_PTR nWay, LPCSTR sComment ="",bool needLog=true, INT_PTR nMsgType = CMsgSystem::mtAddIndexItem, INT_PTR nParam = 0);
	
	//完成一个初始化步骤，玩家需要自身的数据初始化，装备的初始化和技能的初始化才下发创建
	inline void OnFinishOneInitStep(int step); // 被调用次数应绝对等于m_wInitLeftStep
	
	virtual void OnEntityDeath();

	virtual void OnLevelUp(int nUpdateVal);

	virtual void OnEntityScriptDelayMsgCallback(unsigned int nMsgIdx, int nParam1, int nParam2, int nParam3, int nParam4, int nParam5);

	INT_PTR removeEntityScriptDelayMsg(unsigned int nMsgIdx);

	//成功击中了
	virtual void OnAttackOther(CAnimal * pEntity, bool bChgDura=true);

	//被别人攻击 bSetVest:是否设置归属
	virtual void OnAttacked(CAnimal * pEntity, bool bSetVest=true); //被攻击

	//当攻击速度和移动速度变化的时候，要重置一下反外挂的设置
	void ResetSpeedTime();

	/*
	* Comments:设置回城点
	* Param int nPosX:x坐标
	* Param int nPosY:y坐标
	* Param unsigned int nSceneId:场景id
	* @Return void:
	*/
	void SetCityPoint(int nPosX,int nPosY,unsigned int nSceneId);

	/*
	* Comments:设置复活点
	* Param int nPosX:x坐标
	* Param int nPosY:y坐标
	* Param unsigned int nSceneId:场景id
	* Param bool boFb:是否副本
	* @Return void:
	*/
	void SetRelivePoint(int nPosX,int nPosY,unsigned int nSceneId,bool boFb=false);

	/*
	* Comments:回城
	* @Return void:
	*/
	void ReturnCity();

	/*
	* Comments:复活！如果现在在普通场景，则复活到上一个普通场景的复活点，否则复活在副本里
	* param isReturn 是否强制退出副本
	* @Return void:
	*/
	void Relive(bool isReturn);
	
	/*
	* Comments:复活时候触发事件
	* @Return void:
	*/
	void OnRelive();

	/*
	* Comments:如果当前点不是重载地图的区域，则保存这个点
	* Param int nSceneId:场景id
	* Param int x:x坐标
	* Param int y:y坐标
	* @Return void:
	*/
	void SetNotReloadMapPos(int nSceneId,int x,int y);

	void GetNotReloadMapPos(int& nSceneId,int& x,int& y);

	//设置进入副本前的场景地图id和位置
	void SetEnterFuBenMapPos(int nSceneId,int x,int y);

	/*
	* Comments: 获取玩家Gm等级
	* @Return void:
	*/
	int GetGmLevel();
	void SetGmLevel(int);
	 
	//玩家角色创建好了，已经给客户端发了创建的数据包了
	void OnCreated();

	
	/*
	* Comments: 设置当天获取的杀怪经验总量
	* Param unsigned int nVal:
	* @Return void:
	* @Note：调试用！！！
	*/
	inline void SetExpGetTodayForDebug(unsigned int nVal)
	{
		m_nExpGetToday = nVal;
	}


	///*
	//* Comments: 延迟VIP有效时间和增加VIP经验
	//* Param unsigned int nValidTime:  VIP有效时长
	//* Param unsigned int nExp:  VIP升级经验
	//* @Return void:
	//*/
	//void addVipExpireTimeAndVipExp(unsigned int  nValidTime, unsigned int nExp);

	/*
	* Comments: 获取异步处理收集器
	* @Return AsyncOpCollector &:
	*/
	AsyncOpCollector &GetAsyncOpCollector() { return m_asopCollector; }

	/*
	* Comments:某些区域场景在进入的时候会自动加一些buff，所以要在离开的时候删除掉这些buff
	* pArea:区域属性
	* @Return void:
	*/
	void RemoveAreaBuff(SCENEAREA* pArea);
		
	//获取进入副本前的位置
	void GetEnterFbPos(	INT_PTR &nEnterFbScenceID,INT_PTR &nEnterFbPosx,INT_PTR &nEnterFbPosy);

	//获取最新一个网络数据包的发送时间
	inline long long GetGateNetWorkTickCount(){ return m_lGateTickCount;}
	// 获取逻辑服最近接收到包的时间
	inline long long GetLastRcvPacketTick() { return m_llRcvPacketTick; }
	
	//void SetCommonOpNextTimeImpl(INT_PTR nNextTime,bool bSendResult2Client ,bool bResult, unsigned int nSendPkgTime, bool bUseGateTime = true);
	// @Param nStep:如果是移动结果，这里携带移动步长
	// @Param nSendPkgTick: 客户端发送数据包的时刻，针对移动操作
	void SetCommonOpNextTimeImpl(INT_PTR nNextTime, 
								bool bSendResult2Client, 
								bool bResult, 
								unsigned int nStep, 
								unsigned int nSendPkgTick,
								bool bUseGateTime = true,
								bool bUseCheckCd=true,
								bool bNearAttackSuc = true
								);

	//检测攻击CD时间
	bool CheckAttackOpTick(bool bUseGateTime = true);

	/*
	* Comments: 获取金钱的描述
	* Param INT_PTR nMoneyType:金钱的类型
	* @Return char *:返回金钱的名字，比如元宝返回yb,绑定元宝返回byb,银两返回coin,绑定返回bcoin
	*/
	static const char * GetMoneyTypeDesc (INT_PTR nMoneyType);

	/*
	* Comments:获取奖励类型的名称，比如0,加道具的id返回道具的名称，6返回金币，也支持返回带聊天连接的
	* Param INT_PTR nAwardType:奖励的类型	
	* Param INT_PTR nAwardId:奖励的ID,只在道具有效，其他传入0
	* Param bool useChatLink:是否返回链接，只在道具有效，其他默认参数
	* Param CUserItem *pUserItem:物品的指针，只在需要连接的时候有效
	* @Return char *:奖励名称
	*/
	static const char* GetAwardTypeDesc(INT_PTR nAwardType,INT_PTR nAwardId =0,bool useChatLink=false,CUserItem *pUserItem =NULL);

	/* 
	* Comments: 通过货币类型获得对应的奖励类型
	* Param INT_PTR nMoneyType: 货币类型
	* @Return INT_PTR 奖励类型
	*/
	static INT_PTR GetAwardTypeByMoneyType(INT_PTR nMoneyType);
	/*
	* Comments:是否是消耗元宝
	* Param INT_PTR nLogId:消费 的来源
	* @Return bool:是返回true，否则返回false
	*/

	static INT_PTR GetMoneyTypeByAward(INT_PTR nAwardType);

	inline bool isConsumeYuanbao(INT_PTR nLogId)
	{
		switch(nLogId)
		{ 
		case GameLog::clDealTransYuanbao:	//交易
		case GameLog::clStallBuyItemMoney:	//摆摊
		case GameLog::clStallSellItemMoney: //摆摊
		case GameLog::clDepotMoneyOpt:		//从仓库里面取
		case GameLog::clActorDepotMoney:	//人物仓库金钱操作
		case GameLog::clConsignmentBuyItem:	//购买寄卖物品
		case GameLog::clOpenServerEarYb:	//开服赚元宝
		case GameLog::clRedPacketSend:		//发红包
		case GameLog::Log_Deal:		//私人交易
		case GameLog::Log_Consigment://寄售行
			return false;
		default: 
			return true;
		}
	}


	inline bool isRechargeYuanbao(INT_PTR nLogId)
	{
		switch(nLogId)
		{ 
		// case GameLog::clChargeYuanbao:	//通过道具充值元宝
		case GameLog::Log_Recharge:	//通过道具充值元宝
		case GameLog::Log_RechargeCard:	//通过道具充值元宝
			return true;
		default: 
			return false;
		}
	}

	/*
	* Comments: 获取角色上次下线那天获取的阵营贡献
	* @Return unsigned int:
	*/
	inline unsigned int GetZyContriLogout()
	{
		return m_nZyContriGetLastLogout;
	}

	//获得了活跃度
	void GetAcitivityAward(int value);

	/*
	* Comments:增加怒气值
	* Param int nValue:增加的值
	* Param bool boFull:是否加满
	* @Return void:
	*/
	void AddAnger(int nValue, bool boFull = false);

	
	/*
	* Comments: 设置是否追踪玩家数据包
	* Param bool bTrace:
	* @Return void:
	* @Remark:
	*/
	inline void SetTracePacket(bool bTrace)
	{
		if (m_bTracePacket != bTrace)
			m_bTracePacket = bTrace;
	}


	inline int GetRawServerIndex() const { return m_nRawServerIndex; }
	inline void SetRawServerIndex(int index) { m_nRawServerIndex = index; }

	// 是否返利卡充值
	inline bool GetRebateCharge() const { return m_bRebateCharge; }
	inline void SetRebateCharge(bool bRebateCharge) { m_bRebateCharge = bRebateCharge; }
	/*
	* Comments: 检玩家当前位置是否具有某个区域属性
	* Param BYTE bType:
	* Param SCENEAREA * pArea:
	* @Return bool:
	* @Remark: 具体参数参见CScene类的HasMapAttribute函数说明
	*/
	bool HasMapAttribute(BYTE bType, SCENEAREA* pArea = NULL);
	bool HasMapAttribute(BYTE bType, int value, SCENEAREA* pArea = NULL);

	/*
	* Comments: 是否隐藏原来玩家信息
	* @Return bool:
	* @Remark: 根据玩家所在区域属性判断
	*/
	bool HideOtherPlayerInfo() { return HasMapAttribute(aaCannotSeeName); }

	/*
	* Comments:是否在场景中改变了模型
	* @Return bool:
	*/
	bool IsSceneChangeModel() { return HasMapAttribute(aaChangeModel);}

	//设置攻击其它玩家的标志
	void SetAttackOthersFlag(bool bFlag);

	//获取玩家名字颜色类型  0 白色 1 黄色 2 褐名 3 红名
	int GetNameColorData();

	//获取回城的场景的ID，用于回城随机传送
	int GetCitySceneId(){return m_nCityPointSceneId;}

	/*
	* Comments:设置玩家转数
	* Param int nCircle:转数
	* @Return bool:
	*/
	bool SetActorCircle(int nCircle);

	//设置玩家当前祝福值
	bool SetActorBless(int nBlessValue);

	/*
	* Comments:设置全屏播放的场景特效
	* @Return bool:
	*/
	void AddSceneEffectToList(int nEffectId,int nSceneId,int nSec);

	//进入或者退出场景检查是否需要播放场景特效
	void NeedBroadSceneEffect(int nSceneId,int nLastSceneId);

	/* 
	* Comments: 设置角色头衔
	* Param int: 头衔ID
	* Param bool: true为添加头衔 false为删除头衔
	* @Return bool:  成功则返回true
	*/
	bool SetHeadTitle(INT_PTR nTitleId, bool bFlag = true);

	//判断是否有头衔
	inline bool HasHeadTitle(INT_PTR nTitleId)
	{
		/*
		if (nTitleId < 0 || nTitleId >= MAX_HEADTITLE_COUNT) 
			return false;

		unsigned nTitle = GetProperty<unsigned int>(PROP_ACTOR_HEAD_TITLE);
		return (nTitle & (BYTE)(1 << nTitleId)) ? true : false;
		*/
		return false;
	}

	/* 
	* Comments: 更新消耗元宝数据
	* Param INT_PTR: 消耗元宝数量
	* @Return void:  
	*/
	void UpdateConsumeYuanbao(const INT_PTR nConsumerCount,LPCSTR sComment = NULL);

	/*
	* Comments:检查是否可以领取奖励
	* Param INT_PTR nType:类型
	* Param INT_PTR nId:id
	* Param INT_PTR nValue:count
	* Param INT_PTR nQuality:
	* Param INT_PTR nStrong:
	* Param INT_PTR nBindFlag:
	* Param bool boNotice:出错时是否提示
	* @Return bool:能添加奖励返回true
	*/
	bool CanGiveAward(INT_PTR nType, INT_PTR nId, INT_PTR nValue, INT_PTR nQuality, INT_PTR nStrong, INT_PTR nBindFlag, bool boNotice = true);

	/*
	* Comments:当角色死亡时触发掉落物品
	* @Return int:掉落了几个物品
	*/
	//int OnDeathDropItem(CVector<DROPITEM> &listDropItem,CEntity* pEntity = NULL);
	int OnDeathDropItem();

	/*
	* Comments:死亡掉经验
	* @Return:
	*/
	void OnDeathDropExp();

	/*
	* Comments:判断是否死亡无惩罚
	* @Return bool:能死亡无惩罚将返回true
	*/
	bool CanDeathNotPunish();
	
	/*
	* Comments:判断是否可以传送
	* Param int nSceneId:目标场景Id，如果没ID就去取场景名字
	* Param LPCSTR sSceneName:场景名字，ID和名字只要有一个就可以
	* Param int nX:坐标x
	* Param int nY:坐标y
	* @Return bool:能传送将返回true
	*/
	bool CanTelport(int nSceneId, LPCSTR sSceneName = NULL, int nX = 0, int nY = 0);

	//设置工资
	inline int SetSalary(int nNowValue, int nLastValue) 
	{
		return m_nSalary = MAKELONG(nNowValue, nLastValue);
	}
	//获取工资
	inline int GetSalary(int &nNowValue, int &nLastValue)
	{
		nNowValue = LOWORD(m_nSalary);
		nLastValue = HIWORD(m_nSalary);
		return m_nSalary;
	}

	//获得加密脚本字符串的key
	int GetRundomKey() {return m_RundomKey;}

	//设置随机数
	void SetRundomKey(int nValue) {m_RundomKey = nValue;}

	/* 
	* Comments:删除全屏特效
	* Param int nEffectId:特效id
	* @Return void:
	*/
	void DeleteSceneEffect(int nEffectId);

	/* 
	* Comments:获取时装战力
	* @Return unsigned int:
	*/
	inline unsigned int GetFashionBattle()
	{
		return m_nFashionBattle;
	}

	/* 
	* Comments:设置玩家时装战力
	* Param unsigned int nVal:战力值
	* @Return void:
	*/
	void SetFashionBattle(unsigned int nVal);

	/* 
	* Comments:获取玩家登陆ip
	* @Return void:
	*/
	LPCSTR GetIp()
	{
		return m_sIp;
	}

	static LPCSTR GetFiveAttrDesc(BYTE btType);

	/*
	* Comments: 新的一天到来相关的处理
	* @Return void:
	*/
	void OnNewDayArrive(int nDayDiff = 1);

	//获取明天0点的时间
	inline unsigned int GetTomorrowTime()
	{
		return m_tomorrowDateTime.tv;
	}
	//************************************
	// Method:    SendMoneyNotEnough
	// FullName:  CActor::SendMoneyNotEnough
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: BYTE btType 货币类型，对应eMoneyType
	// Parameter: int nCount  所差货币数量
	//************************************
	void SendMoneyNotEnough(BYTE btType, int nCount);

	//
	/* 
	* Comments: 提示资源，货币，物品不足
	* Param BYTE btType: 奖励类型，
	* Param WORD wItemId: 物品id
	* Param int nCount: 数量（不是数量差)
	* @Return void:  
	*/
	void SendAwardNotEnough(BYTE btType, WORD wItemId, int nCount);

	void ChangeRecordData(RecordType btValue);

	//************************************
	// Method:    ChangeBindYuanbaoAndYuanbao 只能用于扣除绑定元宝，用元宝替代
	// FullName:  CActor::ChangeBindYuanbaoAndYuanbao
	// Access:    public 
	// Returns:   INT_PTR 返回扣除的元宝数量 返回-1代表不能扣钱 返回>=0代表成功
	// Qualifier:
	// Parameter: INT_PTR nValue 要扣除的绑定元宝的数量 
	// Parameter: WORD wLogId	 日志id
	// Parameter: LPCSTR sComment 日志描述
	//************************************
	INT_PTR DeleteBindYuanbaoAndYuanbao(INT_PTR nValue,WORD wLogId, int nCount,LPCSTR sComment,bool needLog=true, bool boForceLog = false,bool bNotice = true);
private:
	
	VOID  SendCreateData(); //发送主角创建的消息到客户端
	
	/*
	* Comments:判断复活时间是否超时（用户超过5分钟没有选择回城复活还是原地复活），如果到达，脚本处理，送回主城复活
	* @Return void:
	*/
	void ReliveTimeOut();

	//void CheckFcm(); //检测玩家的防沉迷

	/**
	* @brief: 更新玩家或者其队友所在的副本场景怪物动态属性
	* @note:  在外面升级、下线时更新
	*/
	void updateFuBenNpcDynProp(int nLevelUpdateVal);
	
	void SaveOfflinePropertyData();

	void SaveCustomInfoData();
	
	/*
	* Comments: 打包角色基本数据
	* Param ACTORDBDATA &data:
	* @Return void:
	* @Remark:
	*/
	void PackActorBasicData(ACTORDBDATA &data, int nLogoutServerId);

	/*
	* Comments: 添加消息过滤器
	* Param const int nSystemId:
	* Param const int nMsgId:
	* @Return void:
	* @Remark:
	*/
	void AddMsgFilter(const int nSystemId, const int nMsgId);

public:
	unsigned int GetActorAttackValueMax();			//获取玩家最大攻击力（区分职业）

	unsigned int GetActorDefenceValueMax();			//获取玩家最大防御力（区分职业）

	/*
	* Comments: 触发玩家被击杀死亡事件
	* Param CEntity * pKiller: 击杀者
	* @Return void:
	* @Remark:
	*/
	void TriggerBeKilledEvent(CEntity* pKiller,int nExploit);

	//更改属性值
	void ChangePropertyValue(int propId ,INT_PTR nValue, INT_PTR nAddTipId, INT_PTR nReduceTipId);


	//
	bool IsNightFighting();

	bool IsBadMan();
	//设置加入禁地时间
	// void  SetJDJoinTime(int nTime){ m_sJoinMap.m_nJDJoinTime = time(NULL)+ nTime;}
	int  GetJDJoinTime(){ return m_sJoinMap.m_nJDJoinTime ;}
	void  SetJDJoinMap(int nJdMap, int nTime, int nMagGrop = 0)
	{ 
		m_sJoinMap.m_nJDJoinMapId = nJdMap;
		m_sJoinMap.m_nJDJoinTime = time(NULL)+ nTime;
		m_sJoinMap.m_nMapGroup = nMagGrop;

	}
	//处理玩家禁地进入时间
	void DealActorJDTime();
	void AddDailyActivityDoneNum(int nActivityId, int nAddNum);

	void GetMaxCircleAndLevel(int &nCircle, int &nLevel);

	//校验玩家最大等级转数是否满足条件，转数优先
	bool CheckMaxCircleAndLevel(int nLevel, int nCircle);

	//获取回城点信息
	void GetCityPoint(WORD& nCityPointSceneId, WORD& nCityPointPosX, WORD& nCityPointPosY )
	{
		nCityPointSceneId	=	m_nCityPointSceneId;	//回城点场景id
		nCityPointPosX		=	m_nCityPointPosX;		//回城点x坐标
		nCityPointPosY		=	m_nCityPointPosY;		//回城点Y坐标
	}


	/*
	* Comments:掉落奖励
	* @Return bool:
	*/
	bool GiveDropAward(int nDropGroupId,int nLogId, bool nSend2Client = false);

	/*
	*Comments:掉落奖励，通过掉落组，带掉落物品信息
	*@Return bool：
	*/
	bool GiveDropAwardWithRet(int nDropGroupId, int nLogId,LPCTSTR , std::vector<DROPGOODS> &RetDropInfo);

	/*
	* Comments:检查玩家资源
	* nTipmsgType tips类型
	* @Return bool:
	*/
	bool CheckActorSource(int nType, int nId, int nCount, int nTipmsgType = 0);
	// /*
	// * Comments:扣除玩家资源
	// * @Return bool:
	// */
	// bool DeductionActorSource(int nType, int nCount,  int nId = 0);

	//取消boss 归属
	void DeathCancelBossBeLong(CActor* pActor = NULL);

	void SelfCancelBossBeLong();
	//设置归属的boss
	void SetBeLongBoss(int nBossId, int nSceneId);

	//判断归属玩家
	bool IsBeLongBossActor() { return blongLists.count() > 0; };

	void SetTestSimulator(bool issimu){	m_IsTestSimulator = issimu; };
	bool OnGetIsTestSimulator(){return m_IsTestSimulator; };

	void SetSimulator(bool issimu){	m_IsSimulator = issimu; };
	bool OnGetIsSimulator(){return m_IsSimulator; };//限制网络和部分DB读取

private:
	static CUpdateMask *s_actorBroadcastMask; //那些属性需要发送到客户端
	static char s_actorForbitMask[CUpdateMask::MAX_MASK_BYTE_COUNT];	 //禁止更新的属性

	/*
	* Comments: 初始化角色当日杀怪获取经验值
	* Param unsigned int nExpLastTime: 上次下线那天杀怪经验值
	* @Return void:
	*/
	void InitActorExpGetToday(unsigned int nExpLastTime);


	/*
	* Comments: 添加今日打怪获取经验
	* Param unsigned int nExp: 更新的经验数目。
	* @Return void:
	*/
	void AddExpGetToday(unsigned int nExp);


	/*
	* Comments: 检测玩家当日获取的经验是否超过了每日上限
	* @Return bool: 超过了返回true；否则返回false
	*/
	bool CheckExpGetTodayUpLimit();

	/*
	* Comments: 当日杀怪获取经验是否超过最大限度状态改变
	* Param bool bOverUpperLmt: 为true表明超过上限值；否则表明低于上限值
	* @Return void:
	*/
	void OnExpGetTodayStatusChanged(bool bOverUpperLmt);


	/*
	* Comments: 角色升级更新当日打怪获取经验属性是否超过了当前等级每日获取经验值上限
	* @Return void:
	*/
	void UpdateExpGetTodayWhenLevelUp();

	
	/*
	* Comments: 玩家登陆是初始化明天时间点（相对于当前登陆时间点的明天）
	* @Return void:
	*/
	void InitTomorrowDateTime();


	/*
	* Comments: 玩家准备下线
	* @Return void:
	* @Note: 主要做定时清理相关的补充工作。因为NextDayArrive 24点整点刷新实际上为了分散压力不是真正的准点，
	* 而是分配到随后的60s时间内执行。这样就有个问题，如果玩家随机刷新点是第二天零点50s，而他在第二天零点10s下线，
	* 这样就无法完成刷新，然后他再上线，因为db保存的上次下线时间是第二天0点10s，所以上线后也不会刷新。为了解决这个
	* 问题，在玩家下线的时候判断下是否过了真正的12点，并且还没到随机出来的整点刷新时间，如果是，就执行一次NewDayArrive的刷新。
	*/
	void PreCloseActor();
	
	/*
	* Comments:定期检查玩家所在的区域属性，做相应的处理
	* @Return void:
	*/
	void TimeCheckAreaAttri();

	/*
	* Comments:删除我未处理的messagebox消息
	* @Return void:
	*/
	void RemoveMyMessageBox();


	/*
	* Comments: 初始化玩家的阵营贡献值
	* @Return void:
	*/
	void InitActorCampContr();

	virtual int getLeaveCombatTime() { return 4*1000; }
	
	/*
	* Comments: 初始化角色当日阵营贡献
	* Param unsigned int nExpLastTime: 上次下线那天获得的阵营贡献
	* @Return void:
	*/
	void InitActorZyContriToday(unsigned int nZyContriLastTime);

	/*
	* Comments: 添加今日阵营贡献
	* Param unsigned int nZyContri: 更新的阵营贡献数目。
	* @Return void:
	*/
	void AddZyContriToday(unsigned int nZyContri);

	/*
	* Comments: 在收到角色基本数据消息后调用此函数，继续请求角色子系统数据
	* @Return void:
	* @Remark:
	*/
	void StartRequestActorSubSystemData();
	/*
	* Comments: 请求角色子系统数据
	* @Return void:
	* @Remark:
	*/
	void RequestActorSubSystemDataImpl();

	/*
	* Comments: 发送开始传送消息给客户端
	* Param int nDestServerId:
	* @Return void:
	* @Remark:
	*/
	void SendClientStartTransmit(int nDestServerId);

	/*
	* Comments: 初始化角色场景复活点位置数据
	* Param const ACTORDBDATA * pActorData:
	* @Return void:
	* @Remark:
	*/
	void InitActorRelivePointData(const ACTORDBDATA *pActorData);

	/*
	* Comments: 保存角色复活点相关的数据
	* Param ACTORDBDATA & dbData:
	* @Return void:
	* @Remark:
	*/
	void SaveActorRelivePointData(ACTORDBDATA &dbData);

	/*
	* Comments: 拆分复活点
	* Param long long point: 复活点的64位整形存储格式
	* Param WORD & sceneId: 拆分出point中的场景ID
	* Param WORD & posX: 拆分出point中的X坐标数据
	* Param WORD & posY: 拆分出point中的Y坐标数据
	* @Return void:
	* @Remark:
	*/
	void PraseRevivePoint(const long long point, WORD &sceneId, WORD &posX, WORD &posY);
		
	/*
	* Comments: 构造64位整形的复活点信息
	* Param long long & point:
	* Param const int nSceneId:
	* Param const int nPosX:
	* Param const int nPosY:
	* @Return void:
	* @Remark:
	*/
	void ComposeRevivePoint(long long &point, const int nSceneId, const int nPosX, const int nPosY);

	/*
	* Comments: 设置公共服务器的标记
	* @Return void:
	*/
	void SetCommonServerFlag();

	
	/*
	* Comments:创建爆出物品
	* Param INT_PTR nLoopTimes:数量
	* Param CVector<CUserItem * > & vDropIt:装备的指针列表
	* Param  CVector<BYTE> &equipos:装备的位置列表
	* Param  LPCSTR sMsg 用于掉落邮件
	* @Return bool:
	*/
	bool OnCreateDropBox(CVector<CUserItem*> &vDropIt, CVector<BYTE> &equipos,LPSTR sMsg = NULL);
	
	//设置GM头衔
	void OnGmTitle();

	
	/*
	* Comments:点击NPC时
	* Param CNpc * pNpc:npc指针对象
	* Param LPCTSTR sFuncName:客户端发过来的字符串
	* @Return void:
	*/
	void ClickNpc(CNpc * pNpc, short nFuncId, CDataPacket &pack);
 

	//是否强制记录日志
	inline bool isForceNeedLogMoney(INT_PTR nLogId)
	{
		switch(nLogId)
		{ 
			case GameLog::clStallBuyItemMoney:	//摆摊买物品
			case GameLog::clDealTransferMoney:	//交易
			case GameLog::clStallSellItemMoney:	//摆摊出售
			case GameLog::clBagDestroyMoney:	//扔金币
			case GameLog::clUseEntrust:			//多倍任务消耗金钱
			case GameLog::clFlushQuest:			//刷新任务
			case GameLog::clAchieveRewardMoney:	//成就奖励金钱
			case GameLog::clMailGetItem:		//邮件提取
			case GameLog::clGiveSalary:			//领取工资
				return true;
			default: 
				return false;
		}
	}
	//是否记录经验日志
	bool IsNeedLogExp(INT_PTR nLogId);

	//通用传送
	void  CommonTelePort(unsigned int nType, unsigned int nValue, unsigned char nDir=1);
	

public:
	static CVector<MessageBoxItem>*		MessageBoxList;
	static int							MsgBox_Id;					//消息的序列号
	static int                          s_nTraceAccountId;  		//追踪的玩家的账户的ID,-1表示所有的都打印,0表示不打打印，大于0表示只打印特定的账户
	static int                          s_nKickQuickUserCount;  	//踢玩家下线的数据包的个数 
	static float                         s_fKickQuickRate;  		//踢玩家下线的比例
	static int							s_nCheckQuickCountKick;  	//检查麻痹几次后就踢下线,0表示不踢
	static bool							s_boCheckQuickSealActor;	//踢下线时，是否封帐号
	static int							s_nQuickBuffId;				//麻痹buffID

	const static int	s_CheckCommOpTimes=4 ; // 标记累计多少次公共操作检测一下（用于用户外挂行为的检测）

	bool                    m_isFirstLogin:1; 		//是否是第一次登录
	bool                    m_isSuperman:1; 		//GM使用的
	bool                    m_needTraceMsg:1;		//是否需要跟踪数据包 

	WORD                    m_wInitLeftStep;  		//玩家初始化剩余的步骤
	unsigned				m_nEnterFbScenceID; 	//进入副本前的位置
	int						m_nEnterFbPosx;   	 	//进入副本前x的坐标
	int						m_nEnterFbPosy;  		//进入副本前的y的坐标
	int                     m_nFcmTime;  			//防止沉迷时间，单位秒
	int						m_nQuickBuffTimes;		//检测玩家累积添加外挂处理buff的次数
	bool					m_boCallScript;

#ifdef _DEBUG
	static LONG g_boPacketAlreadyAlloced;
	static LPCSTR g_sPacketAllocFile;
	static INT_PTR g_nPacketAllocLine;
#endif

//这里是一些不需要客户端知道的属性，放这里，免得每次加一个属性，客户端都需要修改

public:
	
	static int		    	m_sSaveDBInterval;		// 玩家存盘间隔

	/*
	*
	*	2021-08-20 火墙 暂时性修改
	
	std::deque<EntityHandle> m_deqLastFireSkillEntity; //玩家的火墙限制（9个）

	*/


private:	

	CTimer<500>				m_t500ms; 				// 500 毫秒
	CTimer<1000>			m_t1s ; 				// 1秒定时器
	CTimer<3000>			m_t3s ; 				// 3秒定时器
	CTimer<5000>            m_5s;      				// 5秒定时器(用于体力恢复)
	CTimer<15000>           m_t15s;      				// 10秒定时器
	CTimer<60000>			m_t1minute ; 			// 1分定时器
	CTimer<300000>			m_t5minute ; 			// 5分定时器
	CTimer<720000>          m_t12minute;  			// 12分定时器
	
	CTimer<1000>			m_HeartBeatTime ; 		// 心跳包定时器
	CTimer<10000>			m_EquipChkTimer;		// 装备子系统时间检查Timer		

	CTimer<20000>			m_saveDbTime;			// 存盘定时器,5分钟存盘一次
	CTimer<25000>			m_saveBasicTime;		// 角色基础数据存盘
	CTimer<20000>           m_xiuweiTimer; 			// 修为定时器，每24秒回复修为	

	TICKCOUNT				m_dwClientTicksValue;		// 客户端的心跳包上发的tickCount
	TICKCOUNT				m_nRecvHeatbeatTickCount; 	// 服务器收到客户端上一个心跳包的tickCount
	
private:	

	int						m_CreateIndex;			// 创建角色时进入的新手村索引
	int						m_LonginDays;			// 登陆的天数
	unsigned int			m_nCeateTime;			// 注册时间
	CMiniDateTime			m_CurLoginTime;			// 登陆时间 用于计算在线时长

	CMiniDateTime           m_nLoginTime;			// 本次的登录时间
	char					m_sIp[32];				// 本次的登录登陆ip
	CMiniDateTime           m_nLastLogOutTime; 		// 上次的登出时间
	CMiniDateTime           m_nUpdateTime; 			// 同步时间
	CMiniDateTime           m_nAtvTotalOnlineTime; //活动同步在线时间
	LONGLONG				m_lLastLoginIp;	   		// 上次登录ip，登录就修改
	int						m_nLastLoginServerId;	// 上次登录服务器ID

private:	

	SOCKET					m_nUserSocket;  		// SOCKET					（用这3个向网关发送数据）
	unsigned				m_nGateID; 				// LogicGate服务器的编号	 （用这3个向网关发送数据）
	unsigned				m_nGateSessionIndex; 	// 在网关上的编号			 （用这3个向网关发送数据）
	unsigned				m_nServerSessionIndex; 	// 在logicGate服务器上的索引
	unsigned int			m_nAccountID; 			// 账号的ID
	ACCOUNT					m_sAccount;				// 玩家账号字符串
	UINT64                  m_lKey;      			// 角色在网关的唯一的key
	CMiniDateTime			m_tomorrowDateTime;		// 指示明天的开始时间，也就是今天的结束时间，用于24点的刷新处理（活动、任务等逻辑）

	long long				m_lGateTickCount;		// 网关的时间
	long long				m_llRcvPacketTick;		// 逻辑服接受收据时间



private:	

	CBaseList<CDataPacket*>	m_netMsgPool;			// 网络消息缓冲
	CActorPacket			m_FastPacket;
	
	CActorProperty			m_property;  			// 属性集
	CGameSetsSystem			m_GameSets;  			// 游戏设置
	CActorVarSubSystem		m_varSystem;			// 玩家变量系统
	CUserBag				m_Bag;					// 背包系统
	CUserEquipment			m_Equiptment;			// 装备系统
	CUserDepot              m_deportSystem ; 		// 仓库系统
	CChatSystem				m_chatSystem ;			// 聊天系统
	CMailSystem				m_MailSystem;			// 邮件系统
	CGameStoreBuySystem		m_GameStoreBuySystem;	// 商城系统
	CDealSystem				m_DealSystem;			// 交易系统
	//CStallSystem			m_stallSystem;			// 摆摊系统
	CLootSystem             m_lootSystem;  			// 拾取系统
	CFubenSystem			m_fubenSystem;			// 副本系统
	CTeamSystem             m_teamSystem;  			// 队伍系统
	CGuildSystem			m_guildSystem;			// 帮会系统
	CFriendSystem			m_friendSystem;			// 好友系统
	CMsgSystem				m_msgSystem;			// 消息系统
	CPkSystem				m_pkSystem;				// pk系统
	CAchieveSystem          m_achieveSystem; 		// 成就称号系统
	CQuestSystem			m_questSystem;			// 任务系统
	CHeroSystem             m_heroSystem;       	// 英雄系统
	CBossSystem				m_BossSystem;			// BOSS系统
	CNewTitleSystem			m_NewTitleSystem;		// 新头衔系统
	CLootPetSystem			m_LootPetSystem;		// 宠物系统
	CAlmirahSystem			m_AlmirahSystem;		// 衣橱子系统(时装翅膀)
	ConsignmentSystem		m_ConsignmentSystem;	// 寄卖系统(交易行)
	CStaticCountSystem      m_StaticCountSystem;    // 计数器系统 --用于统计玩家计数器--次数限制... 
	BasicFunctionsSystem    m_BasicFunctionSystem;  // 基础功能系统
	CPetSystem              m_petSystem;  			// 宠物系统
	CCheckSpeedSystem		m_CkSpeedSystem;		// 检测加速外挂子系统
	CCombatSystem			m_CombatSystem;			// 职业竞技子系统
	CMiscSystem             m_miscSystem; 			// 杂七杂八处理
	CTreasureSystem         m_treasureSystem; 		// 宝物
	CActivitySystem			m_activitySystem;		// 个人活动系统
	CStrengthenSystem       m_strengthenSystem;     // 附加强化系统
	CGhostSystem			m_nGhostSystem;     	// 神魔
	CHallowsSystem			m_nHallowsSystem;     	// 圣物
	CCrossSystem            m_nCCrossSystem;        //跨服系统
	CReviveDurationSystem   m_ReviveDurationSystem; //复活特权系统
	CRebateSystem   		m_RebateSystem; 		//返利系统

	CActorCustomProperty	m_ProCustom;  			// 其他属性集
private:	

	// 普通场景复活点
	WORD					m_nRelivePointSceneId;	//普通场景复活点的场景id
	WORD					m_nRelivePointPosX;		//普通场景复活点的x坐标
	WORD					m_nRelivePointPosY;		//普通场景复活点的y坐标

	// 回城复活点。记录最近走过的主城(带有主城区域标记)的中心点
	WORD					m_nCityPointSceneId;	//回城点场景id
	WORD					m_nCityPointPosX;		//回城点x坐标
	WORD					m_nCityPointPosY;		//回城点Y坐标
	
	// 副本复活点
	WORD					m_nFubenReliveSceneId;	//副本的复活点场景id
	WORD					m_nFubenRelivePosX;		//副本复活点的x坐标
	WORD					m_nFubenRelivePosY;		//副本复活点的y坐标

	//////////////////////////////////////////////////////////////////////////
	// 下面的三个复活点是给跨服服务器用。不用存盘，在进入跨服传送广场的时候需要设置一下。
	ScenePosInfo			m_commsrvRelivePointPos;	// 跨服服务器普通场景复活点位置信息
	ScenePosInfo			m_commsrvFBRelivePointPos;	// 跨服服务器副本复活点位置信息
	ScenePosInfo			m_commsrvCityPointPos;		// 跨服服务器回城点信息
	ScenePosInfo			m_commsrvPosBeforeEnterFB;	// 跨服服务器进入副本前的位置信息
	ScenePosInfo			m_rawsrcScenePos;			// 在跨服服务器中记录角色在原始服务器的原始场景位置信息

	// 非重配地图复活点
	WORD					m_nNotReloadMapSceneId;	//普通场景中，非重配地图的场景id
	WORD					m_nNotReloadMapPosX;	//普通场景中，非重配地图的场景x坐标
	WORD					m_nNotReloadMapPosY;	//普通场景中，非重配地图的场景y坐标

	int						m_nLastSceneId;				// 记录本来检测上一次移动位置所在的场景id
	int						m_nLastPosX;				// 
	int						m_nLastPosY;				// 
	enTransmitStatus		m_nTransmitStatus;			// 角色传送状态
	int						m_nRawServerIndex;			// 角色所在原生逻辑服务器ID
	int						m_nTransmitDestServerId; 	// 传送目标服务器ID
	bool					m_bTracePacket;				// 追踪数据包
	CActorDataPacketStat	m_sPacketStat;				// 玩家数据包统计
	

	CVector<int>			m_FriendList;					// 好友列表 		
	AsyncOpCollector		m_asopCollector;				// 异步操作类
	INT_PTR					m_nCommOpTimes; 				// 通用操作次数（累计10次计算一次误差）
	TICKCOUNT				m_nStartCommOpTime;				// 本轮通用操作开始时间
	TICKCOUNT				m_nOccuCommOpTime; 				// 本轮累计的通用操作时间
	
private: // 玩家脱离战斗处理

	int						m_nEnterSceneFlag;			//玩家进入某个区域的标志  1 沙巴克区域

	bool					m_InDartFlag;				//正在押镖  TO DELETE
	BYTE                    m_useQuickSoftTime;    	 	//使用加速外挂的次数  TO DELETE
	int						m_nSwingLevel;				//翅膀的档次
	int						m_nSwingid;					//身上穿戴翅膀的物品id

	int						m_AttackOthersTime;		//攻击的开始时间
	unsigned long long		m_lWorkDay;				//活动与副本当日未完成次数

	int						m_nSalary;				//工资点数   TO DELETE

	unsigned int			m_nWholeAwards;			//全民奖励标记   TO DELETE
	int						m_RundomKey;			//脚本加密key   TO DELETE
	unsigned int			m_nFashionBattle;		//时装战力
	CVector<ACTORSRCEFFECT> m_SceneEffect;			//全屏的场景特效   TO DELETE

	TICKCOUNT					m_dwAutoAddExpTick;//自动加经验的执行时间   TO DELETE
	unsigned int			m_NextChangeVocTime;//下一次转职时间戳
	unsigned int			m_nOldSrvId;//原区服id

public:
	// struct timespec ts_outputTipmsg15min;
	// struct timespec ts_outputTipmsg15min_2;
	// struct timespec ts_outputTipmsg15min_3;

	static CVector<CUserItem*>* s_itemDropPtrList; // 静态列表，用于死亡掉落

	static CVector<BYTE>* s_itemDropTypeList; // 静态列表，用于死亡掉落

	INT_PTR DelItemById(int wItemId,int btCount,int btQuality=-1,int btStrong=-1,int btBindFlag = -1,
		char * sComment=NULL,int nLogID=2,const int nDuraOdds = -1,bool bIncEquipBar = false);

	void SendShortCutMsg(const char* szMsg);

	// buff 被删(时间到了被删，不包含主动删除)
	void TriggerBuffRemovedEvent(int nBuffId);

	double m_nExploitRate;   //战绩的倍率   TO DELETE

	CVector<BeLongBoss> blongLists;// 归属boss 信息
public:
	void GenerateItemAttrsInSmith( CUserItem* pUserItem, int nSmithId, int nAttrNum, int nLockTypes[], int nFlag, unsigned char nJob =0 );
	bool TransferItemAttrs( CUserItem* pSrcUserItem, CUserItem* pDesUserItem, int nFlag );
	//多陪经验是否已达上限
	bool checkGetMaxDouExp();
	void SendRankTips();
	char* GetRankIndexTipByIndex(int nIndex);
	char* GetRankIndexTipByJob(int nJob, int nType);
	LPCTSTR GetGuildName()
	{
		CGuild* pGuild = GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->GetGuildName();
		}
		return NULL;
	}

	void CheckCombineMail();
	//原区服id
	int getOldSrvId() {
		return m_nOldSrvId;
	}
	
	int getTotalOnlineTime();

	//救主灵刃CD时间保存(针对离线CD重置的处理)
	void SaveJZLRCDTime(LPCTSTR szVarName, int nTime);
	//救主灵刃CD时间设置(针对离线CD重置的处理)
	int SetJZLRCDTime(LPCTSTR szVarName);


	//检查血饮狂刀是否触发以及相关Buff的移除
	void CheckXYKD();

	void SendScriptTotalOnlineTime();//发送累计在线时间到脚本

	JoinMap    m_sJoinMap;//
	/*********************************************************************
	跨服相关
	*********************************************************************/
	void SendMsg2CrossServer(int nType);

	/*
	/* Comments: 玩家跨服登陆
	/* Param UINT_PTR nSocket: socketID
	/* Param UINT_PTR nActorID: 玩家的角色ID
	/* Param UINT_PTR nGateSessionID: 在网关上的sessionID
	/* Param UINT_PTR nGateIndex: 网关编号
	/* Param UINT_PTR nServerSessionId: 在服务器上的sessionID
	/* Param UINT_PTR nAccountID:玩家的帐户ID
	/* Param INT_PTR nCrossSrvId:跨服srvId
	/* Return void: 
	*/
	
	void Crosslogin(UINT_PTR nSocket,UINT_PTR nActorID,UINT_PTR nGateID, 
		UINT_PTR nGateSessionIndex,UINT_PTR nServerSessionIndex,
		UINT_PTR nAccountID,INT_PTR nCrossSrvId);

	//跨服数据初始化
	bool CrossInit(void * pData, size_t size);
	//跨服步长
	void OnCrossFinishOneInitStep(int step);
	//发送跨服数据
	bool SendLoginData2CrossServer();

	inline unsigned int GetCrossActorId()
	{
		return GetProperty<unsigned int>(PROP_CROSS_SERVER_ACTORID);
	}
	//进入跨服场景
	void EnterCrossServerScene();
	void Mailtest();
	void SetScriptCrossServerData(int nRealyActorId);
	//加载玩家跨服工会id
	void SendLoadCrossServerGuild();
	//
	void CrossServerRetDb(int nCmd,int nErrorCode,CDataPacketReader& reader);
	//跨服获取玩家 原服 真实id 和服务器id
	static void GetCSRealyActorId(int nActorId, int& nRealyActor, int& nRawSrvId);
	//离线邮件
	void LoginCheckOfflineMail();
	//登录需要踢掉跨服的玩家
	void LoginCloseActor2Center();
	
	void DeleteScriptAccountId(int nAccountId, int nRawSrvId);
	void SetScriptAccountId(int nAccountId, int nRawSrvId );
	void KickUserAccount();

	// 设置玩家充值状态 0 ：初始值；1：已完成首充；2：已完成二充
	void SetRechargeStatus(LPCTSTR szVarName, int nStatus);
	int GetRechargeStatus(LPCTSTR szVarName);

	// 设置跨天登录跨的天数
	void SetNewDayArrive(int nNewDayArrive) { m_nNewDayArrive = nNewDayArrive; };
	int GetNewDayArrive() { return m_nNewDayArrive; };

	//backStage命令处理的函数 
	//void OnSetNewCdkGift(std::string giftCode);

	//多人同屏网络优化部分
	bool CheckNetworkCutDown(const int fuId, const int SceneId);
	void InitCanDisplay();
	bool CanNetworkCutDown();
	void OnAddCanDisplay(EntityHandleTag& tag);
	void OnCalculateDisplay();				//计算显示的玩家
	int CanNetworkCutDownDisplayCount();//可以特殊同屏的数量
 
	//增加可视玩家
	bool OnAddCanSeeEntity(EntityHandle & hEntity); 
	bool OnClearCanSeeEntity(EntityHandle & hEntity);
	bool OnCheckCanSeeEntity(EntityHandle & hEntity);

	//后台功能 
	void OnBackForbidUser(INT_PTR nDuration);	//后台封禁角色 
	void OnBackUnForbidUser();					//解封

	//DB返回
	//玩家额外数据
	virtual VOID OnDbCustomInfoData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader &reader);

	//自定义数据处理
	int OnGetCustomInfoInt(int index);
	unsigned int OnGetCustomInfoUInt(int index); 
	BYTE OnGetCustomInfoByte(int index);

	 
public:
	WORD                    m_wCrossInitLeftStep;  		//玩家初始化剩余的步骤
	int                     nCrossActorId = 0;//
	/**********************************************************************/
	
private: //基础属性
	int                     m_nGmLevel;				///< 玩家的GM等级。GM等级大于0才是GM
	bool					m_bRebateCharge;		// 是否 返利卡充值 

	//多人同屏网络优化部分
	bool m_CanNetworkCutDown;
	std::vector<EntityHandleTag> m_CanDisplayList;		//可以看的
	std::vector<CEntity*> m_PrecedenceDisplayList;		//优先 
	int						m_nNewDayArrive;		// 设置跨天登录跨的天数 
 
	//每个玩家自己的可视列表查询用
	std::map<unsigned long long, EntityHandle> m_CanSeeEntityList;
 
	//定时器
	CTimer<3600000>	m_SecTimer;	//1小时检查一次3600000 

	bool m_IsTestSimulator;
	bool m_IsSimulator; 
	//CDataPacket m_SimulaPack;
	BYTE m_cbSimulatorData[10240]; //虚拟的数据用于发送客户端消息屏蔽
};
