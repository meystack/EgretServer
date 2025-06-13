#include "StdAfx.h"
#include "CombatSystem.h"
#include "../base/Container.hpp"
CCombatSystem::CCombatSystem()
{
	m_nJobType = 0;
	m_nMaxRankLevel = 0;
	m_nLastRankLevel = 0;
	m_nCurrentRankLevel = 0;
	m_nCombatScore = 0;
	m_nChallengeCount = 0;
	m_nPickCount = 0;
	m_nClearTime = 0;
	m_bCdFlag = false;
	m_nSelectedIndex[0] = 0;
	m_bPickSwitch = true;
	m_bSaveInfoFlag = false;
	m_bSaveLogFlag = false;
}

bool CCombatSystem::Initialize(void *data,SIZE_T size)
{
	// PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	// if(pActorData ==NULL || size != sizeof(ACTORDBDATA))
	// {
	// 	//OutputMsg(rmWaning,_T("创建角色的数据长度错误，帮派系统初始化失败"));
	// 	return false;
	// }

	return true;
}


void CCombatSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader)
{
	// if(m_pEntity ==NULL) return;
	// switch(nCmd)
	// {
	// 	case jxInterSrvComm::DbServerProto::dcLoadCmobatGameInfo:
	// 		{
	// 			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	// 			{
	// 				LoadCombatInfoFromDbResult(reader);
	// 			}
	// 			break;
	// 		}
	// 	case jxInterSrvComm::DbServerProto::dcLoadCombatRecord:
	// 		{
	// 			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	// 			{
	// 				LoadCombatRecordFromDbResult(reader);
	// 			}
	// 			break;
	// 		}
	// 	default:
	// 		{
	// 			break;
	// 		}
	// }
}

void CCombatSystem::Save(PACTORDBDATA pData)
{
	// if (m_pEntity == NULL ) return;

	// if(m_bSaveInfoFlag)
	// {
	// 	m_bSaveInfoFlag = false;
	// 	SaveCombatGameInfo();
	// }

	// if(m_bSaveLogFlag)
	// {
	// 	m_bSaveLogFlag = false;
	// 	SaveCombatRecord();
	// }
}

void CCombatSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// if (false == ((CActor*)m_pEntity)->CheckLevel(nCombatConfig.m_CombatRankConfig.nLevLimit, nCombatConfig.m_CombatRankConfig.nCircleLimit))
	// {
	// 	return;
	// }

	// UpdateCombatRanking();
}

void CCombatSystem::OnEnterGame()
{
// 	m_nJobType = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);

// 	LoadCombatInfoFromDb();
// 	LoadCombatRecordFromDb();
}

void CCombatSystem::LoadCombatInfoFromDb()
{
	// if(!m_pEntity) return;

	// int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);

	// CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	// if (pDbClient->connected())
	// {
	// 	CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCmobatGameInfo);
	// 	DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	// 	DataPacket << (int)nActorId;
	// 	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	// }
}

void CCombatSystem::SaveCombatGameInfo()
{
	// if(!m_pEntity) return;

	// int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);

	// CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	// if (pDbClient->connected())
	// {
	// 	CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveCombatGameInfo);
	// 	DataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	// 	DataPacket << nActorId;
	// 	DataPacket << m_nCurrentRankLevel;
	// 	DataPacket << m_nMaxRankLevel;
	// 	DataPacket << m_nLastRankLevel;
	// 	DataPacket << m_nCombatScore;
	// 	DataPacket << m_nChallengeCount;
	// 	DataPacket << m_nPickCount;
	// 	DataPacket << m_nClearTime;
	// 	DataPacket << (int)m_bCdFlag;

	// 	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	// }
}

void CCombatSystem::LoadCombatInfoFromDbResult(CDataPacketReader &reader)
{
	// int nChallengeCount = 0;
	// int nPickCount = 0;
	// reader >> m_nCurrentRankLevel;
	// reader >> m_nMaxRankLevel;
	// reader >> m_nLastRankLevel;
	// reader >> m_nCombatScore;
	// reader >> nChallengeCount;
	// reader >> nPickCount;
	// reader >> m_nClearTime;
	// int nCdFalg = 0;
	// reader >> nCdFalg;
	// m_bCdFlag = nCdFalg > 0;

	// if (!m_bSaveInfoFlag)
	// {
	// 	m_nChallengeCount = nChallengeCount;
	// 	m_nPickCount = nPickCount;
	// }

	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);
	// if(pRanking)
	// {
	// 	int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);
	// 	INT_PTR nCurIndex = 0;
	// 	CRankingItem* pRItem = pRanking->GetPtrFromId(nActorId, nCurIndex);
	// 	m_nCurrentRankLevel = (int)nCurIndex + 1;
	// 	if (m_nMaxRankLevel <= 0 || m_nMaxRankLevel > m_nCurrentRankLevel)
	// 	{
	// 		m_nMaxRankLevel = m_nCurrentRankLevel;
	// 	}
	// }
}


void CCombatSystem::LoadCombatRecordFromDb()
{
	// if(!m_pEntity) return;

	// int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);

	// CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	// if (pDbClient->connected())
	// {
	// 	CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCombatRecord);
	// 	DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	// 	DataPacket << (int)nActorId;
	// 	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	// }
}

