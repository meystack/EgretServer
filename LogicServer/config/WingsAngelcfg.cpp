#include "StdAfx.h"
#include "WingsAngelcfg.h"

CWingsAngel::CWingsAngel()
	:Inherited(),m_NewDataAllocator(_T("WingsAngelDataAlloc"))
{
}

CWingsAngel::~CWingsAngel()
{
}

bool CWingsAngel::LoadConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on RideGame config"));
		//读取标准物品配置数据
		Result = readConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load RideGame config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load RideGame config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CWingsAngel::readConfigs()
{
	if ( !openGlobalTable("WingCfg") )
		return FALSE;

	if (!openFieldTable("level"))	
	{
		return FALSE;
	}

	size_t nLvCount = lua_objlen(m_pLua,-1);
	CDataAllocator dataAllocator;
	m_Config.nCount = (int)nLvCount;
	m_Config.nWing = (tagLevelConfig*)dataAllocator.allocObjects(sizeof(tagLevelConfig)*nLvCount);

	if ( enumTableFirst() )
	{
		int nIdx = 1;
		do 
		{
			tagLevelConfig &tempStar = m_Config.nWing[nIdx-1];
			tempStar.nLuck = getFieldInt("LuckValue");
			nIdx++;
		}
		while(enumTableNext());
	}
	closeTable();

	if (!openFieldTable("basePropConfig"))
	{
		return FALSE;
	}
	size_t nCount = lua_objlen(m_pLua,-1);
	if(nLvCount != nCount)
	{
		return FALSE;
	}

	if ( enumTableFirst() )
	{
		int nIdx = 1;
		do 
		{
			tagLevelConfig &tempStar = m_Config.nWing[nIdx-1];
			size_t sCount = lua_objlen(m_pLua,-1);
			tempStar.nLevel = nIdx;
			tempStar.nCount = (int)sCount;
			if ( enumTableFirst() )
			{
				INT_PTR sIndex = 1;
				tempStar.PropList = (GAMEATTR*)dataAllocator.allocObjects(sizeof(GAMEATTR)*sCount);
				do 
				{
					GAMEATTR &tmpProp = tempStar.PropList[sIndex-1];
					tmpProp.type = getFieldInt("type");
					tmpProp.reserve[0] = getFieldInt("addmode");
					switch(AttrDataTypes[tmpProp.type])
					{
					case adSmall:
					case adShort:
					case adInt:
						tmpProp.value.nValue = getFieldInt("value");
						break;
					case adUSmall:
					case adUShort:
					case adUInt:
						tmpProp.value.uValue = (UINT)getFieldInt64("value");
						break;
					case adFloat:
						tmpProp.value.fValue = (float)getFieldNumber("value");
						break;
					}
					sIndex++;
				}
				while(enumTableNext());
			} 
			nIdx++;
		}
		while(enumTableNext());
	}
	closeTable();

	//原来的内存块释放到
	m_NewDataAllocator.~CObjectAllocator();
	//将临时的内存块复制到正式的
	m_NewDataAllocator = dataAllocator;
	//清空临时的
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));

	return true;
}
