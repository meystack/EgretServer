#pragma once
/***************************************************************/
/*
/* 拾取子系统
/* 玩家的是拾取的管理
/* 管理玩家身上的掉落包裹，处理客户端的各种数据包
/***************************************************************/
//class DROPBAG;

enum enLootSelectStyle
{
	lsLootStyleCancel =1, //放弃拾取
	lsLootStyleDemand =2, //需求
	lsLootStyleGreed =3,  //贪婪
};
class CLootSystem:
	public CEntitySubSystem<enLootSystemID,CLootSystem,CActor>
{
	
public:
	typedef CEntitySubSystem<enLootSystemID,CLootSystem,CActor> Inherid;

	//自由拾取的时间(毫秒数)，一个队伍物品，如果所属人没有拾取，那么剩下的时间将变为自由拾取状态
	static const unsigned int FreeLootTime =60000; //暂定为1分钟
	/*
	 *处理网络数据包
	 *nCmd ，分派到该系统里命令，从0开始
	 *packet，网络数据
	*/
	virtual  void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	/*
	* Comments: 玩家能否拾取背包里的金钱或者物品
	* Param CTeam * pTeam: 队伍的指针
	* Param CActor * pUser: 玩家的指针
	* Param CDropBag * pBag:背包的指针
	* Param unsigned int nBagID:背包的ID
	* @Return bool: 能够返回true，否则返回false
	*/
	static bool CanLootMoneyOrItem(CTeam * pTeam,CActor * pUser,CDropBag * pBag,unsigned int nBagID);
private:

	//拾取一个新的道具，新版本的
	bool LootDropItem(unsigned int nItemHandle);

	bool PetLootDropItem(unsigned int nItemHandle);
	
};

