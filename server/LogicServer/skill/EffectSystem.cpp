#pragma once


#include "StdAfx.h"

void CEffectSystem::AddEffect(INT_PTR nType,INT_PTR nId,INT_PTR nDurTime)
{
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CEntityEffect *pMsg = pLogicEngine->AllocEntityEffect();
	pMsg->bEffectType =(BYTE) nType;
	pMsg->wEffectID = (WORD)nId;
	pMsg->nExpiredTick = ((nDurTime<0)?MAXLONG64:pLogicEngine->getTickCount() + nDurTime );
	m_effects.linkAtLast(pMsg);
}

bool CEffectSystem::IsExist(int nEffectType ,int nEffectId)
{
	CList<CEntityEffect*>::Iterator it(m_effects);
	CList<CEntityEffect*>::NodeType *pNode;
	CEntityEffect *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	INT_PTR nResult =0;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (pMsg->bEffectType  == nEffectType && pMsg->wEffectID ==nEffectId )
		{
			return true;
		}
	}
	return false;
}

//追加特效
void CEffectSystem::AppendEffectData(CDataPacket &data)
{
	//数量
	INT_PTR nCount = m_effects.count();
	data <<(BYTE) nCount; 
	if(nCount <=0 )return;

	CList<CEntityEffect*>::Iterator it(m_effects);
	CList<CEntityEffect*>::NodeType *pNode;
	CEntityEffect *pMsg;
	TICKCOUNT nTick =   GetGlobalLogicEngine()->getTickCount() ;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		INT_PTR nLeftTime = (INT_PTR)(pMsg->nExpiredTick - nTick);
		if(nLeftTime <=0) nLeftTime =0; //这里防止出现负数
		if((int)nLeftTime < 0) nLeftTime = ((DWORD)nLeftTime) >> 2;	//如果数字太大的话，改小点，以免客户端溢出，反正数字足够大就可以了
		data << (BYTE)pMsg->bEffectType << (WORD)pMsg->wEffectID << DWORD(nLeftTime);
	}
}

//删除特效
INT_PTR  CEffectSystem::DelEffect(INT_PTR nType,INT_PTR nId)
{
	CList<CEntityEffect*>::Iterator it(m_effects);
	CList<CEntityEffect*>::NodeType *pNode;
	CEntityEffect *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	INT_PTR nResult =0;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (pMsg->bEffectType  == nType && pMsg->wEffectID ==nId )
		{
			OnDeleteEffect(pMsg);
			it.remove(pNode);
			pLogicEngine->DestoryEntityEffect(pMsg);
			nResult ++;
		}
	}
	return nResult;
}

INT_PTR  CEffectSystem::DelAllEffect(bool nNeedBroadCast )
{
	CList<CEntityEffect*>::Iterator it(m_effects);
	CList<CEntityEffect*>::NodeType *pNode;
	CEntityEffect *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	INT_PTR nResult =0;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if(nNeedBroadCast)
		{
			OnDeleteEffect(pMsg); //删除这条消息
		}
		pLogicEngine->DestoryEntityEffect(*pNode);
		nResult ++;
	}
	m_effects.clear(); 
	return nResult;
}

void CEffectSystem::OnDeleteEffect(const CEntityEffect * pEffect)
{
	//只有大于一定时间的效果才需要广播的
	if(pEffect->nExpiredTick - GetGlobalLogicEngine()->getTickCount() > 1000 )
	{
		char buff[128];
		CDataPacket data(buff,sizeof(buff));
		data << (BYTE)enDefaultEntitySystemID <<(BYTE) sDelEffect ;
		data <<  (Uint64)GetHandle();
		data << (BYTE) pEffect->bEffectType << (WORD)pEffect->wEffectID; //特效的类型以及ID
		m_pEntity->GetObserverSystem()->BroadCast(data.getMemoryPtr(), data.getPosition(),true); 
	}
}

VOID CEffectSystem::OnTimeCheck(TICKCOUNT nTick)
{
	if ( !m_pEntity )
	{
		return;
	}
	if(m_effects.count() <=0) return;
	CEntityEffect *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CList<CEntityEffect*>::Iterator it(m_effects);
	CList<CEntityEffect*>::NodeType *pNode;

	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (nTick >= pMsg->nExpiredTick)
		{
			it.remove(pNode);
			pLogicEngine->DestoryEntityEffect(pMsg);
		}
	}
}