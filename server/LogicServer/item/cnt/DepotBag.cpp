#include "StdAfx.h"
#include "DepotBag.h"



//添加物品
VOID CDeportBag::OnAddItem(const CStdItem * pStdItem, INT_PTR nItemCount,INT_PTR nLogIdent,const CUserItem *pUserItem)
{

}


VOID CDeportBag::LogNewItem(const CUserItem *pUserItem, const CStdItem *pStdItem, LPCSTR lpSender, const INT_PTR nLogIdent) const 
{
	if(pUserItem ==NULL || !pStdItem )return;
	if ( m_pEntity->GetHandle().GetType() == enActor )
	{
		char sSeries[64]; 
		sprintf(sSeries,"%lld",pUserItem->series.llId); //序列号

		char sHandPos[64];
		sprintf(sHandPos,"%d",pUserItem->btHandPos); //序列号
		if(GetLogicServer()->GetLocalClient() && pStdItem->m_Flags.recordLog)
		{
		}
	}
}

VOID CDeportBag::LogItemCountChange(const CUserItem *pUserItem, const CStdItem *pStdItem, const INT_PTR nCountChg, LPCSTR lpSender, const INT_PTR nLogIdent) const 
{
	if ( m_pEntity->GetHandle().GetType() == enActor )
	{
	//TODO:实现记录获得物品并叠加到现有物品的日志，日志号是nLogIdent，物品给予者是lpSender，物品变更的数量为nCountChg
		if(pUserItem ==NULL || !pStdItem )return;
		char sSeries[64]; 
		sprintf(sSeries,"%lld",pUserItem->series.llId); //序列号

		char sHandPos[64];
		INT_PTR nPos = CEquipVessel::GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType,pUserItem->btHandPos);
		sprintf(sHandPos,"%d",nPos); //序列号

		SendItemCountChange(pUserItem);
		if(GetLogicServer()->GetLocalClient() && pStdItem->m_Flags.recordLog)
		{
		}
	}
}

VOID CDeportBag::SendAddItem(const CUserItem *pUserItem,INT_PTR nLogIdent, BYTE bNoBatchUse, BYTE bNotice) const 
{
	if(m_pEntity ==NULL || pUserItem ==NULL ) return;
	if(nLogIdent )
	{
		CActorPacket pack;
		CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
		data << (BYTE)enDepotSystemID << (BYTE)sDeportAddItem ;
		(CUserItem)(*pUserItem) >> data;
		pack.flush();
	}
		/*
		const CStdItem  * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if(pStdItem  && pStdItem->isEquipment() ) //装备需要发提示
		{
			(( CDeportBag*)this)->SendBagItemChangeTipmsg(pUserItem->wItemId,pUserItem->btCount,(CUserItem*)pUserItem,nLogIdent ); //装备肯定是一件
		}
		*/
	
}

//物品的叠加数量发生改变的时候发消息给玩家
VOID CDeportBag::SendItemCountChange(const CUserItem *pUserItem, bool isGetNewCount, BYTE bNoBatchUse) const
{
	if (!m_pEntity || m_pEntity->GetType() != enActor || !m_pEntity->IsInited() || m_pEntity->IsDestory())
	{
		return;
	}
	CActorPacket pack;
	CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
	data << (BYTE)enDepotSystemID << (BYTE)sDeportItemCountChange;
	data<<	(LONG64) pUserItem->series.llId << (WORD) pUserItem->wCount;
	pack.flush();
}


//删除物品
VOID CDeportBag::OnDeleteItem(const CStdItem * pStdItem, INT_PTR nItemCount,INT_PTR nLogIdent, int nMsgId,const CUserItem *pUserItem)
{

}

VOID CDeportBag::SendDeleteItem(const CUserItem *pUserItem,INT_PTR nLogIdent) const 
{
	if(m_pEntity ==NULL) return;
	if ( m_pEntity->GetHandle().GetType() == enActor )
	{
		CActorPacket pack;
		CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
		data << (BYTE)enDepotSystemID << (BYTE)sDeportDelItem ;
			data << (LONG64)pUserItem->series.llId;
		pack.flush();
	}
}

