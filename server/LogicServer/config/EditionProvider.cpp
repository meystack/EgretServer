#include "StdAfx.h"

CBufferAllocator* CEditionProvider::m_pAllocator;

CEditionProvider::CEditionProvider() :Inherited()
{
    m_pAllocator = new CBufferAllocator();
}

CEditionProvider::~CEditionProvider()
{
    delete m_pAllocator;
    m_pAllocator = NULL;
}

bool CEditionProvider::Load(LPCTSTR sFilePath)
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
            showError(_T("syntax error on Edition"));

        Result = ReadConfig("editionConf");
    }
    catch (RefString &s)
    {
        OutputMsg(rmError, _T("load Edition error: %s"), s.rawStr());
        FILE* fp = fopen("scripterror.txt", "wb");
        if (fp)
        {
            fputs(sText, fp);
            fclose(fp);
        }
    }
    catch (...)
    {
        OutputMsg(rmError, _T("unexpected error on load Edition"));
    }

    //销毁脚本虚拟机
    setScript(NULL);
    return Result;
}

bool CEditionProvider::ReadConfig(LPCTSTR sFileName)
{
    int nDefaultInt = 0;
    if ( sFileName && openGlobalTable(sFileName) )
	{
        m_EditionInfo.nStart = getFieldInt("Button", &nDefaultInt);
        m_EditionInfo.nJiaoYiEDuRate = getFieldInt("JiaoYiEDuRate",&nDefaultInt);
        m_EditionInfo.nProportion = getFieldInt("proportion", &nDefaultInt);
        m_EditionInfo.nNoSetOpenDay = getFieldInt("nosetopenday", &nDefaultInt);

        m_EditionInfo.canSimulor = (getFieldInt("phoneyopen", &nDefaultInt) == 1) ? true : false;
        m_EditionInfo.nSimulorSize = getFieldInt("phoneynumber", &nDefaultInt);
   
        if (feildTableExists("phoneymaprange") && openFieldTable("phoneymaprange"))
        {
            if (enumTableFirst())
            {
                do 
                {
                    SimulorMapPosInfo info;// = m_EditionInfo.nSimulorSceneInfo;
                    info.nSimulorMapSceneId = getFieldInt("map", &nDefaultInt);
                    info.nSimulorMinPosX = getFieldInt("min_x", &nDefaultInt);
                    info.nSimulorMinPosY = getFieldInt("min_y", &nDefaultInt);
                    info.nSimulorMaxPosX = getFieldInt("max_x", &nDefaultInt);
                    info.nSimulorMaxPosY = getFieldInt("max_y", &nDefaultInt); 
                    m_EditionInfo.nSimulorSceneInfo.push_back(info);
                } while (enumTableNext());
            }		
            closeTable();	
        }

        if (feildTableExists("privilege") && openFieldTable("privilege"))
        {
            m_EditionInfo.nPrivilege = getFieldInt("CardLv", &nDefaultInt);
            closeTable();
        }

        m_EditionInfo.nSecondChargeIds.clear();
        if (feildTableExists("SecondChargeID") && openFieldTable("SecondChargeID"))
        {
            if (enumTableFirst())
            {
                do 
                {
                    INT_PTR id= getFieldInt(NULL);
                    m_EditionInfo.nSecondChargeIds.push_back(id);
                }
                while (enumTableNext());
            }
            closeTable();
        }

        m_EditionInfo.vecCreateRoleAutoAddTitleId.clear();
        if (feildTableExists("Createtitle") && openFieldTable("Createtitle"))
        {
            if (enumTableFirst())
            {
                do 
                {
                    INT_PTR id= getFieldInt(NULL);
                    m_EditionInfo.vecCreateRoleAutoAddTitleId.push_back(id);
                }
                while (enumTableNext());
            }
            closeTable();
        }

        m_EditionInfo.vecServerNameId.clear();
        if (feildTableExists("NamingRightID") && openFieldTable("NamingRightID"))
        {
            if (enumTableFirst())
            {
                do 
                {
                    INT_PTR id= getFieldInt(NULL);
                    m_EditionInfo.vecServerNameId.push_back(id);
                }
                while (enumTableNext());
            }
            closeTable();
        }

        closeTable();

        return true;
    }
    return false;    
}