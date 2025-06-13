#pragma once
//class CComponentManager;
#include "JXAbsGameMap.h"

//多倍活动结构
typedef struct tagActivityRate
{
	CommonActivityId	m_nActivityId;	//活动id
	float				m_fActRate;		//当前活动经验倍率
	UINT				m_uRateTime;	//多倍结束时间
}ACTIVITYRATE;

class CLogicEngine :
	public wylib::thread::CBaseThread
{
public:
	typedef CBaseThread Inherited;

public:
	//逻辑处理统一TICKCOUNT
	static TICKCOUNT	nCurrTickCount;		
	//全局功能脚本文件路径
	static LPCTSTR		szGlobalFuncScriptFile;
	//全局怪物脚本文件路径
	static LPCTSTR		szMonsterFuncScriptFile;

	//任务脚本文件
	static LPCTSTR		szQuestNpcFile;
	//物品脚本文件
	static LPCTSTR		szItemNpcFile;

public:
	CLogicEngine();
	~CLogicEngine();

	//启动逻辑引擎
	BOOL StartEngine();
	//停止逻辑引擎
	VOID StopEngine();

	/*
	* Comments:提供接口给数据服务器连接调用，当连接上的时候
	* @Return VOID:
	*/
	VOID InitComponent()
	{
		if (HasDbDataInit())
			return;
		m_GlobalVarMgr.Load();
		m_ActivityMgr.Load();
		m_BossMgr.Load();
		m_RankingMgr.LoadBaseRankData();
		m_MiscMgr.Init();
		m_ConsignmentMgr.Load();
		m_MailMgr.Load();
		OnDbInitData();
	}
	//保存
	VOID SaveComponent()
	{
		if (!HasDbDataInit())
			return;
		m_GlobalVarMgr.Save();
		m_ActivityMgr.Save();
		m_BossMgr.Save();
		m_ConsignmentMgr.SaveConsignItemToDb();

		m_GuildMgr.SaveAllGuildEvent();
		m_GuildMgr.SaveGuildApplyReslut();
	}

	/* 设置主逻辑循环性能参数
	 * dwTimeLimit	主循环中每次循环时间限制，单位是毫秒
	 * nLoopCount	主循环中每次循环处理逻辑数据次数限制
	 * dwSleep		主循环中每次循环休眠时间，单位是毫秒
	 */
	inline VOID setMainLoopPerformanceParam(TICKCOUNT dwTimeLimit, INT_PTR nLoopCount, TICKCOUNT dwSleep)
	{
		m_dwMainLoopTimeLimit = dwTimeLimit;
		m_nMainLoopLimit = nLoopCount;
		m_dwMainLoopSleep = dwSleep;
	}
	/* 获取主逻辑循环性能参数 */
	inline VOID getMainLoopPerformanceParam(TICKCOUNT& dwTimeLimit, INT_PTR& nLoopCount, TICKCOUNT& dwSleep)
	{
		dwTimeLimit = m_dwMainLoopTimeLimit;
		nLoopCount = m_nMainLoopLimit;
		dwSleep = m_dwMainLoopSleep;
	}
	/* 获取上次主逻辑循环的性能统计值 
	 * dwTimeConsume	输出上次主逻辑循环消耗的处理器时间，单位是毫秒
	 * nLoopCount		输出上次主逻辑循环的逻辑处理次数
	 */
	inline VOID getLastLoopPerformance(TICKCOUNT& dwTimeConsume, INT_PTR& nLoopCount)
	{
		dwTimeConsume = m_dwLastLoopTime;
		nLoopCount = m_nLastLoopCount;
	}
	inline CEntityManager* GetEntityMgr(void){return &m_EntityMgr; }
	inline CGuildComponent& GetGuildMgr(void) {return m_GuildMgr;}
	inline CActivityComponent& GetActivityMgr(void) {return m_ActivityMgr;}
	inline CFuBenManager* GetFuBenMgr(void) {return &m_FuBenMgr;}
	inline CNetWorkMsgHandle* GetNetWorkHandle() { return &m_NetWorkHandle;}	
	inline CNpc* GetGlobalNpc() { return m_pGlobalFuncNpc;}
	inline lua_State * GetGlobalNpcLua() { return m_pGlobalFuncNpc->GetScript().GetLuaState(); }
	inline CNpc* GetMonFuncNpc() { return m_pMonFuncNpc; }
	inline CNpc * GetItemNpc() { return m_pItemNpc;}
	inline CRankingMgr& GetRankingMgr() { return m_RankingMgr;}
	inline CGlobalVarMgr& GetGlobalVarMgr() { return m_GlobalVarMgr; }
	inline CBossManager& GetBossMgr() { return m_BossMgr; }

	inline CTopTitleMgr& GetTopTitleMgr() { return m_TopTitleMgr;}
	inline CEncryptFunctionName & GetEncryptFuncMgr() {return m_EncryptFunctionMgr;}
	inline CTeamManager& GetTeamMgr(){return m_teamMgr;} //获取队伍管理器
	inline CScriptTimeCallManager& GetScriptTimeCallManager(){ return m_ScriptTimeCallMgr; }

	inline CMiscMgr& GetMiscMgr() { return m_MiscMgr;}

	inline CWorldLevelMgr& GetWorldLevelMgr() { return m_WorldLevelMgr;}
	inline CConsignmentMgr& GetConsignmentMgr(){return m_ConsignmentMgr;}
	inline CMailMgr& GetMailMgr(){return m_MailMgr;}
	inline CTransmitMgr& GetTransmitMgr() { return m_TransmitMgr; }
	inline CCrossMgr& GetCrossMgr() {return m_CrossMgr;} 
	//获取时间管理的组件
	inline CTimeStatisticMgr*  GetStatisticMgr(){return m_timeStatisticMgr; }

	//获取聊天的组件
	inline CChatManager & GetChatMgr(){return m_chatMgr;}
	//获取公告的组件
	inline CNoticeManager & GetNoticeMgr(){return m_NoticeMgr;}
	
	inline CSimulatorMgr* GetSimulatorMgr() { return &m_SimulatorMgr;}


	inline void SetUserItemSpId(int nSpId)
	{
		m_UserItemAllocator.SetSpid(nSpId);
	}

	inline void SetUserItemServerId(int nSvrId)
	{
		m_UserItemAllocator.SetServerIndex(nSvrId);
	}

	inline CUserItem::ItemSeries BuildMailSn()
	{
		return m_UserItemAllocator.BuildMailSn();
	}

	inline CUserItem::ItemSeries BuildNeedBuySn()
	{
		return m_UserItemAllocator.BuildNeedBuySn();
	}

	//返回离线玩家的管理器
	inline COfflineUserMgr& GetOfflineUserMgr() {return m_offlineUserMgr;}
	
	
	//返回跨服离线玩家的管理器
	inline COfflineCenterUserMgr& GetOfflineCenterUserMgr() {return m_offlineCenterUserMgr;}
	
	/* 
	* Comments: 拷贝从物品管理器分配一个物品，用于从网络字节流里读取物品以后，拷贝到物品管理器
	* Param const bool boNewSeries: 是否需要申请序列号
	* Param CUserItem * pTempUserItem: 读取的临时存放的物品的指针
	* @Return CUserItem*: 物品管理器的新指针
	*/
	inline CUserItem* _CopyAllocUserItem(const bool boNewSeries,CUserItem *pTempUserItem, LPCSTR file, INT_PTR line)
	{
		CUserItem *pUserItem= _AllocUserItem(boNewSeries,file,line); 
		if(pUserItem == NULL)
		{
			OutputMsg(rmError,"[ItemTrace],AllocUserItem alloc return nil,file=%s,line=%d",file,(int)line);
			return NULL;
		}
		else
		{
			memcpy(pUserItem,pTempUserItem,sizeof(CUserItem));
			return pUserItem;
		}	
	}
	#define CopyAllocUserItem(boNewSeries,pTempUserItem) _CopyAllocUserItem(boNewSeries, pTempUserItem,__FILE__, __LINE__)

	//申请用户物品对象，boGenNewSeries参数用于决定是否分配新的物品系列号
	inline CUserItem* _AllocUserItem(const bool boNewSeries, LPCSTR file, INT_PTR line )
	{ 
		return m_UserItemAllocator.allocItem(boNewSeries,file,line); 
	}
	#define AllocUserItem(boNewSeries) _AllocUserItem(boNewSeries, __FILE__, __LINE__)

	
	inline void _DestroyUserItem(CUserItem* pUserItem, LPCSTR file, INT_PTR line )
	{ 
		m_UserItemAllocator.DestroyUserItem(pUserItem,file,line); 
	}
	#define DestroyUserItem(pUserItem) _DestroyUserItem(pUserItem, __FILE__, __LINE__)

	//输出物品的使用情况
	void TraceItem()
	{
		m_UserItemAllocator.TraceItem();
	}

	//申请实体消息
	CEntityMsg* AllocEntityMsg(LPCSTR file, INT_PTR line );
	
	

	//销毁实体消息
	void __DestoryEntityMsg(CEntityMsg *pMsg,CEntity * pEntity,LPCSTR file, INT_PTR line);
	#define DestoryEntityMsg(pMsg,pEntity) __DestoryEntityMsg(pMsg,pEntity,__FILE__,__LINE__)

	//创建实体的特效
	inline CEntityEffect* AllocEntityEffect(){ return m_EntityEffectAllocator.allocObject(); }
	
	//删除实体的特效
	inline void DestoryEntityEffect(CEntityEffect* pMsg){m_EntityEffectAllocator.freeObject(pMsg); }
	
	//获取同步的系统逻辑日期和时间
	inline SYSTEMTIME& getSysTime(){ return m_CurSysTime; }
	//获取同步的系统TICKCOUNT
	inline TICKCOUNT getTickCount(){ return m_dwCurTick; }
	//获取同步的逻辑短日期时间值
	inline unsigned int getMiniDateTime(){ return m_CurMiniDateTime; }
	//判断是否同个星期 nRefreshDay:判断星期 0星期天为分界 1星期1分界
	bool IsSameWeek(unsigned int t1, unsigned int t2, unsigned int nRefreshDay=0);
	//判断是否同个月
	bool IsSameMonth(unsigned int t1, unsigned int t2)
	{
		SYSTEMTIME st1, st2;
		((CMiniDateTime)t1).decode(st1);
		((CMiniDateTime)t2).decode(st2);
		if (st1.wYear != st2.wYear || st1.wMonth != st2.wMonth)	//判断是否同一个月
		{
			return false;
		}
		return true;
	}
	//获取当前在线的人数
	inline int getOnlinceCount() {return (int)m_nOnliceCount;}
	/*
	* Comments: 获取一个随机数，暂时用当前的滴答数量，这个可能还会修改
	* @Return INT_PTR: 返回一个随机数
	*/
	inline unsigned int GetRandValue()
	{
		return wrandvalue();
	}

	//返回 [0, nModule)
	inline unsigned int GetRandValue(unsigned int nModule)
	{
		if (nModule ==0) return 0;
		return ( (unsigned int) wrandvalue() + (unsigned int)_getTickCount() ) % (unsigned int) nModule;
	}

	//返回ai管理器
	inline CAiMgr & GetAiMgr(){return m_aiMgr;}
	//获取给脚本存储的动态数据
	inline CCLVariant& GetDyanmicVar() { return m_DynamicVar; }
	// 获取后台资源加载模块
	inline CBackResLoadModule& GetBackResLoadModule() { return m_backLoadModule; }
	// 重新加载物品配置数据
	void reloadItemConfig();
	//获取计分器管理器
	inline CScoreRecorderManager& GetScoreRecorderManager(){ return m_ScoreRecorderManager; }

	//获取数据包统计
	inline CActorDataPacketStat& GetDataPacketStat() {return m_sPacketRecord;}
	
	//获取是否统计数据包的标志
	inline BOOL GetPacketRecordState() {return m_bTracePacketRecord;}

	inline void SetPacketRecordState(bool bState) {m_bTracePacketRecord = bState;}
	

	//设置服务器多少秒后自动停机维护，单位是秒
	void SetStopServerTime(UINT nTime);
	void CancelStopServer();
	//发停服公告
	void SendStopServerNotice();

	/*
	* Comments:设置经验倍率
	* Param INT_PTR nRate:倍率
	* Param INT_PTR nTime:持续时间，如果是0，表示不限时
	* @Return INT_PTR:返回成功设置了多少倍率
	*/
	INT_PTR SetSysExpRate(INT_PTR nRate,INT_PTR nTime);

	INT_PTR GetSysExpRate() { return (INT_PTR)m_ExpRate; }

	/// @param nTime 持续时长
	bool SetActivityCommonRate(CommonActivityId	nActivityId,float nRate, INT_PTR nTime);

	//获取活动多倍加成比率
	float GetActivityCommonRate(CommonActivityId nActivityId);

	/*
	* Comments: 获取角色消息过滤器
	* @Return CActorMsgFilter&:
	* @Remark:
	*/
	CActorMsgFilter& GetActorMsgFilter() { return m_AcotrMsgFilter; }

	// 输出内存池详细信息
	void DumpDataAllocator();

	//发送消息给数据服务器停止数据服
	void SendDbServerStop();

	//引擎是否启动了 
	volatile BOOL inline IsEngineerRuning()
	{
		if( TRUE == InterlockedCompareExchange(&m_boEngineRunning, TRUE, TRUE))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
		
	}
	//获取stringbuff
	inline CStringBuff& GetStringBuff()
	{
		return m_strBuff;
	}

	/*
	* Comments:开启性能日志记录
	* Param bool isOpen:是否开启，true表示开启,false不开启
	* @Return void:
	*/
	inline void SetOpenPerfLog(bool isOpen)
	{

		m_bNeedPerfLog  =isOpen;
		CTimeProfDummy::SetOpenFlag(isOpen);
	}

	VOID AddDealToLog(int nLogid,
		int nActorid,int nTarActorid,
		INT nItemId=0,WORD nItemNum=0,WORD nMoneyType=0,INT64 nMoneyNum=0, int nDues = 0, unsigned int nServerId = 0);

	/*
	* Comments:记录元宝的消费
	* Param WORD nLogid:消息类型
	* Param int nActorid: 角色id
	* Param LPCTSTR szAccount:帐号名称
	* Param LPCTSTR szCharName:角色名称
	* Param int nCount:购买数量
	* Param int nPaymentCount:消费总价钱
	* Param LPCTSTR szConsumeDescr:描述，如物品的名字
	* int  nConsumeLevel:玩家的消费的等级
	* @Return VOID:
	*/
	VOID AddCosumeToLog(WORD nLogid,int nActorid,LPCTSTR szAccount,
		LPCTSTR szCharName,BYTE nMoneytype,int nCount,int nPaymentCount,LPCTSTR szConsumeDescr=NULL,int nConsumeLevel=0, int nBalance = 0,unsigned int nServerId = 0);

	//发送累计玩家的消费
	void SendActorConsumeLog();

	/*
	* Comments:记录玩家击杀掉落
	* Param int nActorid: 角色id
	* Param LPCTSTR szAccount:帐号名称
	* Param LPCTSTR szCharName:角色名称
	* Param LPCTSTR szMonName:怪物名称
	* Param LPCTSTR szSceneName:场景名称
	* Param LPCTSTR szItemName:道具名称
	* Param int nCount:数据
	* Param int nPosx:x坐标
	* Param int nPosy:y坐标
	* @Return VOID:
	*/
	VOID AddKillDropToLog(int nActorid,LPCTSTR szAccount,LPCTSTR szCharName,LPCTSTR szMonName,
	LPCTSTR szSceneName,LPCTSTR szItemName,int nCount,int nPosX, int nPosY,unsigned int nServerId = 0);

	//发送击杀掉落统计
	void SendKillDrop2Log();

	/*
	* Comments:记录玩家击杀掉落
	* Param int nActorid: 角色id
	* Param LPCTSTR szAccount:帐号名称
	* Param LPCTSTR szCharName:角色名称
	* Param int nAtvId:活动id
	* Param int nIndex:索引
	* @Return VOID:
	*/
	VOID AddJoinAtvToLog(int nActorid,LPCTSTR szAccount,LPCTSTR szCharName,int nAtvId, int nIndex,unsigned int nServerId = 0);
	void SendJoinAtv2Log();
	void BackStageSetSecondPswFlag(bool boFlag);
	bool GetSecondPswFlag() {return m_boSeoncdPswFlag;}
	bool GetLoginSecondPswFlag() {return m_boLoginSecondPswFlag;}
	void SetLoginSecondPsw()
	{
		m_boLoginSecondPswFlag = !m_boLoginSecondPswFlag;
	}

	static void PostAsyncWorkMsg(void* pCBData);
	
	void SetPrintDamageInfo(bool v){  m_boPrintDamageInfo = v; }
	bool IsPrintDamageInfo() { return m_boPrintDamageInfo; }

protected:
	VOID OnRountine();
	//virtual void OnThreadStarted();
	//virtual void OnTerminated();
	//单次逻辑处理
	VOID LogicRun();
	//逻辑引擎启动函数（由主逻辑线程启动时调用）
	VOID LogicEngineStarted();
	//逻辑引擎停止函数（由主逻辑线程即将退出前调用）
	VOID LogicEngineStoped();

private:
	//初始化保留场景
	VOID InitReserveScene();
	//初始化保留NPC
	BOOL InitReserveNpc();
	// 设置线程的亲和度
	void SetThreadAffinite(DWORD_PTR dwMask);

	//DB初始化子系统的数据完成
	inline void OnDbInitData()
	{
		m_hasDbInitData =true;
	}

	//DB的数据是否初始化完毕
	inline bool HasDbDataInit()
	{
		return m_hasDbInitData;
	}
	
	//销毁消息管理器
	void TraceMsgMgr();

private:
	static CTimeStatisticMgr*    m_timeStatisticMgr;     //时间统计组件
	volatile LONG			m_boEngineStarted;		//逻辑引擎是否已经启动
	volatile LONG			m_boEngineRunning;		//逻辑引擎正在运行

	TICKCOUNT				m_dwMainLoopTimeLimit;	//主循环中每次循环时间限制，单位是毫秒
	INT_PTR					m_nMainLoopLimit;		//主循环中每次循环处理逻辑数据次数限制
	TICKCOUNT				m_dwMainLoopSleep;		//主循环中每次循环休眠时间，单位是毫秒
	TICKCOUNT				m_dwLastLoopTime;		//上次完整逻辑循环消耗的时间，单位是毫秒
	INT_PTR					m_nLastLoopCount;		//上次逻辑循环次数
#ifdef WIN32
	HANDLE					m_hStartEvent;			//启动信号事件
	HANDLE					m_hStopEvent;			//停止信号事件
#else
	sem_t                   m_hStartEvent;           //启动信号事件
	sem_t                   m_hStopEvent;           //停止信号事件
#endif
	SYSTEMTIME				m_CurSysTime;			//当前系统日期和时间，每个循环更新一次
	TICKCOUNT				m_dwCurTick;			//当前系统TICKCOUNT，每个循环更新一次
	CLogicServer *          m_pLogicServer; 
	CEntityManager          m_EntityMgr;      
	CRankingMgr				m_RankingMgr;	//排行榜管理器
	CSimulatorMgr			m_SimulatorMgr;	//模拟人物管理器 
	CGlobalVarMgr			m_GlobalVarMgr;
	CBossManager			m_BossMgr; //boss信息
	CTopTitleMgr			m_TopTitleMgr;
	CMiscMgr				m_MiscMgr;
	CMailMgr				m_MailMgr;
	CWorldLevelMgr			m_WorldLevelMgr;
	CConsignmentMgr			m_ConsignmentMgr;
	CTransmitMgr			m_TransmitMgr;
	CCrossMgr               m_CrossMgr;//跨服管理
	CEncryptFunctionName	m_EncryptFunctionMgr;
	CNetWorkMsgHandle		m_NetWorkHandle;
	CFuBenManager			m_FuBenMgr;	
	CMiniDateTime			m_CurMiniDateTime;		//当前短日期时间
	COfflineUserMgr         m_offlineUserMgr;  //离线玩家的信息管理器
	COfflineCenterUserMgr   m_offlineCenterUserMgr;//跨服离线玩家数据
	CTeamManager            m_teamMgr; //队伍管理器
	CScriptTimeCallManager	m_ScriptTimeCallMgr;	//脚本定时调用管理器
	CChatManager            m_chatMgr;    //聊天管理器
	CNoticeManager         	m_NoticeMgr;    //公共管理器
	CUserItemAllocator		m_UserItemAllocator;	//用户物品申请器
	CActorMsgFilter			m_AcotrMsgFilter;		// 角色消息过滤器
	CSingleObjectAllocator<CEntityMsg>	m_EntityMsgAllocator;//实体消息内存管理器
	CSingleObjectAllocator<CEntityEffect> m_EntityEffectAllocator; //实体的特效的内存管理器

	jxcomm::gameMap::CAbstractMap		m_ReserveMapData;//保留场景的地图数据
	CScene*					m_pReserveScene;//保留场景，用于存放保留NPC
	CNpc*					m_pGlobalFuncNpc;	//执行全局的脚本函数
	CNpc*					m_pMonFuncNpc;//全局怪物脚本
	CNpc *					m_pItemNpc;		//物品npc
	CAiMgr                  m_aiMgr; //ai管理器
	unsigned int			s_randSeed; //随机数种子,为了避免一个循环里取随机数相同
	CTimer<0>				m_CheckLuaMemTimer;	//检查脚本内存管理器内存的定时器

	CTimer<300000>			m_SendOnlineCountTimer;//发送在线人数的定时器,1分钟一次
	CTimer<120000>			m_timeStatTimer;// 耗时分析,2分钟一次	
	INT_PTR					m_nOnliceCount;//在线人数，每分钟更新
	CCLVariant				m_DynamicVar;		//一个用来给脚本保存全局变量数据的变量，数据不保存db

	

	CBackResLoadModule		m_backLoadModule;	// 后台加载模块
	CScoreRecorderManager	m_ScoreRecorderManager;//计分器管理器

	UINT					m_StopServerTime;	//停机维护倒计时，0表示不停机
	UINT					m_StopServNoticeTimer;		//停服公告每15s发一次
	
	CStringBuff             m_strBuff;   //字符串缓冲器，用于大的字符串操作的
	float					m_ExpRate;	//当前的系统经验倍率
	//float					m_ActExpRate;	//当前活动经验倍率
	UINT					m_ExpRateTime;//系统经验倍率的过期时间，即到期就恢复成0（无加成经验）
	//UINT					m_ActExpRateTime;
	TICKCOUNT				m_dwCheckOnlineSceneLog;//下次检查记录在线人数的场景信息
	BOOL					m_boOnlineSceneLoged;	//是否已经记录在线人数的场景信息

	TICKCOUNT				m_dwActorConsumeLogTime; //发送消息的消费日志的间隔
	TICKCOUNT				m_dwKillDropLogTime; //发送怪物击杀日志的间隔
	TICKCOUNT				m_dwJoinAtvLogTime; //

	bool                    m_bNeedPerfLog;        //是否需要记录性能日志

	BOOL					m_bTracePacketRecord;	//是否统计数据包
	CActorDataPacketStat	m_sPacketRecord;			//数据包统计	

	bool					m_hasDbInitData;		//DB是否初始化了改子系统的数据(防止每次重启DB都初始数据)
	
	CGuildComponent			m_GuildMgr;	
	CActivityComponent		m_ActivityMgr;	

	CVector<LOGCONSUMESAVE>			m_ActorTotalLog;	//发送到全局的消费日志  汇总所有的消费项，如果有消费定时向服务发包 累计玩家每天的消费
	CVector<LOGKILLDROPSAVE>		m_KillDropLog;	//发送到全局的击杀掉落日志
	CVector<LOGACTIVITYSCHEDULE>	m_JoinAtvlog;	//参与活动数据
	CVector<ACTIVITYRATE>			m_AvtivityRateList; //活动多倍加成数据

	bool					m_boSeoncdPswFlag; //false为开启二级密码验证，true为关闭二级密码验证
	bool					m_boLoginSecondPswFlag;//是否跳过登录验证二级密码 true为跳过
	bool					m_boPrintDamageInfo;
public:
	static LPCTSTR s_szBuffAllocStatLogName;

};


