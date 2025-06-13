#pragma once

// 掉落物品信息
//tolua_begin
class DropItemInfo
{
public:
	unsigned char	btAwardType;
	unsigned short	wItemId;
	unsigned char	btQuality;
	unsigned char	btStrong;
	unsigned char	btBind;
	int  	btCount;
	unsigned char   btAuxParam; // 辅助参数
	unsigned int	nTime;		// 剩余存在时间，秒为单位
	unsigned short  nQualityDataIndex; //生成极品属性( >0才有效 )
};

// 掉落物品列表
class DropItemList
{
public:
	DropItemList() { ZeroMemory(this, sizeof(DropItemList));}

	inline void addItem(const DropItemInfo &dropItem)
	{
		for (INT_PTR i = 0; i < MAX_DROP_ITEM_COUNT; i++)
		{
			//修改了一个bug
			if (itemList[i].wItemId || itemList[i].btAwardType )
			{
				if (itemList[i].wItemId == dropItem.wItemId && dropItem.btAwardType == qatEquipment)
					return; // already exist
			}
			else 
			{
				itemList[i] = dropItem;
				itemCount++;
				return;
			}
		}
		return;
	}

	inline void addMoney(unsigned int nVal) { moneyCount += nVal; }

	inline int getCount() const { return itemCount; }

	inline DropItemInfo getItem(int index) const {		
		if (index >= 0 && index < (int)itemCount)
			return itemList[index];

		DropItemInfo info;
		info.btCount = 0;
		return info;
	}

	unsigned int		moneyCount;							// 金钱数目
	unsigned int		itemCount;							// 物品数目
	DropItemInfo		itemList[MAX_DROP_ITEM_COUNT];		// 物品列表
};
//tolua_end