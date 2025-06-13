#include "StdAfx.h"
#include "CStaticCountSystem.h"

bool CStaticCountSystem::Initialize(void* data, size_t size)
{
    isInitData = false;
    isDataModify = false;
    m_StaticCounts.clear();
    return true;
}
void CStaticCountSystem::OnEnterGame()
{
}
void CStaticCountSystem::DailyRefresh()
{
    if(!isInitData || !m_pEntity)
    { 
        return;
    }
    ClearDailyStaticCount();
    ClearWeekStaticCount();
    ClearMonthStaticCount();
}
LONGLONG CStaticCountSystem::GetStaticCount(INT_PTR nStaticType)
{
    if(!m_pEntity) return 0;
    std::map<int, LONGLONG>::iterator it = m_StaticCounts.find(nStaticType);
    if(it != m_StaticCounts.end())
    {
       return it->second;
    }
    return 0;
}
void CStaticCountSystem::SetStaticCount(INT_PTR nStaticType, INT_PTR nValue)
{
    if(!m_pEntity) return;
    std::map<int, LONGLONG>::iterator it = m_StaticCounts.find(nStaticType);
    if(it != m_StaticCounts.end())
    {
        it->second = nValue;
    }
    else
    {
        // m_StaticCounts[nStaticType] = nValue;
        m_StaticCounts.insert(std::make_pair(nStaticType, nValue));
    }
    //set里面用不到战令币 ，净化基础函数，业务逻辑在上层函数做
    // if(nStaticType == eSTATIC_TYPE_ORDERWARD_MONEY)
    // {
    //     if(m_StaticCounts[nStaticType] < 0)
    //     {
    //         m_StaticCounts[nStaticType] = 0;
    //     }
    //     ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_ZL_MONEY, m_StaticCounts[nStaticType] );
    // }
    isDataModify = true; 
}
void CStaticCountSystem::AddStaticCount(INT_PTR nStaticType, INT_PTR nValue)
{
    if(!m_pEntity) 
        return;
    if(nStaticType == eSTATIC_TYPE_DIMENSIONAL_KEY)
    {
        OnAddDimensionalKey(nValue);
        return;
    }
    if(nStaticType == eSTATIC_TYPE_ORDERWARD_MONEY)
    {
        OnAddOrderWardMoney(nValue);
        return;
    }
    std::map<int, LONGLONG>::iterator it = m_StaticCounts.find(nStaticType);
    if(it != m_StaticCounts.end())
    {
        it->second += nValue;
    }
    else
    {
        // m_StaticCounts[nStaticType] = nValue;
        m_StaticCounts.insert(std::make_pair(nStaticType, nValue));
    }
    //set里面用不到战令币 ，净化基础函数，业务逻辑在上层函数做
    // if(nStaticType == eSTATIC_TYPE_ORDERWARD_MONEY)
    // {
    //     if(m_StaticCounts[nStaticType] < 0)
    //     {
    //         m_StaticCounts[nStaticType] = 0;
    //     }
    //     ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_ZL_MONEY, m_StaticCounts[nStaticType] );
    // }
    // else
    //  if(nStaticType == eSTATIC_TYPE_DIMENSIONAL_KEY) //次元钥匙
    // {
    //     if(m_StaticCounts[nStaticType] < 0)
    //     {
    //         m_StaticCounts[nStaticType] = 0;
    //     }
    //     ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_DIMENSIONALKEY, m_StaticCounts[nStaticType] );
    // }
    isDataModify = true;
}
void CStaticCountSystem::OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet)
{
    if(!m_pEntity) return;

    if(nCmd == jxInterSrvComm::DbServerProto::dcLoadStaticCount && nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回数据
    {
        int nCount = 0;
        packet >> nCount;
        m_StaticCounts.clear();
        for(int i = 0; i < nCount; i++)
        {
            int nType = 0; 
            int nValue = 0;
            packet >> nType >> nValue;
            m_StaticCounts[nType] = nValue;
            if(nType == eSTATIC_TYPE_ORDERWARD_MONEY)
            {
                if(m_StaticCounts[nType] < 0)
                {
                    m_StaticCounts[nType] = 0;
                }
                ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_ZL_MONEY, m_StaticCounts[nType] );
            }
        }
    }
    isInitData = true;
    ((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_STATICCOUNT_SYSTEM); //完成一个步骤
}
void CStaticCountSystem::SaveToDb()
{
    if(!m_pEntity) return;

    if(!isInitData) return;
    
    if(!isDataModify) return;

    CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveStaticCount);
    dataPacket <<((CActor*)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();

    dataPacket <<(unsigned int)m_pEntity->GetId(); //玩家id
    size_t count = m_StaticCounts.size();
    dataPacket <<(int)count;
    std::map<int, LONGLONG>::iterator it = m_StaticCounts.begin();

    for(; it != m_StaticCounts.end(); it++)
    {
        dataPacket <<(int)(it->first);
        dataPacket <<(int)(it->second);
    }
    GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

    isDataModify = false;
}
void CStaticCountSystem::ProcessNetData(INT_PTR nCmd, CDataPacketReader& packet)
{
    switch (nCmd)
    {
        case enStaticCountSystemcExchangeCount:
        {
            SendCircleExchangeCount();
            break;
        }
        case enStaticCountSystemcUpCircle:
        {
            UpCircle();
            break;
        }
        case enStaticCountSystemcExchangeSoul:
        {
            ExchangeCircleSoul(packet);
            break;
        }
        case enStaticCountSystemcMeridiansUPLv:
        {
            UpMeridiansLv();
            break;
        }
    default:
        break;
    }
}
void CStaticCountSystem::SendCircleExchangeCount()
{
    if(!m_pEntity) return;
    CActor * pActor = (CActor*)m_pEntity;
    const CIRCLECFG& cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_CircleLevelConfig;
    int nDay = GetLogicServer()->GetDaysSinceOpenServer();

    if(!pActor->CheckLevel(cfg.nOpenLv, 0))
    {
        pActor->SendTipmsgFormatWithId(tmCircleLevelLimit, tstUI,cfg.nOpenLv);
    }
    if(nDay < cfg.nOpenServerDay)
    {
        pActor->SendTipmsgFormatWithId(tmCircleOpenServerDayLimit, tstUI, cfg.nOpenServerDay);
        return;
    }
    CActorPacket ap;
    CDataPacket& dataPack = ((CActor*)m_pEntity)->AllocPacket(ap);
    dataPack << (BYTE)enStaticCountSystemID <<(BYTE)enStaticCountSystemExchangeCount;
    int nCount = cfg.m_lExSoul.count;
    dataPack << (BYTE)nCount;
    for(int i = 0; i < nCount; i++)
    {
        EXCHANGECIRCLE& dfCfg = cfg.m_lExSoul[i];
        int nValue = GetStaticCount(dfCfg.nStaticType);
        dataPack << (BYTE)dfCfg.nType << (BYTE)nValue;
    }
    ap.flush();
}
void CStaticCountSystem::ExchangeCircleSoul(CDataPacketReader& packet)
{  
    if(!m_pEntity) return;

    BYTE nType = 0;
    packet >> nType;
  
    CActor* pActor = (CActor*)m_pEntity;
    
    const CIRCLECFG& cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_CircleLevelConfig;
    if(nType > cfg.m_lExSoul.count)
    {
        pActor->SendTipmsgFormatWithId(	tmDataError, tstUI);
        return;
    }
    const EXCHANGECIRCLE& dhCfg = cfg.m_lExSoul[nType-1];
    int nNum = GetStaticCount(dhCfg.nStaticType);
    if(nNum >= dhCfg.nUseLimit)
    {
        pActor->SendTipmsgFormatWithId(tmCircleLimtiTimes, tstUI);
        return;
    }
    
    unsigned int nSoul = pActor->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL);
    unsigned int nAdd = 0;
    
    if(nType == 1)//转生丹
    {
        int nItemid = 259;
        CUserBag& bag = pActor->GetBagSystem();
        CUserItem* pUserItem = bag.FindItem(nItemid);
        if(!pUserItem || pUserItem->wCount < dhCfg.nCost)
        {
            pActor->SendTipmsgFormatWithId(tmItemNotEnough,tstUI);
            return;
        }
        bag.DeleteItem(pUserItem,dhCfg.nCost,"self_del_item",GameLog::Log_CircleEx,true);
    }
    else if(nType == 2)//经验
    {
        Uint64 nActorExp = pActor->GetProperty<Uint64>(PROP_ACTOR_EXP);
        if(nActorExp < dhCfg.nCost)
        {
            pActor->SendTipmsgFormatWithId(tmExpLimited,tstUI);
            return;
        }
        nActorExp -= dhCfg.nCost;
        if(nActorExp < 0)
            return;

        pActor->SetProperty<Uint64>(PROP_ACTOR_EXP,nActorExp);

    }else if(nType == 3)//回收积分
    {
        unsigned int nPoints = pActor->GetProperty<unsigned int>(PROP_ACTOR_RECYCLE_POINTS);
        if(nPoints < dhCfg.nCost)
        {
            pActor->SendTipmsgFormatWithId(tmPointNotEnough,tstUI);
            return;
        }
        nPoints -= dhCfg.nCost;
        if(nPoints < 0)
            return;

        pActor->SetProperty<unsigned int>(PROP_ACTOR_RECYCLE_POINTS,nPoints);
    }
    else
    {
        pActor->SendTipmsgFormatWithId(tmDataError, tstUI);
        return;
    }

    nSoul += dhCfg.nValue;
    pActor->SetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL, nSoul);
    SetStaticCount(dhCfg.nStaticType, nNum+1);
    CActorPacket ap;
    CDataPacket& dataPack = pActor->AllocPacket(ap);
    dataPack << (BYTE)enStaticCountSystemID <<(BYTE)enStaticCountSystemsExchangeSoul <<(BYTE)1;
    ap.flush();
}

