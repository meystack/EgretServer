#include "StdAfx.h"
#include "../interface/HeroExportFun.h"
namespace Hero
{
	
	int addHero(void* pEntity,int  nHeroId, int  nStage, int nLevel)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		return ((CActor *)pEntity)->GetHeroSystem().AddHero(nHeroId,nStage, nLevel);	
	}

	bool  canAddHero(void *pEntity,  bool bWithTipmsg)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetHeroSystem().CanAddHero(bWithTipmsg);
	}
	
	bool  StageUp(void *pEntity, int  nHeroId,int  nStage,  int nBless)
	{
	    if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetHeroSystem().StageUp(nHeroId,nStage,nBless);
	}
	
	unsigned int getHeroProperty(void *pEntity,int nHeroId, int nPropId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return -1;	
		const CHeroSystem::HERODATA * pHero = ((CActor *)pEntity)->GetHeroSystem().GetHeroData(nHeroId);
		if(pHero ==NULL)
		{
			return 0;
		}
		switch ( nPropId)
		{
		case enHeroStage:
		    return pHero->data.bStage;
		case enHeroLevel:
			return pHero->data.bLevel;
			break;
		case enHeroExp:
		   return (unsigned int)pHero->data.nExp;
		   break;
		case enHeroBless:
		   return pHero->data.nBless;
		case enHeroForce:
		   return pHero->data.nBodyforce;
		default:
			return 0;
		}
		return 0;
	}

	//设置英雄属性
	bool setHeroProperty(void *pEntity,int nHeroId, int nPropId,unsigned int nValue)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CHeroSystem::HERODATA * pHero = ((CActor *)pEntity)->GetHeroSystem().GetHeroData(nHeroId);
		if(pHero ==NULL)
		{
			return false;
		}
		switch ( nPropId)
		{
		case enHeroStage:
			pHero->data.bStage = (BYTE)nValue;
			((CActor *)pEntity)->GetHeroSystem().SendHeroData(*pHero);
			((CActor *)pEntity)->GetHeroSystem(). SetHeroDataModifyFlag(0,true);
			return true;
		case enHeroLevel:
			pHero->data.bLevel = (BYTE)nValue;
			((CActor *)pEntity)->GetHeroSystem().SendHeroData(*pHero);
			((CActor *)pEntity)->GetHeroSystem(). SetHeroDataModifyFlag(0,true);
			return true;
		case enHeroExp:
			pHero->data.nExp =  nValue;
			((CActor *)pEntity)->GetHeroSystem().SendHeroData(*pHero);
			((CActor *)pEntity)->GetHeroSystem(). SetHeroDataModifyFlag(0,true);
			return true;
		case enHeroBless:
			pHero->data.nBless = (int)nValue;
			((CActor *)pEntity)->GetHeroSystem().SendHeroData(*pHero);
			((CActor *)pEntity)->GetHeroSystem(). SetHeroDataModifyFlag(0,true);
			return true;
		case enHeroForce:
			pHero->data.nBodyforce = (int)nValue;
			((CActor *)pEntity)->GetHeroSystem().SendHeroData(*pHero);
			((CActor *)pEntity)->GetHeroSystem(). SetHeroDataModifyFlag(0,true);
			return true;
		default:
			return false;
		}
		return false;
	}

	bool learnSkill(void *pEntity,int nHeroID, int nSkillID)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetHeroSystem().LearnSkill(nHeroID,nSkillID);
	}

	int getSkillLevel(void* pEntity,int nHeroId, int nSkillId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->GetHeroSystem().GetSkillLevel(nHeroId,nSkillId);
	}
	bool skillLevelUp(void *pEntity,int nHeroID, int nSkillID )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetHeroSystem().SkillLevelUp(nHeroID,nSkillID);
	}

	int getBattleHeroId(void * pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		return   ((CActor *)pEntity)->GetHeroSystem().GetBattleHeroId();		
	}

	void sendHeroOpResult(void * pEntity,int nHeroId,int nOpId, bool result)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		CActor *pActor = (CActor *)pEntity;
		if( pActor &&pActor->IsInited() )
		{
			CActorPacket pack;
			pActor->AllocPacket(pack);		
			pack << (BYTE)(enHeroSystemId)<< (BYTE) sHeroOpResult;
			pack << (BYTE)(nOpId) ;
			pack << (BYTE) nHeroId;
			if(result)
			{
				pack <<(BYTE)1;
			}
			else
			{
				pack <<(BYTE)0;
			}
			pack.flush();
 		}
	}

	void setHeroBattle(void* pEntity, int nHeroId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor *)pEntity)->GetHeroSystem().SetHeroBattle(nHeroId);
	}

	char * getHeroName(void * pEntity ,int nHeroId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		const CHeroSystem::HERODATA * pHero = ((CActor *)pEntity)->GetHeroSystem().GetHeroData(nHeroId);
		if(pHero ==NULL)
		{
			return NULL;
		}
		return (char*)(pHero->data.name);
	}

	int getHeroCount(void * pEntity)
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return (int)((CActor *)pEntity)->GetHeroSystem().GetHeroCount();
	}

	void  addExp(void *pEntity, unsigned int nExp , int nHeroId )
	{
	    if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor *)pEntity)->GetHeroSystem().OnAddExp(nExp,false, nHeroId);
	}

	void addExpByHeroId(void *pEntity, int  nHeroId, unsigned int nExp)
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		INT_PTR   nPos = ((CActor *)pEntity)->GetHeroSystem().GetHeroPos(nHeroId);
	    ((CActor *)pEntity)->GetHeroSystem().RealChangeExp(nPos,nExp);
	}

	void  CallBackHero(void *pEntity,bool boClient)
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		 ((CActor *)pEntity)->GetHeroSystem().CallbackBattleHero(boClient);
	}
}