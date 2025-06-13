#pragma once
/***************************************************************/
/*
/*  杂七杂八子系统
/*  处理游戏里的各种杂项需求
/*  防沉迷在这里
/*  
/***************************************************************/

#define  MAXGAMESETTING     16

#define GAME_SETTING_TYPE_BASE     1           //游戏设置-基本
#define GAME_SETTING_TYPE_BASE_AUTO_INVITETEAM 4//自动接受组队邀请和入队邀请
static LPCTSTR  szDayHadExchangeExploitCount	= "dayExchangeExploitCount";		//每日已兑换战绩次数

enum MembershipCommType //会员属性请求命令
{
    eMembershipComm_NULL = 0,     
    eMembershipComm_White = 5,    //百卡 
    eMembershipComm_Green = 6,    //绿卡 
    eMembershipComm_Blue = 7,     //蓝卡
    eMembershipComm_Violet = 8,   //紫卡 
    eMembershipComm_Orange = 9,   //橙卡 
    eMembershipComm_OrangeStar = 12,    //橙星卡 
    eMembershipComm_OrangeMoon = 13,    //橙月卡 
    eMembershipComm_MAX, 
};
enum MembershipLvType //会员等级属性
{
    eMembershipLv_NULL = 0,     
    eMembershipLv_White = 3,    //百卡 
    eMembershipLv_Green = 4,    //绿卡 
    eMembershipLv_Blue = 5,     //蓝卡
    eMembershipLv_Violet = 6,   //紫卡 
    eMembershipLv_Orange = 7,   //橙卡 
    eMembershipLv_OrangeStar = 8,     //橙星卡
    eMembershipLv_OrangeMoon = 9,     //橙月卡
    eMembershipLv_MAX, 
};

