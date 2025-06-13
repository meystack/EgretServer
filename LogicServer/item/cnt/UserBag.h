#pragma once


#define ITEM_SUNWATER			349						//太阳水
#define ITEM_SUPER_SUNWATER		350						//强效太阳水


class CAnimal;

class CUserBag :
	public CUserItemContainer,
	public CEntitySubSystem<enBagSystemID, CUserBag, CAnimal>
{
	friend class CUserEquipment;
	
public:
	typedef CUserItemContainer Inherited;
	typedef CEntitySubSystem<enBagSystemID, CUserBag, CAnimal> InheritedSybSystem;
	friend class  CEntitySubSystem<enBagSystemID, CUserBag, CAnimal>;

	const static int s_nMaxItemCDGroupCount = 30;

	enum enLuckAwardOpt
	{
		laoGetBigGift,// 0:领取百服庆典福利
		laoviewBigGift,//1:查看百服庆典福利领取情况
		laoRequestConsumeRank,// 2:请求消费排名
		laoProperty,// 3:查看属性大提升的次数
		laoExchange,//4:纪念币兑换
		laoGetLeftTime,//5查询活动剩余时间
		laoGetLLKScore,//6查看连连看分数最高玩家
		laogetActivityStatus,//7查询百服缤纷兑换状态
	};
	enum enOpId
	{
		enGetBigGift =3,//获取庆典福利
		enViewBigGift=4,//查看庆典福利领取情况
		enViewPropertyCount=5,//查看属性大提升完成情况
		enAddPropertyCount=6,//增加属性大提升 完成次数
		enExchangeCoin=7,//兑换纪念币
		enSetLLKScore=8,//设置连连看积分
		enViewLLKScore= 9,//查看连连看分数
		enViewActivityStatus=10,//查看百服缤纷状态
		enUpdateNewHundredRank = 11, // 更新新百服活动排行榜
	};
public:
	/*** 覆盖CEntitySubSystem父类的函数集 ***/
	//初始化
	virtual bool Initialize(void *data,SIZE_T size);
	//析构
	virtual VOID Destroy();
	//发送物品变动日志
	void SendItemChangeLog(int nType, int nCount, int nItemId, int nLogId = 0,LPCSTR sDes= NULL);

	//获取品质的颜色描述字符串
	static char * GetQualityColorStr(int nQuality)
	{
		switch(nQuality)
		{
		case CStdItem::iqItemWhite:
			return "FFFFFFFF";
			break;
		case CStdItem::iqItemGreen:
			return "FF0AB41E";
			break;
		case CStdItem::iqItemBlue:
			return "FF147ED7";
			break;
		case CStdItem::iqItemPurple:
			return "FFBC40F6";
			break;
		case CStdItem::iqItemRed:
			return "FFFF4848";
			break;
		case CStdItem::iqItemOrange:
			return "FFFF8F45";
			break;
		default:
			return "FFFFFFFF";
			break;
		}
	}
	
	/*
	* Comments: 玩家的数据存盘
	* Param PACTORDBDATA pData:角色数据指针
	* @Return void:
	*/
	virtual void Save(PACTORDBDATA  pData);
	
	/*
	* Comments: 处理网络数据包
	* Param INT_PTR nCmd: 命令码
	* Param CDataPacketReader & packet: 数据
	* @Return VOID:
	*/
	virtual VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet ); 
	

	/*
	* Comments: db 返回数据
	* Param INT_PTR nCmd:命令码
	* Param INT_PTR nErrorCode:错误码
	* Param CDataPacketReader & reader:数据读取器
	* @Return VOID:
	*/
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	/*
	* Comments: 会话 返回数据
	* Param INT_PTR nCmd:命令码
	* Param INT_PTR nErrorCode:错误码
	* Param CDataPacketReader & reader:数据读取器
	* @Return VOID:
	*/
	virtual VOID OnSsRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	/* 定时处理函数
	*  主要用于检查哪些物品的时间是否已经到期
	*/
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount);


	void OnEnterGame();

	/*
	* Comments: 修改背包里的一件装备的属性
	* Param CUserItem * pUserItem:装备的指针
	* Param INT_PTR nPropertyID: 属性ID
	* Param INT_PTR nValue: 新的数值
	* @Return bool:成功返回true,否则返回false
	*/
	bool SetItemProperty(CUserItem * pUserItem,INT_PTR nPropertyID,INT_PTR nValue);
	
	/*
	* Comments:获取物品的链接
	* Param WORD wItemId:物品ID
	*Param CUserItem * pUserItem,物品的指针，普通物品可以为空
	* @Return char *: 返回物品的描述"装备的话<#b%s/v%d,%d,%lld>,普通物品<#b%s/v%d>
	*/
	static char * GetItemLink(WORD wItemId, const CUserItem * pUserItem =NULL);
	//获取useritem链接的格式
	static char * GetUseItemLinkFormat(const CUserItem* pUserItem);
	//获取寄卖物品的链接，点击弹出购买框
	static char * GetConsignItemLink(const CUserItem *pUserItem, unsigned int nPrice);

	/*
	* Comments:获取一件物品的详细的日志信息，包括强化，品质以及开孔的数目，宝石的信息
	* Param CUserItem * pUserItem:物品的指针
	* @Return char *:返回一个字符串，包括物品的信息count:quality:strong_holecount:ddiamond1:diamond2:diamond3
	*/
	static char * GetItemDetailLogStr(const CUserItem *pUserItem );

	/*
	* Comments:获取一件物品中的部分属性放到MidStr0字段上
	* Param const CUserItem * pUserItem:
	* @Return char *:
	*/
	static char * GetItemMidStr1(const CUserItem * pUserItem);

	/*
	* Comments:获取一件物品中的部分属性放到MidStr1字段上
	* Param const CUserItem * pUserItem:
	* @Return char *:
	*/
	static char * GetItemMidStr2(const CUserItem * pUserItem);

	//添加一个卖给商店的物品，如果成功返回true
	bool AddSellItem(CUserItem * itemPtr);

	//删除一个卖给物品的指针，如果成功返回true
	bool DelSellItem(CUserItem *pUserItem);

	//获得卖给商店的物品的指针
	CUserItem *  GetSellItem(unsigned long long guid);

	//通知客户端一个装备的数据改变了
	virtual void NotifyItemChange(CUserItem *pUserItem);

	/*
	* Comments: 玩家进入场景处理
	* @Return void:
	*/
	void OnEnterScene();

	/*
	* Comments:玩家扩展背包
	* Param int nGridCount:扩充的背包的数量
	* @Return bool:成功返回true，否则返回false
	*/
	bool EnlargeBag(int nGridCount);


	/*
	* Comments: 删除指定类型的物品
	* Param int nType: 物品类型
	* Param const char * sComment: 注释
	* Param int nLogId: 日志Id	
	* @Return INT_PTR: 返回删除的物品数目
	* @Remark:
	*/
	INT_PTR RemoveItemByType(int nType, const char * sComment, int nLogId);

	/*
	* Comments: 查询指定类型的物品
	* Param int nType: 物品类型
	* @Return CUserItem*: 返回第一个搜索到的物品对象指针。如果没找到，返回NULL
	* @Remark:
	*/
	CUserItem* GetItemByType(int nType);

	/*
	* Comments:是否有装备强化达到了指定的等级。一般任务系统使用到
	* Param INT_PTR nStrong:强化值
	* @Return bool:
	*/
	bool IsStrongLevel(INT_PTR nStrong);

	/*
	* Comments:是否有精锻过的物品，一般任务系统使用到
	* @Return bool:
	*/
	//bool IsForgeItem();

	/*
	* Comments:获取耐久度满的物品
	* Param INT_PTR nItemId:物品id
	* @Return CUserItem*:返回NULL表示没有
	*/
	CUserItem* GetDuraFullItemById(INT_PTR nItemId);

	//设置数据是否修改过了
	virtual VOID OnDataModified()
	{
		SetDataModifyFlag(true);
	}

	/* 
	* Comments: 获取限时物品 限时时间
	* Param CUserItem * pUserItem: 物品指针
	* @Return unsigned int:  返回限时时间
	*/
	unsigned int GetItemLeftTime(CUserItem *pUserItem);

	/*
	* Comments: 减少物品使用的次数(耐久)
	* Param CUserItem * pUserItem: 物品的指针
	* Param int nCount:删除物品耐久数量级 nCount*m_dwUseDurDrop
	* Param LPCSTR sComment:  备注
	* Param int nLogID: 物品的日志号
	* Param bool bNeedFreeMemory:  是否需要释放内存
	* @Return int: 减少的次数
	*/
	INT_PTR RemoveItemDura(CUserItem * pUserItem, INT_PTR nCount,LPCSTR sComment,INT_PTR nLogID,bool bNeedFreeMemory =true);

	/*
	* Comments:到0点时清空锋利值
	* @Return void:
	*/
	void ClearEquipSharp(bool boCast);


	/* 
	* Comments: 
	* Param BYTE nActivityId: 活动id
	* Param BYTE nOpId:  操作id
	* Param bool bCloseNeedTrigger: 活动结束了是否需要触发脚本
	* Param INT_PTR nParam: 
	* @Return void:  
	*/
	void TriggerHundredEvent(BYTE nActivityId, BYTE nOpId, bool bCloseNeedTrigger = true, INT_PTR nParam = -1);
		/* 
	* Comments: 检查人物身上静态变态,不是当前百服活动就清空
	* @Return void:  
	*/
	void CheckHundrerActorStaticVar();

	//设置活动开关
	void SendHundredActInit(int nType); 
	//发送百服活动开关
	void SendHundredAct(int nType);

	/*
	* Comments: GM添加物品都调用这个功能
	* Param INT_PTR nItemID:		物品的ID
	* Param INT_PTR nCount:			物品的数量
	* Param INT_PTR nStar:			物品的强化星级
	* Param INT_PTR nLostStar:		物品强化损失星级
	* Param INT_PTR nBind			绑定
	* Param INT_PTR nInscriptLv		铭刻等级
	* Param INT_PTR nAreaId	铭刻经验
	* Param INT_PTR nTime:			物品时间
	* @Return bool: 成功返回true，否则返回false
	*/
	bool AddItemByItemId(INT_PTR nItemID, INT_PTR nCount, INT_PTR nStar=0, INT_PTR nLostStar=0,INT_PTR nBind=0, INT_PTR nInscriptLv =0, INT_PTR nAreaId=0,INT_PTR nTime=0); 