void CStaticCountSystem::UpCircle()
{
    if(m_pEntity == NULL) return;
    CActor* pActor = (CActor*)m_pEntity;
    CIRCLECFG &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_CircleLevelConfig;
    unsigned int nCircleLevel = pActor->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);

    int nMaxLevel = cfg.m_lCircleLv.size()-1;
    if(nCircleLevel >= nMaxLevel)
    {
        pActor->SendTipmsgFormatWithId(tmCircleMaxLevelLimit, tstUI);
        return;
    }
    std::map<int, CIRCLELEVEL>::iterator it = cfg.m_lCircleLv.find(nCircleLevel+1);
    if(it != cfg.m_lCircleLv.end())
    {
        CIRCLELEVEL& levelCfg = it->second;//cfg.m_lCircleLv[nCircleLevel];
        int nDay = GetLogicServer()->GetDaysSinceOpenServer();
        if(nDay < levelCfg.nServerOpenDay)
        {
            std::string strOpenDay = intToString(levelCfg.nServerOpenDay);
            pActor->SendTipmsgFormatWithId(tmOpenDayLimitCircle, tstUI, strOpenDay.c_str() );
            return;
        }

        if(!pActor->CheckLevel(levelCfg.nLevelLimit, 0))
        {
            pActor->SendTipmsgFormatWithId(tmCircleLevelLimit, tstUI, levelCfg.nLevelLimit);
            return;
        }

        if(levelCfg.cost.size() > 0) {
            for(int i = 0; i < levelCfg.cost.size(); i++) {
                ACTORAWARD& cost = levelCfg.cost[i];
                if(!pActor->CheckActorSource(cost.btType, cost.wId, cost.wCount, tstUI))
                    return;
            }
        }
        if(levelCfg.nCostLevel > 0) {
            int nNowLv = pActor->GetLevel();
            nNowLv -= levelCfg.nCostLevel;
            if(nNowLv < 0) nNowLv = 0;
            pActor->SetLevel(nNowLv);
        }
        if(levelCfg.cost.size() > 0) {
            for(int i = 0; i < levelCfg.cost.size(); i++) {
                ACTORAWARD& cost = levelCfg.cost[i];
                pActor->RemoveConsume(cost.btType, cost.wId, cost.wCount, -1, -1,-1,0, GameLog::Log_UpCircle);
            }
        }
        
        // unsigned int now = nsoul-nNeedCost;
        // pActor->SetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL,now);
        nCircleLevel++;
        if(levelCfg.nTipId) {
            LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(levelCfg.nTipId);
            if( sFormat) {
                char sContent[1024]={0};
                sprintf_s(sContent, sizeof(sContent), sFormat, ( char *)(pActor->GetEntityName()), nCircleLevel);
                GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(sContent,tstRevolving);
                // GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(sContent,tstChatSystem);
            }
        }
        
        //pActor->SetProperty<unsigned int>(PROP_ACTOR_CIRCLE,nCircleLevel);
        pActor->SetActorCircle(nCircleLevel);
        // ((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
        GetGlobalLogicEngine()->GetGlobalVarMgr().AddCountByCircleLevel(nCircleLevel);
        CActorPacket ap;
        CDataPacket& dataPack = pActor->AllocPacket(ap);
        int nCost = 0;//cfg.GetCost(nCircleLevel+1, nDay);
        dataPack << (BYTE)enStaticCountSystemID <<(BYTE)enStaticCountSystemsUpCircle <<(BYTE)1 <<(BYTE)nCircleLevel << nCost;
        ap.flush();
    }
} 
void CStaticCountSystem::ClearDailyStaticCount()
{
    //CMiniDateTime now_time = CMiniDateTime::now();
    int32_t nNowTime = time(NULL);
    int32_t nNextDailyTime = GetStaticCount(eSTATIC_TYPE_DAILY_TIME); 
    if(nNowTime >= nNextDailyTime)
    {
        std::map<int, LONGLONG>::iterator it = m_StaticCounts.begin();
        for(; it != m_StaticCounts.end();)
        {
            bool canReset = false;
            //策划可配置的字段
            if( it->first >= nDailyStaticStart 
                && it->first <= nDailyStaticEnd)
            {
                canReset = true;
            }
            
            //程序内部使用的字段
            if( it->first >= eSTATIC_TYPE_INNER_ITEM_DAILY_START 
                && it->first <= eSTATIC_TYPE_INNER_ITEM_DAILY_END)
            {
                canReset = true;
            } 

            if(canReset)
            {
                m_StaticCounts.erase(it++);
            }
            else
            {
                it++;
            } 
        }
        int32_t nDailyTime = getNowZeroTime() + 24 * 3600;
        SetStaticCount(eSTATIC_TYPE_DAILY_TIME, nDailyTime);

        //CMiniDateTime next_time = CMiniDateTime::tomorrow(); 
        //SetStaticCount(eSTATIC_TYPE_DAILY_TIME, next_time.tv);
    }

    //程序内部使用的计数器  
}
void CStaticCountSystem::ClearWeekStaticCount()
{
    CMiniDateTime now_time = CMiniDateTime::now();
    int nNowTime = time(NULL);
    int nweekTime = GetStaticCount(eSTATIC_TYPE_WEEK_TIME);
    if(nNowTime >= nweekTime)
    {
        std::map<int, LONGLONG>::iterator it = m_StaticCounts.begin();
        for(; it != m_StaticCounts.end();)
        {
            bool canReset = false;
 
            //策划可配置的字段
            if( it->first >= nWeekStaticStart 
                && it->first <= nWeekStaticEnd)
            {
                canReset = true;
            }
            
            //程序内部使用的字段
            if( it->first >= eSTATIC_TYPE_INNER_ITEM_WEEK_START 
                && it->first <= eSTATIC_TYPE_INNER_ITEM_WEEK_END)
            {
                canReset = true;
            } 
            if(canReset)
            {
                m_StaticCounts.erase(it++);
            }
            else
            {
                it++;
            }
        }
        int nCurYear,nCurMon,nCurDay,nCurHour,nCurMin,nCurSec,nCurWeek;
        {
            SYSTEMTIME SysTime;// = GetGlobalLogicEngine()->getSysTime();
            now_time.decode(SysTime);
            nCurYear = SysTime.wYear;
            nCurMon = SysTime.wMonth;
            nCurDay = SysTime.wDay;
            nCurHour = SysTime.wHour;
            nCurMin = SysTime.wMinute;
            nCurSec = SysTime.wSecond;

            struct tm tmResult;
            memset(&tmResult, 0, sizeof(tmResult));
            tmResult.tm_isdst = 0;
            tmResult.tm_year = nCurYear - 1900;
            tmResult.tm_mon = nCurMon - 1;
            tmResult.tm_mday = nCurDay;
            mktime(&tmResult);
            nCurWeek = tmResult.tm_wday;
        }
        //周末改为nCurWeek：7 好计算
        if (nCurWeek == 0)
        {
            nCurWeek = 7;
        }
        //nweekTime = getNowZeroTime()+7*24*3600;
        nweekTime = getNowZeroTime() + (8 - nCurWeek) * 24 * 3600;
        SetStaticCount(eSTATIC_TYPE_WEEK_TIME, nweekTime);
    }
}
void CStaticCountSystem::ClearMonthStaticCount()
{
    CMiniDateTime now_time = CMiniDateTime::now(); 
    int nNowTime = time(NULL);
    int nweekTime = GetStaticCount(eSTATIC_TYPE_MONTH_TIME);
    if(nNowTime >= nweekTime)
    {
        std::map<int, LONGLONG>::iterator it = m_StaticCounts.begin();
        for(; it != m_StaticCounts.end();)
        {
            bool canReset = false;
 
            //策划可配置的字段
            if( it->first >= nMonthStaticStart 
                && it->first <= nMonthStaticEnd)
            {
                canReset = true;
            }
            
            //程序内部使用的字段
            if( it->first >= eSTATIC_TYPE_INNER_ITEM_MONTH_START 
                && it->first <= eSTATIC_TYPE_INNER_ITEM_MONTH_END)
            {
                canReset = true;
            } 
            if(canReset)
            {
                m_StaticCounts.erase(it++);
            }
            else
            {
                it++;
            }
        }
        //nweekTime = getNowZeroTime()+7*24*3600;
   
        int nCurYear,nCurMon,nCurDay,nCurHour,nCurMin,nCurSec,nCurWeek;
     
        SYSTEMTIME SysTime;// = GetGlobalLogicEngine()->getSysTime();
        now_time.decode(SysTime);
        if(SysTime.wMonth == 12)
        { 
            //计算跨年
            SysTime.wYear = SysTime.wYear + 1;
            SysTime.wMonth = 1;
            SysTime.wDay = 1;
        }
        else
        { 
            SysTime.wMonth = SysTime.wMonth + 1;
            SysTime.wDay = 1;
        }
        nCurYear = SysTime.wYear;
        nCurMon = SysTime.wMonth;
        nCurDay = SysTime.wDay;
        nCurHour = SysTime.wHour;
        nCurMin = SysTime.wMinute;
        nCurSec = SysTime.wSecond;

        struct tm tmResult;
        memset(&tmResult, 0, sizeof(tmResult));
        tmResult.tm_isdst = 0;
        tmResult.tm_year = nCurYear - 1900;
        tmResult.tm_mon = nCurMon - 1;
        tmResult.tm_mday = nCurDay;
        tmResult.tm_hour = 0;
        tmResult.tm_min = 0;
        tmResult.tm_hour = 0; 
        unsigned int nextTime = mktime(&tmResult); 
      
        SetStaticCount(eSTATIC_TYPE_MONTH_TIME, nextTime);

        // CMiniDateTime NextMiniDateTime;
        // NextMiniDateTime = NextMiniDateTime.encode(SysTime);
     
        // SetStaticCount(eSTATIC_TYPE_MONTH_TIME, NextMiniDateTime.tv);
    }
} 