void CCombatSystem::SaveCombatRecord()
{
	// if(!m_pEntity) return;

	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// int nCount = nCombatConfig.m_CombatRankConfig.nMaxChallangeRecord;
	// if (nCount > m_CombatRecordList.count())
	// {
	// 	nCount = (int)m_CombatRecordList.count();
	// }

	// if(nCount <= 0)
	// {
	// 	return;
	// }

	// int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);
	// CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	// if (pDbClient->connected())
	// {
	// 	CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveCombatRecord);

	// 	DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	// 	DataPacket << (int)nActorId;
	// 	DataPacket << (int)nCount;

	// 	for (int i = (int)m_CombatRecordList.count() - nCount; i < m_CombatRecordList.count(); ++i)
	// 	{
	// 		COMBATRECORD& nCombatRecord = m_CombatRecordList[i];
	// 		DataPacket << nCombatRecord.nType;
	// 		DataPacket << nCombatRecord.nResult;
	// 		DataPacket << nCombatRecord.nActorId;
	// 		DataPacket.writeString(nCombatRecord.strName); 
	// 		DataPacket << nCombatRecord.nRank;
	// 	}	

	// 	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	// }
}


void CCombatSystem::LoadCombatRecordFromDbResult(CDataPacketReader &reader)
{
	// int nCount = 0;
	// reader >> nCount;

	// if(nCount>0)
	// {
	// 	m_CombatRecordList.reserve(nCount);
	// 	m_CombatRecordList.trunc(nCount);

	// 	for (int i = 0; i < nCount; i++)
	// 	{
	// 		COMBATRECORD& nCombatRecord = m_CombatRecordList[i];
	// 		reader >> nCombatRecord.nType;
	// 		reader >> nCombatRecord.nResult;
	// 		reader >> nCombatRecord.nActorId;
	// 		reader.readString(nCombatRecord.strName, ArrayCount(nCombatRecord.strName));
	// 		reader >> nCombatRecord.nRank;
	// 	}
	// }
}


void CCombatSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	// if (NULL == m_pEntity || ((CEntity*)m_pEntity)->GetType() != enActor)
	// {
	// 	return;
	// }
	// //等级限制
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// if (false == ((CActor*)m_pEntity)->CheckLevel(nCombatConfig.m_CombatRankConfig.nLevLimit, nCombatConfig.m_CombatRankConfig.nCircleLimit))
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpCombatLevelLimit, ttFlyTip, nCombatConfig.m_CombatRankConfig.nLevLimit);
	// 	return;
	// }

	// switch (nCmd)
	// {
	// case cGetCombatInfo:
	// 	{
	// 		SendCombatBaseInfo();
	// 		break;
	// 	}
	// case cSelectCombatPayer:
	// 	{
	// 		int nActive;
	// 		packet >> nActive;
	// 		PickRankPlayer(nActive);
	// 		break;
	// 	}
	// case cCombatStart:
	// 	{
	// 		int nActorId, nRank;
	// 		packet >> nActorId;
	// 		packet >> nRank;
	// 		StartCombatChallage(nActorId, nRank);
	// 		((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtCombatCount, 0, 1, true);
	// 		break;
	// 	}
	// case cClearCooldown:
	// 	{
	// 		ClearCombatCd();
	// 		break;
	// 	}
	// case cRechargeAwrad:
	// 	{
	// 		int nAwardId;
	// 		packet >> nAwardId;
	// 		RechargeRankAward(nAwardId);
	// 		break;
	// 	}
	// case cGetCombatLog:
	// 	{
	// 		GetCombatLog();
	// 		break;
	// 	}
	// case cGetCombatRank:
	// 	{
	// 		int nCurrentPage;
	// 		packet >> nCurrentPage;
	// 		GetCombatRank(nCurrentPage);
	// 		break;
	// 	}
	// case cGetShopInfo:
	// 	{
	// 		GetRankAwardList();
	// 		break;
	// 	}
	// default:
	// 	{
	// 		break;
	// 	}
	// }
}


void CCombatSystem::SendCombatBaseInfo()
{
	// //下发基本信息
	// CActorPacket AP;
	// CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	// DataPacket << (BYTE)enCombatSystemID << (BYTE)sSendCombatInfo;

	// DataPacket << m_nCurrentRankLevel;
	// DataPacket << m_nMaxRankLevel;
	// DataPacket << m_nLastRankLevel;
	// DataPacket << m_nCombatScore;
	// DataPacket << m_nChallengeCount;
	// DataPacket << m_nPickCount;
	// DataPacket << m_nClearTime;
	// DataPacket << (BYTE)m_bCdFlag;

	// AP.flush();
}


void CCombatSystem::PickRankPlayer(int nActive)
{
	// //玩家主动请求
	// if (nActive > 0)
	// {
	// 	CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// 	if (m_nPickCount >= nCombatConfig.m_CombatRankConfig.nMaxPickTimes)
	// 	{
	// 		m_pEntity->SendOldTipmsgWithId(tpCombatPickCountLimit, ttFlyTip);
	// 		return;
	// 	}

	// 	m_nPickCount++;
	// 	m_bSaveInfoFlag = true;
	// 	SendCombatBaseInfo();

	// 	m_bPickSwitch = false;
	// 	PickRankPlayerImpl();
	// }
	// else if (m_bPickSwitch)
	// {
	// 	m_bPickSwitch = false;
	// 	PickRankPlayerImpl();
	// }

	// SendPickRankPlayer();
}

