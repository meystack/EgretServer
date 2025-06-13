#pragma once

//一级分类
typedef struct tagConsingmentClass
{
	int			 class1Id;		//一级分类Id
	DataList<int> class2List;	//二级分类列表
}ConsingmentClass;

typedef struct tagTaxAutomaticConfig
{
	int			 class1Id;		//一级分类Id
	int          nOpenday;//开始时间
	int          nEndday; //结束时间
	int          nSpace;//每轮cd
	int          nId;//id
	int           nNumber;//数量  
	int            nPrice;//价格
	tagTaxAutomaticConfig()
	{
		memset(this, 0, sizeof(*this));
	}
}TaxAutomaticConfig;



class ConsignmentProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

public:
	ConsignmentProvider();
	~ConsignmentProvider();

	//从文件加载配置
	bool LoadConsignConfig(LPCTSTR sFilePath);
	
protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);
	bool readConfigs();
public :
	//获取二级分类所在的一级分类id
	int getConsignTypeClass1Id(int nConsignType);
	//自动上架数量
	std::map<int, TaxAutomaticConfig>& getAutoInConsignment()
	{
		return m_nAutoInConsignmentLists;
	}
	TaxAutomaticConfig* getPtrAutoInConsignment(int iD)
	{
		auto it = m_nAutoInConsignmentLists.find(iD);
		if(it != m_nAutoInConsignmentLists.end())
		{
			return &(it->second);
		}
		return NULL;
	}
private:
	CDataAllocator			m_DataAllocator;	//对象申请器

public:
	std::map<int, TaxAutomaticConfig>         m_nAutoInConsignmentLists;//自动上架
	unsigned int			nMinSellPrice;		//单个最低售价
	unsigned int			nMaxSellPrice;		//单个最高售价
	int						nPageCount	;		//每页显示多少条数据
	int						nMaxShelfCount;		//最多上架数量
	int						nMaxServerCont;		//全服最多上架数量
	unsigned int			nSellTime	;		//售卖时间(小时)
	int						nOpenLevel	;		//开启等级
	int						nConsignLevel[2] ={0,0}	;		//开启转生等级
	float					fTaxRate	;		//交税的比例
	int						nShoutTime  ;		//吆喝间隔(秒)
	int                     nSellCost   ;//	交易行上架手续费(绑金)
	int			nShoutMoney ;		//吆喝金币
	DataList<ConsingmentClass> classList ;		//分类
	int          nDealCd;                      //私人交易邀请cd
	int          nPrivateDealDistance;                      //私人交易双方距离

	int          nMinDealLevel;//私人交易最低等级
	int						nDealLevel[2] ={0,0}	;		//私人交易开启转生等级
	int          nDuty;//私人交易元宝交易税
	int          nDues;//私人交易金币交易税
	char 					sDealLimitTips[128];//交易寄售行限制提示
};


