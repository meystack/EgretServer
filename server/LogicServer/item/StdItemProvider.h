#pragma once

#define HUNTCOUNT 5  //1-5为寻宝
#define HUNDRED_37W_FILE_INDEX 7 //37玩百服文件索引
#define TREASUREHUNTFILECOUNT 7 //TreasureHunt文件数量
#define HUNDREDCOUNT 1//百服连连看数
#define	HUNDRED_TYPE 5			//百服连连看类型
#define HUNTGRID 18 // 20130909 19个改成18个啦
#define HUNDREDGRID 5//百服连连看总物品数量
#define HUNDRED_REFRESH_COUNT 8//刷新那里固定显示数量

enum enFiveAttrType
{
	enFiveAttrErro	= 0,
	enFiveAttrGold	= 1,
	enFiveAttrWood	= 2,
	enFiveAttrWater	= 3,
	enFiveAttrFire	= 4,
	enFiveAttrEarth	= 5,
	enFiveAttrMax,
};
/**
/* 定义游戏中极品属性描述结构
*********************************/
#pragma pack(push,1)
typedef struct tagRandomAttValue
{
	GAMEATTRVALUE	minValue;	//属性值下限
	GAMEATTRVALUE	maxValue;	//属性值上限
	unsigned short prob;				//选择的几率，1表示1%
}RANDOMATTVALUE,*PRANDOMATTVALUE;

typedef struct tagRandomAttributeTerm
{
	unsigned char	type;		//属性类型，值为GAMEATTRTYPE的枚举成员，使用BYTE成员用于节省内存空间
	unsigned char	job;		//职业要求，0-全职业，1-战士，2-法师，3-道士
	unsigned short prob;				//选择的几率，1表示1%
	unsigned char	maxNum;		//最大数量，0-不限制
	unsigned int	lockMaxValue;	//锁定最大值，如果指定某属性，且当前属性大于等于该值，则属性值也不变化
	DataList<RANDOMATTVALUE> valueList; //属性列表
}RANDOMATTRTERM, *PRANDOMATTRTERM;
#pragma pack(pop)

typedef struct ComposeTable
{
	int nId;
	int nType;
	int nCount;
	ComposeTable()
	{
		memset(this, 0, sizeof(*this));
	}
}ComposeTableCfg;

typedef struct ItemCompose
{
	int nId;
	ComposeTableCfg composeItem;
	std::vector<ComposeTableCfg> table;
	int nLevelLimt;
	int nCircle;
	int nOpenServerDay;
	int nArea;//区域限制
	int nOfficeId;//
	int nVipLimit;//
	int nGhostLevelLimit;//神魔等级限制
	ItemCompose()
	{
		nId = 0;
		table.clear();
		nLevelLimt = 0;
		nCircle = 0;
		nOpenServerDay = 0;
		nArea = 0;
		nVipLimit = 0;
		nOfficeId = 0;
		nGhostLevelLimit = 0;
	}

}ItemComposeCfg;


typedef struct tagItemRecoverCfg
{
	int nId;
	std::vector<ComposeTable> cost;
	std::map<int, std::vector<ComposeTable> > nAwards;
	int nLevelLimt;
	int nCircle;
	int nOpenServerDay;
	int nRightLimt;
	int nTimesLimit;
	int nTimeLimit;
	tagItemRecoverCfg()
	{
		nId = 0;
		cost.clear();
		nAwards.clear();
		nLevelLimt = 0;
		nCircle = 0;
		nOpenServerDay = 0;
		nRightLimt = 0;
		nTimesLimit = 0;
		nTimeLimit = 0;
	}

}ItemRecoverCfg;



typedef struct ItemForge
{
	int nId;
	int nDropId;
	int nLevel;
	ItemForge()
	{
		memset(this, 0 , sizeof(*this));
	}

}ItemForgeCfg;
typedef std::vector<ItemForgeCfg> vItemForge;


typedef struct tagSuitAttr
{
	int nSuitId; //套装id
	int nSuitNum; //套装数量
	int nPercent;//加成百分比
	AttriGroup	attri;	//属性
	char name[1024];//
	tagSuitAttr()
	{
		memset(this,0, sizeof(*this));
	}
}SuitAttr;

//资源类型
typedef struct tagSourceConfig
{
	int nId; //套装id
	char name[1024];//
	tagSourceConfig()
	{
		memset(this,0, sizeof(*this));
	}
}SourceConfig;


