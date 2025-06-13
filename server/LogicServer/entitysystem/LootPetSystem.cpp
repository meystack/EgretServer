#include "StdAfx.h"
#include "../base/Container.hpp"
CLootPetSystem::CLootPetSystem()
{
	isInit = false;
	m_vLootPets.clear();
}


CLootPetSystem::~CLootPetSystem()
{

}

bool CLootPetSystem::Initialize(void *data,SIZE_T size)
{
	LoadFromDB();
	return true;
}

void CLootPetSystem::Destroy()
{
}

VOID CLootPetSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor) 
	{
		return;
	}
	INT_PTR nCount = m_vLootPets.size();
	if ( nCount <= 0 )
	{
		return;
	}
	unsigned int nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime(); //当前的时间
	for(INT_PTR i= nCount -1 ; i>=0 ; i--)
	{
		auto tempData = &m_vLootPets[i];
		if (!tempData)
		{
			return;
		}
        if(tempData->nContinueTime >= 0 && tempData->nContinueTime <= nCurrentTime)
        {
			delLootPet(tempData->wId);
		}
	}
}
void CLootPetSystem::OnEnterGame()
{
	if(!m_pEntity) return;


	int nPetId = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_LOOT_PET_ID);
	if (IsHaveLootPet(nPetId) < 0)
	{
		SetCurLootPet(0);
	}
}

VOID CLootPetSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader)
{	
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("LootPet Data is Error..."));
		return;
	}
	if ( nCmd == jxInterSrvComm::DbServerProto::dcLoadLootPetData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return LootPet repeated,actorid=%u "),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}
		
		int nCount = 0;
		reader >> nCount;
		int nMax = 0;
		for (int i=0;i<nCount;i++)
		{
            WORD wId = 0;
            WORD wType = 0;
            int nTime = 0;
			reader >> wType >> wId >> nTime;
			LOOTPETDATA pet;
			pet.wId = wId;
            pet.nType = wType;
			pet.nContinueTime = nTime;
			m_vLootPets.emplace_back(pet);
		}

		// 当所有宠物都不显示时宠物默认捡取距离
		int nLootPetType = INT_MAX;
		if ( m_vLootPets.size() > 0)
		{
			for (size_t i = 0; i < m_vLootPets.size(); i++)
			{
				if ( m_vLootPets[i].nType < nLootPetType)
				{
					nLootPetType = m_vLootPets[i].nType;
					int nDropLootDistance = 0;
					auto cfg  = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(m_vLootPets[i].wId);
					if(cfg) {
						nDropLootDistance = cfg->nLootPetDistance;
					}
					m_pEntity->SetProperty<unsigned long long>(PROP_ACTOR_DEFAULTLOOTPETDIS, nDropLootDistance);
				}
			}
		}

		OnDbInitData();
	}

	((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_LOOTPET_SYSTEM); //完成一个步骤15
}

void CLootPetSystem::LoadFromDB()
{
	if(!m_pEntity) return;

	unsigned int nActorId = m_pEntity->GetId();
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadLootPetData);
		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)nActorId;
		
		int nActorType = m_pEntity->GetHandle().GetType();
		if (nActorType == enActor && ((CActor*)m_pEntity)->OnGetIsTestSimulator())
		{
			return;
		} 
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket); 
	}	
}

void CLootPetSystem::SaveToDB()
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
	CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveLootPetData);
	DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	DataPacket << (unsigned int)nActorId;

	DataPacket << int(m_vLootPets.size());
	for (int i=0; i<m_vLootPets.size();i++)
	{
		auto petData = &m_vLootPets[i];
		if ( petData )
		{
            DataPacket << (WORD)petData->wId <<(WORD)petData->nType << (int)petData->nContinueTime;
		}
	}
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	
	SetDataModifyFlag(false);
	
}

VOID CLootPetSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet)
{

	if(m_pEntity == NULL) return;
	if(m_pEntity->GetHandle().GetType() != enActor) return;
	switch(nCmd)
	{
		case cClientGetAllLootPets:
			{
				SendAllLootPetData();
				break;
			}
		case cClientEquiptLootPet:
			{
				ClientCutOverLootPetId(packet);
				break;
			}
		default:
			break;
	}
}

void CLootPetSystem::ClientCutOverLootPetId(CDataPacketReader &packet)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	WORD wId = 0;
	packet >> wId;
	if (wId != 0 && IsHaveLootPet(wId) < 0)
	{
		return;
	}
	SetCurLootPet(wId);
}

int CLootPetSystem::IsHaveLootPet( WORD wId)
{
	for (INT_PTR i = m_vLootPets.size() - 1; i >= 0 ; i--)
	{
		auto tempData = &m_vLootPets[i];
		if (tempData)
		{
			if ( tempData->wId == wId)
			{
				return (int)i;
			}
		}
	}
	return -1;
}


