#include "StdAfx.h"
#include "HallowsSystem.h"
#include "config/HallowsConfig.h" 

CHallowsSystem::~CHallowsSystem()
{  
    std::map<int, char*>::iterator it = m_vcRefining.begin();
    for (; it != m_vcRefining.end(); ++it)
    {
        if (it->second)
        {
            GFreeBuffer(it->second);
            it->second = NULL;
        }
    }
    
    it = m_vcBestAttr.begin();
    for (; it != m_vcBestAttr.end(); ++it)
    {
        if (it->second)
        {
            GFreeBuffer(it->second);
            it->second = NULL;
        }
    } 
}
bool CHallowsSystem::Initialize(void* data, size_t size)
{
    m_isInitData = false;
    m_isDataModify = false;
    m_nHallowsSoulWeapon.clear();
 
    int len = 200;
    for (size_t i = 1; i <= GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetSoulWeapon(); i++)
    {
        char* buff = (char*)GAllocBuffer(len+1); 
        memset(&(*buff), 0, sizeof(char) * (len+1));
        m_vcRefining.insert(std::make_pair(i, buff));
        
        char* buff2 = (char*)GAllocBuffer(len+1);
        memset(&(*buff2), 0, sizeof(char) * (len+1));
        m_vcBestAttr.insert(std::make_pair(i, buff2));
    }   
    for (size_t i = eHallowsTotle_SoulWeapon; i < eHallowsTotle_MAX; i++)
    {
        std::map<int, HallowsSoulWeaponInfo>& temp = m_nHallowsSoulWeapon[i];
        for (size_t j = 1; j <= GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetSoulWeapon(); j++)
        {
            if (j >= eHallowsSSC_MAX || j <= eHallowsSSC_NULL)
            {
                continue;
            }
            HallowsSoulWeaponInfo info;
            info.nId = j;
            info.nLorderId = 0;
            info.nLvId = 0;
            info.nStarId = 0; 
            temp.insert(std::make_pair(j, info)); 
        }
    }  
    
    m_isDataModify = true;
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected())
    {
        CDataPacket& dataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadHallowsSystemInfo);
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
 
void CHallowsSystem::OnEnterGame()
{
    
}  
void CHallowsSystem::OnSaveToDb()
{ 
    if(!m_pEntity) 
        return;

    if(!m_isInitData)
        return;
    
    if(!m_isDataModify)
        return;

    CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveHallowsSystemInfo);
    dataPacket <<((CActor*)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();

    dataPacket <<(unsigned int)m_pEntity->GetId(); //玩家id
    size_t count = 0;
    int nPos = dataPacket.getPosition();
    std::map<int, HallowsSoulWeaponInfo>& temp = m_nHallowsSoulWeapon[eHallowsTotle_SoulWeapon];
    count = 0;//temp.size();
    dataPacket <<(int)count; 
    std::map<int, HallowsSoulWeaponInfo>::iterator itSub = temp.begin();
    for(; itSub != temp.end(); itSub++) 
    {
        HallowsSoulWeaponInfo& info = itSub->second;
        if (info.nId >= eHallowsSSC_MAX || info.nId <= eHallowsSSC_NULL)
        {
            continue;
        } 
        dataPacket /*<<(info.nId) */<< info.nLorderId <<info.nStarId<<info.nLvId;  
        
		//OutputMsg(rmNormal, _T("--------------->CHallowsSystem::SaveToDb POS:%d nLorderId=%d, nStarId=%d, nLvId=%d,"), (int)(itSub->first), info.nLorderId, info.nStarId, info.nLvId);
        char *BestBuff = m_vcBestAttr[(int)(itSub->first)]; 
        char *RefiningBuff = m_vcRefining[(int)(itSub->first)]; 
        dataPacket.writeString(BestBuff); 
        dataPacket.writeString(RefiningBuff); 
        count++;//累计
    } 
 
    int *Ptr = (int*)dataPacket.getPositionPtr(nPos);
	*Ptr = (int)count;
    GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

    m_isDataModify = false;

}
 
