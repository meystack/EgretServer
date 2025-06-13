#include "StdAfx.h"

void CActorCacheData::PostponeNextSaveTick()
{
	m_nNextSaveTick = _getTickCount() + CDBDataCache::s_nActorCacheSaveInterval;
}

//const TICKCOUNT CActorCacheData::s_nExpireTime = 20; // for test
//const TICKCOUNT CDBDataCache::s_nCheckInterval = 1000; // for test
//TICKCOUNT	CDBDataCache::s_nActorCacheSaveInterval = 5000; // for test
const TICKCOUNT CActorCacheData::s_nExpireTime = 15 * CMiniDateTime::SecOfMin * 1000;
const TICKCOUNT CDBDataCache::s_nCheckInterval = 5 * 1000;
TICKCOUNT	CDBDataCache::s_nActorCacheSaveInterval = 20000;//10 * CMiniDateTime::SecOfMin * 1000;

//s_aActorCacheHandler与s_aCDT2RespSrvCmdID顺序要一一对应
const CDBDataCache::SaveHandler CDBDataCache::s_aActorCacheHandler[] = {	
	&CDBDataCache::SaveActorBasicData,
	&CDBDataCache::SaveBagItemData,
	&CDBDataCache::SaveEquipItemData,
	&CDBDataCache::SaveDepotItemData,
	&CDBDataCache::SavePetItemData,
	&CDBDataCache::SaveSkillData,
	&CDBDataCache::SaveOtherGameSetsData,
	&CDBDataCache::SaveQuestData,
	&CDBDataCache::SaveGuildData,
	&CDBDataCache::SaveMiscData,
	&CDBDataCache::SaveAchieveData,
	&CDBDataCache::SaveScriptData,
	&CDBDataCache::SavePetData,
	&CDBDataCache::SavePetSkillData,
	&CDBDataCache::SavCombatGameInfoData,
	&CDBDataCache::SavCombatRecordData,
	&CDBDataCache::SaveFriendData,
	&CDBDataCache::SavGamesetData,
	&CDBDataCache::SaveActorMailData,
	&CDBDataCache::SaveAlmirahItemData,
	&CDBDataCache::SaveDeathData,
	&CDBDataCache::SaveDeathDropData,
	&CDBDataCache::SaveConsignData,
};

//s_aActorCacheHandler与s_aCDT2RespSrvCmdID顺序要一一对应
const jxSrvDef::INTERSRVCMD CDBDataCache::s_aCDT2RespSrvCmdID[] = {
	dcSave,
	dcSaveBagItem,
	dcSaveEquipItem,
	dcSaveDepotItem,
	dcSaveHeroItem,
	dcSaveSkill,
	dcSaveOtherGameSets,
	dcDefault,		// for quest data!!! no response
	dcSaveActorGuild,
	dcDefault,		// for misc data!!! no response
	dcDefault,		// for achieve data!!! no response
	dcSaveVar,
	dcSaveHeroData,
	dcSaveHeroSkillData,
	dcSaveCombatGameInfo,
	dcLoadCombatRecord,
	dcSaveFriendsData,
	dcSaveGameSetData,
	dcSaveOfflineMail,	
	dcSaveAlmirahItem,
	dcSaveDeathData,
	dcSaveDeathDropData,
	dcSaveActorConsignment,
};

jxSrvDef::INTERSRVCMD CDBDataCache::GetCmdIdByCDT(enActorCacheDataType tp)
{
	if (tp >= 0 && tp < enCT_Max)
		return s_aCDT2RespSrvCmdID[tp];

	return dcDefault;
}

CDBDataCache::CDBDataCache() : m_sACDHandleMgr(_T("ActorCacheDataHandleMgr"))
{
	m_bHasExpireCache = false;
	SetDBClient(NULL, NULL);	
	m_nNextCheckTime = _getTickCount();
	m_listIter.setList(m_list);
}

void CDBDataCache::SetDBClient(ILogicDBRequestHost* pHost, CDBDataClientHandler* handler)
{
	m_pHost = pHost;
	m_pClientHandler = handler;
}

