#include "StdAfx.h"
#include "QuestSystem.h"
#include "../base/Container.hpp"

void CQuestSystem::OnEnterGame()
{
	CheckCanAcceptQuest();
}
CQuestSystem::CQuestSystem()
{
	m_GoingQuestList.clear();
	isDataModify = false;
	memset(m_FinishedQuest, 0, sizeof(m_FinishedQuest));
}

CQuestSystem::~CQuestSystem()
{
	m_GoingQuestList.clear();
}

bool CQuestSystem::Initialize(void *data,SIZE_T size)
{
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
	{
		OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
		return false;
	}
	//将db的数据拷贝过来
	memcpy( m_FinishedQuest,pActorData->quest,sizeof(m_FinishedQuest));
	

	return true;
}

VOID CQuestSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode, CDataPacketReader &inPacket)
{
	OutputMsg(rmNormal,_T("CQuestSystem::OnDbRetData nCmd=%d"), nCmd);
	if (m_pEntity == NULL) return;
	if (nCmd == jxInterSrvComm::DbServerProto::dcSaveQuest)
	{
		//这个是保存数据时返回的结果
		if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
		{
			OutputMsg(rmTip,_T("save quest data,errorcode=%d,ActorId=%d"),nErrorCode,m_pEntity->GetId());
		}
		return;
	}
	else if(nCmd == jxInterSrvComm::DbServerProto::dcLoadQuest)
	{
		m_GoingQuestList.clear();
		WORD nCount = 0;
		inPacket >> nCount;
		if(HasDbDataInit()) return ; //如果已经装载了任务的数据，就不装载了
		for (int i = 0; i < nCount; i++)
		{
			QuestItem item;
			inPacket.readBuf(&item, sizeof(item));
			if(item.nStartId > 0)
				m_RingStartQId.push_back(item.nStartId);
			m_GoingQuestList.push_back(item);
		}
		OnDbInitData();
		//第一次下发可接任务
	}
}

VOID CQuestSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
}

void CQuestSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if (m_pEntity == NULL)
	{
		return;
	}
	switch (nCmd)
	{
	case sQueryQuest:
		SendAllQuestData();
		break;
	case cChangetQuestState:
	{
		int nQid = 0;
		int nState = 0;
		Uint64 npcHandle = 0;
		packet >> nQid >> nState >> npcHandle;
		ChangeQuestItemState(nQid, nState, npcHandle);
		break;
	}
	case cQuestTel: //传送
	{
		int nQid = 0;
		BYTE nType = 0;
		BYTE nId = 0;
		packet >>nQid >> nType >> nId;
		QuestTeleport(nQid, nType, nId);
		break;
	}
	default:
		OutputMsg(rmTip, _T("%s:客户端请求[%d]的数据包没处理，请检查是否做了多余的请求"), __FUNCTION__, nCmd);
		break;
	}
}

