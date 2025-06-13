#include "StdAfx.h"
#include "LogicDataProvider.h"
#include "../base/Container.hpp"
 
#include "HallowsConfig.h"

const TCHAR CLogicDataProvider::IncludeFileName[] = _T("data/DataConfig.txt");
const TCHAR CLogicDataProvider::FileDirName[] = _T("data");

CLogicDataProvider::CLogicDataProvider()
	:Inherited(), 
	m_pHallowsthenConfig(NULL), 
	m_DataAllocator(_T("LogicDataProviderAlloc"))
{
	m_pLogicServer = GetLogicServer();
	m_pLogicEngine = m_pLogicServer->GetLogicEngine();
 
	m_pHallowsthenConfig = new CHallowsthenCfg();
}

CLogicDataProvider::~CLogicDataProvider()
{ 
	delete m_pHallowsthenConfig;
	m_pHallowsthenConfig = NULL;
} 
bool CLogicDataProvider::LoadHallowsConfig(LPCTSTR sFilePath)
{ 
	return m_pHallowsthenConfig->LoadAll(sFilePath); 
}  

bool CLogicDataProvider::LoadConfig()
{
	LPCTSTR sResult = NULL;
	wylib::stream::CMemoryStream ms;
	
	CCustomLuaPreProcessor pp;
	pp.addIncludeDirectory(_T(FileDirName));
	GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);

	//预处理器定义为了加快脚本读取，会注册语言包已加载的宏，此处应当去掉，否则无法读取TipMsg
	pp.removeMacro("_LANGUAGE_INCLUDED_");	

	if ( ms.loadFromFile(_T(IncludeFileName)) > 0 )
	{
		sResult = pp.parse((LPCTSTR)ms.getMemory(), _T(IncludeFileName));
	}
	if (sResult == NULL)
	{
		OutputMsg(rmError,_T("preprocess file %s fail"),IncludeFileName);
		return false;
	}
	if ( !setScript(sResult) )
	{
		OutputMsg(rmError,_T("preprocess files fail"));
		return false;
	}

	if ( !LoadConfigTerms(LoadTermList) )
		return false;
	setScript(NULL);
	OutputMsg(rmTip,_T("DataProvider source load complete!"));
	return true;
}

bool CLogicDataProvider::LoadRunTimeData()  //引擎开启时才加载
{
	for (INT_PTR i = 0; i < RunTimeDataTermCount; ++i)
	{
		if ( RunTimeDataTermList[i].boLoad )
		{
			OutputMsg(rmNormal, _T("loading %s RunTime Data..."), RunTimeDataTermList[i].sTermName);
			if ( !((this->*RunTimeDataTermList[i].loadProc)(RunTimeDataTermList[i].sFilePath)) )
			{
				OutputMsg(rmError, _T("Error loading RunTimeData %s"), RunTimeDataTermList[i].sTermName);
				//return false;
			}
			else
			{
				OutputMsg(rmTip, _T("load %s RunTime Data complete"), RunTimeDataTermList[i].sTermName);
			}
			
		}
	}
	return true;
}

bool CLogicDataProvider::SaveRunTimeData()
{
	for (INT_PTR i = 0; i < RunTimeDataTermCount; ++i)
	{
		if ( RunTimeDataTermList[i].boLoad )
		{
			OutputMsg(rmNormal, _T("saving %s RunTime Data..."), RunTimeDataTermList[i].sTermName);
			if ( !((this->*RunTimeDataTermList[i].saveProc)(RunTimeDataTermList[i].sFilePath)) )
			{
				OutputMsg(rmError, _T("Error saving RunTimeData %s"), RunTimeDataTermList[i].sTermName);
				return false;
			}
			OutputMsg(rmTip, _T("save %s RunTime Data complete"), RunTimeDataTermList[i].sTermName);
		}
	}
	return true;
}


