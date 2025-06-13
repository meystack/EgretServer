#pragma once

static const LPCTSTR g_szTodayConsumeYuBao = _T("TodayConsumeYuBaoName.Rank");			//今天元宝消耗排行
static const LPCTSTR g_szTodayConsumeYuBaoRanking = _T("TodayConsumeYuBaoRank.Rank");

static const LPCTSTR g_szYesterdayConsumeYuBao = _T("YesterdayConsumeYuBaoName.Rank");	//昨日元宝消耗排行
static const LPCTSTR g_szYesterdayConsumeYuBaoRanking = _T("YesterdayConsumeYuBaoRank.Rank");

class CGameStoreRank
{
public:
	//商城排行文件头标志
	static const UINT32 GameStoreRankFileIdent = MAKEFOURCC(0, 'R', 'S', 'S');
	//商城排行文件版本号
	static const UINT32 GameStoreRankFileVersion = MAKEFOURCC(1, 11,10 , 11);
	/* 定义商城商品销量排行文件头 */
	typedef struct GameStoreRankFileHeader
	{
		UINT32	uIdent;		//文件标志头，固定为GameStoreRankFileIdent
		UINT32  uVersion;	//文件版本号，当前版本号为GameStoreRankFileVersion
		UINT32  uRecCount;	//文件中记录的商品排行数据数量
		BYTE    reseve[52];	//保留字节使得文件头大小固定为64字节
	}*PGSRFHEADER;
	
	
	/* 定义商品销售排行项结构 */
	typedef struct SaleData
	{
		INT		nId;		//商品唯一ID
		UINT	uCount;		//累积销售数量
		INT     nActorId;   //玩家的actorid，这个是用于存储
	}*PSALADATA;

	//消费元宝排行
	typedef struct tagConsumeYuanBao
	{
		int				nActorId;		//玩家id
		int				nCount;			//消耗的元宝
		ACTORNAME		nActorName;		//玩家姓名
	}CONSUMEYB;


public:
	~CGameStoreRank();
	CGameStoreRank()
	{
		m_hasDataChange =false;
	}
	//更新商品销售数量，函数内会更新商品排行并返回商品的总计销量
	UINT UpdateSelaCount(INT nMerchandiseId, INT nSellCount,unsigned int nActorId =0, bool bNeedRank =false);
	
	//获取排行记录数量
	inline INT_PTR RankCount(){ return m_SaleRankList.count(); }

	//从文件加载商城销量排行数据
	bool LoadFromFile(LPCTSTR sFilePath);
	//将商城销量排行数据保存到文件
	bool SaveToFile(LPCTSTR sFilePath) ;

	//从数据流中加载商城销量排行数据
	bool LoadFromStream(wylib::stream::CBaseStream &stream);
	//将商城销量排行数据保存到数据流中
	bool SaveToStream(wylib::stream::CBaseStream &stream) const;

	//清空列表
	inline void Clear()
	{
		m_SaleRankList.clear();
		m_hasDataChange =true;
	}

	//获取某个的数目,以及是否存在这个商品
	UINT GetIdCount(INT nId,bool &isExist,INT nActorId =0)
	{
		for(INT_PTR i=0;i < m_SaleRankList.count(); i++)
		{
			if( m_SaleRankList[i].nId == nId)
			{
				if(nActorId ==  m_SaleRankList[i].nActorId || nActorId ==0)
				{
					isExist =true;
					return  m_SaleRankList[i].uCount;
				}
			}
		}
		isExist =false;
		return 0;
	}

	bool RemoveIndex(INT_PTR nIndex)
	{
		if(nIndex <0 || nIndex >= m_SaleRankList.count())
		{
			return false;
		}
		m_SaleRankList.remove(nIndex);
		return true;
	}
	CVector<SaleData>& GetActorConsume(INT nActorId);
	
	//加载今天消耗的排行榜
	void LoadTodayConsumeRankFile();

	//保存今天消耗的排行榜
	void SaveTodayConsumeRankToFile();

	//加载昨天消耗的排行榜
	void LoadYesterdayConsumeRank();

	//保存昨天消耗的排行榜
	void SaveYesterdayConsumeRankToFile();

	//每天0点更新
	void RfeshConsumeRank();

	void SetDataChange(bool bDataChange = true)
	{
		m_hasDataChange = bDataChange;
	}
public:
	inline operator const PSALADATA () const
	{
		return m_SaleRankList;
	}

protected:
	//商品销售数据对比函数，用于排行排序
	static INT __cdecl CompareMerchandiseSaleData(CGameStoreRank::SaleData &s1, CGameStoreRank::SaleData &s2);

protected:
	CVector<SaleData>	m_SaleRankList;	 //商品销售排行榜

	CVector<CONSUMEYB>	m_TodayComsumeRank;			//今天消耗元宝排行
	CVector<CONSUMEYB>  m_YesterdayComsumeRank;		//昨天消耗元宝排行

	bool                m_hasDataChange; //是否已经改变过
};
