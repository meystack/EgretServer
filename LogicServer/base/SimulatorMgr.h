#pragma once

/********************************************
*
*	模拟玩家管理器
*
*********************************************/ 
class CSimulatorMgr
{ 
public: 
	typedef std::map<INT_PTR, CRanking, std::less<INT_PTR>, RankAllocator> RankMap;
	typedef typename RankMap::iterator RankIterator;

public: 
	CSimulatorMgr();
	virtual ~CSimulatorMgr();
  
	int GetTestActorId(){ return 111111;};//测试玩家ID
	bool CheckTestActorId();
	void DoTestActorId();
  
	void OnRemoveRuningActor(int actorId);
	//获得随机位置
	void OnGetRandSimuPos(int& SenceId, int& posX, int& posY);
	
	//处理跨服的逻辑
	void OnAddRuningCsActor(int actorId); 
 	void OnRemoveRuningCsActor(int actorId);
	//释放资源
	VOID  Destroy();
	//初始化
	BOOL  Initialize();
 
	VOID RunOne(TICKCOUNT currTick);
   
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size); 
	
public: 
	static 	CBufferAllocator*	m_pAllocator;	//排行榜统一的内存分配器

private:  
	std::vector<int> m_RankActorVec; 
	std::map<int, int> m_RuningActorMap;	//正在跑的虚拟玩家
	std::map<int, int> m_RuningCSActorMap;	//正在跨服的玩家
	CTimer<3600000>				m_SaveTime;	//1个小时存盘一次
	//CTimer<600000>				m_RefreshRank;//10分钟处理一次
	CTimer<60000>				m_RefreshRank;//10分钟处理一次
	CTimer<10000>				m_RefreshSimu;//2秒处理一次玩家
	CTimer<60000>				m_LoadCenterTankTime;//跨服排行定时 拉取 --43200000
	CTimer<60000>				m_LoalCenterMoBaiRankTime;// 拉取 --86400000 
	std::map<unsigned int, std::set<int> > m_oldrankLists;
	std::map< unsigned int, std::set<int> > m_newrankLists; 

	bool m_InitTestActor;
};
