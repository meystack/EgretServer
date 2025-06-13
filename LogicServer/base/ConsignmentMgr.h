#pragma once

/****************************************
	寄卖系统，这个类用来管理全部的玩家寄卖物品，整个程序只有一个实例，类似帮派系统
*****************************************/

#define CONSIGN_SAVE_COUNT_EACH_TIME	300		//保存寄卖数据，每个包发送的物品数
#define INCOME_SAVE_COUNT_EACH_TIME		1000	//保存收益数据，每个包发送的数量

typedef struct tagAutoConsignmentData
{
	int nId;// id
	int nEndDay;//过期时间
	int nCd;//每轮时间
	int          nItemId;//id
	int           nNumber;//数量  
	int            nPrice;//价格
	tagAutoConsignmentData()
	{
		memset(this, 0, sizeof(*this));
	}
}AutoConsignmentData,PAutoConsignmentData;

class CConsignmentMgr
{
public:	
	CConsignmentMgr();
	~CConsignmentMgr();
	void Destroy();
	/*
	* Comments:从数据库获取所有寄卖物品的列表
	* @Return VOID:
	*/
	VOID Load();

	/*
	* Comments:数据服务器返回数据,这里读取所有寄卖物品的内容
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	*/
	VOID OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);	//

	/*
	* Comments:定期检查
	* @Return VOID:
	*/
	VOID RunOne(CMiniDateTime& minidate,TICKCOUNT tick);
public:
	//检查过期物品
	void CheckConsignTimeOut();
	//获取某个玩家的收益
	unsigned int  GetIncomeValue(unsigned int nActorId);
	//获取某个玩家的收益
	ConsignmentIncome*  GetIncome(unsigned int nActorId);
	//删除某个玩家的收益数据
	bool DelIncome(unsigned int nActorId);
	//增加收益
	void AddIncome(ConsignmentData* pData, int nCount);
	//获取寄卖数据
	ConsignmentData* GetConsignItem(uint64_t nSellIndex);
	//增加一个寄售物品
	//ConsignmentData* AddConsignItem( CUserItem* pUserItem, unsigned int nActorId, unsigned int nPrice, unsigned int nCount);

	bool AddConsignItem(ConsignmentData& nData, bool dbReturn = false);
	//删除一个寄售物品
	bool DelConsignItem(ConsignmentData* pDelData);
	//排序物品
	void SortSearchItem();
	//按条件过滤寄卖物品
	void FilterSearchItem(CVector<ConsignmentData*>& List, BYTE nClass1Id, BYTE nClass2Id, BYTE nMinLevel,BYTE nMaxLevel, char nCircle, BYTE nJob, char nQuality, const char *sName);
	//保存寄卖物品数据(按玩家,收益+物品)
	void SaveActorConsignToDb();
	//保存寄卖物品数据(主要是系统的nActorId=0,数据量有可能很多，分包发送，不用cache)
	void SaveConsignItemToDb();
	//玩家数据是否修改过
	bool IsActorDataModify(unsigned int nActorId);
	//设置玩家数据修改过
	void SetActorDataModify(unsigned int nActorId);
	//从管理器里读取自己的寄卖数据
	void GetMyConsignItem(unsigned int nMyActorId,std::vector<ConsignmentData*>& myItemList);
	//从管理器里读取自己的正在寄卖数据
	void  GetMyConsignOnSellItem(unsigned int nMyActorId, std::vector<ConsignmentData*>& myItemList);
	//从管理器里读取自己可领取的寄卖数据
	bool GetMyCanGetConsignItem(unsigned int nMyActorId);

	void DealBuyConsignItem(ConsignmentData* nData, unsigned int nActorId);
	int nCount() {return m_consignmentDataList.size();};
	struct WaitingTaxAutomaticCmpter
    {
        bool operator()(const TaxAutomaticConfig _x, const TaxAutomaticConfig _y) const
        { return (unsigned int)(_x.nOpenday) == (unsigned int)(_y.nOpenday)? _x.class1Id < _y.class1Id : (unsigned int)(_x.nOpenday) < (unsigned int)(_y.nOpenday); }
    };

	void AddConsignItemByAuto(AutoConsignmentData& nData);
	
	bool CheckInConsignItemByAutoId(int nId);

	// void addNewConsignItem(ConsignmentData& dbdata);

	void addNewConsignItem(const TaxAutomaticConfig& cfg);

	void CheckCanAutoConsign();
	//删除过期的上架物品
	void DelOffShelf();
public:
	CTimer<60000>						m_1minuteTimer;     //1分钟的定时器
	CTimer<180000>						m_3minuteTimer;     //3分钟的定时器
	bool								m_boInited;			//是否已经读入数据
	std::map<int , AutoConsignmentData>       m_OpenLists;   //已经开启的上架的物品信息            
	std::set<TaxAutomaticConfig,WaitingTaxAutomaticCmpter>     m_nWaitAutoIns;//自动上架
	std::vector<ConsignmentData>		m_consignmentDataList;//寄卖数据
	int64_t                             m_nSellIndex;         //交易行当前最大标识
	// std::vector<ConsignmentIncome>		m_consignmentIncomeList;//收入数据
	std::vector<unsigned int>			m_ActorDataModifyList;	//玩家数据修改标志
	bool								m_boNeedSort;			//是否需要排序
};