#pragma once

#define OneTimePairAllocNum 30

#define YBRebateStartId 	10001	// 元宝返利 起始Id
#define SingleRebateStartId 	5001	// 单笔返利 起始Id

enum RebateTimeType //时间配置
{   
    eRebateTimeType_KFSJ = 0,    //开服时间  
    eRebateTimeType_GDSJ = 1,    //固定时间 
    eRebateTimeType_HFSJ = 2,    //合服时间 
    eRebateTimeType_XHSJ = 3,    //循环时间 
    eRebateTimeType_MAX, 
};

enum RebateOfflineType //时间配置
{   
	eRebateOfflineType_MIN  = 0,
    eRebateOfflineType_OneDayRecharge = 1,    //单日累充 
    eRebateOfflineType_DaysRecharge   = 2,    //多日累充 
    eRebateOfflineType_OneDayConsume  = 3,    //单日累消
    eRebateOfflineType_DaysConsume    = 4,    //多日累消
    eRebateOfflineType_MAX, 
};

struct RebateInfo
{
	// int nId;							// 序号
	int nIndex;							// 奖励序号
	int nPay;							// 累计充值元宝数
	int nMinPay;						// 最小累计充值元宝数(元宝返利)
	int nMaxPay;						// 最大累计充值元宝数(元宝返利)
	std::vector<ACTORAWARD> vecAwardList;// 奖励
	int nRebatePercentage;				// 奖励比例(万分比)(元宝返利)
	char *sTitle;						// 邮件标题
	char *sContent;						// 邮件内容
	RebateInfo()
    {
        memset(this, 0 ,sizeof(*this));
    }		
};

struct RebateGroupInfo
{
	int nId;							// 序号
	int nTimeType;						// 时间类型
	CMiniDateTime nStartTime;			// 开始时间
	CMiniDateTime nEndTime;				// 结束时间
	OneTimePair* pTimeDetail;			// 时间配置
	short nTimeCount;
	short nTimeIdx;
	int nOfflineType;					// 活动类型
	int nOpenServer;					// 指定区服开放
	int nAtonce;						// 立刻发放奖励
	int nOverlay;						// 是否可叠加
	std::map<int, RebateInfo> RebatesInfo;
	RebateGroupInfo()
    {
        memset(this, 0 ,sizeof(*this));
    }		
};

typedef std::map<int, RebateGroupInfo>::iterator RebateGroupIterator;
typedef std::map<int, RebateInfo>::iterator RebateIterator; 

class CRebateProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig	Inherited;
	 
public:
	CRebateProvider();
	~CRebateProvider();

	bool Load(LPCTSTR sFilePath);
	bool ReadConfig(LPCTSTR sFileName);

public:// 功能接口

	// 更新活动开启时间 nNextTime 是否开启下一个时间点的计算
	bool UpdateOpenTime(lua_State * pLuaState, RebateGroupInfo& rebateGroupInfo, bool nNextTime = false );

	std::map<int, RebateGroupInfo>& GetRebateConfig(){ return m_RebatesGroup; }
	
private: 
    static 	CBufferAllocator* m_pAllocator; 

	bool ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out);
	bool ReadFixedTimeValue(lua_State * pLuaState, LPCTSTR sKey, int nKeyLen, SYSTEMTIME& out);

	std::map<int, RebateGroupInfo>m_RebatesGroup;
};
