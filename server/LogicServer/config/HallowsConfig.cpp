#include "StdAfx.h"
#include "HallowsConfig.h" 

bool CHallowsthenCfg::LoadAll(LPCSTR FileName )
{ 
	bool boResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(FileName) <= 0 )
			showErrorFormat(_T("%s unable to load from %s"), __FUNCTION__, FileName);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), FileName);
		//设置脚本内容
		if ( !setScript(sText) )
			showErrorFormat(_T("%s syntax error on Load config:%s"), __FUNCTION__, FileName);
        //读取主配置相关参数
		boResult = readSoldierSoulMain();
		//升阶
		readWeaponLorderConfig();  
        //升级
		readWeaponLvConfig();  
        //升星
		readStarConfig(); 
        
        //洗练
        readWpRefiningConfig();
        readWpReplaceConfig();
        boResult = true;

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),FileName, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load %s config"), FileName);
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return boResult;
}
bool CHallowsthenCfg::readSoldierSoulMain()
{
    if ( !openGlobalTable("soulWeaponConfig") )
		return false;
    
    int nDefInt_0 = 0;
 
    m_HallowsSSCMainInfo.nSaviorPercen = getFieldInt("saviorPercen", &nDefInt_0);
    m_HallowsSSCMainInfo.nFrozenAddValue = getFieldInt("FrozenAddValue", &nDefInt_0);
    m_HallowsSSCMainInfo.nReduceAttack = getFieldInt("reduceAttack", &nDefInt_0);
    m_HallowsSSCMainInfo.nFrozenTime = getFieldInt("FrozenTime", &nDefInt_0);
    m_HallowsSSCMainInfo.nToBuffResidueHP = getFieldInt("toBuffResidueHP", &nDefInt_0); 
    //m_HallowsSSCMainInfo.nReduceAttackMon = getFieldInt("reduceAttackMon", &nDefInt_0);  

    m_HallowsSSCMainInfo.nMaxFrozen = getFieldInt("MaxFrozen", &nDefInt_0); 
    m_HallowsSSCMainInfo.nTXJZLR = getFieldInt("TXsaviorPercen", &nDefInt_0);
    m_HallowsSSCMainInfo.nTXJDCJ = getFieldInt("TXpoisonbuff", &nDefInt_0);
    m_HallowsSSCMainInfo.nTXSZAS = getFieldInt("TXFrozen", &nDefInt_0);
    m_HallowsSSCMainInfo.nFrozenBuffId = getFieldInt("FrozenBuffId", &nDefInt_0);
    m_HallowsSSCMainInfo.nTXResidueHP = getFieldInt("TXResidueHP", &nDefInt_0);
    m_HallowsSSCMainInfo.nFrozenTimes = getFieldInt("FrozenTimes", &nDefInt_0);
    if (feildTableExists("poisonbuff") && openFieldTable("poisonbuff"))
    {
        if(enumTableFirst())
        {
            int nId = 0;
            do
            {
                if(nId >= 20)
                    break;

                int nBuffId = getFieldInt(NULL);
                m_HallowsSSCMainInfo.vPoisonBuff.emplace_back(nBuffId);
                nId++;
            } while (enumTableNext());
            
        }
        closeTable();
    } 
    if (feildTableExists("reduceAttackMon") && openFieldTable("reduceAttackMon"))
    {
        if(enumTableFirst())
        {
            int nId = 0;
            do
            {
                if(nId >= 20)
                    break;

                int nBuffId = getFieldInt(NULL);
                m_HallowsSSCMainInfo.vReduceAttackMon.emplace_back(nBuffId);
                nId++;
            } while (enumTableNext());
            
        }
        closeTable();
    } 
    if (feildTableExists("saviorBuffid") && openFieldTable("saviorBuffid"))
    {
        if(enumTableFirst())
        {
            int nId = 0;
            do
            {
                if(nId >= 20)
                    break;

                int nBuffId = getFieldInt(NULL);
                m_HallowsSSCMainInfo.vJZLRBuff.emplace_back(nBuffId);
                nId++;
            } while (enumTableNext());
            
        }
        closeTable();
    } 
    closeTable();
    return true;
}

