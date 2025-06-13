#include "StdAfx.h"
#include "../base/Container.hpp"
CReviveDurationSystem::CReviveDurationSystem()
{
	m_ReviveDurationDataList.clear();
}


CReviveDurationSystem::~CReviveDurationSystem()
{
}

bool CReviveDurationSystem::Initialize(void *data,SIZE_T size)
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

void CReviveDurationSystem::Destroy()
{
}

VOID CReviveDurationSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor) 
	{
		return;
	}
	INT_PTR nCount = m_ReviveDurationDataList.count();
	if ( nCount <= 0 )
	{
		return;
	}
	
	//已拥有永久复活特权
	if (-1 == m_pEntity->GetProperty<int>(PROP_ACTOR_CURREVIVEDURATION) )
	{
		return;
	}

	unsigned int nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime(); //当前的时间
	for(INT_PTR i= nCount -1 ; i>=0 ; i--)
	{
		ReviveDurationDATA* tempData = &m_ReviveDurationDataList[i];
		if (!tempData)
		{
			return;
		}

		//更新持续时间复活特权
		bool boTime = GetLogicServer()->GetDataProvider()->GetReviveDurationsConfig().IsTimeReviveDuration(tempData->wId);
		if ( boTime )
		{
			if (tempData->nRemainTime <= nCurrentTime)
			{
				DelReviveDuration(tempData->wId);
			}
		} else {
			m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, -1);
		}
	}
}


VOID CReviveDurationSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader)
{	
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("LoadReviveDuration Data is Error..."));
		return;
	}
	if ( nCmd == jxInterSrvComm::DbServerProto::dcLoadReviveDurationData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return ReviveDuration repeated,actorid=%u "),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
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

			if( nTime == 0 )
			{
				m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, -1);
			}

			ReviveDurationDATA ReviveDurationData;
			ReviveDurationData.wId = wId;
			ReviveDurationData.nRemainTime = nTime;
			m_ReviveDurationDataList.add(ReviveDurationData);
		}

		if (-1 != m_pEntity->GetProperty<int>(PROP_ACTOR_CURREVIVEDURATION) && nCount > 0)
		{
			m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, m_ReviveDurationDataList[nCount - 1].nRemainTime);
		}

		OnDbInitData();
		
		// ((CActor*)m_pEntity)->OnFinishOneInitStep(13);
	}
	else if ( nCmd == jxInterSrvComm::DbServerProto::dcSaveReviveDurationData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		return;
	}
}

void CReviveDurationSystem::LoadFromDB()
{
	if(!m_pEntity) return;

	if (((CActor*)m_pEntity)->OnGetIsTestSimulator())
	{ 
		return; 
	}
	unsigned int nActorId = m_pEntity->GetId();
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadReviveDurationData);
		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)nActorId;
 
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket); 
	}	
}

void CReviveDurationSystem::SaveToDB()
{
	if (!m_pEntity)
	{
		return;
	}

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (!pDbClient->connected())
	{
		return;
	}
	unsigned int nActorId = m_pEntity->GetId();
	CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveReviveDurationData);
	DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	DataPacket << (unsigned int)nActorId;
	INT_PTR nDataCount = 0;
	int nPos = DataPacket.getPosition();
	DataPacket << int(nDataCount);
	for (int i = 0; i < m_ReviveDurationDataList.count(); i++)
	{
		ReviveDurationDATA* ReviveDurationData = &m_ReviveDurationDataList[i];
		if ( ReviveDurationData )
		{
			OneReviveDurationConfig* pConfig = GetLogicServer()->GetDataProvider()->GetReviveDurationsConfig().GetReviveDurationConfig(ReviveDurationData->wId);
			if (pConfig)
			{
				unsigned int nMakeLong = MAKELONG(ReviveDurationData->wId,MAKEWORD(0,0));
				DataPacket << (unsigned int)nMakeLong << (unsigned int)ReviveDurationData->nRemainTime;
				nDataCount++;
			}
		}
	}
	
	int *ptrPos = (int*)DataPacket.getPositionPtr(nPos);
	*ptrPos = (int)nDataCount;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	
	SetDataModifyFlag(false);
	
}

