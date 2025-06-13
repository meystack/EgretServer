#pragma  once
using namespace FDOP;
using namespace wylib::stream;

/****************************************
	头衔系统，这个类用来管理读取排行榜数据
*****************************************/

#define MAX_RANK_COUNT 4 //最大排行榜个数

#define MAX_TOP_TITLE_COUNT 3	//排行榜头衔数量

#define MAX_TOP_TITLE_MASTER 1	//法师
#define MAX_TOP_TITLE_WARRIOR 2 //战士
#define MAX_TOP_TITLE_WIZARD 3	//道士
#define MAX_TOP_TITLE_SKILL 8	//技高一筹
#define MAX_TOP_TITLE_EQUIP15 9	//兵甲初备
#define MAX_TOP_TITLE_EQUIP40 10	//兵强甲亮
#define MAX_TOP_TITLE_SKILL_2 20	//技惊四座
														//第一法师					第一战士			第一道士
static const char* szTopTitleRand[MAX_TOP_TITLE_COUNT] = {_T("TopTitleFirstMaster"),_T("TopTitleFirstWarrior"), _T("TopTitleFirstWizard")};

static const char *szTopMaxLevel	= "szTopMaxLevel";		//全服最大等级

//头衔排行榜信息结构
typedef struct tagTopTitleRankInfo
{
	unsigned int	m_nActorId;	//角色ID
	int				m_nRankId;	//排行榜ID
	int				m_nRank;	//排名
	int				m_nValue;	//值
	int				m_nParam;	//保留参数
	ACTORNAME   	m_sName;	//名字
	int   	        m_nJob;	//职业
	int         	m_nSex;	//性别
	int             n_yyVip;//yy超玩
	ACTORNAME   	m_sGuildName;	//名字
	tagTopTitleRankInfo()
	{
		memset(this, 0, sizeof(*this));
	}
}TOPTITLERANKINFO,*PTOPTITLERANKINFO;

//跨服排行榜结构
typedef struct tagRankData
{
	unsigned int	nActorId;		//角色ID
	char			sName[32];	//角色名
	unsigned int	nSex;		//玩家的性别
	int				nVocation;	//职业
	int				nServerId;	//服务器ID
	unsigned int	nValue;		//排行指定值

	tagRankData()
	{
		memset(this, 0, sizeof(*this));
	}
}RANKDATA, *PRANKDATA;

//这里表示排行榜类型
enum RankType 
{
	// rtBattlePower = 0,			//战力排行
	// rtWarriorBp,				//战士战力排行
	// rtMagicianBp,				//法师战力排行
	// rtWizardBp,					//道士战力排行
	rtAllLevel = 0,				//全职业等级排行
	rtWarriorLevel,				//战士等级排行
	rtMagicianLevel,			//法师等级排行
	rtWizardLevel,				//道士等级排行
	// rtCoin,						//金币排行
	// rtGirlCharm,				//女性魅力排行榜
	// rtBoyCharm,					//男性帅气排行榜
	// rtHeroLevel,				//英雄等级排行
	// rtWarriorHeroLevel,			//战士英雄等级排行
	// rtMagicianHeroLevel,		//法师英雄等级排行
	// rtWizardHeroLevel,			//道士英雄等级排行
	// rtWarriorHeroBp,			//战士英雄战力排行
	// rtMagicianHeroBp,			//法师英雄战力排行
	// rtWizardHeroBp,				//道士英雄战力排行
	// rtMagicItemBp,				//法宝战力排行
	// rtBindCoin,					//绑定金币排行
	// rtSoldierSoul,				//兵魂排行榜		
	// rtWing,						//翅膀排行
};

typedef CVector<TOPTITLERANKINFO>  RANKUSERLIST; //排行榜玩家的列表

