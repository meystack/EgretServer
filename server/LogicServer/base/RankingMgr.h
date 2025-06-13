#pragma once

/********************************************
*
*	排行榜管理器
*
*********************************************/
class CRanking;

class RankAllocator: public std::allocator<std::pair<int, CRanking> >
{
    typedef CRanking* pointer;
    pointer allocate(size_type _n, const void* = 0);
    void deallocate(pointer _p, size_type);
    void construct(pointer, const CRanking& rank) { }
    void destroy(pointer);
};

class CRankingMgr
{

public:

	friend class RankAllocator;
	typedef std::map<INT_PTR, CRanking, std::less<INT_PTR>, RankAllocator> RankMap;
	typedef typename RankMap::iterator RankIterator;

	/*
	* Comments:获取一个排行榜
	*/
	CRanking* GetRanking(INT_PTR rankId);

	/*
	* Comments:删除排行榜
	*/
	void Remove(INT_PTR rankId);

	/*
	* Comments:增加一个排行榜
	*/
	CRanking* Add(INT_PTR rankId,LPCSTR sName,INT_PTR nMax,int boDisplay,int nBroadCount = 0);

	//获取列表
	RankMap& GetList() { return m_RankingList;}

	//向所有玩家发送通知，排行榜改变了
	void NotifyUpdate();

	/*
	* Comments:向所有跨服服务器发送通知，排行榜改变了
	*/
	void NotifyRankUpdateCs(INT_PTR rankId);

	/*
	* Comments:更新跨服排行榜
	* Param CDataPacketReader &packet:
	* @Return void:
	*/
	void OnUpdateCsRank(CDataPacketReader &packet);

	//释放资源
	VOID  Destroy();
	//初始化
	BOOL  Initialize();

	/*
	* Comments:定期检查是否有信息需要保存
	* @Return VOID:
	*/
	VOID RunOne(TICKCOUNT currTick);

	//强制存盘操作
	VOID Save();

	bool CheckFilterRank(INT_PTR rankId);

	// 更新基础排行榜
	void UpdateBaseRankData();
	// 更新基础排行榜 跨服
	void UpdateBaseRankCSData(); 
	// 加载基础排行榜
	void LoadBaseRankData();
	
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);

	void SendRankTitle(CActor* pActor = NULL);
	
public:

	CRankingMgr();
	virtual ~CRankingMgr();

public:

	static 	CBufferAllocator*	m_pAllocator;	//排行榜统一的内存分配器

private:


	//CVector<CRanking*>		m_RankingList;//全服的排行榜的列表,根据名字查找
	RankMap 					m_RankingList;
	CTimer<3600000>				m_SaveTime;	//1个小时存盘一次
	CTimer<600000>				m_LoadBaseRankTime;
	CTimer<60000>			m_LoadCenterTankTime;//跨服排行定时 拉取 --43200000
	CTimer<60000>			m_LoalCenterMoBaiRankTime;// 拉取 --86400000
	// CTimer<30000>			m_LoadCenterTankTime;//跨服排行定时 拉取 
	// CTimer<60000>			m_LoalCenterMoBaiRankTime;// 拉取 
	std::map<unsigned int, std::set<int> > m_oldrankLists;
	std::map< unsigned int, std::set<int> > m_newrankLists;
public:
	//跨服相关
	void SetRankList(int nRankId, CDataPacketReader& inPacket);
	void GetRankListByRankId(int nRankId, CDataPacket& inPacket);
	void LoadCenterRankData(int nRankId);//定时拉取跨服排行
};
