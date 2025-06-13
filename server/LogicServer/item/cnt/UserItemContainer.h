#pragma once
#include<vector>

/************************************************************************/
/* 
/*                         用户物品容器类
/*
/*   提供添加、删除、管理用户物品的功能并实现相关消息发送以及日志记录的接口函数。是实现
/* 用户包裹、仓库系统的基础类。
/*
/************************************************************************/


class CUserItemContainer //:protected std::vector<CUserItem*>
{
public:
	// typedef std::vector<CUserItem*> Inherited;

	//定义物品操作参数，用于添加、删除以及更新一个物品
	struct ItemOPParam
	{
	public:
		BYTE	btQuality;
		BYTE	btStrong;
		WORD	wItemId;
		
		BYTE	btBindFlag;
		BYTE    btReserver;
		WORD	wCount;			//数量溢出
		WORD	wStar;			//强化星级
		BYTE	bLostStar;		//强化损失星级
		BYTE    btLuck;			//幸运
		WORD    wIdentifyslotnum;
		WORD    wIdentifynum;
		int		nSmith[CUserItem::MaxSmithAttrCount];
		//byte	nFiveIdx;			//五行类型
		//byte	nFivePropIdx;		//五行属性值
		
		int		nLeftTime; // 物品剩余时间。用于物品再添加的时候动态指定
		char    cBestAttr[200];  //极品属性
		//int		nStrongStar;

		//---来源
		BYTE	bInSourceType;		//来源
		WORD	nDropMonsterId;			//怪物id
		int		nAreaId;			//场景id
		char    cSourceName[200];  //归属昵称
		CMiniDateTime	nCreatetime;		//物品的创建时间，类型为CMiniDateTime

	public:
		inline ItemOPParam()
		{
			memset(this, 0, sizeof(*this));
			btBindFlag =-1;
		}
		void setSource(int nSourceType,int nTime, int nSceneId = 0, int nMonsterId = 0, const char* pKillerName = NULL)
		{
			memset(&this->cSourceName, 0, sizeof(this->cSourceName));
			if(pKillerName)
				memcpy(&this->cSourceName, pKillerName, sizeof(this->cSourceName));
			this->nAreaId = nSceneId;
			this->nCreatetime = nTime;//GetGlobalLogicEngine()->getMiniDateTime();
			this->bInSourceType =nSourceType;// tagItemSource::iqKillMonster;
			this->nDropMonsterId = nMonsterId;
		} 
		inline ItemOPParam(const INT_PTR nItemId, const INT_PTR nCount = 1, const INT_PTR nQuality = 0, 
			const INT_PTR nStrong = 0, const INT_PTR nBindFlag = -1, const INT_PTR nItemStrongStar = 0)
		{
			wItemId = (WORD)nItemId;
			btQuality = (BYTE)nQuality;
			wStar  = (WORD)nStrong;
			wCount = (WORD)nCount;
			btBindFlag = (BYTE)nBindFlag;
			nLeftTime	= 0;
			memset(&cBestAttr, 0, sizeof(cBestAttr)/sizeof(char));
		}
	};
public:
	CUserItemContainer()
		: m_hasDataModify(false)
		, m_nCapacity(0)
	{

	}
	/*
	* Comments:添加物品到容器中
	* Param const ItemOPParam & param:添加物品的参数，包含物品ID、数量、品质、强化、绑定标志；
	* Param LPCSTR lpSender:表示物品添加者的实体名称，主要用于记录日志中包含此实体的名称；
	* Param const INT_PTR nLogIdent：表示日志号，0表示不记录日志
	* Param BYTE bNoBatchUse：表示显示批量使用与否 0 显示 1不显示
	* @Return INT_PTR: 实际添加成功的物品的数量
	*/
	INT_PTR AddItem(const ItemOPParam& param, LPCSTR lpSender, const INT_PTR nLogIdent, BYTE bNoBatchUse = 0);
	
	/* 添加物品到容器中
	 * pUserItem用户物品对象；
	 * Sender	表示物品添加者的实体名称，主要用于记录日志中包含此实体的名称；
	 * nLogIdent表示本次物品添加操作的日志记录号，0表示不用记录日志；
	 * Param bool bNeedSendClient: 是否需要通知客户端
		bDupCount = false 能叠加的也强制不叠加
	 * @return	返回添加了多少个物品
	 */
	INT_PTR AddItem(CUserItem *pUserItem, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedSendClient=true,bool bDupCount=true, BYTE bNotice = 1);
	INT_PTR AddItem2depot(CUserItem *pUserItem, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedSendClient=true,bool bDupCount=true, BYTE bNotice = 1);
	
