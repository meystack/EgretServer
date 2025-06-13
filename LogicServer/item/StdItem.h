#pragma once

/************************************************************************/
/*
/*                            游戏物品数据描述类
/*
/************************************************************************/

/** 装备死亡掉落拆分 **/
typedef struct ItemDeathSplit
{
	int nCount;		//条件数量
	int nType; 
	int nItemId;
};


/** 可选宝箱数据**/
typedef struct tagSelectItemInfo
{
	int nIndex ;//下标
	std::vector<ItemDeathSplit> items;
	int m_ndropGroupid; //掉落id
	tagSelectItemInfo()
	{
		memset(this, 0, sizeof(*this));
	}
}SelectItemInfo;

struct CUserItem;
class CAnimal;
class CStdItem : public Counter<CStdItem>
{
public:
	static const INT_PTR MaxItemQuality = 7;	  //物品品质等级上限，包括品质0
	static const INT_PTR MaxItemStrong = 21;	  //物品强化等级上限，包括强化等级0
	static const INT_PTR MaxItemHole = 4;		  //物品镶嵌槽数量上限
	static const INT_PTR MaxOrangeEquipCount =11; //最大的橙装的数目
	
	//目前游戏中用到的物品的品质的定义
	enum tagItemQuality
	{
		iqItemWhite =0,   //白装
		iqItemGreen  =1,  //绿装
		iqItemBlue  =2,   //蓝装
		iqItemPurple =3,  //紫装
		iqItemRed=4,      //红装
		iqItemOrange = 5,  //橙装
		
	};
	enum tagItemSource
	{
		iqKillMonster = 1, //杀怪
		iqNPCAward    = 2, //夺标
		iqOther       = 3, //其他
	};


	/* 定义属性组数据结构，属性组将用于描述物品一个品质或强化等级的属性表 */
	typedef struct AttributeGroup
	{
		INT_PTR		nCount;	//属性数量
		PGAMEATTR	pAttrs;	//属性表
	};

	/* 定义物品联合属性
	 * ★联合属性指一个装备的某些属性仅在角色穿戴有其他装备的时候才出现的属性
	 * ★这些具有联合关系的物品称为联合物品
	 */
	typedef struct UnitedAttributeGroup :
		public AttributeGroup
	{
		INT_PTR		nUnitedItemCount;	//联合物品数量
		PWORD		pUnitedItemIds;		//联合物品ID表
	};
		
	/* 定义联合属性表 */
	typedef struct UnitedAttributeTable
	{
		INT_PTR				 nCount;	//联合属性数量
		UnitedAttributeGroup *pGroups;//联合属性表	
	};
		

	typedef struct tagExistScenes
	{
		INT_PTR				nCount;		// 可存在场景数量
		int					*pSceneId;	// 场景ID
	}ExistScenes;

	/** 定义物品的标志属性结构 **/
	typedef struct ItemFlags
	{
	public:
		union 
		{
			Uint64 dwFlag;	//标志组1
			
			struct
			{
				bool recordLog:1;      //是否记录流通日志
				bool denyStorage:1;     //是否禁止存角色仓库
				bool denyGuildDepot:1;     //是否禁止存行会仓库
				bool autoBindOnTake:1;  //是否在穿戴后自动绑定
				bool autoStartTime:1;   //是否在获得时即开始计算时间，如果不具有此标志则将在装备第一次被穿戴的时候开始计时
				bool denyDeal:1;     //是否禁止交易
				bool denySell:1;		//是否禁止出售到商店
				bool denyDestroy:1;		//是否禁止销毁
				bool destroyOnOffline:1;//是否在角色下线时自动消失

				bool destroyOnDie:1;	//是否在角色死亡时自动消失
				bool denyDropdown:1; //是否禁止在死亡时爆出
				bool dieDropdown:1;		//是否在角色死亡时强制爆出
				bool offlineDropdown:1; //是否在角色下线时强制爆出 
				bool asQuestItem:1;		 //是否作为任务需求物品使用
				bool showLootTips:1;    //是否显示掉落提示
				bool denyBuffOverlay:1;  //buff物品，时间不叠加的配置true
				bool matchAllSuit:1;        // 匹配所有的套装，用于稀有神器，穿上激活所有套装该部位的属性
				bool canMoveKb:1;        // 可拖至快捷栏



				// bool hideDura:1;          //隐藏耐久
				// bool denySplite:1;        //是否禁止在物品叠加后进行拆分（通常用于带有实现限制的物品）
				
				// bool monAlwaysDropdown:1;//是否在怪物死亡爆出时不检查杀怪者等级差而均掉落
				// bool hideQualityName:1;  //隐藏品质颜色
				// bool inlayable:1;		//是否可打孔（打孔后才能镶嵌）
				// bool denyTipsAutoLine:1;  //拒绝换行

				// bool denyDropDua:1;		//死亡或攻击等时禁止扣除耐久
				// bool denyRepair:1;		//装备禁止修理
				// bool canDig:1;         //能够挖矿
				// bool fullDel:1;         //buff药，满了buff要消失，同时满了，是无法添加上buff

				// bool skillRemoveItem:1;		//标记是否能通过技能扣除物品
				// bool denyHeroUse:1;			//禁止英雄使用

				// bool notConsumeForCircleForge:1;	//转生锻造时不需要副装备
				// bool notShowAppear:1;		//不显示外观(用于装备)
				// bool boDelete:1;			//是否废弃,建议在配置文件注释掉，不要加进来占内存
				// bool showdura:1;    //功能物品使用dura计算次数(例如:回城卷)
				// bool bMeltingFlag:1;	//是否熔炼装备
				// bool bCanIdentify:1;     //是否可以被鉴定，true-可以鉴定，false-不能鉴定
			};
		};
		ItemFlags()
		{
			dwFlag =0;
		}
	};

