#include "StdAfx.h"
#include "Refining.h"
#include "../base/Container.hpp"


CRefiningCfg::CRefiningCfg()
	:Inherited(),m_DataAllocator(_T("FashionataAlloc"))
{
    m_nReiningCfgs.clear();
}

CRefiningCfg::~CRefiningCfg()
{
}

bool CRefiningCfg::LoadConfig(LPCTSTR sFilePath)
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
bool CRefiningCfg::ReadAllConfig()
{
    int Def = 0;
    if(openGlobalTable("RefiningmaterialsConfig"))
    {
        m_nReiningCfgs.clear();

        if ( enumTableFirst() )
        {
            do 
            {
                RefiningCfg attCfg;
                attCfg.nId = getFieldInt("id");
                attCfg.nRefineId = getFieldInt("refineId",&Def);
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
                m_nReiningCfgs[attCfg.nId ] = attCfg;
            }while(enumTableNext());
        }
        closeTable();
    }

    // if(openGlobalTable("RefiningBaseConfig"))
    // {
    //     nVoucher2 = getFieldInt("voucher2");
    //     nRefininglevel = getFieldInt("Refininglevel");
    //     nVoucher1 = getFieldInt("voucher1");
    //     nMaterialprice = getFieldInt("Materialprice");
    //     closeTable();
    // }

    if(openGlobalTable("RefiningReplaceConfig"))
    {
        m_nReplaces.clear();

        if ( enumTableFirst() )
        {
            do 
            {
                ReplaceCfg attCfg;
                attCfg.nItemId = getFieldInt("itemid");
                attCfg.nId = getFieldInt("id");
                attCfg.nPrice = getFieldInt("price",&Def);
                m_nReplaces[attCfg.nItemId] = attCfg;
            }while(enumTableNext());
        }
        closeTable();
    }

	if (openGlobalTable("RefiningBaseConfig"))
    {
        int nDefValue = 0;
        m_RefingAttributeMaps.clear();
        if (enumTableFirst())
        {		
            do 
            {
                int nId = getFieldInt("type",&nDefValue);
                DROPATTRIBUTECFG group;
                group.nRate =  (WORD)getFieldInt("rate",&nDefValue);
                group.m_vAttribute.clear();
                if(feildTableExists("drop") && openFieldTable("drop"))
                {
                    if(enumTableFirst())
                    {
                        do
                        {
                            DROPATTRIBUTE info;
                            info.nRate = getFieldInt("rate", &nDefValue);
                            info.nType = getFieldInt("type", &nDefValue);
                            info.nValue = getFieldInt("value", &nDefValue);
                            info.nFollowrate = getFieldInt("followrate", &nDefValue);
                            info.nFollowcount = getFieldInt("followcount", &nDefValue);
                            group.m_vAttribute.push_back(info);

                        } while (enumTableNext());

                        closeTable();
                    }

                }

                m_RefingAttributeMaps[nId] = group;
            } while (enumTableNext());
        }
        closeTable();
    }
    

	return true;
}



RefiningCfg* CRefiningCfg::getRefiningCfg(int nId)
{
    std::map <int, RefiningCfg>::iterator it = m_nReiningCfgs.find(nId);
    if(it != m_nReiningCfgs.end())
    {
        return &(it->second);
    }

    return NULL;
}



void CRefiningCfg::GetRefineAttribute(int type, DROPGOODS& item)
{
	std::map<int, DROPATTRIBUTECFG>::iterator dropAttrCfg = m_RefingAttributeMaps.find(type);
	if(dropAttrCfg != m_RefingAttributeMaps.end())
	{
		DROPATTRIBUTECFG cfg = dropAttrCfg->second;
		int rand = wrand(100);
		if(rand <= cfg.nRate)
		{
			int count = cfg.m_vAttribute.size();
			for(int i = 0; i < count; i++)
			{
				DropAttribute& info = cfg.m_vAttribute[i];
				int rand_ = wrand(100);
				// printf("GetDropAttribute: %d, %d\n",rand_,info.nRate);
				if(rand_ <= info.nRate)
				{
					// printf("GetDropAttribute: %d, %d\n",rand_,info.nRate);
					item.AddDropItemAttr(info);
					if(info.nFollowcount)
					{
						for(int j = 0; j < info.nFollowcount; j++)
						{
							int rand = wrand(100);
							if(rand <= info.nFollowrate)
							{
								item.AddDropItemAttr(info);
                                continue;
							}
							break;
						}
					}
				}
			} 
		}
	}
}