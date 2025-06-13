#include "StdAfx.h"
#include "BlessCfg.h"
#include "../base/Container.hpp"


CBlessCfg::CBlessCfg()
	:Inherited(),m_DataAllocator(_T("BlessataAlloc"))
{
    m_nBlessVectors.clear();
}

CBlessCfg::~CBlessCfg()
{
}

bool CBlessCfg::LoadBlessCfg(LPCTSTR sFilePath)
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
			showError(_T("syntax error on ai config"));
		//读取标准物品配置数据
		Result = ReadAllConfig();
		
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load monster config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadAI config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;

}
bool CBlessCfg::ReadAllConfig()
{
	if ( !openGlobalTable("BlessConfig") )
		return false;
	

	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	//如果物品数量为0则输出未配置任何物品的错误
	if ( nCount <= 0 )
	{
		showError(_T("no BlessCfg data on ai config"));
		return false;
	}


	//循环读取技能配置数据
	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		do 
		{
            BlessCfg bless;
			bless.nLevel = getFieldInt("level");
            bless.nNeedBlessValue = getFieldInt("needBlessValue");
			bless.m_nDailyCost = getFieldInt("blessdown");
            bless.m_nRate.m_RateVectors.clear();
			if(feildTableExists("bless") && openFieldTable("bless"))
            {
                if(enumTableFirst())
                {  
                    do
                    {
                        RateCfg rate;
                        rate.nRate = getFieldInt("rate");
                        rate.nValue = getFieldInt("value");
                        bless.m_nRate.nMaxRate += rate.nRate;
                        bless.m_nRate.m_RateVectors.push_back(rate);
                    } while (enumTableNext());

                }
                closeTable();
            }
            if(feildTableExists("attrs") && openFieldTable("attrs"))
            {
                int  nCount = (int)lua_objlen(m_pLua,-1);
				PGAMEATTR pAttr = NULL;
				bless.attri.nCount = nCount;
				if(nCount > 0)
				{
					bless.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
					ZeroMemory(bless.attri.pAttrs, sizeof(GAMEATTR) * nCount);
					pAttr = bless.attri.pAttrs;
				}
				if(enumTableFirst())
				{
					if(!LoadAttri(pAttr))
					{
						return false;
					}
					pAttr++;
				}
				closeTable();
            }
            m_nBlessVectors.push_back(bless);
		}
		while (enumTableNext());
	}

	closeTable();

    ReadBlessConst();
	return true;
}

bool CBlessCfg::ReadBlessConst()
{
    if ( !openGlobalTable("BlesseConstConfig") )
		return false;
    int nDef = 0;
	m_nItemId = getFieldInt("itemid", &nDef);
    m_nDailyCost = getFieldInt("blessdown", &nDef);
    closeTable();
	return true;
}

BlessCfg* CBlessCfg::GetBlessCfg(int nBlessValue)
{
    int nMax = 0;
    BlessCfg* pCfg = NULL;
    int nSize = m_nBlessVectors.size();
    for(int i = 0; i < nSize; i++)
    {
        BlessCfg& cfg = m_nBlessVectors[i];
        if(nBlessValue >= cfg.nNeedBlessValue && cfg.nNeedBlessValue >= nMax)
        {
            nMax = cfg.nNeedBlessValue;
            pCfg = &m_nBlessVectors[i];
        }
    }

    return pCfg;
}