void CCombatSystem::StartCombatChallage(int nActorId, int nRank)
{
	// if (nActorId == m_pEntity->GetProperty<int>(PROP_ENTITY_ID))
	// {
	// 	return;
	// }

	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);
	// if(pRanking)
	// {
	// 	INT_PTR nCurIndex = 0;
	// 	CRankingItem* nItem = pRanking->GetPtrFromId(nActorId, nCurIndex);
	// 	if (nRank != nCurIndex + 1)
	// 	{
	// 		PickRankPlayerImpl();
	// 		SendPickRankPlayer();
	// 		return;
	// 	}

	// 	int nTotalTimes = nCombatConfig.m_CombatRankConfig.nMaxChallageTimes;
	// 	int nCdTime = nCombatConfig.m_CombatRankConfig.nAddCD;
	// 	unsigned int nIntervel = nCombatConfig.m_CombatRankConfig.nMaxCD;

	// 	unsigned int nTime =  GetGlobalLogicEngine()->getMiniDateTime();
	// 	if (m_bCdFlag)
	// 	{
	// 		if (m_nClearTime > nTime)
	// 		{
	// 			m_pEntity->SendOldTipmsgFormatWithId(tpCombatCDLimit, ttFlyTip);
	// 			return;
	// 		}
	// 		else
	// 		{
	// 			m_nClearTime = nTime + nCdTime;
	// 			m_bCdFlag = false;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if (m_nClearTime > nTime)
	// 		{
	// 			m_nClearTime += nCdTime;
	// 			if (m_nClearTime - nTime >= nIntervel)
	// 			{
	// 				m_bCdFlag = true;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			m_nClearTime = nTime + nCdTime;
	// 		}
	// 	}

	// 	m_nChallengeCount++;
	// 	m_bSaveInfoFlag = true;

	// 	TriggerEvent(OpCombatStart, nActorId);
	// }
}

void CCombatSystem::ClearCombatCd() 
{
	// unsigned int nTime =  GetGlobalLogicEngine()->getMiniDateTime();
	// if (!m_bCdFlag || m_nClearTime <= nTime)
	// {
	// 	return;
	// }

	// int nMoney = (m_nClearTime - nTime + 59) / 60;
	// //元宝不够
	// if(m_pEntity->GetProperty<int>(PROP_ACTOR_YUANBAO) < nMoney)
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpNomoreYubao,ttFlyTip);
	// 	return;
	// }

	// ((CActor*)m_pEntity)->ChangeMoney(mtYuanbao, -nMoney, 118, 0, "combat clear cd");
	// m_nClearTime = 0;
	// m_bCdFlag = false;
	// m_bSaveInfoFlag = true;

	// SendCombatBaseInfo();
}

void CCombatSystem::RechargeRankAward(int nAwardId)
{
	//TriggerEvent(OpRechargeAwrad, nAwardId, m_nCurrentRankLevel);
}

void CCombatSystem::GetCombatLog()
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// int nCount = nCombatConfig.m_CombatRankConfig.nMaxChallangeRecord;
	// if(nCount > m_CombatRecordList.count())
	// {
	// 	nCount = (int)(m_CombatRecordList.count());
	// }

	// CActorPacket AP;
	// CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	// DataPacket << (BYTE)enCombatSystemID << (BYTE)sSendCombatLog;
	// DataPacket << (int)nCount;

	// for (int i = (int)m_CombatRecordList.count() - 1; m_CombatRecordList.count() - i <= nCount; --i)
	// {
	// 	DataPacket << (BYTE)m_CombatRecordList[i].nType;
	// 	DataPacket << (BYTE)m_CombatRecordList[i].nResult;
	// 	DataPacket << (int)m_CombatRecordList[i].nActorId;
	// 	DataPacket.writeString(m_CombatRecordList[i].strName);
	// 	DataPacket << (int)m_CombatRecordList[i].nRank;
	// }	
	// AP.flush();
}

void CCombatSystem::GetCombatRank(int nCurrentPage)
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);
	// if(pRanking)
	// {
	// 	int nPageNum = nCombatConfig.m_CombatRankConfig.nShouCount;
	// 	int ItemCount = pRanking->GetItemCount();
	// 	if(ItemCount > 100)
	// 		ItemCount = 100;
	// 	int nTotalPage = (ItemCount + nPageNum - 1) / nPageNum;
	// 	if (nCurrentPage > nTotalPage)
	// 	{
	// 		return;
	// 	}

	// 	CActorPacket AP;
	// 	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	// 	DataPacket << (BYTE)enCombatSystemID << (BYTE)sSendCombatRank;
	// 	DataPacket << (int)nTotalPage;
	// 	DataPacket << (int)nCurrentPage;

	// 	int startIndex = (nCurrentPage-1) * nPageNum;
	// 	int endIndex = nCurrentPage * nPageNum;
	// 	if(endIndex > ItemCount)
	// 		endIndex = ItemCount;
	// 	DataPacket << (int)(endIndex - startIndex);

	// 	CVector<CRankingItem*>& itemList = pRanking->GetList();
	// 	for(int i= startIndex; i < endIndex; ++i)
	// 	{
	// 		CRankingItem* item = itemList[i];
	// 		DataPacket << (int)(i + 1);
	// 		DataPacket << (int)item->GetId();
	// 		DataPacket.writeString(item->GetSubData(0));
	// 		DataPacket << (int)item->GetPoint();
	// 		DataPacket << (int)atoi((item->GetSubData(1)));
	// 		DataPacket << (int)atoi((item->GetSubData(2)));
	// 	}	
	// 	AP.flush();
	// }
}

