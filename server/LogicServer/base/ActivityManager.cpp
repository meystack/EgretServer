#include "StdAfx.h"
#include "ActivityManager.h"

using namespace jxInterSrvComm::DbServerProto;

CActivityComponent::CActivityComponent()
{
}

CActivityComponent::~CActivityComponent()
{
}


// 请求DBServer加载数据
VOID CActivityComponent::Load()
{
    CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected())
    {
        CDataPacket& dataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadActivityList);
        int nServerId = GetLogicServer()->GetServerIndex();
        dataPacket << nServerId << nServerId;
        GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
    }
}
// 推送数据到DBServer进行存储
VOID CActivityComponent::Save()
{
    CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected())
    {
        CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveActivityList);
        int nServerId = GetLogicServer()->GetServerIndex();
        dataPacket << nServerId << nServerId;

        int nCount = 0;
        INT_PTR pos = dataPacket.getPosition();
        dataPacket << nCount;
        // 运行中的活动
        std::map<int, GlobalActivityData>::iterator runIter = m_RunningActivity.begin();
        for (;runIter != m_RunningActivity.end(); ++runIter)
        {
            GlobalActivityData& activitydata = (*runIter).second;
            dataPacket.writeBuf(&activitydata,sizeof(activitydata));
            OutputMsg(rmTip,"[Global Activity] 活动存储(%d)，活动Type=%d, 活动ID=%d !", nCount, activitydata.nActivityType, activitydata.nId);
            nCount++;
        }
        int* pCount = (int*)dataPacket.getPositionPtr(pos);
        *pCount = nCount;

        GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
    }
}
// 处理DBServer返回数据
VOID CActivityComponent::OnDbReturnData(INT_PTR nCmd, char * data, SIZE_T size)
{
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    int nRawServerId, nLoginServerId;
    unsigned int nGid = 0;
    CDataPacketReader inPacket(data,size);
    BYTE nErrorCode;
    inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;

    switch(nCmd)
    {
    case dcLoadActivityList:
        if (nErrorCode == reSucc)
        {
            CScriptValueList paramList;
            int nCount = 0;
            inPacket >> nCount;

            // 配置的全局活动
            std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();

            // 加载进行中活动列表
            GlobalActivityData activitydata;
            for(int i = 0; i < nCount; ++i)
            {
                inPacket.readBuf(&activitydata, sizeof(activitydata)); 
                OutputMsg(rmTip,"[Global Activity] 活动加载(%d)，活动Type=%d, 活动ID=%d !", i, activitydata.nActivityType, activitydata.nId);
                if (globalActivityConfs.find(activitydata.nId) != globalActivityConfs.end())
                { 
                    GLOBALACTIVITY& activityConf = globalActivityConfs[activitydata.nId];
                 
                    bool result = false;
                    //更新数据库重新启动时候的任务时间 循环活动的nTimeIdx永远不等于nTimeCount，这个nTimeIdx已在UpdateOpenTime中维护了。
                    if (activityConf.nTimeIdx != activityConf.nTimeCount)
                    {
                        result = GetLogicServer()->GetDataProvider()->GetActivityConfigs().UpdateOpenTime(GetGlobalLogicEngine()->GetGlobalNpcLua(), activityConf, /*计算当前时间*/false);
                        if (result)
                        {
                            activitydata.nStartTime = activityConf.nStartTime;
                            activitydata.nEndTime = activityConf.nEndTime;
                            activitydata.nNextTime = false;//不能设置true
                        }
                    }
                    if (!result)
                    {
                        continue;//如果活动过期了，则过
                    }  

                    m_RunningActivity[activitydata.nId] = activitydata;
                    m_RunningSeq.insert(&m_RunningActivity[activitydata.nId]);
                    if (globalActivityConfs[activitydata.nId].nTimeType == 3)
                    {
                        m_CircleActivityMap[activitydata.nId] = &m_RunningActivity[activitydata.nId];
                    }

                    // 触发加载活动事件
                    paramList.clear();
                    paramList << (int)CActivitySystem::enOnLoad;
                    paramList << (int)activitydata.nActivityType;
                    paramList << (int)activitydata.nId;
                    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                    {
                        OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnLoad，活动Type=%d, 活动ID=%d !",activitydata.nActivityType,activitydata.nId);
                    }
                }
            }

            // 加载等待列表
            std::map<int,GLOBALACTIVITY>::iterator iter =  globalActivityConfs.begin();
            for (; iter != globalActivityConfs.end(); iter++)
            {
                GLOBALACTIVITY& activityConf = (*iter).second;

                // 运行中的活动，不加入等待列表
                if (m_RunningActivity.find(activityConf.nId) != m_RunningActivity.end())
                {
                    continue;
                }

                // 不在运行中，并将来会开启的活动，则放入等待列表（都配置时，之后都不开的活动，将）
                AddActivity(&activityConf);
            }
            
            // 初始化完成
            m_IsInited = true;

            // 加载等待公告
            auto& globalNoticeConfs = GetLogicServer()->GetDataProvider()->GetNoticeConfigs().getNoticeConf();
            auto notice_iter = globalNoticeConfs.begin();
            for (; notice_iter != globalNoticeConfs.end(); ++notice_iter)
            {
                GLOBALNOTICE& noticeConf = (*notice_iter).second;
                m_WaittingNotice.insert(&noticeConf);
            }
        }
        else
        {
            OutputMsg(rmError,_T("Guild LoadGuildListResult Error!code=%d"),nErrorCode);
        }
        break;
    }
}
// 玩家登录时
VOID CActivityComponent::OnEnterScene(CActor* pActor)
{ 
    typedef std::map<int, GlobalActivityData>::iterator Iterator;
	DECLARE_TIME_PROF("CActivityComponent::OnEnterScene");
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    typedef std::map<int, GlobalActivityData>::iterator Iterator;
     std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
    if (pActor)
    { 
        if (pActor->OnGetIsTestSimulator()
           || pActor->OnGetIsSimulator() )
        {
            return;
        }

        CActorPacket pack;
        CDataPacket & outPack = pActor->AllocPacket(pack);
        outPack << (BYTE)enActivityID << (BYTE)sSendGlobalActivity;
		INT_PTR nOffer = outPack.getPosition();
        outPack << (short)0;

        CScriptValueList paramList;
        short num = 0;
        Iterator runIter = m_RunningActivity.begin();
        for (;runIter != m_RunningActivity.end();++runIter)
        {
            GlobalActivityData& activity = (*runIter).second;
            int nLevel = 1;
            int nCircle = 1;
           
            if (globalActivityConfs.find(activity.nId) != globalActivityConfs.end())
            {
                nLevel = globalActivityConfs[activity.nId].nOpenLevel;
                nCircle = globalActivityConfs[activity.nId].nOpenCircle;
            }
            if(!pActor->CheckLevel(nLevel, nCircle)) continue;
            // 记录每个活动数据长度
            INT_PTR len_offset = outPack.getPosition();
            outPack << (unsigned short)0;

            // 活动固定数据
            outPack << (unsigned int)activity.nId;
            outPack << (unsigned int)activity.nStartTime;
            outPack << (unsigned int)activity.nEndTime;

            // 触发获取活动数据
            paramList.clear();
            paramList << (int)CActivitySystem::enOnReqData;
            paramList << (int)activity.nActivityType;
            paramList << (int)activity.nId;
            paramList << pActor;
            paramList << &pack;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnReqData，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
            }

            // 调整活动数据长度
            INT_PTR len_curoffset = outPack.getPosition();
            outPack.setPosition(len_offset);
            outPack << (short)(len_curoffset - len_offset);
            outPack.setPosition(len_curoffset);

            num++;
        }
        INT_PTR nCurOffer = outPack.getPosition();
        outPack.setPosition(nOffer);
        outPack << (short)num;
        outPack.setPosition(nCurOffer);
		pack.flush();
    }
}