	/* 从物品容器中删除并销毁物品
	 * param	删除物品的参数，包含物品ID、数量、品质、强化；
	 * nQuality	物品品质，-1表示匹配所有品质；
	 * nQuality	物品强化等级，-1表示匹配所有强化等级;
	 * nDuraOdds 耐力差 = 最大耐力-当前耐力 -1表示不检测 检测少于此耐力差的物品符合条件
	 * @param pActor 如果传入这指针，则必须足够数量才扣
	 * @return	返回删除了多少个物品
	 */
	INT_PTR DeleteItem(const ItemOPParam& param, LPCSTR lpSender, const INT_PTR nLogIdent, const int nDuraOdds = -1/*, CActor* pActor = nullptr*/);
	
	/* 清空背包*/
	void DeleteAllItem();
	

	/*
	* Comments:删除玩家背包里指定指针同状态不同series的其他物品
	* Param CUserItem * pUserItem: 物品的指针
	* Param INT_PTR nCount: 要删除的数量
	* Param bool bReqSB:是否要判断强化与绑定条件
	* @Return INT_PTR: 实际删除的物品的数量
	*/
	INT_PTR DeleteOtherItem(CUserItem * pUserItem,INT_PTR nCount, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedFreeMemory =true,bool bReqSB = false);

	/*
	* Comments:从物品容器中移除一个物品
	* Param const CUserItem::ItemSeries series:物品系列号
	* Param bool bNeedFreeMemory:是否需要释放内存，如果否的话内存数据将保留
	* @Return bool:
	*/
	bool RemoveItem(const CUserItem::ItemSeries series, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedFreeMemory =true, int nMsgId = 0);
	
	/*
	* Comments:删除玩家身上的物品
	* Param CUserItem * pUserItem: 物品的指针
	* Param INT_PTR nCount: 要删除的数量
	* @Return INT_PTR: 实际删除的物品的数量
	*/
	INT_PTR DeleteItem(CUserItem * pUserItem,INT_PTR nCount, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedFreeMemory =true);

	/*
	* Comments:从物品容器中移除一个物品
	* Param const INT_PTR nIndex:物品在容器中的索引
	* Param bool bNeedFreeMemory: 是否需要释放内存，如果否的话内存数据将保留
	* @Return VOID:
	*/
	bool RemoveItem(const INT_PTR nIndex, LPCSTR lpSender,INT_PTR nLogIdent,bool bNeedFreeMemory =true, int nMsgId = 0);
	
	/* 查找一个用户物品
	 * wItemId	物品ID
	 * nQuality	物品品质，-1表示匹配所有品质；
	 * nQuality	物品强化等级，-1表示匹配所有强化等级;
	 * nMinCount 物品最低个数，1表示默认最少1个
	 * boRemove	参数用于规定在找到物品后是否将物品从容器中移除（只是移除，并不会销毁）；
	 * @return	返回第一个匹配的用户物品指针
	 */
	CUserItem* FindItem(const INT_PTR wItemId, const INT_PTR nQuality = -1, const INT_PTR nStrong = -1, WORD nMinCount = 1);
	

	/*
	* Comments:通过序列号查找无哦
	* Param const CUserItem::ItemSeries series: 物品的序列号
	* Param bool boRemove 是否需要删除
	* @Return CUserItem*:如果需要删除的话，那么找到就删除，返回NULL,否则返回查找的指针
	*/
	CUserItem* FindItemByGuid(const CUserItem::ItemSeries series);

	
	/* 通过物品系列号查找用户物品对象
	 * boRemove	参数用于规定在找到物品后是否将物品从容器中移除（只是移除，并不会销毁）；
	 * @return	返回第一个匹配的用户物品指针
	*/
	INT_PTR FindIndex(const CUserItem::ItemSeries series);
	
	/* 统计指定物品的数量
	 * wItemId	物品ID
	 * nQuality	物品品质，-1表示匹配所有品质；
	 * nQuality	物品强化等级，-1表示匹配所有强化等级;
	 * nDuraOdds 耐力差 = 最大耐力-当前耐力 -1表示不检测 检测少于此耐力差的物品
	 * void * pUserItem:如果存在物品指针，不计算该指针的数量
	 * @return	返回匹配的物品总数量
	 */
	INT_PTR GetItemCount(const INT_PTR wItemId, const INT_PTR nQuality = -1, const INT_PTR nStrong = -1,const INT_PTR nBind =-1,const INT_PTR nDuraOdds = -1, void * pNoItem = NULL);


