#include "StdAfx.h"
#include "ActivitySystem.h"


CBufferAllocator* CActivitySystem::m_pAllocator;

PActivityAllocator::pointer
PActivityAllocator::allocate(size_type _n, const void*)
{
    if (CActivitySystem::m_pAllocator)
    {
        return (pointer)CActivitySystem::m_pAllocator->AllocBuffer(_n);
    }
    return NULL;
}

void
PActivityAllocator::deallocate(pointer _p, size_type)
{
    if (CActivitySystem::m_pAllocator)
    {
        CActivitySystem::m_pAllocator->FreeBuffer(_p);
    }
}

CActivitySystem::CActivitySystem()
{
    m_pAllocator = new CBufferAllocator();
    m_IsInited = false;
}

CActivitySystem::~CActivitySystem()
{
    delete m_pAllocator;
    m_pAllocator = NULL;
}

bool CActivitySystem::Initialize(void *,SIZE_T)
{
    m_RunningActivity.clear();
    //m_RunningSeq.clear();
    m_StopActivity.clear();
    m_Waitting.clear();
    m_IsInited = false;
    
    //请求加载活动数据
    CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected())
    {
        if (((CActor*)m_pEntity)->OnGetIsTestSimulator())
	    {

        }
        else
        {
            CDataPacket& dataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadPersonActivity);
            int nServerId = GetLogicServer()->GetServerIndex();
            dataPacket << ((CActor*)m_pEntity)->GetRawServerIndex() << nServerId;
            dataPacket << (int)(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
            GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
        } 
    }
    return true;
}

VOID CActivitySystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
    if (!m_pEntity) return;
    
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    CActor *pActor = (CActor*)m_pEntity;
    switch(nCmd)
	{
	    case jxInterSrvComm::DbServerProto::dcLoadPersonActivity:
        {
            if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
            {
                CScriptValueList paramList;
                int nCount = 0;
                reader >> nCount;

                CActivityProvider& activitycfg= GetLogicServer()->GetDataProvider()->GetActivityConfigs();

                // 加载进行中的活动
                PersonActivtyData activitydata;
                for(int i = 0; i < nCount; ++i)
                {
                    reader.readBuf(&activitydata,sizeof(activitydata));

                    if (!activitycfg.IsPersonActivity(activitydata.nId))
                    {
                       m_StopActivity[activitydata.nId] = activitydata.nActivityType;
                       continue;
                    }
                    
                    m_RunningActivity[activitydata.nId] = activitydata;
                    //m_RunningSeq.insert(&m_RunningActivity[activitydata.nId]);
                    
                    // 触发加载活动事件
                    paramList.clear();
                    paramList << (int)CActivitySystem::enOnLoad;
                    paramList << (int)activitydata.nActivityType;
                    paramList << (int)activitydata.nId;
                    paramList << m_pEntity;
                    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                    {
                        OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnLoad，活动Type=%d, 活动ID=%d !",activitydata.nActivityType,activitydata.nId);
                    }
                }

                // 加载结束的活动
                nCount = 0;
                reader >> nCount;
                for(int i = 0; i < nCount; ++i)
                {
                    // reader.readBuf(&activitydata,sizeof(activitydata));
                    int nId=0,nType=0;
                    reader >> nId >> nType;
                    // std::pair<int, int> _pair(nId, nType);
                    // m_StopActivity.insert(_pair);
                    m_StopActivity[nId] = nType;
                }
            }
            
            pActor->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_ACTIVITY_SYSTEM);

            m_IsInited = true;
            
        }
        break;
    }
}

