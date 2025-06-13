#include "StdAfx.h"
#include "AlmirahSystem.h"
#include "../base/Container.hpp"
CAlmirahSystem::CAlmirahSystem(void)
{
}

CAlmirahSystem::~CAlmirahSystem(void)
{

}

void  CAlmirahSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(!m_pEntity) return;
	switch(nCmd)
	{
	case cTakeOnAndOff:
		{
			int nModelId = 0;
			packet >> nModelId;
			byte state = 0;
			packet >> state;
			SetTakeon(nModelId, state);
		}							
		break;
	case cGetInfo:
		{
			OnSendInitData();
		}
		break;
	case cActivation:
		{
			int nModelId = 0;
			packet >> nModelId;
			ActivationAlmirah(nModelId);
			break;
		}
	case cUpdateFashion:
		{
			int nModelId = 0;
			packet >> nModelId;
			UpGradeFashion(nModelId);
			break;
		}
	default:
		break;
	}
}

void CAlmirahSystem::UpGradeFashion(int nModleId)
{
	if(!m_pEntity) return;
	int error = 0;
	int nLv = 1;
	do
	{
		if(!GetActivation(nModleId))
		{
			error = 1;
			break;
		}
		nLv = GetFashionLv(nModleId);
		tagFashionAttribute* actCfg = GetLogicServer()->GetDataProvider()->GetFashionConfig().getFashionUpdate(nModleId,nLv);
		if(!actCfg)
		{
			error = 2;
			break;
		}

		tagFashionAttribute* nextCfg = GetLogicServer()->GetDataProvider()->GetFashionConfig().getFashionUpdate(nModleId,nLv+1);
		if(!nextCfg)
		{
			error = 3;
			break;
		}

		if(nextCfg->cost.size())
		{
			for(int i = 0; i < nextCfg->cost.size(); i++)
			{
				if(!((CActor*)m_pEntity)->CheckActorSource(nextCfg->cost[i].nType, nextCfg->cost[i].nId, nextCfg->cost[i].nCount, tstUI) )
				{
					error = 4;
					break ;
				}
			}
		}
		if(error)
			break;

		if(nextCfg->cost.size())
		{
			for(int i = 0; i < nextCfg->cost.size(); i++)
			{
				((CActor*)m_pEntity)->RemoveConsume(nextCfg->cost[i].nType, nextCfg->cost[i].nId, nextCfg->cost[i].nCount, -1, -1, -1, 0,GameLog::Log_Fashion);
			}
		}
		nLv += 1;
		UpdateFashion(nModleId,nLv);
		//需要重新计算属性
		((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);	
		// 更新升级后的血、蓝
	} while (false);
	
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data <<(BYTE)GetSystemID() << (BYTE)sUpdateFashion;
	data << (BYTE)error;
	data <<(int)nModleId;
	data <<(int)nLv;
	ap.flush();

}

void CAlmirahSystem::Save(PACTORDBDATA  pData)
{
	if(m_pEntity ==NULL) return;
	if (!HasDataModified())
	{
		return;
	}
	CDataClient * pDb = GetLogicServer()->GetDbClient();
	if (!pDb->connected())
	{
		return;
	}
	CDataPacket& dataPacket = pDb->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveAlmirahItem);
	dataPacket << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	dataPacket <<(unsigned int ) m_pEntity->GetId(); //玩家ID
	WORD nCount = (WORD)m_cAlmirahLists.count();
	dataPacket  << (WORD)nCount;
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CAlmirahItem & oneItem = m_cAlmirahLists[i];

		dataPacket << (WORD)oneItem.nModelId;
		dataPacket << (byte)oneItem.btTakeOn;
		dataPacket << (int)oneItem.nLv;
	}
	pDb->flushProtoPacket(dataPacket);
	SetDataModifyFlag(false);
}

void CAlmirahSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("装载玩家衣橱的数据出错,实体指针为MULL"));
		return;
	}
	//装载物品数据
	if(nCmd == jxInterSrvComm::DbServerProto::dcLoadAlmirahItem &&  nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回物品的列表
	{
		WORD nCount =0;
		reader >> nCount;
		m_cAlmirahLists.clear();
		CAlmirahItem oneItem;
		for (WORD i = 0; i < nCount; i++)
		{
			
			reader >> oneItem.nModelId;		
			reader >> oneItem.nLv;
			reader >> oneItem.btTakeOn;
			m_cAlmirahLists.add(oneItem);
			// if(oneItem.btTakeOn == 1)
			// {
			// 	((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_SOLDIERSOULAPPEARANCE,oneItem.nModelId);
			// }
		}
		OnDbInitData();				//DB加载好了
	}
	else if(nCmd == jxInterSrvComm::DbServerProto::dcLoadAlmirahItem &&  nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
	{
		OutputMsg(rmError,_T("装载玩家[%s]的衣橱的数据出错,actorid=%u"),((CActor*)m_pEntity)->GetEntityName(),m_pEntity->GetId());
		((CActor*)m_pEntity)->CloseActor(lwiAlmirahDbFail, false); //如果装载装备的数据出错就踢出
	}
}

void CAlmirahSystem::OnSendInitData()
{
	if(!m_pEntity) return;
	byte nSystemId = (byte)GetSystemID();
	byte count = (byte)m_cAlmirahLists.count();
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << nSystemId << (byte)sSendInfo;
	netPack << (byte)count;
	for (INT_PTR i = 0; i < count; i++)
	{
		CAlmirahItem & oneItem = m_cAlmirahLists[i];
		netPack << (int)oneItem.nModelId;
		netPack << (BYTE)oneItem.btTakeOn;
		netPack << (int)oneItem.nLv;
	}
	pack.flush();
}

CAttrCalc& CAlmirahSystem::CalcAttr(CAttrCalc &calc)
{
	if(m_pEntity == NULL)
		return calc;

	
	INT_PTR nCount = m_cAlmirahLists.count();
	for(INT_PTR i=0; i<nCount; i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		tagFashionAttribute* cfg = GetLogicServer()->GetDataProvider()->GetFashionConfig().getFashionUpdate(stAlmirahItem.nModelId, stAlmirahItem.nLv);
		if(cfg)
		{
			for(int  j = 0; j < cfg->attri.nCount; j++)
			{
				calc << cfg->attri.pAttrs[j];
			}
		}
	}

	return calc;
}

bool CAlmirahSystem::SetActivation(int nModleId)
{
	// INT_PTR nCount = m_WingItem.count();
	// byte nState = 1;
	// byte nSystemId = (byte)GetSystemID();
	// for(INT_PTR i=0; i<nCount; i++)
	// {
	// 	CAlmirahItem& stAlmirahItem =  m_WingItem[i];
	// 	if( stAlmirahItem.nModelId == nModleId )
	// 	{
	// 		SetDataModifyFlag(true); //数据修改过了
	// 		CActorPacket pack;
	// 		CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// 		netPack << nSystemId << (byte)sActivation;
	// 		netPack << (WORD)nModleId;
	// 		netPack << nState;
	// 		pack.flush();
	// 		return true;
	// 	}
	// }
	return false;
}

bool CAlmirahSystem::GetActivation(int nModleId)
{
	INT_PTR nCount = m_cAlmirahLists.count();
	for(INT_PTR i=0; i<nCount; i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		if( stAlmirahItem.nModelId == nModleId )
		{
			return true;
		}
	}
	return false;
}

bool CAlmirahSystem::SetTakeon(int nModleId, byte nState)
{
	if(m_pEntity ==NULL) 
		return false;
	if(nState < 0 || 1 < nState)
		return false;
	CFashionCfg& Cfg = GetLogicServer()->GetDataProvider()->GetFashionConfig();
	FashionActive* actCfg = Cfg.getFashionActive(nModleId);
	if(!actCfg)
		return false;
	if(nState == 1)
	{
		if(Cfg.takeofflist.find(actCfg->nType) != Cfg.takeofflist.end()) {
			std::set<int> takeOff(Cfg.takeofflist[actCfg->nType].begin(), Cfg.takeofflist[actCfg->nType].end());
			TakeOffAlmirah(takeOff);
		}
	}

	for(INT_PTR i=0; i<m_cAlmirahLists.count(); i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		// FashionActive* Cfg = Cfg.getFashionActive(stAlmirahItem.nModelId);
		// if(!Cfg)
		// 	continue;
		
		if( stAlmirahItem.nModelId == nModleId )
		{
			stAlmirahItem.btTakeOn = nState;
		}

		// if(Cfg->nType == actCfg->nType && stAlmirahItem.nModelId != nModleId && stAlmirahItem.btTakeOn == 1)
		// {
		// 	stAlmirahItem.btTakeOn = 0;
		// }
	}
	

	
	byte nSystemId = (byte)GetSystemID();
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << nSystemId << (byte)sTakeOnAndOff;
	netPack << (int)nModleId;
	netPack << nState;
	netPack << (BYTE) actCfg->nType;
	pack.flush();
	if(nState == 0)
	{
		nModleId = 0;
	}
	((CActor*)m_pEntity)->GetEquipmentSystem().RefreshAppear(); //刷外观
	m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	
	// 更新升级后的血、蓝
	
	// if(actCfg->nType == 1)//衣服
	// {
	// 	((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_SOLDIERSOULAPPEARANCE,nModleId);
	// }
	// else if(actCfg->nType == 2)//武器
	// {
	// 	((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_WEAPONAPPEARANCE,nModleId);
	// }
	
	SetDataModifyFlag(true); //数据修改过了
	// OnSendInitData();
	return false;
}


int CAlmirahSystem::GetNowModeId()
{
	INT_PTR nCount = m_cAlmirahLists.count();
	for(INT_PTR i=0; i<nCount; i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		if( stAlmirahItem.btTakeOn == 1  )
		{
			
			return stAlmirahItem.nModelId;
		}
	}
	return 0;
}


int CAlmirahSystem::GetFashionLv(int nModeId)
{
	INT_PTR nCount = m_cAlmirahLists.count();
	for(INT_PTR i=0; i<nCount; i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		if( stAlmirahItem.nModelId == nModeId )
		{
			return  m_cAlmirahLists[i].nLv;
		}
	}
	return 0;
}


void CAlmirahSystem::ActivationAlmirah(int nModleId)
{
	if(!m_pEntity) return;
	int error = 0;
	int nWear = 0;
	int nType = 0;
	do
	{
		if(GetActivation(nModleId))
		{
			error = 1;
			break;
		}
		CFashionCfg& Cfg = GetLogicServer()->GetDataProvider()->GetFashionConfig();
		FashionActive* actCfg = Cfg.getFashionActive(nModleId);
		if(!actCfg)
		{
			error = 2;
			break;
		}

		if(actCfg->cost.size())
		{
			for(int i = 0; i < actCfg->cost.size(); i++)
			{
				if(!((CActor*)m_pEntity)->CheckActorSource(actCfg->cost[i].nType, actCfg->cost[i].nId, actCfg->cost[i].nCount, tstUI) )
				{
					error = 3;
					break ;
				}
			}
		}
		if(error)
			break;

		if(actCfg->cost.size())
		{
			for(int i = 0; i < actCfg->cost.size(); i++)
			{
				((CActor*)m_pEntity)->RemoveConsume(actCfg->cost[i].nType, actCfg->cost[i].nId, actCfg->cost[i].nCount, -1, -1, -1, 0,GameLog::Log_Fashion);
			}
		}
		if(actCfg->nWear)
		{
			if(Cfg.takeofflist.find(actCfg->nType) != Cfg.takeofflist.end()) {
				std::set<int> takeOff(Cfg.takeofflist[actCfg->nType].begin(), Cfg.takeofflist[actCfg->nType].end());
				TakeOffAlmirah(takeOff);
			}
			// for(INT_PTR i=0; i<m_cAlmirahLists.count(); i++)
			// {
			// 	CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
			// 	// FashionActive* Cfg = Cfg.getFashionActive(stAlmirahItem.nModelId);
			// 	// if(!Cfg)
			// 	// 	continue;

			// 	if(Cfg->nType == actCfg->nType && stAlmirahItem.btTakeOn == 1)
			// 	{
			// 		stAlmirahItem.btTakeOn = 0;
			// 	}
			// }
		}
		nWear = AddNewFashion(nModleId,actCfg->nWear);
		nType = actCfg->nType ;
		if(actCfg->nWear)
		{
			((CActor*)m_pEntity)->GetEquipmentSystem().RefreshAppear(); //刷外观
			m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	
			// 更新升级后的血、蓝
		}
		
			
	} while (false);
	
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data <<(BYTE)GetSystemID() << (BYTE)sActivation;
	data << (BYTE)error;
	data <<(int)nModleId;
	data <<(BYTE)nWear;
	data <<(BYTE)nType;
	ap.flush();
}


int CAlmirahSystem::AddNewFashion(int nModelId, int nWear)
{
	SetDataModifyFlag(true);
	CAlmirahItem oneItem;
	oneItem.nModelId = nModelId;		
	oneItem.btTakeOn = nWear;
	oneItem.nLv = 1;
	m_cAlmirahLists.add(oneItem);
	return oneItem.btTakeOn;
}

void CAlmirahSystem::UpdateFashion(int nModelId, int nLv)
{
	SetDataModifyFlag(true);
	INT_PTR nCount = m_cAlmirahLists.count();
	for(INT_PTR i=0; i<nCount; i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		if( stAlmirahItem.nModelId == nModelId)
		{
			stAlmirahItem.nLv = nLv;
		}
	}
}



FashionActive* CAlmirahSystem::GetFashionModeId(int nType)
{
	INT_PTR nCount = m_cAlmirahLists.count();
	for(INT_PTR i=0; i<nCount; i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		FashionActive* cfg = GetLogicServer()->GetDataProvider()->GetFashionConfig().getFashionActive(stAlmirahItem.nModelId);
		if(cfg && cfg->nType == nType && stAlmirahItem.btTakeOn == 1)
		{
			return cfg;
		}
	}
	return NULL;
}


void CAlmirahSystem::returnInfo(CDataPacket& netPack)
{
	byte count = 0;
	if(!m_pEntity)
	{
		netPack << (byte)count;
		return;
	}
	count = (byte)m_cAlmirahLists.count();
	netPack << (byte)count;
	for (INT_PTR i = 0; i < count; i++)
	{
		CAlmirahItem & oneItem = m_cAlmirahLists[i];
		netPack << (int)oneItem.nModelId;
		netPack << (BYTE)oneItem.btTakeOn;
		netPack << (int)oneItem.nLv;
	}
}

void CAlmirahSystem::TakeOffAlmirah(std::set<int>& takeOffs)
{
	if(!takeOffs.size()) return ;
	for(INT_PTR i=0; i<m_cAlmirahLists.count(); i++)
	{
		CAlmirahItem& stAlmirahItem =  m_cAlmirahLists[i];
		FashionActive* Cfg = GetLogicServer()->GetDataProvider()->GetFashionConfig().getFashionActive(stAlmirahItem.nModelId);
		if(!Cfg)
			continue;

		if(takeOffs.count(Cfg->nType) && stAlmirahItem.btTakeOn == 1)
		{
			stAlmirahItem.btTakeOn = 0;
		}
	}
}

//跨服数据
void CAlmirahSystem::SendMsg2CrossServer(int nType)
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
	size_t count = m_cAlmirahLists.count();
	packet << (int)count  ; //玩家物品的数量
	if(count > 0)
	{
		for(int i = 0; i < count; i++) {
			packet <<m_cAlmirahLists[i].nModelId;
			packet <<m_cAlmirahLists[i].nLv;
			packet <<m_cAlmirahLists[i].btTakeOn;
		}
	}
	pCrossClient->flushProtoPacket(packet);
}


void CAlmirahSystem::OnCrossInitData(std::vector<CAlmirahItem>& m_Almira)
{
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("装载玩家衣橱的数据出错,实体指针为MULL"));
		return;
	}
	//装载物品数据
	WORD nCount =m_Almira.size();
	// reader >> nCount;
	m_cAlmirahLists.clear();
	CAlmirahItem oneItem;
	for (WORD i = 0; i < nCount; i++)
	{
		
		// reader >> oneItem.nModelId;		
		// reader >> oneItem.nLv;
		// reader >> oneItem.btTakeOn;
		oneItem =m_Almira[i];
		m_cAlmirahLists.add(oneItem);
		OutputMsg(rmTip,_T("OnCrossInitData CAlmirahSystem nActorID:%d nModelId:%d,level:%d"),
			m_pEntity->GetId(),oneItem.nModelId,oneItem.nLv);
	}
	SetDataModifyFlag(true); //数据修改过了
	((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CFASH);
}