void CHallowsSystem::OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet)
{
    if(!m_pEntity)
        return;
    if(m_isInitData) return;
    CActor *pActor = (CActor*)m_pEntity;
    if(nCmd == jxInterSrvComm::DbServerProto::dcLoadHallowsSystemInfo && nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回数据
    {
        char cTmpAttr[50]; 
        memset(cTmpAttr, 0, sizeof(cTmpAttr));
        int nCount = 0;
        packet >> nCount;  
        if(nCount != 0)
        {
            std::map<int, HallowsSoulWeaponInfo>& temp = m_nHallowsSoulWeapon[eHallowsTotle_SoulWeapon];
            for(int nPos = eHallowsSSC_JZLR; nPos < eHallowsSSC_MAX; ++nPos)
            {
                std::map<int, HallowsSoulWeaponInfo>::iterator itPos = temp.find(nPos);
                if(itPos == temp.end())
                {
                    continue;
                } 
                HallowsSoulWeaponInfo & info = itPos->second;
                packet >> info.nLorderId >> info.nStarId >> info.nLvId;

                //OutputMsg(rmNormal, _T("++++++++++++++++++++CHallowsSystem::OnDbRetData POS:%d nLorderId=%d, nStarId=%d, nLvId=%d,"), (int)(nPos), info.nLorderId, info.nStarId, info.nLvId);
        
                char *BestBuff = m_vcBestAttr[nPos]; 
                char *RefiningBuff = m_vcRefining[nPos]; 

                memset(cTmpAttr, 0, sizeof(cTmpAttr));
                packet.readString(cTmpAttr, ArrayCount(cTmpAttr));  
                memset(&(*BestBuff), 0, sizeof(char) * (200+1));
                memcpy(BestBuff, cTmpAttr, sizeof(cTmpAttr));
                
                memset(cTmpAttr, 0, sizeof(cTmpAttr));
                packet.readString(cTmpAttr, ArrayCount(cTmpAttr));  
                memset(&(*RefiningBuff), 0, sizeof(char) * (200+1));
                memcpy(RefiningBuff, cTmpAttr, sizeof(cTmpAttr));  
            }
        }
    }
    m_isInitData = true;
    ((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_HALLOWS_SYSTEM); //完成一个步骤14
}

void CHallowsSystem::ProcessNetData(INT_PTR nCmd, CDataPacketReader& packet)
{
    switch (nCmd)
    {
    case cGetHallowshenSystemInfo:
        {
            OnCustomGetSoulWeaponInfo(packet);
            break;
        }
    case cUpHallowshenSystemInfo:
        {
            OnCustomUpHallows(packet);
            break;
        }  
    case cRefiningHallowsSystemInfo://洗炼
        {
            OnCustomRefining(packet);
            break;
        } 
    case cRefiningReplaceHallowsSystemInfo://洗练替换
        {
            OnCustomRefiningReplace(packet);
            break;
        }
    default:
        break;
    }
}
 
void CHallowsSystem::OnCustomGetSoulWeaponInfo(CDataPacketReader& packet) 
{
    if(!m_pEntity)
        return;
    BYTE index = 0;
    packet >> index;

    std::map<int, std::map<int, HallowsSoulWeaponInfo> >::iterator it = m_nHallowsSoulWeapon.find(index);
    CActorPacket ap ;
    CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
    data <<(BYTE)GetSystemID()<<(BYTE)sGetHallowshenSystemInfo;
    BYTE nCount = 0;
    data <<(BYTE)(index);
    int pos = data.getPosition();
    if(it != m_nHallowsSoulWeapon.end()) 
    { 
        std::map<int, HallowsSoulWeaponInfo>& temp = m_nHallowsSoulWeapon[eHallowsTotle_SoulWeapon];
        nCount = temp.size();
        data <<(BYTE)(nCount);
        if(nCount > 0) 
        {
            std::map<int, HallowsSoulWeaponInfo>::iterator iter = temp.begin();
            for(; iter != temp.end(); iter++)
            {
                HallowsSoulWeaponInfo& info = iter->second;
                
                char *RefBuff = NULL;
                char *BestBuff = NULL; 
                std::map<int, char*>::iterator itRef = m_vcRefining.find(info.nId);
                std::map<int, char*>::iterator itBest = m_vcBestAttr.find(info.nId);
        
                // if (itRef == m_vcRefining.end()
                //     || itBest == m_vcBestAttr.end())
                // {
                //     continue;
                // } 
                if(itRef != m_vcRefining.end())
                    RefBuff = itRef->second;
                if(itBest != m_vcBestAttr.end())
                    BestBuff = itBest->second; 

                data <<(BYTE)(info.nId);
                data <<(int)(info.nLorderId);
                data <<(int)(info.nStarId);
                data <<(int)(info.nLvId); 
		        data.writeString(BestBuff); 
		        data.writeString(RefBuff); 
                
            }
        }
    } 
    BYTE *pCount = (BYTE *)data.getPositionPtr(pos);
    *pCount = (BYTE)nCount;
    ap.flush();
}

void CHallowsSystem::OnCustomUpHallows(CDataPacketReader& packet)
{
    if(!m_pEntity) 
        return;

    CActor* pActor = (CActor*)m_pEntity;
    BYTE nType = 0;
    BYTE nPos = 0;
    BYTE nSubType = 0;
    BYTE nReqLv = 0;
    BYTE nReqOp = 0;
    packet >> nType >> nPos >> nSubType >> nReqLv >> nReqOp;
    int nCode = 0;
    int nLevel = 0;
    int nNowLv = nReqLv;
     
    do
    { 
        if(HallowsUpgrade_Lorder == nSubType
            || HallowsUpgrade_Star == nSubType
            || HallowsUpgrade_Lv == nSubType ) 
        { 
            nType == eHallowsTotle_SoulWeapon;
            //获取
            nNowLv = GetHallowsLv(eHallowsTotle_SoulWeapon, nPos, nSubType); 
            if (nNowLv != (int)nReqLv)
            {
                //校验客户端参数 客户端发送的当前等级不对
                nCode = 2;
                break;
            }
            
            int nReqUpLv = nReqLv + 1;//请求的是当前等级 需要服务器自己计算加1
            
            //char sText[1024];//提示
            //memset(sText, 0, sizeof(sText));
            std::vector<HallowsthCost>* pDataCost = NULL;
            std::vector<HallowsUpLimit>* pLimit = NULL;
            if(HallowsUpgrade_Lorder == nSubType ) 
            {  
                HallowsLorderInfo* pCfg = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetLorderInfo((int)nPos, nReqUpLv);
                
                if(!pCfg)
                { 
                    //返回报错不能升级了
                    nCode = 1;
                    break;
                } 
                
                pDataCost = &(pCfg->cost);
                pLimit = &(pCfg->limit);
            }
            if(HallowsUpgrade_Star == nSubType ) 
            {  
                HallowsStarInfo* pCfg = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetStarInfo((int)nPos, nReqUpLv);
                
                if(!pCfg)
                { 
                    //返回报错不能升级了
                    nCode = 1;
                    break;
                } 
                
                pDataCost = &(pCfg->consume);
                pLimit = &(pCfg->limit);
                //if(strlen(pCfg->nTips) > 0)
                //{ 
                //    sprintf_s(sText, sizeof(sText), pCfg->nTips, ( char *)((CActor*)m_pEntity->GetEntityName())); 
                //}
            }
            if(HallowsUpgrade_Lv == nSubType ) 
            {  
                HallowsLvInfo* pCfg = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetLvInfo((int)nPos, nReqUpLv);
                
                if(!pCfg)
                { 
                    //返回报错不能升级了
                    nCode = 1;
                    break;
                } 
                
                pDataCost = &(pCfg->cost);
                pLimit = &(pCfg->limit);
                //if(strlen(pCfg->nTips) > 0)
                //{ 
                //    sprintf_s(sText, sizeof(sText), pCfg->nTips, ( char *)((CActor*)m_pEntity->GetEntityName())); 
                //}
            }
             
           
            if( (HallowsUpgrade_Star == nSubType  //星和级升级要看阶
                || HallowsUpgrade_Lv == nSubType) 
                &&  pLimit
                && (*pLimit).size())
             { 
                for(int i = 0; i < (*pLimit).size(); i++) 
                {
                    HallowsUpLimit& limit = (*pLimit)[i];
                    if(limit.nPos)
                     { 
                        int nCurrLv = GetHallowsLv(eHallowsTotle_SoulWeapon, limit.nPos, HallowsUpgrade_Lorder); //获得当前阶数
                        if (nCurrLv < limit.nLimitlv)
                        {
                            if (HallowsUpgrade_Lv == nSubType) 
                            { 
                                nCode = 7;
                            }
                            else
                            { 
                                nCode = 8;
                            }
                            break;
                        } 
                    }
                    /*  //不做级数的限制
                    else
                     {
                        if(!((CActor*)m_pEntity)->CheckLevel(limit.nLimitlv, 0))
                         {
                            nCode = 8;
                            break;
                        }
                    }
                    */
                }
            }
 
            if(nCode)
                break;

            int nLogId = GameLog::Log_SoulWeapon; 

            if(HallowsUpgrade_Star != nSubType ) 
            {
                if(pDataCost->size()) 
                { 
                    for(int i = 0; i < pDataCost->size(); i++) 
                    {

                        HallowsthCost& cost = (*pDataCost)[i];
                        if(!m_pEntity->CheckActorSource(cost.nType, cost.nId, cost.nCount, tstUI) )
                        {
                            //消耗道具不够
                            nCode = 6;
                            //break;
                        }
                    }
                } 
                if(nCode)
                    break;
                if(pDataCost->size())
                {
                    for(int i = 0; i < pDataCost->size(); i++)
                    {
                        HallowsthCost& cost = (*pDataCost)[i];
                        m_pEntity->RemoveConsume(cost.nType, cost.nId, cost.nCount, -1, -1,-1,0, nLogId);
                    }
                }
            }
            else
            {
                std::map<int, ACTORAWARD> CalcCostAll;
                int nCostYb = 0; 
                if(pDataCost->size())
                {
                    for(int i = 0; i < pDataCost->size(); i++)
                    {
                        bool bReplace = false;
                        HallowsthCost& table = (*pDataCost)[i];
                        WpReplaceInfo* res = GetLogicServer()->GetDataProvider()->GetHallowthen().getReplaceCfgByPos(eHallowsReplaceType_Star, (int)nPos, table.nId); 
                        //ReplaceCfg* res = GetLogicServer()->GetDataProvider()->GetRefiningConfig().getReplaceCfg(table.nId);
                        if(res)
                        {
                            if(nReqOp & (1<< res->idx))
                            {
                                //选择替换方案
                                for(int k = 0; k < res->vPrice.size(); k++)
                                {
                                    HallowsthCost &rep = res->vPrice[k];
                                    if (rep.nType == qatYuanbao)
                                    {
                                        nCostYb += table.nCount * (rep.nCount);
                                    } 
                                    else if (rep.nType == 0)
                                    {   
                                        ACTORAWARD& tmp = CalcCostAll[rep.nId]; 
                                        tmp.btType = rep.nType;
                                        tmp.wId = rep.nId;
                                        tmp.wCount += rep.nCount*table.nCount;
                                    }
                                }
                                bReplace = true;
                            }
                        } 
                        if(!bReplace)
                        {
                            ACTORAWARD& tmp = CalcCostAll[table.nId]; 
                            tmp.btType = table.nType;
                            tmp.wId = table.nId;
                            tmp.wCount += table.nCount;
                        }				
                    }
                }
                
                if(nCostYb)
                {
                    if(!pActor->CheckActorSource(qatYuanbao, qatYuanbao, nCostYb, tstUI))
                    {
                        nCode = 5;
                        break;
                    }
                } 
                if(CalcCostAll.size())
                { 
                    std::map<int, ACTORAWARD>::iterator it = CalcCostAll.begin();
                    for (; it != CalcCostAll.end(); ++it)
                    {  
                        if(!pActor->CheckActorSource((it->second).btType, (it->second).wId, (it->second).wCount, tstUI))
                        { 
                            nCode = 6;  //消耗道具不够
                            //nCode = 4;//道具不够
                            //break;
                        }
                    }
                }
                if(nCode)
                    break;
 
                HallowsStarInfo* pCfg = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetStarInfo((int)nPos, nReqUpLv);
                
                int nDownLv = 0;
                if(wrand(10000) > pCfg->nUpRate) 
                {
                    int nrate = wrand(pCfg->nMaxRate);
                    if(pCfg->m_rates.size()) 
                    {
                        int nrand = 0;
                        for(auto r : pCfg->m_rates)
                         {
                            nrand += r.nRate;
                            if(nrate <= nrand)
                             {
                                nDownLv = r.nValue;
                                break;
                            }
                        }
                    }

                    if(nDownLv > nNowLv)
                    { 
                        nDownLv = nNowLv;
                    }
                    ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmUpStarFailDownStar, tstUI, nDownLv);	
                    //pUseItem->wStar -= nDownLv; 
                    ReducelvByType(eHallowsTotle_SoulWeapon, nPos, nSubType, nDownLv);
                    nCode = 11; //升星发生了降星 事件
                }
                else
                {
                    /*
                    nDownLv += 1;
                    int nTipId = 0;
                    if(pUseItem->wStar >= 11)
                     {
                        nTipId = tmItemUpMaxStar2;
                    }
                    else if(pUseItem->wStar >= 8)
                    {
                        nTipId = tmItemUpMaxStar1;
                    }
                    if(nTipId > 0)
                    {
                        LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nTipId);
                        const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUseItem->wItemId);
                        if(pStdItem && sFormat) {
                            char sContent[1024]={0};
                            sprintf_s(sContent, sizeof(sContent), sFormat, ( char *)(pActor->GetEntityName()), ( char *)(pStdItem->m_sName), pUseItem->wStar);
                            GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(sContent, tstRevolving);
                            GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(sContent, tstChatSystem);
                        }
                    }
                    */
                } 

                if(nCostYb)
                {
                    pActor->RemoveConsume(qatYuanbao, qatYuanbao, nCostYb, -1, -1,-1,0, GameLog::Log_Refining);
                }
                if(CalcCostAll.size())
                { 
                    std::map<int, ACTORAWARD>::iterator it = CalcCostAll.begin();
                    for (; it != CalcCostAll.end(); ++it)
                    {  
                        pActor->RemoveConsume((it->second).btType, (it->second).wId, (it->second).wCount, -1, -1,-1,0, GameLog::Log_Refining); 
                    }
                }
            }  
            if (nCode == 11)//发生了降星
            {
                nNowLv = GetHallowsLv(eHallowsTotle_SoulWeapon, nPos, nSubType); 
            }
            else
            { 
                nNowLv++; 
                AddlvByType(eHallowsTotle_SoulWeapon, nPos, nSubType);
            }
            ((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);	
            m_isDataModify = true;  
     
        }  
    } while (false);  
    
    CActorPacket ap ;
    CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
    data <<(BYTE)GetSystemID()<<(BYTE)sUpStrengthenSystemInfo ;
    data <<(BYTE)(nCode);
    data <<(BYTE)(nType);
    data <<(BYTE)(nPos);
    data <<(BYTE)(nSubType); 
    data <<(int)(nNowLv);
    ap.flush();
}
void CHallowsSystem::OnCustomRefiningReplace(CDataPacketReader& packet)
{ 
    if(!m_pEntity) 
        return;
    
	byte nCode = 0; 
    BYTE nType = 0;
    BYTE nPos = 0; 
    packet >> nType >> nPos;  
	int nEqPos = 0;
    
    char *RefBuff = NULL;
    char *BestBuff = NULL;
	do
	{ 
        if (m_vcRefining.size() < nPos
            || m_vcBestAttr.size() < nPos)
        {
            //超出类型
            nCode = 4;
            break;
        }
        std::map<int, char*>::iterator itRef = m_vcRefining.find(nPos);
        std::map<int, char*>::iterator itBest = m_vcBestAttr.find(nPos);
 
        if (itRef == m_vcRefining.end()
            || itBest == m_vcBestAttr.end())
        {
            //超出类型
            nCode = 4;
            break;
        } 
        RefBuff = itRef->second;
        BestBuff = itBest->second; 
		
		if(nCode)
			break; 

		if(strlen(RefBuff) == 0)
		{
			nCode = 3;
			break;
		}
 
        memcpy(&(*BestBuff), &(*RefBuff), sizeof(char) * 200);
        memset(&(*RefBuff), 0, sizeof(char) * 200);
		//需要重新计算属性
		m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	
		// 更新升级后的血、蓝

	} while (false);
	
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data <<(byte)GetSystemID() <<(byte)sRefiningReplaceHallowsSystemInfo;
	data <<(byte)nCode; 
	data <<(byte)nType;
	data <<(byte)nPos;  
	//if(pUseItem) 
	{ 
		data.writeString(BestBuff);
		data.writeString(RefBuff);
	}
	ap.flush(); 
    m_isDataModify = true;
} 
void CHallowsSystem::OnCustomRefining(CDataPacketReader& packet)
{ 
    if(!m_pEntity) 
        return;

    BYTE nType = 0;
    BYTE nPos = 0; 
    BYTE nReqOp = 0;
    packet >> nType >> nPos >> nReqOp; 
 
	CActor* pActor = (CActor*)m_pEntity;
	int nCode = 0; 
	BYTE bCanReplace = 0; 
    char *RefBuff = NULL;
    char *BestBuff = NULL;
	do
	{ 
        if (m_vcRefining.size() < nPos
            || m_vcBestAttr.size() < nPos)
        {
            //超出类型
            nCode = 4;
            break;
        }
        std::map<int, char*>::iterator itRef = m_vcRefining.find(nPos);
        std::map<int, char*>::iterator itBest = m_vcBestAttr.find(nPos);
 
        if (itRef == m_vcRefining.end()
            || itBest == m_vcBestAttr.end())
        {
            //超出类型
            nCode = 4;
            break;
        } 
        RefBuff = itRef->second;
        BestBuff = itBest->second;

    
        std::vector<HallowsUpLimit>* pLimit = NULL; 
        WpRefiningInfo* pCfg = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetWpRefiningInfo((int)nPos); 
		if(!pCfg)
		{
            //找不到配置类型
			nCode = 3;
			break;
		}
        pLimit = &(pCfg->limit); 
           
        if(pLimit
            && (*pLimit).size())
        { 
            for(int i = 0; i < (*pLimit).size(); i++) 
            {
                HallowsUpLimit& limit = (*pLimit)[i];
                if(limit.nPos)
                    { 
                    int nCurrLv = GetHallowsLv(eHallowsTotle_SoulWeapon, limit.nPos, HallowsUpgrade_Lorder); //获得当前阶数
                    if (nCurrLv < limit.nLimitlv)
                    { 
                        nCode = 9; 
                        break;
                    } 
                } 
            }
        }

        if(nCode)
            break;

        std::map<int, ACTORAWARD> CalcCostAll;
		int nCostYb = 0; 
		if(pCfg->cost.size())
		{
			for(int i = 0; i < pCfg->cost.size(); i++)
			{
				bool bReplace = false;
				HallowsthCost& table = pCfg->cost[i];
                WpReplaceInfo* res = GetLogicServer()->GetDataProvider()->GetHallowthen().getReplaceCfgByPos(eHallowsReplaceType_Refining, (int)nPos, table.nId); 
				if(res)
				{
					if(nReqOp & (1<< res->idx))
					{
                        //选择替换方案
                        for(int k = 0; k < res->vPrice.size(); k++)
                        {
                            HallowsthCost &rep = res->vPrice[k];
                            if (rep.nType == qatYuanbao)
                            {
						        nCostYb += table.nCount * (rep.nCount);
                            } 
                            else if (rep.nType == 0)
                            {   
                                ACTORAWARD& tmp = CalcCostAll[rep.nId]; 
                                tmp.btType = rep.nType;
                                tmp.wId = rep.nId;
                                tmp.wCount += table.nCount * rep.nCount;
                            }
                        }
						bReplace = true;
					}
				} 
				if(!bReplace)
				{
                    ACTORAWARD& tmp = CalcCostAll[table.nId]; 
                    tmp.btType = table.nType;
                    tmp.wId = table.nId;
                    tmp.wCount += table.nCount;
				}				
			}
		}
		
		if(nCostYb)
		{
			if(!pActor->CheckActorSource(qatYuanbao, qatYuanbao, nCostYb, tstUI))
			{
				nCode = 5;
				break;
			}
		} 
		if(CalcCostAll.size())
		{ 
            std::map<int, ACTORAWARD>::iterator it = CalcCostAll.begin();
            for (; it != CalcCostAll.end(); ++it)
            {  
                if(!pActor->CheckActorSource((it->second).btType, (it->second).wId, (it->second).wCount, tstUI))
                {
                    nCode = 4;//道具不够
                    break;
                }
            }
		}
		if(nCode)
			break;

		if(nCostYb)
		{
			pActor->RemoveConsume(qatYuanbao, qatYuanbao, nCostYb, -1, -1,-1,0, GameLog::Log_Refining);
		}
		if(CalcCostAll.size())
		{ 
            std::map<int, ACTORAWARD>::iterator it = CalcCostAll.begin();
            for (; it != CalcCostAll.end(); ++it)
            {  
                pActor->RemoveConsume((it->second).btType, (it->second).wId, (it->second).wCount, -1, -1,-1,0, GameLog::Log_Refining); 
            }
		} 

		DROPGOODS item; 
        
        if (m_vcRefining.size() < nPos
            || m_vcBestAttr.size() < nPos)
        {
            //超出类型
            nCode = 4;
            break;
        }
  
        GetRefineAttribute(pCfg, item);
		std::map<int, int> new_attr; 
		//memset(pUseItem->Refining, 0, sizeof(pUseItem->Refining));
		memset(&(*RefBuff), 0, sizeof(char) * 200);
		int num = item.v_bestAttr.size();
		for(int j = 0; j < num; j++)
		{
			char buf[10];
			if(j != 0)
				//strcat(pUseItem->Refining, "|");
				strcat(&(*RefBuff), "|");

			DropAttribute data = item.v_bestAttr[j];
			sprintf(buf, "%d,%d", data.nType, data.nValue);
			new_attr[data.nType] = data.nValue;
			//strcat(pUseItem->Refining, buf);
			strcat(&(*RefBuff), buf);
		}
		//pUseItem->Refining[sizeof(pUseItem->Refining)-1]= '\0';	
		//RefBuff[sizeof(char) * 200 - 1]= '\0';	 

		//默认木有极品属性的数据直接替换
		if(strlen(BestBuff) == 0 && strlen(RefBuff))
		{
			memcpy(&(*BestBuff), &(*RefBuff), sizeof(char) * 200);
			memset(&(*RefBuff), 0, sizeof(char) * 200);
			bCanReplace = 1;
		}
		std::map<int, int> old_attr;
		if (!bCanReplace && strlen(BestBuff))
		{
			std::vector<std::string> results = SplitStr(BestBuff, "|");
			int nCount = results.size();
			for (int i = 0; i < nCount; i++) {
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() >= 2) {
					int nType = atoi(temp_res[0].c_str());
					int nValue = atoi(temp_res[1].c_str());
					old_attr[nType] = nValue;
				}
			}
			if(old_attr.size() )
			{
				bool bAutoReplace = true;
				std::map<int, int>::iterator it = old_attr.begin();
				for(; it != old_attr.end(); it++)
				{
					std::map<int, int>::iterator iter = new_attr.find(it->first);
					if(iter == new_attr.end() || (iter->second < it->second))
					{
						bAutoReplace = false;
						break;
					}
				}
				if(bAutoReplace) {
					memcpy(&(*BestBuff), &(*RefBuff), sizeof(char) * 200);
					memset(&(*RefBuff), 0, sizeof(char) * 200);
					bCanReplace = 1;
				}
			}
		}

		if(bCanReplace)
		{
			//需要重新计算属性
			m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	
			// 更新升级后的血、蓝
		}
		if(nReqOp)
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmRefiningReplace,tstUI);
		}
	} while(false);
	
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data <<(byte)GetSystemID()<<(byte)sRefiningHallowsSystemInfo;
	data <<(byte)nCode; 
	data <<(byte)nType;
	data <<(byte)nPos; 
	data << (byte)bCanReplace;
	//if(!nCode)
	{  
		data.writeString(BestBuff);
		data.writeString(RefBuff);
	}
	ap.flush();
	
    m_isDataModify = true;
    return;
}