int CLootPetSystem::IsHaveLootPetType( WORD wType)
{
	for (INT_PTR i = m_vLootPets.size() - 1; i >= 0 ; i--)
	{
		auto tempData = &m_vLootPets[i];
		if (tempData)
		{
			if ( tempData->nType == wType)
			{
				return (int)tempData->wId;
			}
		}
	}
	return -1;
}
bool CLootPetSystem::addLootPetTime( WORD wId )	//头衔续期
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	auto pConfig = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(wId);
	if ( !pConfig  || pConfig->nContinueTime <= 0)
	{
		return false;
	}

	for (INT_PTR i = m_vLootPets.size() - 1; i >= 0 ; i--)
	{
		auto tempData = &m_vLootPets[i];
		if (tempData)
		{
			if (tempData->nType == pConfig->nLootPetType )
			{
				unsigned int nTime = 0;
				tempData->nContinueTime = tempData->nContinueTime + pConfig->nContinueTime;
				SendAddTimeLootPetId(tempData->wId, tempData->nType, tempData->nContinueTime);
				return true;
			}
		}
	}
	return false;
}

bool CLootPetSystem::addLootPet( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	auto pConfig = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(wId);
	if ( !pConfig )
	{
		return false;
	}

	SetDataModifyFlag(true);
	if (pConfig->bAutoUse)
	{
		//--自动设置
		SetCurLootPet(wId);
	}
	
	// ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmSelfGetNewTitleNotice, tstUI,pConfig->name);
	if ( IsHaveLootPetType(pConfig->nLootPetType) >= 0 )
	{
		return addLootPetTime(wId );
	}
	LOOTPETDATA newData;
	newData.wId = pConfig->nLootPetId;
    newData.nType = pConfig->nLootPetType;
	unsigned int nTime = 0;
    newData.nContinueTime = -1;
	if (pConfig->nContinueTime > 0 )
	{
		nTime = GetGlobalLogicEngine()->getMiniDateTime() + pConfig->nContinueTime;
	}
	newData.nContinueTime = nTime;

	if (m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_LOOT_PET_ID) == 0)
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_LOOT_PET_ID, newData.wId);

		//通知周围玩家
		((CActor *)m_pEntity)->GetObserverSystem()->UpdateActorEntityProp();
	}

	m_vLootPets.emplace_back(newData);


	// 当所有宠物都不显示时宠物默认捡取距离
	int nLootPetType = INT_MAX;
	if ( m_vLootPets.size() > 0)
	{
		for (size_t i = 0; i < m_vLootPets.size(); i++)
		{
			if ( m_vLootPets[i].nType < nLootPetType)
			{
				nLootPetType = m_vLootPets[i].nType;
				int nDropLootDistance = 0;
				auto cfg  = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(m_vLootPets[i].wId);
				if(cfg) {
					nDropLootDistance = cfg->nLootPetDistance;
				}
				m_pEntity->SetProperty<unsigned long long>(PROP_ACTOR_DEFAULTLOOTPETDIS, nDropLootDistance);
			}
		}
	}

	SendAddLootPetId(pConfig->nLootPetId, pConfig->nLootPetType, newData.nContinueTime);
	
	((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	return true;
}

bool CLootPetSystem::delLootPet( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	int nIndex = IsHaveLootPet(wId);
	if ( nIndex < 0 )
	{
		return false;
	}

	WORD wType = m_vLootPets[nIndex].nType;

	m_vLootPets.erase(m_vLootPets.begin() + nIndex);

	// 当所有宠物都不显示时宠物默认捡取距离
	int nLootPetType = INT_MAX;
	if ( m_vLootPets.size() > 0)
	{
		for (size_t i = 0; i < m_vLootPets.size(); i++)
		{
			if ( m_vLootPets[i].nType < nLootPetType)
			{
				nLootPetType = m_vLootPets[i].nType;
				int nDropLootDistance = 0;
				auto cfg  = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(m_vLootPets[i].wId);
				if(cfg) {
					nDropLootDistance = cfg->nLootPetDistance;
				}
				m_pEntity->SetProperty<unsigned long long>(PROP_ACTOR_DEFAULTLOOTPETDIS, nDropLootDistance);
			}
		}
	} else if (m_vLootPets.size() == 0)
	{
		m_pEntity->SetProperty<unsigned long long>(PROP_ACTOR_DEFAULTLOOTPETDIS, 0);
	}

	int nPetId = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_LOOT_PET_ID);
	if ( nPetId == wId )
	{
		SetCurLootPet(0);
	}
	((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	SendDelLootPetId(wId, wType);
	SetDataModifyFlag(true);

	ONELOOTPETCONFIG* pOneLootPetConfig = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(wId);
	((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmLootPetMaturity, tstChatSystem, pOneLootPetConfig->sLootPetName);

	return true;
}

void CLootPetSystem::SetCurLootPet( WORD wId )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	pActor->SetProperty<unsigned int>(PROP_ACTOR_LOOT_PET_ID, wId);
	
	//通知周围玩家
	((CActor *)m_pEntity)->GetObserverSystem()->UpdateActorEntityProp();
}

void CLootPetSystem::SendAllLootPetData()
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)(GetSystemID()) << (BYTE)sClientGetAllLootPets;
	INT_PTR nCount = m_vLootPets.size();
	DataPacket << (int)nCount;
	for (int i = 0; i < nCount; i++)
	{
		auto tempData = &m_vLootPets[i];
		if ( tempData )
		{
			DataPacket << (WORD)tempData->wId;
			DataPacket << (WORD)tempData->nType;
            DataPacket << (int)tempData->nContinueTime; 
		}
	}
	AP.flush();
}

