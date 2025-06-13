#pragma once
#include<string>
//禁止编译器对此结构进行字节对齐
#pragma pack (push, 1)

//这里表示物品的存储位置，与db通信用的
enum tagItemStorageType
{
	itItemBag=1,    		//背包物品
	itItemEquip =2, 		//装备物品
	itItemDepot =3, 		//仓库物品
	itItem4 = 4,
	itItemHero =5,    		//玩家的宠物的物品
	itItem6 = 6,
	itItemOfflineEquip =7, 	//装载玩家的离线装备数据	
	itItem8 = 8,
	itItem9 = 9,
};


#define  SMITH_COUNT_BYTE_ID  0    //精锻度保存的那个reserver
#define  DEPORT_BAG_ID_BYTE_ID 5   //物品reserver数据里存储的仓库背包的id的那个字节

#define  MAX_WING_ITEM_NUM 4	 //翅膀吞噬材料最大的种类	

struct CUserItem
{
public:
	static const INT_PTR MaxSmithAttrCount = 5;	//物品的精锻属性数量

	/** 物品系列号数据类型定义
	** ★★★千万不要给这个联合结构增加构造函数！否则巨慢★★★
	**/
	CUserItem()
	{
		memset(this, 0, sizeof(*this));
	}
	
	union ItemSeries
	{
		LONG64	llId;
	
		struct  
		{
			unsigned int time;	//31-0位表示物品产生的日期和时间，值为短日期类型，单位是秒
			WORD	wSeries;	//47-32位表示在一秒内的序列，每产生一个物品则序列值+1，一秒后序列值归零
			WORD	btServer;	//55-48位表示服务器ID
		}t;
	public:
		//转换为LONG64的类型转换运算符
		inline operator LONG64 () const
		{
			return llId;
		}
		//提供对物品系列号是否相等的运算符重载
		inline bool operator == (const ItemSeries& series) const
		{
			return llId == series.llId;
		}
		//提供对物品系列号是否不等的运算符重载
		inline bool operator != (const ItemSeries& series) const
		{
			return llId != series.llId;
		}	
	};
	

	/**
	** 物品镶嵌槽数据结构定义
	**/
	struct ItemInlayHole
	{
	public:
		static const WORD HoleOpenFlag = 0x8000;//镶嵌槽是否已经开启的标志
		static const WORD HoleItemMask = 0x7FFF;//镶嵌槽镶嵌的物品id的掩码位
	public:
		WORD wData;
	public:
		//判断镶嵌槽是否已经开启
		inline bool opened() const
		{
			return (wData & HoleOpenFlag) != 0;
		}
		//开启或关闭镶嵌孔，注意：★★★关闭镶嵌孔后之前镶嵌的物品ID会继续保留
		inline void open(const bool boOpened)
		{
			if ( boOpened )
				wData = wData | HoleOpenFlag;
			else wData = wData & HoleItemMask;
		}
		//获取镶嵌槽中镶嵌的物品的ID
		inline WORD getInlayedItemId() const
		{
			return (wData & HoleItemMask);
		}
		//设置镶嵌槽中镶嵌的物品的ID
		inline ItemInlayHole& setInlayedItem(const INT_PTR wItemId)
		{
			wData = (wData & HoleOpenFlag) | (wItemId & HoleItemMask);
			return *this;
		}
		
	};

public:
	//判断物品是否已经被绑定或者不可交易
	inline bool binded() const { return btFlag != ufUnBind; }
	//是否禁止交易
	inline bool IsDenyDeal() const { return btFlag  == ufDenyDeal; }
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

