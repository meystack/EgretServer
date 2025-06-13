#pragma once
#include "StdAfx.h"
#include "LogicDataProvider.h"

/*
 bool CLogicDataProvider:: LoadSkillConfig()
{
	//副本配置表
	/*
	PSKILLVEC pData = GetLogicServer()->GetDataProvider()->GetSkillConfig();
	if (!openGlobalTable("SkillConfig")) 
	{
		return FALSE;
	}
	SKILLINFO oneSkillInfo; //一个技能的配置
	SKILLLEVELINFO oneLevelInfo; //1个等级的信息
	if (enumTableFirst())
	{
		do
		{
			
			
			unsigned id = getFieldInt("id"); //技能ID
			
			INT_PTR nSkillType =getFieldInt("skillType"); //技能类型 
			//哪些职业能够使用
			
			if ( openFieldTable("vocationFlags") ==false)
			{
				return false;
			}
			
			INT_PTR nVocationMask =0;
			INT_PTR j =0;
			closeTable();
			if(nVocationMask )
			{
				oneSkillInfo.nVocationMask =nVocationMask;
			}
			else
			{
				oneSkillInfo.nVocationMask = ~0; //什么职业都能使用
			}
			oneSkillInfo.nSkillType =(BYTE) nSkillType;

			if ( openFieldTable("skillSubLevel") ==false)
			{
				return false;
			}
			if (enumTableFirst())
			{
				
				do
				{
					
					if ( openFieldTable("spellResult") ==false)
					{
						return false;
					}
					if (enumTableFirst())
					{
						do
						{

						}while (enumTableNext());
					}
					

				} while (enumTableNext());
				closeTable();

			}
		}
	}
	
	return true;
};

*/