#include "stdafx.h"


CEquipSystem::EquipMessageHandler CEquipSystem::s_aHandler[] = {
	&CEquipSystem::DefaultProcessHandler,
	&CEquipSystem::ProcessTakeOnEquip,
	&CEquipSystem::ProcessTakeOffEquip,
	&CEquipSystem::ProcessInitEquip,		// 初始化物品列表
};

void  CEquipSystem::ProcessTakeOnEquip(CDataPacketReader& packet)
{
	CUserItem::ItemSeries series;
	packet >> series.llId;
	CUserItem* pItem = m_pClient->GetLogicAgent().GetItemSystem().GetItem(series);
	m_equipList.add(*pItem);
	
}
void  CEquipSystem::ProcessTakeOffEquip(CDataPacketReader& packet)
{
	CUserItem::ItemSeries series;
	packet >> series.llId;
	for (int i=0; i < m_equipList.count(); i++)
	{
		CUserItem& useItem =  m_equipList[i];
		if (useItem.series.llId == series.llId)
		{
			m_equipList.remove(i);
			break;
		}
	}
}
void CEquipSystem::ProcessInitEquip(CDataPacketReader& packet)
{
	//OutputMsg(rmNormal, _T("Recv Initialize EquipList"));
	BYTE bCount = 0;
	packet >> bCount;
	CUserItem item;
	for (INT_PTR i = 0; i < bCount; i++)
	{
		packet >> item;
		m_equipList.add(item);
	}
}

void CEquipSystem::OnRecvData(INT_PTR nCmd, CDataPacketReader& packet)
{
	if (nCmd < 0 || nCmd >= ArrayCount(s_aHandler))
		return;

	return (this->*s_aHandler[nCmd])(packet);
}

void CEquipSystem::OnEnterGame()
{
	RequestEquipList();
}

void CEquipSystem::RequestEquipList()
{
	CDataPacket& pack = m_pClient->AllocProtoPacket();
	pack << (BYTE)enEuipSystemID << (BYTE)cGetEquip;
	m_pClient->FlushProtoPacket(pack);
}

void CEquipSystem::CheckAndTakeOnEquip(CUserItem& userItem)
{
	// const CStdItem* pStdItem = GetRobotMgr()->GetGlobalConfig().GetItemConfig().GetStdItemConfig(userItem.wItemId);
	// if (!pStdItem)
	// {
	// 	return;
	// }

	// for (int i =0; i < m_equipList.count(); i++)
	// {
	// 	CUserItem& useItem =  m_equipList[i];
	// 	const CStdItem* pStdEquip = GetRobotMgr()->GetGlobalConfig().GetItemConfig().GetStdItemConfig(userItem.wItemId);
	// 	if (!pStdEquip ) continue;
	// 	if (pStdEquip == pStdItem)
	// 	{
	// 		return;
	// 	}
	// 	if (pStdEquip->m_btType == pStdItem->m_btType)
	// 	{
	// 		if (pStdEquip->GetUseConditionValue(CStdItem::ItemUseCondition::ucLevel) > pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucLevel))
	// 		{
	// 			return;
	// 		}
	// 		else if (pStdEquip->GetUseConditionValue(CStdItem::ItemUseCondition::ucMinCircle) > pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucMinCircle))
	// 		{
	// 			return;
	// 		}
	// 	}
	// }
	// TakeOnEquip(userItem.series);
}
void CEquipSystem::TakeOnEquip(CUserItem::ItemSeries series)
{
	CDataPacket& packet = m_pClient->AllocProtoPacket();
	packet << (BYTE)enEuipSystemID << (BYTE)cTakeOnEquip;
	packet << series.llId;
	m_pClient->FlushProtoPacket(packet);
}