void CCombatSystem::GetRankAwardList()
{
	//TriggerEvent(OpGetAwardList);
}


void CCombatSystem::TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1,INT_PTR nParam2,INT_PTR nParam3,INT_PTR nParam4)
{
	// CScriptValueList paramList;
	// //((CActor*)m_pEntity)->InitEventScriptVaueList(paramList, aeCombat);
	// paramList << (int)nSubEvent;
	// if(nParam1 >=0)
	// {
	// 	paramList << (int)nParam1;
	// 	if(nParam2 >=0)
	// 	{
	// 		paramList << (int)nParam2;
	// 		if(nParam3 >= 0)
	// 		{
	// 			paramList << (int)nParam3;
	// 		}
	// 	}	
	// }

	// ((CActor*)m_pEntity)->OnEvent(aeCombat,paramList,paramList);
}


void CCombatSystem::AddToCombatRank(int nActorId, int nValue)
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);

	// if(pRanking)
	// {
	// 	int ItemCount = pRanking->GetItemCount();
	// 	if(ItemCount >= nCombatConfig.m_CombatRankConfig.nMaxRankNum)
	// 	{
	// 		return;
	// 	}

	// 	CRankingItem *pRItem = pRanking->AddItem(nActorId, nValue, false);
	// 	//添加到排行榜
	// 	if (pRItem)
	// 	{
	// 		pRItem->SetSub(0, (char*)((CActor*)m_pEntity->GetEntityName()));

	// 		int nCircle = m_pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
	// 		char buf1[32];
	// 		sprintf_s(buf1, sizeof(buf1), "%d", nCircle);
	// 		pRItem->SetSub(1, buf1);

	// 		int nLevle = m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
	// 		char buf2[32];
	// 		sprintf_s(buf2,sizeof(buf2),"%d",nLevle);
	// 		pRItem->SetSub(2, buf2);

	// 		int nSex = m_pEntity->GetProperty<int>(PROP_ACTOR_SEX);
	// 		char buf3[32];
	// 		sprintf_s(buf3,sizeof(buf3),"%d",nSex);
	// 		pRItem->SetSub(3,buf3);

	// 		int nIcon = m_pEntity->GetProperty<int>(PROP_ENTITY_ICON);
	// 		char buf4[32];
	// 		sprintf_s(buf4,sizeof(buf4),"%d",nIcon);
	// 		pRItem->SetSub(4,buf4);

	// 		int nModeId = m_pEntity->GetProperty<int>(PROP_ENTITY_MODELID);
	// 		char buf5[32];
	// 		sprintf_s(buf5,sizeof(buf5),"%d",nModeId);
	// 		pRItem->SetSub(5,buf5);

	// 		int nWeaponId = m_pEntity->GetProperty<int>(PROP_ACTOR_WEAPONAPPEARANCE);
	// 		char buf6[32];
	// 		sprintf_s(buf6,sizeof(buf6),"%d",nWeaponId);
	// 		pRItem->SetSub(6,buf6);

	// 		int nSwingId = m_pEntity->GetProperty<int>(PROP_ACTOR_SWINGAPPEARANCE);
	// 		char buf7[32];
	// 		sprintf_s(buf7,sizeof(buf7),"%d",nSwingId);
	// 		pRItem->SetSub(7,buf7);

	// 		int nHp = m_pEntity->GetProperty<int>(PROP_CREATURE_MAXHP);
	// 		char buf8[32];
	// 		sprintf_s(buf8,sizeof(buf8),"%d",nHp);
	// 		pRItem->SetSub(8,buf8);

	// 		int nAttack = ((CActor*)m_pEntity)->GetActorAttackValueMax();
	// 		char buf9[32];
	// 		sprintf_s(buf9,sizeof(buf9),"%d",nAttack);
	// 		pRItem->SetSub(9,buf9);

	// 		int nDefence = ((CActor*)m_pEntity)->GetActorDefenceValueMax();
	// 		char buf10[32];
	// 		sprintf_s(buf10,sizeof(buf10),"%d",nDefence);
	// 		pRItem->SetSub(10,buf10);

	// 		int nHit = m_pEntity->GetProperty<int>(PROP_CREATURE_HITVALUE);
	// 		char buf11[32];
	// 		sprintf_s(buf11,sizeof(buf11),"%d",nHit);
	// 		pRItem->SetSub(11,buf11);

	// 		int nCrit = m_pEntity->GetProperty<int>(PROP_ACTOR_CRIT_RATE);
	// 		char buf12[32];
	// 		sprintf_s(buf12,sizeof(buf12),"%d",nCrit);
	// 		pRItem->SetSub(12,buf12);

	// 		int nDodge = m_pEntity->GetProperty<int>(PROP_CREATURE_DODVALUE);
	// 		char buf13[32];
	// 		sprintf_s(buf13,sizeof(buf13),"%d",nDodge);
	// 		pRItem->SetSub(13,buf13);

	// 		pRItem->SetSub(14, "1");

	// 		m_nCurrentRankLevel = ItemCount + 1;
	// 		if (m_nMaxRankLevel <= 0 || m_nMaxRankLevel > m_nCurrentRankLevel)
	// 		{
	// 			m_nMaxRankLevel = m_nCurrentRankLevel;
	// 		}
	// 		m_bSaveInfoFlag = true;
	// 	}
	// }
}

