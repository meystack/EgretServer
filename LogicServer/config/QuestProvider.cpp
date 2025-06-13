#include "StdAfx.h"
#include "QuestProvider.h"
using namespace wylib::stream;

CQuestProvider::CQuestProvider() : m_DataAllocator(_T("QuestDataAlloc"))
{
	m_vAllQuestConfig.clear();
	m_vMainRootQuestConfig.clear();
}
CQuestProvider::~CQuestProvider()
{

}

bool CQuestProvider::LoadQuest(LPCTSTR sFilePath)
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
			showError(_T("syntax error on quest config"));
		//读取标准物品配置数据

		Result	= ReadQuestItem();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load quest config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load quest config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CQuestProvider::ReadQuestItem()
{
	if ( !openGlobalTable("TaskDisplayConfig") )
		return false;
	//获取配置中的物品数量
	// size_t nItemCount = lua_objlen(m_pLua, -1);
	// //如果物品数量为0则输出未配置任何物品的错误
	// if ( nItemCount <= 0 )
	// {
	// 	showError(_T("no quest data on Quest config"));
	// 	return FALSE;
	// }
	//循环读取物品配置数据
	int idx = 0;
	if ( enumTableFirst() )
	{
		do 
		{
			if ( enumTableFirst() )
			{
				do 
				{
					QuestInfoCfg cfg;
					readItemData(cfg);
					m_vAllQuestConfig[cfg.nQid][cfg.nState] = cfg;
					if(cfg.nisFirst == -1)
						m_vMainRootQuestConfig[cfg.nQid] = GetQuestConfig(cfg.nQid, cfg.nState);
				}
				while (enumTableNext());
			}
		}
		while (enumTableNext());
	}

	closeTable();

	return true;
}

