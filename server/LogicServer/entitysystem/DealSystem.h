#pragma once

typedef struct DealActorCd
{
	unsigned int nActorId = 0;
	uint32_t   nCd = 0;
};

class CDealSystem :
	public CEntitySubSystem<enDealSystemID, CDealSystem, CActor>
{
public:
	typedef CEntitySubSystem<enDealSystemID, CDealSystem, CActor> Inherited;
	//单次交易物品数量限制
	static const INT_PTR MaxDealItemCount = 5;

public:
	CDealSystem();

	//判断是否正在交易
	inline bool IsDealing(){ return m_boDealing; }
	//获取交易放入的钱币数量
	inline INT_PTR GetCoinCount(){ return m_nDealCoin; }
	//获取交易放入的物品数量
	inline INT_PTR GetItemCount(){ return m_nItemCount; }
	//判断交易是否已经被锁定
	inline bool IsLocked(){ return m_boDealing && m_boDealLocked; }
	//取消交易
#pragma __CPMSG__(★ATTENTION:注意，每当角色离线时以及周期性例行保存数据前，必须判断角色是否正在交易，如果正在交易则必须取消交易)
	void CancelDeal(bool boComplete = false);
public:
	/*** 覆盖父类的函数集 ***/
	//清空内存
	virtual void  Destroy();			

	/*处理网络数据
	 * nCmd 消息号
	 * packet 网络包
	 */
	virtual void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet);

private:
	//客户端发起交易
	void ClientRequestDeal(CDataPacketReader &packet);

	//客户端回应交易请求
	void ClientReplyDeal(CDataPacketReader &packet);

	//客户端添加交易物品
	void ClientDealAddItem(CDataPacketReader &packet);

	void ClientChangeDealCoin(CDataPacketReader &packet);

	//客户端锁定交易
	void ClientLockDeal(CDataPacketReader &packet);

	//客户端取消交易
	void ClientCancelDeal(CDataPacketReader &packet);

	//客户端确认交易
	void ClientConfimDeal(CDataPacketReader &packet);

	/*
	* Comments: 检测交易双方距离（可视范围内才允许交易）
	* Param CActor * pActor1：交易甲方
	* Param CActor * pActor2：交易乙方
	* @Return bool:
	*/	
	bool CheckDealTargetDistance(CActor *pActor1, CActor *pActor2);

private:
	//清空交易数据
	inline void ClearDealData()
	{
		ZeroMemory(m_DealItems, sizeof(m_DealItems));
		ZeroMemory(m_DealItemList, sizeof(m_DealItemList));
		ZeroMemory(m_DealItemCount, sizeof(m_DealItemCount));
		m_nDealCoin = 0;
		m_nItemCount = 0;
		m_boDealing = false;
		m_boDealLocked = false;
		m_boDealConfimed = false;
		m_pDealTarget = NULL;
		m_nDealYb =0;
	}

	//验证交易物品是否有效
	bool ValidateDealItems();
	//完成交易并取走对方的物品和钱币
	void CompleteDeal();
	bool DealInCd(unsigned int nActorId)
	{
		int nNowTime = time(NULL);
		std::map<uint32_t, uint32_t>::iterator it = m_nActorIdCds.begin();
		for(;it != m_nActorIdCds.end(); it++)
		{
			if(it->first == nActorId && it->second > nNowTime)
			{
				return false;
			}
		}
		return true;
	}
	//获取以满足申请数量
	int  getApplyCount()
	{
		int nNowTime = time(NULL);
		int nCount = 0;
		std::map<uint32_t, uint32_t>::iterator it = m_nActorIdCds.begin();
		for(;it != m_nActorIdCds.end(); it++)
		{
			if( it->second > nNowTime)
				nCount++;
		}
		return nCount;
	}

	int  AddApplyIdCd(unsigned int nActorId, int nCd)
	{
		m_nActorIdCds[nActorId] = nCd;
	}
public:
	//查找交易物品
	bool FindDealItem(const CUserItem *pUserItem);
private:
	CActor*			m_pDealTarget;					//交易目标角色
	INT_PTR			m_nItemCount;					//交易放入的物品数量
	CUserItem*		m_DealItems[MaxDealItemCount];	//交易放入的物品列表
	int             m_DealItemList[MaxDealItemCount]; //交易放入物品的位置
	int             m_DealItemCount[MaxDealItemCount]; //交易放入物品的位置
	unsigned int	m_nDealCoin;					//交易放入的钱币数量
	unsigned int	m_nDealYb;					    //交易放入的元宝数量
	bool			m_boDealing;					//我现在是否正在交易
	bool			m_boDealLocked;					//交易是否已被锁定
	bool			m_boDealConfimed;				//交易是否已经确认

	std::map<uint32_t, uint32_t> m_nActorIdCds; 				//邀请Cd
	std::map<uint32_t, uint32_t> m_nApplyIdCds; 				//申请id cd
public:
	int             m_nApplyCount = 0;              //被邀请数量

};
