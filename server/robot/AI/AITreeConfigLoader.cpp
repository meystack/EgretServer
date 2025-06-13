#include "stdafx.h"
using namespace wylib::stream;

CAITreeConfigLoader::CAITreeConfigLoader()
{
}

void CAITreeConfigLoader::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Scene Config Error]");
	s += sError;
	throw s;
}

bool CAITreeConfigLoader::Load(LPCTSTR pConfigPath)
{
	bool bRet		= false;
	LPCTSTR sResult = NULL;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{
		GetRobotMgr()->GetVSPDefine().RegisteToPreprocessor(pp);
		if (ms.loadFromFile(pConfigPath) <= 0)
		{
			showErrorFormat(_T("unable to load config file from %s"), pConfigPath);
			return false;
		}

		sResult = pp.parse((LPCTSTR)ms.getMemory(), pConfigPath);
		if (!sResult)
		{
			OutputMsg(rmError, _T("预处理文件%s失败"), pConfigPath);
			return false;
		}
		if (!setScript(sResult))
		{
			OutputMsg(rmError,_T("读取预处理后的配置文件%s失败"), pConfigPath);
			return false;
		}
		bRet = LoadAITreeConfig();
	}
	catch (RefString& s)
	{
		OutputMsg(rmError, (LPCTSTR)s);
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexcepted error on load config: %s"), pConfigPath);
	}
	setScript(NULL);
	return bRet;
}

void CAITreeConfigLoader::Run(CRobotClient* pActor, TICKCOUNT tick)
{
	m_AIRootNode->Execute(pActor, 0, false);
}

bool CAITreeConfigLoader::LoadAITreeConfig()
{
	std::vector<AINodeDescribe> des_vtr;
	char sdesc[64];
	if (openGlobalTable("AITreeConfig"))
	{
		int nid = des_vtr.size() + 1;
		int ntype = getFieldInt("ntype");
		getFieldStringBuffer("desc",sdesc,ArrayCount(sdesc));
		des_vtr.push_back(AINodeDescribe(nid, 0, ntype, sdesc));
		OutputMsg(rmNormal, _T("AITree [%d]-[%d] = [%d]%s"),0,nid,ntype,sdesc);
		LoadSubTree(nid, des_vtr);
	} else return FALSE;
	closeTable();
	m_AIRootNode = AINodeHelper::SharedHelper()->CreateNodeTree(des_vtr); 
	return true;
}

bool CAITreeConfigLoader::LoadSubTree(int pid, std::vector<AINodeDescribe>& des_vtr)
{
	char sdesc[64];
	if (feildTableExists("nodes") && openFieldTable("nodes"))
	{
		int nCount = (int)lua_objlen(m_pLua, -1);
		for (size_t i = 1; i <= nCount; i++)
		{
			if (!getFieldIndexTable(i))
			{
				OutputMsg(rmError, "AITree open Index(%d) table failed. ", i);
				return false;
			}
			int nid = des_vtr.size() + 1;
			int ntype = getFieldInt("ntype");
			getFieldStringBuffer("desc",sdesc,ArrayCount(sdesc));
			des_vtr.push_back(AINodeDescribe(nid, pid, ntype, sdesc));
			OutputMsg(rmNormal, _T("AITree [%d]-[%d] = [%d]%s"),pid,nid,ntype,sdesc);
			LoadSubTree(nid, des_vtr);
			closeTable();
		}
		closeTable();
	}
	else if (feildTableExists("node") && openFieldTable("node"))
	{
		int nid = des_vtr.size() + 1;
		int ntype = getFieldInt("ntype");
		getFieldStringBuffer("desc",sdesc,ArrayCount(sdesc));
		des_vtr.push_back(AINodeDescribe(nid, pid, ntype, sdesc));
		OutputMsg(rmNormal, _T("AITree [%d]-[%d] = [%d]%s"),pid,nid,ntype,sdesc);
		closeTable();
	}
}