void CStaticCountSystem::UpMeridiansLv()
{
    if(m_pEntity == NULL) return;
    CActor* pActor = (CActor*)m_pEntity;
    std::map<int, MeridianUpCfg> &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_MeridianUpCfg;
    unsigned int nMeridinLv = pActor->GetProperty<unsigned int>(PROP_ACTOR_MERIDIALv);

    int nMaxLevel = cfg.size();
    if(nMeridinLv >= nMaxLevel)
    {
        pActor->SendTipmsgFormatWithId(tmMeridiansMaxLv, tstUI);
        return;
    }
    std::map<int, MeridianUpCfg>::iterator it = cfg.find(nMeridinLv+1);
    if(it != cfg.end())
    {
        MeridianUpCfg& levelCfg = it->second;
        int nDay = GetLogicServer()->GetDaysSinceOpenServer();
        if(nDay < levelCfg.nServerOpenDay)
        {
            pActor->SendTipmsgFormatWithId(tmCircleOpenServerDayLimit, tstUI, levelCfg.nServerOpenDay );
            return;
        }
        if(pActor->GetLevel() < levelCfg.nLevelLimit)
        {
            pActor->SendTipmsgFormatWithId(tmCircleLevelLimit, tstUI, levelCfg.nLevelLimit);
            return;
        }

        if(pActor->GetCircleLevel() < levelCfg.nCircle)
        {
            pActor->SendTipmsgFormatWithId(tmCircleLvLimit, tstUI);
            return;
        }

        // int nExp = pActor->GetProperty<unsigned int>(PROP_ACTOR_EXP);
        // if(nExp < levelCfg.nCostExp)
        // {
        //     pActor->SendTipmsgFormatWithId(tmExpLimited, tstUI);
        //     return;
        // }
        // int nBindCoin = pActor->GetProperty<unsigned int>(PROP_ACTOR_BIND_COIN);
        // if(nBindCoin < levelCfg.nCostBindCoin)
        // {
        //     pActor->SendTipmsgFormatWithId(tmNoMoreBindCoin, tstUI);
        //     return;
        // }
        // int nItemid = levelCfg.nCostId; //初级秘籍
        if(levelCfg.costs.size() > 0)
        {
            for(auto cost : levelCfg.costs) {
                ACTORAWARD& table = cost;
                if(!pActor->CheckActorSource(cost.btType,cost.wId, cost.wCount, tstUI))
                    return;
            }
        }

        if(levelCfg.costs.size() > 0)
        {
            for(auto cost : levelCfg.costs) {
                pActor->RemoveConsume(cost.btType,cost.wId, cost.wCount, -1, -1,-1,0, GameLog::Log_MeridiansUP);
            }
        }
        
        // pActor->RemoveConsume(qatExp, 0, levelCfg.nCostExp, -1, -1,-1,0, GameLog::Log_MeridiansUP);
        // pActor->RemoveConsume(0, nItemid, levelCfg.nCostBook,-1, -1,-1,0, GameLog::Log_MeridiansUP);
        // pActor->RemoveConsume(qatBindMoney, 0, levelCfg.nCostBindCoin, -1, -1,-1,0, GameLog::Log_MeridiansUP);

        nMeridinLv++;
        if(strlen(levelCfg.nTips) > 0) {
            char sText[1024];
            sprintf_s(sText, sizeof(sText), levelCfg.nTips, ( char *)((CActor*)m_pEntity->GetEntityName()));
            GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(sText,tstKillDrop);
        }
        pActor->SetProperty<unsigned int>(PROP_ACTOR_MERIDIALv,nMeridinLv);
        pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveJmLevl, nMeridinLv);
        ((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);

        CActorPacket ap;
        CDataPacket& dataPack = pActor->AllocPacket(ap);
        dataPack << (BYTE)enStaticCountSystemID <<(BYTE)enStaticCountSystemsMeridiansUPLv <<(BYTE)1 <<(int)nMeridinLv;
        ap.flush();
    }
}
//物品处理函数 
int CStaticCountSystem::UpItemInnerIndexDaily(int ItemIndex)//获得物品处理过的内部ID
{
    if (ItemIndex < 0 || ItemIndex > STATIC_TYPE_ITEM_INDEX_MAX )
    {
        return (int)eSTATIC_TYPE_INNER_ITEM_DAILY_START;
    }
    
    return ItemIndex + eSTATIC_TYPE_INNER_ITEM_DAILY_START;
}
int CStaticCountSystem::UpItemInnerIndexWeek(int ItemIndex)//获得物品处理过的内部ID
{
    if (ItemIndex < 0 || ItemIndex > STATIC_TYPE_ITEM_INDEX_MAX )
    {
        return (int)eSTATIC_TYPE_INNER_ITEM_WEEK_START;
    }
    
    return ItemIndex + eSTATIC_TYPE_INNER_ITEM_WEEK_START;
}
int CStaticCountSystem::UpItemInnerIndexMonth(int ItemIndex)//获得物品处理过的内部ID
{
    if (ItemIndex < 0 || ItemIndex > STATIC_TYPE_ITEM_INDEX_MAX )
    {
        return (int)eSTATIC_TYPE_INNER_ITEM_MONTH_START;
    }
    
    return ItemIndex + eSTATIC_TYPE_INNER_ITEM_MONTH_START;
} 
void CStaticCountSystem::OnReduceItemInnerStaticCountDaily(INT_PTR nStaticType, INT_PTR nValue)
{
    int innerindex = UpItemInnerIndexDaily(nStaticType); 
    LONGLONG last = GetStaticCount(innerindex);
    if (last - 1 >= nValue)
    {
        SetStaticCount(innerindex, last - nValue); 
    }
    else
    { 
        SetStaticCount(innerindex, 1); 
    } 
} 
void CStaticCountSystem::OnReduceItemInnerStaticCountWeek(INT_PTR nStaticType, INT_PTR nValue)
{
    int innerindex = UpItemInnerIndexWeek(nStaticType); 
    LONGLONG last = GetStaticCount(innerindex);
    if (last - 1 >= nValue)
    {
        SetStaticCount(innerindex, last - nValue); 
    }
    else
    { 
        SetStaticCount(innerindex, 1); 
    } 
} 