bool CDBDataCache::CacheData(enActorCacheDataType cdType, CDataPacketReader& packet)
{
	if (cdType >= 0 && cdType < enCT_Max)
	{
		unsigned int nActorId = 0;
		int nRawServerId = 0, nLoginServerId = 0;

		switch (cdType)
		{
		case enCT_ActorBasicData:
		case enCT_BagItemData:
		case enCT_EquipItemData:
		case enCT_DepotItemData:
		case enCT_PetItemData:
		case enCT_SkillData:
		case enCT_GameOtherSetData:
		case enCT_QuestData:		
		case enCT_MiscData:
		case enCT_AchieveData:
		case enCT_ScriptData:
		case enCT_PetData:
		case enCT_PetSkillData:
		case enCT_CombatGameData:
		case enCT_CombatRecordData:
		case enCT_FriendData:
		case enCT_DeathData:
		case enCT_DeathDropData:
		case enCT_GameSetData:
		case enCT_MailData:
		case enCT_AlmirahItemData:
		case enCT_SoldierSoulData:
		case enCT_ConsignData:
			nActorId = GetActorId(packet, 8);
			break;
		case enCT_GuildData:
			nActorId = GetActorId(packet, 12);
			break;
		default:
			break;
		}

		if (nActorId > 0)
		{
			CActorCacheDataHandle acdhandle;
			CActorCacheData* pData = GetActorCacheDataByActorId(nActorId);
			if (!pData)
			{
				pData = AllocActorCacheData(nActorId, acdhandle);
				if (pData)
				{
					pData->Init(m_allocator, nActorId);
					pData->PostponeNextSaveTick();					
				}
			}
			
			//pData->SetDirty(true); 修改为每类数据更新标记
			pData->SetDirty(cdType, true);
			CDataPacket& cachePacket = pData->m_sActorCacheData[cdType];
			cachePacket.setPosition(0);
			size_t nLen = packet.getAvaliableLength();
			cachePacket.setLength(nLen);
			cachePacket.writeBuf(packet.getOffsetPtr(), nLen);
			cachePacket.setPosition(0);
		}	

		// Response to request
 		jxSrvDef::INTERSRVCMD respCmdId = GetCmdIdByCDT(cdType);
		if (respCmdId != dcDefault && m_pClientHandler)
		{
			//CDataPacket& respPacket = m_pHost->AllocDataPacket(respCmdId);//m_pDBClient->allocProtoPacket(respCmdId);
			//respPacket << nActorId << (BYTE)reSucc;
			////m_pDBClient->flushProtoPacket(respPacket);
			//m_pHost->FlushDataPacket(respPacket);
		}
	}
	else
	{
		OutputMsg(rmError, _T("CacheData[type=%d] failed"), (int)cdType);
	}

	return true;
}

unsigned int CDBDataCache::GetActorId(CDataPacketReader& packet, INT_PTR nOffset)
{
	unsigned int nActorId = 0;
	if (packet.getAvaliableLength() < sizeof(nActorId) + nOffset)
		return 0;
	INT_PTR nPos = packet.getPosition();
	if (nOffset != 0)
		packet.adjustOffset(nOffset);
	packet >> nActorId;
	packet.setPosition(nPos);
	return nActorId;
}

void CDBDataCache::OnActorLogin(CDataPacketReader& packet)
{
	unsigned int nActorId = GetActorId(packet, 0);
	SaveActorDataImmediately(nActorId, false);
}

void CDBDataCache::OnActorLogout(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	//int nActorId = GetActorId(packet, 0);
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorId = 0;
	bool bNeedAck = false;
	packet >> nRawServerId >> nLoginServerId >> nActorId >> bNeedAck;
	SaveActorDataImmediately(nActorId, false);
	GetLogicDBRequestHandler().UpdateCharOnlineStatus(nActorId, 0);
	if (bNeedAck && m_pHost)
	{
		CDataPacket &retPacket = m_pHost->AllocDataPacket(dcLogout);
		retPacket << nRawServerId << nLoginServerId << nActorId << jxInterSrvComm::DbServerProto::reSucc;
		m_pHost->FlushDataPacket(retPacket);
	}
}

