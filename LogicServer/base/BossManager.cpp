#include "StdAfx.h"
using namespace jxInterSrvComm::DbServerProto;

CBossManager::CBossManager()
{
    m_IsInited = false;
    m_nBossReferList.clear();
    m_nWaitBossReferList.clear();
}

CBossManager::~CBossManager()
{

}

// 请求DBServer加载数据
VOID CBossManager::Load()
{
    // CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    // if (pDbClient->connected())
    // {
    //     CDataPacket& dataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadBossInfo);
    //     int nServerId = GetLogicServer()->GetServerIndex();
    //     dataPacket << nServerId << nServerId;
    //     GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
    // }
}
// 推送数据到DBServer进行存储
VOID CBossManager::Save()
{
    // CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    // if (pDbClient->connected())
    // {
    //     CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveBossInfo);
    //     int nServerId = GetLogicServer()->GetServerIndex();
    //     dataPacket << nServerId << nServerId;

    //     int nCount = 0;
    //     INT_PTR pos = dataPacket.getPosition();
    //     dataPacket << nCount;
    //     // 运行中的活动
    //     std::map<int, BOSSREFERINFO>::iterator runIter = m_nBossRefers.begin();
    //     for (;runIter != m_nBossRefers.end(); ++runIter)
    //     {
    //         BOSSREFERINFO& boss = (*runIter).second;
    //         dataPacket.writeBuf(&boss.nBossInfo, sizeof(boss.nBossInfo));
    //         nCount++;
    //     }
    //     int* pCount = (int*)dataPacket.getPositionPtr(pos);
    //     *pCount = nCount;

    //     GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
    // }
}
// 处理DBServer返回数据
VOID CBossManager::OnDbReturnData(INT_PTR nCmd, char * data, SIZE_T size)
{
    // int nRawServerId, nLoginServerId;
    // unsigned int nGid = 0;
    // CDataPacketReader inPacket(data,size);
    // BYTE nErrorCode;
    // inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;

    // switch(nCmd)
    // {
    // case dcLoadBossInfo:
    //     if (nErrorCode == reSucc)
    //     {
    //         int nCount = 0;
    //         inPacket >> nCount;

    //         // 加载进行中活动列表
    //         BOSSREFERINFO boss;
    //         for(int i = 0; i < nCount; ++i)
    //         {
    //             inPacket.readBuf(&boss.nBossInfo,sizeof(boss.nBossInfo));
    //             // if(boss.nBossInfo.nDeath == 0)
    //             // {
    //             //     boss.nBossInfo.nReferTime = 0;
    //             // }
    //             boss.nBossInfo.nDeath = 0;
    //             boss.bNeedRefer = true;
    //             boss.nBossInfo.nReferTime  = 0;
    //             SetReferBossBasicInfo(boss);
    //             m_nBossRefers[boss.nBossInfo.nBossId] = boss;
    //             m_RunBossRefer.insert(&m_nBossRefers[boss.nBossInfo.nBossId]);
    //         }

    //         // 加载等待列表
    //         std::map<int,BOSSCONFIGINFO>& bossCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoList();
    //         std::map<int,BOSSCONFIGINFO>::iterator iter =  bossCfg.begin();
    //         for (; iter != bossCfg.end(); iter++)
    //         {
    //             BOSSCONFIGINFO& cfg = (*iter).second;
    //             if(cfg.nTab == 0)
    //                 continue;
    //             // 已在刷新列表
    //             if (m_nBossRefers.find(iter->first) != m_nBossRefers.end())
    //             {
    //                 continue;
    //             }

    //             BOSSREFERINFO boss;
                
    //             boss.nBossInfo.nBossId = iter->first;
    //             SetReferBossBasicInfo(boss);
    //             m_nWaitBossRefer[boss.nBossInfo.nBossId] = boss;
    //             // 不在刷新列表中，并将来会开放的boss，则放入等待列表

    //         }
            
    //         // 初始化完成
    //         m_IsInited = true;
    //     }
    //     else
    //     {
    //         OutputMsg(rmError,_T("Guild LoadGuildListResult Error!code=%d"),nErrorCode);
    //     }
    //     break;
    // }
}


bool CBossManager::CheckCondition(int nOpenCircle, int nOpenDay)
{
    //转生 和开服天数 都为0 表示直接开启
    if(nOpenCircle && !GetGlobalLogicEngine()->GetGlobalVarMgr().GetCountByCircleLevel(nOpenCircle))
        return false;
    if(nOpenDay && !GetLogicServer()->CheckOpenServer(nOpenDay))
        return false;

    return true;
}

