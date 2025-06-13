#include "StdAfx.h"
#include "ActivityProvider.h"

CBufferAllocator* CActivityProvider::m_pAllocator;

CActivityProvider::CActivityProvider() :Inherited()
{
    m_pAllocator = new CBufferAllocator();
}

CActivityProvider::~CActivityProvider()
{
    delete m_pAllocator;
    m_pAllocator = NULL;
}

bool CActivityProvider::LoadAll(LPCTSTR sFilePath)
{
    bool Result = false;
    CMemoryStream ms;
    CCustomLuaPreProcessor pp;
    LPCTSTR sText;

    try
    {
        //从文件加载配置脚本
        if ( ms.loadFromFile(sFilePath) <= 0 )
            showErrorFormat(_T("unable to load from %s"), sFilePath);

        //对配置脚本进行预处理
        GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
        sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);

        //设置脚本内容
        if ( !setScript(sText) )
            showError(_T("syntax error on activity config"));

        Result = ReadAllActivities();
    }
    catch (RefString &s)
    {
        OutputMsg(rmError, _T("load activity config error: %s"), s.rawStr());
        FILE* fp = fopen("scripterror.txt", "wb");
        if (fp)
        {
            fputs(sText, fp);
            fclose(fp);
        }
    }
    catch (...)
    {
        OutputMsg(rmError, _T("unexpected error on load activity config"));
    }

    //销毁脚本虚拟机
    setScript(NULL);
    return Result;
}