void CCombatSystem::UpdateCombatRanking()
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);

	// if(pRanking)
	// {
	// 	int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);
	// 	int nPoint = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_BATTLE_POWER);
	// 	CRankingItem* pRItem = pRanking->GetPtrFromId(nActorId);
	// 	//更新排行榜
	// 	if (pRItem)
	// 	{
	// 		pRItem->SetPoint(nPoint);

	// 		pRItem->SetSub(0, (char*)((CActor*)m_pEntity->GetEntityName()));

	// 		int nCircle = m_pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
	// 		char buf1[32];
	// 		sprintf_s(buf1, sizeof(buf1), "%d", nCircle);
	// 		pRItem->SetSub(1, buf1);

	// 		int nLevle = m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
	// 		char buf2[32];
	// 		sprintf_s(buf2,sizeof(buf2),"%d",nLevle);
	// 		pRItem->SetSub(2, buf2);

	// 		int nSex = m_pEntity->GetProperty<int>(PROP_ACTOR_SEX);
	// 		char buf3[32];
	// 		sprintf_s(buf3,sizeof(buf3),"%d",nSex);
	// 		pRItem->SetSub(3,buf3);

	// 		int nIcon = m_pEntity->GetProperty<int>(PROP_ENTITY_ICON);
	// 		char buf4[32];
	// 		sprintf_s(buf4,sizeof(buf4),"%d",nIcon);
	// 		pRItem->SetSub(4,buf4);

	// 		int nModeId = m_pEntity->GetProperty<int>(PROP_ENTITY_MODELID);
	// 		char buf5[32];
	// 		sprintf_s(buf5,sizeof(buf5),"%d",nModeId);
	// 		pRItem->SetSub(5,buf5);

	// 		int nWeaponId = m_pEntity->GetProperty<int>(PROP_ACTOR_WEAPONAPPEARANCE);
	// 		char buf6[32];
	// 		sprintf_s(buf6,sizeof(buf6),"%d",nWeaponId);
	// 		pRItem->SetSub(6,buf6);

	// 		int nSwingId = m_pEntity->GetProperty<int>(PROP_ACTOR_SWINGAPPEARANCE);
	// 		char buf7[32];
	// 		sprintf_s(buf7,sizeof(buf7),"%d",nSwingId);
	// 		pRItem->SetSub(7,buf7);

	// 		int nHp = m_pEntity->GetProperty<int>(PROP_CREATURE_MAXHP);
	// 		char buf8[32];
	// 		sprintf_s(buf8,sizeof(buf8),"%d",nHp);
	// 		pRItem->SetSub(8,buf8);

	// 		int nAttack = ((CActor*)m_pEntity)->GetActorAttackValueMax();
	// 		char buf9[32];
	// 		sprintf_s(buf9,sizeof(buf9),"%d",nAttack);
	// 		pRItem->SetSub(9,buf9);

	// 		int nDefence = ((CActor*)m_pEntity)->GetActorDefenceValueMax();
	// 		char buf10[32];
	// 		sprintf_s(buf10,sizeof(buf10),"%d",nDefence);
	// 		pRItem->SetSub(10,buf10);

	// 		int nHit = m_pEntity->GetProperty<int>(PROP_CREATURE_HITVALUE);
	// 		char buf11[32];
	// 		sprintf_s(buf11,sizeof(buf11),"%d",nHit);
	// 		pRItem->SetSub(11,buf11);

	// 		int nCrit = m_pEntity->GetProperty<int>(PROP_ACTOR_CRIT_RATE);
	// 		char buf12[32];
	// 		sprintf_s(buf12,sizeof(buf12),"%d",nCrit);
	// 		pRItem->SetSub(12,buf12);

	// 		int nDodge = m_pEntity->GetProperty<int>(PROP_CREATURE_DODVALUE);
	// 		char buf13[32];
	// 		sprintf_s(buf13,sizeof(buf13),"%d",nDodge);
	// 		pRItem->SetSub(13,buf13);

	// 		pRItem->SetSub(14, "1");
	// 	}
	// 	else
	// 	{
	// 		AddToCombatRank(nActorId, nPoint);
	// 	}
	// }
}

