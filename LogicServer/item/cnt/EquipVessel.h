#pragma once


/*
  装备容器的基类，为了支持宠物的装备，以及以后其他的各种的装备容器，从装备类里把一些基础的函数抽取出来组成了一个类
  
*/



class CEquipVessel
{
public:
	static const INT_PTR EquipmentCount = itMaxEquipPos;
	
	CEquipVessel()
	{
		clear();
		m_nSuitList.clear();
	}
	void clear()
	{
		memset(m_Items, 0, sizeof(m_Items));
	}
	//穿上神炉装备(直接用UserItem，不经过背包)
	bool TakeOnGodStove(CActor *pActor, CUserItem* pUserItem);
	//穿上装备
	/*
	* Comments:穿上装备
	* Param CAnimal * pOner:穿戴的实体的指针
	* Param const CUserItem::ItemSeries & series:
	* Param const PETDBDATA *pPetData: 宠物的数据指针，大部分情况下不用配置
	* Param INT_PTR nPos：穿戴的位置，如果0表示是找个合适的位置，1表示是左手，1表示是右手
	* Param bool isBagEquip 是否背包里的装备，（如果不是背包上的装备，不经背包那套处理，而且装备位置上不能有旧装备），现用于神炉系统
	* @Return INT_PTR:
	*/
	bool TakeOnEquip(CActor *pActor, const CUserItem::ItemSeries &series,const HERODBDATA *pPetData =NULL,INT_PTR nLocation =0, bool isBagEquip = true);
	bool TakeOnEquip(CActor *pActor,CUserItem* pUserItem,const HERODBDATA *pPetData =NULL,INT_PTR nLocation =0, bool isBagEquip = true);
	//取下装备
	bool TakeOffEquip(CActor *pOner, const CUserItem::ItemSeries &series, bool boCheck = true, bool boHero=false);

	//取下装备
	bool TakeOffEquip(CActor *pOner,CUserItem *pUserItem, bool boCheck = true, bool boHero=false);

	
	/*
	* Comments:删除装备，注意，这里的物品指针是不负责删除的，调用者自己负责删除
	* Param CActor * pOwner:主人
	* Param CUserItem * pUserItem:数据的指针
	* @Return bool:正确返回true，否则返回false
	*/
	bool DirectRemoveEquip(CActor *pOwner,CUserItem *pUserItem, LPCSTR pSender="", INT_PTR nLogIdent=0, bool boDeedFreeMem=false);

	
	//通过物品系列号获取装备在列表中的位置，没有找到则返回-1
	INT_PTR FindIndex(const CUserItem::ItemSeries series);

	//通过GUID获取装备
	inline CUserItem *GetEquipByGuid(const CUserItem::ItemSeries series)
	{
		INT_PTR nIndex = FindIndex(series);
		if(nIndex <0) return NULL;
		return m_Items[nIndex];
	}

	/*
	* Comments:获取玩家当前已经装备的装备的数量
	* @Return INT_PTR: 当前身上穿在身上的装备的数量
	*/
	INT_PTR GetEquipCount()
	{
		INT_PTR nCount=0; //有效的装备的位置
		for(INT_PTR i=0; i< EquipmentCount; i ++)
		{
			if(m_Items[i] !=NULL )
			{
				nCount ++;
			}
		}
		return nCount;
	}

	/*
	* Comments:检测各套装的件数
	* Param CSuitCal & suitCal:套装的计数器
	* @Return void:
	*/
	// void CheckSuit(CSuitCal &suitCal) const;

	//通过物品的id在装备列表里查找
	CUserItem* FindItemByID( const INT_PTR wItemId)
	{
		for (INT_PTR i=0; i<EquipmentCount; i ++)
		{
			if ( m_Items[i] && m_Items[i]->wItemId == wItemId)
			{
				return m_Items[i];
			}
		}
		return NULL;
	}


	//通过物品的扣掉玩家身上的装备
	bool DelItemByID( const INT_PTR wItemId)
	{
		for (INT_PTR i=0; i<EquipmentCount; i ++)
		{
			if ( m_Items[i] && m_Items[i]->wItemId == wItemId)
			{
				m_Items[i] = NULL;
				return true;
			}
		}
		return false;
	}

	/* 统计指定物品的装备数量
	 * wItemId	物品ID
	 * nQuality	物品品质，-1表示匹配所有品质；
	 * nQuality	物品强化等级，-1表示匹配所有强化等级;
	 * nDuraOdds 耐力差 = 最大耐力-当前耐力 -1表示不检测 检测少于此耐力差的物品
	 * void * pUserItem:如果存在物品指针，不计算该指针的数量
	 * @return	返回匹配的物品总数量
	 */
	INT_PTR GetEquipCountByID(const INT_PTR wItemId, const INT_PTR nQuality = -1, const INT_PTR nStrong = -1,const INT_PTR nBind =-1,const INT_PTR nDuraOdds = -1, void * pNoItem = NULL);

	//当装备穿戴到身上的时候，做一些发数据包到客户端
	virtual void AfterEquipTakenOn(CActor * pOnwer, INT_PTR nPos,CUserItem *pUserItem,INT_PTR nLocation=0) {}

