#pragma once
/*
*掉落包裹以及掉落物品的定义
*设定掉落包裹的一些数据定义以及简单的函数操作
*/

#define MAX_DROP_ITEM_COUNT 20 //最大的掉落物品的数量，使用一个最大值，齐整内存


//const unsigned int ITEM_OWNER_TEAM_ALL = 0;						 //队伍中的所有人都能够拾取
//const unsigned int ITEM_OWNER_IS_ROLLING  =unsigned int (-1);	 //物品的所有人正在筛子确认中
//const unsigned int ITEM_OWNER_IS_CAPTIN  =unsigned int (-2);	 //物品正在等待队长分配

/// 物品的拾取状态
enum tagItemLootStyle
{
	ITEM_OWNER_ALL_MEMBER		=0,	///< 所有人都能够拾取
	ITEM_OWNER_SPECIAL_ACTOR	=1, ///< 特定一个玩家,已经确定了所有者了
	ITEM_OWNER_WAIT_ROLL		=2, ///< 物品等等待Roll，还没Roll
	ITEM_OWNER_IS_WATING_CAPTIN	=3, ///< 正在等待队长分配
	ITEM_OWNER_UNDER_ROLLING	=4, ///< 物品正当Roll，等待Roll的结果
	ITEM_OWNER_INVALID			=5,	///<出现了错误
};

/// 掉落物品的数据结构
class  CDropItem : public Counter<CDropItem>
{
public:
	CDropItem()
	{
		memset(this,0,sizeof(*this));
	}


	static inline bool GetByteFlag(BYTE btValue,INT_PTR nIndexID)
	{
		if(nIndexID >5) return false; //0-4有效，因为只有一个byte存储
		return !!(btValue & (0x1 << nIndexID));
	}

	static inline bool SetByteFlag(BYTE & btValue,INT_PTR nIndexID,bool flag)
	{
		if(GetByteFlag(btValue,nIndexID) == flag) return false; // 无需改变
		if(flag)
		{
			btValue |= BYTE (1 << nIndexID);
		}
		else
		{
			BYTE btShiftValue = ~(1 << nIndexID);
			btValue &= btShiftValue;
		}
		return true;
	}

	/*
	* Comments:设置一个任务物品该玩家能否拾取的标记
	* Param INT_PTR nItemID:物品的ID
	* Param INT_PTR nPosID: 标记位的ID
	* Param bool flag: 值
	* @Return void:
	*/
	inline void SetItemOwnerFlag( INT_PTR nPosID,bool flag)
	{
		if(nPosID <0 || nPosID >=5) return; //0-7有效，因为只有一个byte存储
		if(item.wItemId )
		{
			SetByteFlag(btOWnerFlag,nPosID,flag);
		}
	}

	/*
	* Comments: 获取一个任务物品的归属标记
	* Param INT_PTR nItemID: 物品的id
	* Param INT_PTR nPosID: 位置，0-7有效，可以用来标记队伍里的玩家
	* @Return bool:如果这个位是0表示这个物品该玩家可以拾取
	*/
	inline bool GetItemOwnerFlag( INT_PTR nPosID )
	{
		if(nPosID <0 || nPosID >5) return true; //0-7有效，因为只有一个byte存储
		
		if(item.wItemId )
		{
			return  GetByteFlag(btOWnerFlag,nPosID);
		}
		return false;
	}

	/*
	* Comments:设置一个物品Roll的分数
	* Param INT_PTR nItemID:物品的ID
	* Param INT_PTR nPosID: 标记位的ID
	* Param bool btValue: 分数
	* @Return bool: 成功设置分数返回返回true，否则返回false
	*/
	inline bool SetItemRollScore(INT_PTR nPosID,BYTE btValue)
	{
		if(nPosID <0 || nPosID >=5) return false; //0-4有效，因为只有一个byte存储
		if( GetRollFlag(nPosID) ) return false; //已经投掷过骰子了
		scores[nPosID] = btValue;
		return true;
	}	
	
	/*
	* Comments: 获取一个物品的拾取状态
	* Param INT_PTR nState: 返回拾取状态
	* @Return void:
	*/
	inline void SetItemLootState(INT_PTR nState)
	{
		btLootType =(BYTE)nState;
	}

	/*
	* Comments: 获取一个物品的拾取状态
	* @Return INT_PTR:返回拾取状态
	*/
	inline INT_PTR GetItemLootState()
	{
		return btLootType;
	}

	//设置Roll状态
	inline bool SetRollFlag(INT_PTR nPosID,bool flag)
	{
		return SetByteFlag(btRollFlag,nPosID,flag);
	}

	//获取Roll状态
	inline bool GetRollFlag(INT_PTR nPosID)
	{
		return GetByteFlag(btRollFlag,nPosID);
	}

