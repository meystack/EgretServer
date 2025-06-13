#pragma once

class CActor;

/*******************************************************/
/*
/*					游戏商城类
/*
/*   实现了游戏中商城物品的配置读取、商品销售排行以及购买商品的功能。
/*  
/*商品的刷出是优先开服时间来算的，比如开服第1天卖什么，第2天卖什么...
/*在开服判断完以后，如果找不到合适的商品，那么按照星期几找
/*星期的限制只要用来后期的，如果开服时间比较久了，按星期来，每天不同 
/*******************************************************/
class CActor;
enum BuyShopLimitType
{
	BuyShopLimitLevel= 1, //等级
	BuyShopLimitCircle = 2, //转生
	BuyShopLimitOpenServerDay = 3, //开服天数
};
typedef struct tagShopInfo
{
	int nItemId = 0; // 物品id
	int nType = 0; // 物品类型
	int nCount = 0;// 物品数量
	tagShopInfo()
	{
		memset(this, 0, sizeof(*this));
	}

}ShopInfo;

typedef struct tagShopPrice
{
	int nId = 0; //类型
	int nCount = 0;//数量
	int nType = 0; //
	tagShopPrice()
	{
		memset(this, 0, sizeof(*this));
	}
}ShopPrice;


typedef struct TagShopBuyType
{
	int nType = 0;//限制类型
	std::map<int, int>limits; //限制
	TagShopBuyType()
	{
		nType = 0;
		limits.clear();
	}
}ShopBuyType;



typedef struct TagShopConfig
{
	int nShoptype = 0; //商城类型
	int nShopId = 0; //商品id
	ShopInfo shop; //商品详情
	ShopPrice price;//价格
	int nLimitLv; //等级限制
	int nReincarnationlimit; //转生等级要求
	int nDaylimit; //开服天数限制
	int nStaticType ;// 计数器
	int nFlag ;// 横幅类型
	char tips[1024];// 购买商品广播消息
	int nGuildLevelLimit; //等级限制
	std::vector<ShopBuyType> buyLimit;//购买限制
	std::vector<int> nNpc; //是否需要判断于npc距离
	TagShopConfig()
	{
		nShoptype = 0; //
		nShopId = 0; //
		nLimitLv = 0; //
		nReincarnationlimit = 0; //
		nDaylimit = 0; //
		nStaticType = 0;
		nGuildLevelLimit = 0;
		buyLimit.clear();//
		nNpc.clear();
	}

	bool InNpc(int nId)
	{
		std::vector<int>::iterator it = nNpc.begin();
		for(; it != nNpc.end(); it++)
		{
			if(*it == nId)
				return true;
		}
		return false;
	}
	
}ShopConfig;



class CGameStore :
	protected CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig Inherited;

	const static INT REFRESH_TIME_INTERVAL =24 * 60 * 60 ; //热销商品的刷新间隔

	/* 定义商城中的商品数据结构 */
	typedef struct Merchandise : public Counter<Merchandise>
	{
		INT			nId;				//商品唯一ID
		BYTE		btDealGoldType;		//交易币种类型
		bool		boBind;				//购买后是否绑定
		WORD		wItemId;			//商品的物品ID		
		WORD		wItemCount;			//一次购买的商品的数量（即1次某买可以获得N个物品）
		BYTE        bQuality;			//品质
		WORD		wQualityDataIndex;	//生成极品属性用
		BYTE        bStrong;			//强化等级
		BYTE        bLabelId;			//分类的ID
		bool        bDynamicAdd ;		//是否是动态添加的
		
		WORD        bSingleBuyLimit;	//单个商品购买的数目
		WORD        dFreshBuyLimit;		//单个商品在每次刷新后购买的数目（神秘商店使用）
		BYTE        bReserver;       
		WORD        wLabelBuyLimit;		//商品分类最大的购买数目
	
		UINT		dwPrice;			//实际购买价格
		BYTE		bSaleLevel;			//销售级别（0-禁止销售，1-可销售（通过商城页面），2-可销售（不通过商城页面））
		int			nJob;				//对应职业(0-全职业，1-战士，2-法师，3-巫师)
		int			nSex;				//对应性别(-1，不分性别,0-男，1-女)
		BYTE		bUse;				//能否立即使用（0 不能 1 可以）

	}*PMERCHANDISE;

	
	//刷新商品的配置
	typedef struct MerchanRefresh
	{
		INT nMerchandiseId;   //商品的ID
		INT nCount;           //数目,如果没有限制，就放一个很大的数字，比如100000000
		WORD nPercent;			//命中概率(10000为基数)	
		
		BYTE nWeekDay;         //周几刷新,-1表示不限制星期几，0表示周日，1表示周1 ,...6表示周6
		BYTE nLabelId;         //分类的ID
		BYTE nOpenServerDay;   //开服的时间,1表示开服第1天，2第2天，0表示不做开服的限制 
		BYTE bMonth;          //月
		BYTE bDay;            //日
		BYTE nMergeServerDay; //合服以来的第几天
		BYTE nBeforeMergeDay;	//合服前的第几天
		BYTE wReserver;     
	
	}*MERCHANREFRESH;

	
