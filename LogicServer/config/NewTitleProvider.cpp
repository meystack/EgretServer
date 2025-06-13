#include "StdAfx.h"
#include "NewTitleProvider.h"

CNewTitleProvider::CNewTitleProvider()
	: Inherited(), m_DataAllocator(_T("NewTitleAlloc")), m_CustomTitlesDataAllocator(_T("CustomTitleAlloc"))
{
}


CNewTitleProvider::~CNewTitleProvider()
{
}

bool CNewTitleProvider::LoadNewTitleConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on NewTitle config"));
		Result = readNewTitleConfigs();

		if ( Result )
		{
			readmCustomTitleConfigs();
		}
		
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load NewTitle config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load NewTitle config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CNewTitleProvider::readNewTitleConfigs()
{
	if ( !openGlobalTable("TitleConfig") )
		return false;
	CDataAllocator dataAllocator;
	size_t nConfigCount = lua_objlen(m_pLua,-1);
	if (enumTableFirst())
	{
		bool boDefault = false;
		int nDefInt = 0;
		int nIdx = 0;
		m_NewTitlesCount = (int)nConfigCount;
		m_NewTitlesConfig = (OneNewTitleConfig*)dataAllocator.allocObjects(sizeof(OneNewTitleConfig)*nConfigCount);
		int bInt = 0;
		do 
		{
			OneNewTitleConfig &item = m_NewTitlesConfig[nIdx];
			item.nNewTitleId = getFieldInt("id");
			item.nNewTitleTime = getFieldInt("titleTime");
			getFieldStringBuffer("titleName",item.name,sizeof(item.name) );
			item.bBroadcast = getFieldBoolean("broadcast",&boDefault);
			item.bAutoUse = getFieldInt("active", &nDefInt);
			item.nVIPnotice = getFieldInt("VIPnotice", &nDefInt);
			item.nNoticeLv = getFieldInt("noticeLv", &nDefInt);
			item.nOfflieDelete = getFieldInt("offlinedelete", &nDefInt);
			item.bCrossServerHide = getFieldInt("displayType", &nDefInt);
			if (feildTableExists("attr") && openFieldTable("attr"))
			{
				size_t nCount = lua_objlen(m_pLua,-1);
				if ( enumTableFirst() )
				{
					INT_PTR nIndex = 0;
					item.nPropCount = (int)nCount;
					item.PropList = (GAMEATTR*)dataAllocator.allocObjects(sizeof(GAMEATTR)*nCount);
					do 
					{
						GAMEATTR& tmpTitle = item.PropList[nIndex];
						tmpTitle.type = getFieldInt("type");
						switch(AttrDataTypes[tmpTitle.type])
						{
						case adSmall:
						case adShort:
						case adInt:
							tmpTitle.value.nValue = getFieldInt("value");
							break;
						case adUSmall:
						case adUShort:
						case adUInt:
							tmpTitle.value.uValue = (UINT)getFieldInt64("value");
							break;
						case adFloat:
							tmpTitle.value.fValue = (float)getFieldNumber("value");
							break;
						}
						nIndex++;
					} while (enumTableNext());
				}
				closeTable();
			}
			
			if (feildTableExists("Conditions") && openFieldTable("Conditions"))
			{
				int nCount = lua_objlen(m_pLua,-1);
				if ( enumTableFirst() )
				{
					INT_PTR nIndex = 0;
					item.nConditionCount = (int)nCount;
					item.ConditionList = (GAMEATTR*)dataAllocator.allocObjects(sizeof(GAMEATTR)*nCount);
					do 
					{
						GAMEATTR& tmpTitle = item.ConditionList[nIndex];
						tmpTitle.type = getFieldInt("type");
						switch(AttrDataTypes[tmpTitle.type])
						{
						case adSmall:
						case adShort:
						case adInt:
							tmpTitle.value.nValue = getFieldInt("value");
							break;
						case adUSmall:
						case adUShort:
						case adUInt:
							tmpTitle.value.uValue = (UINT)getFieldInt64("value");
							break;
						case adFloat:
							tmpTitle.value.fValue = (float)getFieldNumber("value");
							break;
						}
						nIndex++;
					} while (enumTableNext());
				}
				closeTable();
			}
			
			nIdx++;
		} while (enumTableNext());
	}
	closeTable();
	m_DataAllocator.~CObjectAllocator();
	m_DataAllocator = dataAllocator;
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	
	return true;
}