// 帧处理
VOID CBossManager::RunOne(TICKCOUNT nTickCount)
{
    if (IsInited())
    {
		// UINT nNow = time(NULL);
        int nNow = GetGlobalLogicEngine()->getMiniDateTime();
       
        if (m_2sTimer.CheckAndSet(nTickCount))
        {
            BossIterator waitIter = m_nWaitBossReferList.begin();
            for (;waitIter != m_nWaitBossReferList.end();)
            {
                BOSSREFERINFO& boss = waitIter->second;
                BOSSCONFIGINFO* pOneBoss = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(boss.nBossId);
                if(pOneBoss && CheckCondition(pOneBoss->nOpenCircle, pOneBoss->nOpenDay))
                {
                    // 放入运行列表
                    if(m_nBossReferList.find(boss.nBossId) != m_nBossReferList.end())
                    {
                        OutputMsg(rmError,"[boss] bossId=%d !",(int)boss.nBossId);
                    }
                    boss.nReferTime = pOneBoss->GetNextReferTime(boss.nReferTime);
                    boss.bNeedRefer = true;
                    boss.bIsPush = true;
                    boss.nDeath = 1;
                    m_nBossReferList[boss.nBossId] = boss;
                    m_RunningReferList.insert(&m_nBossReferList[boss.nBossId]);
                    m_nMapBossList[pOneBoss->nMapId].push_back(&m_nBossReferList[boss.nBossId]);
                    // 删除
                    waitIter = m_nWaitBossReferList.erase(waitIter);
                }else
                {
                    waitIter++;
                }
                
            }
        }

        if (m_1sTimer.CheckAndSet(nTickCount))
        {
            RunBossIter runIter = m_RunningReferList.begin();
            for (;runIter != m_RunningReferList.end(); ++runIter)
            {
                BOSSREFERINFO& boss = *(*runIter);
               
                if(boss.nReferTime <= nNow)
                {
                    if(boss.nDeath == 1 && boss.bNeedRefer)
                    {
                        BOSSCONFIGINFO* pOneBoss = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(boss.nBossId);
                        if(pOneBoss )
                        {
                            if(!boss.bIsPush && pOneBoss->nNotice == 1)
                            {
                               
                                GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(pOneBoss->nContent, tstBigRevolving);
                            }
                             if(pOneBoss->nShowwindow) {
                                 
                                char bossId[10] = {0};
                                sprintf_s(bossId, sizeof(bossId), "%d", pOneBoss->nSerial);
                                GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(bossId, tstBossRelive);
                            }
                            boss.bIsPush = true;
                            
                            boss.nReferTime = pOneBoss->GetNextReferTime(boss.nReferTime);

                            BossCallLuaScript(boss.nBossId, CBossSystem::enOnInit);
                       
                        }
                        boss.nDeath = 0;
                        CreateBossEntity(boss);
                    }
                }
                // else
                // {
                //     break;
                // }
                
            }
        }
    }
}

void CBossManager::CreateBossEntity(BOSSREFERINFO& boss)
{
    // CScene* pScene = NULL;
   
    BOSSCONFIGINFO* pOneBoss = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(boss.nBossId);
    if(pOneBoss )
    {
        CScene* pScene = NULL;
        CFuBen * pRetFb =NULL;
        GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneId(pOneBoss->nFubenId, pOneBoss->nMapId, pRetFb, pScene);
        if(!pScene) return;
        int nBossId = boss.nBossId%BOSSSOCAIL_MUTIPLY;
        if(boss.bNeedRefer && boss.nDeath == 0)
        {
            BOSSGROWDATA* pGrowCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossGrowDataById(nBossId);
            if(pGrowCfg)
            {
                boss.nBossLv = pGrowCfg->GetBossGrowLv( GetLogicServer()->GetDaysSinceOpenServer(), GetGlobalLogicEngine()->GetTopTitleMgr().GetTopMaxLevel(), &GetGlobalLogicEngine()->GetGlobalVarMgr());
                boss.nBuffAddValue = pGrowCfg->GetBuffAddValue(boss.nBossLv);
            }
            REFRESHCONFIG referCfg;
            referCfg.nEntityId =nBossId;
            referCfg.nMobX = boss.nReferX;
            referCfg.nMobY = boss.nReferY;
            referCfg.nMobRange = boss.nReferRange;
            referCfg.nEntityLevel = boss.nBossLv;
            int nCount = 1;
        
            CEntity* pMoster = CreateBossEntity(boss, referCfg, pScene, nCount);
            if(pMoster)
            {
                boss.bNeedRefer = false;
            }
        
        }
    }
}



