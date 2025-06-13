
#include "StdAfx.h"
#include "../base/Container.hpp"

CBuffProvider::CBuffProvider()
{

}

CBuffProvider::~CBuffProvider()
{

}

bool CBuffProvider::Load( LPCTSTR sFilePath )
{
	bool boResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("%s unable to load from %s"), __FUNCTION__, sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showErrorFormat(_T("%s syntax error on Load config:%s"), __FUNCTION__, sFilePath);
		//读取标准物品配置数据
		boResult = ReadBuffConfig();

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load %s config"), sFilePath);
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return boResult;
}

bool CBuffProvider::ReadBuffConfig()
{
	if (!openGlobalTable("BuffConf"))
	{
		return false;
	}
	size_t nCount = lua_objlen(m_pLua, -1);
	if (nCount <= 0)
	{
		return false;
	}
	m_vectorBuff.clear();
	m_vectorBuff.reserve(nCount);
	m_vectorBuff.trunc(nCount);

	if (enumTableFirst())
	{
		int nDef = 0;
		double dDef = 0;
		const bool boDef = false;
		
		do 
		{
			int nId = (WORD)getFieldInt("id"); 
			tagBuff& buff = m_vectorBuff[nId-1];
			buff.nId = nId;
			buff.nType = (WORD)getFieldInt("type");
			if (buff.nType < aUndefined || buff.nType >= GameAttributeCount)
			{
				OutputMsg(rmError, _T("%s:配置一个没定义的BUFF类型:%d,ID是%d"),__FUNCTION__, buff.nType, buff.nId);
				return false;
			}
			buff.nGroup = (WORD)getFieldInt("group", &nDef);
			buff.dValue = (double)getFieldNumber("value", &dDef);
			buff.nTimes = (int)getFieldInt("times", &nDef);
			buff.boTimeOverlay = (bool)getFieldBoolean("timeoverlay", &boDef);
			buff.boFullDel = (bool)getFieldBoolean("fullDel", &boDef);
			buff.nInterval = (WORD)getFieldInt("interval", &nDef);
			buff.nDuration = (WORD)getFieldInt("duration", &nDef);
			buff.nParam = (int)getFieldInt("param", &nDef);
			buff.nParam2 = (int)getFieldInt("param2", &nDef);
			buff.nParam3 = (int)getFieldInt("param3", &nDef);
			buff.nEffectType = (int)getFieldInt("effectType", &nDef);
			buff.nEffectId = (int)getFieldInt("effectId", &nDef);
			buff.nEffectTotalCount = (int)getFieldInt("effecttotalcount", &nDef);
			buff.nEffectDirCount = (int)getFieldInt("effectdircount", &nDef);
			buff.nDebuff = (int)getFieldInt("debuff", &nDef);
			buff.btIcon = (byte)getFieldInt("icon", &nDef);
			buff.nPropColor = (int)getFieldInt("colorPriority", &nDef);
			buff.nColor = (int)getFieldInt("color", &nDef);
			buff.btDisplay = (byte)getFieldInt("display", &nDef);
			buff.boSave = (bool)getFieldBoolean("issave", &boDef);
			getFieldStringBuffer("name", buff.sName, ArrayCount(buff.sName));

			buff.btDelEvent = (byte)getFieldInt("delEvent", &nDef);
			buff.boDieRemove = (bool)getFieldBoolean("dieremove", &boDef);
			buff.boDelGiverByLeaveGuild = getFieldBoolean("delGiverByLeaveGuild",&boDef);

			m_typeConfMap[buff.nType].push(&buff);

			if (buff.nType == aNextSkillFlag)
			{
				int nskillid = buff.dValue;
				m_SkillBuffMap[nskillid] = &buff;
			}
			
		} while (enumTableNext());
	}
	closeTable();
	return true;
}