VOID CActivitySystem::OnEnterGame()
{
    if (!m_pEntity) 
    {
        return;
    }

    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return;//假人不处理活动
    }
	DECLARE_TIME_PROF("CActivitySystem::OnEnterGame");
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    CActor *pActor = (CActor*)m_pEntity;
    // 发送活动开启数据
    CScriptValueList paramList;
 
    CActorPacket pack;
    CDataPacket & outPack = pActor->AllocPacket(pack);
    outPack << (BYTE)enActivityID << (BYTE)sSendPersonActivity;

    short nCount = 0;
    INT_PTR pos = outPack.getPosition();
    outPack << nCount;
    std::map<int, PersonActivtyData>::iterator runIter = m_RunningActivity.begin();
    for (;runIter != m_RunningActivity.end(); ++runIter)
    {
        // 记录每个活动数据长度
        INT_PTR len_offset = outPack.getPosition();
        outPack << (unsigned short)0;

        // 活动固定数据
        PersonActivtyData& activity = (*runIter).second;
        outPack << (int)activity.nId;
        outPack << (unsigned int)activity.nExpiredTime;

        // 触发获取活动数据
        paramList.clear();
        paramList << (int)enOnReqData;
        paramList << (int)activity.nActivityType;
        paramList << (int)activity.nId;
        paramList << m_pEntity;
        paramList << &pack;
        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        {
            OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnReqData，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
        }
    
        // 调整活动数据长度
        INT_PTR len_curoffset = outPack.getPosition();
        outPack.setPosition(len_offset);
        outPack << (short)(len_curoffset - len_offset);
        outPack.setPosition(len_curoffset);

        nCount++;
    }
    short* pCount = (short*)outPack.getPositionPtr(pos);
    *pCount = nCount;
    pack.flush();


    // 加载等待活动
    std::map<int,PERSONACTIVITY>& personActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetPersonActivities();
    typedef CActivityProvider::PersonActivityIterator PersonActivityIterator;
    PersonActivityIterator aIter = personActivityConfs.begin();
    for (; aIter != personActivityConfs.end(); ++aIter)
    {
        PERSONACTIVITY& activityConf = (*aIter).second;

        // 不曾开过的活动，丢进等待列表
        if (!IsActivityHasBeenOpened(activityConf.nId))
        {
            PersonActivtyData activity;
            activity.nId = activityConf.nId;			                // 活动ID
            activity.nActivityType = activityConf.nActivityType;	    // 活动类型
            activity.nTimeType = activityConf.nTimeType;		        // 时间类型
            activity.nStartTime = activityConf.nStartTime;          // 开始时间
            activity.nExpiredTime = activityConf.nEndTime;               // 结束时间
            m_Waitting[activityConf.nId] = activity;
        }
    }
}

VOID CActivitySystem::Save(PACTORDBDATA)
{
    if (!m_pEntity) return;
	DECLARE_TIME_PROF("CActivitySystem::Save");
    CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected())
    {
        CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSavePersonActivity);
        int nServerId = GetLogicServer()->GetServerIndex();
        dataPacket << ((CActor*)m_pEntity)->GetRawServerIndex() << nServerId;
	    dataPacket << (int)(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));

        // 运行中的活动
        int nCount = 0;
        INT_PTR pos = dataPacket.getPosition();
        dataPacket << nCount;
        std::map<int, PersonActivtyData>::iterator runIter = m_RunningActivity.begin();
        for (;runIter != m_RunningActivity.end(); ++runIter)
        {
            PersonActivtyData& activitydata = (*runIter).second;
            dataPacket.writeBuf(&activitydata,sizeof(activitydata));
            nCount++;
        }
        int* pCount = (int*)dataPacket.getPositionPtr(pos);
        *pCount = nCount;

        // 结束的活动
        nCount = 0;
        pos = dataPacket.getPosition();
        dataPacket << nCount;
        StopIterator stopIter = m_StopActivity.begin();
        for (;stopIter != m_StopActivity.end(); ++stopIter)
        {
            dataPacket << (int)(*stopIter).first;   //id
            dataPacket << (int)(*stopIter).second;  //type
            nCount++;
        }
        pCount = (int*)dataPacket.getPositionPtr(pos);
        *pCount = nCount;

        GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
    }
}

