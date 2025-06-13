#include "StdAfx.h"
#include "GhostSystem.h"

bool CGhostSystem::Initialize(void* data, size_t size)
{
    isInitData = false;
    isDataModify = false;
    m_nGhostLvs.clear();
     
    //请求加载活动数据
    CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected())
    {
        CDataPacket& dataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGhostInfo);
        int nServerId = GetLogicServer()->GetServerIndex();
        dataPacket << ((CActor*)m_pEntity)->GetRawServerIndex() << nServerId;
	    dataPacket << (int)(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));

		if (!((CActor*)m_pEntity)->OnGetIsTestSimulator())
		{         
            GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket); 
        }    
      }
    return true;
}


void CGhostSystem::OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet)
{
    if(!m_pEntity) return;
    if(isInitData) return;
    if(nCmd == jxInterSrvComm::DbServerProto::dcLoadGhostInfo && nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回数据
    {
        int nCount = 0;
        packet >> nCount;
        m_nGhostLvs.clear();
        for(int i = 0; i < nCount; i++)
        {
            GhostData data;
            packet >> data.nId >> data.nLv >> data.nBless;
            m_nGhostLvs[data.nId] = data;
        }
    }
    isInitData = true;
    ((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_GHOST_SYSTEM); //完成一个步骤
}

void CGhostSystem::SaveToDb()
{
    if(!m_pEntity) return;

    if(!isInitData) return;
    
    if(!isDataModify) return;

    CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveGhostInfo);
    dataPacket <<((CActor*)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();

    dataPacket <<(unsigned int)m_pEntity->GetId(); //玩家id
    size_t count = m_nGhostLvs.size();
    dataPacket <<(int)count;
    for(auto it : m_nGhostLvs) {
        // dataPacket << it.second;
        GhostData& data = it.second;
        dataPacket <<data.nId;
        dataPacket <<data.nLv;
        dataPacket << data.nBless;
    }
    GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

    isDataModify = false;

}

void CGhostSystem::ProcessNetData(INT_PTR nCmd, CDataPacketReader& packet)
{
    switch (nCmd)
    {
        case cGetGHostInfos:
        {
            sendGhostInfo();
            break;
        }
        case cUpGhostLv:
        {
            upGhostLv(packet);
            break;
        }
    default:
        break;
    }
}

void CGhostSystem::sendGhostInfo()
{
    if(!m_pEntity) return;
    CActorPacket ap;
    CDataPacket& dataPack = ((CActor*)m_pEntity)->AllocPacket(ap);
    dataPack << (BYTE)enGhostSystemID <<(BYTE)sGetGHostInfos;
    // int nCount = m_nGhostLvs.size();
    // dataPack << (BYTE)nCount;
    // for(auto it : m_nGhostLvs)
    // {
    //     GhostData& data = it.second;
    //     dataPack <<(BYTE) data.nId;
    //     dataPack <<data.nLv;
    //     dataPack << data.nBless;
    // }
    Data2Packet(dataPack);

    ap.flush();

}

void CGhostSystem::upGhostLv(CDataPacketReader& packet)
{
    if(m_pEntity == NULL) return;
    BYTE nPos = 0; 
    packet >> nPos;
    byte nReplace = 0; //1替换 0 ：不替换
	packet >> nReplace;
    CActor* pActor = (CActor*)m_pEntity;
    int nErrorCode = 0;
    do
    {
        GhostDataCfg &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_nGhostCfg;
        int nLv = GetLvByPos(nPos);

        // int nMaxLevel = cfg.m_gHostUpLvCfg.size();
        // if(nLv >= nMaxLevel)
        // {
        //     nErrorCode = 1;
        //     break;
        // }
        GhostConfig* it = cfg.GetGhostLvCfg(nPos, nLv+1);
        if(!it)
        {
            nErrorCode = 1;
            break;
        }
        GhostConfig& levelCfg = (*it);//
        int nDay = GetLogicServer()->GetDaysSinceOpenServer();
        if(nDay < levelCfg.nServerOpenDay)
        {
            nErrorCode = 2;
            break;
        }

        if(!pActor->CheckLevel(levelCfg.nLevelLimit, levelCfg.nCircle))
        {
            nErrorCode = 3;
            break;
        }
        int nBless = GetBlessByPos(nPos);
        if(nBless < cfg.nMaxBless)
        {
            if(cfg.nProbability > 0)
                nBless = (nBless / cfg.nProbability) * cfg.nAddProbability;
        }
        nBless += levelCfg.nProbability;

        int nCostYb = 0;
		if(levelCfg.cost.size())
		{
			for(int i = 0; i < levelCfg.cost.size(); i++)
			{
				bool bReplace = false;
				ACTORAWARD& table = levelCfg.cost[i];
				gHostReplaceCfg* res = GetLogicServer()->GetDataProvider()->GetMiscConfig().getGhostReplaceCfg(table.wId);
				if(res)
				{
					if(nReplace & (1<< res->nId))
					{
						bReplace = true;
						nCostYb += table.wCount*(res->nPrice);
					}
				}
                int tips = 0;
                if(!bReplace)
                    tips = tstUI;
                if(!pActor->CheckActorSource(table.btType, table.wId, table.wCount, tips))
                {
                    if(!bReplace)
                    {
                        nErrorCode = 4;
                        return;
                    }

                }
			}
		}
		
		if(nCostYb)
		{
			if(!pActor->CheckActorSource(qatYuanbao, qatYuanbao, nCostYb, tstUI))
			{
				nErrorCode = 5;
				return;
			}
		}

		if(levelCfg.cost.size())
		{
			for(int i = 0; i < levelCfg.cost.size(); i++)
			{
				bool bReplace = false;
				ACTORAWARD& table =levelCfg.cost[i];
				gHostReplaceCfg* res = GetLogicServer()->GetDataProvider()->GetMiscConfig().getGhostReplaceCfg( table.wId);
				if(res)
				{
					if(nReplace & (1<< res->nId))
						bReplace = true;
				}
				if(!bReplace)
				{
					pActor->RemoveConsume(table.btType, table.wId, table.wCount, -1, -1,-1,0, GameLog::Log_ghost);
				}
				
			}
		}
		if(nCostYb)
		{
			pActor->RemoveConsume(qatYuanbao, qatYuanbao, nCostYb, -1, -1,-1,0, GameLog::Log_ghost);
		}
        
        // if(levelCfg.cost.size() > 0) {
        //     for(int i = 0; i < levelCfg.cost.size(); i++) {
        //         ACTORAWARD& cost = levelCfg.cost[i];
        //         if(!pActor->CheckActorSource(cost.btType, cost.wId, cost.wCount, tstUI))
        //             return;
        //     }
        // }
        
        // if(levelCfg.cost.size() > 0) {
        //     for(int i = 0; i < levelCfg.cost.size(); i++) {
        //         ACTORAWARD& cost = levelCfg.cost[i];
        //         pActor->RemoveConsume(cost.btType, cost.wId, cost.wCount, -1, -1,-1,0, GameLog::Log_ghost);
        //     }
        // }
         if(wrand(10000) > nBless )
        {
            nErrorCode = 4;
            ((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
            SetLvByPos(nPos,0, cfg.nAddBless);
            break;
        }
        ((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
        SetLvByPos(nPos,nLv+1, cfg.nAddBless);
        if(levelCfg.nTipId) {
            LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(levelCfg.nTipId);
            if( sFormat) {
                char sContent[1024]={0};
                sprintf_s(sContent, sizeof(sContent), sFormat, ( char *)(pActor->GetEntityName()), ( char *)(levelCfg.name), levelCfg.nLevel);
                GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(sContent,tstRevolving);
            }
        }

        if(nLv+1 >= cfg.nMaxLv)
            SendAward();
        
        // CActorPacket ap;
        // CDataPacket& dataPack = pActor->AllocPacket(ap);
        // dataPack << (BYTE)enGhostSystemID <<(BYTE)sUpGhostLv <<(BYTE)nErrorCode;
        // returnPb(nPos, dataPack);
        // ap.flush();
        // return;
        if(nReplace)
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmRefiningReplace,tstUI);
		}
    } while (false);
    
    CActorPacket ap;
    CDataPacket& dataPack = pActor->AllocPacket(ap);
    dataPack << (BYTE)enGhostSystemID <<(BYTE)sUpGhostLv <<(BYTE)nErrorCode;
    returnPb(nPos, dataPack);
    ap.flush();
    
}


int CGhostSystem::GetLvByPos(int nPos)
{
    int nLv = 0;
    if(m_nGhostLvs.count(nPos)) {
        nLv = m_nGhostLvs[nPos].nLv;
    }
    return nLv;
}
int CGhostSystem::GetBlessByPos(int nPos)
{
    int nBless = 0;
    if(m_nGhostLvs.count(nPos)) {
        nBless = m_nGhostLvs[nPos].nBless;
    }
    return nBless;
}

void CGhostSystem::GetAwardNum(int nAddValue)
{
    GhostDataCfg &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_nGhostCfg;
    for(int i = 0; i < cfg.nMaxCount; i++) {
        AddBlessByPos(i+1, nAddValue);
    }
    sendGhostInfo();
}


void CGhostSystem::AddBlessByPos(int nPos, int nAddValue)
{
    if(m_nGhostLvs.count(nPos)) {
        m_nGhostLvs[nPos].nBless += nAddValue;
    }else
    {
        GhostData data;
        data.nId = nPos;
        data.nLv = 0;
        data.nBless = nAddValue;
        m_nGhostLvs[nPos] = data;
    }
    isDataModify = true;
}

void CGhostSystem::returnPb(int nPos, CDataPacket& dataPack)
{
    if(m_nGhostLvs.count(nPos)) {
        GhostData& data = m_nGhostLvs[nPos];
        dataPack <<(BYTE) data.nId;
        dataPack <<data.nLv;
        dataPack << data.nBless;
    }
}

void CGhostSystem::SetLvByPos(int nPos, int nLevel, int nBless)
{
    if(m_nGhostLvs.count(nPos)) {
        m_nGhostLvs[nPos].nLv = nLevel;
        m_nGhostLvs[nPos].nBless += nBless;
    }else
    {
        GhostData data;
        data.nId = nPos;
        data.nLv = nLevel;
        data.nBless = nBless;
        m_nGhostLvs[nPos] = data;
    }

    isDataModify = true;
}


void CGhostSystem::SendAward()
{
    if(!m_pEntity) return;
    GhostDataCfg &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_nGhostCfg;
    int nCount = 0;
    for(auto it : m_nGhostLvs) {
        GhostData& data = it.second;
        if(data.nLv < cfg.nMaxLv) {
            return;
        }
        nCount++;
    }
    if(nCount >= cfg.nMaxCount)
    {
        CVector<ACTORAWARD> awardlist;
        for(auto it : cfg.awards)
        {
            if (it.wId ==0)
            {
                continue;
            }
            awardlist.add(it);
        }
        CMailSystem::SendMailByAward(m_pEntity->GetId(), cfg.sTitle, cfg.sContent, awardlist);
    }
}


VOID CGhostSystem::CalcAttributes(CAttrCalc &calc)
{
	if(m_pEntity ==NULL) return ;
	GAMEATTR attr;
    std::vector<int> v_lists;
    GhostDataCfg &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_nGhostCfg;
    for(auto it : m_nGhostLvs)
    {
        GhostData& data = it.second;
        v_lists.push_back(data.nLv);
        GhostConfig* pCfg = cfg.GetGhostLvCfg(data.nId, data.nLv);
        if(pCfg) {
            for(int  i = 0; i < pCfg->attri.nCount; i++)
            {
                calc << pCfg->attri.pAttrs[i];
            }
        }
    }
    if(v_lists.size()) {
        sort(v_lists.begin(), v_lists.end());
        int nLv = v_lists[0];
        GhostConfig* pCfg = cfg.GetGhostCommonCfg(0, nLv);
        if(pCfg) {
            for(int  i = 0; i < pCfg->attri.nCount; i++)
            {
                calc << pCfg->attri.pAttrs[i];
            }
        }
        nLv = v_lists[v_lists.size()-1];
        pCfg = cfg.GetGhostCommonCfg(1, nLv);
        if(pCfg) {
            for(int  i = 0; i < pCfg->attri.nCount; i++)
            {
                calc << pCfg->attri.pAttrs[i];
            }
        }
    }
}


void CGhostSystem::Data2Packet(CDataPacket& dataPack)
{
    int nCount = m_nGhostLvs.size();
    dataPack << (BYTE)nCount;
    for(auto it : m_nGhostLvs)
    {
        GhostData& data = it.second;
        dataPack <<(BYTE) data.nId;
        dataPack <<data.nLv;
        dataPack << data.nBless;
    }
}

int CGhostSystem::GetSumLv()
{
    int nSum = 0;
    for(auto it : m_nGhostLvs)
    {
        GhostData& data = it.second;
        nSum += data.nLv;
    }
    return nSum;
}


//跨服数据
void CGhostSystem::SendMsg2CrossServer(int nType)
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
    
	size_t count = m_nGhostLvs.size();
    int pos = packet.getPosition();
	packet << (int)count  ; //玩家物品的数量
	for(auto& it : m_nGhostLvs) {

        packet << (int)(it.second.nId);
        packet << (int)(it.second.nLv);
        packet << (int)(it.second.nBless);
    }
	pCrossClient->flushProtoPacket(packet);
}


void CGhostSystem::OnCrossInitData( std::vector<GhostData>& mGhost)
{
    if(!m_pEntity) return;

    
    {
        int nCount = mGhost.size();
        // packet >> nCount;
        m_nGhostLvs.clear();
        for(int i = 0; i < nCount; i++)
        {
            GhostData data = mGhost[i];
            // packet >> data.nId >> data.nLv >> data.nBless;
            m_nGhostLvs[data.nId] = data;
            OutputMsg(rmTip,_T("OnCrossInitData CGhostSystem nActorID:%d nId:%d,nLv:%d"),
						m_pEntity->GetId(),data.nId,data.nLv);    
        }
    }
    isDataModify = true;
    isInitData = true;
    ((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CGHOST); //完成一个步骤
}