void CHallowsSystem::GetRefineAttribute(WpRefiningInfo* pCfg, DROPGOODS& item)
{
	//std::map<int, DROPATTRIBUTECFG>::iterator dropAttrCfg = m_RefingAttributeMaps.find(type);
	//if(dropAttrCfg != m_RefingAttributeMaps.end())
	{
		//DROPATTRIBUTECFG cfg = dropAttrCfg->second;
		int rand = wrand(100);
		if(rand <= pCfg->nRate)
		{
			int count = pCfg->vAttribute.size();
			for(int i = 0; i < count; i++)
			{
				DropAttribute& info = pCfg->vAttribute[i];
				int rand_ = wrand(100);
				// printf("GetDropAttribute: %d, %d\n",rand_,info.nRate);
				if(rand_ <= info.nRate)
				{
					// printf("GetDropAttribute: %d, %d\n",rand_,info.nRate);
					item.AddDropItemAttr(info);
					if(info.nFollowcount)
					{
						for(int j = 0; j < info.nFollowcount; j++)
						{
							int rand = wrand(100);
							if(rand <= info.nFollowrate)
							{
								item.AddDropItemAttr(info);
                                continue;
							}
							break;
						}
					}
				}
			} 
		} 
	}
}
int CHallowsSystem::GetHallowsLv(/*第一个参数默认是兵魂*/int nType, int nPos, int nSubType)
 {
    std::map<int, std::map<int, HallowsSoulWeaponInfo> >::iterator it = m_nHallowsSoulWeapon.find(nType);

    if(it != m_nHallowsSoulWeapon.end())
     {
        std::map<int, HallowsSoulWeaponInfo>::iterator itPos = (it->second).find(nPos);       
        if(itPos != (it->second).end()) 
        {
            HallowsSoulWeaponInfo &temp = itPos->second; 
            
            if (HallowsUpgrade_Lorder == nSubType)
            { 
                return temp.nLorderId; 
            }
            if (HallowsUpgrade_Star == nSubType)
            { 
                return temp.nStarId;  
            }
            if (HallowsUpgrade_Lv == nSubType)
            { 
                return temp.nLvId;  
            }
        }  
    }

    return 0;
} 
HallowsSoulWeaponInfo* CHallowsSystem::GetHallowsPtrInfo(int nType, int nPos)
{ 
    std::map<int, std::map<int, HallowsSoulWeaponInfo> >::iterator it = m_nHallowsSoulWeapon.find(nType);

    if(it != m_nHallowsSoulWeapon.end())
    {
        std::map<int, HallowsSoulWeaponInfo>::iterator itPos = (it->second).find(nPos);
        if(itPos != (it->second).end()) 
        {
            return &itPos->second; 
        } 
    }

    return NULL;
} 
bool CHallowsSystem::CheckHallowsthenLv(int nType, int nPos, int nLv) {
    
    std::map<int, std::map<int, HallowsSoulWeaponInfo> >::iterator it = m_nHallowsSoulWeapon.find(nType);

    if(it != m_nHallowsSoulWeapon.end()) 
    { 
        std::map<int, HallowsSoulWeaponInfo>::iterator itPos = (it->second).find(nPos);
        if(itPos != (it->second).end()) 
        { 
            HallowsSoulWeaponInfo &temp = itPos->second;
            if (temp.nLvId >= nLv)
            { 
                return true;
            }
        } 
    }
    return false;
} 
VOID CHallowsSystem::CalcAttributes(CAttrCalc &calc)
{
	if(m_pEntity ==NULL) 
        return ; 
 
    AttriGroup RefineAttri;	//属性 
    std::map<int, int> Refine_attr;
    for (size_t i = 1; i <= GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetSoulWeapon(); i++)
    {
        char *BestBuff = m_vcBestAttr[i]; 
		if (strlen(BestBuff))
		{
			std::vector<std::string> results = SplitStr(BestBuff, "|");
			int nCount = results.size();
			for (int i = 0; i < nCount; i++) 
            {
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() >= 2) 
                {
					int nType = atoi(temp_res[0].c_str());
					int nValue = atoi(temp_res[1].c_str());
                    if (Refine_attr.find(nType) == Refine_attr.end())
                    {
					    Refine_attr[nType] = nValue;
                    }
                    else
                    { 
					    Refine_attr[nType] += nValue;
                    } 
				}
			}
        }
    }
     
    GAMEATTR tmpAttr;
    ZeroMemory(&tmpAttr, sizeof(GAMEATTR));
    std::map<int, int>::iterator it_refine = Refine_attr.begin();
    for(; it_refine != Refine_attr.end(); ++it_refine)
    {  
        tmpAttr.type = it_refine->first;
        
		if ( tmpAttr.type < aUndefined || tmpAttr.type >= GameAttributeCount )
		{
			//showErrorFormat(_T("LoadAttri type config error %d"),pAttr->type);
		}
		switch(AttrDataTypes[tmpAttr.type])
		{
		case adSmall:
		case adShort:
		case adInt:
			tmpAttr.value.nValue = it_refine->second;
			break;
		case adUSmall:
		case adUShort:
		case adUInt:
			tmpAttr.value.uValue = (UINT)it_refine->second;
			break;
		case adFloat:
			tmpAttr.value.fValue = (float)it_refine->second;
			break;
		} 
        calc << tmpAttr;  
    } 
	std::map<int, std::map<int, HallowsSoulWeaponInfo> >::iterator it = m_nHallowsSoulWeapon.begin();
    int nJob = ((CActor*)m_pEntity)->GetJob();
    for(; it != m_nHallowsSoulWeapon.end(); it++)
    {
        std::map<int, HallowsSoulWeaponInfo>& v_list = it->second;
        
        std::map<int, HallowsSoulWeaponInfo>::iterator itSub = v_list.begin();
        for(; itSub != v_list.end(); itSub++)
        //for(int i = 0; i < v_list.size(); i++)
        {
            HallowsSoulWeaponInfo& info = itSub->second;
            
            if (it->first == eHallowsTotle_SoulWeapon)
            {
                HallowsLorderInfo* pCfg = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetLorderInfo(info.nId, info.nLorderId); 
                if(pCfg)
                { 
                    for(int  i = 0; i < pCfg->attri.nCount; i++)
                    {
                        calc << pCfg->attri.pAttrs[i];
                    }
                }
                
                HallowsLvInfo* pCfgLv = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetLvInfo(info.nId, info.nLvId); 
                if(pCfgLv)
                { 
                    for(int  i = 0; i < pCfgLv->attri.nCount; i++)
                    {
                        calc << pCfgLv->attri.pAttrs[i];
                    }
                }
                
                HallowsStarInfo* pCfgStar = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetStarInfo(info.nId, info.nStarId); 
                if(pCfgStar)
                { 
                    for(int  i = 0; i < pCfgStar->attri.nCount; i++)
                    {
                        calc << pCfgStar->attri.pAttrs[i];
                    }
                }
            } 
        }
    }
}
//升级
int CHallowsSystem::AddlvByType(int nType, int nPos, int nSubType)
 {
    std::map<int, std::map<int, HallowsSoulWeaponInfo> >::iterator it = m_nHallowsSoulWeapon.find(nType);
    int Lv = 0; 
    if(it != m_nHallowsSoulWeapon.end()) 
    {
        std::map<int, HallowsSoulWeaponInfo>::iterator itPos = (it->second).find(nPos);
        if(itPos != (it->second).end()) 
        { 
            HallowsSoulWeaponInfo &temp = itPos->second;
            if (HallowsUpgrade_Lorder == nSubType)
            { 
                temp.nLorderId++;
                Lv = temp.nLorderId;  
            }
            if (HallowsUpgrade_Star == nSubType)
            { 
                temp.nStarId++;
                Lv = temp.nStarId;  
            }
            if (HallowsUpgrade_Lv == nSubType)
            { 
                temp.nLvId++;
                Lv = temp.nLvId;  
            }  
        }
    } 
    return Lv;   
}
//降级
int CHallowsSystem::ReducelvByType(int nType, int nPos, int nSubType, int nLeve)
 {
    std::map<int, std::map<int, HallowsSoulWeaponInfo> >::iterator it = m_nHallowsSoulWeapon.find(nType);
    int Lv = 0; 
    if(it != m_nHallowsSoulWeapon.end()) 
    {
        std::map<int, HallowsSoulWeaponInfo>::iterator itPos = (it->second).find(nPos);
        if(itPos != (it->second).end()) 
        { 
            HallowsSoulWeaponInfo &temp = itPos->second;
            if (HallowsUpgrade_Lorder == nSubType)
            { 
                if (temp.nLorderId < nLeve)
                {
                    temp.nLorderId = 0;
                }
                
                temp.nLorderId -= nLeve;
                Lv = temp.nLorderId;  
            }
            if (HallowsUpgrade_Star == nSubType)
            { 
                if (temp.nStarId < nLeve)
                {
                    temp.nStarId = 0;
                }
                
                temp.nStarId -= nLeve;
                Lv = temp.nStarId;  
            }
            if (HallowsUpgrade_Lv == nSubType)
            { 
                if (temp.nLvId < nLeve)
                {
                    temp.nLvId = 0;
                }
                temp.nLvId -= nLeve;
                Lv = temp.nLvId;  
            }  
        }
    } 
    return Lv;   
}
HallowsLorderInfo* CHallowsSystem::GetHallowsLoderv(int nType, int nPos)
{ 
    std::map<int, HallowsSoulWeaponInfo> &Tmp = m_nHallowsSoulWeapon[eHallowsTotle_SoulWeapon];
 
    std::map<int, HallowsSoulWeaponInfo>::iterator itPos = Tmp.find(nPos);       
    if(itPos != Tmp.end()) 
    {
        HallowsSoulWeaponInfo &temp = itPos->second; 
        HallowsLorderInfo* pCfg = GetLogicServer()->GetDataProvider()->GetHallowthen().OnGetLorderInfo((int)nPos, temp.nLorderId);
        return pCfg;  
    }   
    return NULL;
}
VOID CHallowsSystem::OnReturnHallowsSoulWeaponInfo(CDataPacket &data)
{
    //离线数据
    //data << (BYTE)(m_nHallowsSoulWeapon.size());  
    std::map<int, HallowsSoulWeaponInfo>& temp = m_nHallowsSoulWeapon[eHallowsTotle_SoulWeapon];
    
    BYTE nCount = temp.size();
    data << nCount;
    if(nCount > 0) 
    {
        std::map<int, HallowsSoulWeaponInfo>::iterator iter = temp.begin();
        for(; iter != temp.end(); iter++)
        {
            HallowsSoulWeaponInfo& info = iter->second;
            
            char *BestBuff = NULL; 
            std::map<int, char*>::iterator itBest = m_vcBestAttr.find(info.nId);
    
            if (itBest != m_vcBestAttr.end())
            {
                BestBuff = itBest->second; 
            } 

            data <<(BYTE)(info.nId);
            data <<(int)(info.nLorderId);
            data <<(int)(info.nStarId);
            data <<(int)(info.nLvId); 
            data.writeString(BestBuff); 
            //data.writeString(RefBuff);
        }
    } 
}