void CCombatSystem::SwapCombatRank(int nActorId, int nTagetId)
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);

	// if(pRanking)
	// {
	// 	CVector<CRankingItem*>& nItemList = pRanking->GetList();

	// 	INT_PTR nTargetIndex =-1;
	// 	//查找目标的index 和指针
	// 	CRankingItem* nItem = pRanking->GetPtrFromId(nTagetId, nTargetIndex);
	// 	if(nItem)
	// 	{
	// 		INT_PTR nTempIndex =-1;
	// 		CRankingItem* mItem = pRanking->GetPtrFromId(nActorId,nTempIndex);

	// 		//挑战方排名在1000以内的
	// 		if(mItem)
	// 		{
	// 			pRanking->SwapItem(nTargetIndex,nTempIndex);
	// 		}
	// 		else if (nItemList.count() < nCombatConfig.m_CombatRankConfig.nMaxRankNum)	//当排行榜最大人数还没1000人的时候
	// 		{
	// 			INT_PTR nItemCount = nItemList.count();
	// 			AddToCombatRank(nActorId, m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_BATTLE_POWER));
	// 			if (nItemCount < nItemList.count())	//加入排行榜成功
	// 			{
	// 				nTempIndex = nItemList.count() - 1;
	// 				pRanking->SwapItem(nTargetIndex, nTempIndex);
	// 			}
	// 		}
	// 		else	//排名在1000外的情况
	// 		{
	// 			nItem->SetId(nActorId);
	// 			nItem->SetPoint(m_pEntity->GetProperty<int>(PROP_ACTOR_BATTLE_POWER));
	// 			nItem->SetSub(0, (char*)((CActor*)m_pEntity->GetEntityName()));

	// 			int nCircle = m_pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
	// 			char buf1[32];
	// 			sprintf_s(buf1, sizeof(buf1), "%d", nCircle);
	// 			nItem->SetSub(1, buf1);

	// 			int nLevle = m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
	// 			char buf2[32];
	// 			sprintf_s(buf2,sizeof(buf2),"%d",nLevle);
	// 			nItem->SetSub(2, buf2);

	// 			int nSex = m_pEntity->GetProperty<int>(PROP_ACTOR_SEX);
	// 			char buf3[32];
	// 			sprintf_s(buf3,sizeof(buf3),"%d",nSex);
	// 			nItem->SetSub(3,buf3);

	// 			int nIcon = m_pEntity->GetProperty<int>(PROP_ENTITY_ICON);
	// 			char buf4[32];
	// 			sprintf_s(buf4,sizeof(buf4),"%d",nIcon);
	// 			nItem->SetSub(4,buf4);

	// 			int nModeId = m_pEntity->GetProperty<int>(PROP_ENTITY_MODELID);
	// 			char buf5[32];
	// 			sprintf_s(buf5,sizeof(buf5),"%d",nModeId);
	// 			nItem->SetSub(5,buf5);

	// 			int nWeaponId = m_pEntity->GetProperty<int>(PROP_ACTOR_WEAPONAPPEARANCE);
	// 			char buf6[32];
	// 			sprintf_s(buf6,sizeof(buf6),"%d",nWeaponId);
	// 			nItem->SetSub(6,buf6);

	// 			int nSwingId = m_pEntity->GetProperty<int>(PROP_ACTOR_SWINGAPPEARANCE);
	// 			char buf7[32];
	// 			sprintf_s(buf7,sizeof(buf7),"%d",nSwingId);
	// 			nItem->SetSub(7,buf7);

	// 			int nHp = m_pEntity->GetProperty<int>(PROP_CREATURE_MAXHP);
	// 			char buf8[32];
	// 			sprintf_s(buf8,sizeof(buf8),"%d",nHp);
	// 			nItem->SetSub(8,buf8);

	// 			int nAttack = ((CActor*)m_pEntity)->GetActorAttackValueMax();
	// 			char buf9[32];
	// 			sprintf_s(buf9,sizeof(buf9),"%d",nAttack);
	// 			nItem->SetSub(9,buf9);

	// 			int nDefence = ((CActor*)m_pEntity)->GetActorDefenceValueMax();
	// 			char buf10[32];
	// 			sprintf_s(buf10,sizeof(buf10),"%d",nDefence);
	// 			nItem->SetSub(10,buf10);

	// 			int nHit = m_pEntity->GetProperty<int>(PROP_CREATURE_HITVALUE);
	// 			char buf11[32];
	// 			sprintf_s(buf11,sizeof(buf11),"%d",nHit);
	// 			nItem->SetSub(11,buf11);

	// 			int nCrit = m_pEntity->GetProperty<int>(PROP_ACTOR_CRIT_RATE);
	// 			char buf12[32];
	// 			sprintf_s(buf12,sizeof(buf12),"%d",nCrit);
	// 			nItem->SetSub(12,buf12);

	// 			int nDodge = m_pEntity->GetProperty<int>(PROP_CREATURE_DODVALUE);
	// 			char buf13[32];
	// 			sprintf_s(buf13,sizeof(buf13),"%d",nDodge);
	// 			nItem->SetSub(13,buf13);

	// 			nItem->SetSub(14, "1");
	// 		}
	// 	}
	// }
}


