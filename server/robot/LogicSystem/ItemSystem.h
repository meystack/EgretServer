#pragma once

class CItemSystem : public CEntitySystem<enBagSystemID>
{
public:
	void OnRecvData(INT_PTR nCmd, CDataPacketReader& packet);

	void OnEnterGame();
public:
	CUserItem* GetItem(CUserItem::ItemSeries series);
private:
	// 消息处理
	void DefaultProcessHandler(CDataPacketReader&){}
	void ProcessInitItems(CDataPacketReader& packet);

	// 请求物品列表
	void RequestItemList();

	/*
	* Comments:添加新物品
	* Param CDataPacketReader & packet:
	* @Return void:
	*/
	void AddNewItem(CDataPacketReader& packet);
	void DelItem(CDataPacketReader& packet);

	void UseItem(LONG64 nGuid, bool bIsHeroUse = false, int nParam = 0 );
private:
	typedef void (CItemSystem::*ItemMessageHandler)(CDataPacketReader& packet);
	
	static ItemMessageHandler s_aHandler[];

	CBaseList<CUserItem>			m_itemList;
};