public:
	CGameStore();
	~CGameStore();

	void Buy(CActor *pActor);
	//通过商品ID获取商品对象
	const PMERCHANDISE GetMerchandise(const INT_PTR nId) const;
	//获取商品销量排行对象
	inline CGameStoreRank& GetStoreRank(){ return m_SaleRank; };

	//获取消耗排行对象
	inline CGameStoreRank& GetConsumeRank(){ return m_YBConsumeRank; };

	//加载商城商品配置
	bool LoadGameStore(LPCTSTR sFilePath);

	//刷新商城刷新配置表
	bool LoadGameRefresh(LPCTSTR sFilePath); 

	//加载商品销量排行数据
	inline bool LoadGameStoreRank(LPCTSTR sFilePath){ return m_SaleRank.LoadFromFile(sFilePath); }
	
	//保存商品销量排行数据
	bool SaveGameStoreRank(LPCTSTR sFilePath)  { return m_SaleRank.SaveToFile(sFilePath); }

	//装载动态商品的数据
	inline bool LoadDynamicMerchands(LPCTSTR sFilePath){ return m_dynamicMerchands.LoadFromFile(sFilePath); }

	//保存动态商品的数据
	inline bool SaveDynamicMerchands(LPCTSTR sFilePath){ return m_dynamicMerchands.SaveToFile(sFilePath); }
	
	/*
	* Comments:通过物品id获取商城物品
	* Param WORD wItemId:
	* @Return CGameStore::PMERCHANDISE:
	*/
	const PMERCHANDISE GetShopItem(WORD wItemId)const;
	
	
	/*
	* Comments: 刷新一个分类里的商品
	* Param INT nLabel:分类的ID
	* Param bool bNeedBroadcast:是否需要广播
	* @Return void:
	*/
	void ResetDynamicMerchadise(INT nLabel,bool bNeedBroadcast = true);
	

	/*
	* Comments:获取动态商品的数目
	* Param INT nId:商品的ID
	* Param bool & isExist:是否存在这个商品
	* @Return INT:返回商品的数目
	*/
	inline INT GetDynamicMerchCount(INT nId,bool &isExist)
	{
		return m_dynamicMerchands.GetIdCount(nId,isExist);
	}

	//获取当前的动态商品的存储
	CGameStoreRank & GetDynamicMerchans(){return m_dynamicMerchands; }

	//获取消费者消费的商品的列表
	CGameStoreRank & GetConsumeMerchans(){return m_consumerMerchands; }


	//获取刷新时间
	CGameStoreRank & GetRefreshTimeData(){return m_refreshTime; }

	//获取消费者消费的物品列表
	CVector<CGameStoreRank::SaleData>& GetActcorConsumeData(INT nActorId);
	
	//装载动态商城区数据
	bool LoadDynamicData();
	
	//保存动态商城区动态数据
	bool SaveDynamicData();

	int getLimitTimes(std::vector<ShopBuyType>&buyLimit, CActor * pActor, int nOpenDay = 0);

protected:
	//读取商城配置表
	bool ReadConfig(CBaseList<Merchandise> &merchandList);
	//新商城配置
	bool ReadShopConfig();
	//读取商品价格，如果价格有效则返回true，否则返回false
	bool ReadMerchandisePrice(LPCSTR sCurrSPID, Merchandise &merchandise);

	//读取刷新配置表
	bool ReadRefreshConfig(CBaseList<MerchanRefresh> &refreshList);

	/** Comments:添加动态销售商品
	* Param INT nID:商品的id
	* Param int nCount:商品的数目
	* @Return void:
	*/
	inline void AddDynamicMerchadise(INT nID,int nCount)
	{
		m_dynamicMerchands.UpdateSelaCount(nID,nCount,false);
	}
	

	//更新一个分类的刷新时间为现在
	void UpdateRefreshTime(INT nLabelId);

	//获取一个分类的刷新时间
	unsigned int GetRefreshTime(INT nLableId);

	//重置一个分类的数据
	void ResetLabelConfigData(INT nLabelID);

protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:
	CVector<Merchandise>		m_Merchands;	    //商品列表
	CVector<MerchanRefresh>     m_refreshConfig;    //商品的刷新的配置

	CGameStoreRank				m_SaleRank;		    //商品销量排行榜
	
	CGameStoreRank              m_refreshTime;      //商品的刷新时间
	CGameStoreRank              m_dynamicMerchands ;//动态的商品，就是脚本刷进来的商品

	CGameStoreRank              m_consumerMerchands ;//消费的道具的记录

	CGameStoreRank				m_YBConsumeRank;		//元宝消费排行榜

	CVector<CGameStoreRank::SaleData>			m_actorConsumeList; //玩家的消费的列表，用这个查询的时候用

	//INT                         m_nLabelCount;       //分类的数目
	CVector<int>                m_labelCount;       //每个分类每次刷出多少个

public:

	std::map<int, std::map<int, std::vector<ShopConfig> > > m_shops;//商品列表 商城类型  标签  数据
};
