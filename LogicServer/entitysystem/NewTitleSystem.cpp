#include "StdAfx.h"
#include "../base/Container.hpp"
CNewTitleSystem::CNewTitleSystem()
{
	isInit = false;
	m_NewTitleDataList.clear();
	m_CustomTitleDataList.clear();
}


CNewTitleSystem::~CNewTitleSystem()
{

}

bool CNewTitleSystem::Initialize(void *data,SIZE_T size)
{
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
	{
		OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
		return false;
	}
	LoadFromDB();
	
	return true;
}

void CNewTitleSystem::Destroy()
{
}


void CNewTitleSystem::OnEnterScene()
{
	if ((!isInit) && ((CActor*)m_pEntity)->m_isFirstLogin)
	{
		std::vector<int> vecCreateRoleAutoAddTitleId = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo().vecCreateRoleAutoAddTitleId;
		int num = vecCreateRoleAutoAddTitleId.size();
		for (size_t i = 0; i < num; i++)
		{
			if(vecCreateRoleAutoAddTitleId[i]) 
			{
				addNewTitle(vecCreateRoleAutoAddTitleId[i]);
			}
		}
		isInit = true;
	}
}

VOID CNewTitleSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor) 
	{
		return;
	}
	INT_PTR nCount = m_NewTitleDataList.count();
	// if ( nCount <= 0 )
	// {
	// 	return;
	// }
	unsigned int nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime(); //当前的时间
	for(INT_PTR i= nCount -1 ; i>=0 ; i--)
	{
		NEWTITLEDATA* tempData = &m_NewTitleDataList[i];
		if (!tempData)
		{
			return;
		}
		bool boTime = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().IsTimeNewTitle(tempData->wId);
		if ( boTime )
		{
			if (tempData->nRemainTime <= nCurrentTime)
			{
				delNewTitle(tempData->wId);
			}
		}
	}

	nCount = m_CustomTitleDataList.count();
	// if ( nCount <= 0 )
	// {
	// 	return;
	// }
	nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime(); //当前的时间
	for(INT_PTR i= nCount -1 ; i>=0 ; i--)
	{
		NEWTITLEDATA* tempData = &m_CustomTitleDataList[i];
		if (!tempData)
		{
			return;
		}
		bool boTime = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().IsTimeCustomTitle(tempData->wId);
		if ( boTime )
		{
			if (tempData->nRemainTime <= nCurrentTime)
			{
				delCustomTitle((unsigned int)tempData->wId);
			}
		}
	}
}
void CNewTitleSystem::OnEnterGame()
{
	if(!m_pEntity) return;
	if(m_nMaxTips.size() > 0) {
		for(auto i : m_nMaxTips) {
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(i);
			if (pConfig)
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(pConfig->nVIPnotice, tstExclusive, pConfig->name, ((CActor*)m_pEntity)->GetEntityName());
			}
		}
	}
	if(m_nMaxCustomTips.size() > 0) {
		for(auto i : m_nMaxCustomTips) {
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(i);
			if (pConfig)
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(pConfig->nVIPnotice, tstExclusive, pConfig->name, ((CActor*)m_pEntity)->GetEntityName());
			}
		}
	}

	GetGlobalLogicEngine()->GetRankingMgr().SendRankTitle(((CActor*)m_pEntity));

	int nCurNewTitle = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CURNEWTITLE);
	OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(nCurNewTitle);
	if (IsHaveNewTitle(nCurNewTitle) < 0 || ( GetLogicServer()->IsCrossServer() && pConfig && pConfig->bCrossServerHide ) )
	{
		SetCurNewTitle(0);
	}

	int nCurCustomTitle = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CURCUSTOMTITLE);
	pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(nCurCustomTitle);
	if (IsHaveCustomTitle(nCurCustomTitle) < 0 || ( GetLogicServer()->IsCrossServer() && pConfig && pConfig->bCrossServerHide ) )
	{
		SetCurCustomTitle(0);
	}
}