void CStaticCountSystem::OnReduceItemInnerStaticCountMonth(INT_PTR nStaticType, INT_PTR nValue)
{
    int innerindex = UpItemInnerIndexMonth(nStaticType);  
    LONGLONG last = GetStaticCount(innerindex);
    if (last - 1 >= nValue)
    {
        SetStaticCount(innerindex, last - nValue); 
    }
    else
    { 
        SetStaticCount(innerindex, 1); 
    } 
}  

bool CStaticCountSystem::OnGetItemInnerIndexDailyInit(int ItemIndex)
{
    int innerindex = UpItemInnerIndexDaily(ItemIndex);  
    LONGLONG last = GetStaticCount(innerindex);
    if (last == 0)
    { 
        return true;
    }
    else
    { 
        return false;
    }
}  

bool CStaticCountSystem::OnGetItemInnerIndexWeekInit(int ItemIndex)
{
    int innerindex = UpItemInnerIndexWeek(ItemIndex);  
    LONGLONG last = GetStaticCount(innerindex);
    if (last == 0)
    { 
        return true;
    }
    else
    { 
        return false;
    }
}  
bool CStaticCountSystem::OnGetItemInnerIndexMonthInit(int ItemIndex)
{
    int innerindex = UpItemInnerIndexMonth(ItemIndex);  
    LONGLONG last = GetStaticCount(innerindex);
    if (last == 0)
    { 
        return true;
    }
    else
    { 
        return false;
    }
}
LONGLONG CStaticCountSystem::OnGetItemInnerIndexDaily(int ItemIndex)
{ 
    int innerindex = UpItemInnerIndexDaily(ItemIndex);  
    LONGLONG last = GetStaticCount(innerindex); 
    
    return last - 1;
}
LONGLONG CStaticCountSystem::OnGetItemInnerIndexWeek(int ItemIndex)
{ 
    int innerindex = UpItemInnerIndexWeek(ItemIndex);  
    LONGLONG last = GetStaticCount(innerindex); 
    
    return last - 1;
}
LONGLONG CStaticCountSystem::OnGetItemInnerIndexMonth(int ItemIndex)
{ 
    int innerindex = UpItemInnerIndexMonth(ItemIndex);  
    LONGLONG last = GetStaticCount(innerindex); 
    
    return last - 1;
}
void CStaticCountSystem::OnAddOrderWardMoney(INT_PTR nValue)
{  
    LONGLONG last = GetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY);
    if (nValue < 0)
    {
        if (last >= -nValue)
        {
            SetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY, last + nValue); 
        }
        else
        { 
            SetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY, 0); 
        }  
    }
    else
    { 
        SetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY, last + nValue); 
    } 
     ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_ZL_MONEY, GetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY));
} 
void CStaticCountSystem::OnAddDimensionalKey(INT_PTR nValue)
{  
    LONGLONG last = GetStaticCount(eSTATIC_TYPE_DIMENSIONAL_KEY);
    if (nValue < 0)
    {
        if (last >= -nValue)
        {
            SetStaticCount(eSTATIC_TYPE_DIMENSIONAL_KEY, last + nValue); 
        }
        else
        { 
            SetStaticCount(eSTATIC_TYPE_DIMENSIONAL_KEY, 0); 
        }  
    }
    else
    { 
        SetStaticCount(eSTATIC_TYPE_DIMENSIONAL_KEY, last + nValue); 
    } 
    ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_DIMENSIONALKEY, GetStaticCount(eSTATIC_TYPE_DIMENSIONAL_KEY));
}