class CTopTitleMgr:
	public CComponent
{
public:

	/* 
	* Comments: 初始化
	* @Return void:  
	*/
	bool Initialize();

	/*
	* Comments: 定时检测
	* Param TICKCOUNT nTickCount:
	* @Return void:
	* @Remark:
	*/
	void RunOne(TICKCOUNT nTickCount);

	/* 
	* Comments: 从数据库获取所有有关头衔系统的排行榜列表
	* @Return void:  
	*/
	//void Load();

	/* 
	* Comments: 请求数据服务器保存头衔排行榜
	* @Return void:  
	*/
	//void UpdateBaseRankData();

	/* 
	* Comments: 请求数据服务器保存排行榜数据
	* @Return void:  
	*/
	//void UpdateRankData();
	
	/* 
	* Comments: 数据服务器返回数据,这里读取所有有关头衔系统的排行榜列表
	* Param INT_PTR nCmd: 消息命令
	* Param char * data: 返回数据
	* Param SIZE_T size: 数据大小
	* @Return void:  
	*/
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);
	//void SendTopRankData(CActor * m_pEntity);

	/* 
	* Comments: 获取排行榜名单列表
	* @Return RANKUSERLIST&:  
	*/
	inline RANKUSERLIST* GetRankUserList() 
	{
		return m_rankActorList;
	}

	/* 
	* Comments: 设置在线玩家头衔
	* @Return bool:  
	*/
	bool SetOnlineActorHeadTitle();

	//增加引用计数，函数返回增加后的引用计数
	virtual int AddTopRankRef(){ m_nRefCount++; return m_nRefCount; }


	//减少引用计数，函数返回减少后的引用计数
	virtual int ReleaseTopRankRef() { 	
		int nRet = --m_nRefCount;
		// 计数少于等于0 读取头衔数据更新头衔
		if (nRet <= 0)
			// Load();
		return nRet;
	}

	//重置玩家的离线玩家信息
	void ResetOfflineUser();

	//重置NPC外观属性
	void ResetNpcStatue();

	/* 
	* Comments: 设置NPC雕像
	* Param INT_PTR nType: 雕像类型
	enum NpcStatueType 
	{
		stNormal = 0,
		stRsCastellan = 1,	//本服沙巴克城主雕像
		stCsCastellan = 2,	//跨服沙巴克城主雕像
		stRsVocation = 3,	//本服职业雕像
		stCsVocation = 4,	//跨服职业雕像
	};
	* Param INT_PTR nActorId: 设置actorid actorid==0空雕像
	* Param LPCTSTR sName: 设置名字
	* Param INT_PTR nSex: 性别
	* Param INT_PTR nParam: 辅助参数 当nType = 3或4时代表职业ID
	* @Return bool:  
	*/
	bool SetNpcStatue(INT_PTR nType, INT_PTR nActorId, LPCTSTR sName, INT_PTR nSex, INT_PTR nParam = 0);
	
	//更新职业雕像
	void UpdateVocNpcStatue(INT_PTR nType);

	//SSDB返回数据
	void OnSSDBReturnData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);

	//其他逻辑服转发的消息
	//nServerId 其他服务器ID
	void OnOtherLogicServerRetData(int nCmd, int nOtherServerId, CDataPacketReader &reader);

public:

	//更新各个职业的排序
	void UpdateRawRankList();

	//发送到SS保存战力排行
	void SaveCsRank();
	//加载跨服战力排行数据
	void LoadCsRank();
	//清除跨服战力排行数据
	void ClearCsRank(INT_PTR nServerId);

	//广播所有逻辑服将雕像榜单数据发到session
	void BroadLogicToSaveCsRank();
	//广播所有逻辑服重新取下榜单
	void BroadLogicToLoadCsRank();

	//装载跨服玩家的离线消息
	void LoadUserOfflineData(int nServerId, unsigned int nActorId);

	//返回跨服玩家的离线下消息
	void ReturnUserOfflineData(int nDestServerId, unsigned int nActorId);

	//设置角色离线数据
	void SetUserOfflineData(CDataPacketReader &reader);

	//在m_CsRankList获取serverId
	int GetServerIdByCsRank(unsigned int nActorId);

	bool SetRankHeadTitle(CActor *pActor);

	//获取排行榜的第几名的角色id
	unsigned int GetActorIdByCsRank(INT_PTR nRankIndex,int nRank);
	PTOPTITLERANKINFO GetActorInfoByCsRank(INT_PTR nRankIndex,int nRank);
	void maintainTopTitle();
	void SetTopMaxLevel(int nLevel);
	int GetTopMaxLevel() const {return nMaxTopLevel;};
	int GetTopLevel();

public:
	const static int g_nRadius = 20;	//可视更新半径
	CTopTitleMgr();
	~CTopTitleMgr();

	int nMaxTopLevel = 1;//最大等级
	
private:
	RANKUSERLIST						m_rankActorList[MAX_RANK_COUNT];	// 排行榜列表
	int									m_nRefCount;						// 引用计数，用于不同排行榜之间更新进行计数
	
	TICKCOUNT							m_nDelay;							// 雕像延时更新时间
	
	CVector<RANKDATA>					m_RawRankList;						// 本服职业战力排行
	CVector<RANKDATA>					m_CsRankList;						// 跨服职业战力排行
	int                                 m_nUpdateTime = 0;                  // 更新时间
	bool                                m_bInit = false;                        //更新状态
};