VOID CNewTitleSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader)
{	
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("LoadNewTitle Data is Error..."));
		return;
	}
	if ( nCmd == jxInterSrvComm::DbServerProto::dcLoadNewTitleData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return NewTitle repeated,actorid=%u "),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}
		
		int nCount = 0;
		reader >> nCount;
		int nMax = 0;
		for (int i=0;i<nCount;i++)
		{
			unsigned int nValue = 0,  nTime = 0;
			reader >> nValue >> nTime;
			WORD wId = LOWORD(nValue);
			NEWTITLEDATA newTitleData;
			newTitleData.wId = wId;
			newTitleData.nRemainTime = nTime;
			m_NewTitleDataList.add(newTitleData);
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(wId);
			if (pConfig && pConfig->nNoticeLv != 0)
			{
				if(pConfig->nNoticeLv == nMax)
					m_nMaxTips.push_back(wId);
				
				if(pConfig->nNoticeLv > nMax){
					m_nMaxTips.clear();
					m_nMaxTips.push_back(wId);
					nMax = pConfig->nNoticeLv;
				}
			}
		}
	}
	else if ( nCmd == jxInterSrvComm::DbServerProto::dcSaveNewTitleData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		return;
	}
	else if ( nCmd == jxInterSrvComm::DbServerProto::dcLoadCustomTitleData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return NewTitle repeated,actorid=%u "),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}
		
		int nCount = 0;
		reader >> nCount;
		int nMax = 0;
		for (int i=0;i<nCount;i++)
		{
			unsigned int nValue = 0,  nTime = 0;
			reader >> nValue >> nTime;
			WORD wId = LOWORD(nValue);
			NEWTITLEDATA newTitleData;
			newTitleData.wId = wId;
			newTitleData.nRemainTime = nTime;
			m_CustomTitleDataList.add(newTitleData);
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(wId);
			if (pConfig && pConfig->nNoticeLv != 0)
			{
				if(pConfig->nNoticeLv == nMax)
					m_nMaxCustomTips.push_back(wId);
				
				if(pConfig->nNoticeLv > nMax){
					m_nMaxCustomTips.clear();
					m_nMaxCustomTips.push_back(wId);
					nMax = pConfig->nNoticeLv;
				}
			}
		}
		OnDbInitData();
		((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_NEWTITLE_SYSTEM);
	}
	else if ( nCmd == jxInterSrvComm::DbServerProto::dcSaveCustomTitleData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		return;
	}
}

void CNewTitleSystem::LoadFromDB()
{
	if(!m_pEntity)
	{
 		return;
	}

    if (((CActor*)m_pEntity)->OnGetIsTestSimulator() )
    {
        return; 
    } 
	unsigned int nActorId = m_pEntity->GetId();
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadNewTitleData);
		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)nActorId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

		CDataPacket& DataPacket2 = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCustomTitleData);
		DataPacket2 << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket2 << (unsigned int)nActorId;
		 
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket2); 
	}	
}

void CNewTitleSystem::SaveToDB()
{
	if (!m_pEntity)
	{
		return;
	}
	if (!HasDataModified())
		return;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (!pDbClient->connected())
	{
		return;
	}
	unsigned int nActorId = m_pEntity->GetId();
	CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveNewTitleData);
	DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	DataPacket << (unsigned int)nActorId;
	INT_PTR nDataCount = 0;
	int nPos = DataPacket.getPosition();
	DataPacket << int(nDataCount);
	for (int i=0; i<m_NewTitleDataList.count();i++)
	{
		NEWTITLEDATA* newTitleData = &m_NewTitleDataList[i];
		if ( newTitleData )
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(newTitleData->wId);
			if (pConfig && !pConfig->nOfflieDelete)
			{
				unsigned int nMakeLong = MAKELONG(newTitleData->wId,MAKEWORD(0,0));
				DataPacket << (unsigned int)nMakeLong << (unsigned int)newTitleData->nRemainTime;
				nDataCount++;
			}
		}
	}
	int *ptrPos = (int*)DataPacket.getPositionPtr(nPos);
	*ptrPos = (int)nDataCount;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

	CDataPacket& DataPacket2 = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveCustomTitleData);
	DataPacket2 << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	DataPacket2 << (unsigned int)nActorId;
	nDataCount = 0;
	nPos = DataPacket2.getPosition();
	DataPacket2 << int(nDataCount);
	for (int i=0; i < m_CustomTitleDataList.count();i++)
	{
		NEWTITLEDATA* newTitleData = &m_CustomTitleDataList[i];
		if ( newTitleData )
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(newTitleData->wId);
			if (pConfig && !pConfig->nOfflieDelete)
			{
				unsigned int nMakeLong = MAKELONG(newTitleData->wId,MAKEWORD(0,0));
				DataPacket2 << (unsigned int)nMakeLong << (unsigned int)newTitleData->nRemainTime;
				nDataCount++;
			}
		}
	}
	ptrPos = (int*)DataPacket2.getPositionPtr(nPos);
	*ptrPos = (int)nDataCount;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket2);
	
	SetDataModifyFlag(false);
	
}

