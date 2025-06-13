#include "StdAfx.h"
#include "CStrengthenSystem.h"

bool CStrengthenSystem::Initialize(void* data, size_t size)
{
    isInitData = false;
    isDataModify = false;
    m_nStrengths.clear();
    return true;
}
void CStrengthenSystem::OnEnterGame()
{
    
}



void CStrengthenSystem::SaveToDb()
{
    if(!m_pEntity) return;

    if(!isInitData) return;
    
    if(!isDataModify) return;

    CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveActorStrengthenInfo);
    dataPacket <<((CActor*)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();

    dataPacket <<(unsigned int)m_pEntity->GetId(); //玩家id
    size_t count = 0;
    int nPos = dataPacket.getPosition();
    dataPacket <<(int)count;
    strongMapIter it = m_nStrengths.begin();
   
    for(; it != m_nStrengths.end(); it++)
    {
        std::vector<StrengthInfo>& v_list = it->second;
        for(int i = 0; i < v_list.size(); i++)
        {
            StrengthInfo& info = v_list[i];
            dataPacket <<(int)(it->first);
            dataPacket <<(int)(info.nId);
            dataPacket <<(int)(info.nLv);
            count++;;
        }
    }
    int *Ptr = (int*)dataPacket.getPositionPtr(nPos);
	*Ptr = (int)count;
    GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

    isDataModify = false;

}

