#include "StdAfx.h"
#include "RebateProvider.h"

CBufferAllocator* CRebateProvider::m_pAllocator;

CRebateProvider::CRebateProvider() :Inherited()
{
    m_pAllocator = new CBufferAllocator();
    m_RebatesGroup.clear();
}

CRebateProvider::~CRebateProvider()
{
    delete m_pAllocator;
    m_pAllocator = NULL;
}

bool CRebateProvider::Load(LPCTSTR sFilePath)
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
            showError(_T("syntax error on Rebate"));

        Result = ReadConfig("RebateDisposeConfig"); // 线下返利
        if ( Result )
        {
            Result = ReadConfig("YBRebateConfig");  // 元宝返利
        }
        if ( Result )
        {
            Result = ReadConfig("SingleRebateConfig");  // 单笔返利
        }
    }
    catch (RefString &s)
    {
        OutputMsg(rmError, _T("load Rebate error: %s"), s.rawStr());
        FILE* fp = fopen("scripterror.txt", "wb");
        if (fp)
        {
            fputs(sText, fp);
            fclose(fp);
        }
    }
    catch (...)
    {
        OutputMsg(rmError, _T("unexpected error on load Rebate"));
    }

    //销毁脚本虚拟机
    setScript(NULL);
    return Result;
}

