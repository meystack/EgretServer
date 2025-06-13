#include "StdAfx.h"
#include "LuaConfigBase.h"


CLuaConfigBase::CLuaConfigBase(void)
{
}


CLuaConfigBase::~CLuaConfigBase(void)
{

}

bool CLuaConfigBase::Load(LPCTSTR sFilePath)
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
		{
			char error[MAX_PATH];
			sprintf(error,_T("syntax error on %s"), sFilePath);
			showError(error);
		}
		//读取标准物品配置数据
		Result = ReadAllConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s error: %s"), sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load %s"), sFilePath);
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;
}

bool CLuaConfigBase::ReadIntArray(LPCSTR lpTableName, IntArray& intArray)
{
	if ( feildTableExists(lpTableName) && openFieldTable(lpTableName) )
	{
		intArray.~IntArray();
		intArray.count = (unsigned int)LuaTableLen();

		if (enumTableFirst())
		{
			intArray.pDatas = (int*)GetLogicServer()->GetBuffAllocator()->AllocBuffer(sizeof(int) * intArray.count);
			INT_PTR nIdx = 0;
			do 
			{					
				intArray.pDatas[nIdx++] = getFieldInt(NULL); 
			}while (enumTableNext());
		}
		closeTable(); // lpTableName
		return true;
	}
	return false;
}

bool CLuaConfigBase::LoadAttri(PGAMEATTR pAttr, LPCSTR reserve0Name)
{
	do
	{
		pAttr->type = getFieldInt("type");
		//
		if ( pAttr->type < aUndefined || pAttr->type >= GameAttributeCount )
		{
			showErrorFormat(_T("LoadAttri type config error %d"),pAttr->type);
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
		int nDef =0;
		if (reserve0Name)
		{
			pAttr->reserve[0] = getFieldInt(reserve0Name,&nDef);
		}
		++pAttr;
	}while(enumTableNext());
	return true;
}
