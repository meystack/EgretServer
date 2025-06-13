#pragma once

/********************************************
*
*	排行榜类，主要导出给脚本使用
*
*********************************************/
class CRankingMgr;
class CRanking;


#define RANKING_ITEM_CHECUSUM 0xFFEECC11
#define RANKING_CHECUSUM 0xFFEECC00



/// 排行榜项，每项表示一个用户的分值，每个排行榜由多个排行榜项组成
class CRankingItem
{
friend class CRanking;

public:
	CRankingItem(CRanking *pRank)
	{
		nColCount = 0;
		sCd = NULL;
		nId = 0;
		nPoint = 0;
		nMaxCol = 0;
		pParent =pRank;
		nParam = 0;
		nParam1 = 0;
		nChecuSum = RANKING_ITEM_CHECUSUM;
	}

	/// 设置父亲的节点的指针，用于查找位置	
	VOID SetParent(CRanking *pRank)
	{
		if(pParent != NULL) return;
		pParent = pRank;
	}

	/// 是否是合法的
	bool IsValid() {return nChecuSum ==RANKING_ITEM_CHECUSUM;}

#define  CHECK_RANK_ITEM_VALID() \
	if(nChecuSum !=RANKING_ITEM_CHECUSUM) {\
			OutputMsg(rmError,"%s,%d,ranking error",__FUNCTION__,__LINE__);\
	for(int i=0; i< nColCount;i ++)\
	{\
		if(sCd[i])\
		{\
			OutputMsg(rmError,"column %d:%s",i,sCd[i]);\
		}\
	}\
	return; }

#define  CHECK_RANK_ITEM_VALID_RETSTR() \
	if( nChecuSum != RANKING_ITEM_CHECUSUM) {OutputMsg(rmError, "%s,%d,ranking get index[%d] str ret error", __FUNCTION__, __LINE__, nIndex);\
	return "-"; }

	void Destroy()
	{
		CHECK_RANK_ITEM_VALID()
		if (sCd)
		{
			for (int i = 0; i < nColCount; i++)
			{
				if(sCd[i])
				{
					CRankingMgr::m_pAllocator->FreeBuffer(sCd[i]);
					sCd[i]=NULL;
				}
			}
			CRankingMgr::m_pAllocator->FreeBuffer(sCd);
			nColCount = 0;
			sCd = NULL;
			nId = 0;
			nPoint = 0;
			nMaxCol = 0;
		}
	}

	virtual ~CRankingItem()
	{
		Destroy();
		nChecuSum =0; 
		pParent =NULL;
	}

	/// 设置列数
	void SetColCount(INT_PTR nCount)
	{ 
		CHECK_RANK_ITEM_VALID()
		if (nCount > nMaxCol)
		{
			INT_PTR nSize = sizeof(char*)*nCount;
			char** ss = (char**)CRankingMgr::m_pAllocator->AllocBuffer(nSize);
			ZeroMemory(ss,nSize);
			if (sCd)
			{
				memcpy(ss,sCd,sizeof(char*)*nMaxCol);
				CRankingMgr::m_pAllocator->FreeBuffer(sCd);
			}
			sCd  = ss;
			nMaxCol = (BYTE)nCount;
		}
		nColCount = (BYTE)nCount;	
	}

	void SetSub(INT_PTR nIndex, LPCSTR sData);
	void SetSub(INT_PTR nIndex, unsigned int nValue);


	/// 在指定的列前增加一列.-1表示在最后插入.bUnique列名是否唯一,true则唯一 
	void AddColumn(LPCSTR sData,INT_PTR nIndex=-1, bool bUnique=false);
	

	/// 设置列的标题
	void SetColumnTitle(LPCSTR sData, INT_PTR nIndex);
	

	/// 删除某列的数据
	void RemoveColumn(INT_PTR nIndex);
	

	void SetId(unsigned int id);

