#include "StdAfx.h"
#include "ItemUpStarConfig.h"
#include "../base/Container.hpp"


ItemUpStarConfig::ItemUpStarConfig()
	:Inherited(),m_DataAllocator(_T("UpStar"))
{
    m_ItemUpStarMaps.clear();
}

ItemUpStarConfig::~ItemUpStarConfig()
{
}

bool ItemUpStarConfig::LoadConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on upstar config"));
		//读取标准物品配置数据
		Result = ReadAllConfig();
		
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load upstar config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load upstar config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;

}
bool ItemUpStarConfig::ReadAllConfig()
{
    int Def = 0;
    if(openGlobalTable("UpstarConfig"))
    {
        m_ItemUpStarMaps.clear();

        if ( enumTableFirst() )
        {
            do 
            {
                if ( enumTableFirst() )
                {
                    do 
                    {
                        ItemUpStarCfg attCfg;
                        attCfg.nId = getFieldInt("id");
                        attCfg.nStarLv = getFieldInt("level");
                        if(feildTableExists("consume") && openFieldTable("consume"))
                        {
                            if ( enumTableFirst() )
                            {
                                do 
                                {   
                                    ACTORAWARD cost;
                                    cost.wId = getFieldInt("id");
                                    cost.btType = getFieldInt("type");
                                    cost.wCount = getFieldInt("count");
                                    attCfg.cost.push_back(cost);
                                }while(enumTableNext());
                            }
                            closeTable();
                        }
                        if(feildTableExists("recovery") && openFieldTable("recovery"))
                        {
                            if ( enumTableFirst() )
                            {
                                do 
                                {   
                                    ACTORAWARD reco;
                                    reco.wId = getFieldInt("id");
                                    reco.btType = getFieldInt("type");
                                    reco.wCount = getFieldInt("count");
                                    attCfg.recoverys.emplace_back(reco);
                                }while(enumTableNext());
                            }
                            closeTable();
                        }
                        if(feildTableExists("attribute") && openFieldTable("attribute"))
                        {
                            int  nCount = (int)lua_objlen(m_pLua,-1);
                            PGAMEATTR pAttr = NULL;
                            attCfg.attr.nCount = nCount;
                            if(nCount > 0)
                            {
                                attCfg.attr.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
                                ZeroMemory(attCfg.attr.pAttrs, sizeof(GAMEATTR) * nCount);
                                pAttr = attCfg.attr.pAttrs;
                                if(enumTableFirst())
                                {
                                    if(!LoadAttri(pAttr))
                                    {
                                        return false;
                                    }
                                    pAttr++;
                                }
                            }
                           
                            closeTable();
                        }

                        attCfg.nUpRate = getFieldInt("rate",&Def);
                        int nMaxRate = 0;
                        if(feildTableExists("reducestar") && openFieldTable("reducestar")) {
                            if(enumTableFirst()) {
                                do {
                                    RateConf rate;
                                    rate.nRate = getFieldInt("rate");
                                    rate.nValue = getFieldInt("level");
                                    nMaxRate = rate.nRate;
                                    attCfg.m_rates.emplace_back(rate);
                                }while(enumTableNext());

                            }
                            closeTable();
                        }
                        attCfg.nMaxRate = nMaxRate;

                        m_ItemUpStarMaps[attCfg.nId ].push_back(attCfg);
                    }while(enumTableNext());
                }              
            }while(enumTableNext());
        }
        closeTable();
    }
    if(openGlobalTable("UpstarPriceConfig"))
    {
        m_nReplaces.clear();

        if ( enumTableFirst() )
        {
            do 
            {
                StarReplaceCfg attCfg;
                attCfg.nItemId = getFieldInt("itemid");
                attCfg.nId = getFieldInt("id");
                attCfg.nPrice = getFieldInt("price",&Def);
                m_nReplaces[attCfg.nItemId] = attCfg;
            }while(enumTableNext());
        }
        closeTable();
    }

	return true;
}



ItemUpStarCfg* ItemUpStarConfig::getItemUpStarCfg(int nId, int nStar)
{
    std::map <int, std::vector<ItemUpStarCfg> >::iterator it = m_ItemUpStarMaps.find(nId);
    if(it != m_ItemUpStarMaps.end())
    {
       for(auto& cfg : it->second){
           if(cfg.nId ==nId &&  cfg.nStarLv == nStar) {
               return &cfg;
           }
       }
    }

    return NULL;
}
