#include "StdAfx.h"
#include "LanguageTextProvider.h"

using namespace wylib::stream;

CLanguageTextProvider::CLanguageTextProvider()
	:Inherited2(),Inherited(),m_TextAllocator(_T("LangDataAlloc"))
{
	m_pAllocator = &m_TextAllocator;
}

CLanguageTextProvider::~CLanguageTextProvider()
{
	CLangTextSection::clear();
}

bool CLanguageTextProvider::LoadFromFile(LPCTSTR sFileName)
{
	bool boResult = false;
	try
	{
		CMemoryStream ms;
		if (ms.loadFromFile(sFileName) <= 0)
		{
			showErrorFormat(_T("unable to load file %s error code %d"), sFileName, GetLastError());
			return false;
		}
		CCustomLuaPreProcessor pp;
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);

		//预处理器定义为了加快脚本读取，会注册语言包已加载的宏，此处应当去掉，否则无法读取语言包
		pp.removeMacro("_LANGUAGE_INCLUDED_");

		LPCSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFileName);
		if (!sText)
		{
			showErrorFormat(_T("unable to pre-precess Language config"));
			return false;
		}
		if (!setScript(sText))
		{
			showErrorFormat(_T("unable to set Language code-text"));
			return false;
		}
		//TO DELETE
		if (globalTableExists("OldLang") && openGlobalTable("OldLang"))
		{
			clear();
			boResult = readLanguagePacket(*this);
			if(boResult)
			{
				//m_LogicDataAllocator.~CObjectAllocator(); //将申请的内容释放掉
				LoadOldLangConfig( GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig(), "System");
				LoadOldLangConfig( GetLogicServer()->GetDataProvider()->GetActorMoodLangConfig(), "ActorMood");
				LoadOldLangConfig( GetLogicServer()->GetDataProvider()->GetMonsterShoutConfig(), "MonserSay");
				//LoadOldLangConfig( GetLogicServer()->GetDataProvider()->GetBigSmileConfig(), "BigSmile");
			}
			closeTable();
			if (boResult)
			{
				OutputMsg(rmTip, _T("[LANGUAGE]Luaguage Load Complete!"));
			}
		}
		//新语言包，替代旧的语言包
		if (globalTableExists("Lang") && openGlobalTable("Lang"))
		{
			clear();
			boResult = readLanguagePacket(*this);
			if(boResult)
			{
				//m_LogicDataAllocator.~CObjectAllocator(); //将申请的内容释放掉 注意：这种做法不安全
				LoadLangConfig( GetLogicServer()->GetDataProvider()->GetTipmsgConfig(), "TipMsg", "Lang");
			}
			closeTable();
			if (boResult)
			{
				OutputMsg(rmTip, _T("[LANGUAGE]Luaguage Load Complete!"));
			}
		}
		
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Language config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Language config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return boResult;
}

/*
	加载语言包的配置
*/
bool CLanguageTextProvider::LoadOldLangConfig(TIPMSGCONFIG & dataConfig,LPCTSTR sTableName,LPCTSTR sLangName)
{
//TIPMSGCONFIG & dataConfig = GetLogicServer()->GetDataProvider()->GetTipmsgConfig();

	if(sTableName ==NULL) return false;

	if (!openGlobalTable("Lang"))
	{
		OutputMsg(rmError,_T("装载系统提示表失败，找不到Global table system"));
		return false;
	}

	if (!openFieldTable(sTableName))
	{
		OutputMsg(rmError,_T("无法打开表%s，初始化失败"),sTableName);
		closeTable();
		return false;
	}

	if(dataConfig.data.count() >0)
	{
		dataConfig.data.clear();
	}
	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	if(nCount  <=0)
	{
		OutputMsg(rmWaning,_T("系统提示系统信息表为空"));
		return true;
	}

	char buff[1024];
	dataConfig.data.reserve(nCount);
	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		//CObjectAllocator<char>	dataAllocator;
		do 
		{
			INT_PTR size= getFieldStringBuffer(NULL,buff,sizeof(buff));
			char * pMemo =m_LogicDataAllocator.allocObjects(size +1);
			memcpy(pMemo,buff,size);
			dataConfig.data.add(pMemo) ;
			nIdx ++;
		}
		while (enumTableNext());
	}
	closeTable();
	return true;
};

