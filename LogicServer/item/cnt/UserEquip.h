#pragma once
#define MAXCHANGEDURASENDPACKET 100
class CActor;


class CUserEquipment:
	public CEquipVessel,
	public CEntitySubSystem<enEuipSystemID, CUserEquipment, CActor>
	
{
public:
	const static int MAX_EQUIP_STRONG_LEVEL =15; //最大的装备强化的等级
	const static int MAX_EQUIP_DIAMOND_LEVEL =10; //最大的装备强化的等级
	const static int MAX_EQUIP_LEVEL =10; //最大的装备等级

	//定义角色身上装备的数量
	//static const INT_PTR EquipmentCount = itMaxEquipPos;
	typedef CEntitySubSystem<enEuipSystemID, CUserEquipment, CActor> Inherited;
	friend class CEntitySubSystem<enEuipSystemID, CUserEquipment, CActor>;

	typedef  CEquipVessel EquipBaseClass;

	//装备，人物，坐骑的外观的数据结构的定义
	typedef struct tagEntityAppearValue
	{
		union
		{
			int nValue;
			struct 
			{
				WORD wLow;
				WORD wHigh;
			};
		};
	} ENTITYAPPEARVALUE;

	enum emPropMove{
		pmInitSmith = 1,
		pmStrong,
		pmAuth,
		pmLuck,
	};
public:
	/*** 覆盖父类的函数集 ***/
	//初始化
	virtual bool Initialize(void *data,SIZE_T size);

	//析构
	virtual VOID Destroy()
	{
		Clear(NULL, 0);
		Inherited::Destroy();
	}

	
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
	* Comments: 存盘
	* Param PACTORDBDATA pData: 数据指针
	* @Return void:
	*/
	virtual void Save(PACTORDBDATA  pData);

	virtual void OnEnterGame();
	//pos位置装备发生变化
	void EquipChange(int nPos);

	/*
	* 定时调用
	* 检查装备的到期时间
	* 这里要判断源泉类的物品添加的数值
	*/
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount); 
	//装备成就
	void DealEquipmentAchieve();

	/*
	* Comments:装备从传上去的时候调用
	* Param CAnimal * pOnwer:
	* Param INT_PTR nPos:
	* Param CUserItem * pUserItem:
	* Param INT_PTR nLocation:穿戴的位置，0表示左手，1表示右手
	* @Return void:
	*/
	virtual void AfterEquipTakenOn(CActor * pOnwer,INT_PTR nPos,CUserItem *pUserItem,INT_PTR nLocation=0);

	//装备脱下来的时候
	virtual void AfterEquipTakenOff(CActor * pOnwer,CUserItem *pUserItem);

	bool DeleteEquip(int nIdx, LPCSTR pSender, INT_PTR nLogIdent,bool bNeedFreeMemory=true);

	

	/* 从装备容器中删除并销毁物品
	 * param	删除物品的参数，包含物品ID、数量、品质、强化；
	 * nQuality	物品品质，-1表示匹配所有品质；
	 * nQuality	物品强化等级，-1表示匹配所有强化等级;
	 * nDuraOdds 耐力差 = 最大耐力-当前耐力 -1表示不检测 检测少于此耐力差的物品符合条件
	 * @return	返回删除了多少个物品
	 */
	INT_PTR DeleteEquip(const CUserItemContainer::ItemOPParam& param, LPCSTR lpSender, const INT_PTR nLogIdent, const int nDuraOdds = -1);