void CQuestSystem::QuestTeleport(int nQid, int nType, int nId)
{
	if( !m_pEntity ) return;
	int nCode =  0;
	int nNpcId = 0;
	int nIsOpen = 0;
	int nQstate = 0;
	int nAuto = 0;
	do
	{
		CActor* pActor = (CActor*)m_pEntity;
		if(GetfinishQuestState(nQid)) {
			nCode = 1; 
			break; 
		}

		QuestItem* item = GetGoingQuestData(nQid);
		if(!item) {
			nCode = 5;
			break;
		}

		QuestInfoCfg* cfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(item->nQid, item->nState);
		if(!cfg) {
			nCode = 3;
			break;
		}

		if(!GetLogicServer()->CheckOpenServer(cfg->nOpenDayLimit) || !m_pEntity->CheckLevel(cfg->nLevelLimit, cfg->nCircle)) {
			nCode = 4;
			break;
		}
		TeleportInfo* tel = cfg->GetTelInfo(nType, nId);
		if(!tel)
		{
			nCode = 10;
			break;
		}
		QuestAward pCost;;
		cfg->GetCostFsValue(item->nUseFsTimes, pCost);
		if(pCost.nCount)
		{
			if(!pActor->CheckActorSource(pCost.nType, pCost.nId, pCost.nCount, tstUI))
			{
				nCode = 20;
				break;
			}
		}
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		if( pActor->FlyShoseToStaticScene(tel->nMapid, tel->nX, tel->nY, tel->nRange,0,enDefaultTelePort,nEffId))
		{
			if(pCost.nCount)
			{
				pActor->RemoveConsume(pCost.nType, pCost.nId, pCost.nCount, -1, -1, -1, 0,GameLog::Log_Quest);
			}
		}
		if(nType == 3)
		{
			item->nUseFsTimes++;
			returnClientQuestState(nQid, 0);
		}
		m_infos.nAuto = tel->nAuto;
		m_infos.nIsOpen = tel->nIsOpen;
		m_infos.nQstate = item->nState;
		m_infos.nTelNpcId = tel->nTelNpcId;
		m_infos.nQid = nQid;
		if(tel->nTelNpcId) {
			return;
		}
		// nAuto = tel->nAuto;
		// nIsOpen = tel->nIsOpen;
		// nQstate = item->nState;
	}
	while(false);
	// SendQuestTelResult(nCode);
	CActorPacket ap;
	CDataPacket & data = m_pEntity->AllocPacket(ap);
	data<<(BYTE)GetSystemID() << (BYTE)sQuestTel;
	data<<(BYTE)nCode;
	data<<(BYTE)m_infos.nIsOpen;
	data<<(BYTE)m_infos.nAuto;
	data<<(int)m_infos.nQid;
	data<<(int)m_infos.nQstate;
	data<<(int)m_infos.nTelNpcId;
	ap.flush();
		memset(&m_infos, 0,sizeof(m_infos));
}

void CQuestSystem::SendQuestTelResult(int nCode)
{
	if( nCode != 0 || m_infos.nTelNpcId != 0 )
	{
		CActorPacket ap;
		CDataPacket & data = m_pEntity->AllocPacket(ap);
		data<<(BYTE)GetSystemID() << (BYTE)sQuestTel;
		data<<(BYTE)nCode;
		data<<(BYTE)m_infos.nIsOpen;
		data<<(BYTE)m_infos.nAuto;
		data<<(int)m_infos.nQid;
		data<<(int)m_infos.nQstate;
		data<<(int)m_infos.nTelNpcId;
		ap.flush();
		memset(&m_infos, 0,sizeof(m_infos));
	}
	
}


void CQuestSystem::SendAllQuestData()
{
	if (m_pEntity == NULL) return;
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);

	//需保证任务数据已经从数据库中载入
	DataPacket << (BYTE)enQuestSystemID << (BYTE)sQueryQuest;
	//遍历任务数据
	size_t nQCountPos = DataPacket.getPosition();

	DataPacket << (int)m_GoingQuestList.size();//任务的数量
	int nCount = 0;
	for (int i = 0; i < m_GoingQuestList.size(); i++)
	{
		QuestItem& item = m_GoingQuestList[i];
		QuestInfoCfg* cfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(item.nQid, item.nState);
		if(!cfg)
			continue;
		if(GetfinishQuestState(item.nQid))
			continue;
		if(returnQuestData(&item, DataPacket))
			nCount++;

	}
	int * pInt = (int*)DataPacket.getPositionPtr(nQCountPos);
	*pInt = nCount;
	AP.flush();
}


bool CQuestSystem::HasQuest(int nQid)
{
	for(int i = 0; i < m_GoingQuestList.size(); i++)
	{
		if(m_GoingQuestList[i].nQid == nQid)
			return true;
	}
	return false;
}



bool CQuestSystem::HasInRingQuest(int nQid)
{
	for(int i = 0; i < m_RingStartQId.size(); i++)
	{
		if(m_RingStartQId[i] == nQid)
			return true;
	}
	return false;
}