bool CNewTitleProvider::readmCustomTitleConfigs()
{
	if ( !openGlobalTable("CustomisedTitleConfig") )
		return false;
	CDataAllocator dataAllocator;
	size_t nConfigCount = lua_objlen(m_pLua,-1);
	if (enumTableFirst())
	{
		bool boDefault = false;
		int nDefInt = 0;
		int nIdx = 0;
		m_CustomTitlesCount = (int)nConfigCount;
		m_CustomTitlesConfig = (OneNewTitleConfig*)dataAllocator.allocObjects(sizeof(OneNewTitleConfig)*nConfigCount);
		int bInt = 0;
		do 
		{
			OneNewTitleConfig &item = m_CustomTitlesConfig[nIdx];
			item.nNewTitleId = getFieldInt("id");
			item.nNewTitleTime = getFieldInt("titleTime");
			getFieldStringBuffer("titleName",item.name,sizeof(item.name) );
			item.bBroadcast = getFieldBoolean("broadcast",&boDefault);
			item.bAutoUse = getFieldInt("active", &nDefInt);
			item.nVIPnotice = getFieldInt("VIPnotice", &nDefInt);
			item.nNoticeLv = getFieldInt("noticeLv", &nDefInt);
			item.nOfflieDelete = getFieldInt("offlinedelete", &nDefInt);
			item.bCrossServerHide = getFieldInt("displayType", &nDefInt);
			if (feildTableExists("attr") && openFieldTable("attr"))
			{
				size_t nCount = lua_objlen(m_pLua,-1);
				if ( enumTableFirst() )
				{
					INT_PTR nIndex = 0;
					item.nPropCount = (int)nCount;
					item.PropList = (GAMEATTR*)dataAllocator.allocObjects(sizeof(GAMEATTR)*nCount);
					do 
					{
						GAMEATTR& tmpTitle = item.PropList[nIndex];
						tmpTitle.type = getFieldInt("type");
						switch(AttrDataTypes[tmpTitle.type])
						{
						case adSmall:
						case adShort:
						case adInt:
							tmpTitle.value.nValue = getFieldInt("value");
							break;
						case adUSmall:
						case adUShort:
						case adUInt:
							tmpTitle.value.uValue = (UINT)getFieldInt64("value");
							break;
						case adFloat:
							tmpTitle.value.fValue = (float)getFieldNumber("value");
							break;
						}
						nIndex++;
					} while (enumTableNext());
				}
				closeTable();
			}
			
			if (feildTableExists("Conditions") && openFieldTable("Conditions"))
			{
				int nCount = lua_objlen(m_pLua,-1);
				if ( enumTableFirst() )
				{
					INT_PTR nIndex = 0;
					item.nConditionCount = (int)nCount;
					item.ConditionList = (GAMEATTR*)dataAllocator.allocObjects(sizeof(GAMEATTR)*nCount);
					do 
					{
						GAMEATTR& tmpTitle = item.ConditionList[nIndex];
						tmpTitle.type = getFieldInt("type");
						switch(AttrDataTypes[tmpTitle.type])
						{
						case adSmall:
						case adShort:
						case adInt:
							tmpTitle.value.nValue = getFieldInt("value");
							break;
						case adUSmall:
						case adUShort:
						case adUInt:
							tmpTitle.value.uValue = (UINT)getFieldInt64("value");
							break;
						case adFloat:
							tmpTitle.value.fValue = (float)getFieldNumber("value");
							break;
						}
						nIndex++;
					} while (enumTableNext());
				}
				closeTable();
			}
			
			nIdx++;
		} while (enumTableNext());
	}
	closeTable();
	m_CustomTitlesDataAllocator.~CObjectAllocator();
	m_CustomTitlesDataAllocator = dataAllocator;
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	
	return true;
}

void CNewTitleProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[InVestMentProvider]");
	s += sError;
	throw s;
}

OneNewTitleConfig* CNewTitleProvider::GetNetTitleConfig(WORD wId)const
{
	for (int i = 0; i < m_NewTitlesCount; i++)
	{
		if (wId == m_NewTitlesConfig[i].nNewTitleId)
		{
			return &m_NewTitlesConfig[i];
		}
	}
	return NULL;
}

bool CNewTitleProvider::IsTimeNewTitle( WORD wId )
{
	for (int i=0; i< m_NewTitlesCount; i++)
	{
		if (wId == m_NewTitlesConfig[i].nNewTitleId)
		{
			return m_NewTitlesConfig[i].nNewTitleTime > 0;
		}
	}
	return false;
}

OneNewTitleConfig* CNewTitleProvider::GetCustomTitleConfig(WORD wId)const
{
	for (int i = 0; i < m_CustomTitlesCount; i++)
	{
		if (wId == m_CustomTitlesConfig[i].nNewTitleId)
		{
			return &m_CustomTitlesConfig[i];
		}
	}
	return NULL;
}

bool CNewTitleProvider::IsTimeCustomTitle( WORD wId )
{
	for (int i=0; i< m_CustomTitlesCount; i++)
	{
		if (wId == m_CustomTitlesConfig[i].nNewTitleId)
		{
			return m_CustomTitlesConfig[i].nNewTitleTime > 0;
		}
	}
	return false;
}
