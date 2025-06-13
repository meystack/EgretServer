#include "StdAfx.h"

void AsyncOpCollector::SetActor(CActor *pActor)
{
	m_pActor = pActor;
}

bool AsyncOpCollector::CanAsyncOp(tagAsyncOPType nType)
{
	if (m_curAsyncOpType != asop_none)
		return false;

	return true;
}

bool AsyncOpCollector::StartAsyncOp(tagAsyncOPType nType, unsigned int nExpireTime, void *data, bool bActive)
{	
	if (asop_none == nType || (bActive && !CanAsyncOp(nType))) return false;
		
	if (bActive)
		m_curAsyncOpType = nType;
		
	AsyncOpItem item;
	item.m_nType		= nType;
	item.m_bActive		= bActive;
	item.m_nExpireTime	= GetGlobalLogicEngine()->getTickCount() + nExpireTime;
	item.m_pData		= data;
	m_asyncOpList.add(item);

	return true;
}

void AsyncOpCollector::Update(TICKCOUNT nCurrentTime)
{
	for (INT_PTR i = m_asyncOpList.count()-1; i >= 0 ; i--)
	{
		AsyncOpItem &item = m_asyncOpList[i];
		if (item.m_nExpireTime < nCurrentTime)
		{
			OnAsyncOpExpired(item);
			if (item.m_pData)
				delete item.m_pData;
			m_asyncOpList.remove(i);
		}
	}
}


void AsyncOpCollector::NotifyAsyncOpResult(tagAsyncOPType nType, int nResult, void *pData)
{
	switch (nType)
	{
	case asop_withdrawYuanBao:
		OnResultWidthdrawYuanBao(nResult);
		break;
	case asop_useSACard:
		OnResultUseSACard(nResult, pData);
		break;
	case asop_createGuild:
		break;
	default:
		break;
	}
}

void AsyncOpCollector::OnAsyncOpExpired(AsyncOpItem &item)
{	
	switch (item.m_nType)
	{
	case asop_withdrawYuanBao:
		break;
	case asop_useSACard:
		OnUseSACardExpired();
		break;
	case asop_createGuild:
		break;
	default:
		break;
	}
}

void AsyncOpCollector::OnResultUseSACard(int nResult, void *pData)
{	
	for (INT_PTR i = m_asyncOpList.count()-1; i >= 0 ; i--)
	{
		AsyncOpItem &item = m_asyncOpList[i];
		if (item.m_nType == m_curAsyncOpType && item.m_bActive)
		{		
			if (item.m_pData)
				delete item.m_pData;
			m_asyncOpList.remove(i);
		}
	}

	m_curAsyncOpType = asop_none;
	if (!pData || !m_pActor) return;
	SACardInfo *pCard = (SACardInfo *)pData;

	CScriptValueList paramList, retParamList;
	//m_pActor->InitEventScriptVaueList(paramList, aeAsyncOpResult);
	paramList << (BYTE)asop_useSACard << (BYTE)nResult << (BYTE)(pCard->m_bQuery) << (BYTE)(pCard->m_nErrorCode);
	/*char szBuf[21] = {0};
	sprintf(szBuf, "%lld", (__int64)(pCard->m_nSN));
	paramList << szBuf;*/
	double dSn;
	memcpy(&dSn, &(pCard->m_nSN), sizeof(double));
	paramList << dSn;
	paramList << pCard->m_nType << pCard->m_nSubType;	
	m_pActor->OnEvent(aeAsyncOpResult,paramList, retParamList);
}

void AsyncOpCollector::OnUseSACardExpired()
{
	CScriptValueList paramList, retParamList;
	//m_pActor->InitEventScriptVaueList(paramList, aeAsyncOpResult);
	paramList << (BYTE)asop_useSACard << (BYTE)op_Expired;
	m_pActor->OnEvent(aeAsyncOpResult,paramList, retParamList);
}

void AsyncOpCollector::OnResultWidthdrawYuanBao(int nResult)
{
}