void CStrengthenSystem::OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet)
{
    if(!m_pEntity) return;

    if(nCmd == jxInterSrvComm::DbServerProto::dcLoadActorStrengthenInfo && nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回数据
    {
         int nCount = 0;
        packet >> nCount;
        m_nStrengths.clear();
        for(int i = 0; i < nCount; i++)
        {
            int nType = 0; 
            StrengthInfo it;
            packet >> nType >>it.nId >> it.nLv;
            if(it.nId > 0)
                m_nStrengths[nType].push_back(it);
        }
    }
    isInitData = true;
    ((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_STRENGTHEN_SYSTEM); //完成一个步骤
}

void CStrengthenSystem::ProcessNetData(INT_PTR nCmd, CDataPacketReader& packet)
{
    switch (nCmd)
    {
    case cGetStrengthenSystemInfo:
        {
        GetStrongInfo(packet);
        break;
        }
    case cUpStrengthenSystemInfo:
        {
            UpStrong(packet);
            break;
        }
    default:
        break;
    }
}


void CStrengthenSystem::GetStrongInfo(CDataPacketReader& packet) {
    
    if(!m_pEntity) return;
    BYTE index = 0;
    packet >> index;

    strongMapIter it = m_nStrengths.find(index);
    CActorPacket ap ;
    CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
    data <<(BYTE)GetSystemID()<<(BYTE)sGetStrengthenSystemInfo;
    BYTE nCount = 0;
    data <<(BYTE)(index);
    int pos = data.getPosition();
    data <<(BYTE)(nCount);
    if(it != m_nStrengths.end()) {
       
        nCount = it->second.size();
        if(nCount > 0) {
            
            strongVectorIter iter = it->second.begin();
            for(; iter != it->second.end(); iter++)
            {
                StrengthInfo& info = *iter;
                data <<(BYTE)(info.nId);
                data <<(int)(info.nLv);
            }
        }
    }
    BYTE *pCount = (BYTE *)data.getPositionPtr(pos);
    *pCount = (BYTE)nCount;
    ap.flush();
}

void CStrengthenSystem::UpStrong(CDataPacketReader& packet)
{
    if(!m_pEntity) return;
    BYTE nType = 0;
    BYTE nPos = 0;
    packet >> nType >> nPos;
    int nCode = 0;
    int nLevel = 0;
    int nNowLv = 0;
    // StrengthInfo* pInfo = nullptr;
    StrengthenInfo* pCfg  = nullptr;
    do {
        
        //判断当前发送的数据是否存在
        int nJob = ((CActor*)m_pEntity)->GetJob();
        pCfg = GetLogicServer()->GetDataProvider()->GetStrengthen().getPstrengthenCfg(nType, nPos, 1,nJob);
        if(!pCfg) {

            nCode = 1;
            break;
        }
        //获取
        nNowLv = GetStrongLv(nType, nPos);
        // if(!pInfo) { //--不存在就新加

        //     pInfo = addNewStrongInfo(nType, nPos);
        // }

        // if(!pInfo)
        // {
        //     nCode = 2;
        //     break;
        // }
        //配置

        pCfg = GetLogicServer()->GetDataProvider()->GetStrengthen().getPstrengthenCfg(nType, nPos, nNowLv+1,nJob);
        if(!pCfg) {

            nCode = 3;
            break;
        }

        if(nType == 1)
        {
            int nNum = m_pEntity->GetStaticCountSystem().GetStaticCount(nSTRENGTHRN_NUM_1);
            if (nNum && (pCfg->nId - 1) != nNum)
            {
                nCode = 7;
                break;
            }
            
        }
        // if(nNowLv != 0)
        // {
        //     StrengthenInfo * pNowCfg = GetLogicServer()->GetDataProvider()->GetStrengthen().getPstrengthenCfg(nType, nPos, nNowLv);
        //     if(!pNowCfg || (pNowCfg->nId != 0 && (pCfg->nId - pNowCfg->nId > 1 ) ) ) {

        //         nCode = 7; //异常操作
        //         break;
        //     }
        // }

        if(pCfg->limit.size()) 
        {
            for(int i = 0; i < pCfg->limit.size(); i++) 
            {
                StrengthUpLimit& limit = pCfg->limit[i];
                if(limit.nPos) 
                {
                    if(!CheckStrengthenLv(nType, limit.nPos, limit.nLimitlv) )
                    {
                        nCode = 4;
                        break;
                    }
                }
                else
                {
                    if(!((CActor*)m_pEntity)->CheckLevel(limit.nLimitlv, limit.zsLevel)) 
                    {
                        nCode = 5;
                        break;
                    }
                }
            }
        }

        if(nCode)
            break;
        
        if(pCfg->cost.size()) {

            for(int i = 0; i < pCfg->cost.size(); i++) {

                StrengthCost& cost = pCfg->cost[i];
                if(!m_pEntity->CheckActorSource(cost.nType, cost.nId, cost.nCount, tstUI) ) {
                    nCode = 6;
                    break;
                }
            }
        }

        if(nCode)
            break;
        int nLogId = GameLog::Log_Strengthen;
        if(nType == 2)
            nLogId = GameLog::Log_FourStarStrength;
        else if(nType == 3)
            nLogId = GameLog::Log_CirtRing;
        else if(nType == 4)
            nLogId = GameLog::Log_DizzyRing;
        else if(nType == 5) //  字诀
            nLogId = GameLog::Log_Words;

        
        if(pCfg->cost.size()) {

            for(int i = 0; i < pCfg->cost.size(); i++) {

                StrengthCost& cost = pCfg->cost[i];
                m_pEntity->RemoveConsume(cost.nType, cost.nId, cost.nCount, -1, -1,-1,0, nLogId);
            }
        }
        if(strlen(pCfg->nTips) > 0) {
            char sText[1024];
            sprintf_s(sText, sizeof(sText), pCfg->nTips, ( char *)((CActor*)m_pEntity->GetEntityName()));
            GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(sText,tstKillDrop);
        }
			
        nNowLv++;
        SetStrengthenLv(nType, nPos, nNowLv);
        ((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);	
        isDataModify = true;
        int lv = nNowLv;
        if(nType == 2)//四象
        {
            lv = 1;//AddlvByType(nType);
        }
        ((CActor*)m_pEntity)->GetAchieveSystem().ActorAchievementComplete(nAchieveStrengthenLv, lv, nType);
        if(nType == 1)
        {
            m_pEntity->GetStaticCountSystem().SetStaticCount(nSTRENGTHRN_NUM_1, pCfg->nId);
        }
    } while (false);
    
    CActorPacket ap ;
    CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
    data <<(BYTE)GetSystemID()<<(BYTE)sUpStrengthenSystemInfo ;
    data <<(BYTE)(nCode);
    data <<(BYTE)(nType);
    data <<(BYTE)(nPos);
    data <<(int)(nNowLv);
    ap.flush();
}


StrengthInfo* CStrengthenSystem::GetStrongPtrInfo(int nType, int nPos) {
    
    strongMapIter it = m_nStrengths.find(nType);

    if(it != m_nStrengths.end()) {
        std::vector<StrengthInfo>& v_list = it->second;
       for(int i = 0; i < v_list.size(); i++)
       {
            if(v_list[i].nId == nPos)
                return &v_list[i];
       }
    }

    return NULL;
}



int CStrengthenSystem::GetStrongLv(int nType, int nPos) {
    
    strongMapIter it = m_nStrengths.find(nType);

    if(it != m_nStrengths.end()) {
        std::vector<StrengthInfo>& v_list = it->second;
       for(int i = 0; i < v_list.size(); i++)
       {
            if(v_list[i].nId == nPos)
                return v_list[i].nLv;
       }
    }

    return 0;
}



bool CStrengthenSystem::CheckStrengthenLv(int nType, int nPos, int nLv) {
    
    strongMapIter it = m_nStrengths.find(nType);

    if(it != m_nStrengths.end()) {
        std::vector<StrengthInfo>& v_list = it->second;
       for(int i = 0; i < v_list.size(); i++)
       {
            if(v_list[i].nId == nPos && v_list[i].nLv >= nLv)
                return true;
       }
    }
    return false;
}


void CStrengthenSystem::SetStrengthenLv(int nType, int nPos, int nLv) {
    
    strongMapIter it = m_nStrengths.find(nType);
    StrengthInfo info;
    info.nLv = nLv;
    info.nId = nPos;
    bool isNew = true;
    if(it != m_nStrengths.end()) {
        std::vector<StrengthInfo>& v_list = it->second;
        for(int i = 0; i < v_list.size(); i++)
        {
                if(v_list[i].nId == nPos)
                {
                    v_list[i].nLv = nLv;
                    isNew = false;
                }
        }
        if(isNew)
            it->second.push_back(info);
    }
    else
    {
        m_nStrengths[nType].push_back(info);
    }
    
}



VOID CStrengthenSystem::CalcAttributes(CAttrCalc &calc)
{
	if(m_pEntity ==NULL) return ;
	GAMEATTR attr;

	strongMapIter it = m_nStrengths.begin();
    int nJob = ((CActor*)m_pEntity)->GetJob();
    for(; it != m_nStrengths.end(); it++)
    {
        std::vector<StrengthInfo>& v_list = it->second;
        for(int i = 0; i < v_list.size(); i++)
        {
            StrengthInfo& info = v_list[i];
            StrengthenInfo* pCfg = GetLogicServer()->GetDataProvider()->GetStrengthen().getPstrengthenCfg(it->first, info.nId, info.nLv,nJob);
            if(pCfg) {
                for(int  i = 0; i < pCfg->attri.nCount; i++)
                {
                    calc << pCfg->attri.pAttrs[i];
                }
            } 
        }
    }
}

VOID CStrengthenSystem::returnStrongInfo(CDataPacket &data)
{
    data <<(BYTE)(m_nStrengths.size());
	strongMapIter it = m_nStrengths.begin();
   
    for(; it != m_nStrengths.end(); it++)
    {
        std::vector<StrengthInfo>& v_list = it->second;
        data <<(BYTE)(it->first);
        data <<(BYTE)(v_list.size());
        for(int i = 0; i < v_list.size(); i++)
        {
            StrengthInfo& info = v_list[i];
            data<<(BYTE)(info.nId);
            data<<(int)(info.nLv);
        }
    }
}



bool CStrengthenSystem::checkCanUPStrengthenLv(int nType, int nLv) {
    if(nLv == 0)
        return true;
    
    strongMapIter it = m_nStrengths.find(nType);
    int Lv = 0;
    if(it != m_nStrengths.end()) {
        std::vector<StrengthInfo>& v_list = it->second;
        for(int i = 0; i < v_list.size(); i++)
        {
            if(v_list[i].nLv < nLv)
                return false;
        }
        
        return true;
    }
    return false;    
}



int CStrengthenSystem::AddlvByType(int nType) {
    strongMapIter it = m_nStrengths.find(nType);
    int Lv = 0;
    // int sub = 1;
    // if(nType == 2)
    //     sub = 30;
    if(it != m_nStrengths.end()) {
        std::vector<StrengthInfo>& v_list = it->second;
        for(int i = 0; i < v_list.size(); i++)
        {
            Lv += v_list[i].nLv;
        }
    }
    return Lv;    
}


//跨服数据
void CStrengthenSystem::SendMsg2CrossServer(int nType)
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
	size_t count = 0;
    int pos = packet.getPosition();
	packet << (int)count  ; //玩家物品的数量
	for(auto it : m_nStrengths) {

        for(auto ik : it.second) {
            packet << (int)it.first;
            packet << (int)ik.nId;
            packet << (int)ik.nLv;
            count++;
        }
    }
    int *Ppos = (int*)packet.getPositionPtr(pos);
    *Ppos = count;
	pCrossClient->flushProtoPacket(packet);
}


void CStrengthenSystem::OnCrossInitData(std::map<int, std::vector<StrengthInfo> >& nStrengths)
{
    if(!m_pEntity) return;

    {
         int nCount = nStrengths.size();
        // packet >> nCount;
        m_nStrengths.clear();
        // for(int i = 0; i < nCount; i++)
        // {
        //     int nType = 0; 
        //     StrengthInfo it;
        //     packet >> nType >>it.nId >> it.nLv;
        //     if(it.nId > 0)
        //         m_nStrengths[nType].push_back(it);
        // }

        for(auto& Values: nStrengths) {

            for(auto info : Values.second) {

                if(info.nId > 0)
                    m_nStrengths[Values.first].push_back(info);

                OutputMsg(rmTip,_T("OnCrossInitData CStrengthenSystem nActorID:%d nId:%d,nLv:%d"),
						m_pEntity->GetId(),info.nId,info.nLv);    
            }
        }
    }
    isInitData = true;
    isDataModify = true;
    ((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CSTRENGTH); //完成一个步骤
}