int CReviveDurationSystem::IsHaveReviveDuration( WORD wId )
{
	for (INT_PTR i = m_ReviveDurationDataList.count() - 1; i >= 0 ; i--)
	{
		ReviveDurationDATA* tempData = &m_ReviveDurationDataList[i];
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

void CReviveDurationSystem::AddReviveDuration( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	OneReviveDurationConfig* pConfig = GetLogicServer()->GetDataProvider()->GetReviveDurationsConfig().GetReviveDurationConfig(wId);
	if ( !pConfig )
	{
		return;
	}

	//已是永久复活特权
	if(-1 == m_pEntity->GetProperty<int>(PROP_ACTOR_CURREVIVEDURATION) )
	{
		return;
	}

	//如果是永久复活特权
	if(pConfig->nReviveDurationTime == 0)
	{
		m_ReviveDurationDataList.clear();
	}

	ReviveDurationDATA newData;
	newData.wId = pConfig->nReviveDurationId;
	unsigned int nTime = 0;
	if (pConfig->nReviveDurationTime > 0 )
	{
		if ( m_ReviveDurationDataList.count() == 0)
		{
			nTime = GetGlobalLogicEngine()->getMiniDateTime() + pConfig->nReviveDurationTime;
		} else {
			nTime = m_ReviveDurationDataList[m_ReviveDurationDataList.count() - 1].nRemainTime  + pConfig->nReviveDurationTime;
		}
	}
	newData.nRemainTime = nTime;
	m_ReviveDurationDataList.add(newData);

	//更新持续时间复活特权
	if( m_ReviveDurationDataList[m_ReviveDurationDataList.count() - 1].nRemainTime )
	{
		m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, m_ReviveDurationDataList[m_ReviveDurationDataList.count() - 1].nRemainTime);
	} 
	else //更新永久复活特权
	{
		m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, -1);
	}
	
	// ((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	SetDataModifyFlag(true);
}

void CReviveDurationSystem::DelReviveDuration( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	int nIndex = IsHaveReviveDuration(wId);
	if ( nIndex < 0 )
	{
		return;
	}

	m_ReviveDurationDataList.remove(nIndex);
	//更新复活特权持续时间
	if (m_ReviveDurationDataList.count() == 0)
	{
		m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, 0);
		// ((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
		SetDataModifyFlag(true);
	}
}




//跨服数据
void CReviveDurationSystem::SendMsg2CrossServer(int nType)
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
	size_t count = m_ReviveDurationDataList.count();
	packet << (int)count  ; //玩家物品的数量
	if(count > 0)
	{
		for(int i = 0; i < count; i++) {
			unsigned int nMakeLong = MAKELONG(m_ReviveDurationDataList[i].wId,MAKEWORD(0,0));
			packet <<(unsigned int) (nMakeLong);
			packet << (unsigned int)(m_ReviveDurationDataList[i].nRemainTime);
		}
	}
	pCrossClient->flushProtoPacket(packet);
}


VOID CReviveDurationSystem::OnCrossInitData(std::vector<CReviveDurationSystem::ReviveDurationDATA>& titles)
{	
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("OnCrossInitData Data is Error..."));
		return;
	}

	{
		m_ReviveDurationDataList.clear();
		int nCount = titles.size();
		// reader >> nCount;
		int nMax = 0;
		for (int i=0;i<nCount;i++)
		{
			ReviveDurationDATA newTitleData = titles[i];
			m_ReviveDurationDataList.add(newTitleData);
			if( newTitleData.nRemainTime == 0 )
			{
				m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, -1);
			}
		}

		if (-1 != m_pEntity->GetProperty<int>(PROP_ACTOR_CURREVIVEDURATION) && nCount > 0)
		{
			m_pEntity->SetProperty<int>(PROP_ACTOR_CURREVIVEDURATION, m_ReviveDurationDataList[nCount - 1].nRemainTime);
		}
		OnDbInitData();
		((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CRELIVE);
	}
}