	/*
	* Comments: 获取一个物品的Roll的分数
	* Param INT_PTR nItemID: 物品的id
	* Param INT_PTR nPosID: 位置，0-7有效，可以用来标记队伍里的玩家
	* @Return bool:返回该物品的分数
	*/
	inline BYTE GetItemRollScore(INT_PTR nItemID, INT_PTR nPosID )
	{
		//0-7有效，因为只有一个byte存储
		if((nPosID >= 0) && (nPosID < 5) && item.wItemId)
		{
			return scores[nPosID];
		}
		return 0;
	}


public:
	CUserItemContainer::ItemOPParam  item;
	BYTE scores[5];			///< 队伍里的每一个人的分数
	BYTE btType;     //物品的类型,为了较少查找
	BYTE btLootType; //该物品的拾取类型
	BYTE btOWnerFlag;		///< 哪些人能够拾取的标记，能够拾取标记位1，取完标记为0
	BYTE btRollFlag;		///< 哪些人参加了Roll
	char btBroadcastType;	///< 是否需要广播
	bool btNeedTrggerEvent; ///< 是否需要触发脚本

	unsigned int nActorID ;  ///< 物品的所属者，这个是队伍分配的时候才用到
};


class CDropBag  : public Counter<CDropBag> //掉落包裹
{
public:
	 CDropBag()
	{
		memset(this,0,sizeof(CDropBag));
	}
	/*
	* Comments:设置背包物品的所有者
	* Param CUserItem * pItem: 物品的指针
	* Param unsigned int actorID: 玩家的actorID
	* @Return bool: 成功返回true,否则返回false
	*/
	inline bool SetItemOnwer(INT_PTR nItemID,unsigned int actorID)
	{
		INT_PTR nIndex = GetItemIndex(nItemID);
		if( (nIndex <0) || (nIndex >= MAX_DROP_ITEM_COUNT)) return false;
		dropItems[nIndex].nActorID = actorID;
		//dropItems[nIndex].btLootType = 
		return true;
	}