	inline unsigned int GetId() { return nId;}
	inline int GetPoint() { return nPoint;}
	inline int GetParam() { return nParam;}
	inline int GetParam1() { return nParam1;}
	void SetPoint(int point);
	void SetParam(int param);
	void SetParam1(int param);
	
	/// 获取这个节点在管理器里的索引
	int GetIndex();
		
	//inline void SetIndex(int index) {CHECK_RANK_ITEM_VALID() nIndex = index;}
	inline int GetColCount() { return nColCount;}
	/// 获取某列的数据
	inline LPCSTR GetSubData(INT_PTR nIndex)
	{
		CHECK_RANK_ITEM_VALID_RETSTR()
		if (sCd && nIndex >=0 && nIndex < nColCount)
		{
			return sCd[nIndex]?sCd[nIndex]:"-";
		}else
		{
			return "-";
		}
	}
private:
	unsigned int	nId;		///< 唯一值，可以是角色id或者帮派id等
	int				nPoint;		///< 分数,排行榜根据这个排序
	int             nParam;      //额外参数
	int             nParam1;      //额外参数1
	char**			sCd;		///< 每列的数据内容，每列都是字符串类型，主要用于显示
	BYTE			nColCount;	///< 列的数目
	BYTE			nMaxCol;	///< 最大列
	//int			nIndex;		///< 自身在排行榜的位置,0是第一位
	CRanking *		pParent;	///< 获得其指针

	int				nChecuSum;  ///< 校验码
	DECLARE_OBJECT_COUNTER(CRankingItem)
};