/*
此方法尽量不要在高频方法中调用，如要调用，务必尽可能屏筛选掉不必要的循环
对于杀怪任务事件，首先由怪物的归属者处理事件，然后队友再处理
还有处理师徒关系
*/
VOID CQuestSystem::OnQuestEvent(INT_PTR nTargetType, INT_PTR nCount, INT_PTR nId)
{
	if (m_pEntity == NULL) return;
	for( int i = 0; i < m_GoingQuestList.size(); i++)
	{
		QuestItem& item = m_GoingQuestList[i];

		int nQuestType = item.nType / 100;
		int nSubType = item.nType % 100;

		if(nTargetType == nQuestType)
		{
			QuestInfoCfg* cfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(item.nQid, item.nState);
			if(!cfg)
				continue;
			int nSceneId = ((CActor*)m_pEntity)->GetSceneID();
			if(!cfg->checkIdMapidInLimitIds(nId, nSceneId))
			{
				continue;
			}
			if(!cfg->inGroup(nId))
				continue;
			//处理技能 或者其他牵扯到子类型的任务进度
			if(nQuestType == CQuestData::qtEquip && (cfg->GeLimitSubType() != 0 && cfg->GeLimitSubType() <= nId )) {

			} else {
				if(cfg->GeLimitSubType() != 0 && cfg->GeLimitSubType() != nId) {
					continue;
				}
			}
			
			if(nSubType == 1) //
				item.nValue += nCount;
			else {
				if (item.nValue < nCount)
					item.nValue = nCount;
			}
			isDataModify = true;
			if(cfg->nShowSchedule)
			{
				returnClientQuestState(item.nQid, 0);
			}
			if(item.nValue >= cfg->GetLimitValue())
			{
				DealAutoChangeQuestState(2, item.nQid, item.nState, cfg);
				returnClientQuestState(item.nQid, 0);
			}
		}
	}
	
	//如果是杀怪，并且同队伍的成员也有这个任务，则两个人都增加值(杀指定怪、场景怪、等级怪)
	
	if (nTargetType == CQuestData::qtLevel || nTargetType == CQuestData::qtcircle)
	{//如果等级变化了，更新可接任务列表
		CheckCanAcceptQuest();
	}
}


void CQuestSystem::CheckCanAcceptQuest() {
	if (m_pEntity == NULL) return;
	int Job = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
	std::map<int, QuestInfoCfg*>& questlist = GetLogicServer()->GetDataProvider()->GetQuestConfig()->m_vMainRootQuestConfig;
	if(questlist.size() > 0) {

		std::map<int, QuestInfoCfg*>::iterator it = questlist.begin();
		for(; it != questlist.end(); it++) {

			QuestInfoCfg* cfg = it->second;
			int nJob = cfg->nQid /QUESTMAIN;
			if( (nJob != 0 && nJob != Job) || HasQuest(cfg->nQid) || GetfinishQuestState(cfg->nQid) || HasInRingQuest(cfg->nQid))
				continue;
			if( (cfg->nFrontQid != 0 && !GetfinishQuestState(cfg->nFrontQid)))
				continue;

			// 等级检查
			CActor* pActor = (CActor *)m_pEntity;
			int level = pActor->GetLevel();
			int circle = pActor->GetCircleLevel();
			if (cfg->nQid == 1 && cfg->nState == 101 && circle >= 2)
			{
				continue;
			}
			if(GetLogicServer()->CheckOpenServer(cfg->nOpenDayLimit) && m_pEntity->CheckLevel(cfg->nLevelLimit, cfg->nCircle))
			 { 
				AddQuest(cfg);
			}
		}	
	}



}


int CQuestSystem::GetfinishQuestState(int nQid)
{
	int nState = 0;
	nQid = nQid%QUESTMAIN;
	int nIndex = nQid/8;
	int nbit = nQid%8 -1;
	if(nbit < 0)
		nbit = 7;
	if((nIndex >= 0) || (nIndex < QUESTBTYE))
	{

		BYTE nValue = m_FinishedQuest[nIndex];
		nState = (int)( nValue & (1 << nbit));
	}

	return nState;
}

void CQuestSystem::SetfinishQuest(int nQid)
{
	nQid = nQid%QUESTMAIN;
	int nIndex = nQid/8;
	int nbit = nQid%8 -1;
	if(nbit < 0)
		nbit = 7;
	if((nIndex >= 0) || (nIndex < QUESTBTYE))
	{
		BYTE nValue = m_FinishedQuest[nIndex];
		nValue |= (1 << nbit);
		m_FinishedQuest[nIndex] = (BYTE)nValue;
	}
	
}

