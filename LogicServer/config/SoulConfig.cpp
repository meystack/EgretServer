#include "StdAfx.h"
#include "SoulConfig.h"


CSoulConfig::CSoulConfig(void)
{
}


CSoulConfig::~CSoulConfig(void)
{
}


bool CSoulConfig::ReadAllConfig(){
	if ( !openGlobalTable("SoulCfg") )
		return false;

	if ( feildTableExists("prop") && openFieldTable("prop") )
	{
		const INT_PTR nCount = LuaTableLen();	// nCount = #prop

		if ( nCount <= 0 )
		{
			//OutputMsg(rmWaning,_T("no skill level data on Skill config,skillID=%d"),pSkill->nSkillID);
		}
		else
		{
			//pAttr = AttrGroup.pAttrs = (PGAMEATTR)dataAllocator.allocObjects(sizeof(*AttrGroup.pAttrs) * AttrGroup.nCount);

			soulCfg.prop.count = (unsigned int)nCount;
			soulCfg.prop.pDatas = (LvProp*)GetLogicServer()->GetBuffAllocator()->AllocBuffer(sizeof(LvProp) * soulCfg.prop.count);

			if ( enumTableFirst() )	// prop[]
			{
				INT_PTR nIdx = 0;
				do 
				{
					LvProp& lvProp(soulCfg.prop.pDatas[nIdx]);
					//读取1个等级的数据
					if( ! ReadPropOneLevel(lvProp) )
					{
						showErrorFormat(_T("Soul level error, level=%d "),nIdx+1);
						return false;
					}

					nIdx++;
				}while (enumTableNext());
			}
		}	
		closeTable(); // prop
	}
	if ( feildTableExists("lianHun") && openFieldTable("lianHun") )
	{		
		ReadIntArray("wuhunExp", soulCfg.lianHun);
		closeTable(); // lianHun
	}
	closeTable(); // SoulCfg
	OutputMsg(rmTip,_T("Load Soul Finished"));
	return true;
}



bool CSoulConfig::ReadPropOneLevel(LvProp& lvProp){
	const INT_PTR nCount = LuaTableLen();

	if ( nCount <= 0 )
	{
		//OutputMsg(rmWaning,_T("no skill level data on Skill config,skillID=%d"),pSkill->nSkillID);
	}
	else
	{
		lvProp.count = (unsigned int)nCount;
		lvProp.pDatas = (GAMEATTR*)GetLogicServer()->GetBuffAllocator()->AllocBuffer(sizeof(GAMEATTR) * lvProp.count);
		GAMEATTR* pAttr = lvProp.pDatas;
		if (enumTableFirst())
		{
			if (!LoadAttri(pAttr++)) return false;
		}//end if
	}
	return true;
}