VOID CActivitySystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
    if (!m_pEntity) return;
	DECLARE_TIME_PROF("CActivitySystem::ProcessNetData");
    CActor* pActor = (CActor*)m_pEntity;
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    int nAtvId = 0, nAtvType = 0;
    packet >> nAtvId;
    std::map<int,PERSONACTIVITY>& personActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetPersonActivities();
    std::map<int,GLOBALACTIVITY>& globalActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetGlobalActivities();
    
    // 是否为全局活动
    bool isGlobal = false;

    // 获取配置
    if (personActivityConfs.find(nAtvId) != personActivityConfs.end())
    {
        PERSONACTIVITY& activityConf = personActivityConfs[nAtvId];
        nAtvType = activityConf.nActivityType;
    }
    else if(globalActivityConfs.find(nAtvId) != globalActivityConfs.end())
    {
        isGlobal = true;
        GLOBALACTIVITY& activityConf = globalActivityConfs[nAtvId];
        nAtvType = activityConf.nActivityType;
    }
    else
    {
        // 没有开始的活动，却请求数据，TODO 加日志或提示
        return;
    }

    if (!IsActivityRunning(nAtvId))
    {
        if (!GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
        {
            pActor->SendTipmsg("活动尚未开始！",tstUI);
            return;
        }
    }
    
    switch (nCmd)
    {
    // 通用操作请求
    case cActivityCommonOperator:
    {
        // 脚本回调
        if (nAtvType != 0)
        {
            CScriptValueList paramList;
            paramList << (int)enOnOperator;
            paramList << (int)nAtvType;
            paramList << (int)nAtvId;
            paramList << m_pEntity;
            paramList << &packet;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                //脚本错误，将以模态对话框的形式呈现给客户端
                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnOperator，活动ID=%d ! \n%s",nAtvId,(const char*)s);
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnOperator，活动Type=%d, 活动ID=%d !",nAtvType,nAtvId);
            }
            GetLogicServer()->SendActivityLocalLog(nAtvId, (CActor*)m_pEntity, nAtvType, 1);
        }
        else
        {
            // 这里是找不到配置， TODO 加提示
        }
    }
    break;
    // 请求获取某个活动的数据
    case cReqActivityData:
    {
        if (isGlobal) GetGlobalLogicEngine()->GetActivityMgr().SendOneActivity(pActor, nAtvId);
        else SendOneActivity(nAtvId);
    }
    break;
    
    default:
        break;
    }
}

VOID CActivitySystem::RunOne_5s(TICKCOUNT nCurrentTime)
{
	if (m_pEntity == NULL)
    { 
        return;
    }
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return;//假人不处理活动
    }
	UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
    CActor *pActor = (CActor*)m_pEntity;

    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();

    // 运行中的活动
    //RunIterator runIter = m_RunningSeq.begin();
    //for (;runIter != m_RunningSeq.end();)
    std::map<int, PersonActivtyData>::iterator runIter = m_RunningActivity.begin();
    for (;runIter != m_RunningActivity.end();)
    {
        PersonActivtyData& activity = (*runIter).second ;

        // 要结束的活动
        if (activity.nExpiredTime != 0 && GetGlobalLogicEngine()->getMiniDateTime() > activity.nExpiredTime)
        {
            // 结束活动，触发Lua脚本结束事件
            CScriptValueList paramList;
            paramList << (int)enOnEnd;
            paramList << (int)activity.nActivityType;
            paramList << (int)activity.nId;
            paramList << m_pEntity;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                //脚本错误，将以模态对话框的形式呈现给客户端
                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnd，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnd，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
            }
            
            // 推送活动结束
            CActorPacket pack;
            CDataPacket & outPack = pActor->AllocPacket(pack);
            outPack << (BYTE)enActivityID << (BYTE)sSendActivityEnd;
            outPack << (int)activity.nId;
            outPack << (unsigned int)activity.nExpiredTime;
            pack.flush();

            // 放入停止活动列表
            if (m_StopActivity.find(activity.nId) != m_StopActivity.end())
            {
                OutputMsg(rmError,"[Personal Activity] 结束的活动，却在结束列表中，活动ID=%d !",(int)activity.nId);
            }
            m_StopActivity[activity.nId] = activity.nActivityType;
            
            // 删除
            //m_RunningActivity.erase(m_RunningActivity.find(activity.nId));
            //m_RunningSeq.erase(runIter++);
            m_RunningActivity.erase(runIter++);

            continue;
        }
        // 还在进行中的（包括无限期的）activity.nExpiredTime==0
        else
        {
            // 触发Lua脚本活动帧更新事件
            CScriptValueList paramList;
            paramList << (int)enOnUpdate;
            paramList << (int)activity.nActivityType;
            paramList << (int)activity.nId;
            paramList << (unsigned int)nNow;
            paramList << m_pEntity;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                //脚本错误，将以模态对话框的形式呈现给客户端
                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnUpdate，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnUpdate，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
            }
        }
        
        ++runIter;
    }
}


