#pragma once
#include "StdAfx.h"
//#include "LogicDataProvider.h"
using namespace wylib::stream;


bool CLogicDataProvider::PreLoadFile(LPCSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("%s,unable to load from %s"), __FUNCTION__, sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);

		//设置脚本内容
		if ( !setScript(sText) )
			showErrorFormat(_T("%s,syntax error on %s config"), __FUNCTION__, sFilePath);
		Result = true;
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load CLogicDataProvider common config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load CLogicDataProvider config"));
	}
	return Result;
}

//读取套装的配置 
// bool CLogicDataProvider::LoadSuitConfig(LPCTSTR sFilePath)
// {
// 	if (!PreLoadFile(sFilePath))
// 	{
// 		return false;
// 	}
// 	try
// 	{
// 		SUITCONFIG& data = GetSuitConfig();
// 		// if (!openGlobalTable("SuitConfig"))
// 		// {
// 		// 	return false;
// 		// }
// 		// closeTable();
// 		//if (feildTableExists("Suit") && openFieldTable("Suit"))
// 		if(!openGlobalTable("Suit"))
// 		{
// 			return false;
// 		}

// 		INT_PTR nCount = lua_objlen(m_pLua, -1);
// 		//如果物品数量为0则输出未配置任何物品的错误
// 		if ( nCount <= 0 )
// 		{
// 			data.count =0;
// 		}
// 		else
// 		{
// 			data.count = nCount;
// 			data.pData =(PONESUIT)m_DataAllocator.allocObjects(nCount * sizeof(ONESUIT));
// 		}

// 		//遍历表
// 		if (enumTableFirst())
// 		{

// 			int nID=0;
// 			PONESUIT pBase =(PONESUIT )data.pData;
// 			do
// 			{
// 				PONESUIT  pSuit= pBase +nID;
// 				pSuit->circle =(WORD)getFieldInt("circle");
// 				pSuit->level =(WORD)getFieldInt("level");
// 				if ( openFieldTable("items") )
// 				{
		
// 					nCount = lua_objlen(m_pLua, -1);
// 					//如果物品数量为0则输出未配置任何物品的错误
// 					if ( nCount <= 0 )
// 					{
// 						pSuit->itmList.count=0;
// 					}
// 					else
// 					{
// 						pSuit->itmList.count=nCount;
// 						pSuit->itmList.pData =(WORD*)m_DataAllocator.allocObjects(nCount * sizeof(WORD));
// 						int j=0;
// 						if (enumTableFirst())
// 						{
// 							do
// 							{
// 								pSuit->itmList[j] =(WORD) getFieldInt(NULL);
// 								j++;
// 							} while (enumTableNext());
// 						}

// 					}
// 					closeTable();
// 				}

// 				if (feildTableExists("ring") && openFieldTable("ring"))
// 				{
// 					INT_PTR count = lua_objlen(m_pLua, -1);
// 					if (count > 0 && enumTableFirst())
// 					{
// 						int nData = 1;
// 						INT_PTR idx = 0;
// 						do 
// 						{
// 							if (idx < 2 )
// 							{
// 								pSuit->ring[idx++] = getFieldInt(NULL, &nData);
// 							}
// 							else
// 							{
// 								endTableEnum();
// 								break;
// 							}
// 						} while (enumTableNext());
// 					}
// 					closeTable();
// 				}

// 				if (feildTableExists("bangle") && openFieldTable("bangle"))
// 				{
// 					INT_PTR count = lua_objlen(m_pLua, -1);
// 					if (count > 0 && enumTableFirst())
// 					{
// 						int nData = 1;
// 						INT_PTR idx = 0;
// 						do 
// 						{
// 							if (idx < 2 )
// 							{
// 								pSuit->bangle[idx++] = getFieldInt(NULL, &nData);
// 							}
// 							else
// 							{
// 								endTableEnum();
// 								break;
// 							}
// 						} while (enumTableNext());
// 					}
// 					closeTable();
// 				}

// 				if ( openFieldTable("attrs") ==false)
// 				{
// 					return false;
// 				}
// 				else
// 				{

// 					nCount = lua_objlen(m_pLua, -1);
// 					//如果物品数量为0则输出未配置任何物品的错误
// 					if ( nCount <= 0 )
// 					{
// 						pSuit->attrs.count=0;
// 					}
// 					else
// 					{
// 						pSuit->attrs.count=nCount;
// 						pSuit->attrs.pData =(PGAMEATTR)m_DataAllocator.allocObjects(nCount * sizeof(GAMEATTR));
// 						int j=0;
// 						if (enumTableFirst())
// 						{
// 							do
// 							{
							
// 								INT_PTR nType= (BYTE) getFieldInt("type");
// 								pSuit->attrs[j].type =(BYTE)nType;

// 								//判断物品属性类型是否有效
// 								if (nType < aUndefined || nType >= GameAttributeCount )
// 								{
// 									showErrorFormat(_T("Suit config errpr nID=%d,type=%d"), nID,nType );
// 									continue;
// 								}
// 								switch(AttrDataTypes[nType])
// 								{
// 								case adSmall:
// 								case adShort:
// 								case adInt:
// 									pSuit->attrs[j].value.nValue = getFieldInt("value");
// 									break;
// 								case adUSmall:
// 								case adUShort:
// 								case adUInt:
// 									pSuit->attrs[j].value.uValue = (UINT)getFieldInt("value");
// 									break;
// 								case adFloat:
// 									pSuit->attrs[j].value.fValue = (float)getFieldNumber("value");
// 									break;
// 								}
// 								pSuit->attrs[j].reserve[0] =(BYTE) getFieldInt("need");
// 								j++;
// 							} while (enumTableNext());
// 							//closeTable();
// 						};
// 					}
// 					closeTable();
			
// 				}
// 				nID++;
// 			} while (enumTableNext());
// 		}
// 		closeTable();
// 	}
// 	catch (RefString &s)
// 	{
// 	OutputMsg(rmError, _T("load suit config error: %s"), s.rawStr());
// 	}
// 	catch (...)
// 	{
// 	OutputMsg(rmError, _T("unexpected error on load LoadSuitConfig config"));
// 	}
// 	return true;
// };