//跨服数据 发送数据
void CHallowsSystem::SendMsg2CrossServer(int nType)
{
	if(m_pEntity ==NULL) return;
	
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = m_pEntity->GetId();
    OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "),nType,nActorID);
	packet << nServerId << nActorID;
	packet <<(BYTE)nType;
	//数据
	packet << m_pEntity->GetCrossActorId();
    packet << m_pEntity->GetAccountID();
    size_t count = 0;
    int nPos = packet.getPosition();
    std::map<int, HallowsSoulWeaponInfo>& temp = m_nHallowsSoulWeapon[eHallowsTotle_SoulWeapon];
    count = 0;//temp.size();
    packet <<(int)count; 
    std::map<int, HallowsSoulWeaponInfo>::iterator itSub = temp.begin();
    for(; itSub != temp.end(); itSub++) 
    {
        HallowsSoulWeaponInfo& info = itSub->second;
        if (info.nId >= eHallowsSSC_MAX || info.nId <= eHallowsSSC_NULL)
        {
            continue;
        } 
        packet << info.nLorderId <<info.nStarId<<info.nLvId;  
        
		//OutputMsg(rmNormal, _T("--------------->CHallowsSystem::SaveToDb POS:%d nLorderId=%d, nStarId=%d, nLvId=%d,"), (int)(itSub->first), info.nLorderId, info.nStarId, info.nLvId);
        char *BestBuff = m_vcBestAttr[(int)(itSub->first)]; 
        // char *RefiningBuff = m_vcRefining[(int)(itSub->first)]; 
        packet.writeString(BestBuff); 
        // packet.writeString(RefiningBuff); 
        count++;//累计
    } 
 
    int *Ptr = (int*)packet.getPositionPtr(nPos);
	*Ptr = (int)count;

	pCrossClient->flushProtoPacket(packet);
}