class CMiscSystem:
	public CEntitySubSystem<enMiscSystemID,CMiscSystem,CActor>
{
public:
	typedef CEntitySubSystem<enMiscSystemID,CMiscSystem,CActor> Inherid;

	//属性排行榜
	typedef struct tagRankItem
	{
		WORD nConfigIndex;	//排行榜属性配置的配置索引
		WORD nRankIndex;	//排名索引
		int	 nPoint;		//分数
	}RANKITEM, *PRANKITEM;

public:
	CMiscSystem()
	: bSignUpToday(false)
	, m_fcmIsOpen(true)
	, m_isAdult(true) 
	{
		m_hasSendLoginTipmsg=false;  //是否已经发送登陆的提示
		m_hasSend1HourTipmsg=false; //是否已经发送1小时的提示
		m_hasSend3HourTipmsg=false; //是否已经发送3小时的提示
		m_hasSend5HourTipmsg=false; //是否已经发送5小时的提示
		//m_isAdult           =true; //是否是成年人
		//m_fcmIsOpen         =true;  //反沉迷系统是否开启了

		//bSignUpToday		=false;

		bNeedSaveGameSetData = false;
		m_isLauncherLogic	= false;
		memset(m_nGameSetData,0,sizeof(m_nGameSetData));
		ResetMiscData();
	}

	virtual ~CMiscSystem() {}

	/*处理网络数据
	 * nCmd 消息号
	 * packet 网络包
	 */
	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );
	
	//重载初始化函数，暂时的话所有的都是GM
	bool Initialize(void *data,SIZE_T size);

	//客户端发来平台防沉迷信息重新初始化
	bool ReInitialFCM(BYTE FcmTag); 

	//销毁
	virtual void Destroy();
	
	void Save(PACTORDBDATA  pActorData);

	//Comments:玩家登出的时候触发
	void OnActorLoginOut();
	
	//定时检测
	void OnTimeCheck(TICKCOUNT  nTickCount);
	
	//获取当前的防沉迷系统的经验和金钱的减半,没有纳入防沉迷是1，否则过时间了会减
	 int GetFcmExpMoneyRate();
	
	//攻击了一次别人，在这里计算连斩
	//void OnAttackOther(CAnimal * pEntity,unsigned int nDurKillTime); 

	//角色各子系统初始化完毕，向客户端发送创建主角的数据包后，调用该接口
	virtual void OnEnterGame();   

	// 购买月卡（1.免费特权，2.月卡，3.大药月卡，4.永久卡）
	void OnBuyMonthCard(BYTE mtype);

	// 领取月卡日常奖励（1.免费特权，2.月卡，3.大药月卡，4.永久卡）
	void OnGetMonthCardAward(BYTE mtype);

	bool IsHasFreePrivilege();//是否拥有免费特权
	bool IsHasMonthCard();//是否拥有月卡
	bool IsHasMedicineCard();//是否拥有大药月卡
	bool IsHasForverCard();//是否拥有永久卡

	bool IsHasShouChongCard();//是否是拥有首充会员卡

	bool IsHasWhiteCard() ;//是否拥有白卡会员
	bool IsHasGreenCard() ;//是否拥有绿卡会员
	bool IsHasBlueCard(); //是否拥有蓝卡会员
	bool IsHasPurpleCard(); //是否拥有紫卡会员
	bool IsHasOrangeCard(); //是否拥有橙卡会员
	//防沉迷
	bool IsAdult(){return m_isAdult;};  

	//特权回收
	int GetRecoverGetGold(int nType, int nId);
	//处理回收特权
	void DealRecover();

	//获取色卡会员最高等级：1：白  2：绿  3：蓝  4：紫  5：橙 
	//非色卡会员返回0 
	int GetMaxColorCardLevel(); 

	//获取色卡会员最大经验衰减百分数, 如70%则返回70 , 非色卡会员或者使用默认衰减的返回0 ；
	int  GetMaxExpDampColorCard();

	//获取首冲卡&&色卡会员的活动泡点经验加成百分数，活动，普通泡点地图, 普通打怪用的都是这个
	int GetSuperRightAtvPaoDianPlus();

	//获取寄售行最大售卖个数
	int GetJiShouHangMaxCount();

	//通过金钱的类型获取金钱的名字
	static LPCSTR GetMoneyName(INT_PTR nMoneyType);

	//通过奖励的类型获取奖励的名字
	static LPCSTR GetAwardName(INT_PTR nType);

	/*
	* Comments:给客户端发送倒计时时间，客户端收到这个消息，显示一个倒计时的特效
	* Param INT_PTR nTime:倒计时时间，单位：秒，不超过10s
	* Param BYTE bFlag :  0 有"开始"字， 1 没有文字
	* @Return void:
	*/
	void SendCountdown(INT_PTR nTime,int  nFlag = 0, int nType = 1);


	// 设置杂项数据
	inline void SetMiscData(enMiscDataType flagType, int nVal)
	{
		if ((flagType >= (enMiscDataType)0) && (flagType < enMiscDataMax))
		{
			m_nMiscData[flagType] = nVal;
		}
	}
	// 获取杂项数据
	inline int GetMiscData(enMiscDataType flagType)
	{
		if ((flagType >= (enMiscDataType)0) && (flagType < enMiscDataMax))
			return m_nMiscData[flagType];
		return 0;
	}

	//加载游戏设置的数据
	void LoadGameSetData();

	//读取游戏设置的数据
	void LoadGameSetDataResult(CDataPacketReader &reader);

	//玩家改名字
	void OnChangeActorName(CDataPacketReader &packet);

	//改名字结果
	void OnChangeActorNameResult(INT_PTR nErrorCode,CDataPacketReader &reader);

	/// 下发游戏设置的数据
	void SendGameSetRsult();

	/// 保存游戏设置
	void SaveGameSetDataToDb();

	/*
	* Comments:处理数据服务器返回的内容
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:
	* Param CDataPacketReader & reader:
	* @Return VOID:
	*/
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	//初始属性排行榜
	void InitRankPropData();
	//更新属性排行榜
	void UpdateRankPropData(int nConfigIndex, int nRankIndex, int nPoint);
	//计算属性排行榜属性
	bool CalcRankPropProperty(CAttrCalc &calc);

	//下发服务器的合区时间(用于后台设置开启某些系统用)
	void SendTempCombineTime();

	//跨天清理签到
	void ClearCheckSignIn();

	//改名字后的处理
	void AfterChangeNameOp(const char* oldName);

	//播放特效消息
	void PlaySrcEffect(int nEffId, int nSec, bool boWorld = false, int nLevel = 0);


	/* 
	* Comments:
	* Param byte nType:
	* Param int nParam:
	* @Return void:
	*/
	void SendCustomEffect(byte nType, int nParam);

	/* 
	* Comments:支持打开或关闭大图标
	* Param byte nFlag:开关，1打开，0关闭
	* Param WORD nIconId:图标id
	* @Return void:
	*/
	void SendTurnOnOffIcon(byte nFlag, WORD nIconId);

	//检查是否用登陆器登陆
	void CheckLauncherLogin();

	void SendTheLineActivity();
	void SendWuYiGoldEggActivity();

	//获取游戏设置值
	int GetGameSetting(int nType, int nIdx);


	void SendActorInfoByNameFuzzy(unsigned char nType, char* sName);

	//登陆
	void OnUserLogin();
	//下发开服时间
	void SendOpenServerDays();
	
	//下发每转人数
	void SendCircleCount();

	/*
	* Comments:触发一个事件
	* Param INT_PTR nEventID: 事件的ID
	* Param INT_PTR nSubEvent: 子事件的ID
	* Param INT_PTR nParam1:参数1，不需要就直接按默认参数为-1
	* Param INT_PTR nParam2:参数2，不需要就直接按默认参数为-1
	* Param INT_PTR nParam3:参数3，不需要就直接按默认参数为-1
	* Param INT_PTR nParam4:参数4，不需要就直接按默认参数为-1
	* @Return void:
	*/
	void TriggerEvent(INT_PTR nEventID,INT_PTR nSubEvent,INT_PTR nParam1=-1,INT_PTR nParam2=-1,INT_PTR nParam3=-1,INT_PTR nParam4=-1);

	//充值相关 
	/* 
	* Comments:下发每日充值
	* Param unsigned int nLeftTime:
	* @Return void:
	*/
	//void SendEveryDayPay(byte nType, unsigned int nLeftTime, unsigned int nMyPayCount = 0, byte nActId = 0);
	 