bool CRebateProvider::ReadConfig(LPCTSTR sFileName)
{
    int nDef = 0;
    char pBuff[1024];

    int nServerIdx = GetLogicServer()->GetServerIndex();

    if ( sFileName && openGlobalTable(sFileName) )
    {

        // 预先分配 30 个 pTimeDetail 所占的空间
        OneTimePair* pTimeDetail = nullptr;
        int nTimeDetailCount = 0;

        if (enumTableFirst())
        {
            pTimeDetail = (OneTimePair*)m_pAllocator->AllocBuffer(sizeof(OneTimePair) * OneTimePairAllocNum);
            do
            {
                if (enumTableFirst())
                {
                    do
                    {
                        int nId = getFieldInt("Id", &nDef);
                        int nOfflineType = getFieldInt("offlineType", &nDef);
                        
                        // 判断Id所对应的组是否已经度去过
                        bool isExist = false;
                        if (nOfflineType <= eRebateOfflineType_MIN || nOfflineType >= eRebateOfflineType_MAX) 
                        {
                            OutputMsg(rmError, _T("Load RebateDisposeConfig Error OfflineType error ! OfflineType : %d"), nOfflineType);
                            continue;
                        }

                        if (m_RebatesGroup.find(nId) != m_RebatesGroup.end())
                        {
                            isExist = true;
                        }


                        if ( !isExist )
                        {
                            // 判断区服是否生效
                            int nOpenServer = getFieldInt("openServer", &nDef);
                            if ( nOpenServer && nOpenServer != nServerIdx )
                            {
                                continue;
                            }

                            RebateGroupInfo& rebateGroupInfo = m_RebatesGroup[nId];
                            rebateGroupInfo.nId = nId;
                            rebateGroupInfo.nOfflineType = nOfflineType;
                            rebateGroupInfo.nOpenServer = nOpenServer;
                            rebateGroupInfo.nTimeType = getFieldInt("timeType", &nDef);
                            rebateGroupInfo.nAtonce = getFieldInt("atonce", &nDef);
                            rebateGroupInfo.nOverlay = getFieldInt("overlay", &nDef);
                                
                            // 详细活动时间
                            if (feildTableExists("TimeDetail") && openFieldTable("TimeDetail"))
                            {
                                int nCount = (int)lua_objlen(m_pLua,-1);
                                rebateGroupInfo.pTimeDetail = (OneTimePair*)m_pAllocator->AllocBuffer(sizeof(OneTimePair) * nCount);
                                for (size_t i = 1; i <= nCount; i++)
                                {
                                    getFieldIndexTable(i);
                                    getFieldStringBuffer("StartTime",rebateGroupInfo.pTimeDetail[i-1].strStartTime,ATIVITY_TIME_LEN);
                                    getFieldStringBuffer("EndTime",rebateGroupInfo.pTimeDetail[i-1].strEndTime,ATIVITY_TIME_LEN);
                                    closeTable();
                                }
                                rebateGroupInfo.nTimeCount = nCount;
                                rebateGroupInfo.nTimeIdx = 0;

                                closeTable();	
                            }

                            // 计算时间（若算不到时间，说明不开启了）
                            if ( !UpdateOpenTime(m_pLua, rebateGroupInfo) )
                            {
                                // 不能释放对应的活动，次日发放的奖励，如果玩家长时间未登录 且过了活动时间，将无法发放奖励，
                            }

                            rebateGroupInfo.RebatesInfo.clear();
                        }
                            
                        RebateGroupInfo& rebateGroupInfo = m_RebatesGroup[nId];

                        // 验证同一Id，奖励序号、时间类型、活动时间、活动类型、指定区开放、立刻发放奖励、是否可叠加等信息是否一致
                        int nIndex = getFieldInt("index", &nDef);
                        if ( isExist )
                        {
                            //验证 奖励序号
                            if ( rebateGroupInfo.RebatesInfo.find(nIndex) != rebateGroupInfo.RebatesInfo.find(nIndex) )
                            {
                                OutputMsg(rmError, _T("Load RebateDisposeConfig Error Index repeat! Id : %d, Index : %d"), nId, nIndex);
                                continue;
                            }

                            //验证 时间类型
                            int nTimeType = getFieldInt("timeType", &nDef);
                            if ( nTimeType != rebateGroupInfo.nTimeType )
                            {
                                OutputMsg(rmError, _T("Load RebateDisposeConfig Error TimeType Different! Id : %d, Index : %d"), nId, nIndex);
                                continue;
                            }
                                
                            //验证 活动时间
                            bool isDifferent = false;
                            if (feildTableExists("TimeDetail") && openFieldTable("TimeDetail"))
                            {
                                int nCount = (int)lua_objlen(m_pLua,-1);
                                memset( pTimeDetail, 0, sizeof( pTimeDetail));
                                for (size_t i = 1; i <= nCount; i++)
                                {
                                    getFieldIndexTable(i);
                                    getFieldStringBuffer("StartTime", pTimeDetail[i-1].strStartTime,ATIVITY_TIME_LEN);
                                    getFieldStringBuffer("EndTime", pTimeDetail[i-1].strEndTime,ATIVITY_TIME_LEN);
                                    closeTable();
                                }

                                nTimeDetailCount = nCount;

                                closeTable();	
                            }
                            if ( nTimeDetailCount != rebateGroupInfo.nTimeCount )
                            {
                                OutputMsg(rmError, _T("Load RebateDisposeConfig Error TimeDetail Count Different! Id : %d, Index : %d"), nId, nIndex);
                                continue;
                            }
                            for (size_t i = 0; i < nTimeDetailCount; i++)
                            {
                                if ( strcmp( pTimeDetail[i].strStartTime, rebateGroupInfo.pTimeDetail[i].strStartTime ) )
                                {
                                    OutputMsg(rmError, _T("Load RebateDisposeConfig Error TimeDetail Different! Id : %d, Index : %d"), nId, nIndex);
                                    isDifferent = true;
                                    continue;
                                }
                                    
                                if ( strcmp( pTimeDetail[i].strEndTime, rebateGroupInfo.pTimeDetail[i].strEndTime ) )
                                {
                                    OutputMsg(rmError, _T("Load RebateDisposeConfig Error TimeDetail Different! Id : %d, Index : %d"), nId, nIndex);
                                    isDifferent = true;
                                    continue;
                                }
                            }
                            if ( isDifferent )
                            {
                                continue;
                            }

                            //验证 活动类型
                            if ( nOfflineType != rebateGroupInfo.nOfflineType )
                            {
                                OutputMsg(rmError, _T("Load RebateDisposeConfig Error OfflineType Different! Id : %d, Index : %d"), nId, nIndex);
                                continue;
                            }

                            //验证 指定区开放
                            int nOpenServer = getFieldInt("openServer", &nDef);
                            if ( nOpenServer != rebateGroupInfo.nOpenServer )
                            {
                                OutputMsg(rmError, _T("Load RebateDisposeConfig Error OpenServer Different! Id : %d, Index : %d"), nId, nIndex);
                                continue;
                            }
                            
                            //验证 立刻发放奖励
                            int nAtonce = getFieldInt("atonce", &nDef);
                            if ( nAtonce != rebateGroupInfo.nAtonce )
                            {
                                OutputMsg(rmError, _T("Load RebateDisposeConfig Error Atonce Different! Id : %d, Index : %d"), nId, nIndex);
                                continue;
                            }

                            //验证 是否可叠加
                            int nOverlay = getFieldInt("overlay", &nDef);
                            if ( nOverlay != rebateGroupInfo.nOverlay )
                            {
                                OutputMsg(rmError, _T("Load RebateDisposeConfig Error Overlay Different! Id : %d, Index : %d"), nId, nIndex);
                                continue;
                            }
                        }

                        RebateInfo& rebateInfo = rebateGroupInfo.RebatesInfo[nIndex];
                        rebateInfo.nIndex = nIndex;

                        // 线下返利、单笔返利
                        if ( rebateGroupInfo.nId < YBRebateStartId )
                        {
                            rebateInfo.nPay = getFieldInt("pay", &nDef);
                            rebateInfo.nMinPay = 0;
                            rebateInfo.nMaxPay = 0;
                        } 
                        else
                        {
                            // 元宝返利
                            rebateInfo.nPay = 0;

                            memset(pBuff, 0, sizeof(pBuff));
                            getFieldStringBuffer("pay",pBuff,sizeof(pBuff));
                            int nTop = lua_gettop(m_pLua);
                            lua_getglobal(m_pLua, "string");
                            lua_getfield(m_pLua, -1, "match");
                            lua_pushlstring(m_pLua, pBuff, strnlen(pBuff, sizeof(pBuff)));
                            lua_pushlstring(m_pLua, "(%d+)-(%d+)", strnlen("(%d+)-(%d+)",sizeof("(%d+)-(%d+)")));
                            int nErr = lua_pcall(m_pLua, 2, 2, 0);
                            if (!nErr)
                            {
                                rebateInfo.nMinPay = lua_tonumber(m_pLua, -2);
                                rebateInfo.nMaxPay = lua_tonumber(m_pLua, -1);
                            }else return false;
                            lua_pop(m_pLua, 2);
                            lua_settop(m_pLua, nTop);
                        }
                        
                        // 线下返利、单笔返利
                        if ( rebateGroupInfo.nId < YBRebateStartId )
                        {
                           if (feildTableExists("awardList") && openFieldTable("awardList"))
                            {
                                rebateInfo.vecAwardList.clear();
                                if(enumTableFirst())
                                {
                                    do
                                    {
                                        ACTORAWARD stTemp ;
                                        stTemp.btType = getFieldInt("type", &nDef) ;
                                        stTemp.wId = getFieldInt("id", &nDef) ;
                                        stTemp.wCount = getFieldInt("count", &nDef) ;
                                        rebateInfo.vecAwardList.emplace_back( stTemp ) ;
                                    } while (enumTableNext());

                                    closeTable();
                                }
                            }
                            rebateInfo.nRebatePercentage = 0;
                        }
                        else 
                        {
                            // 元宝返利
                            rebateInfo.vecAwardList.clear();
                            rebateInfo.nRebatePercentage = getFieldInt("awardList", &nDef);
                        }
                        

                        if (feildTableExists("mailInfo") && openFieldTable("mailInfo"))
                        {
                            rebateInfo.sTitle = (char*)m_pAllocator->AllocBuffer(sizeof(pBuff));
                            memset(pBuff, 0, sizeof(pBuff));
                            getFieldStringBuffer("head", rebateInfo.sTitle, sizeof(pBuff));

                            rebateInfo.sContent = (char*)m_pAllocator->AllocBuffer(sizeof(pBuff));
                            memset(pBuff, 0, sizeof(pBuff));
                            getFieldStringBuffer("context", rebateInfo.sContent,sizeof(pBuff));
                            
                            closeTable();
                        }

                    } while (enumTableNext());
                }
            } while (enumTableNext());

            //释放 pTimeDetail 预分配的内存
            m_pAllocator->FreeBuffer( pTimeDetail);
            pTimeDetail = NULL;  
        }

        closeTable();
        
        return true;
    } else { 
        return false; 
    }
}