//接受数据
void CHallowsSystem::OnCrossInitData( std::vector<SoulWeaponNetData>& vSoulWeapon)
{
    if(!m_pEntity) return;

    {
        char cTmpAttr[50]; 
        memset(cTmpAttr, 0, sizeof(cTmpAttr));
        int nCount = vSoulWeapon.size();
        // packet >> nCount;  
        if(nCount != 0)
        {
            std::map<int, HallowsSoulWeaponInfo>& temp = m_nHallowsSoulWeapon[eHallowsTotle_SoulWeapon];
            for(int nPos = eHallowsSSC_JZLR; nPos < eHallowsSSC_MAX; ++nPos)
            {
                std::map<int, HallowsSoulWeaponInfo>::iterator itPos = temp.find(nPos);
                if(itPos == temp.end())
                {
                    continue;
                } 
                SoulWeaponNetData& it = vSoulWeapon[nPos-1];
                HallowsSoulWeaponInfo & info = itPos->second;
                // packet >> info.nLorderId >> info.nStarId >> info.nLvId;

                //OutputMsg(rmNormal, _T("++++++++++++++++++++CHallowsSystem::OnDbRetData POS:%d nLorderId=%d, nStarId=%d, nLvId=%d,"), (int)(nPos), info.nLorderId, info.nStarId, info.nLvId);
                memcpy(&info, &vSoulWeapon[nPos-1], sizeof(HallowsSoulWeaponInfo));
                char *BestBuff = m_vcBestAttr[nPos]; 

                memset(&(*BestBuff), 0, sizeof(char) * (200+1));
                memcpy(BestBuff, it.cBestAttr, sizeof(it.cBestAttr));
                OutputMsg(rmTip,_T("OnCrossInitData CHallowsSystem nActorID:%d nId:%d,nStarId:%d,nLorderId:%d,nLvId:%d,cBestAttr:%s"),
						m_pEntity->GetId(),it.nId,it.nStarId,it.nLorderId,it.nLvId,it.cBestAttr);    
            }
        }
    }
    m_isInitData = true;
    m_isDataModify = true; 
    ((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CHALIDOME); //完成一个步骤
}
