#include "StdAfx.h"
#include "ResonanceItemProvider.h"

CResonanceItemProvider::CResonanceItemProvider()
	: Inherited(),m_DataAllocator(_T("ResonanceItemAlloc"))
{
}

CResonanceItemProvider::~CResonanceItemProvider()
{
}


bool CResonanceItemProvider::Load(LPCTSTR sFilePath)
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
			showError(_T("syntax error on LootPet config"));
		Result = ReadAllConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LootPet config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LootPet config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CResonanceItemProvider::ReadAllConfig()
{
	if ( !openGlobalTable("ResonanceItemCfg") )
		return false;
	
	//获取配置中的技能数量
	int nCount = (int)lua_objlen(m_pLua,-1);

	// //如果物品数量为0则输出未配置任何物品的错误
	// if ( nCount <= 0 )
	// {
	// 	showError(_T("no ResonanceItem data on ResonanceItem config"));
	// 	return false;
	// }

	//循环读取技能配置数据
	if ( enumTableFirst() )
	{
		int nDefInt = 0;
		do 
		{
			int nResonanceId = getFieldInt("Resonanceid",&nDefInt);
			if(feildTableExists("attr") && openFieldTable("attr"))
            {
                if(enumTableFirst())
                {  
                    do
                    {
						RESONANCEITEMCONFIG item;
						item.nItemId = getFieldInt("itemid",&nDefInt);
						if(feildTableExists("attr") && openFieldTable("attr"))
						{
							int  nCount = (int)lua_objlen(m_pLua,-1);
							PGAMEATTR pAttr = NULL;
							item.attri.nCount = nCount;
							if(nCount > 0)
							{
								item.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
								ZeroMemory(item.attri.pAttrs, sizeof(GAMEATTR) * nCount);
								pAttr = item.attri.pAttrs;
							}
							if(enumTableFirst())
							{
								if(!LoadAttri(pAttr))
								{
									return false;
								}
								pAttr++;
							}
							closeTable();
						}
						m_ResonanceItems[nResonanceId][item.nItemId] = item.attri;
					}while (enumTableNext());
				}
				closeTable();
			}
		}while (enumTableNext());
	}
	closeTable();
	return true;
}