bool CActivityProvider::ReadAllActivities()
{
    bool bDef_false = false;
    int nDefInt_1 = 1;
    int nDefInt_0 = 0;
    int nDefInt_f1 = -1;

    int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
    CMiniDateTime now_time = CMiniDateTime::now();
    // 读取全局活动
    if ( openGlobalTable("ActivitiesConf") )
    {
        //const INT_PTR nCount = lua_objlen(m_pLua, -1) + 1;
        m_GlobalActivities.clear();
        char buff[50];
        //if (nCount > 0)
        //{
            if (enumTableFirst())
            {
               do
               {
                    if (getFieldInt("isClosed", &nDefInt_0) == 1)
                        continue;
                    int nId = getFieldInt("Id");
                    if (m_GlobalActivities.find(nId) != m_GlobalActivities.end()) return false;
                    
                    GLOBALACTIVITY& activity = m_GlobalActivities[nId];
                    activity.nId = nId;
                    activity.nActivityType = getFieldInt("ActivityType");
                    activity.nTimeType = getFieldInt("TimeType");

	                activity.nAfterSrvDay = getFieldInt("AfterSrvDay",&nDefInt_0);	// 开服N天后开
                    activity.bPopupWhenStart = getFieldBoolean("Popup",&bDef_false);
                    activity.nOpenLevel = getFieldInt("openlevel",&nDefInt_0);
                    activity.nOpenCircle = getFieldInt("rebornlevel",&nDefInt_0);
                    activity.nSupportPAtv = getFieldInt("supportflag",&nDefInt_0);
                    

                    if (feildTableExists("TipsLevelLimit") && openFieldTable("TipsLevelLimit"))
                    {
                        activity.nPopLevel = getFieldInt("level",&nDefInt_0);
                        activity.nPopCircle = getFieldInt("zsLevel",&nDefInt_0);
		                closeTable();
                    }

                    // 开服前N天开，如果过了这个时间，永远不开，则删掉
	                activity.nBeforeSrvDay = getFieldInt("BeforeSrvDay",&nDefInt_0);
                    if (activity.nBeforeSrvDay && nOpenServerDay > activity.nBeforeSrvDay)
                    {
                        GlobalActivityIterator it = m_GlobalActivities.find(nId);
                        m_GlobalActivities.erase(it);
                        continue;
                    }

                    if (isExistString("RealTimeLt")) {
                        getFieldStringBuffer("RealTimeLt",buff,sizeof(buff));
                        ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nRealTimeLt);
                    }else activity.nRealTimeLt = 0;

                    // 小于这个时间的才开启
                    if (!CheckOpenTimeLT(activity))
                    {
                        GlobalActivityIterator it = m_GlobalActivities.find(nId);
                        m_GlobalActivities.erase(it);
                        continue;
                    }
                    
                    if (isExistString("RealTimeGt")) {
                        getFieldStringBuffer("RealTimeGt",buff,sizeof(buff));
                        ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nRealTimeGt);
                    }else activity.nRealTimeGt = 0;

                    if (isExistString("HefuTimeLt")) {
                        getFieldStringBuffer("HefuTimeLt",buff,sizeof(buff));
                        ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nHefuTimeLt);
                    }else activity.nHefuTimeLt = 0;

                    // 合服时间小于这个的才开启
                    if (!CheckHefuTimeLt(activity))
                    {
                        GlobalActivityIterator it = m_GlobalActivities.find(nId);
                        m_GlobalActivities.erase(it);
                        continue;
                    }
                    
                    if (isExistString("HefuTimeGt")) {
                        getFieldStringBuffer("HefuTimeGt",buff,sizeof(buff));
                        ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nHefuTimeGt);
                    }else activity.nHefuTimeGt = 0;

                    // 合服时间大于这个的才开启
                    if (!CheckHefuTimeGt(activity))
                    {
                        GlobalActivityIterator it = m_GlobalActivities.find(nId);
                        m_GlobalActivities.erase(it);
                        continue;
                    }
                    
                    // 合服次数达到的才开启
                    activity.nHefuTimes = getFieldInt("HefuTimes",&nDefInt_0);
                    if (!CheckHefuTimes(activity))
                    {
                        GlobalActivityIterator it = m_GlobalActivities.find(nId);
                        m_GlobalActivities.erase(it);
                        continue;
                    }

                    // 详细活动时间
                    if (feildTableExists("TimeDetail") && openFieldTable("TimeDetail"))
                    {
                        int nCount = (int)lua_objlen(m_pLua,-1);
                        activity.pTimeDetail = (OneTimePair*)m_pAllocator->AllocBuffer(sizeof(OneTimePair) * nCount);
                        for (size_t i = 1; i <= nCount; i++)
                        {
                            getFieldIndexTable(i);
                            getFieldStringBuffer("StartTime",activity.pTimeDetail[i-1].strStartTime,ATIVITY_TIME_LEN);
                            getFieldStringBuffer("EndTime",activity.pTimeDetail[i-1].strEndTime,ATIVITY_TIME_LEN);
                            closeTable();
                        }
                        activity.nTimeCount = nCount;
                        activity.nTimeIdx = 0;
		                closeTable();	
                    }

                    // 计算时间（若算不到时间，说明不开启了）
                    if (!UpdateOpenTime(m_pLua, activity))
                    {
                        GlobalActivityIterator it = m_GlobalActivities.find(nId);
                        m_GlobalActivities.erase(it);
                        continue;
                    }

                    // 活动时间类型为非循环时间的活动，需要结束时间大于当前服务器时间，才说明可以开启
                    if (activity.nTimeType != 3)
                    {
                        if ((unsigned int)activity.nEndTime != 0 && (unsigned int)activity.nEndTime <= now_time)
                        {
                            if (activity.pTimeDetail)
                            {
                                m_pAllocator->FreeBuffer(activity.pTimeDetail);
                                activity.pTimeDetail = NULL;
                            }

                            GlobalActivityIterator it = m_GlobalActivities.find(nId);
                            m_GlobalActivities.erase(it);
                            continue;
                        }
                    }

                    // 加入映射表
                    std::vector<int>& vecMap = m_ActType2IdMap[activity.nActivityType];
                    vecMap.push_back(activity.nId);
               } while (enumTableNext());
            }
        //}
	    closeTable();//ActivitiesConf
    }
    else return false;

    // 读取个人活动
    if ( openGlobalTable("PActivitiesConf") )
    {
        //const INT_PTR nCount = lua_objlen(m_pLua, -1) + 1;
        m_PersonActivities.clear();
        char buff[50];
        //if (nCount > 0)
        //{
            if (enumTableFirst())
            {
               do
               {
                    if (getFieldInt("isClosed", &nDefInt_0) == 1)
                        continue;
                    int nId = getFieldInt("Id");
                    if (m_PersonActivities.find(nId) != m_PersonActivities.end()) return false;
                    if (getFieldInt("IsActivity",&nDefInt_1) != 0)
                    {
                        PERSONACTIVITY& activity = m_PersonActivities[nId];
                        activity.nId = nId;
                        activity.nActivityType = getFieldInt("ActivityType");
                        activity.nLevel = getFieldInt("Level",&nDefInt_1);
                        activity.nZSLevel = getFieldInt("ZSLevel",&nDefInt_0);
                        activity.nOpenSrvDate = getFieldInt("OpenSrvDate",&nDefInt_0);
                        activity.nDuration = getFieldInt("Duration",&nDefInt_f1);//0和-1都是无限期
                        activity.nTimeType = getFieldInt("TimeType",&nDefInt_1);
                        activity.bPopupWhenStart = getFieldBoolean("Popup",&bDef_false);
 
                        // 开服前N天开，如果过了这个时间，永远不开，则删掉
                        activity.nBeforeSrvDay = getFieldInt("BeforeSrvDay",&nDefInt_0);
                        if (activity.nBeforeSrvDay && nOpenServerDay > activity.nBeforeSrvDay)
                        {
                            auto it = m_PersonActivities.find(nId);
                            m_PersonActivities.erase(it);
                            continue;
                        }

                        if (isExistString("RealTimeLt")) {
                            getFieldStringBuffer("RealTimeLt",buff,sizeof(buff));
                            ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nRealTimeLt);
                        }else activity.nRealTimeLt = 0;

                        // 小于这个时间的才开启
                        if (!CheckOpenTimeLT(activity))
                        {
                            auto it = m_PersonActivities.find(nId);
                            m_PersonActivities.erase(it);
                            continue;
                        }
                        
                        if (isExistString("RealTimeGt")) {
                            getFieldStringBuffer("RealTimeGt",buff,sizeof(buff));
                            ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nRealTimeGt);
                        }else activity.nRealTimeGt = 0;

                        if (isExistString("HefuTimeLt")) {
                            getFieldStringBuffer("HefuTimeLt",buff,sizeof(buff));
                            ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nHefuTimeLt);
                        }else activity.nHefuTimeLt = 0;

                        // 合服时间小于这个的才开启
                        if (!CheckHefuTimeLt(activity))
                        {
                            auto it = m_PersonActivities.find(nId);
                            m_PersonActivities.erase(it);
                            continue;
                        }
                        
                        if (isExistString("HefuTimeGt")) {
                            getFieldStringBuffer("HefuTimeGt",buff,sizeof(buff));
                            ReadTime(m_pLua, 1, buff, sizeof(buff), activity.nHefuTimeGt);
                        }else activity.nHefuTimeGt = 0;

                        // 合服时间大于这个的才开启
                        if (!CheckHefuTimeGt(activity))
                        {
                            auto it = m_PersonActivities.find(nId);
                            m_PersonActivities.erase(it);
                            continue;
                        }
                        
                        // 合服次数达到的才开启
                        activity.nHefuTimes = getFieldInt("HefuTimes",&nDefInt_0);
                        if (!CheckHefuTimes(activity))
                        {
                            auto it = m_PersonActivities.find(nId);
                            m_PersonActivities.erase(it);
                            continue;
                        }

                        // 详细活动时间
                        if (feildTableExists("TimeDetail") && openFieldTable("TimeDetail"))
                        {
                            int nCount = (int)lua_objlen(m_pLua,-1);
                            activity.pTimeDetail = (OneTimePair*)m_pAllocator->AllocBuffer(sizeof(OneTimePair) * nCount);
                            for (size_t i = 1; i <= nCount; i++)
                            {
                                getFieldIndexTable(i);
                                getFieldStringBuffer("StartTime",activity.pTimeDetail[i-1].strStartTime,ATIVITY_TIME_LEN);
                                getFieldStringBuffer("EndTime",activity.pTimeDetail[i-1].strEndTime,ATIVITY_TIME_LEN);
                                closeTable();
                            }
                            activity.nTimeCount = nCount;
                            activity.nTimeIdx = 0;
                            closeTable();	
                            // 计算时间（若算不到时间，说明不开启了）
                            {
                                if (!UpdateOpenTime(m_pLua, activity)) {
                                    auto it = m_PersonActivities.find(nId);
                                    m_PersonActivities.erase(it);
                                    continue;
                                }
                            }
                        }

                        // 活动时间类型为非循环时间的活动，需要结束时间大于当前服务器时间，才说明可以开启
                        /*
                        4:创角时间
                        5:开服时间
                        */
                        if (activity.nTimeType != 4 && activity.nTimeType != 5 && activity.nTimeType != 3 )
                        {
                            if ((unsigned int)activity.nEndTime != 0 && (unsigned int)activity.nEndTime <= now_time)
                            {
                                if (activity.pTimeDetail)
                                {
                                    m_pAllocator->FreeBuffer(activity.pTimeDetail);
                                    activity.pTimeDetail = NULL;
                                }

                                auto it = m_PersonActivities.find(nId);
                                m_PersonActivities.erase(it);
                                continue;
                            }
                        }
                        // 加入映射表
                        std::vector<int>& vecMap = m_ActType2IdMap[activity.nActivityType];
                        vecMap.push_back(activity.nId);
                    }
               } while (enumTableNext());
            }
        //}
	    closeTable();//PActivitiesConf
    }
    else return false;
    return true;
}

