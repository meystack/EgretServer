
/*
*用于管理游戏里职业和阵营的平衡
*比如战士、法师、道士，在随机选择的时候选择最小的推荐给玩家，避免职业失衡
*同样的是阵营的，在游戏初始化的时候装载，每添加一条记录在内存里更新，定期存盘一次
*避免每次查询职业的平衡都去数据表里查找
*/
#pragma once

// 玩家阵营分布
struct PlayerZYDistribute
{
	PlayerZYDistribute()
	{
		Reset();
	}

	void Reset()
	{
		/*m_nServerIdx = 0;
		for (INT_PTR i = 0; i < zyMax; i++)
			m_anPlayerCount[i] = -1;*/
		ZeroMemory(this, sizeof(*this));
	}

	INT_PTR		m_nServerIdx;			// 服务器Id
	INT_PTR		m_anPlayerCount[zyMax];	// 记录三个阵营的玩家数量
};
// 玩家职业分布
struct PlayerJobDistribute
{
	PlayerJobDistribute()
	{
		Reset();
	}

	void Reset()
	{
		/*m_nServerIdx = 0;
		for (INT_PTR i = 0; i < enMaxVocCount; i++)
			m_anPlayerCount[i] = -1;*/
		ZeroMemory(this, sizeof(*this));
	}
	INT_PTR		m_nServerIdx;					// 服务器Id
	INT_PTR		m_anPlayerCount[enMaxVocCount]; // 记录各个职业的玩家数量
};

struct PlayerNameDistribute
{

	int		nServerIdx;					// 服务器Id
	int		guildid;					//  
	char	sName[32];					// 记录各个职业的玩家数量

	PlayerNameDistribute()
	{
		ZeroMemory(this, sizeof(*this));
	}
};

class CJobZyCountMgr
{
public:
	CJobZyCountMgr();

	//设置参数
	void SetParam(CSQLConenction* pSql)
	{
		m_SQLConnection = pSql;
	}

	// DB服务器启动的时候自动加载玩家阵营分布数据
	VOID LoadZyInitData();
	// 保存玩家阵营分布数据（定时保存）
	VOID SaveZyData();

	// 新玩家创号
	void OnNewPlayerCreated(int nSrvIdx, tagZhenying zy, tagActorVocation voc,char sName[32]);	

	//当新行会创建的时候
	void OnNewGuildNameCreate(int nSrvIdx, char *sName, int nGuildId);
		
	// DB服务器启动的时候自动加载玩家职业分布数据
	VOID LoadJobInitData();
	// 保存玩家职业分布数据（定时保存）
	VOID SaveJobData();


	// 获取优选职业（当前最少的职业）
	int GetPriorityJob(int nSrvIdx);

	// 获取可以选择的阵营（阵营人数低于35%的才能选择），返回按位的掩码
	int GetOptionalZy(int nSrvIdx);
	
	// 获取优选阵营（当前最少的阵营）
	tagZhenying GetPriorityZY(int nSrvIdx);

	//是否装载了职业数据
	bool   HasLoadJobData()
	{
		return m_bLoadJobData ;
	}

	//是否装载了正营数据
	bool HasLoadZyData()
	{
		return m_bLoadZyData;
	}

	//是否装载了玩家名称数据
	bool HasLoadActorNameData()
	{
		return m_bLoadNameData;
	}

	// DB服务器启动的时候自动加载玩家名称数据
	void LoadActorNameInitData();

	//装载行会列表
	void LoadGuildNameInitData();

	//判断是否名称已存在,如果判断在所有的几个服就输入-1
	bool IsActorNameExist(char* sName,int nServerindex=-1);

	//修改名字
	void ChangeNameToList(char* sName,char* srcName,int nServerIndex);

	//名字是否存在
	bool IsGuildNameExist(char* sName,int nServerindex=-1);
	//删除行会
	bool RemoveGuildById(int Guild);

	//int GetStrLenUtf8(LPCTSTR sName);
private:
	
	// 用于创建角色选择阵营和职业	
	bool							m_bLoadJobData;// 是否已经加载了初始的阵营、职业分布数据（针对DB断开重连的情况）

	bool							m_bLoadZyData;// 是否已经加载了初始的阵营、职业分布数据（针对DB断开重连的情况）

	wylib::container::CBaseList<PlayerZYDistribute>		m_vecZyDist;	// 玩家阵营分布
	wylib::container::CBaseList<PlayerJobDistribute>	m_vecJobDist;	// 玩家职业分布
	bool							m_bZyDataDirty;
	bool							m_bJobDataDirty;
	CCSLock							m_zyLock;
	CCSLock							m_jobLock;
	CSQLConenction*                 m_SQLConnection;	//数据库连接对象

	wylib::container::CBaseList<PlayerNameDistribute>	m_vecNameDist;	// 已使用的玩家名称

	
	wylib::container::CBaseList<PlayerNameDistribute>	m_guildNameDist;	//已使用过的行会的名称
	
	bool							m_bLoadNameData;// 是否已经加载了初始的玩家名称数据
	bool                            m_bLoadGuildNameData; //是否装载过行会数据

	CCSLock							m_nameLock;
	CCSLock							m_guildNameLock;  //行会名字的锁
};