void CQuestSystem::ChangeQuestItemState(int nQid, int nState, Uint64 npcHandle)
{
	if(!m_pEntity) return;
	int nCode =  0;
	do
	{
		if(GetfinishQuestState(nQid)) {
			nCode = 1; 
			break; 
		}

		if(!HasQuest(nQid)) {
			nCode = 2; 
			break; 
		}
		QuestItem* item = GetGoingQuestData(nQid);
		if(!item || item->nState != nState) {
			nCode = 5;
			break;
		}

		QuestInfoCfg* cfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(nQid, nState);
		if(!cfg) {
			nCode = 3;
			break;
		}

		if(!GetLogicServer()->CheckOpenServer(cfg->nOpenDayLimit) || !m_pEntity->CheckLevel(cfg->nLevelLimit, cfg->nCircle)) {
			nCode = 4;
			break;
		}
		//策划 王自宜 不需要判断npc 距离
		// TeleportInfo* tel = cfg->GetTelInfo(1);
		// if(tel && tel->nNpcId > 0)
		// {
		// 	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(npcHandle);
		// 	if (!pEntity || !((CActor*)m_pEntity)->CheckTargetDistance((CActor*)pEntity, 15))
		// 	{
		// 		nCode = 14;
		// 		break ;
		// 	}
		// }

		if(!CheckComplete(item, cfg->mQuestInfo.nValue)) {
			nCode = 6;
			break;
		}

		DealAutoChangeQuestState(1, nQid, nState, cfg);
		
	}while(false);
	// //客户端需求 需要改变任务状态进行推送
	// SendClientQuestState(nQid);
	returnClientQuestState(nQid, nCode);
}

void CQuestSystem::returnClientQuestState(int nQid, int nCode)
{
	QuestItem* item = GetGoingQuestData(nQid);
	if(item)
	{
		CActorPacket ap;
		CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(ap);
		data <<(BYTE)enQuestSystemID <<(BYTE)sChangetQuestState;
		data <<(BYTE)nCode;
		returnQuestData(item, data);
		ap.flush();
	}
}

void CQuestSystem::SendClientQuestState(int nQid)
{
	QuestItem* item = GetGoingQuestData(nQid);
	if(item)
	{
		CActorPacket ap;
		CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(ap);
		data <<(BYTE)enQuestSystemID <<(BYTE)sChangeQuestState;
		returnQuestData(item, data);
		ap.flush();
	}
}


void CQuestSystem::DealAutoChangeQuestState(int Type, int nQid, int nState, QuestInfoCfg* cfg)
{
	if(!cfg) return;
	
	if(cfg->awards.size() > 0)
	{
		if(!m_pEntity->GetBagSystem().bagIsEnough(BagRequest))
		{
			m_pEntity->SendTipmsgFormatWithId(tmLeftBagNumNotEnough,tstUI);
			return;
		}

		for(int k = 0; k < cfg->awards.size(); k++)
		{
			((CActor*)m_pEntity)->GiveAward(cfg->awards[k].nType, cfg->awards[k].nId,cfg->awards[k].nCount,0,0,0,0, GameLog::Log_Quest);
		}
	}
	int Job = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
	std::vector<ChangeQuestState> Lists;
	Lists.clear();
	if(Type == 1)
		Lists = cfg->button;
	else
		Lists = cfg->nAuto;
	
	if(Lists.size() > 0) {

		for(int i = 0; i < Lists.size(); i++) {

			if(!ChangeGoingQuestState(Lists[i].nQid, Lists[i].nState)) {

				QuestInfoCfg* newCfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(Lists[i].nQid, Lists[i].nState);
				if(!newCfg)
					continue;
				int nJob = newCfg->nQid /QUESTMAIN;
				if( nJob != 0 && nJob != Job)
					continue;
				if(GetLogicServer()->CheckOpenServer(newCfg->nOpenDayLimit) && m_pEntity->CheckLevel(newCfg->nLevelLimit, newCfg->nCircle)) {
					AddQuest(newCfg);
				}	
			}
		}
	}


	if(cfg->nIscomplete) {

		if(cfg->nAnnular == 0)
			SetfinishQuest(nQid);
		
		//记录状态
		GetLogicServer()->SendTaskLocalLog(nQid, ((CActor*)m_pEntity), cfg->ntasktype, 2);
		RemoveGoingQuestData(nQid);
		CActorPacket ap;
		CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(ap);
		data <<(BYTE)enQuestSystemID <<(BYTE)sDeleteQuest;
		data <<(int)nQid;
		data <<(int)nState;
		ap.flush();
		CScriptValueList paramList,retList;
		static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		paramList << (int)enOnCompleteQuest;
		paramList << (int)nQid;
		paramList << m_pEntity;
		if(!globalNpc->GetScript().CallModule("QuestDispatcher","OnEvent", paramList, retList, 1))
		{
			OutputMsg(rmError,"[QuestSystem] OnEvent CheckEnter 错误，任务ID=%d!",nQid);
		}
	// 	if(retList.count() >= 0 && (bool)retList[0] == false)
	// 	{
	// 		return;
	// 	}
	}

}