bool CHallowsthenCfg::readWpRefiningConfig()
{
    if(!openGlobalTable("soulWpRefiningConfig"))
        return false;
    m_WpRefiningInfo.clear();
    
    int nDefValue = 0;
    if(enumTableFirst())
    {
        do 
        {
            int nId = getFieldInt("type", &nDefValue);
            WpRefiningInfo group;
            group.nRate =  (WORD)getFieldInt("rate", &nDefValue);

            group.limit.clear();  
            if(feildTableExists("limit") && openFieldTable("limit"))
            {
                if(enumTableFirst())
                {
                    do
                    {
                        HallowsUpLimit limit;
                        limit.nPos = getFieldInt("pos", &nDefValue);
                        limit.nLimitlv = getFieldInt("lv", &nDefValue);
                        group.limit.push_back(limit);
                        
                    } while (enumTableNext());
                    
                }
                closeTable();
            } 
            group.cost.clear();
            if(feildTableExists("consume") && openFieldTable("consume"))
            {
                if(enumTableFirst())
                {
                    do
                    {
                        HallowsthCost cost;
                        cost.nType = getFieldInt("type", &nDefValue);
                        cost.nId = getFieldInt("id", &nDefValue);
                        cost.nCount = getFieldInt("count", &nDefValue);
                        group.cost.push_back(cost);
                    } while (enumTableNext());
                    
                }
                closeTable();
            } 
            group.vAttribute.clear();
            if(feildTableExists("drop") && openFieldTable("drop"))
            {
                if(enumTableFirst())
                {
                    do
                    {
                        DropAttribute info;
                        info.nRate = getFieldInt("rate", &nDefValue);
                        info.nType = getFieldInt("type", &nDefValue);
                        info.nValue = getFieldInt("value", &nDefValue);
                        info.nFollowrate = getFieldInt("followrate", &nDefValue);
                        info.nFollowcount = getFieldInt("followcount", &nDefValue);
                        group.vAttribute.push_back(info);

                    } while (enumTableNext());

                    closeTable();
                }
            }
            m_WpRefiningInfo[nId] = group;
        } while (enumTableNext());
    }
    closeTable();
    return true;
}  
bool CHallowsthenCfg::readWpReplaceConfig()
{
    if(!openGlobalTable("soulWpReplaceConfig"))
        return false;
    m_nReplaces.clear();
    
    if(enumTableFirst())
    {
        do
        {
            if(enumTableFirst())
            {
                do
                {
                    if(enumTableFirst())
                    {
                        do
                        { 
                            WpReplaceInfo info;
                            readWpReplaceData(info); 
                            ReplacesMap *pRep = NULL;
                            std::map <int, ReplacesMap>::iterator it = m_nReplaces.find(info.type);
                            if(it != m_nReplaces.end())
                            {
                                pRep = &(it->second);
                            }
                            else 
                            { 
                                pRep = &m_nReplaces[info.type];
                            }
                            (*pRep)[info.nPos].insert(std::make_pair(info.nItemId, info)); 
                        } while (enumTableNext());
                    }
                } while (enumTableNext());
            }
        } while (enumTableNext());
    }
    closeTable();
    return true; 
} 

void CHallowsthenCfg::readWpReplaceData(WpReplaceInfo& info)
{   
    int nDef = 0;
    info.nPos = getFieldInt("pos", &nDef);
    info.type = getFieldInt("type", &nDef);
    info.nItemId = getFieldInt("itemid", &nDef);  
    info.idx = getFieldInt("idx", &nDef);  
    info.vPrice.clear();
    if(feildTableExists("price") && openFieldTable("price"))
    { 
        HallowsthCost cost; 
        cost.nType = getFieldInt("type", &nDef);
        cost.nId = getFieldInt("id", &nDef);
        cost.nCount = getFieldInt("count", &nDef);
        info.vPrice.push_back(cost); 
/*
        if(enumTableFirst())
        {
            do
            {
                HallowsthCost cost;
                cost.nType = getFieldInt("type", &nDef);
                cost.nId = getFieldInt("id", &nDef);
                cost.nCount = getFieldInt("count", &nDef);
                info.vPrice.push_back(cost);
            } while (enumTableNext());
            
        }
*/
        closeTable();
    }  
}
bool CHallowsthenCfg::readWeaponLorderConfig()
{    
    if(!openGlobalTable("soulWeaponLorderConfig"))
        return false;
    m_LorderInfo.clear();
    
    if(enumTableFirst())
    {
        do
        {
            if(enumTableFirst())
            {
                do
                {
                    HallowsLorderInfo info;
                    readWeaponLorderData(info);
                    m_LorderInfo[info.nPos].insert(std::make_pair(info.nLv, info));
                } while (enumTableNext());
            }
        } while (enumTableNext());
    }
    closeTable();
    return true;
}