private:
	//查看玩家的信息
	void OnViewActorInfo(CDataPacketReader &packet)	;

	//保存游戏设置的数据
	void SaveGameSetData(CDataPacketReader &packet);

	//获取服务器的时间
	void OnGetServerTime();

	/*
	* Comments:发送排行榜的数据
	* Param LPCSTR sName:排行榜的名称
	* @Return void:
	*/
	void SendRankingData(LPCSTR sName);
	
	void OnGetYuanbaoPlant(); //获取元宝的种植的数据

	// 重置杂项数据
	inline void ResetMiscData()
	{
		ZeroMemory(m_nMiscData, sizeof(m_nMiscData));
	}

	//改名时更新排行中的名字
	bool ChangeNameInRank(const char * sRankingName, int nRankSubIdx = 0);
	
//跨服相关
public:
    void OnCrossInitData(GameUserDataOther &data);
    void SendMsg2CrossServer(int nType);

private:      
	bool m_isAdult:1;             ///< 是否是成年人
	bool m_fcmIsOpen:1;           //反沉迷是否开启
	bool m_hasSendLoginTipmsg:1;  //是否已经发送登陆的提示
	bool m_hasSend1HourTipmsg:1;  //是否已经发送1小时的提示
	bool m_hasSend2HourTipmsg:1;  //是否已经发送2小时的提示
	bool m_hasSend2Hour55MinuteTipmsg:1;  //是否已经发送2小时55分的提示
	bool m_hasSend3HourTipmsg:1;  //是否已经发送3小时的提示
	bool m_hasSend5HourTipmsg:1;  //是否已经发送5小时的提示
	bool m_isLauncherLogic:1;		//是否用登陆器登陆

	unsigned int m_fcmStartTime;   //该账户开始防沉迷的短时间	
	
	//YBDATA m_ybpPlans[3];         //玩家的元宝的种植的数据
	int	 m_nMiscData[enMiscDataMax]; // 存放一些辅助数据，例如增加日常任务次数、日常副本任务等等。

	int  m_nGameSetData[MAXGAMESETTING];	//游戏设置保存数组
	bool bNeedSaveGameSetData;				//是否需要保存游戏设置的数据到数据库

	bool bSignUpToday;						//今天是否已经签到

	CVector<RANKITEM> m_RankPropData;		//属性排行榜保存的临时排行数据

	CUserItem* m_pUserTreasureItem;			//要投放的聚宝盆物品  
};

