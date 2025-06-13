#include "StdAfx.h"
#include "NoticeProvider.h"
CBufferAllocator* CNoticeProvider::m_pAllocator = NULL;
CNoticeProvider::CNoticeProvider() :Inherited()
{
    m_pAllocator = new CBufferAllocator();
}

CNoticeProvider::~CNoticeProvider()
{
    delete m_pAllocator;
    m_pAllocator = NULL;
}

bool CNoticeProvider::LoadAll(LPCTSTR sFilePath)
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
            showError(_T("syntax error on notice config"));

        Result = ReadAllNotices();
    }
    catch (RefString &s)
    {
        OutputMsg(rmError, _T("load notice config error: %s"), s.rawStr());
        FILE* fp = fopen("scripterror.txt", "wb");
        if (fp)
        {
            fputs(sText, fp);
            fclose(fp);
        }
    }
    catch (...)
    {
        OutputMsg(rmError, _T("unexpected error on load notice config"));
    }

    //销毁脚本虚拟机
    setScript(NULL);
    return Result;
}

bool CNoticeProvider::ReadAllNotices()
{
    bool bDef_false = false;
    int nDefInt_1 = 1;
    int nDefInt_0 = 0;
    int nDefInt_f1 = -1;

    int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
    CMiniDateTime now_time = CMiniDateTime::now();
    if ( openGlobalTable("WholeNoticeConfig") )
    {
        //const INT_PTR nCount = lua_objlen(m_pLua, -1) + 1;
        m_Notices.clear();
        char buff[50];
        //if (nCount > 0)
        //{
            if (enumTableFirst())
            {
               do
               {
                    int nId = getFieldInt("Noticeid");
                    if (m_Notices.find(nId) != m_Notices.end()) return false;
                    
                    GLOBALNOTICE& notice = m_Notices[nId];
                    notice.nId = nId;

	                notice.nAfterSrvDay = getFieldInt("starttime",&nDefInt_0);	 // 开服N天后开
                    notice.nInterval = getFieldInt("noticeInterval",&nDefInt_1); // 公告播放间隔
                    notice.boIsTrundle = getFieldInt("trundle",&nDefInt_0);      // 走马灯
                    notice.boIsChat = getFieldInt("chatting",&nDefInt_0);        // 聊天框信息
                    notice.nTimeType = getFieldInt("TimeType");                 // 时间类型

                    // 开服前N天开，如果过了这个时间，永远不开，则删掉
	                notice.nBeforeSrvDay = getFieldInt("stoptime",&nDefInt_0);
                    if (notice.nBeforeSrvDay && nOpenServerDay > notice.nBeforeSrvDay)
                    {
                        auto it = m_Notices.find(nId);
                        m_Notices.erase(it);
                        continue;
                    }

                    // 公告内容
                    char * pContent=(char *)getFieldString("content");
                    strncpy(notice.sContent, pContent, sizeof(notice.sContent));

                    // 详细公告时间
                    if (feildTableExists("TimeDetail") && openFieldTable("TimeDetail"))
                    {
                        int nCount = (int)lua_objlen(m_pLua,-1);
                        notice.pTimeDetail = (OneTimePair*)m_pAllocator->AllocBuffer(sizeof(OneTimePair) * nCount);
                        for (size_t i = 1; i <= nCount; i++)
                        {
                            getFieldIndexTable(i);
                            getFieldStringBuffer("StartTime",notice.pTimeDetail[i-1].strStartTime,ATIVITY_TIME_LEN);
                            getFieldStringBuffer("EndTime",notice.pTimeDetail[i-1].strEndTime,ATIVITY_TIME_LEN);
                            closeTable();
                        }
                        notice.nTimeCount = nCount;
                        notice.nTimeIdx = 0;
		                closeTable();	
                    }

                    UpdateOpenTime(m_pLua, notice); 

                    // 活动时间类型为非循环时间的公告，需要结束时间大于当前服务器时间，才说明可以开启
                    if (notice.nTimeType != 3)
                    {
                        if ((unsigned int)notice.nEndTime != 0 && (unsigned int)notice.nEndTime <= now_time)
                        {
                            if (notice.pTimeDetail)
                            {
                                m_pAllocator->FreeBuffer(notice.pTimeDetail);
                                notice.pTimeDetail = NULL;
                            }

                            auto it = m_Notices.find(nId);
                            m_Notices.erase(it);
                            continue;
                        }
                    }

               } while (enumTableNext());
            }
        //}
	    closeTable();//ActivitiesConf
    }
    else return false;
    return true;
}

bool CNoticeProvider::ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out)
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