void CHallowsthenCfg::readWeaponLorderData(HallowsLorderInfo& info)
{
    int nDef = 0;
    info.nPos = getFieldInt("pos", &nDef);
    info.nLv = getFieldInt("lv", &nDef);
    //info.nId = getFieldInt("id", &nDef);    
    info.cost.clear();
    if(feildTableExists("cost") && openFieldTable("cost"))
    {
        if(enumTableFirst())
        {
            do
            {
                HallowsthCost cost;
                cost.nType = getFieldInt("type", &nDef);
                cost.nId = getFieldInt("id", &nDef);
                cost.nCount = getFieldInt("count", &nDef);
                info.cost.push_back(cost);
            } while (enumTableNext());
            
        }
        closeTable();
    }  

    info.limit.clear();
    if(feildTableExists("limit") && openFieldTable("limit"))
    {
        if(enumTableFirst())
        {
            do
            {
                HallowsUpLimit limit;
                limit.nPos = getFieldInt("pos", &nDef);
                limit.nLimitlv = getFieldInt("lv", &nDef);
                info.limit.push_back(limit);
                
            } while (enumTableNext());
            
        }
        closeTable();
    }

    if(feildTableExists("attr") && openFieldTable("attr"))
    {
        int  nCount = (int)lua_objlen(m_pLua,-1);
        PGAMEATTR pAttr = NULL;
        info.attri.nCount = nCount;
        if(nCount > 0)
        {
            info.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
            ZeroMemory(info.attri.pAttrs, sizeof(GAMEATTR) * nCount);
            pAttr = info.attri.pAttrs;
        }
        if(enumTableFirst())
        {
            if(!LoadAttri(pAttr))
            {
                return;
            }
            pAttr++;
        }
        closeTable();
    }
    if (feildTableExists("buffid") && openFieldTable("buffid"))
    {
        if(enumTableFirst())
        {
            int nId = 0;
            do
            {
                if(nId >= 20)
                    break;

                int nBuffId = getFieldInt(NULL);
                info.vLorderBuff.emplace_back(nBuffId);
                nId++;
            } while (enumTableNext());
        }
        closeTable();
    } 
}
bool CHallowsthenCfg::readWeaponLvConfig()
{ 
    if(!openGlobalTable("soulWeaponLvConfig"))
        return false;
    m_LvInfo.clear();
    
    if(enumTableFirst())
    {
        do
        {
            if(enumTableFirst())
            {
                do
                {
                    HallowsLvInfo info;
                    readWeaponLvData(info); 
                    m_LvInfo[info.nPos].insert(std::make_pair(info.nLv, info));
                } while (enumTableNext());
            }
        } while (enumTableNext());
    }
    closeTable();
    return true;
}

void CHallowsthenCfg::readWeaponLvData(HallowsLvInfo& info)
{
    int nDef = 0;
    info.nPos = getFieldInt("pos", &nDef);
    info.nLv = getFieldInt("lv", &nDef); 
    //info.nId = getFieldInt("id", &nDef);  
    info.cost.clear();
    if(feildTableExists("cost") && openFieldTable("cost"))
    {
        if(enumTableFirst())
        {
            do
            {
                HallowsthCost cost;
                cost.nType = getFieldInt("type", &nDef);
                cost.nId = getFieldInt("id", &nDef);
                cost.nCount = getFieldInt("count", &nDef);
                info.cost.push_back(cost);
            } while (enumTableNext());
            
        }
        closeTable();
    } 
    info.limit.clear();
    if(feildTableExists("limit") && openFieldTable("limit"))
    {
        if(enumTableFirst())
        {
            do
            {
                HallowsUpLimit limit;
                limit.nPos = getFieldInt("pos", &nDef);
                limit.nLimitlv = getFieldInt("lv", &nDef);
                info.limit.push_back(limit);
                
            } while (enumTableNext());
            
        }
        closeTable();
    }

    if(feildTableExists("attr") && openFieldTable("attr"))
    {
        int  nCount = (int)lua_objlen(m_pLua,-1);
        PGAMEATTR pAttr = NULL;
        info.attri.nCount = nCount;
        if(nCount > 0)
        {
            info.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
            ZeroMemory(info.attri.pAttrs, sizeof(GAMEATTR) * nCount);
            pAttr = info.attri.pAttrs;
        }
        if(enumTableFirst())
        {
            if(!LoadAttri(pAttr))
            {
                return;
            }
            pAttr++;
        }
        closeTable();
    }
}
bool CHallowsthenCfg::readStarConfig()
{
    if(!openGlobalTable("soulWeaponstarConfig"))
        return false;
    m_StarInfo.clear();
    
    if(enumTableFirst())
    {
        do
        {
            if(enumTableFirst())
            {
                do
                {
                    HallowsStarInfo info;
                    readWeaponStarData(info); 
                    m_StarInfo[info.nPos].insert(std::make_pair(info.nLv, info));
                } while (enumTableNext());
            }
        } while (enumTableNext());
    }
    closeTable();
    return true;
}
 