	void setSource(CUserItem* pUserItem)
	{
		if(pUserItem) {
			memset(&this->cSourceName, 0, sizeof(this->cSourceName));
			memcpy(&this->cSourceName, pUserItem->cSourceName, sizeof(this->cSourceName));
			this->nAreaId = pUserItem->nAreaId;
			this->nCreatetime = pUserItem->nCreatetime;
			this->bInSourceType =pUserItem->bInSourceType;// tagItemSource::iqKillMonster;
			this->nDropMonsterId = pUserItem->nDropMonsterId;
		}
	}
	void operator >> (CDataPacket& pack)
	{
		pack<< this->series.llId;
		pack<< this->wItemId;
		pack<< this->btQuality;
		pack<< this->btStrong;
		pack<< this->wCount;
		pack<< this->bLostStar;
		pack<< this->bInSourceType;
		pack<< this->wIdentifySlotNum;
		pack<< this->wStar;
		pack<< this->nCreatetime;
		for (int i = 0; i < MaxSmithAttrCount; i++)
		{
			pack<< (unsigned int)this->smithAttrs[i].nValue;
		}
		pack<< this->nAreaId;
		pack<< this->btFlag;
		pack<< this->btLuck;
		pack<< this->nDropMonsterId;
		pack<< this->btDeportId;
		pack<< this->btHandPos;
		pack<< (BYTE)1;//this->btSharp;/*  */
		pack<< this->wPackageType;
		pack.writeString((const char*)cBestAttr);
		pack.writeString((const char*)Refining);
		pack.writeString((const char*)cSourceName);
	}

	void operator << (CDataPacketReader& pack)
	{
		pack >>this->series.llId;
		pack >> this->wItemId;
		pack >> this->btQuality;
		pack >> this->btStrong;
		pack >> this->wCount;
		pack >> this->bLostStar;
		pack >> this->bInSourceType;
		pack >> this->wIdentifySlotNum;
		pack >> this->wStar;
		pack >> this->nCreatetime;
		for (int i = 0; i < MaxSmithAttrCount; i++)
		{
			pack >> this->smithAttrs[i].nValue;
		}
		
		//pack<< this->smithAttrs;
		pack >> this->nAreaId;
		pack >> this->btFlag;
		pack >>this->btLuck;
		pack >> this->nDropMonsterId;
		pack >> this->btDeportId;
		pack >> this->btHandPos;
		pack >> this->btSharp;//this->btSharp;/*  */
		pack >> this->wPackageType;
		pack.readString(cBestAttr, ArrayCount(cBestAttr));
		cBestAttr[sizeof(cBestAttr)-1] = 0;
		pack.readString(Refining, ArrayCount(Refining));
		Refining[sizeof(Refining)-1] = 0;
		pack.readString(cSourceName, ArrayCount(cSourceName));
		cSourceName[sizeof(cSourceName)-1] = 0;
	}
public:
	ItemSeries series;			//物品唯一系列号
	WORD	wItemId;			//对应的标准物品ID
	BYTE	btQuality;			//物品的品质等级
	unsigned int    wCount;		//物品数量
	WORD	wStar;				//强化星级
	BYTE	btFlag;				//绑定标记 0不绑定，1绑定，2不可交易(见UserItemFlag)
	WORD    wPackageType;   //包裹类别
	char    cBestAttr[200];  //极品属性
	char    Refining[200];  //洗炼属性

	BYTE	btStrong;			//物品的强化等级（ZGame保留不使用）--暂时没用
	BYTE	bLostStar;			//[ZGame使用]当前强化损失的星级（强化） --暂时没用
	WORD    wIdentifySlotNum;	//[ZGame使用]鉴定属性槽被解锁的数量 --暂时没用
	PACKEDGAMEATTR	smithAttrs[MaxSmithAttrCount];	//鉴定生成属性的值 --暂时没用
	char    btLuck;				//动态的幸运值或者诅咒值,祝福油加幸运，杀人减幸运 --暂时没用
	BYTE    btDeportId;			//装备穿戴的位置 --暂时没用
	BYTE    btHandPos;			//是左右还是右手 --暂时没用
	BYTE	btSharp;			//锋利值 --暂时没用

	//---来源
	BYTE	bInSourceType;		//来源
	WORD	nDropMonsterId;			//怪物id
	int		nAreaId;			//场景id
	char    cSourceName[200];  //归属昵称
	CMiniDateTime	nCreatetime;		//物品的创建时间，类型为CMiniDateTime

	int     nType ;//
//整个useritem的大小为53个字节。
};


/// 衣橱物品

struct CAlmirahItem
{
	WORD nModelId;
	byte btTakeOn;								//是否已经穿上
	int  nLv;								//等级
	CAlmirahItem()
	{
		memset(this, 0, sizeof(*this));
	}	
};

#pragma pack(pop)