//仓库
typedef struct tagWarehouseCfg
{
	int nOpenLevel = 0;
	int nOpenCardLv = 0;
	int nInitial = 0;
	std::vector<int> v_warehouses;
	tagWarehouseCfg()
	{
		v_warehouses.clear();
		nOpenLevel = 0;
		nOpenCardLv = 0;
		nInitial = 0;
	}
}WarehouseCfg;

class CStdItemProvider :
	protected CVector<CStdItem>,
	public CCustomLogicLuaConfig
	//public CFileReader<CStdItemProvider>
{
public:
	typedef CVector<CStdItem>		Inherited;
	typedef CCustomLogicLuaConfig	Inherited2;
	typedef CObjectAllocator<char>	CDataAllocator;
	//typedef CFileReader<CStdItemProvider>	InheritedReader;
	static LPCTSTR StdItemCacheFile;	//配置数据缓存文件路径

public:
	CStdItemProvider(); 
	~CStdItemProvider();

	//装备升星效果加成(属性)
	typedef struct tagStarDerive
	{
		WORD wStarCount;//需要多少星
		WORD wAppear;//外形，未用
		CStdItem::AttributeGroup attriGroup;
	}STARDERIVE,*PSTARDERIVE;


	
	//神炉衍生属性
	typedef struct tagGodStoveDerive
	{
		BYTE nLevel;						//全部分支等级
		CStdItem::AttributeGroup attriGroup;
	}GODSTOVEDERIVE,*PGODSTOVEGDERIVE;

	//转换为CStdItem数组的类型转换函数
	inline operator const CStdItem* () const
	{
		CStdItemProvider *pProvider = (CStdItemProvider*)this;
		return pProvider->Inherited::operator CStdItem*();
	}
	//通过物品ID获取物品配置对象
	inline const CStdItem* GetStdItem(const INT_PTR wItemId) const
	{
		//ID为0的物品也视为无效物品
		if ( wItemId <= 0 || wItemId >= Inherited::count() )
			return NULL;
		else return &(this->operator const CStdItem*()[wItemId]);
	}
	inline const char* GetNameById(const INT_PTR nItemId) const
	{
		const CStdItem* stdItem = GetStdItem(nItemId);
		return stdItem?stdItem->m_sName:"";
	}
	/* 
	* Comments: 通过物品的名字返回物品的数据指针
	* Param LPCTSTR sItemName: 物品名字
	* @Return const CStdItem*:物品的指针
	*/
	inline const CStdItem* GetStdItemByName(LPCTSTR sItemName) const
	{
		for(INT_PTR i=0 ;i <Inherited::count() ;i ++)
		{
			const CStdItem* pItem = GetStdItem(i);
			if( pItem && strcmp( pItem->m_sName,sItemName) ==0 )
			{
				return pItem;
			}
		}
		return NULL;
	}

	//统计产出物品
	void AddItemOutPutCount(INT_PTR wItemId,int nCount,const INT_PTR nLogIdent);

	//发送统计
	void SendItemOutPutToLog();

	//获取配置中的物品数量
	inline INT_PTR count() const{ return Inherited::count(); }
	//从文件加载标准物品配置	
	// @Param bBackLoad: 是否是后台加载
	// @Note: 增加此配置项是为了加快启动速度。在配置文件没有改动的情况下，启动服务器直接读取Cache数据，省去预处理和解析lua配置处理的操作耗时。
	bool LoadStdItems(LPCTSTR sFilePath, bool bBackLoad = false);

	//装载装备的精锻的数据
	bool LoadSmithData(LPCTSTR sFilePath);

		
	bool LoadForgeConfig(LPCTSTR sFilePath);
	bool LoadComposeConfig(LPCTSTR sFilePath);


	//加载套装
	bool LoadSuitItemConfig(LPCTSTR sFilePath);
	void ReadSuitItem();

	//仓库
	bool LoadWarehousemConfig(LPCTSTR sFilePath);
	void ReadWarehouse();

	//数值资源
	bool LoadNumericalConfig(LPCTSTR sFilePath);
	void ReadNumerical();

	SourceConfig* GetNumericalConfig(int nId)
	{
		auto it = m_SourceConfigs.find(nId);
		if(it != m_SourceConfigs.end())
		{
			return &(it->second);
		}
		return NULL;
	}

	/// 读取属性表 {{type = 1, value = 1}, {type = 2, value = 2}, {type = 3, value = 3}, }
	// 假如 reserve0Name= "job",则对应配置{type = 1, value = 1, job =1},
	bool LoadAttri(PGAMEATTR pAttr, LPCSTR reserve0Name = NULL);

	//装载装备的评分器
	bool LoadItemEval(LPCTSTR sFilePath)
	{
		/*return  InheritedReader::LoadFile(this,sFilePath,
			&CStdItemProvider::ReadItemEval);*/
		if( Inherited2::LoadFile(sFilePath) )
		{
			bool r= ReadItemEval();
			setScript(NULL);
			return r;
		}
		else
		{
			return false;
		}
	}
	bool LoadGodStoveConfig(LPCTSTR sFilePath);
	/*
	* Comments:通过物品的id获取精锻数据的指针
	* Param int nSmithId 对应哪个库(文件)Id
	* @Return DataList<RANDOMATTRTERM> *: 精锻的物品的指针
	*/
	DataList<RANDOMATTRTERM> * GetSmithData(int nSmithId);

	//获取极品属性 库编号列表
	DataList<int>* GetQualityIndexList(int nIndex);
	INT_PTR  GetQualityIndexListCount(){return m_qualityDataIndexList.count;}
	/*
	* Comments: 更新物品配置数据
	* @Return void:
	*/
	void UpdateItemConfig();
	
	//返回属性评价器
	CAttrEval * GetEval(INT_PTR nVocation=1)
	{
		if(nVocation >=1 && nVocation <= ArrayCount(m_evals))
		{
			return &m_evals[nVocation-1];
		}
		return NULL;
	}


	//装载装备的衍生的数据
	bool LoadEquipDeriveData(LPCTSTR sFilePath)
	{
	/*	return  InheritedReader::LoadFile(this,sFilePath,
		&CStdItemProvider::ReadEquipDerive);	*/
		if( Inherited2::LoadFile(sFilePath) )
		{
			bool r= ReadEquipDerive();
			setScript(NULL);
			return r;
		}
		else
		{
			return false;
		}
	}

	//获取装备升星衍生属性
	inline PSTARDERIVE GetStarDerive(INT_PTR wStarCount)
	{
		PSTARDERIVE pStar = NULL;
		//必须排序好的
		for(int i =0; i< m_starderive.count(); i++)
		{
			if (wStarCount >= m_starderive[i].wStarCount)
			{
				pStar = &m_starderive[i];
			}
			else
			{
				break;
			}
		}
		return pStar;
	}	

	//获取橙装衍生的属性
	inline CStdItem::AttributeGroup * GetOrangeAttr()
	{
		return m_orangeEquipAttr;
	}

	//获取衍生属性
	CStdItem::AttributeGroup* GetGodStoveDeriveAttrByLevel(INT_PTR nLevel);

	ItemComposeCfg* GetComposeCfgById(int nId)
	{
		std::map<int, ItemComposeCfg>::iterator it = m_composeList.find(nId);
		if( it != m_composeList.end())
		{
			return &(it->second);
		}
		return NULL;
	}

	ItemRecoverCfg* GetRecoverItemCfgById(int nId)
	{
		std::map<int, ItemRecoverCfg>::iterator it = m_recoverLists.find(nId);
		if( it != m_recoverLists.end())
		{
			return &(it->second);
		}
		return NULL;
	}
	std::map<int, ItemRecoverCfg>& GetRecoverItemCfgLists()
	{
		return m_recoverLists;
	}

		//套装配置
	SuitAttr* getSuitPtrDataByKey(int nSuitId, int nSuitNum);
	//套装是否存在
	bool SuitIsExists(int nKey);

protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:
	//读取标准物品数据配置完成后，将读取的数据保存到自身中
	void completeRead(CStdItem *pStdItems, const INT_PTR nItemCount, CDataAllocator &dataAllocator, bool bBackLoad = false);
	//从已装载的脚本中读取标准物品配置数据
	bool readStdItems(bool bBackLoad = false);
	//读取物品配置数据
	bool readItemData(CDataAllocator &dataAllocator, CStdItem &item);
	//读取物品属性表
	bool readItemAttributeTable(CDataAllocator &dataAllocator, CStdItem::AttributeGroup &AttrGroup);
	//读取物品联合物品表
	bool readUnitedItemTable(CDataAllocator &dataAllocator, CStdItem::UnitedAttributeGroup &unitedGroup);
	//读取物品标志属性
	bool readItemFlags(CStdItem &item);
	//读取物品使用条件表
	bool readItemConditionTable(CDataAllocator &dataAllocator, CStdItem::ItemUseCondTable &condTable);
	//读取物品预留配置表
	bool readItemReservesTable(CDataAllocator &dataAllocator, CStdItem::ItemReservesTable &table);

	//读取极品属性数据
	bool ReadSmithData(CDataAllocator &dataAllocator);
	//极品属性 库编号索引
	bool ReadQualitdataIndex(CDataAllocator& dataAllocator);

	//装载装备的评分的数据
	bool ReadItemEval();

	//装载装备的衍生数据
	bool ReadEquipDerive();

	//读取神炉配置
	bool ReadGodStoveConfig(CDataAllocator& dataAllocator);

	void ReadItemCompose();
	void ReadItemForge();
	//回收装备
	bool LoadRecoverItemConfig(LPCTSTR sFilePath);
	void ReadRecoverItemConfig();

private:
	//从缓存文件中读取标准物品配置数据，如果缓存数据有效且源数据文件未经修改且读取成功则返回true。
	bool readCacheData(DWORD dwSrcCRC32, bool bBackLoad = false);
	//将配置数据写入缓存文件中
	bool saveCacheData(DWORD dwSrcCRC32);

private:	
	CDataAllocator	m_DataAllocator;		//物品属性对象申请器

	// 用于后台加载数据
	CDataAllocator	m_DataAllocatorBack;	// 物品属性对象申请器(用于后台加载)	
	CVector<CStdItem> m_ItemBack;			// 后台加载的物品数据

	CDataAllocator m_DeriveDataAllocator;	//加载装备衍生配置申请器
	CDataAllocator m_SmithDataAllocator;
	CDataAllocator m_FiveAttrDataAllocator;
	CDataAllocator m_FiveAttrListDataAllocator;
	CDataAllocator m_EquipPosStrongAllocator;
	//CStdItem		*m_pStdItemsBack;		// 分配的静态物品对象数据内存（用于后台加载）
	INT_PTR			m_nItemCount;			// 分配的静态物品对象数量（用于后台加载）
	CCSLock			m_Lock;		
	

	//玩家的精锻的数据列表
	DataList< DataList<RANDOMATTRTERM> > m_smithData;
	DataList<DataList<int> >				m_qualityDataIndexList;//极品属性库编号列表
	CAttrEval m_evals[enMaxVocCount-1] ;    //各职业的装备评分器
	

	CStdItem::AttributeGroup m_orangeEquipAttr[CStdItem::MaxOrangeEquipCount];  //橙装的属性衍生加成配置

	CVector<STARDERIVE> m_starderive;	//升星的属性衍生加成配置
	CVector<GODSTOVEDERIVE>			m_godStoveDerive;	//神炉衍生属性配置 TO DELETE


	//合成
	std::map<int, ItemComposeCfg> m_composeList;
	//
	std::map<int, ItemRecoverCfg> m_recoverLists;
	//锻造
	vItemForge m_forgeList;
	CDataAllocator m_SuitAttrDataAllocator;
	//套装
	std::map<int, SuitAttr> m_SuitAttrs;//套装加成属性


	std::map<int, SourceConfig> m_SourceConfigs;//数值资源
public:
	//仓库 
	WarehouseCfg    m_WarehouseCfg;
public:
	int nForgeItemId = 0;      //锻造消耗道具id
	int nOnceForgeCost = 0;    //锻造一次消耗
	int nTenForgeCost = 0;    //锻造10次消耗
	int nBagten        = 0;    //锻造十次背包至少剩余格数
	int nBagone        = 0;    //锻造一次背包至少剩余格数
	int nEquipment     = 0;    //锻造功能装备背包栏至少剩余格数
	int  GetForgeDropIdByLevel(int nLevel)
	{
		int nDropid = 0;
		int nSize = m_forgeList.size();
		int nMaxlevel = 0;
		for(int size = 0; size < nSize; size++)
		{

			if(nLevel >= m_forgeList[size].nLevel && m_forgeList[size].nLevel >= nMaxlevel)
			{
				nMaxlevel = m_forgeList[size].nLevel;
				nDropid = m_forgeList[size].nDropId;
			}
		}
		return nDropid;
	}

	int  GetForgeIdByLevel(int nLevel)
	{
		int nId = 0;
		int nSize = m_forgeList.size();
		int nMaxlevel = 0;
		for(int size = 0; size < nSize; size++)
		{

			if(nLevel >= m_forgeList[size].nLevel && m_forgeList[size].nLevel >= nMaxlevel)
			{
				nMaxlevel = m_forgeList[size].nLevel;
				nId = m_forgeList[size].nId;
			}
		}
		return nId;
	}
};