bool CActivityProvider::ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out)
{
    switch (nTimeType)
    {
    case 0: //day-hour:minute
    case 2:
    case 3:
    {
        static int nD,nH,nM = 3;

        int nTop = lua_gettop(pLuaState);
        lua_getglobal(pLuaState, "string");
		lua_getfield(pLuaState, -1, "match");
        lua_pushlstring(pLuaState, sKey, strnlen(sKey, nKeyLen));
        lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
        int nErr = lua_pcall(pLuaState, 2, 3, 0);
        if (!nErr)
        {
            nD = lua_tonumber(pLuaState, -3);
            nH = lua_tonumber(pLuaState, -2);
            nM = lua_tonumber(pLuaState, -1);
        }else return false;
        lua_pop(pLuaState, 3);
        lua_settop(pLuaState, nTop);
        out = nD*24*3600 + nH*3600 + nM*60;
    }
    break;
    case 1: // year.month.day-hour:minute
    {
        SYSTEMTIME TimeResult;
        memset(&TimeResult,0,sizeof(TimeResult));

        int nTop = lua_gettop(pLuaState);
        lua_getglobal(pLuaState, "string");
		lua_getfield(pLuaState, -1, "match");
        lua_pushlstring(pLuaState, sKey, strnlen(sKey, nKeyLen));
        lua_pushlstring(pLuaState, "(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)", strnlen("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)",sizeof("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)")));
        int nErr = lua_pcall(pLuaState, 2, 5, 0);
        if (!nErr)
        {
            TimeResult.wYear = lua_tonumber(pLuaState, -5);
            TimeResult.wMonth = lua_tonumber(pLuaState, -4);
            TimeResult.wDay = lua_tonumber(pLuaState, -3);
            TimeResult.wHour = lua_tonumber(pLuaState, -2);
            TimeResult.wMinute = lua_tonumber(pLuaState, -1);
        }else return false;
        lua_pop(pLuaState, 5);
        lua_settop(pLuaState, nTop);
		out.encode(TimeResult);
    }
    break;
    }
}