	/** 定义物品的使用条件结构 */
	typedef struct ItemUseCondition
	{
		/** 定义物品的使用条件类型 **/
		enum UseCondition
		{
			ucLevel				= 1,	//等级必须大于等于value级 如果为魂石则代表效果激活的装备等级
			ucGender			= 2,	//性别编号必须等于value，0男1女
			ucJob				= 3,	//职业编号必须等于value
			ucSocialMask		= 4,	//沙巴克城主
			ucBattlePower		= 5,	//玩家的战力必须大于等于value
			ucMinCircle			= 6,	//大于等于这个转生才能使用
			ucMaxCircle			= 7,	//小于等于这个转生才能使用
			ucVipLevel			= 8,	//vip等级
			ucInternalWorkLevel = 9,	//内功等级必须大于等于value级
			ucGuildLevel 		= 10,	//行会等级必须大于等于value级
		};
		BYTE			btCond;	//条件类型
		INT				nValue;	//条件值
	};

	/** 物品使用条件表 **/
	typedef struct ItemUseCondTable
	{
		INT_PTR				nCount;		//条件数量
		ItemUseCondition*	pConds;		//条件表
	};

	/** 定义物品的保留配置结构 */
	typedef struct ItemReserve
	{
		int			nValue1;	//配置值1
		int			nValue2;	//配置值2
	};

	/** 物品保留配置结构表 **/
	typedef struct ItemReservesTable
	{
		INT_PTR				nCount;		//配置数量
		ItemReserve*		pConfigs;	//配置数据
	};

public:
	/* 计算物品属性 
	 * calc		用于计算物品属性的结果容器
	 * nQuality	物品品质等级
	 * nStrong	物品强化等级
	*/
	CAttrCalc& CalcAttributes(CAttrCalc &calc, const INT_PTR nQuality, const INT_PTR nStrong, const CUserItem* pUserItem = NULL, CAnimal* pMaster = NULL) const ;


	/* 判断物品是否是基本装备 */
	inline bool isBaseEquipment() const { return m_btType > Item::itUndefinedType && m_btType < Item::itEquipMax; }

	/* 判断物品是否是装备 */
	inline bool isEquipment() const { return m_btType > Item::itUndefinedType && m_btType < Item::itEquipMax; }

	inline bool isGodStoveEquipment() const{ return false; }//m_btType >= Item::itBloodSoul && m_btType <= Item::itIntellectBall; }

	/// 衣橱物品	
	inline bool isAlmirahItem() const
	{ 
		/*switch(m_btType)
		{
		default:
			return false;
		}*/
		return false;
	}
	
	/*
	* Comments: 物品是否含有区域相关属性（是否配置了离开某个区域要删除）。
	* @Return bool:
	*/
	inline bool hasSceneProp() const
	{
		for (INT_PTR i = 0; i < m_existScenes.nCount; i++)
		{
			if (m_existScenes.pSceneId[i] != -1)
				return true;
		}

		return false;
	}

	/*
	* Comments: 查询是否应该离开指定的场景要删除物品
	* Param int nSceneId: 场景Id
	* @Return bool:
	*/
	inline bool needDelWhenExitScene(int nSceneId) const
	{
		for (INT_PTR i = 0; i < m_existScenes.nCount; i++)
		{
			if (m_existScenes.pSceneId[i] == -1 || m_existScenes.pSceneId[i] == nSceneId)
				return false;
		}

		return true;
	}

	/*
	* Comments: 初始化一件物品
	* Param CUserItem * pUserItem:物品的指针
	* @Return void:
	*/
	static void AssignInstance(CUserItem *pUserItem,const CStdItem * pItem); 

