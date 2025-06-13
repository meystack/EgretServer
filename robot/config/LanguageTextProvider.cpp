#include "stdafx.h"
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
		GetRobotMgr()->GetVSPDefine().RegisteToPreprocessor(pp);

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
		if (openGlobalTable("Lang"))
		{
			clear();
			boResult = readLanguagePacket(*this);
			if(boResult)
			{
				m_LogicDataAllocator.~CObjectAllocator(); //将申请的内容释放掉
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
