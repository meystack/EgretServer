#pragma once
#include<bitset>
/***************************************************************/
/*
/*  功勋系统（成就子系统）
/* 
/***************************************************************/

#define MAX_ACHIEVE_EVENT_COUNT			512 ///< 成就事件的最大数量，这个需要根据策划的配置增加,为了避免内存的申请释放，使用这个
#define MAX_ACHIEVE_GROUP_BYTE_COUNT	8	///< 成就的分组开启的数据存储

#define MAX_MEDAL_LEVEL 6 //勋章系统的最大等级

static const char *g_szIsAchieveFinishedData		= _T("IsAchieveFinishedData");		//成就完成状态
static const char *g_szIsAchieveGiveAwardsData		= _T("IsAchieveGiveAwardsData");	//成就领奖状态
enum AchievementState
{
	nAchieveNoComplete = 0, //未完成
	nAchieveComplete = 1, //已完成 未领取
	nAchieveIsGet = 2,   //已领取
};


class CAchieveSystem:
	public CEntitySubSystem<enAchieveSystemID,CAchieveSystem,CActor>
{
public:

	//用于存储那种带过期时间的称号的数据
	typedef struct tagTitleTime
	{
		int nTitleId;    //称号的ID
		unsigned int nExpiredMiniTime;  //称号的过期时间
	}TITLETIME,*PTITLETIME; 

	enum tagAchieveConditionType
	{
		eAchieveConditionNone =0,  //没有条件
		eAchieveConditionEnum =1, //枚举型
		eAchieveConditionRange=2, //范围型
		
	};
	typedef CEntitySubSystem<enAchieveSystemID,CAchieveSystem,CActor> Inherid;
	CAchieveSystem()
	{
		memset(m_achieveFinishState,0,sizeof(m_achieveFinishState));
		memset(m_achieveEventData,0,sizeof(m_achieveEventData) );
		memset(m_titles,0,sizeof(m_titles));//称号的列表
		memset(m_BabgeState,0,sizeof(m_BabgeState));
		nCanGetNum.clear();
		memset(m_achieveGroup,0,sizeof(m_achieveGroup));
		m_nAchieveEventDbUseCount =0; //使用了0字节 
		m_hasSendInitData =false;
		isInitData = false;
		isDataModify = false;
		//memset(m_atomEventCount,0,sizeof(m_atomEventCount));
	}
	
	/*处理网络数据
	 * nCmd 消息号
	 * packet 网络包
	 */
	void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );
	//处理db返回
	void OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet);
	//保存
	void SaveData();
	
	//重载初始化函数，暂时的话所有的都是GM
	bool Initialize(void *data,SIZE_T size);
	/*---------------------------------------------------------------------------

	/*
	* Comments: 为成就系统触发事件
	* Param INT_PTR nAtomID: 原子事件的id,在tagAchieveEventID里定义
	* Param INT_PTR nParam1: 参数1 
	* Param INT_PTR nParam2: 参数2
	* Param INT_PTR nParam3: 参数3
	* Param INT_PTR nParam4: 参数
	* @Return void:
	*/
	void OnEvent(INT_PTR nAtomID, INT_PTR nParam1 =-1,INT_PTR nParam2= -1,INT_PTR nParam3 =-1,INT_PTR nParam4 =-1);

	/* 
	* Comments: 兑换称号
	* Param INT_PTR nId:  兑换称号的id
	* @Return bool:  
	*/
	bool ExchangeBadge(INT_PTR nId);


	//枚举型的条件是否满足，也就是一个数值是否在datalist中
	inline bool GetEnumConditionFlag(int nValue, DataList<int> & datalist);

	/*
	* Comments:获取枚举类型是否满足
	* Param int nValue:数值的类型
	* Param DataList<int> & datalist: 范围的列表，第一个是小的，第2个是大的
	* @Return bool:如果在这个范围返回true，否则返回false
	*/
	inline bool GetRangeConditonFlag(int nValue, DataList<int> & datalist);

	
	/*
	*存盘接口，传的是存盘的结构指针,单独存盘的系统就不关住这个数据指针
	*data 玩家存盘数据指针
	*/
	virtual void Save(PACTORDBDATA  pData);

	//玩家进入游戏的时候触发，用于发送某些数据给玩家初始化
	virtual void OnEnterGame();

	//当玩家等级升级的时候
	void OnLevelUp(); 
	
	//一些特殊的成就，比如活动需要每天刷新
	void RefeshPerDay(bool isLogin);

	//获取成就徽印数据
	char* GetAchieveBabgeData()
	{
		return m_BabgeState;
	}

	void SetAchieveUnFinished( INT_PTR nAchieveId);
	//秒完成一个成就，会触发对应所有原子事件，主要用在秒活跃项
	void SetAchieveInstantFinish(INT_PTR nAchieveId, bool boGm = false);