VOID CActivitySystem::OnLevelUp()
{
    CheckActivityBegin();
    GetGlobalLogicEngine()->GetActivityMgr().OnEnterScene(((CActor*)m_pEntity));
    // ((CActor*)m_pEntity)->GetAchieveSystem().ActorAchievementComplete(nAchieveLevel, ((CActor*)m_pEntity)->GetProperty<int>(PROP_CREATURE_LEVEL)); //重新调用一下
}

VOID CActivitySystem::OnCircleUp()
{
    CheckActivityBegin();
}
VOID CActivitySystem::OnChargeYuanBao()
{
    FlushRunningAtvData();
}

VOID CActivitySystem::OnNewDayArrive()
{
    CheckActivityBegin();
}

VOID CActivitySystem::OnEnterScene()
{
    if (!m_pEntity) return;
    CheckActivityBegin();
    ((CActor*)m_pEntity)->GetAchieveSystem().ActorAchievementComplete(nAchieveLevel, ((CActor*)m_pEntity)->GetProperty<int>(PROP_CREATURE_LEVEL)); //重新调用一下
}


VOID CActivitySystem::OnUserLogin()
{
    if (!m_pEntity) return;
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    CActor *pActor = (CActor*)m_pEntity;

    // 运行中的活动
    std::map<int, PersonActivtyData>::iterator runIter = m_RunningActivity.begin();
    for (;runIter != m_RunningActivity.end(); ++runIter)
    {
        // 活动固定数据
        PersonActivtyData& activity = (*runIter).second;
        // 触发Lua脚本活动开始事件
        CScriptValueList paramList;
        paramList << (int)enOnLoginGame;
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

VOID CActivitySystem::OnCombineServer(int diffDay)
{
    if (!m_pEntity) return;
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    CActor *pActor = (CActor*)m_pEntity;

    // 运行中的活动
    std::map<int, PersonActivtyData>::iterator runIter = m_RunningActivity.begin();
    for (;runIter != m_RunningActivity.end(); ++runIter)
    {
        // 活动固定数据
        PersonActivtyData& activity = (*runIter).second;
        // 触发Lua脚本活动开始事件
        CScriptValueList paramList;
        paramList << (int)enOnCombineSrv;
        paramList << (int)activity.nActivityType;
        paramList << (int)activity.nId;
        paramList << (int)diffDay;
        paramList << pActor;
        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        {
            //脚本错误，将以模态对话框的形式呈现给客户端
            //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
            //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnCombineSrv，活动ID=%d ! \n%s",activitydata.nId,(const char*)s);
            OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnCombineSrv，活动Type=%d, 活动ID=%d !",activity.nActivityType,activity.nId);
        }
    
    }

}





void  CActivitySystem::FlushRunningAtvData()
{
    if(!m_pEntity) return;
    // 运行中的活动
    std::map<int, PersonActivtyData>::iterator runIter = m_RunningActivity.begin();
    for (;runIter != m_RunningActivity.end(); ++runIter)
    {
        // 活动固定数据
        PersonActivtyData& activity = (*runIter).second;
        SendOneActivity(activity.nId) ;
    }

}

bool CActivitySystem::CheckActivityBegin()
{
    if (!m_pEntity) 
	{
        return false;
	}
     
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return false; 
    }
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    CActor *pActor = (CActor*)m_pEntity;
    int nLevle = m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
    int nCircle = m_pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
    int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
    int nCombineServerDay = GetLogicServer()->GetDaysSinceCombineServer() ;
    std::map<int,PERSONACTIVITY>& pActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetPersonActivities();
    typedef std::map<int, PersonActivtyData>::iterator Iter;
    
    Iter iter = m_Waitting.begin();
    for (; iter != m_Waitting.end();)
    {
        PersonActivtyData pActivity = (*iter).second;
    
        if (IsActivityHasBeenOpened(pActivity.nId)) 
        {
            if (m_StopActivity.find(pActivity.nId)!=m_StopActivity.end()){
                // 从等待列表中删除
                m_Waitting.erase(iter++);
                //++iter; 
                continue;
            }else {
                ++iter; 
                continue;
            }
            
        }
        int nExpiredTime = 0;
        if(pActivityConfs.find(pActivity.nId) == pActivityConfs.end()) {
            ++iter; 
            continue;
        }

        PERSONACTIVITY* pActivityConf = &pActivityConfs[pActivity.nId];
        if (pActivityConf->nLevel > nLevle) { ++iter; continue;}
        if (pActivityConf->nZSLevel > nCircle) { ++iter; continue;}

        //nTimeType ==3 为合服活动时间类型，其他开服时间为基准
        if(pActivityConf->nTimeType == 3){
            if (pActivityConf->nOpenSrvDate > nCombineServerDay) { ++iter; continue;}
        }else {
            if (pActivityConf->nOpenSrvDate > nOpenServerDay) { ++iter; continue;}
        }

        // 放入进行中的活动
        if (pActivityConf->nDuration > 0)
        {
            nExpiredTime= CMiniDateTime::today();//此时活动结束，赋一个较小值让活动自然结束

            if (pActivityConf->nTimeType == 4 )  //创角时间，直接当前时间加持续时间
                nExpiredTime = GetGlobalLogicEngine()->getMiniDateTime() + 86400 * pActivityConf->nDuration;
            else if (pActivityConf->nTimeType == 5 ) { 
                //判断时间结束不触发
                if(GetLogicServer()->GetDaysSinceOpenServer() > (pActivityConf->nOpenSrvDate + pActivityConf->nDuration) ) { ++iter; continue;}
                else {
                    int nDayNeedToAdd = (pActivityConf->nOpenSrvDate + pActivityConf->nDuration - GetLogicServer()->GetDaysSinceOpenServer()) ;
                    if (nDayNeedToAdd >0) nExpiredTime = CMiniDateTime::today() + 86400 * nDayNeedToAdd;    
                }
            }
            else if (pActivityConf->nTimeType == 6) {
                //判断时间结束不触发
                if(GetLogicServer()->GetDaysSinceCombineServer() > (pActivityConf->nOpenSrvDate + pActivityConf->nDuration) ) { ++iter; continue;}
                else {
                    int nDayNeedToAdd = (pActivityConf->nOpenSrvDate + pActivityConf->nDuration - GetLogicServer()->GetDaysSinceCombineServer()) ;
                    if (nDayNeedToAdd >0) nExpiredTime = CMiniDateTime::today() + 86400 * nDayNeedToAdd;    
                }
            }
            else {
                if((GetGlobalLogicEngine()->getMiniDateTime() < pActivity.nStartTime) || (GetGlobalLogicEngine()->getMiniDateTime() > pActivity.nExpiredTime) ) {
                    ++iter; continue;
                }

                nExpiredTime = pActivity.nExpiredTime;
            }
        }
        
        //m_RunningSeq.insert(&m_RunningActivity[activitydata.nId]);
        PersonActivtyData& activitydata = m_RunningActivity[pActivityConf->nId];
        activitydata.nId = pActivityConf->nId;
        activitydata.nActivityType = pActivityConf->nActivityType;
        activitydata.nExpiredTime = nExpiredTime;
        // 触发Lua脚本活动开始事件
        CScriptValueList paramList;
        paramList << (int)enOnStart;
        paramList << (int)activitydata.nActivityType;
        paramList << (int)activitydata.nId;
        paramList << m_pEntity;
        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        {
            //脚本错误，将以模态对话框的形式呈现给客户端
            //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
            //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnStart，活动ID=%d ! \n%s",activitydata.nId,(const char*)s);
            OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnStart，活动Type=%d, 活动ID=%d !",activitydata.nActivityType,activitydata.nId);
        }

        //如果在脚本star里触发了关闭，continue
        if (m_StopActivity.find(activitydata.nId) != m_StopActivity.end())
        {
            ++iter; 
            continue;

        }
        // 推送活动开启
        {
            CActorPacket pack;
            CDataPacket & outPack = pActor->AllocPacket(pack);
            outPack << (BYTE)enActivityID << (BYTE)sSendPersonActivityUpdate;
            outPack << (int)activitydata.nId;
            outPack << (unsigned int)activitydata.nExpiredTime;

            // 触发获取活动数据
            paramList.clear();
            paramList << (int)enOnReqData;
            paramList << (int)activitydata.nActivityType;
            paramList << (int)activitydata.nId;
            paramList << m_pEntity;
            paramList << &pack;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                //脚本错误，将以模态对话框的形式呈现给客户端
                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnReqData，活动ID=%d ! \n%s",activitydata.nId,(const char*)s);
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnReqData，活动Type=%d, 活动ID=%d !",activitydata.nActivityType,activitydata.nId);
            }
            pack.flush();
        }
        
        // 活动弹框
        if (pActivityConf->bPopupWhenStart)
        {
            CActorPacket pack;
            CDataPacket & outPack = pActor->AllocPacket(pack);
            outPack << (BYTE)enActivityID << (BYTE)SActivityPopup;
            outPack << (int)activitydata.nId;
            pack.flush();
        }


        //触发活动登录事件
        paramList.clear();
        paramList << (int)enOnLoginGame;
        paramList << (int)activitydata.nActivityType;
        paramList << (int)activitydata.nId;
        paramList << pActor;
        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        {
            //脚本错误，将以模态对话框的形式呈现给客户端
            //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
            //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnLoginGame，活动ID=%d ! \n%s",activitydata.nId,(const char*)s);
            OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnLoginGame，活动Type=%d, 活动ID=%d !",activitydata.nActivityType,activitydata.nId);
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
                    if (pAreaAttr->pValues[i] == activitydata.nId)
                    {
                        paramList.clear();
                        paramList << (int)CActivitySystem::enOnEnterArea;
                        paramList << activitydata.nActivityType;
                        paramList << activitydata.nId;
                        paramList << pActor;
                        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                        {
                            OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnExitArea，活动Type=%d, 活动ID=%d !",activitydata.nActivityType,activitydata.nId);
                        }
                        pActor->CrossActivityArea();
                        break;
                    }
                }
            }
        }
        // 从等待列表中删除
        m_Waitting.erase(iter++);
    }
    
}