void CBossManager::CreateBossEntity(CScene* pScene)
{
    // CScene* pScene = NULL;
    // GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneId(nSceneId, NULL, pScene);
    if(!pScene) return;
    int nBuffValue = 0;
    int nReferMapId = pScene->GetSceneId();
    MapBossIter runIter = m_nMapBossList.find(nReferMapId);
    if(runIter != m_nMapBossList.end())
    {
        if(runIter->second.size() == 0)
            return;
        int nSize = runIter->second.size();
        for(int i = 0; i < nSize; i++)
        {
            BOSSREFERINFO& boss = *(runIter->second[i]);
            int nBossId = boss.nBossId%BOSSSOCAIL_MUTIPLY;
            if(boss.bNeedRefer && boss.nDeath == 0)
            {
                BOSSGROWDATA* pGrowCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossGrowDataById(nBossId);
                if(pGrowCfg)
                {
                    boss.nBossLv = pGrowCfg->GetBossGrowLv( GetLogicServer()->GetDaysSinceOpenServer(), GetGlobalLogicEngine()->GetTopTitleMgr().GetTopMaxLevel(), &GetGlobalLogicEngine()->GetGlobalVarMgr());
                    boss.nBuffAddValue = pGrowCfg->GetBuffAddValue(boss.nBossLv);
                }
                REFRESHCONFIG referCfg;
                referCfg.nEntityId =nBossId;
                referCfg.nMobX = boss.nReferX;
                referCfg.nMobY = boss.nReferY;
                referCfg.nMobRange = boss.nReferRange;
                referCfg.nEntityLevel = boss.nBossLv;
                int nCount = 1;
            
                CEntity* pMoster = CreateBossEntity(boss, referCfg, pScene, nCount);
                if(pMoster)
                {
                   boss.bNeedRefer = false;
                }
            
            }
        }
    }
}


CEntity* CBossManager::CreateBossEntity(BOSSREFERINFO& boss, REFRESHCONFIG& referCfg, CScene* pScene, int Index)
{
    if(!pScene) return NULL;
    INT_PTR nMonsterPosX = 0;//pRefreshConf->Points[seed].x;
    INT_PTR nMonsterPosY = 0;//pRefreshConf->Points[seed].y;
    if (!pScene->GetMobMonsterXY(&referCfg,nMonsterPosX,nMonsterPosY))
    {
        return NULL;
    }
    static CLogicDataProvider *pProvider = GetLogicServer()->GetDataProvider();
    PMONSTERCONFIG pMonsterConfig = pProvider->GetMonsterConfig().GetMonsterData(referCfg.nEntityId);
    if(pMonsterConfig == NULL) return NULL;
    //这里属于配置有问题
    switch (pMonsterConfig->btEntityType )
    {
    case enActor:
    case enHero:
    case enPet:
        return NULL;
    }
    //boss.nBuffAddValue = 200;
    if (boss.nBuffAddValue < 100)
        boss.nBuffAddValue = 100;
    CEntity* pEntity = pScene->CreateEntityAndEnterScene(referCfg.nEntityId, 
                                                    pMonsterConfig->btEntityType,	
                                            nMonsterPosX,nMonsterPosY,-1,NULL,
                                            referCfg.nLiveTime, 0,NULL, 0,boss.nBuffAddValue);
    if(pEntity)
    {
        pEntity->SetProperty<unsigned int>(PROP_MONSTER_BELONG_ID, 0);

        CMonster *pMonster = ((CMonster*)pEntity);
        pMonster->SetBossReferId(boss.nBossId);
        OutputMsg(rmTip, "monster %d, nBuffAddValue:%d", pMonster->GetBossReferId(),boss.nBuffAddValue);
    }

}


BOSSREFERINFO* CBossManager::GetBossInfo(int nBossId)
{
    BossIterator it = m_nBossReferList.find(nBossId);
    if(it != m_nBossReferList.end())
    {
        return &(it->second);
    }
    return NULL;
}


void CBossManager::BossDeath(int nBossId)
{
    // OutputMsg(rmTip, "BossDeath =%d", nBossId);
    BossIterator it = m_nBossReferList.find(nBossId);
    if(it != m_nBossReferList.end())
    {
        OutputMsg(rmTip, "boss find sucess =%d", nBossId);
        BOSSREFERINFO& boss = it->second;
        boss.nDeath = 1;
        boss.bNeedRefer = true;
        boss.bIsPush = false;
        unsigned int nTime = GetGlobalLogicEngine()->getMiniDateTime();
        BOSSCONFIGINFO* pOneBoss = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(boss.nBossId);
        if(pOneBoss)
        {
            boss.nReferTime = pOneBoss->GetNextReferTime(nTime);
        }
        // if(pOneBoss && pOneBoss->nReborn == 2) //死亡递延
        //     boss.nReferTime = pOneBoss->GetNextReferTime(time(NULL));
            // boss.nReferTime = pOneBoss->GetNextReferTime(time(NULL));


        // 取消非活动副本Boss归属
        int nTempBossId = boss.nBossId % BOSSSOCAIL_MUTIPLY;
        CMonster* pMonster = GetGlobalLogicEngine()->GetEntityMgr()->GetMonsterPtrByEntityId(nTempBossId);
        if ( pMonster )
        {
            pMonster->SetProperty<unsigned int>(PROP_MONSTER_BELONG_ID, 0);
        }
    }
}