	/*
	* Comments:拆分一个物品
	* Param const CUserItem::ItemSeries series: 物品的序列号
	* Param INT_PTR nCount: 拆分出的数量
	* @Return bool:成功返回true,否则返回false
	*/
	bool SplitItem(const CUserItem::ItemSeries series,INT_PTR nCount);


	/*
	* Comments: 合并物品，从源的物品合并到目的物品，如何合并完了就把删除
	* Param const CUserItem::ItemSeries srcSeries: 源的物品
	* Param const CUserItem::ItemSeries tgtSeries: 目的物品
	@ Param bool bBindInfect:是否绑定感染，即合并的两种物品只要有一种是绑定的，目标就全部变成绑定的
	* @Return bool:成功返回true,否则返回false
	*/
	bool MergeItem(const CUserItem::ItemSeries srcSeries,const CUserItem::ItemSeries tgtSeries, bool bBindInfect=false);

	/*
	* Comments: 获取能够叠加上去的数量
	* Param const ItemOPParam & param: 物品的信息
	* Param const CUserItem * pUserItem:目标的指针
	* @Return INT_PTR: 能够叠加上去的数量
	*/
	INT_PTR GetCanOverlagCount(const ItemOPParam& param,const CUserItem * pUserItem);

	/*
	* Comments: 获取源物品能叠加到目标物品上去的数量
	* Param const CUserItem * pSrcItem: 源物品对象指针
	* Param const CUserItem * pDestItem: 目标物品对象指针
	* Param bool bBindInfect：是否绑定感染
	* @Return INT_PTR: 返回源物品能够叠加到目标物品的数量
	* @Remark: 
	*/
	INT_PTR GetCanOverlapCountEx(const CUserItem *pSrcItem, const CUserItem *pDestItem, bool bBindInfect=false);

	//自动合并背包物品
	void ArrangeItemList(int nLogId);

public:
	// inline operator CUserItem** () const
	// {
	// 	return (CBaseList<CUserItem*>::operator CUserItem**());
	// }
	
	//清空容器中的物品，如果nLogIdent不为0则记录日志并向客户端发送
	INT_PTR Clear(LPCSTR lpSender = NULL, const INT_PTR nLogIdent = 0);

	//设置物品容器的容积
	inline VOID setCapacity(const INT_PTR nCapacity)
	{
		m_nCapacity = nCapacity;
	}

	//获取容器中当前物品数量
	// inline INT_PTR count()const{ return Inherited::count(); }

	//获取容器的容量上限
	inline INT_PTR capacity(){ return m_nCapacity; }

	//获取容器的空闲数量
	inline INT_PTR availableCount(){ return m_nCapacity - count(); } //delete

	//剩余格数是否满足
	bool bagIsEnough(int nType, int nNum = 0);
	/*
	* Comments: 能否添加一批物品，还没实现
	* Param CVector<ItemOPParam> & itemList: 物品列表
	* @Return bool:能够添加返回true，否则返回false
	*/
	bool CanAddItems( CVector<ItemOPParam> &itemList);

	/*
	* Comments: 能否添加物品，还没实现
	* Param const ItemOPParam & param: 物品的信息
	* @Return bool:能够添加返回true，否则返回false
	*/
	bool CanAddItem(const ItemOPParam& param,  bool bNeedCkeckCell = false);

	/*
	* Comments: 能否添加物品
	* Param const CUserItem * pUserItem: 物品的指针
	* @Return bool:能返回true，否则返回false
	*/
	bool CanAddItem( CUserItem * pUserItem, bool bNeedCkeckCell = false);

	/*--策划要求
	* Comments: 添加物品需要的格子的数量，//这里将废弃 考虑叠加的情况如果能够叠加到一个现有的格子上面，那么将返回0
	* Param const ItemOPParam & param: 物品的信息
	* @Return INT_PTR: 返回需要的格子数量
	*/
	INT_PTR GetAddItemNeedGridCount(const ItemOPParam& param);

	//----一支穿云箭 千军万马来相见
	/*
	* Comments: 获取容器的空闲数量
	* Param int& type:背包类型
	* @Return INT_PTR 剩余格数
	*/
	inline INT_PTR availableCount(WORD type);

	/*
	* Comments: 使用格数
	* Paramint& type: 背包类型
	*/
	void  addCostCount( WORD type, int nNum = 1);