// 帧处理
VOID CActivityComponent::RunOne(TICKCOUNT nTickCount)
{
    if (IsInited())
    {
        static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();

        if (m_1sTimer.CheckAndSet(nTickCount))
        {
	        DECLARE_TIME_PROF("CActivityComponent::RunOne");
            // 运行中的活动
            RunIterator runIter = m_RunningSeq.begin();
            for (;runIter != m_RunningSeq.end();)
            {
                if ((*runIter)->nEndTime !=0 && GetGlobalLogicEngine()->getMiniDateTime() > (*runIter)->nEndTime)
                {
                    GlobalActivityData& activity = m_RunningActivity[(*runIter)->nId];

                    // 结束活动，触发Lua脚本结束事件
                    CScriptValueList paramList;
                    paramList << CActivitySystem::enOnEnd;
                    paramList << activity.nActivityType;
                    paramList << activity.nId;
                    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                    {
                        //脚本错误，将以模态对话框的形式呈现给客户端
                        //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                        OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnEnd，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
                    }

                    // 全服推送活动结束
                    char buff[1024];
                    CDataPacket outPack(buff, sizeof(buff));
                    outPack << (BYTE)enActivityID << (BYTE)sSendActivityEnd;
                    outPack << (int)activity.nId;
                    GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());

                    
                    int SupportPAtvFlag = 0 ;
                    std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
                    if (globalActivityConfs.find(activity.nId)!=globalActivityConfs.end())
                    {
                        SupportPAtvFlag = globalActivityConfs[activity.nId].nSupportPAtv ;
                    }

                    if(SupportPAtvFlag !=0 )
                    {
                        // 全服在线玩家推送活动结束
                        CActor *pActor;
                        CVector<void*> actorList;
                        GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
                        for (INT_PTR i = 0; i < actorList.count(); i++)
                        {
                            // 推送活动信息
                            pActor = (CActor *)actorList[i];

                            if (pActor->OnGetIsTestSimulator()
                                || pActor->OnGetIsSimulator() )
                            {
                                continue;//假人不需要接受消息
                            } 

                            CScriptValueList paramList;
                            paramList << CActivitySystem::enOnGPEnd;
                            paramList << activity.nActivityType;
                            paramList << activity.nId;
                            paramList << pActor;
                            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                            {
                                //脚本错误，将以模态对话框的形式呈现给客户端
                                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                                //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPEnd，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                                OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPEnd，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
                            }
                        }
                    }
                    
                    // 单活动的多时间段处理（包括循环活动）
                    //std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
                    if(globalActivityConfs.find(activity.nId) != globalActivityConfs.end())
                    {
                        GLOBALACTIVITY& activityConf = globalActivityConfs[activity.nId];
                            
                        // 循环活动的nTimeIdx永远不等于nTimeCount，这个nTimeIdx已在UpdateOpenTime中维护了。
                        if (activityConf.nTimeIdx != activityConf.nTimeCount)
                        {
                            bool result = GetLogicServer()->GetDataProvider()->GetActivityConfigs().UpdateOpenTime(GetGlobalLogicEngine()->GetGlobalNpcLua(),activityConf,activity.nNextTime);
                            if (result)
                            {
                                activity.nStartTime = activityConf.nStartTime;
                                activity.nEndTime = activityConf.nEndTime;
                                activity.nNextTime = false;
                                
                                if(m_WaitingActivity.find(activity.nId) != m_WaitingActivity.end())
                                {
                                    OutputMsg(rmError,"[Global Activity] 运行中的循环活动，却在等待列表中，活动ID=%d !",(int)activity.nId);
                                }
                                m_WaitingActivity[activity.nId] = activity;
                                m_WaitingSeq.insert(&m_WaitingActivity[activity.nId]);
                                if(m_CircleActivityMap.find(activity.nId) != m_CircleActivityMap.end())
                                    m_CircleActivityMap.erase(m_CircleActivityMap.find(activity.nId));
                            }
                            
                        }
                    }
                    
                    // 删除
                    m_RunningActivity.erase(m_RunningActivity.find(activity.nId));
                    m_RunningSeq.erase(runIter++);
                }
                else
                {
                    // 运行中的活动
                    GlobalActivityData& activity = m_RunningActivity[(*runIter)->nId];

                    // 更新活动，触发Lua脚本结束事件
                    CScriptValueList paramList;
                    paramList << CActivitySystem::enOnUpdate;
                    paramList << activity.nActivityType;
                    paramList << activity.nId;
                    paramList << (int)nNow;
                    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                    {
                        //脚本错误，将以模态对话框的形式呈现给客户端
                        //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                        //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnUpdate，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                        OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnUpdate，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
                    }
                    runIter++;
                }
            }

            // 等待中的活动
            WaitIterator waitIter = m_WaitingSeq.begin();
            for (;waitIter != m_WaitingSeq.end();)
            {
                
                //开服前3天关闭防沉迷
                CMiniDateTime openTime = GetLogicServer()->GetServerOpenTime();

                SYSTEMTIME StartTime; 
                CMiniDateTime nStartTime((*waitIter)->nStartTime);
                nStartTime.decode(StartTime);

                SYSTEMTIME curTime;  
                CMiniDateTime ncurTime(GetGlobalLogicEngine()->getMiniDateTime());
                ncurTime.decode(curTime);
                if (GetGlobalLogicEngine()->getMiniDateTime() >= (*waitIter)->nStartTime)
                {
                    GlobalActivityData& activity = *(*waitIter);

                    // 放入运行列表
                    if(m_RunningActivity.find(activity.nId) != m_RunningActivity.end())
                    {
                        OutputMsg(rmError,"[Global Activity] 等待中的循环活动，却在运行列表中，活动ID=%d !",(int)activity.nId);
                    }
                    m_RunningActivity[activity.nId] = activity;
                    m_RunningSeq.insert(&m_RunningActivity[activity.nId]);




                    OutputMsg(rmTip, "[Global Activity] 全局活动新开启，活动Type=%d, 活动ID=%d !", (int)activity.nActivityType, (int)activity.nId);

                    // 开启活动，触发Lua脚本结束事件
                    CScriptValueList paramList;
                    paramList << CActivitySystem::enOnStart;
                    paramList << activity.nActivityType;
                    paramList << activity.nId;
                    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                    {
                        //脚本错误，将以模态对话框的形式呈现给客户端
                        //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                        //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnStart，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                        OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnStart，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
                    }

                    // 是否开启活动发广播
                    bool bPopupWhenStart = false;
                    int nLevel = 1;
                    int nCircle = 1;
                    int SupportPAtvFlag = 0 ;
                    std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
                    if (globalActivityConfs.find(activity.nId)!=globalActivityConfs.end())
                    {
                        bPopupWhenStart = globalActivityConfs[activity.nId].bPopupWhenStart;
                        nLevel = globalActivityConfs[activity.nId].nPopLevel;
                        nCircle = globalActivityConfs[activity.nId].nPopCircle;
                        SupportPAtvFlag = globalActivityConfs[activity.nId].nSupportPAtv ;
                    }

                    // 全服在线玩家推送活动开启
		            CActor *pActor;
                    CVector<void*> actorList;
                    GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
                    for (INT_PTR i = 0; i < actorList.count(); i++)
                    {
                        // 推送活动信息
                        pActor = (CActor *)actorList[i];

                        if (pActor->OnGetIsTestSimulator()
                            || pActor->OnGetIsSimulator() )
                        {
                            continue;//假人不需要接受消息
                        }
                        if(SupportPAtvFlag !=0 )
                        {
                            CScriptValueList paramList;
                            paramList << CActivitySystem::enOnGPStart;
                            paramList << activity.nActivityType;
                            paramList << activity.nId;
                            paramList << pActor;
                            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                            {
                                //脚本错误，将以模态对话框的形式呈现给客户端
                                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                                //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPStart，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                                OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPStart，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
                            }

                        } 

                        SendOneActivity(pActor,activity.nId);

                        // 活动弹框
                        if (bPopupWhenStart)
                        {
                            if (pActor->GetLevel() >= nLevel && pActor->GetCircleLevel() >= nCircle)
                            {
                                CActorPacket pack;
                                CDataPacket & outPack = pActor->AllocPacket(pack);
                                outPack << (BYTE)enActivityID << (BYTE)SActivityPopup;
                                outPack << (int)activity.nId;
                                pack.flush();
                            }
                        }

                        // 触发进入活动区域事件
                        int nPosX,nPosY;
                        pActor->GetPosition(nPosX,nPosY);
                        CScene* pScene = pActor->GetScene();
                        if (pScene)
                        {
                            SCENEAREA* pArea = pScene->GetAreaAttri(nPosX, nPosY);
                            if (pArea && pScene->HasMapAttribute(nPosX,nPosY,aaActivity,pArea))
                            {
                                bool isInAtvArea = false;
		                        AreaAttri * pAreaAttr = &pArea->attri[aaActivity];
		                        for (size_t i = 0; i < pAreaAttr->nCount; i++)
                                {
                                    if (pAreaAttr->pValues[i] == activity.nId)
                                    {
                                        paramList.clear();
                                        paramList << (int)CActivitySystem::enOnEnterArea;
                                        paramList << activity.nActivityType;
                                        paramList << activity.nId;
                                        paramList << pActor;
                                        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                                        {
                                            OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnExitArea，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
                                        }
                                        pActor->CrossActivityArea();
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // 记录循环活动
                    if (activity.nTimeType == 3)
                    {
                        m_CircleActivityMap[activity.nId] = &m_RunningActivity[activity.nId];
                    }

                    // 删除
                    m_WaitingActivity.erase(m_WaitingActivity.find(activity.nId));
                    m_WaitingSeq.erase(waitIter++);
                }
                else
                {
                    break;//应该最先开启的活动都不满足开启时间，后面的活动开启时间也不达到
                }
            }
        
            // 启动的公告
            auto& globalNoticeConfs = GetLogicServer()->GetDataProvider()->GetNoticeConfigs().getNoticeConf();
            auto runnotice_iter = m_RunningNotice.begin();
            for (;runnotice_iter != m_RunningNotice.end();)
            {
                GLOBALNOTICE& notice = *(*runnotice_iter);

                // 结束
                if (notice.nEndTime !=0 && GetGlobalLogicEngine()->getMiniDateTime() > notice.nEndTime)
                {
                    if(globalNoticeConfs.find(notice.nId) != globalNoticeConfs.end())
                    {
                        if (notice.nTimeIdx != notice.nTimeCount)
                        {
                            bool result = GetLogicServer()->GetDataProvider()->GetNoticeConfigs().UpdateOpenTime(GetGlobalLogicEngine()->GetGlobalNpcLua(),notice,false);
                            if (result)
                            {
                                m_WaittingNotice.insert(&notice);
                            }
                        }
                    }
                    runnotice_iter = m_RunningNotice.erase(runnotice_iter);
                }
                // 公告一次
                else
                {
                    if (GetGlobalLogicEngine()->getMiniDateTime() >= notice.nNextNotice)
                    {
                        if (notice.boIsTrundle)
                            GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(notice.sContent,tstBigRevolving,0);
                        
                        if (notice.boIsChat)
                            GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(notice.sContent,tstChatSystem);
                        
                        notice.nNextNotice += notice.nInterval;
                    }
                    ++runnotice_iter;
                }
            }

            // 等待的公告
            auto waitnotice_iter = m_WaittingNotice.begin();
            for (;waitnotice_iter != m_WaittingNotice.end();)
            {
                GLOBALNOTICE& notice = *(*waitnotice_iter);

                // 时间到
                if (GetGlobalLogicEngine()->getMiniDateTime() >= notice.nStartTime)
                {
                    notice.nNextNotice = notice.nStartTime;
                    m_RunningNotice.insert(&notice);
                    waitnotice_iter = m_WaittingNotice.erase(waitnotice_iter);
                }
                else break;
            }
        }

        if (m_2sSpecialTimer.CheckAndSet(nTickCount, true))
        {
            //检查跨服领主的活动
            
            // 运行中的活动
            RunIterator runIter = m_RunningSeq.begin();
            for (;runIter != m_RunningSeq.end();)
            {
                if ((*runIter)->nEndTime != 0 
                    && GetGlobalLogicEngine()->getMiniDateTime() > (*runIter)->nEndTime)
                { 
                    runIter++; 
                    continue;
                }  
                // 运行中的活动
                GlobalActivityData& activity = m_RunningActivity[(*runIter)->nId];
                if (activity.nActivityType != 19)
                {
                    runIter++; 
                    continue;
                }

                // 更新活动，触发Lua脚本结束事件
                CScriptValueList paramList;
                paramList << CActivitySystem::enOnAtvRank;
                paramList << activity.nActivityType;
                paramList << activity.nId;
                paramList << (int)nNow;
                if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                {
                    //脚本错误，将以模态对话框的形式呈现给客户端
                    //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                    //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnUpdate，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                    OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnUpdate，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
                }
                runIter++; 
            } 
        }
        
        if (m_1hTimer.CheckAndSet(nTickCount,true))
        {
            Save();
        }

        // if (m_5sTimer.CheckAndSet(nTickCount))
        // {
        //     RunIterator runIter = m_RunningSeq.begin();
        //     for (;runIter != m_RunningSeq.end(); ++runIter)
        //     {
        //         // 运行中的活动
        //         GlobalActivityData& activity = m_RunningActivity[(*runIter)->nId];

        //         // 更新活动，触发Lua脚本结束事件
        //         CScriptValueList paramList;
        //         paramList << CActivitySystem::enOnUpdate;
        //         paramList << activity.nActivityType;
        //         paramList << activity.nId;
		//         paramList << (int)nNow;
        //         if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        //         {
        //             //脚本错误，将以模态对话框的形式呈现给客户端
        //             OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnEnd，活动Type=%d, 活动ID=%d ! \n%s",activity.nActivityType,activity.nId);
        //         }
        //     }
        // }
    }
}

bool CActivityComponent::AddActivity(PGLOBALACTIVITY pActivity)
{
    if (pActivity) {
        if ((m_RunningActivity.find(pActivity->nId) == m_RunningActivity.end())
            && (m_WaitingActivity.find(pActivity->nId) == m_WaitingActivity.end()))
        {
            GlobalActivityData& activity = m_WaitingActivity[pActivity->nId];
            activity.nId = pActivity->nId;			                // 活动ID
            activity.nActivityType = pActivity->nActivityType;	    // 活动类型
            activity.nTimeType = pActivity->nTimeType;		        // 时间类型
            activity.nStartTime = pActivity->nStartTime;          // 开始时间
            activity.nEndTime = pActivity->nEndTime;               // 
            m_WaitingSeq.insert(&activity);
            return true;
        }
    } return false;
}

bool CActivityComponent::IsTypeRunning(int nAtvType)
{
    std::vector<int>& AtvIdList = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetActivityIdList(nAtvType);
    for (size_t i = 0; i < AtvIdList.size(); i++)
    {
        if(IsRunning(AtvIdList[i])) return true;
    }
    return false;
}

void CActivityComponent::SendOneActivity(CActor *pActor,int nAtvId)
{
	DECLARE_TIME_PROF("CActivityComponent::SendOneActivity");
    if (!pActor) return;
    GlobalActivityData* pActivity = GetActivity(nAtvId);
    if (!pActivity) return;

    int nLevel = 1;
    int nCircle = 1;
    std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
    if (globalActivityConfs.find(nAtvId)!=globalActivityConfs.end())
    {
        nLevel = globalActivityConfs[nAtvId].nOpenLevel;
        nCircle = globalActivityConfs[nAtvId].nOpenCircle;
    }
    if(!pActor->CheckLevel(nLevel, nCircle)) return;

    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    CScriptValueList paramList;
    OutputMsg(rmTip,"[SendOneActivity] nActivityType:%d, 活动ID=%d !",pActivity->nActivityType,nAtvId);
    CActorPacket pack;
    CDataPacket & outPack = pActor->AllocPacket(pack);
    outPack << (BYTE)enActivityID << (BYTE)sSendGlobalActivityUpdate;
    outPack << (unsigned int)pActivity->nId;
    outPack << (unsigned int)pActivity->nStartTime;
    outPack << (unsigned int)pActivity->nEndTime;
    paramList << (int)CActivitySystem::enOnReqData;
    paramList << (int)pActivity->nActivityType;
    paramList << (int)pActivity->nId;
    paramList << pActor;
    paramList << &pack;
    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
    {
        OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnReqData，活动Type=%d, 活动ID=%d !",pActivity->nActivityType,pActivity->nId);
    }
    pack.flush();
}

unsigned int CActivityComponent::GetStartTime(int nAtvId)
{
    GlobalActivityData* pActivity = GetActivity(nAtvId);
    if (pActivity)
    {
        return (unsigned int)pActivity->nStartTime;
    }
    return 0;
}

unsigned int CActivityComponent::GetEndTime(int nAtvId)
{
    GlobalActivityData* pActivity = GetActivity(nAtvId);
    if (pActivity)
    {
        return (unsigned int)pActivity->nEndTime;
    }
    return 0;
}



void CActivityComponent::CloseActivity(int nAtvId, bool nNextTime)
{
    GlobalActivityData* pActivity = GetActivity(nAtvId);
    if (pActivity)
    {
        pActivity->nEndTime = GetGlobalLogicEngine()->getMiniDateTime();
        pActivity->nNextTime = nNextTime;
        OutputMsg(rmTip,"[Global Activity] CloseActivity，活动Type=%d, 活动ID=%d !",pActivity->nActivityType, nAtvId);
    }
}


void CActivityComponent::UpdateActivityData(int nAtvId, CActor* pActor, int nParam1, int nParma2,int nParma3)
{

    if(!pActor) return;
    CScriptValueList paramList;
    // 全局活动
    if(IsRunning(nAtvId))
    {
        static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
        GlobalActivityData* pActivity = GetActivity(nAtvId);
        if (pActivity)
        {
            paramList.clear();
            paramList << (int)CActivitySystem::enUpdateActivityData;
            paramList << (int)pActivity->nActivityType;
            paramList << (int)pActivity->nId;
            paramList << pActor;
            paramList << nParam1;
            paramList << nParma2;
            paramList << nParma3;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnExitArea，活动Type=%d, 活动ID=%d !",pActivity->nActivityType,pActivity->nId);
            }
        }
    }
}


//拓展支持固定格式时间的个人活动
//使用GPStart和GPEnd时必须注册登陆事件OnUserLogin，
//并在登陆事件里调用CheckGPStartEnd 否则循环活动会有问题

//用于登陆时检测  全局&个人 活动
VOID CActivityComponent::CheckGPStartEnd(CActor* pActor,int atvId, unsigned int ScriptEndTime)
{

    if (!pActor) return;
    int SupportPAtvFlag = 0 ;
    std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
    if (globalActivityConfs.find(atvId)!=globalActivityConfs.end())
    {
        SupportPAtvFlag = globalActivityConfs[atvId].nSupportPAtv ;
    }
    if(SupportPAtvFlag ==0 ) return ;
    
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    GlobalActivityData* pActivity = GetActivity(atvId);

    unsigned int NowEndTime = GetEndTime(atvId) ;
    if(IsRunning(atvId)){
        if(ScriptEndTime == 0)
        {
            //call GPStart
            if (pActivity ) 
            {
                CScriptValueList paramList;
                paramList << CActivitySystem::enOnGPStart;
                paramList << (int)pActivity->nActivityType;
                paramList << (int)pActivity->nId;
                paramList << pActor;
                if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                {
                    //脚本错误，将以模态对话框的形式呈现给客户端
                    //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                    //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPStart，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                    OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPStart，活动Type=%d, 活动ID=%d !",pActivity->nActivityType,pActivity->nId);
                }
            }
        }
        else if( (ScriptEndTime != 0) && (ScriptEndTime != NowEndTime))
        {
            //call GPEnd
            if (pActivity) 
            {
                CScriptValueList paramList;
                paramList << CActivitySystem::enOnGPEnd;
                paramList << (int)pActivity->nActivityType;
                paramList << (int)pActivity->nId;
                paramList << pActor;
                if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                {
                    //脚本错误，将以模态对话框的形式呈现给客户端
                    //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                    //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPEnd，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                    OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPEnd，活动Type=%d, 活动ID=%d !",pActivity->nActivityType,pActivity->nId);
                }
            }

            //call GPStart
            if (pActivity) 
            {
                CScriptValueList paramList;
                paramList << CActivitySystem::enOnGPStart;
                paramList << (int)pActivity->nActivityType;
                paramList << (int)pActivity->nId;
                paramList << pActor;
                if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                {
                    //脚本错误，将以模态对话框的形式呈现给客户端
                    //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                    //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPStart，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                    OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPStart，活动Type=%d, 活动ID=%d !",pActivity->nActivityType,pActivity->nId);
                }
            }
        }
    }
    else
    { 
        //call GPEnd 
        pActivity = GetWaitingActivity(atvId);
        if (pActivity) 
        {
            CScriptValueList paramList;
            paramList << CActivitySystem::enOnGPEnd;
            paramList << (int)pActivity->nActivityType;
            paramList << (int)pActivity->nId;
            paramList << pActor;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                //脚本错误，将以模态对话框的形式呈现给客户端
                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                //OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPEnd，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnGPEnd，活动Type=%d, 活动ID=%d !",pActivity->nActivityType,pActivity->nId);
            }
        }

    }
    
}

//脚本里保存当前活动场次的结束时间用于判断是否是当前活动id的同一场次
VOID CActivityComponent::OnUserLogin(CActor* pActor) 
{
    //这里就是简单的触发
    if (!pActor) return;
    typedef std::map<int, GlobalActivityData>::iterator Iterator;
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();

    // 运行中的活动
    Iterator runIter = m_RunningActivity.begin();
    for (;runIter != m_RunningActivity.end();++runIter)
    {
        // 活动固定数据
        GlobalActivityData& activity = (*runIter).second;
        // 触发Lua脚本活动开始事件
        CScriptValueList paramList;
        paramList << CActivitySystem::enOnLoginGame;
        paramList << (int)activity.nActivityType;
        paramList << (int)activity.nId;
        paramList << pActor;
        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        {
            //脚本错误，将以模态对话框的形式呈现给客户端
            //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
            //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnLoginGame，活动ID=%d ! \n%s",activitydata.nId,(const char*)s);
            OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnLoginGame，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
        }
    
    }

    // 等待下次开启的活动,只对支持  全局&个人 活动的调用脚本
    std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
    runIter = m_WaitingActivity.begin();
    for (;runIter != m_WaitingActivity.end();++runIter)
    {
        // 活动固定数据
        GlobalActivityData& activity = (*runIter).second;

        int SupportPAtvFlag = 0 ;
        if (globalActivityConfs.find(activity.nId)!=globalActivityConfs.end())
        {
                SupportPAtvFlag = globalActivityConfs[activity.nId].nSupportPAtv ;
        }
        if(SupportPAtvFlag !=0 )
        {
            // 触发Lua脚本活动开始事件
            CScriptValueList paramList;
            paramList << CActivitySystem::enOnLoginGame;
            paramList << (int)activity.nActivityType;
            paramList << (int)activity.nId;
            paramList << pActor;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                //脚本错误，将以模态对话框的形式呈现给客户端
                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnLoginGame，活动ID=%d ! \n%s",activitydata.nId,(const char*)s);
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnLoginGame，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
            }

        } 
    } 
}

//