VOID CNewTitleSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet)
{

	if(m_pEntity == NULL) return;
	if(m_pEntity->GetHandle().GetType() != enActor) return;
	switch(nCmd)
	{
		case cClientGetNewTitleData:
			{
				SendAllNewTitleData();
				break;
			}
		case cClientCutOverNewTitle:
			{
				ClientCutOverNewTitleId(packet);
				break;
			}
		case cClientGetCustomTitleData:
			{
				SendAllCustomTitleData();
				break;
			}
		case cClientCutOverCustomTitle:
			{
				ClientCutOverCustomTitleId(packet);
				break;
			}
		default:
			break;
	}
}

void CNewTitleSystem::ClientCutOverNewTitleId(CDataPacketReader &packet)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	WORD wId = 0;
	packet >> wId;
	if (wId != 0 && IsHaveNewTitle(wId) < 0)
	{
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(wId);
		if (pConfig == NULL)
		{
			return;
		}
		return m_pEntity->SendOldTipmsgFormatWithId(tpClientCutOverNewTitleId,ttFlyTip,pConfig->name);
	}

	OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(wId);
	if ( GetLogicServer()->IsCrossServer() && pConfig && pConfig->bCrossServerHide )
	{
		return;
	}

	SetCurNewTitle(wId);
}

void CNewTitleSystem::ClientCutOverCustomTitleId(CDataPacketReader &packet)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	WORD wId = 0;
	packet >> wId;
	if (wId != 0 && IsHaveCustomTitle(wId) < 0)
	{
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(wId);
		if (pConfig == NULL)
		{
			return;
		}
		return m_pEntity->SendOldTipmsgFormatWithId(tpClientCutOverNewTitleId,ttFlyTip,pConfig->name);
	}

	OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(wId);
	if ( GetLogicServer()->IsCrossServer() && pConfig && pConfig->bCrossServerHide )
	{
		return;
	}

	SetCurCustomTitle(wId);
}

int CNewTitleSystem::IsHaveNewTitle( WORD wId )
{
	for (INT_PTR i = m_NewTitleDataList.count() - 1; i >= 0 ; i--)
	{
		NEWTITLEDATA* tempData = &m_NewTitleDataList[i];
		if (tempData)
		{
			if ( tempData->wId == wId )
			{
				return (int)i;
			}
		}
	}
	return -1;
}

int CNewTitleSystem::IsHaveCustomTitle( WORD wId )
{
	for (INT_PTR i = m_CustomTitleDataList.count() - 1; i >= 0 ; i--)
	{
		NEWTITLEDATA* tempData = &m_CustomTitleDataList[i];
		if (tempData)
		{
			if ( tempData->wId == wId )
			{
				return (int)i;
			}
		}
	}
	return -1;
}

bool CNewTitleSystem::addNewTitleTime( WORD wId )	//头衔续期
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(wId);
	if ( !pConfig  || pConfig->nNewTitleTime <= 0)
	{
		return false;
	}
	/*
	if ( IsHaveNewTitle(wId) < 0 )
	{
		return addNewTitle(wId);
	}*/
	for (INT_PTR i = m_NewTitleDataList.count() - 1; i >= 0 ; i--)
	{
		NEWTITLEDATA* tempData = &m_NewTitleDataList[i];
		if (tempData)
		{
			if (tempData->wId == wId )
			{
				unsigned int nTime = 0;
				tempData->nRemainTime = tempData->nRemainTime + pConfig->nNewTitleTime;
				int nLastTime = tempData->nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
				nLastTime = nLastTime > 0 ? nLastTime : 0;
				SendAddTimeTitleId(wId, nLastTime);
				m_pEntity->SendOldTipmsgFormatWithId(tpAddNewTitleTime,ttFlyTip,pConfig->name,pConfig->nNewTitleTime);
				return true;
			}
		}
	}
	return false;
}