	/*
	* Comments: 设置所有物品的所有者
	* Param unsigned int actorID: 玩家的actorID
	* @Return void: 
	*/
	inline void SetAllItemOnwer(unsigned int actorID)
	{
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; i++)
		{
			if( dropItems[i].item.wItemId)
			{
				dropItems[i].nActorID =actorID;
			}
		}
	}

	inline bool SetItemLootStyle(INT_PTR nItemID,INT_PTR nLootStyle)
	{
		INT_PTR nIndex = GetItemIndex(nItemID);
		if( (nIndex <0) || (nIndex >= MAX_DROP_ITEM_COUNT)) return false;
		dropItems[nIndex].btLootType = (BYTE)nLootStyle;
		return true;
	}

	//设置所有物品的拾取方式
	inline void SetAllItemLootStype(INT_PTR nLootStyle)
	{
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; i++)
		{
			if( dropItems[i].item.wItemId)
			{
				dropItems[i].btLootType =(BYTE)nLootStyle;;
			}
		}
	}
	
	 //拾取了金钱，防止重复去取钱
	inline void LootMoney(INT_PTR & nCount)
	{
		nCount= nMoneyCount;
		nMoneyCount =0;
	}

	/*
	* Comments: 获取物品在背包里的索引
	* Param INT_PTR nItemID: 物品的id
	* @Return : 返回索引，如果没有返回-1
	*/
	inline INT_PTR GetItemIndex ( INT_PTR nItemID)
	{
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; i++)
		{
			if(dropItems[i].item.wItemId == nItemID  && dropItems[i].item.wItemId)
			{
				return i;
			}
		}
		return -1;
	}
	/*
	* Comments: 拾取一个物品
	* ParamINT_PTR nItemID: 物品的ID
	* Param unsigned int actorID:拾取者的actorID
	* Param INT_PTR nTeamIndex:拾取者在队伍里的位置，如果没有队伍，就是-1
	* @Return bool: 成功返回true
	*/
	inline bool LootItem( INT_PTR nItemID,unsigned int actorID,INT_PTR nTeamIndex)
	{
		INT_PTR nIndex = GetItemIndex(nItemID);
		if(( nIndex <0) || (nIndex >= MAX_DROP_ITEM_COUNT)) return false;
		
		//如果所有者一致或者这个物品没有设置归属，那么就能够拾取
		if( dropItems[nIndex].nActorID == actorID ||   //属于这个人的
			dropItems[nIndex].btLootType == ITEM_OWNER_ALL_MEMBER  ||  //如果大家都能拾取
			dropItems[nIndex].btType ==Item::itQuestItem  //任务物品都能够拾取
		   )
		{
			if(dropItems[nIndex].btType !=Item::itQuestItem ) //有的物品是能够多个人拾取的
			{
				dropItems[nIndex].item.wItemId =0; //删除这个物品
				return true;
			}

			if(nTeamIndex <0) //如果是一个人拾取的话就删除这个物品
			{
				dropItems[nIndex].item.wItemId =0; //删除这个物品
				return true;
			}

			//队伍拾取的话
			if(dropItems[nIndex].GetItemOwnerFlag(nTeamIndex))
			{
				dropItems[nIndex].SetItemOwnerFlag(nTeamIndex,false);
				if(dropItems[nIndex].btOWnerFlag ==0) //大家都拾取过了
				{
					dropItems[nIndex].item.wItemId =0; //删除这个物品
				}
				return true;
			}
		}

		return false;		
	}

	/*
	* Comments: 往背包里添加一个物品
	* Param CDropItem & item:物品的属性
	* @Return bool:成功返回true,否则返回false
	*/
	inline bool AddDropItem(CDropItem & item )
	{
		
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; i++)
		{
			if(dropItems[i].item.wItemId )
			{
				if( dropItems[i].item.wItemId == item.item.wItemId )
				{
					return false; //已经存在了
				}
			}
			else
			{
				dropItems[i] =item;
				return true;
			}
		}
		return false; //背包已经满了
	}
	
	/*
	* Comments: 背包是否是空的
	* @Return bool: 如没有金钱和物品了，那么返回true，否则返回false
	*/
	inline bool IsEmpty()
	{
		if(nMoneyCount >0) return false; //金钱还没拾取
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; i++)
		{
			if(dropItems[i].item.wItemId )
			{
				return false;
			}
		}
		return true;
	}

	//是否已经过期了
	inline bool IsExpired( TICKCOUNT nCurrentTickCount)
	{
		return nExpiredTime < nCurrentTickCount;
	}


	/*
	* Comments: 通过序列号获取背包物品的指针
	* Param CUserItem::ItemSeries series: 物品的序列号
	* @Return CUserItem *: 物品的指针
	*/
	CDropItem * GetDropBagItem(INT_PTR nItermID)
	{
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; ++i)
		{
			if(dropItems[i].item.wItemId && dropItems[i].item.wItemId  == nItermID )
			{
				return &dropItems[i];
			}
		}
		return NULL;
	}
	/*
	* Comments: 获取包裹里的物品数量
	* @Return INT_PTR:返回数量
	*/
	inline INT_PTR GetItemCount()
	{
		INT_PTR nCount =0;
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; ++i)
		{
			if(dropItems[i].item.wItemId  )
			{
				++nCount ;
			}
		}
		return nCount;
	}
	
	/*
	* Comments: 获取任务物品的数量
	* @Return INT_PTR:
	*/
	inline INT_PTR GetQuestItemCount()
	{
		INT_PTR nCount =0;
		for(INT_PTR i =0; i < MAX_DROP_ITEM_COUNT; ++i)
		{
			if(	dropItems[i].item.wItemId &&  dropItems[i].btType ==  Item::itQuestItem )
			{
				++nCount;
			}
		}
		return nCount;
	}
	
	/*
	* Comments:设置一个包裹队伍里的玩家能否拾取的标记
	* Param INT_PTR nIndexID: 玩家在队伍里的位置,[0-4]
	* Param bool flag: 值
	* @Return void:
	*/
	inline void SetBagOwnerFlag( INT_PTR nIndexID,bool flag)
	{
		CDropItem::SetByteFlag(btOwnerFlag,nIndexID,flag);
	}

	/*
	* Comments: 获取一个包裹的归属标记
	* Param INT_PTR nIndexID: 位置，0-4有效，可以用来标记队伍里的玩家
	* @Return bool:如果这个位是0表示这个物品该玩家可以拾取
	*/
	inline bool GetBagOwnerFlag( INT_PTR nIndexID )
	{
		return CDropItem::GetByteFlag(btOwnerFlag,nIndexID);
	}

	//获取一个背包的打开标记
	inline bool GetBagClientOpenState( INT_PTR nIndexID)
	{
		return CDropItem::GetByteFlag(btClientOpenState,nIndexID);
	}

	//设置一个背包的打开状态
	inline bool SetBagClientOpenState(INT_PTR nIndexID,bool flag)
	{
		return CDropItem::SetByteFlag(btClientOpenState,nIndexID,flag);
	}
	
	//设置背包客户端的闪光状态
	inline bool SetBagClientBlinkState(INT_PTR nIndexID,bool flag)
	{
		return CDropItem::SetByteFlag(btClientBlinkState,nIndexID,flag);
	}

	//获取客户端背包的闪光状态
	inline bool GetBagClientBlinkState(INT_PTR nIndexID)
	{
		return  CDropItem::GetByteFlag(btClientBlinkState,nIndexID);
	}

	unsigned int nTeamID;       //所属的队伍
	unsigned int nMoneyCount;   //金钱的数量
	WORD     nMonsterID ;       //掉落这个物品的怪物的ID
	BYTE     btOwnerFlag;        //哪些人拥有这个包裹
	BYTE btClientBlinkState;     //客户端物品的闪光状态
	BYTE btClientOpenState;      //客户端是否打开了这个包裹
	BYTE btReserver; //保留
	WORD wReserver; //保留

	int          nPosX;         //位置X
	int          nPosY;         //位置y
	int          nSceneID;      //场景ID
	unsigned int nKillerActorID; //杀死怪物的人的
	CDropItem  dropItems[MAX_DROP_ITEM_COUNT];  //掉落物品的指针列表 
	TICKCOUNT nExpiredTime;    //过期时间
};