	inline INT_PTR GetUseConditionValue(INT_PTR btCond) const
	{
		const ItemUseCondition *pCond = m_Conditions.pConds;
		for (INT_PTR i = m_Conditions.nCount-1; i > -1; --i)
		{
			if (pCond->btCond == (BYTE)btCond)
			{
				return pCond->nValue;
			}
			pCond++;
		}
		return 0;
	}

	inline const ItemReservesTable *GetItemReservesTable() const
	{
		return &m_Reserves;
	}

	inline bool isMeltingItem() const
	{
		//return m_btType > Item::itUndefinedType && m_btType <= Item::itSpecialRing && m_btType != Item::itDecoration;
		return m_btType > Item::itUndefinedType && m_btType <= Item::itEquipDiamond && m_btType != Item::itDecoration;
	}

public:
	INT						m_nIndex;			//物品唯一ID
	CHAR		 			m_sName[48];		//物品名称
	BYTE		            m_btType;			//物品类型，值为StdItem::ItemType的枚举类型
	WORD		            m_wIcon;			//物品图标编号
	WORD		            m_wShape;			//物品外形编号，只对具有外形的装备类物品有意义，例如衣服、武器和坐骑、翅膀
	WORD		            m_wBack;			//物品背饰编号，只对具有外形的装备类物品有意义，例如衣服、武器和坐骑、翅膀
	WORD		            m_wDupCount;		//物品允许叠加的数量
	BYTE		            m_btDealType;		//物品在商店中交易的货币类型，类型为eMoneyType
	INT			            m_nPrice;			//物品在商店中购买的价格，购买物品的货币类型通过成员m_btDealType的值限定
	CMiniDateTime			m_UseTime;		    //使用时限
	BYTE					m_btColGroup;		//物品冷却组，对于使用后具有冷却时间的物品来说，相同冷却组的物品将同时进入冷却恢复状态
	int						m_nRecoverId;		//物品的分解的配置ID
	WORD                    m_wSuitID;          //套装的ID(龙珠等级)
	WORD                    m_wResonanceId;     //共鸣ID
	int			            m_nCDTime;			//冷却时间，单位是毫秒，如没有冷却可以不填写
	int						m_nDropBroadcast;	//爆出是否广播，-1：一直广播，0：不广播（默认），大于0表示开服后多少天内才广播。
	BYTE        			b_showQuality;      //显示的品质
	ExistScenes				m_existScenes;	    // 物品可存在的场景列表集合

	BYTE				    m_btBatchType;		//批量使用的类型
	int						m_nBatchValue;		//批量使用的值
	AttributeGroup			m_StaticAttrs;		//物品静态属性表
	ItemUseCondTable		m_Conditions;	    //物品使用条件表
	ItemFlags				m_Flags;		    //物品标志属性
	int                     m_nPackageType;     //包裹类别 -- 0为装备，1为道具，2为材料
	int                     m_ndropGroupid;     //掉落组id
	int                     m_nsuggVocation;    //推荐职业

	int			            m_nOpenDaylimit;	//限制时间，单位是秒，相对开服时间
	
	//UINT		m_dwDura;			//物品的耐久度，1000点在客户端中显示为1
	//UINT		m_dwUseDurDrop;		//每次使用扣除多少点的耐久(物品使用次数)
	
	
	//BYTE		m_btSmithId;		//物品精锻ID，0表示不可精锻
	

	//int			m_nValidFbId;		//物品有效的副本ID。物品离开此副本自动删除。-1表示任何副本都不会删除
	//int			m_nValidSceneId;	//物品有效的场景ID。物品离开此副本自动删除。-1表示任何场景都不会删除

	//WORD        w_candidateIconCount;  //候选的图标的数目，用于骰子
	
	//BYTE		b_specRing;			//特殊戒指类型


	
	AttributeGroup			m_QualityAttrs[MaxItemQuality];	//物品品质属性表
	AttributeGroup			m_StrongAttrs[MaxItemStrong];	//物品强化属性表
	//BYTE					m_StrongCount;	//可强化次数，对应m_StrongAttrs数


	ItemReservesTable		m_Reserves;		//预留配置

	int						m_DropCount;		//物品产出的个数
	
	int                     m_nSillId;  //技能ID
	int                     m_nItemlvl;  //回收等级
	
	std::vector<ItemDeathSplit> m_nDeathsplit; //物品装备时死亡掉落拆分
	//BYTE					m_nConsignType;		//寄卖类型
	int                     nRecycling;  //是否一键回收
	int                     nItemlevel; //装备等级
	int                     nJpdrop; //极品
	int                     nTips;//

	//新增物品使用类型
	BYTE		            m_CanUseType;	//物品使用类型新增，默认无此类型
	BYTE		            m_CanUseCount;	//物品使用次数
	

	std::vector<SelectItemInfo> m_SelectItems;//
};