QuestItem* CQuestSystem::GetGoingQuestData(int nQid)
{
	for(int i = 0; i < m_GoingQuestList.size(); i++)
	{
		if(m_GoingQuestList[i].nQid == nQid)
			return &m_GoingQuestList[i];
	}

	return NULL;
}

void CQuestSystem::RemoveGoingQuestData(int nQid)
{
	std::vector<QuestItem>::iterator it = m_GoingQuestList.begin();
	for(; it != m_GoingQuestList.end();)
	{
		if((*it).nQid == nQid)
		{
			it =m_GoingQuestList.erase(it);
		}
		else
		{
			it++;
		}
	}
	isDataModify = true;
}


bool CQuestSystem::ChangeGoingQuestState(int nQid, int nState)
{
	QuestItem* item = GetGoingQuestData(nQid);
	if(item)
	{
		QuestInfoCfg* cfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(nQid, nState);
		if(!cfg) {
			return false;
		}
		item->nState = nState;
		item->nType = cfg->GeLimitType();
		//客户端需求 需要改变任务状态进行推送
		SendClientQuestState(nQid);
		if (cfg->nAcceptState)
			GetLogicServer()->SendTaskLocalLog(nQid, ((CActor*)m_pEntity), cfg->ntasktype, 1);
		DealAutoCompleteCfgValue(nQid);
		isDataModify = true;
		return true;
	}
	return false;
}


bool CQuestSystem::returnQuestData(QuestItem* item, CDataPacket& DataPacket)
{
	if(item )
	{
		QuestInfoCfg* cfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(item->nQid, item->nState);
		if(!cfg) {
			return false;
		}
		DataPacket << item->nQid;
		DataPacket << item->nState;
		DataPacket << cfg->GetLimitValue();
		DataPacket << item->nValue;
		QuestAward pCost;
		cfg->GetCostFsValue(item->nUseFsTimes, pCost);
		// QuestCostFlyshoe* pCost = cfg->GetCostFsValue(item->nUseFsTimes);
		DataPacket <<(int)(pCost.nCount);
		return true;
	}
	return false;
}

//重载
bool CQuestSystem::ChangeGoingQuestState(std::vector<ChangeQuestState>& nChangeList)
{
	if(nChangeList.size() > 0)
	{
		for(int i = 0; i < nChangeList.size(); i++)
		{
			ChangeGoingQuestState(nChangeList[i].nQid, nChangeList[i].nState);
		}
	}
}


bool CQuestSystem::CheckComplete(QuestItem* item, int nLimit)
{
	if(nLimit == 0) return true;
	if(!item) return false;

	if(item->nValue >= nLimit)
		return true;

	return false;
}


bool CQuestSystem::AddQuest(QuestInfoCfg* cfg)
{
	if(!cfg) return false;
	QuestItem item;
	item.nQid = cfg->nQid;
	item.nType = cfg->mQuestInfo.nType;
	item.nState = cfg->nState;
	if (cfg->nAcceptState)
		GetLogicServer()->SendTaskLocalLog(cfg->nQid, ((CActor*)m_pEntity), cfg->ntasktype, 1);
	//记录接取状态
	// GetLogicServer()->SendTaskLocalLog(cfg->nQid, ((CActor*)m_pEntity), cfg->mQuestInfo.nType, cfg->nState);
	if(CheckComplete(&item, cfg->mQuestInfo.nValue))
	{
		ChangeGoingQuestState(cfg->nAuto);
	}
	
	if(cfg->nAnnular)
	{
		item.nStartId = cfg->nAnnular;
		m_RingStartQId.push_back(item.nStartId);
	}
	m_GoingQuestList.push_back(item);
	
	if (!((CActor*)m_pEntity)->OnGetIsTestSimulator()
		&& !((CActor*)m_pEntity)->OnGetIsSimulator())
	{ 
		SendAddNewQuest(&item);
	}
	DealAutoCompleteCfgValue(cfg->nQid);
	isDataModify = true;
	return true;
}