bool CActivityProvider::ReadFixedTimeValue(lua_State * pLuaState, LPCTSTR sKey, int nKeyLen, SYSTEMTIME& out)
{
    memset(&out,0,sizeof(out));

    int nTop = lua_gettop(pLuaState);
    lua_getglobal(pLuaState, "string");
    lua_getfield(pLuaState, -1, "match");
    lua_pushlstring(pLuaState, sKey, strnlen(sKey, nKeyLen));
    lua_pushlstring(pLuaState, "(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)", strnlen("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)",sizeof("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)")));
    int nErr = lua_pcall(pLuaState, 2, 5, 0);
    if (!nErr)
    {
        out.wYear = lua_tonumber(pLuaState, -5);
        out.wMonth = lua_tonumber(pLuaState, -4);
        out.wDay = lua_tonumber(pLuaState, -3);
        out.wHour = lua_tonumber(pLuaState, -2);
        out.wMinute = lua_tonumber(pLuaState, -1);
    }else return false;
    lua_pop(pLuaState, 5);
    lua_settop(pLuaState, nTop);
}

bool CActivityProvider::UpdateOpenTime(lua_State * pLuaState, GLOBALACTIVITY& activity, bool nNextTime)
{
    //nNextTime 直接开启下一轮
    bool result = false;
    CMiniDateTime now_time = CMiniDateTime::now();

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("活动[%d] 当前时间:(%d-%d-%d %d:%d:%d) "),activity.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    bool isAfterSrvDay = true;
    
    // 开服N天后才开，第N天仍不开，则计算时间时需要偏移
    if (activity.nAfterSrvDay > 0)
    {
        int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
        int nDiff = activity.nAfterSrvDay - nOpenServerDay + 1;
        if (nDiff > 0)
        {
            now_time = CMiniDateTime::today();//定位到活动当天的0点计算
            now_time += nDiff*3600*24;
            isAfterSrvDay = false;
        }
        
#ifdef _DEBUG
        OutputMsg(rmTip, _T("活动[%d] Diff=%d"), activity.nId, nDiff);
#endif
    }

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("活动[%d] 当前调整时间:(%d-%d-%d %d:%d:%d) "),activity.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    // 开服前N天开，包括第N天，超过这个时间将不再开启
    if (activity.nBeforeSrvDay > 0 &&
        GetLogicServer()->GetDaysSinceOpenServer() > activity.nBeforeSrvDay)
    {
        return false;
    }

    switch (activity.nTimeType)
    {
    case eActivityTimeType_KFSJ: // 开服时间
    {
        for (; activity.nTimeIdx < activity.nTimeCount; activity.nTimeIdx++)
        {
            short curidx = activity.nTimeIdx;

            // 获取结束时间
            activity.nEndTime.tv = 0;
            if(strcmp(activity.pTimeDetail[curidx].strEndTime, "-1"))
            {
                ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, activity.nEndTime);
                activity.nEndTime = GetLogicServer()->GetServerOpenTime().rel_today() + activity.nEndTime;
            }
            // 已经结束的，换下一个点 --默认-1 为永久
            if (activity.nEndTime != 0 && activity.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, activity.nStartTime);
            activity.nStartTime = GetLogicServer()->GetServerOpenTime().rel_today() + activity.nStartTime;
            if(nNextTime && activity.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_GDSJ: // 固定时间
    {
        for (; activity.nTimeIdx < activity.nTimeCount; activity.nTimeIdx++)
        {
            short curidx = activity.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, activity.nEndTime);

            // 已经结束的，换下一个点
            if (activity.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, activity.nStartTime);

            // 开服时间检测
            //if (activity.nAfterSrvDay > 0 && (!isAfterSrvDay) && activity.nStartTime >= now_time) continue;
            if (activity.nAfterSrvDay > 0 && (!isAfterSrvDay) && activity.nStartTime < now_time)
            {
                SYSTEMTIME timeinfo;
                ReadFixedTimeValue(pLuaState, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, timeinfo);
                activity.nStartTime = now_time.rel_today(timeinfo.wHour, timeinfo.wMinute, 0);
            }

            // 切换下一个时间点
            if(nNextTime && activity.nStartTime < now_time) continue;

            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_HFSJ: // 合服时间
    {
        for (; activity.nTimeIdx < activity.nTimeCount; activity.nTimeIdx++)
        {
            short curidx = activity.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, activity.nEndTime);
            activity.nEndTime = GetLogicServer()->GetServerCombineTime() + activity.nEndTime;

            // 已经结束的，换下一个点
            if (activity.nEndTime <= now_time || nNextTime) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, activity.nStartTime);
            activity.nStartTime = GetLogicServer()->GetServerCombineTime() + activity.nStartTime;
            if(nNextTime && activity.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_XHSJ: // 循环时间
    {
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
        if (nCurWeek == 0)
        {
            nCurWeek = 7;
        }
        

        static int nStartWeek,nStartHour,nStartMinute;
        static int nEndWeek,nEndHour,nEndMinute;
        static CMiniDateTime openTime,closeTime;

        for (; activity.nTimeIdx < activity.nTimeCount; activity.nTimeIdx++)
        {
            short curidx = activity.nTimeIdx;

            // 获取结束时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, activity.pTimeDetail[curidx].strEndTime, strnlen(activity.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            int nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nEndWeek = lua_tonumber(pLuaState, -3);
                nEndHour = lua_tonumber(pLuaState, -2);
                nEndMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            closeTime.encode(nCurYear, nCurMon, nCurDay + (nEndWeek > 0?nEndWeek - nCurWeek:0), nEndHour, nEndMinute, 0);

//#ifdef _DEBUG
            {
                SYSTEMTIME nowTime;
                memset(&nowTime,0,sizeof(nowTime));
                closeTime.decode(nowTime);
                OutputMsg(rmTip, _T("活动[%d] (%d)结束时间:(%d-%d-%d %d:%d:%d) "),activity.nId,curidx,
                    nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
            }
//#endif
            
            // 已经结束的，换下一个点
            if (closeTime <= now_time) continue;
            
            // 获取开始时间
            nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, activity.pTimeDetail[curidx].strStartTime, strnlen(activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nStartWeek = lua_tonumber(pLuaState, -3);
                nStartHour = lua_tonumber(pLuaState, -2);
                nStartMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            openTime.encode(nCurYear, nCurMon, nCurDay + (nStartWeek > 0?nStartWeek - nCurWeek:0), nStartHour, nStartMinute, 0);

            if(nNextTime && openTime < now_time) continue;
            // 记录时间戳
            activity.nStartTime = openTime;
            activity.nEndTime = closeTime;
            result = true;
            break;
        }

        if (!result)
        {
            // 如果都不在时间内，那就是下一周了的第一个时间了
            activity.nTimeIdx = 0;

            // 获取开始时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, activity.pTimeDetail[0].strStartTime, strnlen(activity.pTimeDetail[0].strStartTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            int nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nStartWeek = lua_tonumber(pLuaState, -3);
                nStartHour = lua_tonumber(pLuaState, -2);
                nStartMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            openTime.encode(nCurYear, nCurMon, nCurDay + (nStartWeek > 0?nStartWeek - nCurWeek:0), nStartHour, nStartMinute, 0);

            // 获取结束时间
            nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, activity.pTimeDetail[0].strEndTime, strnlen(activity.pTimeDetail[0].strEndTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nEndWeek = lua_tonumber(pLuaState, -3);
                nEndHour = lua_tonumber(pLuaState, -2);
                nEndMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            closeTime.encode(nCurYear, nCurMon, nCurDay + (nEndWeek > 0?nEndWeek - nCurWeek:0), nEndHour, nEndMinute, 0);

            int loop = (0 == nStartWeek) || (0 == nEndWeek) ? 1 : 7;

            // 计算矫正
            if (closeTime < openTime)
                openTime = openTime - loop * (24*3600);

            // 计算矫正后起始/结束时间
            activity.nStartTime = openTime;
            activity.nEndTime = closeTime;
            while (now_time > (unsigned int)activity.nEndTime || (nNextTime && now_time > (unsigned int)activity.nStartTime))
            {
                activity.nStartTime = activity.nStartTime + loop * (24*3600);
                activity.nEndTime = activity.nEndTime + loop * (24*3600);
            }

            result = true;
        }
        
        {
            //开服前n天开启的活动需要判定下次开启时间是否超过n
            if (activity.nBeforeSrvDay > 0)
            {
                // int i = GetLogicServer()->GetServerOpenTime().rel_today();
                int nDay = (activity.nStartTime.tv - GetLogicServer()->GetServerOpenTime().rel_today())/(3600*24)+1;
                if (nDay > activity.nBeforeSrvDay)
                    return false;
            }
        }
    }
    break;
    }
    
    //#ifdef _DEBUG
        SYSTEMTIME starTime,endTime;
        memset(&starTime,0,sizeof(starTime));
        memset(&endTime,0,sizeof(endTime));
        activity.nStartTime.decode(starTime);
        activity.nEndTime.decode(endTime);

        OutputMsg(rmTip, _T("活动[%d] 开始时间:(%d-%d-%d %d:%d:%d) 结束时间:(%d-%d-%d %d:%d:%d) "),activity.nId,
            starTime.wYear, starTime.wMonth, starTime.wDay, starTime.wHour, starTime.wMinute, starTime.wSecond, 
            endTime.wYear, endTime.wMonth, endTime.wDay, endTime.wHour, endTime.wMinute, endTime.wSecond);
    //#endif
    return result;
}

bool CActivityProvider::CheckOpenTimeLT(GLOBALACTIVITY& activity)
{   
    //检测开服时间比配置小的才开启活动
    if(activity.nRealTimeLt == 0)
        return true;
    if (GetLogicServer()->GetServerOpenTime() > activity.nRealTimeLt)
        return false;
    return true;
}

bool CActivityProvider::CheckOpenTimeGt(GLOBALACTIVITY& activity)
{
    //检测开服时间比配置大的才开启活动
    if(activity.nRealTimeGt == 0)
        return true;
    if (GetLogicServer()->GetServerOpenTime() < activity.nRealTimeLt)
        return false;
    return true;
}

bool CActivityProvider::CheckHefuTimeLt(GLOBALACTIVITY& activity)
{
    //检测合服时间比配置小的才开启活动
    if(GetLogicServer()->GetServerCombineTime() == 0 || activity.nHefuTimeLt == 0)
        return true;
    if (GetLogicServer()->GetServerCombineTime() > activity.nHefuTimeLt)
        return false;
    return true;
}

bool CActivityProvider::CheckHefuTimeGt(GLOBALACTIVITY& activity)
{
    //检测合服时间比配置大的才开启活动
    if(GetLogicServer()->GetServerCombineTime() == 0 || activity.nHefuTimeGt == 0)
        return true;
    if (GetLogicServer()->GetServerCombineTime() < activity.nHefuTimeGt)
        return false;
    return true;
}

bool CActivityProvider::CheckHefuTimes(GLOBALACTIVITY& activity)
{
    GLOBALCONFIG &data = GetLogicServer()->GetDataProvider()->GetGlobalConfig();

    if(activity.nHefuTimes >= 3 && data.nMergeTimes < activity.nHefuTimes)
        return false;
    
    if(activity.nHefuTimes > 0  && activity.nHefuTimes <= 2 && data.nMergeTimes != activity.nHefuTimes)
        return false;
    
    return true;
}



bool CActivityProvider::UpdateOpenTime(lua_State * pLuaState, PERSONACTIVITY& activity, bool nNextTime)
{
    //nNextTime 直接开启下一轮
    bool result = false;
    CMiniDateTime now_time = CMiniDateTime::now();

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("活动[%d] 当前时间:(%d-%d-%d %d:%d:%d) "),activity.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    bool isAfterSrvDay = true;
    
    // 开服N天后才开，第N天仍不开，则计算时间时需要偏移
    if (activity.nAfterSrvDay > 0)
    {
        int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
        int nDiff = activity.nAfterSrvDay - nOpenServerDay + 1;
        if (nDiff > 0)
        {
            now_time += nDiff*3600*24;
            isAfterSrvDay = false;
        }
        
#ifdef _DEBUG
        OutputMsg(rmTip, _T("活动[%d] Diff=%d"), activity.nId, nDiff);
#endif
    }

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("活动[%d] 当前调整时间:(%d-%d-%d %d:%d:%d) "),activity.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    // 开服前N天开，包括第N天，超过这个时间将不再开启
    if (activity.nBeforeSrvDay > 0 &&
        GetLogicServer()->GetDaysSinceOpenServer() > activity.nBeforeSrvDay)
    {
        return false;
    }

    switch (activity.nTimeType)
    {
    case eActivityTimeType_KFSJ: // 开服时间
    {
        for (; activity.nTimeIdx < activity.nTimeCount; activity.nTimeIdx++)
        {
            short curidx = activity.nTimeIdx;

            // 获取结束时间
            activity.nEndTime.tv = 0;
            if(strcmp(activity.pTimeDetail[curidx].strEndTime, "-1"))
            {
                ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, activity.nEndTime);
                activity.nEndTime = GetLogicServer()->GetServerOpenTime().rel_today() + activity.nEndTime;
            }
            // 已经结束的，换下一个点 --默认-1 为永久
            if (activity.nEndTime != 0 && activity.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, activity.nStartTime);
            activity.nStartTime = GetLogicServer()->GetServerOpenTime().rel_today() + activity.nStartTime;
            if(nNextTime && activity.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_GDSJ: // 固定时间
    {
        for (; activity.nTimeIdx < activity.nTimeCount; activity.nTimeIdx++)
        {
            short curidx = activity.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, activity.nEndTime);

            // 已经结束的，换下一个点
            if (activity.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, activity.nStartTime);

            // 开服时间检测
            //if (activity.nAfterSrvDay > 0 && (!isAfterSrvDay) && activity.nStartTime >= now_time) continue;
            if (activity.nAfterSrvDay > 0 && (!isAfterSrvDay) && activity.nStartTime < now_time)
            {
                SYSTEMTIME timeinfo;
                ReadFixedTimeValue(pLuaState, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, timeinfo);
                activity.nStartTime = now_time.rel_today(timeinfo.wHour, timeinfo.wMinute, 0);
            }

            // 切换下一个时间点
            if(nNextTime && activity.nStartTime < now_time) continue;

            result = true;
            break;
        }
    }
    break;
    case eActivityTimeType_HFSJ: // 合服时间
    {
        for (; activity.nTimeIdx < activity.nTimeCount; activity.nTimeIdx++)
        {
            short curidx = activity.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, activity.nEndTime);
            activity.nEndTime = GetLogicServer()->GetServerCombineTime() + activity.nEndTime;

            // 已经结束的，换下一个点
            if (activity.nEndTime <= now_time || nNextTime) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, activity.nTimeType, activity.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, activity.nStartTime);
            activity.nStartTime = GetLogicServer()->GetServerCombineTime() + activity.nStartTime;
            if(nNextTime && activity.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break;
    }
    
    //#ifdef _DEBUG
        SYSTEMTIME starTime,endTime;
        memset(&starTime,0,sizeof(starTime));
        memset(&endTime,0,sizeof(endTime));
        activity.nStartTime.decode(starTime);
        activity.nEndTime.decode(endTime);

        OutputMsg(rmTip, _T("活动[%d] 开始时间:(%d-%d-%d %d:%d:%d) 结束时间:(%d-%d-%d %d:%d:%d) "),activity.nId,
            starTime.wYear, starTime.wMonth, starTime.wDay, starTime.wHour, starTime.wMinute, starTime.wSecond, 
            endTime.wYear, endTime.wMonth, endTime.wDay, endTime.wHour, endTime.wMinute, endTime.wSecond);
    //#endif
    return result;
}


bool CActivityProvider::CheckOpenTimeLT(PERSONACTIVITY& activity)
{   
    //检测开服时间比配置小的才开启活动
    if(activity.nRealTimeLt == 0)
        return true;
    if (GetLogicServer()->GetServerOpenTime() > activity.nRealTimeLt)
        return false;
    return true;
}

bool CActivityProvider::CheckOpenTimeGt(PERSONACTIVITY& activity)
{
    //检测开服时间比配置大的才开启活动
    if(activity.nRealTimeGt == 0)
        return true;
    if (GetLogicServer()->GetServerOpenTime() < activity.nRealTimeLt)
        return false;
    return true;
}

bool CActivityProvider::CheckHefuTimeLt(PERSONACTIVITY& activity)
{
    //检测合服时间比配置小的才开启活动
    if(GetLogicServer()->GetServerCombineTime() == 0 || activity.nHefuTimeLt == 0)
        return true;
    if (GetLogicServer()->GetServerCombineTime() > activity.nHefuTimeLt)
        return false;
    return true;
}

bool CActivityProvider::CheckHefuTimeGt(PERSONACTIVITY& activity)
{
    //检测合服时间比配置大的才开启活动
    if(GetLogicServer()->GetServerCombineTime() == 0 || activity.nHefuTimeGt == 0)
        return true;
    if (GetLogicServer()->GetServerCombineTime() < activity.nHefuTimeGt)
        return false;
    return true;
}

bool CActivityProvider::CheckHefuTimes(PERSONACTIVITY& activity)
{
    GLOBALCONFIG &data = GetLogicServer()->GetDataProvider()->GetGlobalConfig();

    if(activity.nHefuTimes >= 3 && data.nMergeTimes < activity.nHefuTimes)
        return false;
    
    if(activity.nHefuTimes > 0  && activity.nHefuTimes <= 2 && data.nMergeTimes != activity.nHefuTimes)
        return false;
    
    return true;
}