void CDBDataCache::SaveAllActorDataImmediately()
{
	OutputMsg(rmNormal, _T("Prepare to Save all Actor Data"));
	SaveData(false, false);
	OutputMsg(rmNormal, _T("Save all Actor Data End"));
	GetLogicDBRequestHandler().UpdateCharOnlineStatus(0,0);
}

CLogicDBReqestHandler& CDBDataCache::GetLogicDBRequestHandler()
{
	return m_pClientHandler->GetDBRequestHandler();
}

bool CDBDataCache::SaveActorBasicData(CDataPacketReader& packet, bool bResp)
{		
	//m_pDBClient->SaveActorToDB(packet, bResp);
	GetLogicDBRequestHandler().SaveActorToDB(packet, bResp);
	return true;
}

bool CDBDataCache::SaveBagItemData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveItems(packet, bResp);
	GetLogicDBRequestHandler().SaveItems(packet, bResp);
	return true;
}

bool CDBDataCache::SaveEquipItemData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveItems(packet, bResp);
	GetLogicDBRequestHandler().SaveItems(packet, bResp);
	return true;
}

bool CDBDataCache::SaveDepotItemData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveItems(packet, bResp);
	GetLogicDBRequestHandler().SaveItems(packet, bResp);
	return true;
}

bool CDBDataCache::SavCombatGameInfoData( CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorCombatBaseInfo(packet);
	return true;
}

bool CDBDataCache::SavCombatRecordData( CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorCombatLog(packet);
	return true;
}

bool CDBDataCache::SaveFriendData( CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorFriendsData(packet);
	return true;
}

bool CDBDataCache::SaveDeathData( CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorDeathData(packet);
	return true;
}

bool CDBDataCache::SaveDeathDropData( CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorDeathDropData(packet);
	return true;
}

bool CDBDataCache::SavGamesetData(CDataPacketReader& packet,bool bResp)
{
	GetLogicDBRequestHandler().SaveActorGameSetData(packet);
	return true;
}

bool CDBDataCache::SavePetItemData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveItems(packet, bResp);
	GetLogicDBRequestHandler().SaveItems(packet, bResp);
	return true;
}

bool CDBDataCache::SaveSkillData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveSkillToDB(packet, bResp);
	GetLogicDBRequestHandler().SaveSkillToDB(packet, bResp);
	return true;
}

bool CDBDataCache::SaveOtherGameSetsData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveSkillToDB(packet, bResp);
	GetLogicDBRequestHandler().SaveOtherGameSetsToDB(packet, bResp);
	return true;
}

bool CDBDataCache::SaveQuestData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveQuestToDB(packet, bResp);
	GetLogicDBRequestHandler().SaveQuestToDB(packet, bResp);
	return true;
}

bool CDBDataCache::SaveGuildData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveActorGuild(packet, bResp);
	GetLogicDBRequestHandler().SaveActorGuild(packet, bResp);
	return true;
}

bool CDBDataCache::SaveMiscData(CDataPacketReader& packet, bool bResp)
{
	return true;
}

bool CDBDataCache::SaveAchieveData(CDataPacketReader& packet, bool bResp)
{
	return true;
}

bool CDBDataCache::SaveScriptData(CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveScriptDataToDB(packet, bResp);
	return true;
}

bool CDBDataCache::SavePetData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SavePetToDB(packet, bResp);
	GetLogicDBRequestHandler().SavePetToDB(packet, bResp);
	return true;
}

bool CDBDataCache::SavePetSkillData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SavePetSkillData(packet, bResp);
	GetLogicDBRequestHandler().SavePetSkillData(packet, bResp);
	return true;
}

bool CDBDataCache::SaveActorMailData(CDataPacketReader & packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorMail(packet, bResp);
	return true;
}
bool CDBDataCache::SaveConsignData( CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorConsignment(packet);
	return true;
}

bool CDBDataCache::SaveStaticCounts(CDataPacketReader& packet, bool bResp)
{
	GetLogicDBRequestHandler().SaveActorStaticCounts(packet);
	return true;
}