void CCombatSystem::PickRankPlayerImpl()
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// int nCount = nCombatConfig.m_Count;
	// if(nCount <= 0)
	// {
	// 	return;
	// }

	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);

	// int i = 0;
	// if(pRanking)
	// {
	// 	int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);
	// 	int rIndex = (int)(pRanking->GetIndexFromId(nActorId));

	// 	int itemCount = pRanking->GetItemCount();
	// 	int nMaxRankCount = __min(itemCount, nCombatConfig.m_CombatRankConfig.nMaxRankNum);

	// 	if (rIndex < 0)
	// 	{
	// 		rIndex = nMaxRankCount;	//如果玩家没有排行榜中，暂时把他看成是排行榜最后一位的后一位
	// 	}

	// 	for (i = 0; i < nCount; ++i)
	// 	{
	// 		if (rIndex >= nCombatConfig.m_CombatGroupList[i].nStartRank && rIndex <= nCombatConfig.m_CombatGroupList[i].nEndRank)
	// 		{
	// 			break;	
	// 		}
	// 	}
	// 	if (i == nCount)
	// 	{
	// 		i = nCount - 1;
	// 	}

	// 	for (int j = 0; j < MAXPICKITEMNUM; ++j)
	// 	{
	// 		PICKITEM &pickItem = nCombatConfig.m_CombatGroupList[i].sPickitems[j];
	// 		int nStartIndex = pickItem.nStartPickRank + rIndex;
	// 		int nEndIndex = pickItem.nEndPIckRank + rIndex;
	// 		if (nStartIndex < 0)
	// 		{
	// 			nStartIndex = 0;
	// 		}
	// 		else if (nStartIndex >= nMaxRankCount)
	// 		{
	// 			nStartIndex = nMaxRankCount - 1;
	// 		}
	// 		if (nEndIndex < 0)
	// 		{
	// 			nEndIndex = 0;
	// 		}
	// 		else if (nEndIndex >= nMaxRankCount)
	// 		{
	// 			nEndIndex = nMaxRankCount - 1;
	// 		}
	// 		int randIndex = (nEndIndex - nStartIndex + 1) <= 0 ? 1:(nEndIndex - nStartIndex + 1);
	// 		int nRandom = (rand() % randIndex) + nStartIndex;
	// 		if (j > 0)
	// 		{
	// 			// 剩下的排名选手数不够填充
	// 			if (nMaxRankCount - nRandom <= MAXPICKITEMNUM - j)
	// 			{
	// 				nRandom = m_nSelectedIndex[j] + 1;
	// 			}
	// 			// 后选排名高于前者
	// 			else if (nRandom <= m_nSelectedIndex[j])
	// 			{
	// 				nRandom = m_nSelectedIndex[j] + 1;
	// 			}
	// 		}
	// 		if (nRandom + 1 == m_nCurrentRankLevel) //排除自己
	// 		{
	// 			nRandom++;
	// 		}
	// 		m_nSelectedIndex[j + 1] = nRandom;
	// 	}
	// }
}

void CCombatSystem::SendPickRankPlayer()
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);
	// if(pRanking)
	// {
	// 	CActorPacket AP;
	// 	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	// 	DataPacket << (BYTE)enCombatSystemID << (BYTE)sSendSelectResult;

	// 	size_t pos = DataPacket.getPosition();
	// 	DataPacket << (BYTE)0;

	// 	int nResult = 0;
	// 	CVector<CRankingItem*>& itemList = pRanking->GetList();
	// 	for (int i = 0; i < MAXPICKITEMNUM + 1; ++i)
	// 	{
	// 		if (m_nSelectedIndex[i] >= 0 && m_nSelectedIndex[i] < itemList.count())
	// 		{
	// 			CRankingItem* item = itemList[m_nSelectedIndex[i]];
	// 			DataPacket << (int)item->GetId();
	// 			DataPacket << (int)(m_nSelectedIndex[i] + 1);
	// 			DataPacket << (int)item->GetPoint();
	// 			DataPacket << (int)atoi((item->GetSubData(1)));
	// 			DataPacket << (int)atoi((item->GetSubData(2)));
	// 			DataPacket.writeString(item->GetSubData(0));
	// 			DataPacket << (int)atoi((item->GetSubData(3)));
	// 			DataPacket << (int)atoi((item->GetSubData(4)));
	// 			DataPacket << (int)atoi((item->GetSubData(5)));
	// 			DataPacket << (int)atoi((item->GetSubData(6)));
	// 			DataPacket << (int)atoi((item->GetSubData(7)));
	// 			DataPacket << (int)atoi((item->GetSubData(14)));
	// 			nResult++;
	// 		}
	// 	}

	// 	BYTE* pCount = (BYTE*)DataPacket.getPositionPtr(pos);
	// 	*pCount = (BYTE)nResult;
	// 	AP.flush();
	// }
}

int  CCombatSystem::GetMyCombatRank()
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);
	// if(pRanking == NULL)
	// {
	 	return 0;
	// }
	// int nMyActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);
	// int nMyRank = (int)pRanking->GetIndexFromId(nMyActorId) + 1;
	// return nMyRank;
}


