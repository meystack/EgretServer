#include "StdAfx.h"
#include "ReviveDurationProvider.h"

CReviveDurationProvider::CReviveDurationProvider()
	: Inherited(), m_DataAllocator(_T("ReviveDurationAlloc"))
{
}


CReviveDurationProvider::~CReviveDurationProvider()
{
}

bool CReviveDurationProvider::LoadReviveDurationConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on ReviveDuration config"));
		Result = readConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load ReviveDuration config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load ReviveDuration config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CReviveDurationProvider::readConfigs()
{
	if ( !openGlobalTable("ReviveDurationConfig") )
		return false;
	CDataAllocator dataAllocator;
	size_t nConfigCount = lua_objlen(m_pLua,-1);
	if (enumTableFirst())
	{
		int nDefInt = 0;
		int nIdx = 0;
		m_ReviveDurationsCount = (int)nConfigCount;
		m_ReviveDurationsConfig = (OneReviveDurationConfig*)dataAllocator.allocObjects(sizeof(OneReviveDurationConfig)*nConfigCount);
		int bInt = 0;
		do 
		{
			OneReviveDurationConfig &item = m_ReviveDurationsConfig[nIdx];
			item.nReviveDurationId = getFieldInt("id", &nDefInt);
			item.nReviveDurationTime = getFieldInt("reviveduration", &nDefInt);
			getFieldStringBuffer(("tips"), item.sTip, sizeof(item.sTip));
			nIdx++;
		} while (enumTableNext());
	}
	closeTable();
	m_DataAllocator.~CObjectAllocator();
	m_DataAllocator = dataAllocator;
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	
	return true;
}

void CReviveDurationProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[InVestMentProvider]");
	s += sError;
	throw s;
}

OneReviveDurationConfig* CReviveDurationProvider::GetReviveDurationConfig(WORD wId) const
{
	for (int i = 0; i < m_ReviveDurationsCount; i++)
	{
		if (wId == m_ReviveDurationsConfig[i].nReviveDurationId)
		{
			return &m_ReviveDurationsConfig[i];
		}
	}
	return NULL;
}

bool CReviveDurationProvider::IsTimeReviveDuration( WORD wId )
{
	for (int i=0; i< m_ReviveDurationsCount; i++)
	{
		if (wId == m_ReviveDurationsConfig[i].nReviveDurationId)
		{
			return m_ReviveDurationsConfig[i].nReviveDurationTime > 0;
		}
	}
	return false;
}