bool CNewTitleSystem::addCustomTitleTime( WORD wId )	//头衔续期
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(wId);
	if ( !pConfig  || pConfig->nNewTitleTime <= 0)
	{
		return false;
	}
	/*
	if ( IsHaveNewTitle(wId) < 0 )
	{
		return addNewTitle(wId);
	}*/
	for (INT_PTR i = m_CustomTitleDataList.count() - 1; i >= 0 ; i--)
	{
		NEWTITLEDATA* tempData = &m_CustomTitleDataList[i];
		if (tempData)
		{
			if (tempData->wId == wId )
			{
				unsigned int nTime = 0;
				tempData->nRemainTime = tempData->nRemainTime + pConfig->nNewTitleTime;
				int nLastTime = tempData->nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
				nLastTime = nLastTime > 0 ? nLastTime : 0;
				SendAddTimeCustomTitleId(wId, nLastTime);
				m_pEntity->SendOldTipmsgFormatWithId(tpAddNewTitleTime,ttFlyTip,pConfig->name,pConfig->nNewTitleTime);
				return true;
			}
		}
	}
	return false;
}

bool CNewTitleSystem::addNewTitle( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(wId);
	if ( !pConfig )
	{
		return false;
	}

	SetDataModifyFlag(true);
	if (pConfig->bAutoUse)
	{
		//--自动设置
		SetCurNewTitle(wId);
	}
	
	((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmSelfGetNewTitleNotice, tstUI,pConfig->name);
	if ( IsHaveNewTitle(wId) >= 0 )
	{
		return addNewTitleTime(wId );
	}
	NEWTITLEDATA newData;
	newData.wId = pConfig->nNewTitleId;
	unsigned int nTime = 0;
	if (pConfig->nNewTitleTime > 0 )
	{
		nTime = GetGlobalLogicEngine()->getMiniDateTime() + pConfig->nNewTitleTime;
	}
	newData.nRemainTime = nTime;
	m_NewTitleDataList.add(newData);
	SendAddNewTitleId(wId,pConfig->nNewTitleTime);
	
	((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	if(pConfig->bBroadcast)
	{
		char buff[256];
		sprintf(
			buff,
			GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGetNewTitleNotice),
			m_pEntity->GetEntityName(),
			pConfig->name
			);
		GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(buff,tstRevolving);
	}
	return true;
}

bool CNewTitleSystem::addCustomTitle( unsigned int wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(wId);
	if ( !pConfig )
	{
		return false;
	}

	SetDataModifyFlag(true);
	if (pConfig->bAutoUse)
	{
		//--自动设置
		SetCurCustomTitle(wId);
	}
	
	((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmSelfGetNewTitleNotice, tstUI,pConfig->name);
	if ( IsHaveCustomTitle(wId) >= 0 )
	{
		return addCustomTitleTime(wId );
	}
	NEWTITLEDATA newData;
	newData.wId = pConfig->nNewTitleId;
	unsigned int nTime = 0;
	if (pConfig->nNewTitleTime > 0 )
	{
		nTime = GetGlobalLogicEngine()->getMiniDateTime() + pConfig->nNewTitleTime;
	}
	newData.nRemainTime = nTime;
	m_CustomTitleDataList.add(newData);
	SendAddCustomTitleId(wId,pConfig->nNewTitleTime);
	
	((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	if(pConfig->bBroadcast)
	{
		char buff[256];
		sprintf(
			buff,
			GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGetNewTitleNotice),
			m_pEntity->GetEntityName(),
			pConfig->name
			);
		GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(buff,tstRevolving);
	}
	return true;
}

bool CNewTitleSystem::delNewTitle( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	int nIndex = IsHaveNewTitle(wId);
	if ( nIndex < 0 )
	{
		return false;
	}
	m_NewTitleDataList.remove(nIndex);
	int nCurNewTitle = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CURNEWTITLE);
	if ( nCurNewTitle == wId )
	{
		SetCurNewTitle(0);
	}
	((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	SendDelNewTitleId(wId);
	SetDataModifyFlag(true);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	return true;
}

bool CNewTitleSystem::delCustomTitle( unsigned int wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	int nIndex = IsHaveCustomTitle(wId);
	if ( nIndex < 0 )
	{
		return false;
	}
	m_CustomTitleDataList.remove(nIndex);
	int nCurCustomTitle = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CURCUSTOMTITLE);
	if ( nCurCustomTitle == wId )
	{
		SetCurCustomTitle(0);
	}
	((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	SendDelCustomTitleId(wId);
	SetDataModifyFlag(true);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	return true;
}

void CNewTitleSystem::SetCurNewTitle( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	if(wId !=0)		//设置0是没有头衔
	{
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(wId);
		if ( !pConfig )
		{
			return;
		}
	}
	CActor* pActor = (CActor*)m_pEntity;
	pActor->SetProperty<unsigned int>(PROP_ACTOR_CURNEWTITLE, wId);
}

void CNewTitleSystem::SetCurCustomTitle( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	if(wId !=0)		//设置0是没有头衔
	{
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(wId);
		if ( !pConfig )
		{
			return;
		}
	}
	CActor* pActor = (CActor*)m_pEntity;
	pActor->SetProperty<unsigned int>(PROP_ACTOR_CURCUSTOMTITLE, wId);
}

void CNewTitleSystem::SendAllNewTitleData()
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendNewTitleData;
	INT_PTR nCount = m_NewTitleDataList.count();
	DataPacket << (int)nCount;
	for (int i = 0; i < nCount; i++)
	{
		NEWTITLEDATA* tempData = &m_NewTitleDataList[i];
		if ( tempData )
		{
			DataPacket << (WORD)tempData->wId;
			unsigned int nLastTime = tempData->nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
			nLastTime = nLastTime > 0 ? nLastTime : 0;
			DataPacket << (unsigned int)nLastTime;
		}
	}
	AP.flush();
}

void CNewTitleSystem::SendAllCustomTitleData()
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendCustomTitleData;
	INT_PTR nCount = m_CustomTitleDataList.count();
	DataPacket << (int)nCount;
	for (int i = 0; i < nCount; i++)
	{
		NEWTITLEDATA* tempData = &m_CustomTitleDataList[i];
		if ( tempData )
		{
			DataPacket << (WORD)tempData->wId;
			unsigned int nLastTime = tempData->nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
			nLastTime = nLastTime > 0 ? nLastTime : 0;
			DataPacket << (unsigned int)nLastTime;
		}
	}
	AP.flush();
}

void CNewTitleSystem::SendAddTimeTitleId(WORD wId, int nTime)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendAddTimeTitleData;
	DataPacket << (WORD)wId;
	DataPacket << (int)nTime;
	AP.flush();
}

void CNewTitleSystem::SendAddTimeCustomTitleId(WORD wId, int nTime)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendAddTimeCustomTitleData;
	DataPacket << (WORD)wId;
	DataPacket << (int)nTime;
	AP.flush();
}

void CNewTitleSystem::SendAddNewTitleId(WORD wId, int nTime)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return; 
    } 
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendAddNewTitleData;
	DataPacket << (WORD)wId;
	DataPacket << (int)nTime;
	AP.flush();
}