void CHallowsthenCfg::readWeaponStarData(HallowsStarInfo& info)
{
    int nDef = 0;
    info.nPos = getFieldInt("pos", &nDef);
    info.nLv = getFieldInt("level", &nDef);
    info.nReducestar = getFieldInt("rate", &nDef);
    //info.nId = getFieldInt("pos", &nDef);   
    info.consume.clear();
    if(feildTableExists("consume") && openFieldTable("consume"))
    {
        if(enumTableFirst())
        {
            do
            {
                HallowsthCost cost;
                cost.nType = getFieldInt("type", &nDef);
                cost.nId = getFieldInt("id", &nDef);
                cost.nCount = getFieldInt("count", &nDef);
                info.consume.push_back(cost);
            } while (enumTableNext());
            
        }
        closeTable();
    } 
    info.limit.clear();
    if(feildTableExists("limit") && openFieldTable("limit"))
    {
        if(enumTableFirst())
        {
            do
            {
                HallowsUpLimit limit;
                limit.nPos = getFieldInt("pos", &nDef);
                limit.nLimitlv = getFieldInt("lv", &nDef);
                info.limit.push_back(limit);
                
            } while (enumTableNext());
            
        }
        closeTable();
    } 
    if(feildTableExists("attribute") && openFieldTable("attribute"))
    {
        int  nCount = (int)lua_objlen(m_pLua,-1);
        PGAMEATTR pAttr = NULL;
        info.attri.nCount = nCount;
        if(nCount > 0)
        {
            info.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
            ZeroMemory(info.attri.pAttrs, sizeof(GAMEATTR) * nCount);
            pAttr = info.attri.pAttrs;
        }
        if(enumTableFirst())
        {
            if(!LoadAttri(pAttr))
            {
                return;
            }
            pAttr++;
        }
        closeTable();
    }
    info.nUpRate = getFieldInt("rate",&nDef);
    int nMaxRate = 0;
    if(feildTableExists("reducestar") && openFieldTable("reducestar")) {
        if(enumTableFirst()) {
            do {
                RateConf rate;
                rate.nRate = getFieldInt("rate");
                rate.nValue = getFieldInt("level");
                nMaxRate = rate.nRate;
                info.m_rates.emplace_back(rate);
            }while(enumTableNext());

        }
        closeTable();
    }
    info.nMaxRate = nMaxRate;
}
/*
HallowsSSCInfo* CHallowsthenCfg::getRingtrengthen(std::map<int, std::map<int , std::vector<HallowsSSCInfo> > >& infos, int nPos, int nLevel, int nJob)
{  
    mHallowsthenIter it = infos.find(nPos);
    if(it != infos.end())
    {
        std::map<int , std::vector<HallowsSSCInfo> >& m_list = it->second;
        vHallowsthenIter iter = m_list.find(nLevel);
        if(iter != m_list.end()) {
            std::vector<HallowsSSCInfo>& v_list = iter->second;
            if( v_list.size() == 1)
                return &v_list[0];
            
            for(int i = 0; i < v_list.size(); i++)
            {
                if(v_list[i].nJob == nJob)
                    return &v_list[i];
            }
        }
    } 
    return NULL; 
}


HallowsSSCInfo* CHallowsthenCfg::getPstrengthen(std::map<int, std::vector<HallowsSSCInfo> >& infos, int nPos, int nLevel, int nJob)
{  
    vHallowsthenIter it = infos.find(nPos);
    if(it != infos.end()) {
        std::vector<HallowsSSCInfo>& v_list = it->second;
        for(int i = 0; i < v_list.size(); i++) {
            if(v_list[i].nLv == nLevel)
                return &v_list[i];
        }
    }
    return NULL;
}
 
HallowsSSCInfo* CHallowsthenCfg::getPhallowthenCfg(int nType, int nPos, int nLevel)
{
    if(nType == CHallowsthenCfg::HallowshenEquip)
    {
        return getPstrengthen(m_HallowsSSConfig, nPos, nLevel);
    }
    else if(nType == CHallowsthenCfg::HallowshenFourStar)
    {
        return getPstrengthen(m_FourStarsCfg, nPos, nLevel);
    }
    else if(nType == CHallowsthenCfg::HallowsthenRing)
    {
        return getPstrengthen(m_RingsCfg, nPos, nLevel);
    } 
    
    return NULL; 
}*/