private:

	//玩家能否获得这个成就的奖励
	bool CanGetAward(PACHIEVEAWARD pAward);
	
	
	void InitAchieveEvent(); //初始化成就事件的订阅

	//获取给物品的奖励的logid
	inline INT_PTR GetGiveAwardLogId(INT_PTR nType);
	/*
	* Comments:获取一个成就的奖励
	* Param INT_PTR nAchieveID:成就的ID
	* Param INT_PTR defaultMode:能否直接领取奖励
	* @Return bool:成功返回true，否则返回false
	*/
	bool GetAchieveAwards(INT_PTR nAchieveID,bool defaultMode = false);

	/*
	* Comments:判断是否为充值礼包的成就
	* Param INT_PTR nAchieveID:成就的ID
	* @Return bool:成功返回true，否则返回false
	*/
	bool GetGiftAwardsEvent(INT_PTR nAchieveID, byte & nCombineFlag);

	/*
	* Comments: 获取成就的条件已经完成的次数
	* Param INT_PTR nAchieveEventId：成就的条件
	* @Return INT_PTR：返回成就的条件完成了几个
	*/
	//inline INT_PTR GetAchieveConditionFinishCount(INT_PTR nEventID);
	


	/*
	* Comments:获取一个成就事件存储的位置(如果没有存储就是-1)，以及完成的数量
	* Param INT_PTR nEventID:
	* Param INT_PTR & nPos:
	* Param INT_PTR & nCount:
	* @Return void:
	*/
	inline void GetAchieveEventDataPosCount( INT_PTR nEventID,INT_PTR &nPos,INT_PTR &nCount);


	/*
	* Comments:成就的条件满足了触发一下
	* Param PACHIEVECONDITION pCondition: 成就的条件
	* Param INT_PTR nConditionCount:条件的个数
	* Param INT_PTR progressPlus:进度值，比如充值900，这里就是900
	* @Return bool:
	*/
	bool    AchieveConditionPlus(PACHIEVECONDITION pCondition,INT_PTR nConditionCount,INT_PTR progressPlus=1); //


	//订阅一个事件
	inline void SubscribeEvent(INT_PTR nEventID)
	{
		INT_PTR  nBytePos = nEventID >> 3; //第多少个BYTE
		INT_PTR  nBitPos = nEventID &7;  //一个BYTE里的第几个Bit
		if(nBytePos >= sizeof(m_subscribeEvent)) return;
		m_subscribeEvent[nBytePos] |= (0x1 << nBitPos);
		
	}

	//取消订阅一个事件
	inline void UnsubscribeEvent(INT_PTR nEventID)
	{
		INT_PTR  nBytePos = nEventID >> 3; //第多少个BYTE
		INT_PTR  nBitPos = nEventID &7;  //一个BYTE里的第几个Bit
		if(nBytePos >= sizeof(m_subscribeEvent)) return;
		m_subscribeEvent[nBytePos] &= (~(0x1 << nBitPos));
	}
	
	//一个成就事件是否订阅了
	inline bool IsEventSubscribed(INT_PTR nEventID)
	{
		INT_PTR  nBytePos = nEventID >> 3; //第多少个BYTE
		INT_PTR  nBitPos = nEventID & 7;  //一个BYTE里的第几个Bit
		if(nBytePos >= sizeof(m_subscribeEvent)) return false;
		return (m_subscribeEvent[nBytePos] & (0x1 << nBitPos) )? true:false;
	}

	//设置一个成就的分组是激活的
	inline void SetGroupOpen(INT_PTR nGroupId,bool flag)
	{
		INT_PTR  nBytePos = nGroupId >> 3; //第多少个BYTE
		INT_PTR  nBitPos = nGroupId & 7;  //一个BYTE里的第几个Bit
		if(nBytePos >= sizeof(m_achieveGroup)) return;
		if(flag)
		{
			m_achieveGroup[nBytePos] |= (BYTE) (0x1 << nBitPos);
		}
		else
		{
			m_achieveGroup[nBytePos] &= (~(0x1 << nBitPos));
		}
	}
	//成就分组是否是开启的
	inline bool IsGroupOpen(INT_PTR nGroupID)
	{
		INT_PTR  nBytePos = nGroupID >> 3; //第多少个BYTE
		INT_PTR  nBitPos = nGroupID &7;  //一个BYTE里的第几个Bit
		if(nBytePos >= sizeof(m_achieveGroup)) return false;
		return (m_achieveGroup[nBytePos] & (0x1 << nBitPos) )? true:false;
	}

	/*
	* Comments:检测成就分组的开启条件
	* Param PACHIEVEGROUP pGroup:成就分组的指针
	* @Return bool:可以开启返回true,否则返回false
	*/
	bool CheckGroupOpenCondition(PACHIEVEGROUP pGroup);

	//重刷分组的数据
	void ResetAchieveGroup( );
	//初始化徽章数据
	void InitBabgeData();

	//完成一个徽印 nId 徽章id
	void SetAchieveBabgeState(WORD nId);

	bool IsAllGroupTitleAchieveFinished(int groupId);
	void AddGroupTitle(int groupId);
	void DelGroupTitle(int groupId);
	void FreshAllGroupTitle();
	////////////新成就/////////////////////////////////////////