void CNewTitleSystem::SendAddCustomTitleId(WORD wId, int nTime)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendAddCustomTitleData;
	DataPacket << (WORD)wId;
	DataPacket << (int)nTime;
	AP.flush();
}

void CNewTitleSystem::SendDelNewTitleId(WORD wId)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendDelNewTitleData;
	DataPacket << (WORD)wId;
	AP.flush();
}

void CNewTitleSystem::SendDelCustomTitleId(WORD wId)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enNewTitleSystemID << (BYTE)sSendDelCustomTitleData;
	DataPacket << (WORD)wId;
	AP.flush();
}

void CNewTitleSystem::CalcAttributes( CAttrCalc &calc )
{
	if( !m_pEntity || m_pEntity->GetType() != enActor ) 
	{	
		return;
	}
	INT_PTR nCount = m_NewTitleDataList.count();
	// if (nCount <= 0)
	// {
	// 	return;
	// }
	for (INT_PTR i=0;i<nCount;i++)
	{
		NEWTITLEDATA* tempData = &m_NewTitleDataList[i];
		if (!tempData)
		{
			return;
		}
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(tempData->wId);
		if (!pConfig)
		{
			return;
		}
		for (int j=0;j<pConfig->nPropCount;j++)
		{
			calc << pConfig->PropList[j];
		}
	}

	nCount = m_CustomTitleDataList.count();
	// if (nCount <= 0)
	// {
	// 	return;
	// }
	for (INT_PTR i=0;i < nCount;i++)
	{
		NEWTITLEDATA* tempData = &m_CustomTitleDataList[i];
		if (!tempData)
		{
			return;
		}
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(tempData->wId);
		if (!pConfig)
		{
			return;
		}
		for (int j=0;j<pConfig->nPropCount;j++)
		{
			calc << pConfig->PropList[j];
		}
	}
}

