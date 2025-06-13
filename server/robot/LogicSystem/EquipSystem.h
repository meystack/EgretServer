#pragma once

class CEquipSystem : public CEntitySystem<enEuipSystemID>
{

public:
	static const INT_PTR EquipmentCount = itMaxEquipPos;
	void OnRecvData(INT_PTR nCmd, CDataPacketReader& packet);

	void OnEnterGame();
	void CheckAndTakeOnEquip(CUserItem& userItem);

private:
	// 消息处理
	void DefaultProcessHandler(CDataPacketReader&){}
	void ProcessInitEquip(CDataPacketReader& packet);
	void ProcessTakeOnEquip(CDataPacketReader& packet);
	void ProcessTakeOffEquip(CDataPacketReader& packet);

	// 请求物品列表
	void RequestEquipList();
	void TakeOnEquip(CUserItem::ItemSeries series);
	
	

private:
	typedef void (CEquipSystem::*EquipMessageHandler)(CDataPacketReader& packet);
	
	static EquipMessageHandler s_aHandler[];

	CBaseList<CUserItem>	m_equipList;
};