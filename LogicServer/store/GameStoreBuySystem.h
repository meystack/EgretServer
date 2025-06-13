#pragma once

/************************************************************************/
/*
/*                            购买商城物品子系统
/*
/************************************************************************/

class CGameStoreBuySystem :
	public CEntitySubSystem<enStoreSystemID, CGameStoreBuySystem, CActor>
{
public:
	typedef CEntitySubSystem<enStoreSystemID, CGameStoreBuySystem, CActor> Inherited;
	
	enum eBuyItemOpt
	{
		ebioUseSelf = 1,		//自己用
		ebioGiveFrd = 2,		//赠送给好友
	};

public:
	CGameStoreBuySystem()
	{
		//m_accountYuanbao = 0;
	}
	/*
	 *处理网络数据包
	 *nCmd ，分派到该系统里命令，从0开始
	 *packet，网络数据
	*/
	virtual  void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet);

	//virtual	VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );
	virtual void OnEnterGame();
	//商城列表信息
	void SendShopLists(CDataPacketReader& packet);
	//购买商品
	void BuyShopItem(CDataPacketReader& packet);
	
	void ShowNpcShop(BYTE nType, BYTE ntab);
	
// 	/*
// 	* Comments: 同步元宝数量到客户端
// 	* Param unsigned int count:
// 	* @Return void:
// 	*/
// 	void SyncClientYuanBaoCount(unsigned int count);

// 	/*
// 	* Comments:查询元宝数量
// 	* @Return void:
// 	*/
// 	void ClientQueryYuanBaoCount();
	
// 	/*
// 	* Comments:提取元宝
// 	* Param unsigned int count:
// 	* @Return void:
// 	*/
// 	void ClientWithdrawYuanBaoCount(unsigned int count);	


// 	/*
// 	* Comments: 接收到AM消息
// 	* Param const CAMClient::AMOPData & data:
// 	* @Return void:
// 	*/
// 	void OnAcceptAMMsg(const CAMClient::AMOPData &data);

// 	//广播一件商品的数目发生改变
// 	static void BroadMerchanCountChange(int nMerchanID,int nCount,int nLabelId);

// 	//下发玩家购买商品的数量
// 	void SendActorMerchanCount();
	
// 	//广播刷新了一个分类的数据
// 	static void BroadLabelRefresh(int nLabelId);

// 	//更新个人今天消费元宝到排行版 nValue 本次消费的元宝数
// 	void AddConsumeToRank(int nValue);

// 	//下发领取奖励的提示
// 	void SendGetConsumeAwardsTips();

// 	/*
// 	* Comments:赠送商城物品
// 	* Param CDataPacketReader & packet:数据包
// 	* @Return void:
// 	*/
// 	void GiveStoreItem(CDataPacketReader &packet);


// 	/*
// 	* Comments:查询玩家是否存在的接口
// 	* Param const char * sName:
// 	* Param int nId:
// 	* Param int nCount:
// 	* @Return void:
// 	*/
// 	void DoQueryActorExists(const char * sName, int nId = 0, int nCount=0);
// private:
// 	//客户端购买商城物品
// 	void ClientBuyStoreItem(CDataPacketReader &packet);

// 	/*
// 	* Comments:处理购买商城物品
// 	* Param BYTE nOptType:操作类型，自己购买，赠送
// 	* Param int nId:商城id
// 	* Param int nBuyCount:数量
// 	* Param CActor * pAcceptPlayer:如果是赠送，接受赠送玩家指针
// 	* Param BYTE bUse:是否立即使用
// 	* @Return bool:处理成功返回true
// 	*/
// 	bool OnBuyStoreItem(BYTE nOptType, int nId, int nBuyCount, unsigned int nAcceptId = 0, const char * sAccepName = NULL, BYTE bUse = 0);

	
// 	//检查商品是否允许购买
// 	bool CheckMysticalShopBuy(int nId, int nBuyCount);

// 	//神秘商店商品购买成功
// 	void OnMysticalShopBuy(int nId, int nItemId, int nBuyCount, int nItemCount, int nMoneyType, int nMoneyNum);

// 	//客户端查询商城销量排行
// 	void ClientQueryStoreSaleRank();	
	
// 	//在购买的时候是否公告
// 	bool IsBroadCastBuy();



// 	//改变全服播报的状态
// 	void ClientChangeBroadcastFlag(CDataPacketReader &packet);	

// 	//获取动态商城的数据
// 	void ClientGetDynamicStoreData(CDataPacketReader &packet);

// 	//返回元宝消耗排行
// 	void SendYbConsumeRanking(CDataPacketReader &packet);

// 	//一键购买
// 	void OneKeyBuyItems(CDataPacketReader & packet);

// 	/*
// 	* Comments:处理购买动态添加物品
// 	* Param CGameStore::PMERCHANDISE pMerchandise:商城物品对象指针
// 	* Param int nId:id
// 	* Param int nBuyCount:数量
// 	* @Return bool:成功返回true
// 	*/
// 	bool IsBuyDynaAddItem( CGameStore::PMERCHANDISE pMerchandise, int nId, int nBuyCount );

// 	//获取玩家已购买某个限购商品的数量
// 	int   BuySingleDynaItemCount(int  nId);
	
// 	/*
// 	* Comments:查询是否存在该用户名
// 	* Param CDataPacketReader & reader:
// 	* @Return void:
// 	*/
// 	void OnQueryActorExists(CDataPacketReader &reader);
// private:
// 	int	  m_accountYuanbao;//账号里可提取元宝(充账号的平台才有)
};
