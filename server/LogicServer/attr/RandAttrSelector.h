#pragma once

/************************************************************************/
/*
/*                            随机属性配置选择器
/*
/*   提供基本的随机属性选择功能。随机属性出现在物品、法宝、宠物、坐骑等方面的属性中随机属
/* 性与游戏属性类似，具有属性类型和属性值，不同的是随机属性的属性值具有两个，一个表示下限，
/* 一个表示上限。在依据随机属性项产生实际属性时产生的属性值被规定在随机属性值的下限和上限
/* 之间：
/*                  (maxValue >= 具体属性值 >= minValue)
/*   在依据随机属性项产生随机属性的期间，group（属性组）非0且值相同的属性，只会出现一次；
/* 而属性是否能够被选取的依据至少包含几率值大于等于属性项的prob（几率值）
/*
/************************************************************************/
class RandAttrSelector
{
public:

	/*
	* Comments: 为用户物品选择一个精锻随机属性
	!!!!!!!!和当前的精锻的算法冲突 不用了，但是保留
	* Param OUT PACKEDGAMEATTR & packedAttr: 用于保存所选择并产生值的属性对象
	* Param const CUserItem * pUserItem: 用户物品对象
	* Param int &nSelectIndex: 选中的那个属性的ID,比如0,1,2
	* @Return bool: ★返回值可能为false，请注意检查返回值！★
	*/
	static bool SelectItemSmithAttribute(OUT PACKEDGAMEATTR &packedAttr, const CUserItem* pUserItem,INT_PTR &nSelectIndex);


	/*
	* Comments:精锻一件装备
	* Param CUserItem * pUserItem: 装备的指针
	* Param bool bReserverOld:是否保护属性
	* @Return bool: 成功返回true,否则返回false
	*/
	static bool ItemSmithAttribute( CUserItem* pUserItem, int nSmithIdx, bool bReserverOld = false);

	/*
	* Comments:实际上生成极品属性
	* Param DataList<RANDOMATTRTERM> * pConfig:精锻的配置
	* Param PACKEDGAMEATTR * pAttr:结果的数据指针
	* Param nAttrTypeMust:只能刷出此属性，数值变化
	* @Return bool:成功返回true，否则返回false
	*/
	static bool RealSmithAttribute( DataList<RANDOMATTRTERM> * pConfig,PACKEDGAMEATTR* pAttr, unsigned char nAttrTypeMust=0,
		unsigned char nJob=0, PACKEDGAMEATTR* pAttrsAll=NULL );

	/*
	* Comments:初始化极品属性，useritem.quality!=0,smith未赋值
	* Param CUserItem * pUserItem:
	* Param nQualityDataIndex: 极品属性库编号
	* @Return bool:
	*/
	//static bool InitSmithByUserItem( CUserItem * pUserItem, int nQualityDataIndex);


	/* 
	* Comments: 随机一个极品属性
	* Param WORD wItemId:  物品id
	* Param BYTE btQuality: 品质
	* Param int nQualityDataIndex: 极品属性库编号索引
	* Param int smithArr[CUserItem::MaxSmithAttrCount]:  极品属性
	* @Return void:  
	*/
	//static void InitSmithById(WORD wItemId, BYTE btQuality, int nQualityDataIndex, int smithArr[CUserItem::MaxSmithAttrCount]);


public:		//[ZGame]新的从属性库中抽取属性的方法
	//从smith属性库中抽取鉴定属性->pUserItem->smithAttrs
	static void GenerateItemSmithAttrsInSmith( CUserItem* pUserItem, int nSmithId, int nAttrNum, int nLockTypes[], 
		unsigned char nJob=0 );

	//从极品库中随机抽取属性
	static void GenerateAttrsInSmith( int nSmithId, int nAttrNum, unsigned char nJob=0, unsigned char nAttrTypeMust=0  );
	//从极品库中随机抽取一条属性
	static void GenerateOneAttrInSmith( int nSmithId, unsigned char& nAttrType, int& nAttrValue, unsigned char nJob=0, 
		unsigned char nAttrTypeMust=0 );

	static void GenerateOneAttrInSmithLib( DataList<RANDOMATTRTERM>* pConfig, PACKEDGAMEATTR* pAttrsAll, PACKEDGAMEATTR* pAttr, 
		unsigned char nLockType=enSmithLockType_No, unsigned char nJob=0);

	static unsigned char HasAttrNumInAttrs( PACKEDGAMEATTR* pAttrsAll, unsigned char nAttrType );
};
