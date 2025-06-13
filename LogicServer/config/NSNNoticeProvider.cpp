#include "StdAfx.h"
#include "NSNNoticeProvider.h"
//CBufferAllocator* CNSNNoticeProvider::m_pAllocator = NULL;
CNSNNoticeProvider::CNSNNoticeProvider() :Inherited()
{
    //m_pAllocator = new CBufferAllocator();
}

CNSNNoticeProvider::~CNSNNoticeProvider()
{
    //delete m_pAllocator;
    //m_pAllocator = NULL;
}

bool CNSNNoticeProvider::LoadAll(LPCTSTR sFilePath)
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
        OutputMsg(rmError, _T("load NSNNotice config error: %s"), s.rawStr());
        FILE* fp = fopen("scripterror.txt", "wb");
        if (fp)
        {
            fputs(sText, fp);
            fclose(fp);
        }
    }
    catch (...)
    {
        OutputMsg(rmError, _T("unexpected error on load NSNnotice config"));
    }

    //销毁脚本虚拟机
    setScript(NULL);
    return Result;
}

bool CNSNNoticeProvider::ReadAllNotices()
{
    bool bDef_false = false;
    int nDefInt_1 = 1;
    int nDefInt_0 = 0;
    int nDefInt_f1 = -1;

    int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
    CMiniDateTime now_time = CMiniDateTime::now();
    //if ( openGlobalTable("NSNNoticeConfig") )
    if ( openGlobalTable("NSNoticeConfig") )
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
                    
                    NSNNOTICE& notice = m_Notices[nId];
                    notice.nId = nId;

	                notice.nAfterSrvDay = getFieldInt("starttime", &nDefInt_0);	 // 开服N天后开 
    
                    notice.nInterval = getFieldInt("noticeInterval", &nDefInt_0); // 公告播放间隔
                    notice.boIsTrundle = getFieldInt("trundle", &nDefInt_0);      // 走马灯
                    notice.boIsChat = getFieldInt("chatting", &nDefInt_0);        // 聊天框信息
                    //notice.nTimeType = getFieldInt("TimeType");                 // 时间类型

                    notice.nNextTime = 0;  
                    // 开服前N天开，如果过了这个时间，永远不开，则删掉
	                notice.nBeforeSrvDay = getFieldInt("stoptime",&nDefInt_0); 
                    if (notice.nBeforeSrvDay && nOpenServerDay > notice.nBeforeSrvDay)
                    {
                        auto it = m_Notices.find(nId);
                        m_Notices.erase(it);
                        continue;
                    }

                    // 公告内容
                    char * pContent = (char *)getFieldString("content");
                    strncpy(notice.sContent, pContent, sizeof(notice.sContent));
 
                    //UpdateOpenTime(m_pLua, notice);

               } while (enumTableNext());
            }
        //}
	    closeTable();//ActivitiesConf
    }
    else return false;
    return true;
} 