#pragma once
#include "StdAfx.h"
#include "BagConfig.h"

//加载游戏背包配置

bool CLogicDataProvider::LoadBagConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on Guild config"));
		//读取标准物品配置数据
		Result = readBagConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Guild config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Guild config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CLogicDataProvider:: readBagConfig()
{
	BAGCONFIG & dataConfig = GetLogicServer()->GetDataProvider()->GetBagConfig();

	if (!openGlobalTable("BagConfig"))
	{
		OutputMsg(rmError,_T("装载背包失败，找不到Global BagConfig"));
		return false;
	}
	if (feildTableExists("BagInit") && openFieldTable("BagInit"))
	{
		size_t nCount = lua_objlen(m_pLua, -1);
		if (enumTableFirst())
		{
			int nIdx = 0;
			do 
			{
				dataConfig.nDefaultBagCount[nIdx++] = getFieldInt(NULL);
			} while (enumTableNext());
		}
		closeTable();
	}
	if (feildTableExists("ScrollEnterEffect") && openFieldTable("ScrollEnterEffect"))
	{
		size_t nCount = lua_objlen(m_pLua, -1);
		if (enumTableFirst())
		{
			int nIdx = 0;
			do 
			{
				dataConfig.nScrollEnterEffect[nIdx++] = getFieldInt(NULL);
			} while (enumTableNext());
		}
		closeTable();
	}
	dataConfig.nMaxBagCount     = getFieldInt("Max");
	dataConfig.nOneClickRecycling     = getFieldInt("OneClickRecycling");

	
	closeTable();
	return true;

};