void CCombatSystem::ChallegeOverOp(int nResult, int nActorId, char* strName, int nIsReal)
{
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(nCombatConfig.m_CombatRankConfig.sRankInfo[m_nJobType].nRankName);
	// if(pRanking == NULL)
	// {
	// 	return;
	// }

	// int nMyActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);
	// int nMyRank = (int)pRanking->GetIndexFromId(nMyActorId) + 1;
	// int nTargetRank = (int)pRanking->GetIndexFromId(nActorId) + 1;

	// m_nCurrentRankLevel = nMyRank;
	// if (m_nMaxRankLevel <= 0 || m_nMaxRankLevel > m_nCurrentRankLevel)
	// {
	// 	m_nMaxRankLevel = m_nCurrentRankLevel;
	// }
	// m_bSaveInfoFlag = true;
	// SendCombatBaseInfo();

	// if(nResult == enSuccAndSwap ||  nResult == enSucc)
	// {
	// 	//成功挑战职业宗师X/Y次
	// 	((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtCombatSuccCount, 0, 1, true);
	// }

	// //胜利交换排名需要重刷选择对手
	// if (nResult == enSuccAndSwap)
	// {

	// 	PickRankPlayerImpl();
	// 	SendPickRankPlayer();
	// }

	// TriggerEvent(OpUpdateActorRank, (int)pRanking->GetIndexFromId(nMyActorId) + 1);

	// if(strName) 
	// {
	// 	AddToCombatLog(0, strName, nActorId, nMyRank, nResult);
	// }

	// if (nIsReal > 0)
	// {
	// 	int nConvResult = -nResult;
		
	// 	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	// 	if (pActor != NULL)
	// 	{
	// 		pActor->GetCombatSystem().ChallegeNotice(nConvResult, nMyActorId, (char*)((CActor*)m_pEntity->GetEntityName()), nTargetRank);
	// 	}
	// 	else
	// 	{
	// 		char MsgBuff[MAX_MSG_COUNT];
	// 		CDataPacket packet(MsgBuff, sizeof(MsgBuff));

	// 		packet.writeString("");
	// 		packet.writeString("");
	// 		packet << (int)nConvResult;
	// 		packet << (int)nMyActorId;
	// 		packet << (int)nTargetRank;
	// 		packet.writeString((char*)((CActor*)m_pEntity->GetEntityName()));

	// 		Assert(packet.getPosition() <= MAX_MSG_COUNT);
	// 		CMsgSystem::AddOfflineMsg(nActorId, CMsgSystem::mtCombatLog, MsgBuff, packet.getPosition(), 0);
	// 	}
	// }
}

void CCombatSystem::ChallegeNotice(int nResult, int nActorId, char* strName, int nMyRank)
{
	// m_nCurrentRankLevel = nMyRank;
	// if (m_nMaxRankLevel <= 0 || m_nMaxRankLevel > m_nCurrentRankLevel)
	// {
	// 	m_nMaxRankLevel = m_nCurrentRankLevel;
	// }
	// m_bSaveInfoFlag = true;
	// SendCombatBaseInfo();

	// if(strName) 
	// {
	// 	AddToCombatLog(1, strName, nActorId, nMyRank, nResult);
	// }
}

void CCombatSystem::OfflineCombatOp(int nResult, int nActorId, char* strName, int nMyRank)
{
	// if(strName) 
	// {
	// 	AddToCombatLog(1, strName, nActorId, nMyRank, nResult);
	// }
}

void CCombatSystem::OnNewDayClear()
{
	// m_nChallengeCount = 0;
	// m_nPickCount = 0;
	// m_bSaveInfoFlag = true;

	// SendCombatBaseInfo();
}

void CCombatSystem::AddToCombatLog(int nType, char *strName, int nActorId, int nRank, int nResult)
{
	// m_bSaveLogFlag = true;

	// COMBATRECORD tmpRecord;

	// _asncpytA(tmpRecord.strName, strName);
	// tmpRecord.nType		= nType;
	// tmpRecord.nActorId	= nActorId;
	// tmpRecord.nRank		= nRank;
	// tmpRecord.nResult	= nResult;

	// m_CombatRecordList.add(tmpRecord);	
}


void CCombatSystem::ChangeCombatScore(int nValue) 
{ 
	// m_nCombatScore += nValue; 
	// m_bSaveInfoFlag = true; 

	// SendCombatBaseInfo();
}

void CCombatSystem::OnLevelUp()
{
	// //已经入榜
	// if (m_nCurrentRankLevel > 0)
	// {
	// 	return;
	// }
	// //等级限制
	// CCombatRankProvider& nCombatConfig = GetLogicServer()->GetDataProvider()->GetCombatConfig();
	// if (false == ((CActor*)m_pEntity)->CheckLevel(nCombatConfig.m_CombatRankConfig.nLevLimit, nCombatConfig.m_CombatRankConfig.nCircleLimit))
	// {
	// 	return;
	// }

	// int nActorId = m_pEntity->GetProperty<int>(PROP_ENTITY_ID);
	// int nPoint = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_BATTLE_POWER);
	// AddToCombatRank(nActorId, nPoint);
}