void CNewTitleSystem::OnNewTitleEvent(BYTE btType, INT_PTR nParam1, INT_PTR nParam2)
{
	/*if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CScriptValueList paramList, retParamList;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return;
	paramList << (CActor*)pActor;
	paramList << (BYTE)btType;
	paramList << nParam1;
	paramList << nParam2;
	if (!pNpc->GetScript().Call("NewTitleEventScript", paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}*/
}

void CNewTitleSystem::OnCustomitleEvent(BYTE btType, INT_PTR nParam1, INT_PTR nParam2)
{
	/*if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CScriptValueList paramList, retParamList;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return;
	paramList << (CActor*)pActor;
	paramList << (BYTE)btType;
	paramList << nParam1;
	paramList << nParam2;
	if (!pNpc->GetScript().Call("NewTitleEventScript", paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}*/
}


void CNewTitleSystem::SendClientTitleInfo(CDataPacket& data)
{
	INT_PTR nDataCount = m_NewTitleDataList.count();
	// 跨服服务器	不发送跨服隐藏称号
	if ( GetLogicServer()->IsCrossServer() )
	{
		int nCount = 0;
		for (int i = 0; i < nDataCount; i++)
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(m_NewTitleDataList[i].wId);
			if ( pConfig && pConfig && !pConfig->bCrossServerHide )
			{
				nCount++;
			}
		}

		data << int(nCount);
		for (int i = 0; i < nDataCount; i++)
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(m_NewTitleDataList[i].wId);
			if ( pConfig && pConfig && !pConfig->bCrossServerHide )
			{
				data << (unsigned int)m_NewTitleDataList[i].wId;
				unsigned int nLastTime = m_NewTitleDataList[i].nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
				nLastTime = nLastTime > 0 ? nLastTime : 0;
				data << (unsigned int)nLastTime;
			}
		}
	} 
	else 
	{
		data << int(nDataCount);
		for (int i=0; i<nDataCount;i++)
		{
			NEWTITLEDATA* newTitleData = &m_NewTitleDataList[i];
			if ( newTitleData )
			{
				data << (unsigned int)newTitleData->wId;
				unsigned int nLastTime = newTitleData->nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
				nLastTime = nLastTime > 0 ? nLastTime : 0;
				data << (unsigned int)nLastTime;
			}
		}
	}
}

void CNewTitleSystem::SendClientCustomTitleInfo(CDataPacket& data)
{
	INT_PTR nDataCount = m_CustomTitleDataList.count();
	// 跨服服务器	不发送跨服隐藏称号
	if ( GetLogicServer()->IsCrossServer() )
	{
		int nCount = 0;
		for (int i = 0; i < nDataCount; i++)
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(m_CustomTitleDataList[i].wId);
			if ( pConfig && pConfig && !pConfig->bCrossServerHide )
			{
				nCount++;
			}
		}

		data << int(nCount);
		for (int i = 0; i < nDataCount; i++)
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(m_CustomTitleDataList[i].wId);
			if ( pConfig && pConfig && !pConfig->bCrossServerHide )
			{
				data << (unsigned int)m_CustomTitleDataList[i].wId;
				unsigned int nLastTime = m_CustomTitleDataList[i].nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
				nLastTime = nLastTime > 0 ? nLastTime : 0;
				data << (unsigned int)nLastTime;
			}
		}
	} 
	else 
	{
		data << int(nDataCount);
		for (int i=0; i<nDataCount;i++)
		{
			NEWTITLEDATA* newTitleData = &m_CustomTitleDataList[i];
			if ( newTitleData )
			{
				data << (unsigned int)newTitleData->wId;
				unsigned int nLastTime = newTitleData->nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
				nLastTime = nLastTime > 0 ? nLastTime : 0;
				data << (unsigned int)nLastTime;
			}
		}
	}
}