public:

	//判断 穿戴多少件多少等级的装备
	int GetEquipLevelCount(int nLevel);
	void HandGetRankOtherEquip(CDataPacketReader &packet);

	//判断 穿戴多少件熔铸装备
	int GetEquipMeltingCount(int nLevel);

	//判断 穿戴多少件多少转的装备
	int GetEquipCircleCount(int nCircle);
	
	//检查装备数量
	void CheckEquipCount();

	bool GetEquipByItemId(WORD nItemId);

	//穿上装备(直接用UserItem，不经过背包,如神炉系统)
	bool TakeOn(CUserItem* pUserItem);
	//穿上装备
	bool TakeOn(const CUserItem::ItemSeries &series);

	//取下装备
	bool TakeOff(const CUserItem::ItemSeries &series, bool boCheckGrid = true);

	
	/* 计算装备属性 */
	//CAttrCalc& CalcAttributes(CAttrCalc &calc);
	//清空装备物品，如果nLogIdent不为0则记录日志并向客户端发送
	INT_PTR Clear(LPCSTR lpSender = NULL, const INT_PTR nLogIdent = 0);


	//通过装备的位置获取装备
	inline CUserItem * GetEquipByPos(INT_PTR nPos)
	{
		if(nPos >= itWeaponPos && nPos < itMaxEquipPos)
		{
			return m_Items[nPos];
		}
		else
		{
			return NULL;
		}
	}

	
	/*
	* Comments: 获取第几件装备，比如第1件，第2件，用于脚本遍历装备
	* Param INT_PTR nSortID: 排序的ID，比如有6件装备，nSortID =1,6有效
	* @Return CUserItem *: 返回装备的指针
	*/
	CUserItem * GetEquipBySortID(INT_PTR nSortID)
	{

		INT_PTR nCount=0; //有效的装备的位置
		for(INT_PTR i=0; i< EquipmentCount; i ++)
		{
			if(m_Items[i] !=NULL )
			{
				nCount ++;
				if(nCount == nSortID)
				{
					return m_Items[i];
				}
			}
		}
		return NULL;
	}

	/*
	* Comments:计算装备的属性
	* Param CAttrCalc & cal:属性计算器
	* Param CAttrCalc * pOriginalCal:原始的属性计算器的指针，原始属性是不考虑装备耐久折损的属性，用于装备评分
	* @Return CAttrCalc &:属性
	*/
	CAttrCalc & CalcAttributes(CAttrCalc &cal,CAttrCalc *pOriginalCal=NULL);

	
	//刷玩家的外观的属性
	VOID RefreshAppear();

	/*
	* Comments: 改变一件装备的耐久度
	* Param CUserItem * pEquip:  装备的指针
	* Param int nValue: 改变的数值
	* Param bool bForce: 是否强制改变耐久
	* @Return bool:成功返回true,否则返回false
	*/
	bool ChangeEquipDua(CUserItem * pEquip,int nValue,bool bForce = false);

	/*
	* Comments: 向一个玩家发送物品的耐久发生改变的消息
	* Param CUserItem * pUserItem:物品的指针
	* Param CActor * pActor:玩家的actorID
	* @Return void:
	*/
	static void SendItemDuaChange(CUserItem * pUserItem, CActor *pActor);

	//攻击别人，要处理装备的耐久
	void OnAttackOther(CAnimal *pEntity, bool bChgDura=true);

	//被攻击,要处理装备的耐久
	void OnAttacked(CAnimal *pEntity);

	//死亡的时候需要减装备的耐久
	void OnDeath();



	void SetEquipItemProp(INT_PTR nSlotId, INT_PTR nPropId, INT_PTR nVal);

	/*
	* Comments:玩家进入场景处理
	* @Return void:
	*/
	void OnEnterScene();

	/*
	* Comments: 删除身上的指定类型的物品
	* Param int nType: 物品类型
	* Param const char * sComment: 注释
	* Param int nLogId: 日志Id
	* @Return INT_PTR: 返回删除的物品数量
	* @Remark:
	*/
	INT_PTR RemoveItemByType(int nType, const char * sComment, int nLogId);

	/*
	* Comments: 在装备栏搜索指定类型的物品
	* Param int nType: 物品类型
	* @Return CUserItem*: 返回第一个找到的满足条件的装备物品对象指针；如果找不到，返回NULL
	* @Remark:
	*/
	inline CUserItem* GetItemByType(INT_PTR nType)
	{

		INT_PTR nPos= GetItemTakeOnPosition( (const Item::ItemType) nType);
		if((nPos <0) || (nPos >= EquipmentCount)) return NULL;
		return m_Items[nPos]; 
	}

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
	* Comments:物品是否在装备栏上 
	* Param CUserItem * pItem:物品指针
	* @Return bool:在装备栏返回true
	*/
	bool IsItemInEquip( CUserItem * pItem );

	//void OnSmithEquip(); //精锻装备
	
	void OnTakeOnCheckSuit(); //穿上装备的时候检查套装

	void OnCheckDiamondLevel(); //检查宝石的等级

	void OnCheckEquipStrong();  //检查装备的强化
	
	//通过物品的id在装备列表里查找
	
	inline CUserItem* FindItemByID( const INT_PTR wItemId)
	{
		return EquipBaseClass::FindItemByID(wItemId);
	}

	/*
	* Comments:到0点时清空锋利值
	* @Return void:
	*/
	void ClearEquipSharp(bool boCast);