bool CLogicDataProvider::ReadAttributeTable(CObjectAllocator<char> &dataAllocator, CStdItem::AttributeGroup *pAttrGroup)
{
	PGAMEATTR pAttr;
	pAttrGroup->nCount = lua_objlen(m_pLua, -1);

	if (enumTableFirst())
	{
		pAttr = pAttrGroup->pAttrs = (PGAMEATTR)dataAllocator.allocObjects(sizeof(*(pAttrGroup->pAttrs)) * pAttrGroup->nCount);
		do 
		{
			pAttr->type = getFieldInt("type");
			//判断属性类型是否有效
			if ( pAttr->type < aUndefined || pAttr->type >= GameAttributeCount )
			{
				showErrorFormat(_T("attr type config error %d"),pAttr->type);
			}
			switch(AttrDataTypes[pAttr->type])
			{
			case adSmall:
			case adShort:
			case adInt:
				pAttr->value.nValue = getFieldInt("value");
				break;
			case adUSmall:
			case adUShort:
			case adUInt:
				pAttr->value.uValue = (UINT)getFieldInt64("value");
				break;
			case adFloat:
				pAttr->value.fValue = (float)getFieldNumber("value");
				break;
			}
			pAttr++;
		}
		while (enumTableNext());
	}
	return true;
}

bool CLogicDataProvider::LoadConfigTerms(const CLogicDataProvider::DataProviderLoadTerm *pTermList, const INT_PTR nTermCount)
{
	for (INT_PTR i = 0; i < nTermCount; ++i)
	{
		if ( pTermList[i].boLoad )
		{
			OutputMsg(rmNormal, _T("loading %s..."), pTermList[i].sTermName);
			if ( !((this->*pTermList[i].loadProc)(pTermList[i].sFilePath)) )
			{
				OutputMsg(rmError, _T("Error loading %s"), pTermList[i].sTermName);
				return false;
			}
			OutputMsg(rmTip, _T("load %s complete"), pTermList[i].sTermName);
		}
	}
	return true;
}

void CLogicDataProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	//RefString s = _T("[Config Error]");
	//s += sError;
	OutputMsg(rmError,"%s",sError);
	//throw s;
}

bool CLogicDataProvider::ReloadConfig(char * sConfigDesc)
{
	LPCTSTR sResult = NULL;
	wylib::stream::CMemoryStream ms;

	CCustomLuaPreProcessor pp;
	pp.addIncludeDirectory(_T(FileDirName));
	GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);

	//预处理器定义为了加快脚本读取，会注册语言包已加载的宏，此处应当去掉，否则无法读取TipMsg
	pp.removeMacro("_LANGUAGE_INCLUDED_");	

	if ( ms.loadFromFile(_T(IncludeFileName)) > 0 )
	{
		sResult = pp.parse((LPCTSTR)ms.getMemory(), _T(IncludeFileName));
	}
	if (sResult == NULL)
	{
		OutputMsg(rmError,_T("preprocess file %s fail"),IncludeFileName);
		return false;
	}
	if ( !setScript(sResult) )
	{
		OutputMsg(rmError,_T("preprocess files fail"));
		return false;
	}
	for (INT_PTR i = 0; i < LoadTermCount; ++i)
	{
		if (_stricmp(LoadTermList[i].sTermName,sConfigDesc) ==0  ) //如果找到了这个
		{
			OutputMsg(rmNormal, _T("loading %s..."), LoadTermList[i].sTermName);
			if ( !((this->*LoadTermList[i].loadProc)(LoadTermList[i].sFilePath)) )
			{
				OutputMsg(rmError, _T("Error loading %s"), LoadTermList[i].sTermName);
				return false;
			}
			OutputMsg(rmTip, _T("load %s complete"), LoadTermList[i].sTermName);
			return true;
		}
	}
	return false;
}

