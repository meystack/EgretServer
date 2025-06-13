#pragma once

//职业竞技子系统
class CCombatSystem:public CEntitySubSystem<enCombatSystemID, CCombatSystem, CActor> 
{
public:
	CCombatSystem();
	~CCombatSystem(){};

/****----------------覆盖父类的函数集-----------------***/

	virtual bool Initialize(void *data, SIZE_T size);

	virtual void OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader &reader);

	virtual void Save(PACTORDBDATA pData);

	virtual void OnTimeCheck(TICKCOUNT nTickCount);

	virtual void OnEnterGame();

	virtual void ProcessNetData(INT_PTR nCmd, CDataPacketReader &packet);

/****----------------协议处理的入口函数集-----------------***/

	//下发基本信息
	void SendCombatBaseInfo();

	//筛选对手
	void PickRankPlayer(int nActive);

	//开始挑战
	void StartCombatChallage(int nActorId, int nRank);

	//清除cd时间
	void ClearCombatCd();

	//兑换排名奖励
	void RechargeRankAward(int nAwardId);

	//请求战报
	void GetCombatLog();

	//请求排行榜
	void GetCombatRank(int nCurrentPage);

	//请求已兑换奖励信息
	void GetRankAwardList();

/****----------------其他逻辑功能的函数集-----------------***/

	//脚本操作触发入口
	void TriggerEvent(INT_PTR nSubEvent, INT_PTR nParam1=-1, INT_PTR nParam2=-1, INT_PTR nParam3=-1, INT_PTR nParam4=-1);

	//添加职业宗师竞技战报
	void AddToCombatLog(int nType, char *strName, int nActorId, int nRank, int nResult);

	//添加到职业宗师竞技榜
	void AddToCombatRank(int nActorId,int nValue);

	//更新自身职业宗师竞技榜信息
	void UpdateCombatRanking();

	//筛选对手执行接口
	void PickRankPlayerImpl();

	//发送筛选对手
	void SendPickRankPlayer();

	//交换排名
	void SwapCombatRank(int nActorId, int nTagetId);

	//挑战结束操作
	void ChallegeOverOp(int nResult, int nActorId, char* strName, int nIsReal);

	//获取我的排名
	int  GetMyCombatRank();


	//在线通知被挑战方
	void ChallegeNotice(int nResult, int nActorId, char* strName, int nMyRank);

	//离线消息处理战报
	void OfflineCombatOp(int nResult, int nActorId, char* strName, int nMyRank);

	//加载职业宗师竞技基本信息
	void LoadCombatInfoFromDb();
	void LoadCombatInfoFromDbResult(CDataPacketReader &reader);

	//保存职业宗师竞技基本信息
	void SaveCombatGameInfo();

	//加载职业宗师竞技战报
	void LoadCombatRecordFromDb();
	void LoadCombatRecordFromDbResult(CDataPacketReader &reader);

	//保存职业宗师竞技战报
	void SaveCombatRecord();

	//每天清除信息
	void OnNewDayClear();

	//改变积分
	void ChangeCombatScore(int nValue);

	//获取积分
	int GetCombatScore() { return m_nCombatScore; }

	//等级提升操作
	void OnLevelUp();

public:

	enum tagCombatRecordResult
	{
		enFail			= -1,	//失败
		enFailAndSwap	= -1,	//失败且交换排名
		enSucc			= 1,	//胜利
		enSuccAndSwap	= 2,	//胜利且交换排名
	};

	//战报信息
	typedef struct tagCombatRecord
	{
		int		nType;					//攻防类型：0攻 1防
		int		nResult;				//结果(-1失败  -2失败排名变化  1胜利  2 胜利排名变化)
		int		nActorId;				//id
		char	strName[32];			//姓名
		int		nRank;					//排名
		
	}COMBATRECORD, *PCOMBATRECORD;

private:

	int		m_nJobType;				//职业类型
	int		m_nCurrentRankLevel;	//当前排名
	int		m_nMaxRankLevel;		//历史最高排名
	int		m_nLastRankLevel;		//上次结算排名
	int		m_nCombatScore;			//战力积分
	int		m_nChallengeCount;		//挑战次数
	int		m_nPickCount;			//筛选次数
	unsigned int	m_nClearTime;	//清零时间
	bool	m_bCdFlag;				//冷却标志

	
	CVector<COMBATRECORD>		m_CombatRecordList;		//战报

	int		m_nSelectedIndex[MAXPICKITEMNUM + 1];	//筛选对手索引列表
	bool	m_bPickSwitch;			//筛选开关
	bool	m_bSaveInfoFlag;		//是否保存基本信息
	bool	m_bSaveLogFlag;			//是否保存战报

};