#pragma once
#include "StdAfx.h"

#include "ScriptTimerConfig.h"
using namespace wylib::stream;

bool CScriptTimerConfig::LoadScriptTimerData(LPCTSTR sFilePath)
{
	//return InheritedReader::LoadFile(this,sFilePath,&CScriptTimerConfig::ReadAllScriptData);
	if( Inherited2::LoadFile(sFilePath) )
	{
		bool r= ReadAllScriptData();
		setScript(NULL);
		return r;

	}
	else
	{
		return false;
	}
}

void CScriptTimerConfig::ReadTable(DataList<int> &intList, char * tableName)
{
	if ( feildTableExists(tableName) && openFieldTable(tableName) )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		if ( nCount <= 0 )
		{
			intList.count =0; //没有技能
		}
		else
		{
			intList.count =nCount;
			intList.pData =(int*) m_DataAllocator.allocObjects( sizeof(int) *nCount );
			INT_PTR nId=0;
			if (enumTableFirst())
			{
				do
				{
					int * pData = (int*)(intList.pData + nId);
					*pData = getFieldInt(NULL);
					nId ++;	
				}while(enumTableNext());
			}

		}
		closeTable();
	}
	else
	{
		intList.count =0;
	}
}
bool CScriptTimerConfig::ReadAllScriptData()
{
	if ( !openGlobalTable("TimerConfig") )
		return false;
	if(count() >0)
	{
		Inherited::clear(); //先清除
	}
	if (enumTableFirst())
	{

		do 
		{
			int nDef =0;
			SCRIPTTIMER timerData;
			memset(&timerData,0,sizeof(timerData));

			//读取表格
			ReadTable(timerData.months,"month");
			ReadTable(timerData.days,"day");

			ReadTable(timerData.hours,"hour");
			ReadTable(timerData.weeks,"week");
			ReadTable(timerData.minutes,"minute");
			ReadTable(timerData.serverday,"openserverdays");
			ReadTable(timerData.notopenserverdays,"notopenserverdays");
			ReadTable(timerData.combineserverday,"combineserverdays");
			ReadTable(timerData.notCombineserverday,"notcombineserverdays");
			ReadTable(timerData.beforeCombineday,"beforeCombineday");
			ReadTable(timerData.notBeforeCombineday,"notBeforeCombineday");
			timerData.nNpcId = getFieldInt("npcid", &nDef);
			//getFieldStringBuffer("npcScene",timerData.sSceneName ,ArrayCount(timerData.sSceneName));
			//getFieldStringBuffer("npcName",timerData.sNpcName ,ArrayCount(timerData.sNpcName));
			getFieldStringBuffer("npcFunc",timerData.sFuncName ,ArrayCount(timerData.sFuncName));
			add(timerData);
		}
		while (enumTableNext());
	}

	closeTable();
	return true;
}


