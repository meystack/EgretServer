#include "StdAfx.h"
#include "../interface/PetExportFun.h"
namespace Pet
{
	
	int addPet(void* pEntity,int nPetConfigId,int nLevel, int nAiId,int nLiveSecs)
	{

		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		return ((CActor *)pEntity)->GetPetSystem().AddPet(nPetConfigId,nLevel, nAiId,nLiveSecs);	
	}

	int getPetCountByConfigId( void * pEntity, int nPetId )
	{
		if(pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return -1;
		}
		return ((CActor*)pEntity)->GetPetSystem().GetPetCountByConfigId(nPetId);
	}

	char * getPetName(void * pEntity ,int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		 const PMONSTERCONFIG pConfig =  GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nPetId);
		 if (pConfig)
		 {
			 return pConfig->szName;
		 }
		return NULL;
	}

	bool delPetById(void * pEntity, int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CActor * pActor = (CActor *)pEntity;
		return pActor->GetPetSystem().RemoveAllPetByConfigId(nPetId) > 0;
	}
	/*
	bool setPetStrong(void *pEntity, int nPertId,int nStrong)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().SetPetStrong(nPertId,nStrong);
	}

	
	bool petSmith(void *pEntity, int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().SmithPet(nPetId);

	}
	bool setCircle(void *pEntity, int nPetID, int nCircle)
	{
	if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
	return ((CActor *)pEntity)->GetPetSystem().SetCircle(nPetID,nCircle);
	}


	bool addExp(void *pEntity,int nPetId, int nExp)
	{	
	if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
	return ((CActor *)pEntity)->GetPetSystem().ChangeExp(nPetId,nExp);
	}

	bool  canAddPet(void *pEntity, int nPetConfigId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().CanAddPet(nPetConfigId);
	}

	bool addLoyalty(void *pEntity,int nPetId, int nValue)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().ChangeLoyalty(nPetId,nValue);
	}
	
	int getPetProperty(void *pEntity,int nPetId, int nPropId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		
		const CPetSystem::PETDATA * pPet = ((CActor *)pEntity)->GetPetSystem().GetPetData(nPetId);
		if(pPet ==NULL)
		{
			return -1;
		}
		if( enPetMaxLoyalty ==nPropId)
		{
			return ((CActor *)pEntity)->GetPetSystem().GetMaxLoyalty();
		}

		switch ( nPropId)
		{
		case enPetLevel:
			return pPet->data.bLevel;
			break;
		case enPetStrong:
			return pPet->data.bStrong;
			break;
		case enPetCircle:
			return pPet->data.bCircle;
			break;
		case enPetConfigId:
			return pPet->data.wConfigId;
			break;
		case enPetExp:
			return (int)(pPet->data.nExp);
			break;

		case enPetLoyalty:
			return pPet->data.wLoyalty;
			break;

		case enBaseAttack:
			return (int)(pPet->data.nBaseAttack);
			break;
		case enBaseDefence:
			return (int)(pPet->data.nBaseDefence);
			break;

		case enBaseAgility:
			return (int)(pPet->data.nBaseAgility);
			break;
		case enBasePhysique:
			return (int)(pPet->data.nBasePhysique);
			break;
		case enSmithAttack:
			return (int)(pPet->data.nSmithAttack);
			break;
		case enSmithDefence:
			return (int)(pPet->data.nSmithDefence);
			break;
		case enSmithAgility:
			return (int)(pPet->data.nSmithAgility);
			break;

		case enSmithPhysique:
			return (int)(pPet->data.nSmithPhysique);
			break;
		case enPetScore:
			return (int)(pPet->data.nScore);
			break;
		case enPetAttackType:
			return (int)(pPet->data.bAattackType);
			break;

		case enPetIconId: //宠物的图标ID
			return (int)(pPet->data.bIconId);
			break;

		case enPetWashMergeRate://洗出来的附体的比例
			return (int)(pPet->data.wWashMergeRate);
			break;
		case enPetQuality: //品质
			return (int)(pPet->data.bQuality);
			break;
		case enPetTotalMergeRate: //当前全部的附体资质
			return (int)(pPet->data.wWashMergeRate + pPet->wQualityMergeRate);
			break;
		case enPetMaxMergeRate://当前最大的附体资质比例
			return pPet->wMaxMergeRate;
			break;
			//技能的洗练次数为
		case enPetSkillSlotCount:
			return  pPet->data.bSkillOpenSlot + pPet->data.bCircle;
			break;
		default:
			return -1;
		}
		return -1;
	}
	bool  enlargePetSlot(void *pEntity, int nGridCount)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().EnlargePetSlot(nGridCount);
	}

	bool learnSkill(void *pEntity,int nPetID, int nSkillID,int nSlotId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().LearnSkill(nPetID,nSkillID,nSlotId);
	}

	int getLearnSkillId(void* pEntity,int nPetId,int nSkillSlot)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().GetLearnSkillId(nPetId,nSkillSlot);
	}

	bool forgetSkill(void* pEntity,int nPetId,int nSkillId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().ForgetSkill(nPetId,nSkillId);
	}
	int getSkillLevel(void* pEntity,int nPetId, int nSkillId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->GetPetSystem().GetSkillLevel(nPetId,nSkillId);
	}
	bool skillLevelUp(void *pEntity,int nPetID, int nSkillID )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().SkillLevelUp(nPetID,nSkillID);
	}
	
	bool delPet(void * pEntity, int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().RemovePet(nPetId);
	}

	
	int getBattlePetId(void * pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		return   ((CActor *)pEntity)->GetPetSystem().GetBattlePetId();
		
	}

	int getMergePetId(void * pEntity)
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		return ((CActor *)pEntity)->GetPetSystem().GetMergePetId();
	}

	void sendPetOpResult(void * pEntity,int nPetId,int nOpId, bool result)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		CActor *pActor = (CActor *)pEntity;
		if( pActor &&pActor->IsInited() )
		{
			CActorPacket pack;
			pActor->AllocPacket(pack);
			pack << (BYTE)(enPetSystemID)<< (BYTE) sPetOpResult;
			pack << (BYTE)(nOpId) ;
			pack << (BYTE) nPetId;
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
	bool changeAttackType(void *pEntity, int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return   ((CActor *)pEntity)->GetPetSystem().ChangePetAttackState(nPetId);
	}

	void setPetBattle(void* pEntity, int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor *)pEntity)->GetPetSystem().SetPetBattle(nPetId);
	}
	bool setPetSkin(void* pEntity, int nPetId,int configId,int nQuality)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().SetPetSkin(nPetId,configId,nQuality);
	}
	
	bool setPetMergeRate(void* pEntity, int nPetId,int nRate)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetPetSystem().SetPetMergeRate(nPetId,nRate);
	}
	char * getPetName(void * pEntity ,int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		const CPetSystem::PETDATA * pPet = ((CActor *)pEntity)->GetPetSystem().GetPetData(nPetId);
		if(pPet ==NULL)
		{
			return NULL;
		}
		return (char*)(pPet->data.name);
	}
	bool enlargePetSkillSlot(void * pEntity, int nPetId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return  ((CActor *)pEntity)->GetPetSystem().EnlargePetSkillSlot(nPetId);
		
	}
	*/

}