bool CBossManager::GetBossInfoIsDeath(int nBossId)
{
    BossIterator runIter = m_nBossReferList.begin();
    for (;runIter != m_nBossReferList.end(); ++runIter)
    {
        BOSSREFERINFO& boss = runIter->second;
        int nId = boss.nBossId % BOSSSOCAIL_MUTIPLY;
        if(nId == nBossId )
        {
            if(boss.nDeath != 1)
                return false;
        }
    }
    return true;
}



int CBossManager::GetBossMinReferTime(int nBossId)
{
    BossIterator runIter = m_nBossReferList.begin();
    int nMinTime = 0;
    for (;runIter != m_nBossReferList.end(); ++runIter)
    {
        
        BOSSREFERINFO& boss = runIter->second;
        int nId = boss.nBossId % BOSSSOCAIL_MUTIPLY;
        if(nId == nBossId && ( nMinTime == 0 || nMinTime > boss.nReferTime))
        {
            nMinTime = boss.nReferTime;
        }
    }

    return nMinTime;
}


void CBossManager::SetReferBossBasicInfo(BOSSREFERINFO& boss)
{
    BOSSCONFIGINFO* pBossCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(boss.nBossId);
    if(pBossCfg)
    {
        boss.nReferX = pBossCfg->nBossX;
        boss.nReferY =  pBossCfg->nBossY;
        boss.nReferRange =  pBossCfg->nBossRange;
        boss.nReferMapId =  pBossCfg->nMapId;
        BOSSGROWDATA* pGrowCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossGrowDataById(pBossCfg->nBossId);
        if(pGrowCfg)
        {
            boss.nBuffAddValue = pGrowCfg->GetBuffAddValue(boss.nBossLv);
        }
    }
    
}


//服务器启动就需要全部重新刷新boss
bool CBossManager::Initialize()
{

    // 加载等待列表
    std::map<int,BOSSCONFIGINFO>& bossCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoList();
    std::map<int,BOSSCONFIGINFO>::iterator iter =  bossCfg.begin();
    for (; iter != bossCfg.end(); iter++)
    {
        BOSSCONFIGINFO& cfg = (*iter).second;
        if(cfg.nTab == 0 || cfg.nReborn == 0)
            continue;
        // 已在刷新列表
        if (m_nBossReferList.find(iter->first) != m_nBossReferList.end())
        {
            continue;
        }
        BOSSREFERINFO boss;
        
        boss.nBossId = iter->first;
        boss.bNeedRefer = true;
        boss.nReferTime = cfg.GetNextReferTime(boss.nReferTime);
        boss.bIsPush = true;
        boss.nDeath = 1;
        SetReferBossBasicInfo(boss);
        //if(GetGlobalLogicEngine()->GetGlobalVarMgr().GetCountByCircleLevel(cfg.nOpenCircle) && GetLogicServer()->CheckOpenServer(cfg.nOpenDay))
        if(CheckCondition(cfg.nOpenCircle, cfg.nOpenDay))
        {
            m_nBossReferList[boss.nBossId] = boss;
            m_RunningReferList.insert(&m_nBossReferList[boss.nBossId]);
            m_nMapBossList[cfg.nMapId].push_back(&m_nBossReferList[boss.nBossId]);
        }
        else
        {
            m_nWaitBossReferList[boss.nBossId] = boss;
        }
    }
    
    // 初始化完成
    m_IsInited = true;
    return true;
}


void CBossManager::BossCallLuaScript(int nBossId, int nType, CActor* pActor, CDataPacket* data)
{
	BOSSCONFIGINFO* pOneBoss = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(nBossId);
	if(pOneBoss)
	{
        int nMonsterType = 1;
        int nid = nBossId % BOSSSOCAIL_MUTIPLY;
        PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nid);
		if(pMonsterConfig)
        {
            nMonsterType = pMonsterConfig->btMonsterType;
        }
        CScriptValueList paramList,retList;
        paramList << (int)nMonsterType;//(int)EnBossEvent::enOnCancel;
		paramList << (int)nType;//(int)EnBossEvent::enOnCancel;
		paramList << (int)pOneBoss->nFubenId;
		paramList << (int)pOneBoss->nMapId;
		paramList << (int)nBossId;
		paramList << pActor;
        paramList << data;
        
		if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("MonDispatcher","OnEvent", paramList, retList, 1))
		{
			OutputMsg(rmError,"[MonDispatcher] OnEvent nType 错误，页签类型ID=%d，场景=%d， bossId= %d  !",pOneBoss->nTab,pOneBoss->nMapId, nBossId);
		}
	}
}