bool CNoticeProvider::UpdateOpenTime(lua_State * pLuaState, GLOBALNOTICE& notice, bool nNextTime)
{
    //nNextTime 直接开启下一轮
    bool result = false;
    CMiniDateTime now_time = CMiniDateTime::now();

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("公告[%d] 当前时间:(%d-%d-%d %d:%d:%d) "),notice.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif
    
    // 开服N天后才开，第N天仍不开，则计算时间时需要偏移
    if (notice.nAfterSrvDay > 0)
    {
        int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
        int nDiff = notice.nAfterSrvDay - nOpenServerDay + 1;
        if (nDiff > 0)
        {
            now_time = CMiniDateTime::today();//定位到活动当天的0点计算
            now_time += nDiff*3600*24;
        }
        
#ifdef _DEBUG
        OutputMsg(rmTip, _T("公告[%d] Diff=%d"), notice.nId, nDiff);
#endif
    }

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("公告[%d] 当前调整时间:(%d-%d-%d %d:%d:%d) "),notice.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    // 开服前N天开，包括第N天，超过这个时间将不再开启
    if (notice.nBeforeSrvDay > 0 &&
        GetLogicServer()->GetDaysSinceOpenServer() > notice.nBeforeSrvDay)
    {
        return false;
    }

    switch (notice.nTimeType)
    {
    case 0: // 开服时间
    {
        for (; notice.nTimeIdx < notice.nTimeCount; notice.nTimeIdx++)
        {
            short curidx = notice.nTimeIdx;

            // 获取结束时间
            notice.nEndTime.tv = 0;
            if(strcmp(notice.pTimeDetail[curidx].strEndTime, "-1"))
            {
                ReadTime(pLuaState, notice.nTimeType, notice.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, notice.nEndTime);
                notice.nEndTime = GetLogicServer()->GetServerOpenTime().rel_today() + notice.nEndTime;
            }
            // 已经结束的，换下一个点 --默认-1 为永久
            if (notice.nEndTime != 0 && notice.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, notice.nTimeType, notice.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, notice.nStartTime);
            notice.nStartTime = GetLogicServer()->GetServerOpenTime().rel_today() + notice.nStartTime;
            if(nNextTime && notice.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break;

    case 1: // 固定时间
    {
        for (; notice.nTimeIdx < notice.nTimeCount; notice.nTimeIdx++)
        {
            short curidx = notice.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, notice.nTimeType, notice.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, notice.nEndTime);

            // 已经结束的，换下一个点
            if (notice.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, notice.nTimeType, notice.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, notice.nStartTime);
            if(nNextTime && notice.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break;

    case 2: // 合服时间
    {
        for (; notice.nTimeIdx < notice.nTimeCount; notice.nTimeIdx++)
        {
            short curidx = notice.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, notice.nTimeType, notice.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, notice.nEndTime);
            notice.nEndTime = GetLogicServer()->GetServerCombineTime() + notice.nEndTime;

            // 已经结束的，换下一个点
            if (notice.nEndTime <= now_time || nNextTime) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, notice.nTimeType, notice.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, notice.nStartTime);
            notice.nStartTime = GetLogicServer()->GetServerCombineTime() + notice.nStartTime;
            if(nNextTime && notice.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break;

    case 3: // 循环时间
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

        static int nStartWeek,nStartHour,nStartMinute;
        static int nEndWeek,nEndHour,nEndMinute;
        static CMiniDateTime openTime,closeTime;

        for (; notice.nTimeIdx < notice.nTimeCount; notice.nTimeIdx++)
        {
            short curidx = notice.nTimeIdx;

            // 获取结束时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, notice.pTimeDetail[curidx].strEndTime, strnlen(notice.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN));
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

#ifdef _DEBUG
            {
                SYSTEMTIME nowTime;
                memset(&nowTime,0,sizeof(nowTime));
                closeTime.decode(nowTime);
                OutputMsg(rmTip, _T("公告[%d] (%d)结束时间:(%d-%d-%d %d:%d:%d) "),notice.nId,curidx,
                    nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
            }
#endif
            
            // 已经结束的，换下一个点
            if (closeTime <= now_time) continue;
            
            // 获取开始时间
            nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, notice.pTimeDetail[curidx].strStartTime, strnlen(notice.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN));
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
            notice.nStartTime = openTime;
            notice.nEndTime = closeTime;
            result = true;
            break;
        }

        if (!result)
        {
            // 如果都不在时间内，那就是下一周了的第一个时间了
            notice.nTimeIdx = 0;

            // 获取开始时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, notice.pTimeDetail[0].strStartTime, strnlen(notice.pTimeDetail[0].strStartTime, ATIVITY_TIME_LEN));
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
            lua_pushlstring(pLuaState, notice.pTimeDetail[0].strEndTime, strnlen(notice.pTimeDetail[0].strEndTime, ATIVITY_TIME_LEN));
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
            notice.nStartTime = openTime;
            notice.nEndTime = closeTime;
            while (now_time > (unsigned int)notice.nEndTime || (nNextTime && now_time > (unsigned int)notice.nStartTime))
            {
                notice.nStartTime = notice.nStartTime + loop * (24*3600);
                notice.nEndTime = notice.nEndTime + loop * (24*3600);
            }

            result = true;
        }
    }
    break;
    }
    
    #ifdef _DEBUG
        SYSTEMTIME starTime,endTime;
        memset(&starTime,0,sizeof(starTime));
        memset(&endTime,0,sizeof(endTime));
        notice.nStartTime.decode(starTime);
        notice.nEndTime.decode(endTime);

        OutputMsg(rmTip, _T("公告[%d] 开始时间:(%d-%d-%d %d:%d:%d) 结束时间:(%d-%d-%d %d:%d:%d) "),notice.nId,
            starTime.wYear, starTime.wMonth, starTime.wDay, starTime.wHour, starTime.wMinute, starTime.wSecond, 
            endTime.wYear, endTime.wMonth, endTime.wDay, endTime.wHour, endTime.wMinute, endTime.wSecond);
    #endif
    return result;
}