	//当装备从身上卸载的时候，做一些发数据包到客户端
	virtual void AfterEquipTakenOff(CActor * pOnwer,CUserItem *pUserItem) {}

	//装备被直接爆掉的时候提示玩家
	virtual void AfterEquipDirectRemove(CActor * pOnwer,CUserItem *pUserItem) {}

	virtual void SuitAttrCal(CAttrCalc &calc) {};
	//一件装备是否是基本的装备，基本的装备需要计算衍生属性
	inline bool IsBasicEquip(INT_PTR nItemPos) const
	{
		//return nItemPos >= itWeaponPos && nItemPos <= itIntellectBallPos;
		return nItemPos >= itWeaponPos && nItemPos <= itEquipDiamondPos;
	}
	
	 /*
	* Comments:获取玩家当前已经装备的装备的全身强化等级 按全身装备最低强化等级定位 没有穿戴装备的部位强化等级视为0
	* @Return INT_PTR: 最低装备的等级
	*/
	 INT_PTR GetMinStrongLevel() const;

	 //获取全套装备最低升星数
	 INT_PTR GetSuitMinStar() const;

	 //获取全套装备总升星数
	 int  GetTotalStar() const;

public:


	 /*
	 * Comments:获取装备的穿戴位置编号
	 * Param const btItemType: 物品类型 
	 * Param  btLocation: 左右位置 0左边 1右边
	 * @Return INT_PTR:返回-1表示物品不是装备
	 */
	static INT_PTR GetItemTakeOnPosition(const Item::ItemType btItemType, INT_PTR btLocation = 0);
	//装备昵称
	static LPSTR getEquipName(const CStdItem* pItem );
	//颜色描述
	static LPSTR getEquipNameDesc(const CStdItem* pItem );
	
	static LPSTR getItemColorDesc(const CStdItem* pItem );
	/* 
	* Comments: 获取单件装备的装备评分
	* Param  CUserItem * pUseItem:  装备
	* Param  CActor * pActor: 玩家指针(算部位强化属性)
	* @Return INT_PTR:  
	*/
	static float GetItemAttrSetScore( CUserItem* pUseItem,  CActor* pActor);
	
	//检查物品是否满足穿戴条件
	static INT_PTR CheckTakeOnCondition(const CEntity *pMonster, const CStdItem* pStdItem, const CUserItem *pUserItem);
	
	/*
	* Comments: 计算装备的属性
	* Param CAttrCalc & cal:属性计算器
	* Param CAttrCalc * pOriginalCal:原始属性计算器，不计算装备耐久折损的属性计算器
	* Param CAnimal *pMaster:主人的指针
	* @Return CAttrCalc:属性计算器
	*/ 
	CAttrCalc & CalcAttr(CAttrCalc &cal,CAttrCalc *pOriginalCal=NULL,CAnimal *pMaster=NULL,const HERODBDATA *pPetData = NULL);
	
	//计算一件装备的属性
	static void CalcOneEquipAttr(CAttrCalc &calc, CAttrCalc *pOriginalCal,  CUserItem* pUserItem, CAnimal* pMaster);

	//计算一件装备强化升星的属性
	static void CalcOneEquipStarAttr(CAttrCalc &calc, CAttrCalc *pOriginalCal,  CUserItem* pUserItem, 
		CAnimal* pMaster, bool bNeedAddProp);

	static void CalcOneEquipInscriptAttr(CAttrCalc &calc, CAttrCalc *pOriginalCal,  CUserItem* pUserItem, 
		CAnimal* pMaster, bool bNeedAddProp);

	/* 通过物品ID从装备表中查找物品
	 * wItemId	物品ID
	 * @return	返回第一个匹配的用户物品指针
	 */
	static CUserItem* FindItemByID( const INT_PTR wItemId,CUserItem** pUserItemVec,INT_PTR nItemCount);

	 /*
	 * Comments:判断一个宠物能否穿戴一件装备
	 * Param PETDBDATA * pData: 宠物的数据指针
	 * Param const CStdItem * pStdItem:物品的指针
	 * @Return INT_PTR:返回错误码
	 */
	static INT_PTR GetPetTakeItemErrorCode(const HERODBDATA*pData, const CStdItem* pStdItem, const CUserItem *pUserItem,CAnimal *pActor);

	//获取激活五行的条件，btpos为当前装备位置
	bool CanActivityFiveAttr(BYTE btPos);

	bool IsFiveAttrPos(BYTE btPos);

public:
	CUserItem*	m_Items[EquipmentCount];//装备物品表
	std::map<int, int> m_nSuitList;//
	//bool GetItemsByType(const Item::ItemType btItemType, CVector<CUserItem*>& out);
private:
	/*
	* Comments: 计算内功装备与内功宝石的共鸣属性
	* Param CAttrCalc & cal:属性计算器
	* Param CUserItem * pEquipment:内功装备的指针
	* Param CUserItem *pStone:内功宝石的指针
	*/ 
	void CalcResonanceAttr(CAttrCalc &cal,CUserItem *pEquipment,CUserItem *pStone);
};