bool CActivitySystem::IsTypeRunning(int nAtvType)
{
    std::vector<int>& AtvIdList = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetActivityIdList(nAtvType);
    for (size_t i = 0; i < AtvIdList.size(); i++)
    {
        if(IsActivityRunning(AtvIdList[i])) return true;
    }
    return false;
}

void CActivitySystem::SendOneActivity(int nAtvId)
{
    if (!m_pEntity) return;
    CActor *pActor = (CActor*)m_pEntity;

    PersonActivtyData* pActivity = GetActivity(nAtvId);
    if(!pActivity) {
        if (m_StopActivity.find(nAtvId) != m_StopActivity.end()){
            // 强推活动结束
            CActorPacket pack;
            CDataPacket & outPack = ((CActor*)m_pEntity)->AllocPacket(pack);
            outPack << (BYTE)enActivityID << (BYTE)sSendActivityEnd;
            outPack << (int)nAtvId;
            outPack << (unsigned int)1111;
            pack.flush();

        }
        return;
    }  
    
    if(m_StopActivity.find(nAtvId) != m_StopActivity.end()) return ;

    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
    CActorPacket pack;
    CDataPacket & outPack = pActor->AllocPacket(pack);
    outPack << (BYTE)enActivityID << (BYTE)sSendPersonActivityUpdate;
    outPack << (int)nAtvId;

    unsigned int nExpiredTime = 0;
    if (m_RunningActivity.find(nAtvId) != m_RunningActivity.end())
    {
        nExpiredTime = m_RunningActivity[nAtvId].nExpiredTime;
    }
    else if (m_StopActivity.find(nAtvId) != m_StopActivity.end())
    {
        nExpiredTime = (unsigned int)-1;
    }
    outPack << nExpiredTime;

    CScriptValueList paramList;
    paramList << (int)enOnReqData;
    paramList << (int)pActivity->nActivityType;
    paramList << (int)nAtvId;
    paramList << m_pEntity;
    paramList << &pack;
    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
    {
        OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnReqData，活动Type=%d, 活动ID=%d !",pActivity->nActivityType,nAtvId);
    }

    pack.flush();
}