bool CRebateProvider::ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out)
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

bool CRebateProvider::ReadFixedTimeValue(lua_State * pLuaState, LPCTSTR sKey, int nKeyLen, SYSTEMTIME& out)
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

bool CRebateProvider::UpdateOpenTime(lua_State * pLuaState, RebateGroupInfo& rebateGroupInfo, bool nNextTime)
{
    //nNextTime 直接开启下一轮
    bool result = false;
    CMiniDateTime now_time = CMiniDateTime::now();

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("活动序号[%d] 当前时间:(%d-%d-%d %d:%d:%d) "),rebateGroupInfo.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    // bool isAfterSrvDay = true;
    
//     // 开服N天后才开，第N天仍不开，则计算时间时需要偏移
//     if (rebateGroupInfo.nAfterSrvDay > 0)
//     {
//         int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
//         int nDiff = rebateGroupInfo.nAfterSrvDay - nOpenServerDay + 1;
//         if (nDiff > 0)
//         {
//             now_time += nDiff*3600*24;
//             isAfterSrvDay = false;
//         }
        
// #ifdef _DEBUG
//         OutputMsg(rmTip, _T("活动[%d] Diff=%d"), rebateGroupInfo.nId,rebateGroupInfo.nIndex nDiff);
// #endif
//     }

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T("活动[%d] 当前调整时间:(%d-%d-%d %d:%d:%d) "),rebateGroupInfo.nId,
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    // // 开服前N天开，包括第N天，超过这个时间将不再开启
    // if (rebateGroupInfo.nBeforeSrvDay > 0 &&
    //     GetLogicServer()->GetDaysSinceOpenServer() > rebateGroupInfo.nBeforeSrvDay)
    // {
    //     return false;
    // }

    switch (rebateGroupInfo.nTimeType)
    {
    case eActivityTimeType_KFSJ: // 开服时间
    {
        for (; rebateGroupInfo.nTimeIdx < rebateGroupInfo.nTimeCount; rebateGroupInfo.nTimeIdx++)
        {
            short curidx = rebateGroupInfo.nTimeIdx;

            // 获取结束时间
            rebateGroupInfo.nEndTime.tv = 0;
            if(strcmp(rebateGroupInfo.pTimeDetail[curidx].strEndTime, "-1"))
            {
                ReadTime(pLuaState, rebateGroupInfo.nTimeType, rebateGroupInfo.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, rebateGroupInfo.nEndTime);
                rebateGroupInfo.nEndTime = GetLogicServer()->GetServerOpenTime().rel_today() + rebateGroupInfo.nEndTime;
            }
            // 已经结束的，换下一个点 --默认-1 为永久
            if (rebateGroupInfo.nEndTime != 0 && rebateGroupInfo.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, rebateGroupInfo.nTimeType, rebateGroupInfo.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, rebateGroupInfo.nStartTime);
            rebateGroupInfo.nStartTime = GetLogicServer()->GetServerOpenTime().rel_today() + rebateGroupInfo.nStartTime;
            if(nNextTime && rebateGroupInfo.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_GDSJ: // 固定时间
    {
        for (; rebateGroupInfo.nTimeIdx < rebateGroupInfo.nTimeCount; rebateGroupInfo.nTimeIdx++)
        {
            short curidx = rebateGroupInfo.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, rebateGroupInfo.nTimeType, rebateGroupInfo.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, rebateGroupInfo.nEndTime);

            // 已经结束的，换下一个点
            if (rebateGroupInfo.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, rebateGroupInfo.nTimeType, rebateGroupInfo.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, rebateGroupInfo.nStartTime);

            // // 开服时间检测
            // if (rebateGroupInfo.nAfterSrvDay > 0 && (!isAfterSrvDay) && rebateGroupInfo.nStartTime < now_time)
            // {
            //     SYSTEMTIME timeinfo;
            //     ReadFixedTimeValue(pLuaState, rebateGroupInfo.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, timeinfo);
            //     rebateGroupInfo.nStartTime = now_time.rel_today(timeinfo.wHour, timeinfo.wMinute, 0);
            // }

            // 切换下一个时间点
            if(nNextTime && rebateGroupInfo.nStartTime < now_time) continue;

            result = true;
            break;
        }
    }
    break;
    case eActivityTimeType_HFSJ: // 合服时间
    {
        for (; rebateGroupInfo.nTimeIdx < rebateGroupInfo.nTimeCount; rebateGroupInfo.nTimeIdx++)
        {
            short curidx = rebateGroupInfo.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, rebateGroupInfo.nTimeType, rebateGroupInfo.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, rebateGroupInfo.nEndTime);
            rebateGroupInfo.nEndTime = GetLogicServer()->GetServerCombineTime() + rebateGroupInfo.nEndTime;

            // 已经结束的，换下一个点
            if (rebateGroupInfo.nEndTime <= now_time || nNextTime) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, rebateGroupInfo.nTimeType, rebateGroupInfo.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, rebateGroupInfo.nStartTime);
            rebateGroupInfo.nStartTime = GetLogicServer()->GetServerCombineTime() + rebateGroupInfo.nStartTime;
            if(nNextTime && rebateGroupInfo.nStartTime < now_time) continue;
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

        for (; rebateGroupInfo.nTimeIdx < rebateGroupInfo.nTimeCount; rebateGroupInfo.nTimeIdx++)
        {
            short curidx = rebateGroupInfo.nTimeIdx;

            // 获取结束时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, rebateGroupInfo.pTimeDetail[curidx].strEndTime, strnlen(rebateGroupInfo.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN));
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
                OutputMsg(rmTip, _T("活动[%d] (%d)结束时间:(%d-%d-%d %d:%d:%d) "),rebateGroupInfo.nId,curidx,
                    nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
            }
//#endif
            
            // 已经结束的，换下一个点
            if (closeTime <= now_time) continue;
            
            // 获取开始时间
            nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, rebateGroupInfo.pTimeDetail[curidx].strStartTime, strnlen(rebateGroupInfo.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN));
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
            rebateGroupInfo.nStartTime = openTime;
            rebateGroupInfo.nEndTime = closeTime;
            result = true;
            break;
        }

        if (!result)
        {
            // 如果都不在时间内，那就是下一周了的第一个时间了
            rebateGroupInfo.nTimeIdx = 0;

            // 获取开始时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, rebateGroupInfo.pTimeDetail[0].strStartTime, strnlen(rebateGroupInfo.pTimeDetail[0].strStartTime, ATIVITY_TIME_LEN));
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
            lua_pushlstring(pLuaState, rebateGroupInfo.pTimeDetail[0].strEndTime, strnlen(rebateGroupInfo.pTimeDetail[0].strEndTime, ATIVITY_TIME_LEN));
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
            rebateGroupInfo.nStartTime = openTime;
            rebateGroupInfo.nEndTime = closeTime;
            while (now_time > (unsigned int)rebateGroupInfo.nEndTime || (nNextTime && now_time > (unsigned int)rebateGroupInfo.nStartTime))
            {
                rebateGroupInfo.nStartTime = rebateGroupInfo.nStartTime + loop * (24*3600);
                rebateGroupInfo.nEndTime = rebateGroupInfo.nEndTime + loop * (24*3600);
            }

            result = true;
        }
        
        // {
        //     //开服前n天开启的活动需要判定下次开启时间是否超过n
        //     if (rebateGroupInfo.nBeforeSrvDay > 0)
        //     {
        //         // int i = GetLogicServer()->GetServerOpenTime().rel_today();
        //         int nDay = (rebateGroupInfo.nStartTime.tv - GetLogicServer()->GetServerOpenTime().rel_today())/(3600*24)+1;
        //         if (nDay > rebateGroupInfo.nBeforeSrvDay)
        //             return false;
        //     }
        // }
    }
    break;
    }
    
    //#ifdef _DEBUG
        SYSTEMTIME starTime,endTime;
        memset(&starTime,0,sizeof(starTime));
        memset(&endTime,0,sizeof(endTime));
        rebateGroupInfo.nStartTime.decode(starTime);
        rebateGroupInfo.nEndTime.decode(endTime);

        OutputMsg(rmTip, _T("活动[%d] 开始时间:(%d-%d-%d %d:%d:%d) 结束时间:(%d-%d-%d %d:%d:%d) "),rebateGroupInfo.nId,
            starTime.wYear, starTime.wMonth, starTime.wDay, starTime.wHour, starTime.wMinute, starTime.wSecond, 
            endTime.wYear, endTime.wMonth, endTime.wDay, endTime.wHour, endTime.wMinute, endTime.wSecond);
    //#endif
    return result;
}