public:
	//获取装备的穿戴位置编号，返回-1表示物品不是装备
	//static INT_PTR GetItemTakeOnPosition(const Item::ItemType btItemType);
	
	/*
	* Comments:获取升星衍生的光效
	* Param CEquipVessel * pEquips:装备的指针
	* @Return INT_PTR:获取升星的外观
	*/
	static const INT_PTR GetStarAppear(const CEquipVessel *pEquips);

	/*
	* Comments:获取套装的光效
	* @Return INT_PTR:获取宝石的外观
	*/
	// static const INT_PTR GetSuitAppear(const CEquipVessel *pEquips);

	/*
	* Comments: 获取一个装备的外观，这里是分2个Uint16来存放的，高16位是宝石的，低16位是装备本身的
	* Param CUserItem * pUserItem: 装备的指针
	* Param bool showDiamind:是否显示宝石光效
	* @Return int:返回装备的外观
	*/
	static int GetEquipAppear(CUserItem * pUserItem);

	//获取足迹外观
	static int GetFootAppear(const CUserItem *pUserItem, bool showAppear = true);

	/*
	* Comments:获取装备
	* Param CEquipVessel * pEquips:装备的指针
	* Param bool showFation:是否显示时装
	* Param bool isZanzen:是否在打坐状态
	* Param int nVocation:职业
	* Param int nSex:    性别
	* Param int & nModelId:返回的模型的id
	* Param int & nWeaponAppear:返回武器的内观
	* Param int & nWingAppear:返回翅膀的外观
	* Param int & nSwingLevel:返回翅膀的id
	* Param int & nSoldierSoulAppear:返回兵魂外观
	* @param pMaster 用于获取光效
	* @Return void:
	*/
	static void GetAppear(const CEquipVessel *pEquips,int nVocation, int nSex, 
		int& nModelId,int &nWeaponAppear, int &nWingAppear, int &nSwingLevel, int &nSoldierSoulAppear,
		int nEntityType = enActor,CActor *pMaster=NULL);

	/*
	* Comments:玩家是否选择显示幻武外观
	* Param CUserItem * pItem:物品的指针
	* @Return bool:是否需要显示时装
	*/
	static bool NeedShowWeaponExtend( CActor *pMaster, const HERODBDATA *pPetData=NULL);


	CAttrCalc & CalcFashionAttr(CAttrCalc & cal);
	
	/*
	* Comments:装备吸收经验
	* Param INT_PTR nValue:原始的经验
	* @Return INT_PTR:吸收了多少经验
	*/
	INT_PTR OnAbsorbExp(INT_PTR nValue);
	//套装
	virtual void SuitAttrCal(CAttrCalc &calc);