	/*
	* Comments: 获取背包最大格数
	* Paramint& type: 背包类型
	*/
	int  GetBagCount(WORD type);
	/*
	* Comments: 获取背包最小剩余格数
	*/
	INT_PTR  availableMinCount();

	//最小剩余格数是否满足给定数值
	bool bagLeftEnoughGiveNum(int num);

	inline VOID SetBagNums(const INT_PTR maxNum)
	{
		m_nBagNums = maxNum;
	}

	inline VOID PushBagMaxCount(const INT_PTR maxNum)
	{
		m_BagMaxCounts.push_back(maxNum);
	}

	inline bool UpdateBagMaxCounts(const WORD& type,const INT_PTR maxNum)
	{
		if((type < 0) || (type > m_nBagNums -1))
			return false;
		m_BagMaxCounts[type] = maxNum;
	}
	//----
	inline int count() {return m_pUserItemList.size();};

    inline CUserItem* operator[](size_t __n)
      {return m_pUserItemList[__n];}
protected:
	/** 相关消息操作的接口虚函数 **/
	/* 记录获得新物品的日志
	 * item			新物品对象
	* pStdItem		物品对应的标准物品配置对象
	 * lpSender		物品给予者名称
	 * nLogIdent	日志消息号
	 */
	virtual VOID LogNewItem(const CUserItem *pUserItem, const CStdItem *pStdItem, LPCSTR lpSender, const INT_PTR nLogIdent) const = 0;
	/* 记录物品数量变更的日志
	* item			物品对象
	* pStdItem		物品对应的标准物品配置对象
	 * nCountChg	物品变更的数量，负数表示扣除了物品，正数表示获得了物品
	 * lpSender		物品给予或收取者名称
	 * nLogIdent	日志消息号
	 */
	virtual VOID LogItemCountChange(const CUserItem *pUserItem, const CStdItem *pStdItem, const INT_PTR nCountChg, LPCSTR lpSender, const INT_PTR nLogIdent) const  = 0;
	/* 向角色发送获得物品的消息 */
	virtual VOID SendAddItem(const CUserItem *pUserItem,INT_PTR nLogIdent, BYTE bNoBatchUse = 0, BYTE bNotice = 1) const  = 0;
	/* 向角色发送物品数量变更的消息，必须在修改了物品数量后调用此函数 */
	virtual VOID SendItemCountChange(const CUserItem *pUserItem, bool isGetNewCount = false, BYTE bNoBatchUse = 0) const  = 0;
	/* 向角色发送删除物品的消息 */
	virtual VOID SendDeleteItem(const CUserItem *pUserItem,INT_PTR nLogIdent) const  = 0;

	//当物品添加成功到里边调用，主要用来发系统提示的
	virtual VOID OnAddItem(const CStdItem * pItem, INT_PTR nItemCount,INT_PTR nLogIdent,const CUserItem *pUserItem=NULL){}

	//物品变动日志
	virtual void SendItemChangeLog(int nType, int nCount, int nItemId, int nLogId = 0,LPCSTR sDes = NULL){ }
	
	//当删除了几个物品，用于发系统提示
	virtual VOID OnDeleteItem(const CStdItem * pItem, INT_PTR nItemCount,INT_PTR nLogIdent, int nMsgId = 0,const CUserItem *pUserItem=NULL){}

	// 添加指定数量的物品，主要用于发系统提示
	virtual VOID OnAddItemById(int itemId, INT_PTR nItemCount, INT_PTR nLogIdent,const CUserItem *pUserItem=NULL){}
	//容器里的数据已经被修改
	virtual VOID OnDataModified() {};

	//通知客户端一个装备的数据改变了(比SendItemCountChange()更加全面)
	virtual VOID NotifyItemChange(CUserItem *pUserItem) {};

protected:
	//将物品叠加到物品容器的现有物品中
	inline INT_PTR OverlapToExists(const ItemOPParam& param, INT_PTR nCount, LPCSTR lpSender, const INT_PTR nLogIdent, BYTE bNoBatchUse = 0);
protected:
	INT_PTR		m_nCapacity;		//定义物品容器的容量上限
	bool        m_hasDataModify;   //数据是否发生了改变
public:
	std::vector<CUserItem*>   m_pUserItemList; //物品信息
	int                  m_nBagNums;            //定义多少物品容器
	std::vector<int>     m_BagCostCounts; //所有背包消耗的格数 //1: 背包类型  2：表示用多少
	std::vector<int>     m_BagMaxCounts; //所有背包消耗的格数 1：表示最大 
};