bool CLogicDataProvider::ReloadConfig(INT_PTR nConfigId)
{
	LPCTSTR sResult = NULL;
	wylib::stream::CMemoryStream ms;

	CCustomLuaPreProcessor pp;
	pp.addIncludeDirectory(_T(FileDirName));
	GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);

	//预处理器定义为了加快脚本读取，会注册语言包已加载的宏，此处应当去掉，否则无法读取TipMsg
	pp.removeMacro("_LANGUAGE_INCLUDED_");	

	if ( ms.loadFromFile(_T(IncludeFileName)) > 0 )
	{
		sResult = pp.parse((LPCTSTR)ms.getMemory(), _T(IncludeFileName));
	}
	if (sResult == NULL)
	{
		OutputMsg(rmError,_T("preprocess file %s fail"),IncludeFileName);
		return false;
	}
	if ( !setScript(sResult) )
	{
		OutputMsg(rmError,_T("preprocess files fail"));
		return false;
	}
	for (INT_PTR i = 0; i < LoadTermCount; ++i)
	{
		if ( LoadTermList[i].cdType == nConfigId ) //如果找到了这个
		{
			OutputMsg(rmNormal, _T("loading %s..."), LoadTermList[i].sTermName);
			if ( !((this->*LoadTermList[i].loadProc)(LoadTermList[i].sFilePath)) )
			{
				OutputMsg(rmError, _T("Error loading %s"), LoadTermList[i].sTermName);
				return false;
			}
			OutputMsg(rmTip, _T("load %s complete"),  LoadTermList[i].sTermName);
			return true;
		}
	}
	return false;
}

bool CLogicDataProvider::LoadChangeNameList()
{
	if (openGlobalTable("ChangeNameConfig"))
	{
		m_ChangeNameList.nNum = (int)lua_objlen(m_pLua, -1);
		if (m_ChangeNameList.nNum > 0)
		{
			int nSize = sizeof(LPSTR) * m_ChangeNameList.nNum;
			m_ChangeNameList.pNameList = (LPSTR*)m_DataAllocator.allocObjects(nSize);
			ZeroMemory(m_ChangeNameList.pNameList,nSize);
		}

		int nIdx = 0;
		if (enumTableFirst())
		{
			do 
			{
				nIdx = getFieldInt("idx") - 1;
				m_ChangeNameList.pNameList[nIdx] = (LPSTR)m_DataAllocator.allocObjects(64);
				LPSTR pName = m_ChangeNameList.pNameList[nIdx];
				getFieldStringBuffer("name",pName,64);
				strcat(pName,"\\");
			} while (enumTableNext());
		}
		closeTable();
	}
	return true;
}

bool CLogicDataProvider::LoadChangeNameConfig(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	LPCTSTR sText;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
		{
			showError(_T("syntax error on LoadGlobalConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadChangeNameList();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Guild config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Guild config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CLogicDataProvider::LoadFirstLogin()
{
	int nDef0 = 0;
	if (openGlobalTable("FirstLoginConf"))
	{
		// 默认技能按键
		if (openFieldTable("gameKeyList"))
		{
			if (enumTableFirst())
			{
				do 
				{
					m_FirstLoginKeySet.push_back(getFieldInt(NULL, &nDef0));
				} while (enumTableNext());
			}
			closeTable();
		}

		// 默认穿戴装备
		if (openFieldTable("dressEquip"))
		{
			if (enumTableFirst())
			{
				do 
				{
					m_FirstLoginDress.push_back(getFieldInt(NULL, &nDef0));
				} while (enumTableNext());
			}
			closeTable();
		}

		// 默认赠送物品
		if (openFieldTable("warriorItemList"))
		{
			if (enumTableFirst())
			{
				do 
				{
					ItemAward one;
					one.nType = getFieldInt("type");
					one.nId = getFieldInt("id");
					one.nCount = getFieldInt("count");
					m_WarriorFirstLoginAwards.push_back(one);
				} while (enumTableNext());
			}
			closeTable();
		}
		if (openFieldTable("magicianItemList"))
		{
			if (enumTableFirst())
			{
				do 
				{
					ItemAward one;
					one.nType = getFieldInt("type");
					one.nId = getFieldInt("id");
					one.nCount = getFieldInt("count");
					m_MagicianFirstLoginAwards.push_back(one);
				} while (enumTableNext());
			}
			closeTable();
		}
		if (openFieldTable("wizardItemList"))
		{
			if (enumTableFirst())
			{
				do 
				{
					ItemAward one;
					one.nType = getFieldInt("type");
					one.nId = getFieldInt("id");
					one.nCount = getFieldInt("count");
					m_WizardFirstLoginAwards.push_back(one);
				} while (enumTableNext());
			}
			closeTable();
		}
	}
	return true;
}

bool CLogicDataProvider::LoadFirstLoginConfig(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	LPCTSTR sText;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
		{
			showError(_T("syntax error on LoadGlobalConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadFirstLogin();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Guild config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Guild config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
