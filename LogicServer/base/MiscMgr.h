#pragma once

static const char *szSupplyBatBeginTime		= "supplyBatBeginTime";		//布局争夺战开启时间

//记录击杀玩家的相关时间记录
typedef struct tagKillerData
{
	unsigned int nKillerActorId;	//击杀者的ActorId
	unsigned int nBeKilledActorId;	//被击杀者id
	unsigned int nKillGetExploitTime;//击杀获得战绩时间，击杀同一目标在30分钟内只能获得一次战绩奖励。
	tagKillerData()
	{
		memset(this,0, sizeof(*this));
	}
}KillerData;
class CMiscMgr:
	public CComponent
{

public:
	CMiscMgr();
	~CMiscMgr();
public:
	/*
	* Comments:定期检查
	* @Return VOID:
	*/
	VOID RunOne(CMiniDateTime& minidate,TICKCOUNT tick);

	VOID Init();

	/* 
	* Comments:更新新脚本
	* Param LPCTSTR sFileName:
	* @Return bool:
	*/
	bool HotUpdateScript(LPCTSTR sFileName);

	//************************************
	// Method:    KickAllActorByLevel 后台命令T人与禁用角色
	// FullName:  CMiscMgr::KickAllActorByLevel
	// Access:    public 
	// Returns:   bool
	// Qualifier:
	// Parameter: int nCircle 转数
	// Parameter: int nLevel  等级
	// Parameter: bool boDisApear 是否删除
	//************************************
	bool KickAllActorByLevel(int nCircle, int nLevel, bool boDisApear = false);

	/*
	* Comments:判断当前时间是否在这个时间段内
	* Param int nStartHour:时间段开始的小时
	* Param int nStartMin:分
	* Param int nStartSec:秒
	* Param int nEndHour:时间段结束的小时
	* Param int nEndMin:分
	* Param int nEndSec:秒
	* @Return bool:如果在这个时间段内，返回true，否则false
	*/
	bool isInTimeRange(int nStartHour,int nStartMin,int nStartSec,int nEndHour,int nEndMin,int nEndSec);

	//************************************
	// Method:    isInDateRange 判断是否在指定时间
	// FullName:  CMiscMgr::isInDateRange
	// Access:    public 
	// Returns:   bool
	// Qualifier:
	// Parameter: int nStartYear  
	// Parameter: int nStartMonth
	// Parameter: int nStartDay
	// Parameter: int nStartHour
	// Parameter: int nStartMin
	// Parameter: int nStartSec
	// Parameter: int nEndYear
	// Parameter: int nEndMonth
	// Parameter: int nEndDay
	// Parameter: int nEndHour
	// Parameter: int nEndMin
	// Parameter: int nEndSec
	//************************************
	bool isInDateRange(int nStartYear,int nStartMonth,int nStartDay,int nStartHour,int nStartMin,int nStartSec,int nEndYear,int nEndMonth,int nEndDay,int nEndHour,int nEndMin,int nEndSec);

	//************************************
	// Method:    DoScriptFunction 调用脚本
	// FullName:  CMiscMgr::DoScriptFunction
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: LPCTSTR szFuncName  方法名
	//************************************
	void DoScriptFunction(LPCTSTR szFuncName);

	//************************************
	// Method:    DoScriptFunction 调用脚本
	// FullName:  CMiscMgr::DoScriptFunction
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: LPCTSTR szFuncName 方法名
	// Parameter: int nFlag          参数
	//************************************
	void DoScriptFunction(LPCTSTR szFuncName, int nFlag);

	//************************************
	// Method:    DoScriptFunction 调用脚本
	// FullName:  CMiscMgr::DoScriptFunction
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: CActor * pActor 人物指针
	// Parameter: LPCTSTR szFuncName  方法名
	//************************************
	void DoScriptFunction(CActor* pActor, LPCTSTR szFuncName);

	//************************************
	// Method:    DoScriptFunction 调用脚本
	// FullName:  CMiscMgr::DoScriptFunction
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: CActor * pActor 人物指针
	// Parameter: LPCTSTR szFuncName 方法名
	// Parameter: int nFlag  参数
	//************************************
	void DoScriptFunction(CActor* pActor, LPCTSTR szFuncName, int nFlag);

	//************************************
	// Method:    DoScriptFunction
	// FullName:  CMiscMgr::DoScriptFunction 调用脚本
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: CActor * pActor 人物指针
	// Parameter: LPCTSTR szFuncName 方法名
	// Parameter: int nFlag 参数一
	// Parameter: int nParam 参数二
	//************************************
	void DoScriptFunction(CActor* pActor, LPCTSTR szFuncName, int nFlag, int nParam, int nParam2 = 0);

	//************************************
	// Method:    GetDaysBeforeCombineServer 获取合区前第几天
	// FullName:  CMiscMgr::GetDaysBeforeCombineServer
	// Access:    public 
	// Returns:   int 合区前第几天
	// Qualifier:
	//************************************
	int GetDaysBeforeCombineServer();

	
	void SetServerTempCombineTime(unsigned int time, unsigned int beginTime)
	{
		m_serverTempCombineTime = time;
		m_serverBenginCombineTime = beginTime;
	}

	unsigned int GetServerTempCombineTime(unsigned int &beginTime)
	{
		beginTime = m_serverBenginCombineTime;
		return m_serverTempCombineTime;
	}

	/*
	* Comments:获取一个取模nModule的随机数
	* Param int nModule: 模数
	* @Return int: 返回取模后的随机数,从0到nModule-1
	*/
	unsigned int getRandomNum( int nModule)
	{
		if( nModule <=0 )return 0;
		return   ( (unsigned int) wrandvalue() + (unsigned int)_getTickCount() ) % (unsigned int) nModule ;
	}

	int getRandomNum( int nMin, int nMax)
	{
		int nModule = nMax - nMin;
		int nMid	= (nMax - nMin)/2;
		int nNum = getRandomNum( nModule);
		return nNum - nMid;
	}

	//是否互为好友
	bool IsFriendEach(CActor* pActor1, CActor* pActor2);

	CScene* GetScenePtrById( int nSceneId);

	//获取系统静态变量
	INT_PTR GetSystemStaticVarValue(LPCTSTR szVarName,INT_PTR nDefaultValue=0);
	//设置系统静态变量
	void SetSystemStaticVarValue(LPCTSTR szVarName, INT_PTR nValue);

	//获取系统动态变量
	INT_PTR GetSystemDyanmicVarValue(LPCTSTR szVarName,INT_PTR nDefaultValue);

	//获取某个排行榜的达到限额要求的数量
	int GetRankLimitCount(CRanking *pRanking, int nLimit=0);

	/* 
	* Comments: 设置补偿方案
	* Param unsigned int nId: 补偿方案ID 如果为<=则清空补偿数据
	* Param unsigned int nOverTime: 补偿时间
	* Param bool bClearRank: 是否情况领取补偿名单
	* @Return int: 返回设置方案ID
	*/
	int SetCompensateVar(int nId, unsigned int nBeginTime=0, unsigned int nLastTime=0, 
		int nLevelLimit=0, char* szTitle="", bool bClearRank=false);

	/* 
	* Comments: 获取补偿方案
	* Param unsigned int nId: 补偿方案ID 
	* Param unsigned int nOverTime: 补偿时间
	* @Return bool:  
	*/
	void GetCompensateVar(int &nId, unsigned int &nBeginTime, unsigned int &nOverTime);

	char* GetCompensateTitle();

	bool IsNearBySceneNpc(CActor *pActor, int nSceneId, int nNpcId );

	//后台禁止玩家
	bool OnBackForbidUserMis(int Actorid, INT_PTR nDuration);
	bool OnBackUnForbidUserMis(int Actorid);
	bool OnBackClearAllMsg();//清理所有消息

	// 后台 添加/删除自定义称号
	bool OnBackAddCustomTitle(unsigned int nActorId, int nCustomTitleId);
	bool OnBackDelCustomTitle(unsigned int nActorId, int nCustomTitleId);
	
	VOID OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);
public:
	//增加击杀记录
	KillerData*  AddKillerData(unsigned int nKillerActorId, unsigned int nBeKilledActorId);
	KillerData* GetKillData(unsigned int nKillerActorId, unsigned int nBeKilledActorId);
private:
	void CheckScriptTimer(CMiniDateTime& minidate,TICKCOUNT tick); //定时检测定时器
private:
	CMiniDateTime			m_tomorrow;	//明日凌晨的时间
	CMiniDateTime			m_sendlogout;	//发送登出日志时间
	CMiniDateTime			m_sendlogin;	//发送登入日志时间

	CTimer<60000>           m_1minuteTimer;     //1分钟的定时器
	CTimer<300000>			m_5minuteTimer;		//5分钟的定时器

	TICKCOUNT				m_nBackStageKillActorTime;//后台使用踢人与禁用功能的时间
	unsigned int			m_serverTempCombineTime;	// 服务器合区时间(用于后台设置开启某些系统用)
	unsigned int			m_serverBenginCombineTime;	// 服务器开启合区时间
	CVector<KillerData>		m_killerData;//记录击杀时间
};