void CQuestSystem::SendAddNewQuest(QuestItem* item)
{
	CActorPacket ap;
	CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data <<(BYTE)enQuestSystemID <<(BYTE)sAddQuest;
	returnQuestData(item, data);
	ap.flush();
}
void CQuestSystem::Save(PACTORDBDATA  pActorData)
{
	
	memcpy(pActorData->quest, m_FinishedQuest,sizeof(m_FinishedQuest));
	SaveData();
}

//保存玩家成就完成进度情况
void CQuestSystem::SaveData()
{
	if(!m_pEntity) return;

    // if(!isInitData) return;
    
    if(!isDataModify) return;

    CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveQuest);
    dataPacket <<((CActor*)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();

    dataPacket <<(unsigned int)m_pEntity->GetId(); //玩家id
    size_t count = m_GoingQuestList.size();
    dataPacket <<(int)count;
    for(int i = 0; i < count; i++)
    {	
		QuestItem & item = m_GoingQuestList[i];
        dataPacket.writeBuf(&item, sizeof(item));
    }
    GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

    isDataModify = false;
}



void CQuestSystem::ClearAllQuest()
{
	m_GoingQuestList.clear();
	memset(m_FinishedQuest, 0 ,sizeof(m_FinishedQuest));
	m_RingStartQId.clear();
	CheckCanAcceptQuest();
}


bool CQuestSystem::GMSetQuestState(int nQid, int nState)
{
	if(!ChangeGoingQuestState(nQid, nState)) {

		QuestInfoCfg* newCfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(nQid, nState);
		if(newCfg) {
			AddQuest(newCfg);
		}	
	}

	QuestItem* item = GetGoingQuestData(nQid);
	if(item)
	{
		QuestInfoCfg* newCfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(nQid, nState);
		if(newCfg && item->nValue < newCfg->GetLimitValue())
			item->nValue = 0;
		returnClientQuestState(nQid, 0);
	}
	return true;
}



//对于等级 装备等 需要自动判定是否已经完成了任务
void CQuestSystem::DealAutoCompleteCfgValue(int nQid)
{
	QuestItem* item = GetGoingQuestData(nQid);
	if(item)
	{
		QuestInfoCfg* cfg = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetQuestConfig(nQid, item->nState);
		if(!cfg) {
			return;
		}
		int nQuestType = cfg->GeLimitType() / 100;
		bool nFalg = false;
		if(nQuestType== CQuestData::qtEquip) 
		{
			m_pEntity->GetEquipmentSystem().DealEquipmentAchieve();
		}
		else if(nQuestType == CQuestData::qtLevel)
		{

			OnQuestEvent(CQuestData::qtLevel, ((CActor*)m_pEntity)->GetLevel());
		}
		else if(nQuestType == CQuestData::qtcircle)
		{
			if(cfg->GetLimitValue() <= ((CActor*)m_pEntity)->GetCircleLevel() )
			{
				OnQuestEvent(CQuestData::qtcircle, ((CActor*)m_pEntity)->GetCircleLevel());
			}
		}
		else if(nQuestType == CQuestData::qtSkillLv)
		{
			int nSkillLv = m_pEntity->GetSkillSystem().GetSkillLevel(cfg->GeLimitSubType());
			if(cfg->GetLimitValue() <= nSkillLv )
			{
				OnQuestEvent(CQuestData::qtSkillLv, nSkillLv, cfg->GeLimitSubType());
			}
		}
		else if(nQuestType == CQuestData::qtBless)
		{
			int nBlessValue = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_Bless);
			int nBlessLv = 0;
			BlessCfg* pBlessCfg = GetLogicServer()->GetDataProvider()->GetBlessCfg().GetBlessCfg(nBlessValue);
			if(pBlessCfg)
			{
				nBlessLv = pBlessCfg->nLevel;
			}

			OnQuestEvent(CQuestData::qtBless, nBlessLv, cfg->GeLimitSubType());
		}
	}
}
