#pragma once

/*
*玩家仓库的处理类
*玩家的仓库保留了多个背包
*/

class CUserDepot :
	public CEntitySubSystem<enDepotSystemID, CUserDepot, CActor>
{
	friend class CUserEquipment;
	
public:
	
	typedef CEntitySubSystem<enDepotSystemID, CUserDepot, CActor> InheritedSybSystem;
	friend class CEntitySubSystem<enDepotSystemID, CUserDepot, CActor>;



public:
	/*** 覆盖CEntitySubSystem父类的函数集 ***/
	//初始化
	virtual bool Initialize(void *data,SIZE_T size);
	//析构
	virtual VOID Destroy()
	{
		m_depotBag.Clear(NULL, 0);	
		InheritedSybSystem::Destroy();
	}

	//打开仓库条件
	bool CanOpenDepot(CActor *pEntity);
	/*
	* Comments: 玩家的数据存盘
	* Param PACTORDBDATA pData:角色数据指针
	* @Return void:
	*/
	virtual void Save(PACTORDBDATA  pData);
	
	/*
	* Comments: 处理网络数据包
	* Param INT_PTR nCmd: 命令码
	* Param CDataPacketReader & packet: 数据
	* @Return VOID:
	*/
	virtual VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet ); 
	

	/*
	* Comments: db 返回数据
	* Param INT_PTR nCmd:命令码
	* Param INT_PTR nErrorCode:错误码
	* Param CDataPacketReader & reader:数据读取器
	* @Return VOID:
	*/
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	//通过guid删除仓库中的物品
	INT_PTR DeleteItemByGuid(CUserItem::ItemSeries guid,LPCTSTR sLogStr, int nLogId);

	/* 
	* Comments:获取仓库物品id的数量
	* Param int nItemId:物品id
	* @Return int:返回数量
	*/
	int GetDepotItemCount(int nItemId);

	//增加仓库格子，成就特权使用
	void AddDepot(WORD nIdx);					

protected:
	void HandError(CDataPacketReader &packet){};   
	void HandGetItems(CDataPacketReader &packet); //获取仓库的物品列表
	void HandItemBag2Deport(CDataPacketReader &packet); //把一个物品从背包拖放到仓库
	void HandItemDepot2Bag(CDataPacketReader &packet); //把一个物品从仓库拖放到背包
	void HandRemoveItem(CDataPacketReader &packet); //删除仓库物品
	void HandDepotBuy(CDataPacketReader & packet);		//购买仓库页
	void HandArrangeItemList(CDataPacketReader & packet);//整理

	//向客户端发送一个仓库的数据
	void SendClientDepotData();

	bool HandItemDepot2BagLogic(CUserItem::ItemSeries series);	//根据index从仓库取一个物品

private:
	CDeportBag m_depotBag; //玩家的仓库背包
};