//跨服数据
void CNewTitleSystem::SendMsg2CrossServer(int nType)
{
	if(m_pEntity ==NULL) return;
	
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = m_pEntity->GetId();
	OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "),nType,nActorID);
	packet << nServerId << nActorID;

	packet <<(BYTE)nType;
	packet << m_pEntity->GetCrossActorId();
	packet << m_pEntity->GetAccountID();
	//数据
	size_t count = m_NewTitleDataList.count();

	int nCount = 0;
	for (int i = 0; i < count; i++)
	{
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(m_NewTitleDataList[i].wId);
		if ( pConfig && pConfig && !pConfig->bCrossServerHide )
		{
			nCount++;
		}
	}

	packet << int(nCount);
	if ( nCount > 0 )
	{
		for (int i = 0; i < count; i++)
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(m_NewTitleDataList[i].wId);
			if ( pConfig && pConfig && !pConfig->bCrossServerHide )
			{
				unsigned int nMakeLong = MAKELONG(m_NewTitleDataList[i].wId,MAKEWORD(0,0));
				packet <<(unsigned int) (nMakeLong);
				packet << (unsigned int)(m_NewTitleDataList[i].nRemainTime);
			}
		}
	}

	//数据
	count = m_CustomTitleDataList.count();

	nCount = 0;
	for (int i = 0; i < count; i++)
	{
		OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(m_CustomTitleDataList[i].wId);
		if ( pConfig && pConfig && !pConfig->bCrossServerHide )
		{
			nCount++;
		}
	}

	packet << int(nCount);
	if ( nCount > 0 )
	{
		for (int i = 0; i < count; i++)
		{
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(m_CustomTitleDataList[i].wId);
			if ( pConfig && pConfig && !pConfig->bCrossServerHide )
			{
				unsigned int nMakeLong = MAKELONG(m_CustomTitleDataList[i].wId,MAKEWORD(0,0));
				packet <<(unsigned int) (nMakeLong);
				packet << (unsigned int)(m_CustomTitleDataList[i].nRemainTime);
			}
		}
	}
	pCrossClient->flushProtoPacket(packet);
}


VOID CNewTitleSystem::OnCrossInitData(std::vector<CNewTitleSystem::NEWTITLEDATA>& titles, std::vector<CNewTitleSystem::NEWTITLEDATA>& CustomTitles)
{	
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("OnCrossInitData Data is Error..."));
		return;
	}

	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return NewTitle repeated,actorid=%u "),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}
		m_NewTitleDataList.clear();
		int nCount = titles.size();
		// reader >> nCount;
		int nMax = 0;
		for (int i=0;i<nCount;i++)
		{
			// unsigned int nValue = 0,  nTime = 0;
			// reader >> nValue >> nTime;
			// WORD wId = LOWORD(nValue);
			NEWTITLEDATA newTitleData = titles[i];
			// newTitleData.wId = wId;
			// newTitleData.nRemainTime = nTime;
			OutputMsg(rmTip,_T("OnCrossInitData CNewTitleSystem nActorID:%d wId:%d"),
						m_pEntity->GetId(),newTitleData.wId);
			m_NewTitleDataList.add(newTitleData);
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(newTitleData.wId);
			if (pConfig && pConfig->nNoticeLv != 0)
			{
				
				if(pConfig->nNoticeLv == nMax)
					m_nMaxTips.push_back(newTitleData.wId);
				
				if(pConfig->nNoticeLv > nMax){
					m_nMaxTips.clear();
					m_nMaxTips.push_back(newTitleData.wId);
					nMax = pConfig->nNoticeLv;
				}
			}
		}

		m_CustomTitleDataList.clear();
		nCount = CustomTitles.size();
		// reader >> nCount2;
		nMax = 0;
		for (int i=0;i < nCount;i++)
		{
			// unsigned int nValue = 0,  nTime = 0;
			// reader >> nValue >> nTime;
			// WORD wId = LOWORD(nValue);
			NEWTITLEDATA newTitleData = CustomTitles[i];
			// newTitleData.wId = wId;
			// newTitleData.nRemainTime = nTime;
			OutputMsg(rmTip,_T("OnCrossInitData CNewTitleSystem nActorID:%d wId:%d"),
						m_pEntity->GetId(),newTitleData.wId);
			m_CustomTitleDataList.add(newTitleData);
			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(newTitleData.wId);
			if (pConfig && pConfig->nNoticeLv != 0)
			{
				
				if(pConfig->nNoticeLv == nMax)
					m_nMaxCustomTips.push_back(newTitleData.wId);
				
				if(pConfig->nNoticeLv > nMax){
					m_nMaxCustomTips.clear();
					m_nMaxCustomTips.push_back(newTitleData.wId);
					nMax = pConfig->nNoticeLv;
				}
			}
		}
		OnDbInitData();
		SetDataModifyFlag(true);
		((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CTITLE);
	}
}