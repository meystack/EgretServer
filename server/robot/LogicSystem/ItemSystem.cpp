#include "stdafx.h"


CItemSystem::ItemMessageHandler CItemSystem::s_aHandler[] = {
	&CItemSystem::DefaultProcessHandler,
	&CItemSystem::DelItem,
	&CItemSystem::AddNewItem,
	&CItemSystem::DefaultProcessHandler,	
	&CItemSystem::ProcessInitItems,		// 初始化物品列表
};

void CItemSystem::ProcessInitItems(CDataPacketReader& packet)
{
	OutputMsg(rmNormal, _T("Recv Initialize ItemList"));
	WORD wCount = 0;
	packet >> wCount;
	CUserItem item;
	for (INT_PTR i = 0; i < wCount; i++)
	{
		packet >> item;
		m_itemList.add(item);
	}
}

void CItemSystem::OnRecvData(INT_PTR nCmd, CDataPacketReader& packet)
{
	if (nCmd < 0 || nCmd >= ArrayCount(s_aHandler))
		return;

	return (this->*s_aHandler[nCmd])(packet);
}

void CItemSystem::OnEnterGame()
{
	RequestItemList();
}

void CItemSystem::RequestItemList()
{
}

CUserItem* CItemSystem::GetItem(CUserItem::ItemSeries series)
{
	for (INT_PTR i = 0; i < m_itemList.count(); i++)
	{
		CUserItem* item = &m_itemList[i];
		if (item->series.llId == series.llId)
		{
			return item;
		}
	}
	return NULL;
}
void CItemSystem::DelItem(CDataPacketReader& packet)
{
	CUserItem::ItemSeries series;
	packet >> series.llId;
	for (INT_PTR i = 0; i < m_itemList.count(); i++)
	{
	    CUserItem& item = m_itemList[i];
		if (item.series.llId == series.llId)
		{
			m_itemList.remove(i);
			break;
		}
	}
}
void CItemSystem::UseItem(LONG64 nGuid, bool bIsHeroUse, int nParam )
{
	CDataPacket& packet = m_pClient->AllocProtoPacket();
	packet << (BYTE)enBagSystemID << (BYTE)enBagSystemcUseItem;
	packet << nGuid << (BYTE)bIsHeroUse <<nParam;
	m_pClient->FlushProtoPacket(packet);
}
void CItemSystem::AddNewItem(CDataPacketReader& packet)
{
// 	CUserItem userItem;
// 	packet >> userItem;

// 	m_itemList.add(userItem);

// 	const CStdItem* pStdItem = GetRobotMgr()->GetGlobalConfig().GetItemConfig().GetStdItemConfig(userItem.wItemId);
// 	if (pStdItem)
// 	{
// 		if (pStdItem->m_btType == Item::itFunctionItem)
// 		{
// 			UseItem(userItem.series.llId);
// #ifdef _DEBUG
// 			OutputMsg(rmNormal,"[%s] UseItem %s", m_pClient->GetActorName(),pStdItem->m_sName);
// #endif
// 		}
// 		else if (pStdItem->isEquipment())
// 		{
// 			m_pClient->GetLogicAgent().GetEquipSystem().CheckAndTakeOnEquip(userItem);
// 		}
// 	}
	
}