protected:
	/*** 覆盖CUserItemContainer父类的函数集 ***/
	virtual VOID SendDeleteItem(const CUserItem *pUserItem,INT_PTR nLogIdent) const ;
	virtual VOID LogNewItem(const CUserItem *pUserItem, const CStdItem *pStdItem, LPCSTR lpSender, const INT_PTR nLogIdent) const ;
	virtual VOID LogItemCountChange(const CUserItem *pUserItem, const CStdItem *pStdItem, const INT_PTR nCountChg, LPCSTR lpSender, const INT_PTR nLogIdent) const ;
	virtual VOID SendAddItem(const CUserItem *pUserItem,INT_PTR nLogIdent, BYTE bBatchUseShow = 0, BYTE bNotice = 1) const ;
	virtual VOID SendItemCountChange(const CUserItem *pUserItem, bool isGetNewCount = false, BYTE bBatchUseShow = 0) const ;
	//当物品添加成功到里边调用，主要用来发系统提示的
	
	//添加物品的提示
	virtual VOID OnAddItem(const CStdItem * pItem,INT_PTR nCount,INT_PTR nLogIdent,const CUserItem *pUserItem=NULL);
	//删除物品的提示
	virtual VOID OnDeleteItem(const CStdItem * pItem, INT_PTR nItemCount,INT_PTR nLogIdent, int nMsgId = 0,const CUserItem *pUserItem=NULL);
	//添加物品的提示
	virtual VOID OnAddItemById(int itemId, INT_PTR nItemCount, INT_PTR nLogIdent,const CUserItem *pUserItem=NULL);

	//处理网络消息包，获取背包扩展的费用
	void OnGetBagEnlargeFee();

	//处理网络消息包，扩展背包
	void OnEnlargeBag();

	void DealUserItem();
	
	/*
	* Comments: 使用物品
	* Param CUserItem * pUserItem: 指针
	* Param CStdItem * pStdItem: 标准物品指针
	* Param int nCount: 使用个数，默认为1
	* Param bool isHeroUse : 是否是给英雄使用的
	* @Return void:
	*/
	bool OnUseItem(CUserItem *pUserItem, const CStdItem * pStdItem, int nCount = 1, bool isHeroUse =false, int nParam = 0, bool bNeedRes = true);

	/*
	* Comments: 发送背包的物品改变的系统提示
	* ParamconstINT_PTR nItemID: 物品指针
	* Param INT_PTR nCount: 改变的数量,可以为正，也可以为负数
	* Param CUserItem * pUserItem: 物品的指针
	* Param INT_PTR nLogIndex: 物品的来源
	* Param int nMsgId: 提示信息id
	* @Return void:
	*/
	void SendBagItemChangeTipmsg(INT_PTR nItemID,INT_PTR nCount,const CUserItem * pUserItem,INT_PTR nLogIndex, int nMsgId = 0) const;

	/*
	* Comments:获取背包扩展的费用，格子数量，如果不能扩展，返回错误码
	* Param INT_PTR & nFee:费用，这个也是返回参数
	* Param INT_PTR & nGridCount:可以添加的格子的数量，这个也是返回参数
	* Param INT_PTR & nMoneyType:需要下消耗的金钱，这个也是返回参数
	* @Return INT_PTR:如果能够扩展返回true,否则返回false
	*/
	INT_PTR GetBagEnlargeFeeErorCode(INT_PTR &nFee,INT_PTR & nGridCount,INT_PTR& nMoneyType);
	
	void HandError(CDataPacketReader &packet){};
	void HandAddItem(CDataPacketReader &packet); //添加物品   
	void HandDelItem(CDataPacketReader &packet); //删除物品
	void HandQueryItems(CDataPacketReader &packet); //获取背包列表
	void HandGetBagEnlargeFee(CDataPacketReader &packet); //获取扩大背包费用
	void HandEnlargeBag(CDataPacketReader &packet); //获取扩大背包
	void HandSlpitItem(CDataPacketReader &packet); //拆分
	void HandMergeItem(CDataPacketReader &packet); //合并
	void HandUseItem(CDataPacketReader &packet); //使用物品
	
	void HandProcessItem(CDataPacketReader &packet);//处理装备
	void HandGetProcessItemConsume(CDataPacketReader &packet); //获取强化一件装备需要的消耗
	void HandGetItemProcessConfig(CDataPacketReader &packet); //获取物品处理的配置

	void HandActiveBagList(CDataPacketReader &packet); //获取活动背包的列表内容
	void HandGetActiveBagItem(CDataPacketReader &packet); //领取某个活动背包的物品

	void HandDestroyCoin(CDataPacketReader &packet);   //消耗背包里的银两
	
	void HandleExChangeMoney(CDataPacketReader &packet);//兑换金币

	void DebugRevoceItem(int nType);

	//新增背包道具的特殊属性
	void HandQueryItemsExtraInfo(CDataPacketReader &packet);
	void HandQueryItemsExtraInfoOne(CDataPacketReader &packet);
	
	/*
	依据客户端请求type
	*/
	void HandleRecoverItem(CDataPacketReader &packet);// 回收
	/*
	* Comments:下发消费排名
	* @Return void:
	*/
	void GetConsumeRank();
	
	/*
	* Comments:领取百服庆典福利
	* @Return void:
	*/
	void GetBigGiftBag(BYTE nId);
	 
	
	/*
	回收道具
	*/
	bool RecoverItem(CUserItem* pItem, const CStdItem* pStdItem,bool IsreCoverBast = false);
	/*
	* Comments:发送玩家的活动背包数据
	CDataPacketReader &reader:会话服务器返回的数据
	* Param INT_PTR nType:1:账户绑定，2是角色绑定
	* @Return void:
	*/
	void SendActiveItem(CDataPacketReader &reader,INT_PTR nType);

	/* 
	* Comments:发送活动物品到角色邮箱
	* Param CDataPacketReader & reader:
	* @Return void:
	*/
	void SendActiviItemByMail(CDataPacketReader & reader);

	/*
	* Comments:给予用户后台奖励的物品
	* Param CDataPacketReader & reader:会话或者数据库服务器返回的数据内容
	* Param INT_PTR nType:账户绑定还是角色绑定
	* @Return void:
	*/
	void GetActiveItem(CDataPacketReader &reader,INT_PTR nType);

	void OnGetSACardInfo(CDataPacketReader &reader, bool bQuery);