private:
	//下面是一些网络数据包的处理
	void HandError(CDataPacketReader &packet){}; //这个是没有用的数据包
	void HandTakeOnEquip(CDataPacketReader &packet); //装上装备
	void HandTakeOffEquipWithGuid (CDataPacketReader &packet);//脱下一件装备，参数是装备的GUID
	//void HandTakeOffEquipWithPos(CDataPacketReader &packet); //脱下一件装备，参数是装备的位置
	void HandGetEquip(CDataPacketReader &packet); //获取自身的装备
	void HandGetOtherEquip(CDataPacketReader &packet); //获取其他玩家的装备
	void HandGetBelongActorInfo(CDataPacketReader &packet); //获取非活动副本Boss归属玩家信息

	void HandSetFathionClothFlag(CDataPacketReader &packet); //设置是否显示时装
 
	void HandGetOfflineEquip(CDataPacketReader &packet); //获取离线玩家的装备
	void HandGetHeroOfflineEquip(CDataPacketReader &packet); //获取离线玩家英雄的装备
	void HandEquipInsure(CDataPacketReader &packet); //装备投保

	void HandUseSenderRing(CDataPacketReader & packet);  //用传送戒指传送

	void HandGetSenderRingCd(CDataPacketReader & packet);  //请求传送戒指的CD时间

	void HandMoveEquipProp(CDataPacketReader & packet);		//装备转移
    
	void HandUpgradeStar(CDataPacketReader & packet); //装备升星

	void HandRecover(CDataPacketReader & packet); //装备分解

	void HandComposite(CDataPacketReader & packet); //装备合成

	void HandSetExtraEquipShowFlag( CDataPacketReader & packet );	//[幻武玄甲]设置幻武玄甲的外观是否显示

	void HandRecoverStar(CDataPacketReader & packet); //恢复装备强化失败损失的星级

	void HandTransferStar(CDataPacketReader & packet); //装备强化星级转移

	void HandInscript(CDataPacketReader & packet);	//装备铭刻

	void HandTransferInscript(CDataPacketReader & packet); //装备铭刻转移

	void HandIdentifySlotUnlock( CDataPacketReader & packet );			//鉴定属性槽解锁

	void HandIdentify( CDataPacketReader & packet );					//装备鉴定

	void HandTransferIdentify( CDataPacketReader & packet );			//装备鉴定转移
	/*
	* Comments:玩家是否选择显示时装
	* Param CUserItem * pItem:物品的指针
	* @Return bool:是否需要显示时装
	*/
	bool NeedShowFashion();

	

	bool DeleteEquip(CUserItem *pItem, LPCSTR pSender, INT_PTR nLogIdent, bool boDeedFreeMem = true);

	/*
	* Comments: 检测外形的改变
	* Param CUserItem * pUserItem:物品的指针
	* Param int nPropID: 属性的ID
	* @Return:
	*/
	inline VOID CheckAppearChange(CUserItem * pUserItem,int nPropID)
	{
		//外观
		int nOldAppear	= ((CEntity*)m_pEntity)->GetProperty<int>(nPropID);
		int nNewAppear   = GetEquipAppear(pUserItem); //获取外形
		if(nNewAppear != nOldAppear)
		{
			((CEntity*)m_pEntity)->SetProperty<int>(nPropID,nNewAppear);
		}
	}
	/*
	* Comments: 当装备穿戴上去的时候，初始化一些定时器
	* Param INT_PTR nPos:装备的穿戴的位置
	* Param CStdItem pStdItem:物品的指针
	* @Return void:
	*/
	void EquipTakenInitTimer(INT_PTR nPos,const CStdItem * pStdItem);

	/*
	* Comments: 装备耐久发生变化
	* Param CUserItem * pEquip:装备的指针
	* Param const CStdItem * pStdItem:物品的指针
	* @Return void:
	*/
	void OnEquipDurChanged(CUserItem *pEquip,const CStdItem * pStdItem);


	INT_PTR CalcWeaponEndureConsume();

	INT_PTR CalcDressEndureConsume();

	INT_PTR CalcArmorEndureConsume();

	INT_PTR GetRandomInRange(INT_PTR nUpper, INT_PTR nLower);

	//检查时装战力
	bool CheckSetFashion();
protected:
	//CUserItem*	m_Items[EquipmentCount];//装备物品表
	
	//下面这些用于源泉类的物品
	//CTimer<1000>      m_hpTimer;			//hp的源泉的定时器
	//CTimer<1000>      m_mpTimer;			//mp源泉的定时器

	CTimer<30*1000>	  m_ItemTimeChkTimer;	//物品时间检查的定时器
	TICKCOUNT		  m_nSenderRingCd;
	//HP,MP源泉
	WORD              m_wHpSourceInterval;   //HP源的作用时间间隔
	WORD              m_wHpSourceValue;     //每次添加的数值
	WORD              m_wMpSourceInterval;  //MP源的作用时间间隔
	WORD              m_wMpSourceValue;     //每次添加的数值
	CRandHit		  m_weaponEndureConsHit;	// 武器耐久消耗命中
	CRandHit		  m_dressEndureConsHit;		// 衣服耐久消耗命中	
	CRandHit		  m_armorEndureConsHit;		// 衣服外的其它护甲装备耐久消耗命中	
	unsigned int      m_nEquipScore ;   //装备的得分

	WORD			  m_ChangeDura[EquipmentCount];		//各部位装备改变
	//WORD			  m_nEquipPosStrongLevel[EquipmentCount];	//装备-部位强化数据(现在只存了等级)
	BYTE			  m_nIsExtraWeaponShow;					//是否显示幻武外观
	BYTE			  m_nIsExtraDressShow;					//是否显示玄甲外观

public:
	//跨服数据
	void SendMsg2CrossServer(int nType);
	//跨服数据初始化
	VOID OnCrossInitData(std::vector<CUserItem>& Equips);
	void HandGetCenterRankOtherEquip(CDataPacketReader &packet);
};
