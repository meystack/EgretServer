#include "StdAfx.h"
#include "FashionCfg.h"
#include "../base/Container.hpp"


CFashionCfg::CFashionCfg()
	:Inherited(),m_DataAllocator(_T("FashionataAlloc"))
{
    m_mActive.clear();
    m_mAttribute.clear();
}

CFashionCfg::~CFashionCfg()
{
}

bool CFashionCfg::LoadFashionCfg(LPCTSTR sFilePath)
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
bool CFashionCfg::ReadAllConfig()
{
	if ( !openGlobalTable("fashion") )
		return false;
    int Def = 0;
    if(feildTableExists("FashionupgradeConfig") && openFieldTable("FashionupgradeConfig"))
    {
        m_mAttribute.clear();

        if ( enumTableFirst() )
        {
            do 
            {
                if ( enumTableFirst() )
                {
                    do 
                    {
                        tagFashionAttribute attCfg;
                        attCfg.nId = getFieldInt("id");
                        attCfg.nLv = getFieldInt("lv");
                        if(feildTableExists("consume") && openFieldTable("consume"))
                        {
                            if ( enumTableFirst() )
                            {
                                do 
                                {   
                                    tagFashionCost cost;
                                    cost.nId = getFieldInt("id");
                                    cost.nType = getFieldInt("type");
                                    cost.nCount = getFieldInt("count");
                                    attCfg.cost.push_back(cost);
                                }while(enumTableNext());
                            }
                            closeTable();
                        }

                        if(feildTableExists("attribute") && openFieldTable("attribute"))
                        {
                            int  nCount = (int)lua_objlen(m_pLua,-1);
                            PGAMEATTR pAttr = NULL;
                            attCfg.attri.nCount = nCount;
                            if(nCount > 0)
                            {
                                attCfg.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
                                ZeroMemory(attCfg.attri.pAttrs, sizeof(GAMEATTR) * nCount);
                                pAttr = attCfg.attri.pAttrs;
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
                        m_mAttribute[attCfg.nId].push_back(attCfg);
                    }while(enumTableNext());
                }

            }while(enumTableNext());
        }
        closeTable();
    }

    if(feildTableExists("FashionsetConfig") && openFieldTable("FashionsetConfig"))
    {
        nOpenLv = getFieldInt("openlv");
        nOpenDay = getFieldInt("serveropenday");
        takeofflist.clear();
        if(feildTableExists("cover") && openFieldTable("cover"))
        {
            if ( enumTableFirst() )
            {
                int id = 1;
                do 
                { 
                    std::vector<int> lists;
                    lists.clear();
                    if ( enumTableFirst() )
                    {
                        do 
                        {   
                            int nId = getFieldInt(NULL);
                            takeofflist[id].push_back(nId);
                        }while(enumTableNext());
                    }
                    id++;
                }while(enumTableNext());
            }
            closeTable();
        }
        closeTable();
    }

    if(feildTableExists("FashionattributeConfig") && openFieldTable("FashionattributeConfig"))
    {
        m_mActive.clear();
        if ( enumTableFirst() )
        {
            do 
            {
                if ( enumTableFirst() )
                {
                    do 
                    {
                        tagFashionActive active;
                        active.nId = getFieldInt("id");
                        active.nType = getFieldInt("type");
                        active.nDisplay = getFieldInt("display",&Def);
                        active.nBack = getFieldInt("back",&Def);
                        active.nWear = getFieldInt("wear",&Def);
                        if(feildTableExists("consume") && openFieldTable("consume"))
                        {
                            if ( enumTableFirst() )
                            {
                                do 
                                {   
                                    tagFashionCost cost;
                                    cost.nId = getFieldInt("id");
                                    cost.nType = getFieldInt("type");
                                    cost.nCount = getFieldInt("count");

                                    active.cost.push_back(cost);
                                }while(enumTableNext());
                            }
                            closeTable();
                        }

                        m_mActive[active.nId] = active;
                    }while(enumTableNext());
                }
                
            }while(enumTableNext());
        }
        closeTable();
    }
    closeTable();
	return true;
}



FashionActive* CFashionCfg::getFashionActive(int nId)
{
    std::map <int, tagFashionActive>::iterator it = m_mActive.find(nId);
    if(it != m_mActive.end())
    {
        return &(it->second);
    }

    return NULL;
}

tagFashionAttribute* CFashionCfg::getFashionUpdate(int nId, int nLv)
{
    std::map< int, std::vector<tagFashionAttribute > >::iterator it = m_mAttribute.find(nId);
    if(it != m_mAttribute.end())
    {
       std::vector<tagFashionAttribute >&  res = it->second;
       for(int i = 0; i < res.size(); i++)
       {
            if(res[i].nLv == nLv)
                return &(res[i]);
       }
    }

    return NULL;
}