public:
	void SendAchieveData();
	void SendAchieveResult(WORD nId, BYTE bReslut);
	//设置一个成就完成了
	void SetAchieveFinished(INT_PTR nAchieveId,bool boGm = false);
	//设置已经领取过成就的奖励了
	void SetAchieveGiveAwards(INT_PTR nAchieveId);
	/*
	* Comments: 一个成就是否完成了
	* Param INT_PTR nAchieveId:成就的ID
	* @Return bool: 完成了返回true，否则返回false
	*/
	bool IsAchieveFinished(INT_PTR nAchieveId);
	//是否已经领取过成就的奖励了
	bool IsAchieveGiveAwards(INT_PTR nAchieveId);
	//获取成就CCLVariant值
	INT_PTR GetAchieveVar(CCLVariant* pVar);
	//设置成就CCLVariant值
	void SetAchieveVar(CCLVariant& pVar, const char* sName, INT_PTR nValue);




	//start of 新成就系统
	//成就完成度
	/*
	@nAchieveType :成就类型
	@nAchieveSubType: 成就子类型 --可以为0
	@nValue：值
	*/
	void ActorAchievementComplete(int nAchieveType, int nValue, int nAchieveSubType = 1, int nWay = 1);
	//设置某一个成就完成额度
	void SetAchieveCompleteValue(int AchieveType, int nValue);
	//累加某一个成就完成额度
	void AddAchieveCompleteValue(int AchieveType, int nValue);

	//对应完成度值
	int  GetCompleteConditionValue(ACHIEVECONDITIONCONFIG& condition);//

	//检测是否已满足条件
	bool CheckCompleteCondition(ACHIEVECONDITIONCONFIG& condition);
	//发送当前页签下的 成就信息
	VOID SendAchieveInfoByTab(CDataPacketReader& pack);

	void test();

	//领取成就id对应的奖励
	VOID GetAchieveAwardByTasKId(CDataPacketReader& pack);

	//获取当前成就任务完成进度进度
	int GetActorCompleteConditionValue(ACHIEVECONDITIONCONFIG& condition);

	//通过成就类型获取当前成就任务完成进度
	int GetAchieveCompleteValue(int AchieveType);

	//获取当前成就任务状态
	int GetAchieveState(int nTaskid);//>0表示完成
	//对比当前成就任务 的状态
	int GetAchieveStateCmpStr(int nTaskid, std::string nState);
	//改变当前成就任务状态
	void ChangeAchieveState(ACHIEVETASKCONFIG& info, int nState);
	//调用ActorAchievementComplete 检测当前类型活动成就是否能完成达成
	void CheckTypeAchievementStatue(int nAchieveType);

	int GetAchieveCompleteValueByType(int nAchieveType, int nAchieveSubType = 1);
	
	//end of 新成就系统

   //start of new 勋章系统
   bool medalcheckgoods(const std::vector<GOODS_TAG>& goods_vec);
   bool medalcheckachieve(const std::vector<int>& achievement_vec);
   bool  IsSatisfyMedalLevelUp(ELEM_MEDALCFG& condition,int) ;//是否满足勋章升级条件
   bool  MedalLevelupConsumeGoods(ELEM_MEDALCFG& condition) ; //扣除物品
   VOID GetActorMedalLevel(CDataPacketReader& pack); //客户端请求等级
   VOID MedalLevelUP(CDataPacketReader& pack);       //客户端请求升级


   //end of new 勋章系统

	void SendAchieveRedPoint();

	//成就完成状态
	int GetCompleteAchieveState(int nTaskid);
	//可领取
	void AddCanGet(ACHIEVETASKCONFIG& nInfo);

	void SubCanGet(ACHIEVETASKCONFIG& nInfo);

private:
	char m_achieveFinishState[MAX_ACHIEVE_DATA_SIZE];		//成就领取奖励的状态
	char m_achieveEventData[MAX_ACHIEVE_DATA_SIZE];	//成就事件的完成状态
	BYTE m_atomEvents[eMaxAtomEventID];				 //每个原子事件是否有几个成就事件在关注
	char m_subscribeEvent[ (MAX_ACHIEVE_EVENT_COUNT >> 3) +1];  //订阅了哪些事件,一个原子事件
	char m_titles[MAX_TITLE_DATA_SIZE]; //称号的数据
	int m_nAchieveEventDbUseCount; //成就事件的db使用了多少个int

	char m_achieveGroup[MAX_ACHIEVE_GROUP_BYTE_COUNT];   //成就分组是否开启的判断

	char m_BabgeState[MAX_ACHIEVE_EVENT_DATA_SIZE];		//成就徽章的数据

	bool m_hasSendInitData;    //是否把成就的初始化数据下发，如果没有下发的话不能下发成就数据
	CTimer<60000> m_1minute;   //1分钟的定时器

	std::map<int, int> m_nAchieveInfo; //成就完成进度情况
	std::map<int, std::vector<int> > nCanGetNum;
	bool isDataModify; //成就进度 数据改变
	bool isInitData ;//初始化
	//勋章等级使用staticcount系统记录
};