public:
	//获取装备的系统提示用到的格式化的字符串
	static char s_ItemTipmsgBuff[1024]; 
	static char s_ConsignItemLinkBuff[1024]; //寄卖物品的链接
	static char s_UserItemFormat[512]; 
	//存一些鉴定属性在MidStr0和MidStr1的字段上
	static char s_ItemMidStr1[32];
	static char s_ItemMidStr2[32];

	/* 记录物品数量变更的日志
	* item			物品对象
	* pStdItem		物品对应的标准物品配置对象
	 * nCountChg	物品变更的数量，负数表示扣除了物品，正数表示获得了物品
	 * lpSender		物品给予或收取者名称
	 * nLogIdent	日志消息号
	 */
	static VOID LogChangeItemCount(const CUserItem *pUserItem, const CStdItem *pStdItem, const INT_PTR nCountChg, LPCSTR lpSender, const INT_PTR nTgtId, LPCSTR sTargetName, const INT_PTR nLogIdent, LPCSTR sTargetAccount = "");
	/*
	* Comments: 向客户端发送物品时间到期的消息
	* Param CActor * pActor:
	* Param const CUserItem * pUserItem:
	* Param INT_PTR nPos: 物品位置，0表示背包，1表示装备，2表示仓库
	* @Return VOID:
	*/
	static VOID SendItemTimeUp(CActor *pActor, const CUserItem *pUserItem, INT_PTR nPos);



	/*
	* Comments: 使用增值卡
	* Param UINT64 nCardSeries:增值卡序列号
	* Param bool bQueryOrUse: 标记查询还是使用。true为查询，false为使用
	* @Return void:
	*/
	void UseServicesAddedCard(UINT64 nCardSeries, bool bQueryOrUse);

	void OnUseSACardResult(int nResult, int nErrorCode, UINT64 nSN, int nType, int nSubType, bool bQuery);

	void OnQuerySACardResult(int nResult, int nErrorCode, UINT64 nSN, int nType, int nSubType);
	// 检测物品使用是否进入CD。 如果物品在CD中返回true；否则返回false
	bool CheckUseItemCD(int nGroupId);
	// 物品使用后进入CD
	void ItemUseEnterCD(int nGroupId, int nCDTime);

	// 请求活动背包数据
	void ReqActiveBagData();

	/*
	* Comments:判断任务强化
	* Param INT_PTR propId:强化操作id
	* @Return void:
	*/
	void BagQuestTarget(INT_PTR propId );

	/*
	* Comments:获取能升级的配置
	* Param CDataPacketReader & packet:数据包
	* @Return void:
	*/
	void HandGetUpgradeCfg(CDataPacketReader &packet);

	void HandUseStrongItem(CDataPacketReader &packet);
	void HandSmashGoldEgg(CDataPacketReader &packet);
	void HandLuckAwardOpt(CDataPacketReader &packet);
	void HandGetPerfectCompTimes(CDataPacketReader &packet);
	void HandUseFiveAttrItem(CDataPacketReader &packet);
	void HandUseItemRecoverEx(CDataPacketReader &packet);
	void HandBatchUseItem(CDataPacketReader &packet);
	void HandArrangeItemList(CDataPacketReader & packet);//整理背包
	void HandMergeItemBindInfect(CDataPacketReader &packet);
	/*
	* Comments:获取所有数量
	* @Return INT_PTR:
	*/
	INT_PTR GetAllItemCount();

	/*
	* Comments:通过索引 取物品
	* Param INT_PTR nIdx:
	* @Return CUserItem *:
	*/
	CUserItem * GetItemByIdx(INT_PTR nIdx);

	
	/*
	* Comments: 根据物品指针获取物品在背包的索引
	* Param void * pEntity:
	* @Return void:
	* @Remark:
	*/
	int GetBagItemIndex(CUserItem * itemPtr);


	//查找物品 
	//nItemPos:	1背包 2玩家装备 3英雄装备
	//nHeroId:如果是英雄装备则为英雄ID
	//nEntityId: <=0则遍历查找 如果查找到则赋值nItemPos与nHeroId 如果>0用作heroId
	CUserItem *GetEquipItemPtr(const CUserItem::ItemSeries itemGuid, int &nItemPos, int &nHeroId, int nEntityId=-1);

	/*
	* Comments:获取当前的可以添加经验的物品 
	* @Return CUserItem *:
	*/
	inline CUserItem * GetExpItem()
	{
		return m_pExpItem;
	}

	void AssignExpItem();		//指定一个经验道具作为当前吸收经验道具

	CUserItem* GetOneItemByItemTypeInBag(BYTE nItemType);

	void ChangeHpPotValue(CUserItem *pEquip,int nValue);

	//立即使用，放入背包之后立即使用
	void BatchUseItemAtOnce(WORD wItemId, WORD wCount);

	//批量使用
	void BatchUseItem(CUserItem* pUserItem, WORD wCount);

	CUserItem* GetItemPtrHighestStar(WORD  nItemId);
	void TestAddItem(int itemid, int itemNum);