bool CDBDataCache::SaveData(bool bCheckCD, bool bResp, INT_PTR nTimeLmt)
{	
	//DECLARE_FUN_TIME_PROF()
	TICKCOUNT nEndTick = _getTickCount() + nTimeLmt;
	TICKCOUNT nCurrTick = 0;
	if (bCheckCD)
		nCurrTick = _getTickCount();
	if(m_list.count()==0 || m_pClientHandler ==NULL ) return true;

	CBaseLinkedList<CActorCacheData*>::NodeType* pNode = NULL;
	while (pNode = m_listIter.next())
	{
		CActorCacheData* acd = *pNode;
		SaveActorDataImpl(*acd, bCheckCD, nCurrTick, bResp);	
		// if read operation arrived, just pause it
		if (m_pClientHandler && m_pClientHandler->HasReadOPFlag())
		{
			if (_getTickCount() > nEndTick)
			{
				OutputMsg(rmNormal, _T("有查询数据到来，暂停存盘操作"));
				break;
			}
		}
	}

	return true;
}

bool CDBDataCache::SaveActorDataImmediately(unsigned int nActorId, bool bResp)
{	
	std::map<unsigned int, CActorCacheData*>::iterator iter = m_sACDIdxTbl.find(nActorId);
	if (iter == m_sACDIdxTbl.end())
		return true;

	OutputMsg(rmNormal, _T("SaveActorDataImmediately actorid=%u"), nActorId);

	if (CDBDataServer *pDataServer = CDBServer::s_pDBEngine->getDataServer())
	{
		pDataServer->m_ActorIdMapLock.Lock();
		pDataServer->m_ActorIdMap[nActorId] = false;
		pDataServer->m_ActorIdMapLock.Unlock();
	}

	CActorCacheData* acd = iter->second;
	return SaveActorDataImpl(*acd, false, 0, bResp);
}

void CDBDataCache::RemoveExpireCache()
{
	std::map<unsigned int, CActorCacheData*>::iterator iter		= m_sACDIdxTbl.begin();
	std::map<unsigned int, CActorCacheData*>::iterator iter_end	= m_sACDIdxTbl.end();
	for (; iter != iter_end; )
	{
		CActorCacheData* acd = iter->second;
		if (acd->IsExpired())
		{
			OutputMsg(rmNormal, _T("Remove actor[%u] cache data"), iter->first);
			m_sACDIdxTbl.erase(iter++);
			FreeActorCacheData(acd->GetHandle(), false);
		}
		else
		{
			++iter;
		}
	}
}

bool CDBDataCache::SaveActorDataImpl(CActorCacheData& data, bool bCheckCD, TICKCOUNT nCurrTick, bool bResp)
{
	if (!data.IsDirty())
	{
		if (data.m_nNextSaveTick + CActorCacheData::s_nExpireTime <= nCurrTick)
		{
			data.SetExpired(true);
			OutputMsg(rmNormal, _T("SaveActorDataImpl set Actor[actorid=%u] expired"), data.m_nActorId);

			if (!m_bHasExpireCache)
				m_bHasExpireCache = true;
		}
		return true;
	}

	if (!bCheckCD || (bCheckCD && data.CheckAndSet(nCurrTick)) )
	{
		if (data.IsDirty())
		{
			INT_PTR nCount = __min(ArrayCount(s_aActorCacheHandler), enCT_Max);
			OutputMsg(rmNormal, _T("Save Actor[actorid=%u] Data,dirtyflag=%lld"), data.m_nActorId,data.m_sACDDirtyFlag);
			for (INT_PTR i = 0; i < nCount; i++)
			{
				if (data.IsACDDirty((int)i))
				{
					//OutputMsg(rmNormal, _T("Save ACD, type=%d"), (int)i);
					bool bRet = (this->*s_aActorCacheHandler[i])(data.m_sActorCacheData[i], bResp);
					if (!bRet)
					{
						OutputMsg(rmError, _T("SaveData Failed(Actorid=%u, cacheType=%d"), data.m_nActorId, i);
					}
				}
			}

			data.ResetDirty();
		}
	}

	return true;
}