void CActivitySystem::UpdateActivityData(int nAtvId, int nParam1, int nParma2,int nParma3)
{
    if (!m_pEntity)
    {
        return;
    }
    
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator() )
	{
		return;
	}
    CScriptValueList paramList;
    //全局活动
    GetGlobalLogicEngine()->GetActivityMgr().UpdateActivityData( nAtvId, ((CActor*)m_pEntity),nParam1,nParma2, nParma3);
    // 个人活动
    if(IsActivityRunning(nAtvId))
    {
        static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
        if (PersonActivtyData* pActivty = GetActivity(nAtvId))
        {
            paramList.clear();
            paramList << (int)enUpdateActivityData;
            paramList << (int)pActivty->nActivityType;
            paramList << (int)pActivty->nId;
            paramList << m_pEntity;
            paramList << nParam1;
            paramList << nParma2;
            paramList << nParma3;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnExitArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
            }
        }
    }
}

//手动关闭活动
void CActivitySystem::CloseActivity(int nAtvId)
{
    if (!m_pEntity) return;
    PersonActivtyData* pActivty = GetActivity(nAtvId) ;
    if (pActivty) {
        static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
        CScriptValueList paramList;
        paramList << (int)enOnEnd;
        paramList << (int)pActivty->nActivityType;
        paramList << (int)pActivty->nId;
        paramList << m_pEntity;
        if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        {
            //脚本错误，将以模态对话框的形式呈现给客户端
            //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
            //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnd，活动ID=%d ! \n%s",activity.nId,(const char*)s);
            OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnd，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
        }
        // 放入停止活动列表
        if (m_StopActivity.find(pActivty->nId) != m_StopActivity.end())
        {
            OutputMsg(rmError,"[Personal Activity] 运行中的循环活动，却在等待列表中，活动ID=%d !",(int)pActivty->nId);
        }
        m_StopActivity[pActivty->nId] = pActivty->nActivityType;
        
        // 删除
        m_RunningActivity.erase(m_RunningActivity.find(pActivty->nId));
    }


    if (m_StopActivity.find(nAtvId) == m_StopActivity.end())
    {
        std::map<int,PERSONACTIVITY>& personActivityConfs = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetPersonActivities();
        typedef CActivityProvider::PersonActivityIterator PersonActivityIterator;
        PersonActivityIterator aIter = personActivityConfs.find(nAtvId);
        if (aIter != personActivityConfs.end())
        {
            PERSONACTIVITY& activityConf = (*aIter).second;
            m_StopActivity[activityConf.nId] = activityConf.nActivityType ;

            static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
            CScriptValueList paramList;
            paramList << (int)enOnEnd;
            paramList << (int)activityConf.nActivityType;
            paramList << (int)activityConf.nId;
            paramList << m_pEntity;
            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
            {
                //脚本错误，将以模态对话框的形式呈现给客户端
                //const RefString& s = globalNpc->GetScript().getLastErrorDesc();
                //OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnd，活动ID=%d ! \n%s",activity.nId,(const char*)s);
                OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnd，活动Type=%d, 活动ID=%d !",activityConf.nActivityType,activityConf.nId);
            }
        }

    }

    // 强推活动结束
    CActorPacket pack;
    CDataPacket & outPack = ((CActor*)m_pEntity)->AllocPacket(pack);
    outPack << (BYTE)enActivityID << (BYTE)sSendActivityEnd;
    outPack << (int)nAtvId;
    outPack << (unsigned int)1111;
    pack.flush();
}