bool CQuestProvider::readItemData(QuestInfoCfg& item )
{
	bool boDefVal = false;
	int nDef = 0;
	item.nQid = getFieldInt("id");
	item.nState = getFieldInt("state");
	item.nLevelLimit = getFieldInt("levellimit");
	item.nCircle = getFieldInt("zhuanshenglimit", &nDef);
	item.nOpenDayLimit = getFieldInt("opendaylimit", &nDef);
	item.nisFirst = getFieldInt("isfirst", &nDef);
	item.nIscomplete = getFieldInt("iscomplete", &nDef);
	item.nAnnular = getFieldInt("iscircle", &nDef);
	item.nShowSchedule = getFieldInt("schedule", &nDef);
	item.nFrontQid = getFieldInt("pretask", &nDef);
	item.ntasktype = getFieldInt("tasktype", &nDef);
	item.nAcceptState = getFieldInt("taskaccept", &nDef);
	if ( feildTableExists("buttonchange") && openFieldTable("buttonchange") )
	{
		
		if(enumTableFirst())
		{
			do
			{
				ChangeQuestState state;
				state.nQid = getFieldInt("taskid", &nDef);
				state.nState = getFieldInt("state", &nDef);
				item.button.push_back(state);
			}while(enumTableNext());
		}
		closeTable();
	}
	
	
	if ( feildTableExists("autochange") && openFieldTable("autochange") )
	{
		if(enumTableFirst())
		{
			do
			{
				ChangeQuestState state;
				state.nQid = getFieldInt("taskid", &nDef);
				state.nState = getFieldInt("state", &nDef);
				item.nAuto.push_back(state);
			}while(enumTableNext());
		}
		closeTable();
	}
		
	if ( feildTableExists("changeState") && openFieldTable("changeState") )
	{
		item.mQuestInfo.nType = getFieldInt("type", &nDef);
		item.mQuestInfo.nValue = getFieldInt("value", &nDef);
		item.mQuestInfo.nSubType = getFieldInt("subtype", &nDef);
		if(feildTableExists("mapid") && openFieldTable("mapid"))
		{
			if(enumTableFirst())
			{
				do
				{
					int nId = getFieldInt(NULL);
					item.mQuestInfo.nMaps.push_back(nId);
				}while(enumTableNext());
			}
			closeTable();
		}
		if(feildTableExists("group") && openFieldTable("group"))
		{
			if(enumTableFirst())
			{
				do
				{
					int nId = getFieldInt(NULL);
					item.nGroups.push_back(nId);
				}while(enumTableNext());
			}
			closeTable();
		}

		if(feildTableExists("id") && openFieldTable("id"))
		{
			if(enumTableFirst())
			{
				do
				{
					int nId = getFieldInt(NULL);
					item.mQuestInfo.nIds.push_back(nId);
				}while(enumTableNext());
			}
			closeTable();
		}
		closeTable();
	}

	if ( feildTableExists("flyshoescost") && openFieldTable("flyshoescost") )
	{
		if(enumTableFirst())
		{
			do
			{
				QuestCostFlyshoe flycost;
				flycost.times = getFieldInt("times", &nDef);
				if(feildTableExists("cost") && openFieldTable("cost") )
				{
					flycost.costs.nId = getFieldInt("id",&nDef);
					flycost.costs.nCount = getFieldInt("count",&nDef);
					flycost.costs.nType = getFieldInt("type",&nDef);
					closeTable();
				}
				item.m_nCostFlyShoes.push_back(flycost);
			}while(enumTableNext());
		}
		closeTable();
	}
	int nType = 1;
	if ( feildTableExists("tasknameclick") && openFieldTable("tasknameclick") )
	{
		// TeleportInfo tel;
		// tel.nMapid = getFeildInt("map", &nDef);
		// tel.nId = getFeildInt("id", &nDef);
		// tel.nNpcId = getFeildInt("npcid", &nDef);
		// tel.nX = getFeildInt("x", &nDef);
		// tel.nY = getFeildInt("y", &nDef);
		// tel.nRange = getFeildInt("range", &nDef);
		// item.m_nTelinfo[type].push_back(tel);
		if(enumTableFirst())
		{
			do
			{
				ReadTelePortInfo(item, nType);
			}while(enumTableNext());
		}
		closeTable();
	}
	

	nType++;
	if ( feildTableExists("teleportpos") && openFieldTable("teleportpos") )
	{
		if(enumTableFirst())
		{
			do
			{
				ReadTelePortInfo(item, nType);
			}while(enumTableNext());
		}
		closeTable();
	}

	nType++;
	if ( feildTableExists("flyshoespos") && openFieldTable("flyshoespos") )
	{
		// TeleportInfo tel;
		// tel.nMapid = getFeildInt("map", &nDef);
		// tel.nId = getFeildInt("id", &nDef);
		// tel.nX = getFeildInt("x", &nDef);
		// tel.nY = getFeildInt("y", &nDef);
		// tel.nRange = getFeildInt("range", &nDef);
		// tel.nNpcId = getFeildInt("npcid", &nDef);
		// item.m_nTelinfo[type].push_back(tel);
		if(enumTableFirst())
		{
			do
			{
				ReadTelePortInfo(item, nType);
			}while(enumTableNext());
		}
		closeTable();
	}
	if ( feildTableExists("award") && openFieldTable("award") )
	{
		if(enumTableFirst())
		{
			do
			{
				QuestAward award;
				award.nCount = getFieldInt("count", &nDef);
				award.nType = getFieldInt("type", &nDef);
				award.nId = getFieldInt("id", &nDef);
				item.awards.push_back(award);
			}while(enumTableNext());
		}
		closeTable();
	}
	return TRUE;
}

void CQuestProvider::ReadTelePortInfo(QuestInfoCfg& item , int nType)
{
	TeleportInfo tel;
	int nDef = 0;
	tel.nId = getFieldInt("id", &nDef);
	tel.nMapid = getFieldInt("map", &nDef);
	tel.nX = getFieldInt("x", &nDef);
	tel.nY = getFieldInt("y", &nDef);
	tel.nRange = getFieldInt("range", &nDef);
	tel.nIsOpen = getFieldInt("isopen", &nDef);
	tel.nAuto = getFieldInt("outboard", &nDef);
	tel.nTelNpcId = getFieldInt("npcid", &nDef);
	item.m_nTelinfo[nType].push_back(tel);
}