//获得兵魂总数
int CHallowsthenCfg::OnGetSoulWeapon()
{
    int count = m_LorderInfo.size();
    if (count < m_LvInfo.size())
    {
        count = m_LvInfo.size();
    }
    if (count < m_StarInfo.size())
    {
        count = m_StarInfo.size();
    } 
    return count;
}

bool CHallowsthenCfg::OnCheckLorderInfo(int nPos, int nLevel)
{ 
    std::map<int,  std::map<int, HallowsLorderInfo> >::iterator ifind = m_LorderInfo.find(nPos);
    
    if (ifind == m_LorderInfo.end())
    {
        return false; 
    }
    std::map<int, HallowsLorderInfo>::iterator itfind = ifind->second.find(nLevel);
    if (itfind == ifind->second.end())
    {
        return false; 
    }
    return true; 
} 
bool CHallowsthenCfg::OnCheckLvInfo(int nPos, int nLevel)
{ 
    std::map<int,  std::map<int, HallowsLvInfo> >::iterator ifind = m_LvInfo.find(nPos);
    
    if (ifind == m_LvInfo.end())
    {
        return false; 
    }
    std::map<int, HallowsLvInfo>::iterator itfind = ifind->second.find(nLevel);
    if (itfind == ifind->second.end())
    {
        return false; 
    }
    return true; 
} 
bool CHallowsthenCfg::OnCheckStarInfo(int nPos, int nLevel)
{ 
    std::map<int,  std::map<int, HallowsStarInfo> >::iterator ifind = m_StarInfo.find(nPos);
    
    if (ifind == m_StarInfo.end())
    {
        return false; 
    }
    std::map<int, HallowsStarInfo>::iterator itfind = ifind->second.find(nLevel);
    if (itfind == ifind->second.end())
    {
        return false; 
    }
    return true; 
}     
HallowsLorderInfo* CHallowsthenCfg::OnGetLorderInfo(int nPos, int nLevel)
{ 
    std::map<int,  std::map<int, HallowsLorderInfo> >::iterator find = m_LorderInfo.find(nPos);
    
    if (find == m_LorderInfo.end())
    {
        return NULL; 
    }
    std::map<int, HallowsLorderInfo>::iterator itfind = find->second.find(nLevel);
    if (itfind == find->second.end())
    {
        return NULL; 
    }
    return &(itfind->second); 
}  
HallowsLvInfo* CHallowsthenCfg::OnGetLvInfo(int nPos, int nLevel)
{ 
    std::map<int,  std::map<int, HallowsLvInfo> >::iterator find = m_LvInfo.find(nPos);
    
    if (find == m_LvInfo.end())
    {
        return NULL; 
    }
    std::map<int, HallowsLvInfo>::iterator itfind = find->second.find(nLevel);
    if (itfind == find->second.end())
    {
        return NULL; 
    }
    return &(itfind->second); 
} 
HallowsStarInfo* CHallowsthenCfg::OnGetStarInfo(int nPos, int nLevel)
{ 
    std::map<int,  std::map<int, HallowsStarInfo> >::iterator find = m_StarInfo.find(nPos);
    
    if (find == m_StarInfo.end())
    {
        return NULL; 
    }
    std::map<int, HallowsStarInfo>::iterator itfind = find->second.find(nLevel);
    if (itfind == find->second.end())
    {
        return NULL; 
    }
    return &itfind->second; 
} 
WpRefiningInfo* CHallowsthenCfg::OnGetWpRefiningInfo(int nPos)
{  
    std::map<int, WpRefiningInfo>::iterator itfind = m_WpRefiningInfo.find(nPos); 
    
    if (itfind == m_WpRefiningInfo.end())
    {
        return NULL; 
    } 
    return &(itfind->second); 
}