public:		//装备的一些刷出的属性
	//----------------鉴定属性-------------------------------------------------------
	//是否有鉴定属性（不为全0）
	bool HasItemIdentifyAttrs( CUserItem* pUserItem );
	//转换鉴定属性
	void TransferItemIdentifyAttrs( CUserItem* pSrcUserItem, CUserItem* pDesUserItem );

	
//跨服相关数据
public:
	//发送数据到跨服
	void SendMsg2CrossServer(int nType);
	//跨服数据初始化
	VOID OnCrossInitData(std::vector<CUserItem>& vUseBags);

private:
	CVector<CUserItem*>	 m_sellUserList; //向商店卖的物品的指针列表,用于回购,在玩家下线的时候，这些指针的数据将删除
	CTimer<30*1000>		 m_ItemTimeChkTimer;									// 物品到期时间检查定时器	
	TICKCOUNT			 m_aItemCDGroupExpireTimeTbl[s_nMaxItemCDGroupCount];	// 物品冷却组到期时间表
	static int			 s_nEggYbCircleCount;
	
	bool				m_bGetActiveItemFlag;				//正在领取活动背包物品
	unsigned int		m_nUseXiuWeiCount;

	CUserItem *         m_pExpItem;     //当前可以吸收经验的经验道具（各种经验盒子）
};

