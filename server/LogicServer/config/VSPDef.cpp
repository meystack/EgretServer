#include "StdAfx.h"
#include "VSPDef.h"

LPCTSTR CVSPDefinition::SPID			= _T("SPID");
LPCTSTR CVSPDefinition::LANGUAGE		= _T("LANGUAGE");
LPCTSTR CVSPDefinition::GAMENAME		= _T("GAMENAME");
LPCTSTR CVSPDefinition::BINDCOINNAME	= _T("BINDCOINNAME");
LPCTSTR CVSPDefinition::COINNAME		= _T("COINNAME");
LPCTSTR CVSPDefinition::BINDYBNAME		= _T("BINDYBNAME");
LPCTSTR CVSPDefinition::YBNAME			= _T("YBNAME");

CVSPDefinition::CVSPDefinition()
{

}

CVSPDefinition::~CVSPDefinition()
{
	ClearDefinitions();
}

LPCTSTR CVSPDefinition::GetDefinition(LPCTSTR sDefName) const
{
	INT_PTR i, nCount = m_Defines.count();
	for (i=0; i<nCount; ++i)
	{
		if (_tcscmp(m_Defines[i].sDefName, sDefName) == 0)
		{
			return m_Defines[i].sDefValue;
		}
	}
	return NULL;
}

VOID CVSPDefinition::RegisteToPreprocessor(CCustomLuaPreProcessor &preProcessor)
{
	INT_PTR i, nCount = m_Defines.count();
	for (i=0; i<nCount; ++i)
	{
		preProcessor.addMacro(m_Defines[i].sDefName, m_Defines[i].sDefValue);
	}
	//增加语言包已被加载的标记，语言包已经作为独立的配置读取了，这里注册宏后可以加快脚本读取
	preProcessor.addMacro("_LANGUAGE_INCLUDED_");
	if (GetLogicServer()->IsCommonServer())
		preProcessor.addMacro("_COMMON_SERVER_");
}

#define LN '\n'
bool CVSPDefinition::LoadDefinitions(LPCTSTR sFilePath)
{
	wylib::stream::CMemoryStream ms;
	//加载定义文件
	if ( ms.loadFromFile(sFilePath) <= 0 )
	{
		OutputError(GetLastError(), "unable to load Definitions from %s ", sFilePath);
		return false;
	}

	//清空当前所有定义
	ClearDefinitions();

	//读取定义文件
	LPTSTR sText = (LPTSTR)ms.getMemory();
	LPTSTR sLineEnd, sNewLine, sTag, sPtr;

	//跳过UTF-8 BOM
	if ( (*(PINT)sText & 0x00FFFFFF) == 0xBFBBEF )
		sText += 3;

	while (true)
	{
		//跳过当前行中行首的空白字符
		while (*sText && (unsigned char)*sText <= 0x20 && *sText != LN)
		{
			sText++;
		}
		//字符串是否已经遍历结束
		if (!*sText)
			break;
		//该行如果是空行则换入下一行并继续
		if (*sText == LN)
		{
			sText++;
			continue;
		}
		//确定当前行结束的位置
		sLineEnd = _tcschr(sText, LN);
		if ( !sLineEnd )
			sLineEnd = sText + _tcslen(sText) - 1;

		//跳过当前行中行尾的空白字符并在行尾写入终止符
		sNewLine = sLineEnd + 1;
		while (sText < sLineEnd && (unsigned char)*sLineEnd <= 0x20)
		{
			sLineEnd--;
		}

		if (sLineEnd > sText)
		{
			//在行尾写入终止符
			sLineEnd++;
			sLineEnd[0] = 0;
			//如果该行不是一个注释行，则按定义行分析
			if ( sText[0] != '-' && sText[1] != '-' )
			{
				sTag = _tcschr(sText, '=');
				if ( sTag )
				{
					//跳过定义名称后面的空白字符
					sPtr = sTag - 1;
					while (sPtr > sText && (unsigned char)*sPtr <= 0x20)
					{
						sPtr--;
					}
					sPtr++;
					//如果定义名称有效则设置名称指针
					if ( sPtr > sText )
					{
						sPtr[0] = 0;//在定义名称后面写入终止符
						//跳过定义值前面的空白字符
						sPtr = sTag + 1;
						while (sPtr < sLineEnd && (unsigned char)*sPtr <= 0x20)
						{
							sPtr++;
						}
						//添加此定义
						SetDefinition(sText, sPtr);
					}
				}
			}
		}
		
		sText = sNewLine;
	}
	return true;
}

VOID CVSPDefinition::SetDefinition(LPCTSTR sDefName, LPCTSTR sDefValue)
{
	INT_PTR i, nCount = m_Defines.count();
	SIZE_T nNameLen, nValueLen;
	Definition define;
	
	for (i=0; i<nCount; ++i)
	{
		if (_tcscmp(m_Defines[i].sDefName, sDefName) == 0)
		{
			free(m_Defines[i].sDefName);
			m_Defines.remove(i);
			//如果定义值为空则表示删除定义，否则将终止循环并添加新的定义以实现修改定义值的目的
			if ( !sDefValue )
				return;
			else break;
		}
	}

	nNameLen = _tcslen(sDefName);
	nValueLen = _tcslen(sDefValue);
	//申请定义数据内存
	define.sDefName = (LPTSTR)malloc(sizeof(*sDefName) * (nNameLen + nValueLen + 2));
	//拷贝定义名称
	memcpy(define.sDefName, sDefName, sizeof(*sDefName) * nNameLen);
	define.sDefName[nNameLen] = 0;
	//拷贝定义值
	define.sDefValue = define.sDefName + nNameLen + 1;
	memcpy(define.sDefValue, sDefValue, sizeof(*sDefName) * nValueLen);
	define.sDefValue[nValueLen] = 0;
	//添加为新定义
	m_Defines.add(define);
}

VOID CVSPDefinition::ClearDefinitions()
{
	INT_PTR i, nCount = m_Defines.count();
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif
#endif
	{
		for (i=0; i<nCount; ++i)
		{
			free(m_Defines[i].sDefName);
		}
	}
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
#endif
#endif
	{

	}
	m_Defines.empty();
}