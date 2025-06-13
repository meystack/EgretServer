#include "StdAfx.h"
#include "StrengthConfig.h"




bool CStrengthenCfg::LoadStrengthenConfig(LPCSTR FileName )
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
		//读取标准物品配置数据
		readEquipStrengConfig();
		readFourStarsConfig();
		readRingConfig();
        readDizzyRingConfig();
        readWordsConfig();
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

void CStrengthenCfg::readEquipStrengConfig()
{
    
    m_EquipUpCfg.clear();
    readCfg("EquipStrengthenConfig",m_EquipUpCfg);

}
void CStrengthenCfg::readFourStarsConfig()
{
    m_FourStarsCfg.clear();
    readCfg("FourStarsConfig",m_FourStarsCfg);
}
void CStrengthenCfg::readRingConfig()
{
     m_RingsCfg.clear();
    readCfg("SpecialRingConfig",m_RingsCfg);
}

void CStrengthenCfg::readDizzyRingConfig()
{
     m_DizzyRingsCfg.clear();
    readRing("RingBuyJobConfig",m_DizzyRingsCfg);
}

void CStrengthenCfg::readWordsConfig()
{
    m_WordsCfg.clear();
    readCfg("WordFormulaConfig",m_WordsCfg);
}

void CStrengthenCfg::readRing(LPCSTR Table,  std::map<int, std::map<int, std::vector<StrengthenInfo> > >& infos)
{
    if(!openGlobalTable(Table))
        return;
    infos.clear();
    int nDef = 0;
    if(enumTableFirst())
    {
        do {
            if(enumTableFirst()) {

                do {
                    if(enumTableFirst()) {

                        do {
                            StrengthenInfo info;
                            readData(info);

                            infos[info.nPos][info.nLv].push_back(info);

                        }while(enumTableNext());
                    }
                } while (enumTableNext());
            }
        } while (enumTableNext());
    }
    closeTable();
}


void CStrengthenCfg::readCfg(LPCSTR Table, std::map<int, std::vector<StrengthenInfo> >& infos)
{
    if(!openGlobalTable(Table))
        return;
    infos.clear();
    
    if(enumTableFirst())
    {
        do
        {
            if(enumTableFirst())
            {
                do
                {
                    StrengthenInfo info;
                    readData(info);
                    infos[info.nPos].push_back(info);
                } while (enumTableNext());
            }
        } while (enumTableNext());
    }
    closeTable();
}

void CStrengthenCfg::readData(StrengthenInfo& info)
{
    int nDef = 0;
    info.nPos = getFieldInt("pos", &nDef);
    info.nLv = getFieldInt("lv", &nDef);
    info.nId = getFieldInt("id", &nDef);
    info.nJob = getFieldInt("job", &nDef);
    getFieldStringBuffer(("tips"), info.nTips, sizeof(info.nTips));
    info.cost.clear();
    if(feildTableExists("cost") && openFieldTable("cost"))
    {
        if(enumTableFirst())
        {
            do
            {
                StrengthCost cost;
                cost.nType = getFieldInt("type", &nDef);
                cost.nId = getFieldInt("id", &nDef);
                cost.nCount = getFieldInt("count", &nDef);
                info.cost.push_back(cost);
            } while (enumTableNext());
            
        }
        closeTable();
    }

    if(feildTableExists("limit") && openFieldTable("limit"))
    {
        if(enumTableFirst())
        {
            do
            {
                StrengthUpLimit limit;
                limit.nPos = getFieldInt("pos", &nDef);
                limit.nLimitlv = getFieldInt("lv", &nDef); 
                limit.zsLevel = getFieldInt("zsLevel", &nDef);
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


StrengthenInfo* CStrengthenCfg::getPstrengthen(std::map<int, std::vector<StrengthenInfo> >& infos, int nPos, int nLevel, int nJob)
{  
    vStrengthenIter it = infos.find(nPos);
    if(it != infos.end()) {
        std::vector<StrengthenInfo>& v_list = it->second;
        for(int i = 0; i < v_list.size(); i++) {
            if(v_list[i].nLv == nLevel)
                return &v_list[i];
        }
    }
    return NULL;
}



StrengthenInfo* CStrengthenCfg::getRingtrengthen(std::map<int, std::map<int , std::vector<StrengthenInfo> > >& infos, int nPos, int nLevel, int nJob)
{  
    mStrengthenIter it = infos.find(nPos);
    if(it != infos.end())
    {
        std::map<int , std::vector<StrengthenInfo> >& m_list = it->second;
        vStrengthenIter iter = m_list.find(nLevel);
        if(iter != m_list.end()) {
            std::vector<StrengthenInfo>& v_list = iter->second;
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

StrengthenInfo* CStrengthenCfg::getPstrengthenCfg(int nType, int nPos, int nLevel, int nJob)
{  
    if(nType == CStrengthenCfg::StrengthenEquip)
    {
        return getPstrengthen(m_EquipUpCfg, nPos, nLevel);
    }
    else if(nType == CStrengthenCfg::StrengthenFourStar)
    {
        return getPstrengthen(m_FourStarsCfg, nPos, nLevel);
    }
    else if(nType == CStrengthenCfg::StrengthenRing)
    {
        return getPstrengthen(m_RingsCfg, nPos, nLevel);
    }
    else if(nType == CStrengthenCfg::StrengthenDizzyRing)
    {
        return getRingtrengthen(m_DizzyRingsCfg, nPos, nLevel, nJob);
    }
    else if(nType == CStrengthenCfg::StrengthenWords)
    {
        return getPstrengthen(m_WordsCfg, nPos, nLevel, nJob);
    }
    
    return NULL;
    

}