void CDBDataCache::RunOne(TICKCOUNT nCurrTick)
{	

	if (nCurrTick >= m_nNextCheckTime)
	{
		m_nNextCheckTime += s_nCheckInterval;
		SaveData(true, true, 2000);
	}

	if (m_bHasExpireCache)
	{
		RemoveExpireCache();
		m_bHasExpireCache = false;
	}

}


CActorCacheData* CDBDataCache::AllocActorCacheData(unsigned int nActorId, CActorCacheDataHandle &handle)
{
	CActorCacheData* adc = this->m_sACDHandleMgr.Acquire(handle);
	if (adc)
	{		
		adc->SetHandle(handle);
		m_sACDIdxTbl.insert(std::make_pair(nActorId, adc));
		CBaseLinkedList<CActorCacheData*>::NodeType* node = m_list.linkAfter(adc);
		adc->m_node = node;
	}	
	else
	{
		OutputMsg(rmError,"AllocActorCacheData Acquire fail,actorid=%u",nActorId);
	}
	return adc;
}

CActorCacheData* CDBDataCache::GetActorCacheData(const CActorCacheDataHandle& handle)
{
	CActorCacheData* adc = this->m_sACDHandleMgr.GetDataPtr(handle);
	return adc;
}

CActorCacheData* CDBDataCache::GetActorCacheDataByActorId(unsigned int nActorId)
{
	std::map<unsigned int, CActorCacheData*>::iterator iter = m_sACDIdxTbl.find(nActorId);
	if (iter != m_sACDIdxTbl.end())
		return iter->second;
	return NULL;
}

void CDBDataCache::FreeActorCacheData(const CActorCacheDataHandle& handle, bool updateIndex)
{
	CActorCacheData* acd = GetActorCacheData(handle);
	if (!acd)
		OutputMsg(rmError, _T("%s can't find item in index table"), __FUNCTION__);
	
	//OutputMsg(rmNormal, _T("FreeActorCacheData,actorid=%u"),acd->m_nActorId);
	
	if (updateIndex)
	{
		std::map<unsigned int, CActorCacheData*>::iterator iter = m_sACDIdxTbl.find(acd->m_nActorId);
		if (iter != m_sACDIdxTbl.end())
			m_sACDIdxTbl.erase(iter);
	}
	
	
	m_listIter.remove(acd->m_node);
	acd->m_node = 0;
	m_sACDHandleMgr.Release(handle);

	
}

void  CDBDataCache::Trace()
{
	OutputMsg(rmNormal,"---cache start---");
	INT_PTR nSize =0;
	CBaseLinkedList<CActorCacheData*>::NodeType* pNode = NULL;
	while (pNode = m_listIter.next())
	{
		CActorCacheData* acd = *pNode;
		if(acd)
		{
			nSize += acd->Trace();
		}
	}
	
	OutputMsg(rmNormal," actor cache total size=%d",(int)nSize);

	OutputMsg(rmNormal,"CDBDataCache CBufferAllocator");
	CBufferAllocator::ALLOCATOR_MEMORY_INFO memoryInfo;
	m_allocator.GetMemoryInfo(&memoryInfo);
	OutputMsg(rmNormal,"super alloc=%d,free=%d",(int)memoryInfo.SuperBuffer.dwAllocSize,(int)memoryInfo.SuperBuffer.dwFreeSize);
	OutputMsg(rmNormal,"large alloc=%d,free=%d",(int)memoryInfo.LargeBuffer.dwAllocSize,(int)memoryInfo.LargeBuffer.dwFreeSize);
	OutputMsg(rmNormal,"middle alloc=%d,free=%d",(int)memoryInfo.MiddleBuffer.dwAllocSize,(int)memoryInfo.MiddleBuffer.dwFreeSize);
	OutputMsg(rmNormal,"small alloc=%d,free=%d",(int)memoryInfo.SmallBuffer.dwAllocSize,(int)memoryInfo.SmallBuffer.dwFreeSize);
	OutputMsg(rmNormal,"---cache end---");

}

bool CDBDataCache::SaveAlmirahItemData(CDataPacketReader& packet, bool bResp)
{
	//m_pDBClient->SaveItems(packet, bResp);
	GetLogicDBRequestHandler().SaveAlmirahItem(packet, bResp);
	return true;
}
