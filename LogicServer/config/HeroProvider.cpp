#include "StdAfx.h"
#include "HeroProvider.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

CHeroProvider::CHeroProvider() : m_dataAllocator(_T("HeroDataAlloc"))
{
     m_nMaxHeroCount = 0;
	 m_nHeroAI = 0;
	 m_nDefaultAttackSpeed = 0;
	 m_nBuffGroup = 0;
}
CHeroProvider::~CHeroProvider()
{

}

void CHeroProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CHeroProvider::LoadConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on Hero config"));
		//读取标准物品配置数据
		Result = ReadAllHero();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load  Hero config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Hero config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CHeroProvider::ReadAllHero()
{
	if ( !openGlobalTable("HeroConfig") )
	{
		showError(_T("no global table HeroConfig  Found"));
		return false;
	}
	int  nDef = 0;
    m_nMaxHeroCount   = getFieldInt("MaxHeroCount",&nDef);
	m_nHeroAI =  getFieldInt("HeroAI",&nDef);
    m_nDefaultAttackSpeed =  getFieldInt("DefaultAttackSpeed",&nDef);
	m_nBuffGroup = getFieldInt("BuffGroup",&nDef);
	m_nAddTime    = getFieldInt("AddTime",&nDef);
	m_herosData.clear();
	if ( feildTableExists("HeroList") && openFieldTable("HeroList") )
	{
	     int  nIdx =1; //英雄ID 1起始
		if (enumTableFirst())
		{
			 do 
		  	{
				HEROCONFIG oneHero;
				ReadOneHero(m_dataAllocator,&oneHero,nIdx);
				m_herosData.add(oneHero);
				nIdx++;
			}
			 while (enumTableNext());
		  }
          closeTable();
	}
	closeTable();
	return true;
}

bool CHeroProvider::ReadOneHero(CDataAllocator &dataAllocator,PHEROCONFIG  pHero, int  nHeroId)
{   
     pHero->nId = nHeroId;
	 getFieldStringBuffer("name",pHero->name,sizeof(pHero->name) );
	 int nDef=0;
	 bool boDef = false;
	 pHero->nOpenLevel = getFieldInt("OpenLevel",&nDef);
	 pHero->nMaxLevel   = getFieldInt("MaxLevel",&nDef);
	 pHero->nMaxForce   = getFieldInt("MaxForce",&nDef);
	 pHero->nBattleForce = getFieldInt("BattleForce",&nDef);
	 pHero->fExpRate = (float)getFieldNumber("ExpRate");

	 //Stages
	 if ( feildTableExists("Stages") && openFieldTable("Stages") )
	 {
	       int  nCount  =   (int)lua_objlen(m_pLua,-1);
		   pHero->stages.count = nCount;
		   pHero->stages.pData  =   (PHEROSTAGE)dataAllocator.allocObjects(sizeof(HEROSTAGE) * nCount);
		   if(enumTableFirst())
		    {
		           int  nIdx = 0 ;
				   do 
				   {
                         HEROSTAGE &stage =  pHero->stages[nIdx];   
						 stage.nModel  = getFieldInt("ModelId",&nDef);
						 stage.nWeapon = getFieldInt("WeaponId",&nDef);
					     stage.nMaxLevel = getFieldInt("MaxLevel",&nDef);
						 stage.nMaxBless = getFieldInt("MaxBless",&nDef);
						 stage.nGiveForce = getFieldInt("GiveForce",&nDef);
						 stage.boEveryDayClearBless = getFieldBoolean("EveryDayClearBless",&boDef);
						 nIdx++;
				   } while (enumTableNext());		     
		    }
	       closeTable();
	 }

	 //levels
	 if ( feildTableExists("Levels") && openFieldTable("Levels") )
	 {
		 int  nCount  = (int)lua_objlen(m_pLua,-1);
		 pHero->levels.count = nCount;
		 pHero->levels.pData  =   (PHEROLEVEL)dataAllocator.allocObjects(sizeof(HEROLEVEL) * nCount);
		 if(enumTableFirst())
		 {
			 int  nIdx = 0 ;
			 do 
			 {
			       HEROLEVEL &level = pHero->levels[nIdx];
				   level.nLevelUpExp =   (unsigned int)getFieldNumber("LevelUpExp");
				   if ( feildTableExists("attrs") && openFieldTable("attrs") )
				   {
				          int  nAttrCount = (int)lua_objlen(m_pLua,-1);
						  level.attri.nCount = nAttrCount;
						  PGAMEATTR pAttr = NULL;
						  if(nCount > 0)
						  {
							  level.attri.pAttrs = (PGAMEATTR)dataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
							  ZeroMemory(level.attri.pAttrs, sizeof(GAMEATTR) * nCount);
							  pAttr = level.attri.pAttrs;
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
				   nIdx++;
			 } while (enumTableNext());		     
		 }
		 closeTable();
	 }
     return true;
}


bool CHeroProvider::LoadAttri( PGAMEATTR pAttr )
{
	do
	{
		pAttr->type = getFieldInt("type");
		if ( pAttr->type < aUndefined || pAttr->type >= GameAttributeCount )
		{
			showErrorFormat(_T("HeroLevel attri  type config error %d"),pAttr->type);
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
		pAttr++;
	}while(enumTableNext());
	return true;
}
