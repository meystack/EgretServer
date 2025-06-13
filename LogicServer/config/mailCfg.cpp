#include "StdAfx.h"
#include "mailCfg.h"
#include "../base/Container.hpp"
CMailCfg::CMailCfg()
	:Inherited(),m_NewDataAllocator(_T("CMailCfgDataAlloc"))
{

}

CMailCfg::~CMailCfg()
{
}

bool CMailCfg::LoadConfig(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
		{
			showError(_T("syntax error on RideGame config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
		}
		//读取标准物品配置数据
		Result = readConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load CAchieve config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load CAchieve config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}



bool CMailCfg::readConfigs()
{
   	if ( !openGlobalTable("MailIDConfig") )
		return FALSE; 
    nALlMails.clear();
    if(enumTableFirst())
    {
        do
        {
            MailCfg cfg;
            cfg.nId = getFieldInt("id");
            getFieldStringBuffer(("title"), cfg.cTitle, sizeof(cfg.cTitle));
            getFieldStringBuffer(("content"), cfg.cContent, sizeof(cfg.cContent));
            if(feildTableExists("attachment") && openFieldTable("attachment"))
            {
                if(enumTableFirst())
                {
                    do
                    {
                        ACTORAWARD award;
                        award.btType = (byte)getFieldInt("type");
                        award.wId = (WORD)getFieldInt("id");
                        award.wCount = (WORD)getFieldInt("count");
                        cfg.vAwards.push_back(award);
                    } while (enumTableNext()); 
                }
                closeTable();
            }
            nALlMails[cfg.nId] = cfg;
        } while (enumTableNext());
        
    }

    closeTable();
    readLvMailConfigs();
    readOpenDayMailConfigs();
    readCombineMailConfigs();
    return true;

}
bool CMailCfg::readLvMailConfigs()
{
	if ( !openGlobalTable("LevelMailConfig") )
		return FALSE; 
    m_lvMails.clear();
    if(enumTableFirst())
    {
        do
        {
            int nId = getFieldInt("level");
            if(feildTableExists("idList") && openFieldTable("idList"))
            {
                if(enumTableFirst())
                {
                    do
                    {
                        int id = getFieldInt(NULL);
                        m_lvMails[nId].push_back(id);
                    } while (enumTableNext());
                }
                closeTable();
            }
        } while (enumTableNext());
        
    }
    closeTable();
	return true;
}


bool CMailCfg::readOpenDayMailConfigs()
{
	if ( !openGlobalTable("LoginDayMailConfig") )
		return FALSE; 
    m_nOpenDayMails.clear();
    if(enumTableFirst())
    {
        do
        {
            int nId = getFieldInt("day");
            if(feildTableExists("idList") && openFieldTable("idList"))
            {
                if(enumTableFirst())
                {
                    do
                    {
                        int id = getFieldInt(NULL);
                        m_nOpenDayMails[nId].push_back(id);
                    } while (enumTableNext());
                }
                closeTable();
            }
        } while (enumTableNext());
    }
    closeTable();
	return true;
}


bool CMailCfg::readCombineMailConfigs()
{
	if ( !openGlobalTable("MergeServerMailConfig") )
		return FALSE; 
    m_nCombineMails.clear();
    if(enumTableFirst())
    {
        do
        {
            int nId = getFieldInt("level");
            if(feildTableExists("idList") && openFieldTable("idList"))
            {
                if(enumTableFirst())
                {
                    do
                    {
                        int id = getFieldInt(NULL);
                        m_nCombineMails[nId].push_back(id);
                    } while (enumTableNext());
                }
                closeTable();
            }
        } while (enumTableNext());
    }
    closeTable();
	return true;
}

MailCfg* CMailCfg::getOneMailCfg(int nId)
{
    std::map<int ,MailCfg>::iterator it = nALlMails.find(nId);
    if(it != nALlMails.end())
    {
        return &(it->second);
    }
    return NULL;
}