bool CLanguageTextProvider::LoadLangConfig(NEWTIPMSGCONFIG & dataConfig,LPCTSTR sTableName,LPCTSTR sLangName)
{
	//TIPMSGCONFIG & dataConfig = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig();

	if(sTableName ==NULL) return false;

	if (!openGlobalTable(sLangName))
	{
		OutputMsg(rmError,_T("装载系统提示表失败，找不到Global table system"));
		return false;
	}

	if (!openFieldTable(sTableName))
	{
		OutputMsg(rmError,_T("无法打开表%s，初始化失败"),sTableName);
		closeTable();
		return false;
	}

	// if(dataConfig.data.count() >0)
	// {
	// 	dataConfig.data.clear();
	// }
	if(dataConfig.data.size() >0)
	{
		dataConfig.data.clear();
	}
	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	if(nCount  <=0)
	{
		OutputMsg(rmWaning,_T("系统提示系统信息表为空"));
		return true;
	}

	
	// dataConfig.data.reserve(nCount);
	// for (size_t i = 1; i <= nCount; i++)
	// {
	// 	INT_PTR size = getIndexStringBuffer(i,buff,sizeof(buff));
	// 	char * pMemo = m_LogicDataAllocator.allocObjects(size +1);
	// 	memcpy(pMemo,buff,size);
	// 	dataConfig.data.add(pMemo);
	// 	//OutputMsg(rmWaning,_T("[%d]提示内容：%s"),i,pMemo);
	// }

	// dataConfig.data.reserve(nCount+1);
	if(enumTableFirst())
	{
		do
		{
			char buff[1024] = {0};
			int id = getFieldInt("id");
			// INT_PTR size = getIndexStringBuffer(i,buff,sizeof(buff));
			getFieldStringBuffer(("tipmsg"), buff,sizeof(buff));
			// char * pMemo = m_LogicDataAllocator.allocObjects(sizeof(buff) +1);
			// memcpy(pMemo,buff,sizeof(buff));
			// dataConfig.data[id] = {0};
			memcpy(dataConfig.data[id-1],buff,sizeof(buff));
		} while (enumTableNext());
		
	}
	
	closeTable();
	closeTable();
	return true;
};

void CLanguageTextProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Language Config Error]");
	s += sError;
	throw s;
}

bool CLanguageTextProvider::readLanguagePacket(CLangTextSection &Section)
{
	//这里无法通过lua_objlen取得表内容数量，因为表可能只有None-Array value。所以可能取不到，应当遍历统计
	int nlen = 0;
	
	lua_pushnil(m_pLua);
	while (lua_next(m_pLua, -2))
	{
		nlen++;
		lua_pop(m_pLua, 1);//remove value
	}

	if (nlen <= 0)
		return true;

	bool result = true;

	Section.setPacket(nlen);
	if (enumTableFirst())
	{
		char sNumKey[64];
		const char* pKey;
		int nKeyType, nValType;
		CLangTextSection *pNewSection;

		do 
		{
			nKeyType = lua_type(m_pLua, -2);
			nValType = lua_type(m_pLua, -1);
			//确定key
			if (nKeyType == LUA_TNUMBER)
			{
				sprintf(sNumKey, "%d", lua_tointeger(m_pLua, -2));
				pKey = sNumKey;
			}
			else pKey = lua_tostring(m_pLua, -2);

			//读取和设置值
			if (nValType == LUA_TSTRING || nValType == LUA_TNUMBER)
			{
				pNewSection = Section.addSection(pKey);
				pNewSection->setText(lua_tostring(m_pLua, -1));
			}
			else if (nValType == LUA_TTABLE)
			{
				pNewSection = Section.addSection(pKey);
				result = readLanguagePacket(*pNewSection);
				if (!result)
				{
					endTableEnum();
					break;
				}
			}
		} 
		while (enumTableNext());
	}
	return result;
}