void CLootPetSystem::SendAddTimeLootPetId(WORD wId, WORD wType, int nTime)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)GetSystemID() << (BYTE)sSendAddLootPetTimeData;
	DataPacket << (WORD)wId;
	DataPacket << (WORD)wType;
	DataPacket << (int)nTime;
	AP.flush();
}

void CLootPetSystem::SendAddLootPetId(WORD wId, WORD wType, int nTime)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)GetSystemID() << (BYTE)sClientAddLootPet;
	DataPacket << (WORD)wId;
	DataPacket << (WORD)wType;
	DataPacket << (int)nTime;
	AP.flush();
}

void CLootPetSystem::SendDelLootPetId(WORD wId, WORD wType)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)GetSystemID() << (BYTE)sSendDelLootPetData;
	DataPacket << (WORD)wId;
	DataPacket << (WORD)wType;
	AP.flush();
}

void CLootPetSystem::CalcAttributes( CAttrCalc &calc )
{
	if( !m_pEntity || m_pEntity->GetType() != enActor ) 
	{	
		return;
	}
	INT_PTR nCount = m_vLootPets.size();
	if (nCount <= 0)
	{
		return;
	}
	for (INT_PTR i=0;i<nCount;i++)
	{
		auto tempData = &m_vLootPets[i];
		if (!tempData)
		{
			return;
		}
		auto pConfig = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(tempData->wId);
		if (!pConfig)
		{
			return;
		}
		for(int  j = 0; j < pConfig->attri.nCount; j++)
        {
            calc << pConfig->attri.pAttrs[j];
        }
	}
}

//离线
void CLootPetSystem::SendClientOfflineLootPetInfo(CDataPacket& data)
{
	INT_PTR nDataCount = m_vLootPets.size();
	data << int(nDataCount);
	for (int i=0; i<nDataCount;i++)
	{
		auto tmpData = &m_vLootPets[i];
		if ( tmpData )
		{
			data << (int)tmpData->wId;
            data << (int)tmpData->nContinueTime;
			
		}
	}
}
//跨服数据
void CLootPetSystem::SendMsg2CrossServer(int nType)
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
	size_t count = m_vLootPets.size();
	packet << (int)count  ; //玩家物品的数量
	if(count > 0)
	{
		for(int i = 0; i < count; i++) {
			packet <<(WORD) (m_vLootPets[i].wId);
			packet <<(WORD) (m_vLootPets[i].nType);
			packet << (int)(m_vLootPets[i].nContinueTime);
		}
	}
	pCrossClient->flushProtoPacket(packet);
}


VOID CLootPetSystem::OnCrossInitData(std::vector<CLootPetSystem::LOOTPETDATA>& petLists)
{	
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("CLootPetSystem OnCrossInitData Data is Error..."));
		return;
	}

	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return lootPet repeated,actorid=%u "),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}
		m_vLootPets.clear();
		int nCount = petLists.size();
		for (int i=0;i<nCount;i++)
		{
			LOOTPETDATA tmpData = petLists[i];
			OutputMsg(rmTip,_T("OnCrossInitData CLootPetSystem nActorID:%d wId:%d"),
						m_pEntity->GetId(),tmpData.wId);
			m_vLootPets.emplace_back(tmpData);
		}
		OnDbInitData();
		SetDataModifyFlag(true);
		((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_LOOTPET);
	}
}

VOID CLootPetSystem::OnReturnLootPetInfo(CDataPacket &data)
{
	int nCount = m_vLootPets.size();
    data << nCount;
	if(nCount > 0) 
	{
		for (int i = 0; i < nCount; i++)
		{
			data <<(WORD)(m_vLootPets[i].wId);
			data <<(WORD)(m_vLootPets[i].nType);
			data <<(int)(m_vLootPets[i].nContinueTime);
		}
	}
	
}