/// 排行榜类
class CRanking:
	public CBaseScriptExportObject
{
public:
	struct tagIndex
	{
	public:
		int					id;
		CRankingItem*		pItem;
	};

	/// 排行可见类型
	enum RankVisibleType
	{
		enVisible_All,				///<  世界所有玩家
		enVisible_SameCamp,			///<  同阵营玩家
		enVisible_SameGuild,		///<  同公会玩家
		enVisible_SameTeam,			///<  同队伍
	};
	/// 排行可见参数
	struct RankVisibleParam
	{
		union
		{
			int		nCampId;		///<  阵营Id
			unsigned int		nGuildId;		///<  公会id
			int		nTeamId;		///<  队伍Id
		}data;
	};

public:
	CRanking():m_Title(this){}
	CRanking(LPCSTR sName,INT_PTR nMax,bool boDisp, INT_PTR nMaxBroadCount = 0);
	virtual ~CRanking();

	
#define  CHECK_RANK_VALID() \
	if(m_nCheckSum != RANKING_CHECUSUM) { OutputMsg(rmError,"rank=%s,%s,%d,CRanking is error !!!",m_szFile,__FUNCTION__,__LINE__);\
	return; }

	const static int s_nDur = 120 * 60 * 1000; ///< 存盘，120分钟只存1次

public:
	//继承基类，释放资源，如果引用计数器没到0，不能释放内存
	//virtual int release();
	virtual void destroy();

	/*
	* Comments:从文件读取排行榜数据，如果原来已经读取过一次，会全部清除掉；文件名根据m_szName
	* @Return bool:成功读取返回true
	*/
	bool Load(LPCSTR sFile = nullptr);

	/// 排行榜是否合法
	bool IsValid()
	{
		if(m_nCheckSum != RANKING_CHECUSUM)
		{
			//OutputMsg(rmError,"rank=%s,name=%s,CRanking is error !",m_szFile,m_szName);
			return false;
		}
		return true;
	}
	/*
	* Comments:增加一列
	* Param LPCSTR sTitle:标题
	* Param INT_PTR nIndex:在本索引前插入,-1表示最后增加
	* @Return bool:
	*/
	void AddColumn(LPCSTR sTitle, INT_PTR nIndex, bool bUnique=false);


	/// 获取一个子节点的位置
	inline int GetNodeIndex(CRankingItem * pItem)
	{
		for(int i=0;i <m_ItemList.count(); i++)
		{
			if(m_ItemList[i] == pItem) return i;
		}
		return -10000;
	}
	//检车玩家id 是否在当前排行中
	bool CheckActorIdInRank(unsigned int nActorId);

	//通过条件获取玩家id
	unsigned int GetActorIdBySexJob(LPCSTR nCondition);
	
	/* 
	* Comments: 设置列标题
	* Param LPCSTR sTitle: 标题
	* Param INT_PTR nIndex: 列的索引
	* @Return void:  
	*/
	void SetColumnTitle(LPCSTR sTitle, INT_PTR nIndex);

	/*
	* Comments:删除一列
	* Param INT_PTR nIndex:列的索引
	* @Return void:
	*/
	void RemoveColumn(INT_PTR nIndex);

	
	/*
	* Comments:排行榜存盘
	* Param LPCSTR sFile:另存文件的名字
	* Param bool forceSave:是否强制存盘，true 表示强制存盘
	* @Return void:
	*/
	void Save(LPCSTR sFile = nullptr,bool forceSave =false );

	void ForceSave(LPCSTR sFile = nullptr);

	/*
	* Comments:清除所有数据
	* @Return void:
	*/
	void clear();

	//获取列的数量
	inline INT_PTR ColumnCount() { return m_Title.GetColCount(); }

	/*
	* Comments:增加一个排行榜项
	* Param INT_PTR nId:id值
	* Param INT_PTR nPoints:初始的分值
	* Param bool bRankFlag:true 表示需要检测排序
	* @Return CRankingItem*:成功返回排行榜项，否则返回NULL,如果已经存在对应的id，会加入失败
	*/
	CRankingItem* AddItem(UINT_PTR nId,INT_PTR nPoints = 0,bool bRankFlag = true);

	/*
	* Comments:给某个id增加分值
	* Param INT_PTR nId:排行榜项的id
	* Param INT_PTR nPoint:分值，负数表示减少
	* @Return CRankingItem*:非NULL表示成功
	*/
	CRankingItem* Update(INT_PTR nId, INT_PTR nPoint);

	/*
	* Comments:直接设置某个id的分值
	* Param INT_PTR nId:排行榜项的id
	* Param INT_PTR nPoint:分值
	* @Return CRankingItem*:非NULL表示成功
	*/
	CRankingItem* Set(INT_PTR nId, INT_PTR nPoint);
	
	//获取列表
	inline CVector<CRankingItem*>& GetList(){return m_ItemList;}

	//获取当前排行榜的行数
	inline int GetItemCount() {return (int)m_ItemList.count();}

	/*
	* Comments:删除指定的id
	* Param INT_PTR nId:
	* @Return void:
	*/
	void RemoveId(INT_PTR nId);

	////设置上榜的最多人数,要重新排序
	//void SetRankMax(INT_PTR nCount);

	/*
	* Comments:根据索引获取某一项，用户改变列字符串的数值，不能在这里改变分值
	* Param INT_PTR nIndex:在排行榜的位置
	* @Return CRankingItem*:位置不正确会返回NULL
	*/
	CRankingItem* GetItem(INT_PTR nIndex);

	/*
	* Comments:把该项插到排行榜中,向上更新位置
	* Param CRankingItem * pItem:
	* @Return INT_PTR:返回在排行榜中的位置
	*/
	//INT_PTR UpdateUpPos(CRankingItem* pItem);

	/*
	* Comments:把该项插到排行榜中,向下更新位置
	* Param CRankingItem * pItem:
	* @Return INT_PTR:返回在排行榜中的位置
	*/
	//INT_PTR UpdateDownPos(CRankingItem* pItem);


	/*
	* Comments:获取某个id在排行榜中的位置，第一位是0，不在这个
	* Param INT_PTR nId:
	* @Return INT_PTR:
	*/
	INT_PTR GetIndexFromId(UINT_PTR nId);

	
	/*
	* Comments:根据id获取指针和索引
	* Param INT_PTR nId:ID
	* Param INT_PTR & idIndex:返回的索引
	* @Return CRankingItem*:返回的数据指针
	*/
	CRankingItem* GetPtrFromId(INT_PTR nId,INT_PTR& idIndex);

	
	/*
	* Comments:根据id获取指针
	* Param INT_PTR nId:数据ID
	* @Return CRankingItem*:数据的指针
	*/
	CRankingItem*  GetPtrFromId(UINT_PTR nId);

	//在榜外找个最大的，上榜
	//CRankingItem* FindMaxItem();

	inline LPCSTR GetName(){return m_szName;}
	inline int GetMax() {
		return m_nMaxBroadCount > 0 ? m_nMaxBroadCount: m_RankMax;
	}
	inline int GetColCount() { return m_Title.GetColCount();}

	/*
	* Comments:设置某列是否显示在客户端
	* Param INT_PTR nIndex:列数
	* Param bool disp:是否显示
	* @Return void:
	*/
	void SetColDisplay(INT_PTR nIndex, bool disp);

	/*
	* Comments:判断该列是否显示
	* Param INT_PTR nIndex:列索引
	* @Return bool:显示返回true
	*/
	bool isColDisplay(INT_PTR nIndex);

	//是否发到客户端显示
	inline bool IsDisplay(CActor *pActor) { 
		bool bVisible = true;
		switch (m_rankVisibleType)
		{
		case enVisible_All:
			bVisible = true;
			break;
		case enVisible_SameCamp:
			{
				bVisible =true;
				break;
			}
			break;
			// 这些暂时还没实现！ 以后需要的时候，按照上面的方式实现下就行了！
		case enVisible_SameGuild:
		case enVisible_SameTeam:
			break;
		}

		return m_Display && bVisible; 
	}

	//设置在客户端显示的名称
	inline void SetDisplayName(LPCSTR sName)
	{
		if (sName)
		{
			_asncpytA(m_szCnName,sName);
		}
		else
		{
			m_szCnName[0] = 0;
		}
	}
	//获取在客户端显示的名称
	inline LPCSTR GetDisplayName() { return m_szCnName; }
	//获取标题
	inline CRankingItem& GetTitle() { return m_Title; }

	inline LPCSTR GetIdTitle() { return m_idTitle;}
	inline void SetIdTitle(LPCSTR sName) 
	{
		if (sName)
		{
			_asncpytA(m_idTitle,sName);
		}
		else
		{
			m_idTitle[0] = 0;
		}
	}

	inline LPCSTR GetPointTitle() { return m_pointTitle; }
	inline void SetPointTitle(LPCSTR sName)
	{
		if (sName)
		{
			_asncpytA(m_pointTitle,sName);
		}
		else
		{
			m_pointTitle[0] = 0;
		}
	}

	inline void SetRankVisibleInfo(RankVisibleType vt, RankVisibleParam param)
	{
		m_rankVisibleType = vt;
		m_rankVisibleParam = param;
	}

	//是否有数据改变
	inline bool IsModify() { return m_boModify; }

	//设置是否改变的标记
	inline void SetModify( bool flag=true) {m_boModify =flag;}

	inline void SetEmptySave(bool flag=false) { m_bRmptySave = flag;}

	//更新索引中的id
	//void UpdateTagId(int nOldId,int nNewId);

	
	/*
	* Comments:根据索引进行交换位置，!!!!!如果是根据分数排名的是无法交换的，将失败
	* Param INT_PTR nPos1:位置1
	* Param INT_PTR nPos2:位置2
	* @Return bool:成功返回true，否则返回false
	*/
	bool SwapItem(INT_PTR nPos1,INT_PTR nPos2 );

	//将1个项插队
	bool JumpQueue(CRankingItem *pItem,INT_PTR nNewPos);


	//设置属性排行榜配置数据 nRankPropIndex:属性配置表对应的配置索引 从0开始
	void SetRankPropConfig(int nRankPropIndex);

	/*
	* Comments:更新带属性的排行榜的属性
	* Param CRankingItem * pItem:指针
	* Param INT_PTR nIndex:位置
	* Param bool bClear:是否是删除排行榜里的项
	* @Return void:
	*/
	void UpdateRankProp(CRankingItem *pItem,INT_PTR nIndex, bool bClear=false);


	//把属性与排行榜关联起来
	void AtachRankProp(LPCSTR sName);

	//把属性与排行榜去掉关联
	void DetachhRankProp(LPCSTR sName);

	/*
	* Comments:找一个插入的位置
	* Param int nPoint:分数点
	* @Return INT_PTR:寻找一个插入的位置
	*/
	INT_PTR GetInsertPos(int nPoint);

	//是否排序好了，默认的是按正常的降序排列的，特殊时候，不需要排序的，这里要做1个标记
	inline bool isRanked()
	{
		return m_isRanked;
	}

	//设置是否被排序好了
	inline void SetRanked(bool flag)
	{
		m_isRanked = flag;
	}

	//获取文件的名字
	char * GetFileName()
	{
		return m_szFile;
	}

	void SetSaveFlag(bool flag)
	{
		m_canSave = flag;
	}

	void PushToPack(CActor* pActor, int nNum, CDataPacket* pDataPack);

	void PushRankToPack(int nNum, CDataPacket* pDataPack);
	void PushEspecialRankToPack(int nNum, CDataPacket* pDataPack);

private:
	//保存每项数据到文件中
	void SaveItem( CRankingItem* pItem, CStringBuff &buff );

	//把新id插入到索引中
	//void InsertId( tagIndex &IndexItem );
	

private:
	char							m_szName[32];	//排行榜的名字，脚本通过这个名字访问排行榜，同时也是用这个名字作为文件保存
	char							m_szCnName[32];	//排行榜的中文名，主要显示在客户端的

	char							m_idTitle[32];	//id的标题文字
	char							m_pointTitle[32]; //分值的标题文字

	CRankingItem					m_Title;	//这个项用来表示标题
	CVector<CRankingItem*>			m_ItemList;	//保存所有的列表
	int                             m_columnCount;  //列的个数
	

	//CVector<tagIndex>				m_index;	//索引表，根据id排序，查找时根据二分查找
	int								m_RankMax;	//上榜的最多人数
	int								m_nMaxBroadCount;	//广播给客户端次数
	BYTE							m_ColDisplay[255];	//表示第几列是否显示在客户端，0表示显示，默认是0，非0表示不显示
	bool							m_Display;			//表示是否显示

	bool                            m_isRanked;  //是否被排序好的

	RankVisibleParam				m_rankVisibleParam;				// 排行榜可见类型相关的参数。例如阵营相关的排行，这里存放可见排行的阵营ID
	RankVisibleType					m_rankVisibleType;				// 排行榜可见类型
	static const TCHAR* const					m_sClassName; ///< 不知道有啥用 
	bool							m_boModify;					//是否修改过数据

	char							m_szFile[256];	//保存的文件

	bool							m_bRmptySave;		//当排行榜数据为空时，是否保存排行榜文件

	//排行榜属性添加
	int								m_nRankPropIndex;	//属性排行榜配置索引
	int								m_nRankPropMax;		//最大属性排行榜排行名额
	int								m_nRankPropValue;	//属性榜最少值限制
	unsigned int                    m_nCheckSum;        //校验和
	TICKCOUNT                       m_saveTick;         //上次存盘的时间

	bool							m_canSave;	// 是否存盘的标识
public:
	
	void SendCrossServerToPack(int nNum, CDataPacket& pack);
};
