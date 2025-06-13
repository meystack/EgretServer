#include "StdAfx.h"
#include "SkillCondition.h"

//目标选择的条件
const CSkillCondition::TargetSelCondition CSkillCondition::Handlers[]=
{
	&CSkillCondition::IsError,	//0
	&CSkillCondition::IsEnermy,
	&CSkillCondition::IsFriendly,
	&CSkillCondition::IsSelf,
	&CSkillCondition::IsTeammate,
	&CSkillCondition::IsMonster,	//5
	&CSkillCondition::IsActor,
	&CSkillCondition::LevelAbove,
	&CSkillCondition::LevelBelow,
	&CSkillCondition::PkBelow,
	&CSkillCondition::PkAbove,	//10
	&CSkillCondition::Probalility,
	&CSkillCondition::IsDeath,
	&CSkillCondition::CanAttack, 
	&CSkillCondition::LevelDisBelow,
	&CSkillCondition::RevertLevelDisBelow,	//15
	&CSkillCondition::DistanceLarge,
	&CSkillCondition::CircleAbove,
	&CSkillCondition::CircleBelow,
	&CSkillCondition::CircleDisBelow,
	&CSkillCondition::RevertCircleDisBelow,		//20
	&CSkillCondition::WithBuff,
	&CSkillCondition::WithoutBuff,
	&CSkillCondition::HpAbove,
	&CSkillCondition::HpBelow,
	&CSkillCondition::MpAbove,		//25
	&CSkillCondition::MpBelow,
	&CSkillCondition::SameGuild,
	&CSkillCondition::FaceTarget,
	&CSkillCondition::TargetOneDir,
	&CSkillCondition::IsSpecialMonster,	//30
	&CSkillCondition::SelSameCamp,
	&CSkillCondition::SelfHpRateAbove,
	&CSkillCondition::SelfHpRateBelow,
	&CSkillCondition::SelfIsBrother, //glj 没用上
	&CSkillCondition::IsMaster,	//35
	&CSkillCondition::IsSpecialTypeMonster,
	&CSkillCondition::IsMagicAttackMaxAbove,
	&CSkillCondition::IsMagicAttackMaxBelow,
	&CSkillCondition::IsWizardAttackMaxAbove,
	&CSkillCondition::IsWizardAttackMaxBelow,	//40
	&CSkillCondition::IsSelfMetiralItemId,
	&CSkillCondition::IsError,
	&CSkillCondition::IsSpeciaWeapon,
	&CSkillCondition::IsSbkMember,
	&CSkillCondition::IsError,	//45
	&CSkillCondition::IsError,
	&CSkillCondition::FbId,
	&CSkillCondition::SelfSpecialMonster,
	&CSkillCondition::IsModelId,
	&CSkillCondition::SceneHasMonster,	//50
	&CSkillCondition::SceneWithoutMonster,
	&CSkillCondition::SelfLevelBelow,
	&CSkillCondition::SelfLevelAbove,
	&CSkillCondition::SelfSceneId,
	&CSkillCondition::SceneMonsterCount,	//55
	&CSkillCondition::MobPetCount, 
	&CSkillCondition::EntityType,
	&CSkillCondition::EntityVocation,
	&CSkillCondition::HasSpecialEquip,
	&CSkillCondition::IsError,	//60
	&CSkillCondition::IsError,
	&CSkillCondition::SelfWithBuff,
	&CSkillCondition::SelfWithoutBuff,
	&CSkillCondition::SelfJobPet,
	&CSkillCondition::SelfCircleBelow, //65
	&CSkillCondition::RevertCircleBelow,
	&CSkillCondition::MonsterLevelUpCheck,
	&CSkillCondition::ScenceHasSelfPet,  
	&CSkillCondition::SelfSkillLevelAndTargetLevelAbove,
	&CSkillCondition::SelfSkillLevelAndTargetLevelBelow,
	
};

/*
技能升级的条件，对应skill_x_x.txt中trainConds部分的cond
*/
const CSkillCondition::SkillUseTrainCondition CSkillCondition::UseTrainHandlers[]=
{
	&CSkillCondition::UtConditionNone,	//0
	&CSkillCondition::UtConditionLevel,
	&CSkillCondition::UtConditionCoin,
	&CSkillCondition::UtConditionItem,
	&CSkillCondition::UtConditionExp,
	&CSkillCondition::UtConditionXiuwei,	//5
	&CSkillCondition::UtConditionVocation,
	&CSkillCondition::UtConditionHp,
	&CSkillCondition::UtConditionMp,
	&CSkillCondition::UtConditionTargetExist,
	&CSkillCondition::UtConditionPetCount,	//10
	&CSkillCondition::UtConditionWithBuff,
	&CSkillCondition::UtConditionWithoutBuff,
	&CSkillCondition::UtConditionTargetDistance,
	&CSkillCondition::UtConditionTargetSamePosition,
	&CSkillCondition::UtConditionFaceTarget,	//15
	&CSkillCondition::UtConditionOneDirTarget,	
	&CSkillCondition::UtConditionNone,
	&CSkillCondition::UtConditionHpRateBelow,
	&CSkillCondition::UtConditionSpecialTarget,
	&CSkillCondition::UtConditionTargetCanAttack,	//20
	&CSkillCondition::UtConditionNone,			//21
	&CSkillCondition::UtConditionJumpPower,
	&CSkillCondition::UtConditionBindCoin,
	&CSkillCondition::UtConditionCampTitle,		//glj没用上
	&CSkillCondition::UtConditionBrotherCount,	//25 glj没用上
	&CSkillCondition::UtConditionYAbsDis,
	&CSkillCondition::UtConditionTeamBrotherCount, //glj没用上
	&CSkillCondition::UtConditionBaseMaxMpRate,
	&CSkillCondition::UtConditionBaseMaxHpRate,
	&CSkillCondition::UtConditionBattleState,	//30
	&CSkillCondition::UtConditionGuildContr,		//消耗行会贡献
	&CSkillCondition::UtConditionGuildSkillLevel, 
	&CSkillCondition::UtConditionXpValue,
	&CSkillCondition::UtConditionMatiralDura,
	&CSkillCondition::UtConditionNone,	//35
	&CSkillCondition::UtConditionDigTool,
	&CSkillCondition::UtConditionAngerValue,
	&CSkillCondition::UtConditionAreaProp,
	&CSkillCondition::UtConditionCircle,
	&CSkillCondition::UtConditionLineDistance,	//40
	&CSkillCondition::UtConditionTargetDistanceAbove,
	&CSkillCondition::UtConditionHasJobPetCount,	//42
	&CSkillCondition::UtConditionNone,//
	&CSkillCondition::UtConditionGuildMainHall,		//44 行会主殿等级（即行会等级）
	&CSkillCondition::UtConditionGuildTech,			//45 行会科技（不使用）
	&CSkillCondition::UtConditionHpRateAbove,
};

const CSkillCondition::SkillUseTrainCondition CSkillCondition::SpellHandlers[]=
{
	&CSkillCondition::UtConditionNone,			//0
	&CSkillCondition::UtConditionHp,			//1 消耗红
	&CSkillCondition::UtConditionMp,			//2 消耗蓝
	&CSkillCondition::UtConditionWithoutBuff,	//3 不存在buff
	&CSkillCondition::UtConditionTargetDistance,//4 和目标的距离必须小于
	&CSkillCondition::UtConditionFlyShoes,		//5 消耗小飞鞋或回城卷
};

const CSkillCondition::SkillUseTrainCondition CSkillCondition::UpgradeHandlers[]=
{
	&CSkillCondition::UtConditionNone,			//0
	&CSkillCondition::UtConditionLevel,			//1 等级达到
	&CSkillCondition::UtConditionBindCoin,		//2 绑定金币
	&CSkillCondition::UtConditionItem,			//3 物品
	&CSkillCondition::UtConditionCircle,		//4 转数
	&CSkillCondition::UtConditionExp,			//5 经验
	&CSkillCondition::UtConditionInternalWorkLevel,	//6 内功等级
};

bool  CSkillCondition::Check( CAnimal * pSrcEntity,CAnimal *pTargetEntity,TARGETSELCONDITION &value)
{
	INT_PTR nCmdID = value.nKey;
	if(nCmdID >=0 && nCmdID <ArrayCount(Handlers))
	{
		bool ret = (this->*Handlers[nCmdID] )(pSrcEntity,pTargetEntity,value.value);
		return ret;
	}
	else
	{
		return false;
	}
}

/*
坚持需要的条件，
needConsume：是否扣除资源
*/
INT_PTR CSkillCondition::CheckUseTrainContion(CAnimal * pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	INT_PTR nCmdID = contition.nConditionID;
	if(nCmdID >=0 && nCmdID < ArrayCount (UseTrainHandlers) )
	{
		return (this->*UseTrainHandlers[nCmdID] )(pActor,nSkillID,contition,needConsume);
	}
	else
	{
		return tmNoError;
	}
}

INT_PTR CSkillCondition::CheckSpellContion(CAnimal * pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	INT_PTR nCmdID = contition.nConditionID;
	if(nCmdID >=0 && nCmdID < ArrayCount (SpellHandlers) )
	{
		return (this->*SpellHandlers[nCmdID] )(pActor,nSkillID,contition,needConsume);
	}
	else
	{
		return tmNoError;
	}
}

INT_PTR CSkillCondition::CheckUpGradeContion(CAnimal * pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	INT_PTR nCmdID = contition.nConditionID;
	if(nCmdID >=0 && nCmdID < ArrayCount (UpgradeHandlers) )
	{
		return (this->*UpgradeHandlers[nCmdID] )(pActor,nSkillID,contition,needConsume);
	}
	else
	{
		return tmNoError;
	}
}



//目标为敌人
bool CSkillCondition::IsEnermy(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pSrcEntity ==NULL || pTargetEntity ==NULL) return false;
	if(pSrcEntity == pTargetEntity) return false;
	/*
	INT_PTR nSrcType = pSrcEntity->GetType();
	
	INT_PTR nTargetType = pTargetEntity->GetType();
	if(nSrcType ==enActor) //释放技能者是一个玩家
	{
		if (pTargetEntity->IsMonster() ) return true;
		if( pTargetEntity->isNPC() ) return false;
		if(pTargetEntity->GetType() ==enActor) //这里要根据PK值判断
		{
			return ((CActor *)pSrcEntity)->GetPkSystem().CanAttack((CActor*) pTargetEntity);
		}
		else
		{
			return false;
		}
	}
	else if(nSrcType ==enMonster) //如果释放者是一个怪物
	{
		if(pTargetEntity->GetType() ==enActor)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(nSrcType == enPet) //如果是一个宠物
	{
		if (pTargetEntity->IsMonster() ) return true;
		if( pTargetEntity->isNPC() ) return false;
	}
	
	return false;
	*/
	return pSrcEntity->CanAttack(pTargetEntity) == value.bValue;
}
	
//目标为友方
 bool CSkillCondition::IsFriendly(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	/*
	if (pTargetEntity->IsMonster() ) return false;
	if( pTargetEntity->isNPC() ) return false;
	if(pTargetEntity->GetType() ==enActor) //这里要根据PK值判断
	{
		return true;
	}
	*/
	if(pSrcEntity ==NULL || pTargetEntity ==NULL) return false;
	return ! (pSrcEntity->CanAttack(pTargetEntity) == value.bValue);
}
//目标为自己
 bool CSkillCondition::IsSelf(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if (pSrcEntity ==NULL || pTargetEntity ==NULL) return false;
	 return (pSrcEntity->GetHandle() == pTargetEntity->GetHandle()) == value.bValue ;
}

//目标为队伍成员
 bool CSkillCondition::IsTeammate(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	bool flag =false ;
	INT_PTR nSrcType	= pSrcEntity->GetType();
	INT_PTR nTargetType = pTargetEntity->GetType();
	if( (nSrcType ==enActor || nSrcType == enPet) &&  (nTargetType ==enActor || nTargetType == enPet))
	{
		CActor * pSrcActor ;
		CActor * pTargetActor;
		if(nSrcType ==enActor)
		{
			pSrcActor =(CActor*) pSrcEntity;
		}
		else
		{
			pSrcActor =((CPet*) pSrcEntity)->GetMaster(); //主人的指针
		}

		if(nTargetType ==enActor)
		{
			pTargetActor =(CActor*) pTargetEntity;
		}
		else
		{
			pTargetActor =((CPet*) pTargetEntity)->GetMaster(); //主人的指针
		}
		
		if(pSrcActor !=NULL && pTargetActor != NULL)
		{
			//自己和自己的宠物为队友关系
			if(pSrcActor == pTargetActor  && nSrcType != nTargetType)
			{
				flag =true;
			}
			else
			{
				if( pSrcActor->GetProperty<unsigned int>( PROP_ACTOR_TEAM_ID) !=0 )
				{
					flag=(
						pSrcActor->GetProperty<unsigned int>( PROP_ACTOR_TEAM_ID) 
						== pTargetActor->GetProperty<unsigned int>( PROP_ACTOR_TEAM_ID) );
				}
			}

		}
		
	}
	
	return  flag== value.bValue;
}

//目标为怪物
 bool CSkillCondition::IsMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return pTargetEntity->IsMonster() == value.bValue;
}
//目标为玩家
 bool CSkillCondition::IsActor(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return (pTargetEntity->GetType() == enActor) == value.bValue;
}
//标等级 >= value,
 bool CSkillCondition::LevelAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	//如果自己是人活着玩家的话
	int nCircle = 0;
	if(pTargetEntity->GetType() == enActor)
	{
		nCircle = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pTargetEntity)->GetHeroPtr();
		if (pData)
		{
			nCircle = pData->data.bCircle;
		}
	}
	if (nCircle >= 1)
	{
		return true;
	}

	return pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) >= value.uValue;
}



//目标等级 <= value
 bool CSkillCondition::LevelBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	int nCircle = 0;
	if(pTargetEntity->GetType() == enActor)
	{
		nCircle = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pTargetEntity)->GetHeroPtr();
		if (pData)
		{
			nCircle = pData->data.bCircle;
		}
	}
	if (nCircle >= 1)
	{
		return false;
	}

	return pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) <= value.uValue;
}




//pk值低于数值,整数
 bool CSkillCondition::PkBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if ( !pTargetEntity || pTargetEntity->GetType() != enActor )
	{
		return false;
	}
	return pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_PK_VALUE) < value.uValue;
}
//pk值高于数值 ,整数
 bool CSkillCondition::PkAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if ( !pTargetEntity || pTargetEntity->GetType() != enActor )
	{
		return false;
	}
	return pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_PK_VALUE) >= value.uValue;
}
//随机概率,5000表示百分之50概率，单位万分之1
 bool CSkillCondition::Probalility(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return  ( wrandvalue() % 10000) <= value.uValue ;
		
}
//死亡
 bool CSkillCondition::IsDeath(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return pTargetEntity->IsDeath() == value.bValue;
}
//可攻击
 bool CSkillCondition::CanAttack(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pSrcEntity ==NULL || pTargetEntity ==NULL ) return false;
	bool boAttackNotice = (value.nValue != 0);
	return( pSrcEntity->CanAttack(pTargetEntity, NULL, boAttackNotice) ==(value.nValue !=0) );

}
//就是自己的等级减目标的等级小于等于这个数值
	
 bool CSkillCondition::LevelDisBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	 
	int nSrcLevel = (int)pSrcEntity->GetProperty< int>(PROP_CREATURE_LEVEL); //自己等级
	int nTgtLevel =(int)pTargetEntity->GetProperty< int>(PROP_CREATURE_LEVEL); //目标等级
	int nSrcCircle = 0;
	int nTgtCircle = 0;
	CMiscProvider &config = GetLogicServer()->GetDataProvider()->GetMiscConfig();
	
	if( pSrcEntity->GetType() == enActor )
	{
		nSrcCircle = (int)pSrcEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);		
	}
	else if ( pSrcEntity->GetType() == enHero )
	{
		const CHeroSystem::HERODATA *pData = ((CHero*)pSrcEntity)->GetHeroPtr();
		if ( pData )
		{
			nSrcCircle = pData->data.bCircle;
		}
	}
	else if (pSrcEntity->GetType() == enMonster)
	{
		nSrcCircle = pSrcEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pSrcEntity->GetType() == enPet)
	{
		nSrcCircle = ((CPet*)pSrcEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}
	nSrcLevel +=  nSrcCircle * config.GetPlayerMaxLevel(nSrcCircle);

	if( pTargetEntity->GetType() == enActor )
	{
		nTgtCircle = (int)pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if ( pTargetEntity->GetType() == enHero )
	{
		const CHeroSystem::HERODATA *pData = ((CHero*)pTargetEntity)->GetHeroPtr();
		if ( pData )
		{
			nTgtCircle = pData->data.bCircle;
		}
	}
	else if (pTargetEntity->GetType() == enMonster)
	{
		nTgtCircle = pTargetEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enPet)
	{
		nTgtLevel = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_CREATURE_LEVEL);
		nTgtCircle = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}
	nTgtLevel += nTgtCircle * config.GetPlayerMaxLevel(nTgtCircle);

	int level= nSrcLevel- nTgtLevel ;
	return level  <= value.nValue;
}
	
//就是目标的等级减自己的等级小于等于这个数值
 bool CSkillCondition::RevertLevelDisBelow (CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pSrcEntity == NULL || pTargetEntity ==NULL ) return false;
	
	int nSrcLevel = (int)pSrcEntity->GetProperty< int>(PROP_CREATURE_LEVEL); //自己等级
	int nTgtLevel =(int)pTargetEntity->GetProperty< int>(PROP_CREATURE_LEVEL); //目标等级
	int nSrcCircle = 0;
	int nTgtCircle = 0;
	CMiscProvider &config = GetLogicServer()->GetDataProvider()->GetMiscConfig();
	//static int nMaxLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMaxPlayerLevel;

	if( pSrcEntity->GetType() == enActor )
	{
		nSrcCircle = (int)pSrcEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if ( pSrcEntity->GetType() == enHero )
	{
		const CHeroSystem::HERODATA *pData = ((CHero*)pSrcEntity)->GetHeroPtr();
		if ( pData )
		{
			nSrcCircle = pData->data.bCircle;
		}
	}
	else if (pSrcEntity->GetType() == enMonster)
	{
		nSrcCircle = pSrcEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pSrcEntity->GetType() == enPet)
	{
		nSrcCircle = ((CPet*)pSrcEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}
	nSrcLevel += nSrcCircle  * config.GetPlayerMaxLevel(nSrcCircle);

	if( pTargetEntity->GetType() == enActor )
	{
		nTgtCircle = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if ( pTargetEntity->GetType() == enHero )
	{
		const CHeroSystem::HERODATA *pData = ((CHero*)pTargetEntity)->GetHeroPtr();
		if ( pData )
		{
			nTgtCircle = pData->data.bCircle;
		}
	}
	else if (pTargetEntity->GetType() == enMonster)
	{
		nTgtCircle = pTargetEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enPet)
	{
		nTgtLevel = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_CREATURE_LEVEL);
		nTgtCircle = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}
	nTgtLevel += nTgtCircle  * config.GetPlayerMaxLevel(nTgtCircle);
	int level=nTgtLevel - nSrcLevel ;


	return level  <= value.nValue;
}

bool CSkillCondition::CircleDisBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	int nTargetCircle = 0; 
	if(pTargetEntity->GetType() == enActor)
	{
		nTargetCircle = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pTargetEntity)->GetHeroPtr();
		if (pData)
		{
			nTargetCircle = pData->data.bCircle;
		}
	}
	else if (pTargetEntity->GetType() == enMonster)
	{
		nTargetCircle = pTargetEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enPet)
	{
		nTargetCircle = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}

	int nSrcCircle = 0; 
	if(pSrcEntity->GetType() == enActor)
	{
		nSrcCircle = pSrcEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pSrcEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pSrcEntity)->GetHeroPtr();
		if (pData)
		{
			nSrcCircle = pData->data.bCircle;
		}
	}
	else if (pSrcEntity->GetType() == enMonster)
	{
		nSrcCircle = pSrcEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pSrcEntity->GetType() == enPet)
	{
		nSrcCircle = ((CPet*)pSrcEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}

	return (nSrcCircle - nTargetCircle) <= value.nValue;
}

bool CSkillCondition::RevertCircleDisBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	int nTargetCircle = 0; 
	if(pTargetEntity->GetType() == enActor)
	{
		nTargetCircle = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pTargetEntity)->GetHeroPtr();
		if (pData)
		{
			nTargetCircle = pData->data.bCircle;
		}
	}
	else if (pTargetEntity->GetType() == enMonster)
	{
		nTargetCircle = pTargetEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enPet)
	{
		nTargetCircle = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}

	int nSrcCircle = 0; 
	if(pSrcEntity->GetType() == enActor)
	{
		nSrcCircle = pSrcEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pSrcEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pSrcEntity)->GetHeroPtr();
		if (pData)
		{
			nSrcCircle = pData->data.bCircle;
		}
	}
	else if (pSrcEntity->GetType() == enMonster)
	{
		nSrcCircle = pSrcEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pSrcEntity->GetType() == enPet)
	{
		nSrcCircle = ((CPet*)pSrcEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}

	return (nTargetCircle - nSrcCircle) <= value.nValue;
}

//和目标的距离大于这个值
bool CSkillCondition::DistanceLarge(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pSrcEntity == NULL || pTargetEntity == NULL)
	{
		return false;
	}
	
	//为了支持坡上距离误差问题，距离使用x,或者y的相差的距离的绝对值的最大值作为距离
	int nSelfX,nSelfY, nTargetX,nTargetY;
	pSrcEntity->GetPosition(nSelfX,nSelfY);
	pTargetEntity->GetPosition(nTargetX,nTargetY);
	if( __max ( abs(nTargetX - nSelfX) ,abs(nTargetY-nSelfY) ) > value.nValue )
	{
		return true;
	}
	return false;
}

//目标存在buff
// value1=buff类型 , value2 =buff的组
 bool CSkillCondition::WithBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return pTargetEntity->GetBuffSystem()->Exists((GAMEATTRTYPE)value.wLo,value.wHi);
		
}
//目标不存在buff
 bool CSkillCondition::WithoutBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return !pTargetEntity->GetBuffSystem()->Exists((GAMEATTRTYPE)value.wLo,value.wHi);
}
// 驯服成功
 bool CSkillCondition::TrainOk(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return false;
}

//体力高于百分比,整数，用万分数表示,6000表示60%
 bool CSkillCondition::HpAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return (float)pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_HP)/
		pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP) *10000
	>= value.nValue ;
}
//体力低于百分比,整数，用万分数表示,6000表示60%	
 bool CSkillCondition::HpBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{

	return (float)pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_HP)/
		pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP) *10000
		< value.nValue ;
}
//活力高于百分比,整数，用万分数表示,6000表示60%
 bool CSkillCondition::MpAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return (float)pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_MP)/
		pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP) *10000
		>= value.nValue ;
}
// 活力低于百分比 ,整数，用万分数表示,6000表示60%
 bool CSkillCondition::MpBelow (CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	return (float)pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_MP)/
		pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP) *10000
		< value.nValue ;
}
// 帮派相同,value as bool
 bool CSkillCondition::SameGuild (CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pTargetEntity->GetType() !=enActor || pTargetEntity->GetType() != enActor) return false;
	return (pSrcEntity->GetProperty<unsigned int>( PROP_ACTOR_GUILD_ID) ==
		pTargetEntity->GetProperty<unsigned int>( PROP_ACTOR_GUILD_ID) );
}


 bool CSkillCondition::IsMaster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 bool flag =false;
	 bool confFlag = value.nValue ?true:false;
	 unsigned int nActorId;
	 if(pTargetEntity ==NULL || pTargetEntity->GetType() != enActor) 
	 {
		 return false == confFlag;
	 }

	 switch(pSrcEntity->GetType())
	 {
	 case enPet:
		 flag=( ((CPet*)pSrcEntity)->GetMaster() == pTargetEntity );
		 break;
	 case enHero:
		 flag=( ((CHero*)pSrcEntity)->GetMaster() == pTargetEntity );
		 break;
	 case enMonster:
	 case enGatherMonster:
		 nActorId =((CMonster*)pSrcEntity)->GetOwnerActorId() ;
		 if(nActorId)
		 {
			  flag= (nActorId == pTargetEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
		 }
		 else
		 {
			 flag= (((CMonster*)pSrcEntity)->GetOwner() == pTargetEntity->GetHandle());
		 }
	 }
	 return flag == confFlag;
 }



 //需要面向目标
 bool CSkillCondition::FaceTarget(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	  if(pTargetEntity == NULL || pSrcEntity ==NULL  ) return false;
	 INT_PTR nDir = pSrcEntity->GetTargetDir(pTargetEntity); //目标在自己的方位
	 int nSelfDir = pSrcEntity->GetProperty<int>(PROP_ENTITY_DIR); //自身的朝向
	 bool flag = abs( nSelfDir - nDir) < 4 ;
	 return flag == (value.nValue !=0);
 }

 //和目标的方向一致
 bool CSkillCondition::TargetOneDir(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pTargetEntity == NULL || pSrcEntity ==NULL  ) return false;
	 
	 int nDir = pTargetEntity->GetProperty<int>(PROP_ENTITY_DIR); //获取目标的朝
	 int nSelfDir= pSrcEntity->GetProperty<int>(PROP_ENTITY_DIR);
	 bool flag =  abs( nDir - nSelfDir)  < 4  ;
	 return flag == (value.nValue !=0);
 }

 //是否是特定的一个怪物
 bool CSkillCondition::IsSpecialMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	if(pTargetEntity == NULL || pSrcEntity ==NULL  ) return false;
	if(pTargetEntity->GetType() ==enMonster && pTargetEntity->GetProperty<int>(PROP_ENTITY_ID) ==value.nValue )
	{
		return true;
	} 
	else
	{
		return false;
	}
 }

 //阵营相同
 bool CSkillCondition::SelSameCamp(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	if(pSrcEntity ==NULL  || pTargetEntity ==NULL) return false;
	INT_PTR nSrcCampId= pSrcEntity->GetCampId();
	INT_PTR nTargetCampID = pTargetEntity->GetCampId();
	if(nSrcCampId == 0 || nTargetCampID ==0)
	{
		return (false == value.bValue);
	}
	else
	{
		return ( nSrcCampId  ==nTargetCampID)  == value.bValue;
	}
	
 }



//自身的hp比率高于一定的比率
 bool CSkillCondition::SelfHpRateAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 return (float)pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_HP)/
		 pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP) *10000
		 >= value.nValue ;
 }

//自身的hp比率低于一定的比例
 bool CSkillCondition::SelfHpRateBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 return (float)pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_HP)/
		 pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP) *10000
		 < value.nValue ;
 }



 //和自己是结拜的关系，这个暂时没实现
 bool CSkillCondition::SelfIsBrother(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	return false;
 }


 inline bool CSkillCondition::IsSpecialTypeMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pTargetEntity == NULL || pSrcEntity ==NULL  ) return false;
	 if(pTargetEntity->GetType() ==enMonster && ((CMonster*)(pTargetEntity))->GetMonsterType()==value.nValue )
	 {
		 return true;
	 } 
	 else
	 {
		 return false;
	 }
 }

 //最大魔法攻击>= 一个值
 inline bool CSkillCondition::IsMagicAttackMaxAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pTargetEntity == NULL   ) return false;
	 if(pTargetEntity->GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MAX) >= value.nValue)
	 {
		return true;
	 }
	 else
	 {
		return false;
	 }
 }

 //最大魔法攻击< 一个值
 inline bool CSkillCondition::IsMagicAttackMaxBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pTargetEntity == NULL   ) return false;
	 if(pTargetEntity->GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MAX) < value.nValue)
	 {
		 return true;
	 }
	 else
	 {
		 return false;
	 }
 }

 //最大道术攻击>= 一个值
 inline bool CSkillCondition::IsWizardAttackMaxAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pTargetEntity == NULL   ) return false;
	 if(pTargetEntity->GetProperty<int>(PROP_CREATURE_WIZARD_ATTACK_MAX) >= value.nValue)
	 {
		 return true;
	 }
	 else
	 {
		 return false;
	 }
 }

 //最大道术攻击< 一个值
 inline bool CSkillCondition::IsWizardAttackMaxBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pTargetEntity == NULL   ) return false;
	 if(pTargetEntity->GetProperty<int>(PROP_CREATURE_WIZARD_ATTACK_MAX) < value.nValue)
	 {
		 return true;
	 }
	 else
	 {
		 return false;
	 }
 }


 //自身的材料的id
 inline bool CSkillCondition::IsSelfMetiralItemId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pSrcEntity->GetType() != enActor) return false; //不是玩家无法使用
	 /* 
	 CUserItem * pItem =  ((CActor*)pSrcEntity)->GetEquipmentSystem().GetItemByType(Item::itMeterial); 

	 if(pItem == NULL)
	 {
		 return value.nValue ==0;
	 }
	 return pItem->wItemId ==  value.nValue ;
	 */
	 return true;
 }

 bool CSkillCondition::IsSpeciaWeapon(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pSrcEntity->GetType() != enActor) return false; //不是玩家无法使用

	 CUserItem * pItem =  ((CActor*)pSrcEntity)->GetEquipmentSystem().GetItemByType(Item::itWeapon); 

	 if(pItem == NULL)
	 {
		 return value.nValue ==0;
	 }
	 return pItem->wItemId ==  value.nValue ;

 }

 //是否是沙巴克成员
 bool  CSkillCondition::IsSbkMember(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	  if(pTargetEntity ==NULL || pTargetEntity->GetType() != enActor) return false; //不是玩家无法使用
	  CActor *pActor = (CActor * )pTargetEntity;
	  INT flag =1;

	  unsigned int nGuildid= pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	  if(nGuildid ==0)
	  {
		  flag =0;
	  }
	  else
	  {
		  if(GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId() !=nGuildid )
		  {
				flag =0;
		  }
	  }
	 
	  return ( flag == value.nValue);
 }

 //场景里是否存在路灯
//  bool CSkillCondition::HasLandscape(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
//  {
// 	 if(pTargetEntity ==NULL ) return false; //不是玩家无法使用
// 	 CScene *pScene = pSrcEntity->GetScene();
// 	 CEntityList &landList = pScene->GetLandscapeList(); //获取路灯列表
// 	 CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
// 	 CLinkedListIterator<EntityHandle> it(landList);
// 	 CLinkedNode<EntityHandle> *pNode = NULL;
// 	 for (pNode = it.first(); pNode; pNode = it.next())
// 	 {
// 		 CEntity *pEntity = pEntityMgr->GetEntity(pNode->m_Data);
// 		 if (pEntity && enLandscape == pEntity->GetType())
// 		 {
// 			 if(pEntity->GetProperty<int>(PROP_ENTITY_MODELID) == value.nValue)
// 			 {
// 				return true;	 
// 			 }
//			
// 		 }
// 	 }
// 	 return false;
//  }

//  bool  CSkillCondition::WithoutLandscape(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
//  {
// 	 return !HasLandscape(pSrcEntity,pTargetEntity,value);
//  }

 //在副本里，并且副本id为
 bool  CSkillCondition::FbId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pSrcEntity ==NULL) return false;
	 CFuBen * pFb=pSrcEntity->GetFuBen();
	 if(pFb ==NULL ) return false;
	 return pFb->GetFbId() == value.nValue;
 }


 //自身是怪物，并且怪物的id=value
 bool  CSkillCondition::SelfSpecialMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	  if(pSrcEntity ==NULL) return false;
	  if(pSrcEntity->GetType() != enMonster)
	  {
			return false;
	  }
	  return pSrcEntity->GetProperty<int>(PROP_ENTITY_ID) == value.nValue;
 }

 bool CSkillCondition::IsModelId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	   if(pSrcEntity ==NULL ) return false; //不是玩家无法使用
	   return pSrcEntity->GetProperty<int>(PROP_ENTITY_MODELID) == value.nValue;

 }

 //场景里是否存在怪物
 bool CSkillCondition::SceneHasMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pTargetEntity ==NULL ) return false; //不是玩家无法使用
	 CScene *pScene = pSrcEntity->GetScene();
	 if (!pScene) return false;
	 CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
	 CEntityList &landList = pScene->GetMonsterList(); //获取路灯列表
	 CLinkedListIterator<EntityHandle> it(landList);
	 CLinkedNode<EntityHandle> *pNode = NULL;
	 for (pNode = it.first(); pNode; pNode = it.next())
	 {
		 CEntity *pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
	std::map<EntityHandle,CMonster*>::iterator iter = pScene->GetMonsterList().begin();
	for (; iter != pScene->GetMonsterList().end() ; ++iter)
	{
		CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
		 if (pEntity)
		 {
			 if(pEntity->GetProperty<int>(PROP_ENTITY_ID) == value.nValue && !pEntity->IsDeath())
			 {
				 return true;	 
			 }

		 }
	 }
	 return false;
 }

 //场景不存在怪物
 bool CSkillCondition::SceneWithoutMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 return !SceneHasMonster(pSrcEntity,pTargetEntity,value);
 }

 //自己的等级 <= value
  bool CSkillCondition::SelfLevelBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 if(pSrcEntity ==NULL) return false;
	 INT_PTR nType = pSrcEntity->GetType();
	 if(nType ==enActor)
	 {
		if(pSrcEntity->GetProperty<int>(PROP_CREATURE_LEVEL)  <= value.nValue )
		{
			return true;
		}
		else
		{
			return false;
		}
	 }
	 else
	 {
		return pSrcEntity->GetProperty<int>(PROP_CREATURE_LEVEL)  <= value.nValue;
	 }
 }

 //己的等级 > value
  bool CSkillCondition::SelfLevelAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
 {
	 return !SelfLevelBelow(pSrcEntity,pTargetEntity,value);
 }

  //自身的场景的id
  bool CSkillCondition::SelfSceneId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
  {
	  if(pSrcEntity ==NULL ==false) return false;
	  return pSrcEntity->GetSceneID() == value.nValue;
  }

  bool CSkillCondition::SceneMonsterCount(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
  {
	  if(pSrcEntity ==NULL) return false;
	  CScene *pScene = pSrcEntity->GetScene();
	  if (pScene == NULL) return false;

	  if (pScene->GetLiveMonsterCount() <= value.nValue)
	  {
		  return true;
	  }

	  return false;

  }

bool CSkillCondition::MobPetCount(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pSrcEntity ==NULL || pSrcEntity->GetType() != enActor ) return false;
	
	 return ((CActor*)pSrcEntity)->GetPetSystem().GetPetCountBySkillId(value.wLo) < value.wHi;

}

bool CSkillCondition::EntityType(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pTargetEntity ==NULL  ) return false;

	return (pTargetEntity->GetType() == value.nValue);

}

bool CSkillCondition::EntityVocation(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if (pTargetEntity ==NULL) return false;

	unsigned int nVocation = 0;
	if(pTargetEntity->GetType() == enActor)
	{
		nVocation = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_VOCATION);
	}
	else if (pTargetEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pTargetEntity)->GetHeroPtr();
		if (pData)
		{
			nVocation = pData->data.bVocation;
		}
	}

	if (value.nValue == 0) return true;

	return (nVocation == value.nValue);

}

bool CSkillCondition::HasSpecialEquip(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if (pSrcEntity ==NULL) return false;

	if ( pSrcEntity->GetType() == enActor )
	{

		return ((CActor*)pSrcEntity)->GetEquipmentSystem().GetItemByType(value.nValue) !=NULL;

	}

	return false;
}

//目标转数 > value,
bool CSkillCondition::CircleAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	unsigned int nCircle = 0; 
	if(pTargetEntity->GetType() == enActor)
	{
		nCircle = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pTargetEntity)->GetHeroPtr();
		if (pData)
		{
			nCircle = pData->data.bCircle;
		}
		
	}else if (pTargetEntity->GetType() == enMonster)
	{
		nCircle = pTargetEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}else if (pTargetEntity->GetType() == enPet)
	{
		nCircle = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}
	
	return nCircle > value.uValue;
}



//目标转数 <= value
bool CSkillCondition::CircleBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	unsigned int nCircle = 0; 
	if(pTargetEntity->GetType() == enActor)
	{
		nCircle = pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enHero)
	{
		const CHeroSystem::HERODATA *pData = ((CHero *)pTargetEntity)->GetHeroPtr();
		if (pData)
		{
			nCircle = pData->data.bCircle;
		}

	}else if (pTargetEntity->GetType() == enMonster)
	{
		nCircle = pTargetEntity->GetProperty<int>(PROP_MONSTER_CIRCLE);
	}
	else if (pTargetEntity->GetType() == enPet)
	{
		nCircle = ((CPet*)pTargetEntity)->GetMaster()->GetProperty<int>(PROP_ACTOR_CIRCLE);
	}
	return nCircle <= value.uValue;
}


//自己身上有buff
bool CSkillCondition::SelfWithBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pSrcEntity ==NULL) return false;

	return pSrcEntity->GetBuffSystem()->Exists((GAMEATTRTYPE)value.wLo,value.wHi);

}

//不存在buff
bool CSkillCondition::SelfWithoutBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
{
	if(pSrcEntity ==NULL) return false;
	return !pSrcEntity->GetBuffSystem()->Exists((GAMEATTRTYPE)value.wLo,value.wHi);
}


bool CSkillCondition::SelfJobPet( CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value )
{
	if (pSrcEntity == NULL || pSrcEntity->GetType() != enActor)
	{
		return false;
	}
	if (pTargetEntity == NULL || pTargetEntity->GetType() != enPet)
	{
		return false;
	}
	return ((CActor *)pSrcEntity)->GetPetSystem().IsMyJobPet((CPet *)pTargetEntity);
}


bool CSkillCondition::SelfCircleBelow( CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value )
{
	if (pSrcEntity == NULL || pSrcEntity->GetType() != enActor)
	{
		return false;
	}
	if (pTargetEntity == NULL || pTargetEntity->GetType() != enActor)
	{
		return false;
	}
	int nMyCircle = pSrcEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
	int nTargetCircle = pTargetEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
	return nMyCircle - nTargetCircle <= value.nValue;
}


bool CSkillCondition::RevertCircleBelow( CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value )
{
	if (pSrcEntity == NULL || pSrcEntity->GetType() != enActor)
	{
		return false;
	}
	if (pTargetEntity == NULL || pTargetEntity->GetType() != enActor)
	{
		return false;
	}
	int nMyCircle = pSrcEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
	int nTargetCircle = pTargetEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
	return nTargetCircle - nMyCircle  <= value.nValue;
}

bool CSkillCondition::MonsterLevelUpCheck(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value)
{
	bool boResult = false;
	if (!pSrcEntity || pSrcEntity->GetType() != enMonster)
	{
		return boResult;
	}
	UINT nId = pSrcEntity->GetProperty<UINT>(PROP_ENTITY_ID);
	PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nId);
	if ( !pConfig )// || !pConfig->pflags->CanGrowUp)
	{
		return boResult;
	}
	UINT nBornLevel = pSrcEntity->GetProperty<UINT>(PROP_CREATURE_LEVEL);
	UINT nOriginalLevel = pConfig->nOriginalLevel;
	if ( nBornLevel < nOriginalLevel )
	{
		return boResult;
	}
	if ( nBornLevel - nOriginalLevel == value.nValue )
	{
		boResult = true;
	}
	return boResult;
}

bool CSkillCondition::ScenceHasSelfPet(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value)
{
	if (pSrcEntity == NULL || pSrcEntity->GetType() != enActor)
	{
		return false;
	}
	CScene *pScene = pSrcEntity->GetScene();
	if(pScene == NULL) return false;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CEntityList &petList = pScene->GetPetList();
	CLinkedListIterator<EntityHandle> it(petList);
	CLinkedNode<EntityHandle> *pNode = NULL;
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity *pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		if(pEntity  && enPet == pEntity->GetType())
		{
			if( ((CActor *)pSrcEntity)->GetPetSystem().IsMyJobPet((CPet *)pEntity) )
			{
				return true;
			}
		}
	}
	return false;
}

bool CSkillCondition::SelfSkillLevelAndTargetLevelAbove(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value)
{
	if ( !pSrcEntity || !pTargetEntity )
	{
		return false;
	}
	unsigned int nTargetLevel = pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	CSkillSubSystem::PSKILLDATA pSkillData = pSrcEntity->GetSkillSystem().GetSkillInfoPtr(value.nValue); 
	if (!pSkillData)
	{
		return false;
	}
	if ((unsigned int)pSkillData->nLevel * SKILLCON_PARAM >= nTargetLevel)
	{
		return true;
	}
	
	return  false;
}

bool CSkillCondition::SelfSkillLevelAndTargetLevelBelow(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value)
{
	if ( !pSrcEntity || !pTargetEntity )
	{
		return false;
	}
	unsigned int nTargetLevel = pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	CSkillSubSystem::PSKILLDATA pSkillData = pSrcEntity->GetSkillSystem().GetSkillInfoPtr(value.nValue); 
	if (!pSkillData)
	{
		return false;
	}
	if ((unsigned int)pSkillData->nLevel * SKILLCON_PARAM < nTargetLevel)
	{
		return true;
	}

	return  false;
}


 //////////////////////////////////////////////

 //没有条件
 INT_PTR   CSkillCondition::UtConditionNone(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
 {
	 return tmNoError;
 }


 //等级必须大于等于一定的级别
 INT_PTR  CSkillCondition::UtConditionLevel(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
 {

	//  int nCircle = 0;
	//  if(pActor->GetType() == enActor)
	//  {
	// 	 nCircle = pActor->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	//  }
	//  else if (pActor->GetType() == enHero)
	//  {
	// 	 const CHeroSystem::HERODATA *pData = ((CHero *)pActor)->GetHeroPtr();
	// 	 if (pData)
	// 	 {
	// 		 nCircle = pData->data.bCircle;
	// 	 }
	//  }
	//  if (nCircle >= 1)
	//  {
	// 	 return tpNoError;
	//  }

	 if(pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) < (unsigned int)contition.nValue)
	 {
		 return tmLevelLimited; //等级不够
	 }
	 return tmNoError;
 }

 //金钱
 INT_PTR  CSkillCondition::UtConditionCoin(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
 {
	 if (pActor->IsMonster() ) return tpNoError;
	 if(pActor->GetProperty<unsigned int>(PROP_ACTOR_COIN) < (unsigned int)contition.nValue)
	 {
		 return tmNoMoreCoin; //金钱不够
	 }
	 if(needConsume)
	 {
		 LPCSTR sSkillName = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(nSkillID);
		 ((CActor *)pActor)->ChangeCoin(-contition.nValue,GameLog::clLearnSkillMoney,0,sSkillName,true);
	 }
	 return tpNoError;
 }

 //消耗小飞鞋或回城卷
 INT_PTR  CSkillCondition::UtConditionFlyShoes(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
 {
	 if (pActor->IsMonster() ) return tpNoError;
	 if(pActor->GetProperty<unsigned int>(PROP_ACTOR_FLYSHOES) < (unsigned int)contition.nValue && 
	 	((CActor*)pActor)->GetBagSystem().GetItemCount(contition.nCount) <  contition.nValue)
	 {
		((CActor*)pActor)->SendTipmsgWithId(tmItemNotEnough,tstUI);
		//((CActor*)pActor)->SendTipmsg("小飞鞋数量不足",tstEcomeny);
		return tmItemNotEnough;
	 }
	 if(needConsume)
	 {
		if (contition.nCount > 0)
		{
			if( ((CActor*)pActor)->GetBagSystem().GetItemCount(contition.nCount) >=  contition.nValue)
			{
				CUserItemContainer::ItemOPParam param;
				param.wItemId = (WORD)contition.nCount;
				param.wCount =  contition.nValue;
				param.btQuality = -1;
				param.btStrong =-1;
				LPCSTR sSkillName = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(nSkillID);
				((CActor *)pActor)->GetBagSystem().DeleteItem(param,sSkillName,GameLog::Log_SkillCostItem);
				return tmNoError;
			}
		}
		 LPCSTR sSkillName = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(nSkillID);
		 ((CActor *)pActor)->ChangeFlyShoes(-contition.nValue,GameLog::Log_SkillCostItem,0,sSkillName,true);
	 }
	 return tmNoError;
 }

 //物品
 INT_PTR  CSkillCondition::UtConditionItem(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	 if (pActor ==NULL ||  pActor->GetType() !=enActor) return tpNoError;
	 
	 bool bNeedChcek =true;

	 if(bNeedChcek )
	 {
		 if( ((CActor*)pActor)->GetBagSystem().GetItemCount(contition.nValue)  <  contition.nCount)
		 {
			 return tmItemNotEnough; //物品不够
		 }
	 }
	 
	 if(needConsume)
	 {
		 CUserItemContainer::ItemOPParam param;
		 param.wItemId = (WORD)contition.nValue;
		 param.wCount =  contition.nCount;
		 param.btQuality = -1;
		 param.btStrong =-1;
		 LPCSTR sSkillName = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(nSkillID);
		 ((CActor *)pActor)->GetBagSystem().DeleteItem(param,sSkillName,GameLog::Log_LearnSkill);
	 }

	 return tmNoError;
 }

 //经验
 INT_PTR  CSkillCondition::UtConditionExp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	  if (pActor->IsMonster() ) return tpNoError;
	 if(pActor->GetProperty<Uint64>(PROP_ACTOR_EXP) <(Uint64) contition.nValue )
	 {
		 return tmExpLimited; //经验不够
	 }
	 if(needConsume)
	 {
		 ((CActor *)pActor)->DecExp(contition.nValue ); //降低exp
	 }
	 return tmNoError;
 }

 //内功等级
 INT_PTR  CSkillCondition::UtConditionInternalWorkLevel(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	if(pActor->GetType() != enActor)
	{
		return tpNoError;
	}
	if(pActor->GetProperty<unsigned int>(PROP_ACTOR_MERIDIALv) < contition.nValue)
	{
		return tmInternalWorkLevelLimit; //内功等级不足
	}
	return tmNoError;
 }

 //修为
 INT_PTR  CSkillCondition::UtConditionXiuwei(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	  if (pActor->IsMonster() ) return tmNoError;
	  /*
	 if(pActor->GetProperty<unsigned int>(PROP_ACTOR_XIUWEI) <(unsigned int)contition.nValue )
	 {
		 return tpSkillTrainXiuweiLimited; //修为不够
	 }
	 if(needConsume)
	 {
		 
		 ((CActor *)pActor)->ChangeXiuwei(- contition.nValue); //降低修为
	 }
	 */

	 return tmNoError;
 }


 //职业
 INT_PTR  CSkillCondition::UtConditionVocation(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	  if (pActor->IsMonster() ) return tmNoError;
	 if(pActor->GetProperty<unsigned int>(PROP_ACTOR_VOCATION) !=  (unsigned int)contition.nValue )
	 {
		 return tpSkillTrainVocationLimited; //职业
	 }
	 return tmNoError;
 }

 //HP是否足够
 INT_PTR  CSkillCondition::UtConditionHp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	 if(pActor->GetProperty<unsigned int>(PROP_CREATURE_HP) < (unsigned int)  contition.nValue)
	 {
		 return tpSkillSpellHpLimited;
	 }
	 if(needConsume)
	 {
		 pActor->ChangeHP(-1 *(int)contition.nValue );
	 }

	 return tmNoError;
 }

 //MP
 INT_PTR  CSkillCondition::UtConditionMp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	 if( pActor->GetProperty<unsigned int>(PROP_CREATURE_MP) <(unsigned int)  contition.nValue)
	 {
		 return tpSkillSpellMpLimited;
	 }
	 if(needConsume)
	 {
		 pActor->ChangeMP(-1 *(int)contition.nValue );
	 }
	 return tmNoError;
 }

 //目标存在
 INT_PTR  CSkillCondition::UtConditionTargetExist(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
 {
	 if(GetEntityFromHandle (pActor->GetTarget() )  ==NULL)
	 {
		 return tpSkillSpellNoTarget;
	 }
	 return tmNoError;
 }

 //宠物id=value的数量<param
 INT_PTR  CSkillCondition::UtConditionPetCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
 {
	 if(pActor->GetType() != enActor)
	 {
		 return tpSkillTrainPetCountLimit;
	 }
	 if( ((CActor*)pActor)->GetPetSystem().GetPetCountByConfigId((int)contition.nValue) >= contition.nCount)
	 {
		 return tpSkillTrainPetCountLimit;
	 }
	 return tpNoError;

 }

 //存在buff
 INT_PTR  CSkillCondition::UtConditionWithBuff(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {

	 if( pActor->GetBuffSystem()->Exists((const GAMEATTRTYPE)contition.nValue,-1) )
	 {
		 return tpNoError;
		 
	 }
	 else
	 {
		 return tpSkillTrainWithBuff;
	 }
 }

 //不存在buff
 INT_PTR CSkillCondition::UtConditionWithoutBuff(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
 {
	 if( pActor->GetBuffSystem()->Exists((const GAMEATTRTYPE)contition.nValue,-1)  )
	 {
		 return tpSkillTrainWithoutBuff;
	 }
	 return tpNoError;
 }

 //和目标的距离
 INT_PTR  CSkillCondition::UtConditionTargetDistance(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	 if(pActor->GetType() == enActor)
	 {
		 return tpNoError;
	 }
	 else
	 {
		 CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
		 if( pTarget ==NULL)
		 {
			 return tpSkillSpellNoTarget;
		 }
		 else
		 {
			 //为了支持坡上距离误差问题，距离使用x,或者y的相差的距离的绝对值的最大值作为距离
			 int nSelfX,nSelfY, nTargetX,nTargetY;
			 pActor->GetPosition(nSelfX,nSelfY);
			 pTarget->GetPosition(nTargetX,nTargetY);
			 if( __max ( abs(nTargetX - nSelfX) ,abs(nTargetY-nSelfY) ) > contition.nValue )
			 {
				 return tpSkillTrainDistance;
			 }

		 }
		 return tpNoError;
	 }
 }

 //和目标能否重合
 INT_PTR  CSkillCondition::UtConditionTargetSamePosition(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
 {
	 CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
	 if( pTarget ==NULL)
	 {
		 return tpSkillSpellNoTarget;
	 }
	 int nPosX, nPosY ;
	 pTarget->GetPosition(nPosX,nPosY); //获取目标的坐标
	 int nSelfPosX, nSelfPosY;
	 pActor->GetPosition(nSelfPosX,nSelfPosY); //获取自己的坐标
	 if(nPosX == nSelfPosX && nPosY ==nSelfPosY )
	 {
		 return tpSkillTrainOverLap;
	 }
	 
	 return tpNoError;
 }

//需要面向目标
INT_PTR  CSkillCondition::UtConditionFaceTarget(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
	if( pTarget ==NULL)
	{
		return tpSkillSpellNoTarget;
	}
	INT_PTR nDir = pActor->GetTargetDir(pTarget); //目标在自己的方位
	int nSelfDir = pActor->GetProperty<int>(PROP_ENTITY_DIR); //自身的朝向
	if( abs( nSelfDir - nDir) >=4 )
	{
		return tpSkillTrainFaceTarget;
	}
	return tpNoError;
}

//需要和目标一个方向
INT_PTR  CSkillCondition::UtConditionOneDirTarget(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
{
	CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
	if( pTarget ==NULL)
	{
		return tpSkillSpellNoTarget;
	}
	int nDir = pTarget->GetProperty<int>(PROP_ENTITY_DIR); //获取目标的朝
	int nSelfDir= pActor->GetProperty<int>(PROP_ENTITY_DIR);
	
	if( abs( nDir - nSelfDir) >=4 ) //方向相差不太多就行
	{
		return tpSkillTrainOneDirTarget;
	}
	return tpNoError;
}

//自身的血的百分比必须低于一定的值
INT_PTR  CSkillCondition::UtConditionHpRateBelow(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
{
	unsigned int nHp = pActor->GetProperty<unsigned int>(PROP_CREATURE_HP);
	unsigned int nMaxHp = pActor->GetProperty<unsigned int>(PROP_CREATURE_MAXHP);
	int nRate = (int)(  ((float)nHp /nMaxHp)*100 );
	if(nRate > contition.nValue )
	{
		return tpSkillTrainHpRateBelow;
	}
	return tpNoError;
}
INT_PTR CSkillCondition::UtConditionHpRateAbove(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
{
	if (pActor == NULL)
	{
		return tpSkillTrainHpRateAbove;
	}
	unsigned int nHp = pActor->GetProperty<unsigned int>(PROP_CREATURE_HP);
	unsigned int nMaxHp = pActor->GetProperty<unsigned int>(PROP_CREATURE_MAXHP);
	int nRate = (int)(  ((float)nHp /nMaxHp)*100 );
	if(nRate < contition.nValue )
	{
		return tpSkillTrainHpRateAbove;
	}
	return tpNoError;
}

//需要特定的目标
 INT_PTR   CSkillCondition::UtConditionSpecialTarget (CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
	if( pTarget ==NULL)
	{
		return tpSkillSpellNoTarget;
	}
	GAMEATTRVALUE value;
	value.bValue =true;
	if(contition.nValue == 1) //1表示敌人
	{
		if(pTarget->isAnimal()==false)
		{
			return tpSkillTrainEnemyTarget;
		}
		if(!IsEnermy(pActor,(CAnimal*)pTarget, value) )
		{
			return tpSkillTrainEnemyTarget;
		}
	}
	else if (contition.nValue == 2) //2表示朋友
	{
		if(pTarget->isAnimal()==false)
		{
			return tpSkillTrainFriendTarget;
		}
		if(!IsFriendly(pActor,(CAnimal*)pTarget, value) )
		{
			return tpSkillTrainFriendTarget;
		}
	}
	return tpNoError;
}

//目标可攻击
 INT_PTR   CSkillCondition::UtConditionTargetCanAttack (CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
	if( pTarget ==NULL)
	{
		return tpSkillSpellNoTarget;
	}
	if(pTarget->isAnimal()==false)
	{
		return tpNoError;
	}
	GAMEATTRVALUE value;
	value.bValue =true;
	if( ! CanAttack(pActor,(CAnimal*)pTarget,value))
	{
		return tpNoError;
	}

	return tpNoError;
}

 //需要玩家的跳跃距离
 INT_PTR   CSkillCondition::UtConditionJumpPower (CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
 {
	 /*
	 if(pActor->GetProperty<unsigned int>(PROP_ACTOR_JUMP_POWER) < (unsigned int)contition.nValue) 
	 {
		 return tpSkillTrainJumpPowerNotEnough;
	 }
	 if(needConsume)
	 {
		 pActor->SetProperty<unsigned int>(PROP_ACTOR_JUMP_POWER,pActor->GetProperty<unsigned int>(PROP_ACTOR_JUMP_POWER) - contition.nValue); //设置属性
	 }
	 */

	 return tmNoError;
 }


 //金钱
 INT_PTR  CSkillCondition::UtConditionBindCoin(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
 {
	 if (pActor->IsMonster() ) return tmNoError;
	 /*
	 if(pActor->GetProperty<unsigned int>(PROP_ACTOR_BIND_COIN) < (unsigned int)contition.nValue)
	 {
		 return tpSkillTrainBindcoinLimited; //金钱不够
	 }
	 */
	 if( ((CActor*)pActor)->HasEnoughBindCoin(contition.nValue) ==false)
	 {
		 return tmNoMoreCoin; //金钱不够
	 }

	 if(needConsume)
	 {
		 LPCSTR sSkillName = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(nSkillID);
		 ((CActor *)pActor)->ChangeBindCoin(-contition.nValue,GameLog::Log_LearnSkill,0,sSkillName,true); //绑定银两
		//((CActor *)pActor)->ConsumeCoin(contition.nValue,GameLog::clLearnSkillMoney,sSkillName,true,true);
	 }
	 return tmNoError;
 }


//阵营的称号
INT_PTR  CSkillCondition::UtConditionCampTitle(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	return tpSkillConfigError;	
}

//结拜的数目
INT_PTR  CSkillCondition::UtConditionBrotherCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	return tpSkillConfigError;
}

//与目标Y轴的绝对的距离
INT_PTR  CSkillCondition::UtConditionYAbsDis(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
	if( pTarget ==NULL)
	{
		return tpSkillSpellNoTarget;
	}
	int nSelfX,nSelfY;
	int nTargetX,nTargetY;
	pActor->GetPosition(nSelfX,nSelfY);
	pTarget->GetPosition(nTargetX,nTargetY);
	if(abs(nSelfY-nTargetY ) > contition.nValue)
	{
		return tpSkillTrainYDistance;
	}
	else
	{
		return tpNoError;
	}
}

INT_PTR CSkillCondition::UtConditionTeamBrotherCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	return tpNoError;
}



INT_PTR CSkillCondition::UtConditionBaseMaxMpRate(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	/*
	unsigned int nValue= (unsigned int) ( pActor->GetProperty<unsigned int>(PROP_CREATURE_BASE_MAXMP)*(float)contition.nValue/ (float)10000 );

	if( pActor->GetProperty<unsigned int>(PROP_CREATURE_MP)< nValue) 
	{
		return tpSkillSpellMpLimited;
	}
	if(needConsume)
	{
		 pActor->ChangeMP(-1 * nValue);
	}
	*/

	return tpNoError;

}



INT_PTR CSkillCondition::UtConditionBaseMaxHpRate(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	/*
	unsigned int nValue= (unsigned int) ( pActor->GetProperty<unsigned int>(PROP_CREATURE_BASE_MAXHP)*(float)contition.nValue/ (float)10000 );

	if( pActor->GetProperty<unsigned int>(PROP_CREATURE_HP)< nValue) 
	{
		return tpSkillSpellHpLimited;
	}
	if(needConsume)
	{
		pActor->ChangeHP(-1 * nValue);
	}
	*/

	return tpNoError;

}

//在与玩家的战斗状态,value 1表示是 0表示否
INT_PTR CSkillCondition::UtConditionBattleState(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	TICKCOUNT nCurrentick = GetGlobalLogicEngine()->getTickCount();
	if(contition.nValue ==0) //不在战斗状态才能使用
	{
		if( pActor->HasState(esStateBattle) &&pActor->GetTarget().GetType() ==enActor )
		{
			
			return tpSkillTrainInBattle;
		}
	}
	else //表示在战斗中才能使用
	{
		if( pActor->HasState(esStateBattle) ==false)
		{
			return tpSkillTrainNotInBattle;
		}
		else
		{
			/*
			EntityHandle handle = pActor->GetTarget();
			if( !handle.IsNull() && handle.GetType() == enActor && handle != pActor->GetHandle())
			{
				return tpNoError;
			}
			else
			{
				return tpSkillTrainNotInBattle;
			}
			*/
			return tpNoError;
		}
	}
	return tpNoError;
}

//消耗行会贡献
INT_PTR CSkillCondition::UtConditionGuildContr(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	if(pActor->GetType() != enActor)
	{
		return tpNoGuild;
	}

	CGuild* pGuild = ((CActor*)pActor)->GetGuildSystem()->GetGuildPtr();
	if ( !pGuild )
	{
		return tpNoGuild;
	}

	if(pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILDEXP) < (unsigned int)  contition.nValue)
	{
		return tpSkillTrainGuildContri;
	}
	if(needConsume)
	{
		((CActor*)pActor)->ChangeGuildContri(-1 *(int)contition.nValue );
	}

	return tpNoError;
}


//需要帮派对应的技能等级达到
INT_PTR  CSkillCondition::UtConditionGuildSkillLevel(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	if(pActor->GetType() != enActor)
	{
		return tpNoError;
	}	
	int nLevel = ((CActor*)pActor)->GetGuildSystem()->GetGuildSkillLevel((int)nSkillID); //获得指定的技能的等级
	if(contition.nValue > nLevel)
	{
		return tpSkillTrainGuildSkillLevel;
	}
	else
	{
		return tpNoError;
	}
}

INT_PTR   CSkillCondition::UtConditionXpValue(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	return tpNoError;
}

INT_PTR  CSkillCondition::UtConditionAngerValue(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	if(pActor->GetType() != enActor)
	{
		return tpNoError;
	}	

	int nValue = LOWORD(((CActor*)pActor)->GetProperty< int>(PROP_ACTOR_ANGER)); 
	if(contition.nValue > nValue)
	{
		return tpNoMoreAnger;
	}
	if(needConsume)
	{
		((CActor*)pActor)->RemoveConsume(qatAnger,0,contition.nValue); 	
	}
	return tpNoError;
}

INT_PTR  CSkillCondition::UtConditionMatiralDura(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	if(pActor->GetType() != enActor)
	{
		return tpNoError;
	}	
	/*
	//获取材料
	CUserItem * pItem =  ((CActor*)pActor)->GetEquipmentSystem().GetItemByType(Item::itMeterial); 

	if(pItem == NULL)
	{
		return tpNoMetiral;
	}
	int nCurrent =MAKELONG(pItem->wDura,pItem->wDuraMax); //当前玩家的耐久

	//材料耐久不够
	int nValue = (int)contition.nValue;
	if(nCurrent < nValue)
	{
		return tpMetiralDuraLimit;
	}
	if(needConsume)
	{
		((CActor*)pActor)->GetEquipmentSystem().ChangeEquipDua (pItem,-nValue); //改变xp值	
	}
	*/
	return tpNoError;
}



//需要装备有挖矿工具
INT_PTR  CSkillCondition::UtConditionDigTool(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	if(pActor->GetType() != enActor)
	{
		return tpNoError;
	}
	//获取武器的指针
	CUserItem *pUserItem =((CActor *) pActor)->GetEquipmentSystem().GetItemByType(Item::itWeapon);

	if(pUserItem ==NULL  )return tpSkillConditionDigTool;
	
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL) return false;
	// if(!pStdItem->m_Flags.canDig)
	// {
	// 	return tpSkillConditionDigTool;
	// }

	// //耐久没有了
	// if(pUserItem->wDura <= 0)
	// {
	// 	return tpSkillConditionDigToolNoDua;
	// }

	return tpNoError;
}

INT_PTR  CSkillCondition::UtConditionAreaProp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume)
{
	
	if(pActor->GetType() != enActor)
	{
		return tpNoError;
	}
	CScene *pScene = pActor->GetScene();
	if (!pScene) return tpNoError;

	int x,y;
	pActor->GetPosition(x,y);

	SCENEAREA* pArea = pScene->GetAreaAttri(x,y);	
	if(pScene->HasMapAttribute(x,y,(BYTE)contition.nValue,pArea))
	{
		return tpNoError;
	}
	else
	{
		return tpSkillConditionArea;
	}
}

INT_PTR CSkillCondition::UtConditionCircle( CAnimal * pActor, INT_PTR nSkillID, SKILLTRAINSPELLCONDITION &condition, bool needConsume )
{
	if (pActor->GetType() != enActor)
	{
		return tmNoError;
	}
	int nCircle = ((CActor *)pActor)->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	if( nCircle >= condition.nValue )
	{
		return tmNoError;
	}
	return tmCircleNotEnough;
}


//需要和目标一个方向
INT_PTR  CSkillCondition::UtConditionLineDistance(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
{
	CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
	if( pTarget ==NULL)
	{
		return tpSkillSpellNoTarget;
	}
	else
	{
		int nSelfX,nSelfY, nTargetX,nTargetY;
		pActor->GetPosition(nSelfX,nSelfY);
		pTarget->GetPosition(nTargetX,nTargetY);
		int nAbsX = abs(nTargetX - nSelfX);
		int nAbsY = abs(nTargetY - nSelfY);
		if (nAbsX == 0 && nAbsY == 0)
		{
			return tpSkillTrainOverLap;
		}
		if (nSelfX == nTargetX)
		{
			if (nAbsX <= contition.nValue )
			{
				return tpNoError;
			}
		}
		else if (nSelfY == nTargetY)
		{
			if (nAbsX <= contition.nValue )
			{
				return tpNoError;
			}
		}
		else if (nAbsX == nAbsY)
		{
			if (nAbsX <= contition.nValue )
			{
				return tpNoError;
			}
		}

		return tpSkillTrainDistance;
	}
}

//需要和目标一个方向
INT_PTR  CSkillCondition::UtConditionTargetDistanceAbove(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume )
{
	if(pActor->GetType() == enActor)
	{
		return tpNoError;
	}
	else
	{
		CEntity * pTarget= GetEntityFromHandle (pActor->GetTarget() );
		if( pTarget ==NULL)
		{
			return tpSkillSpellNoTarget;
		}
		else
		{
			//为了支持坡上距离误差问题，距离使用x,或者y的相差的距离的绝对值的最大值作为距离
			int nSelfX,nSelfY, nTargetX,nTargetY;
			pActor->GetPosition(nSelfX,nSelfY);
			pTarget->GetPosition(nTargetX,nTargetY);
			if( __max ( abs(nTargetX - nSelfX) ,abs(nTargetY-nSelfY) ) < contition.nValue )
			{
				return tpSkillTrainDistance;
			}

		}
		return tpNoError;
	}
}

//需要当前职业宠物数量>=value
INT_PTR CSkillCondition::UtConditionHasJobPetCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
{
	if(pActor->GetType() != enActor)
	{
		return tpSkillTrainNoHasJobPetCount;
	}
	if( ((CActor*)pActor)->GetPetSystem().GetJobPetCount() < contition.nValue )
	{
		return tpSkillTrainNoHasJobPetCount;
	}
	return tpNoError;

}

INT_PTR CSkillCondition::UtConditionGuildMainHall(CAnimal * pActor, INT_PTR nSkillId,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
{
	if ( pActor->GetType() != enActor )
	{
		return tpNoGuildMainHall;
	}
	CGuild* pGuild = ((CActor*)pActor)->GetGuildSystem()->GetGuildPtr();
	if ( !pGuild )
	{
		return tpNoGuildMainHall;
	}
	if (pGuild->m_MainHallLev < contition.nValue)
	{
		return tpNoGuildMainHall;
	}
	return tpNoError;
}


INT_PTR CSkillCondition::UtConditionGuildTech(CAnimal * pActor, INT_PTR nSkillId,SKILLTRAINSPELLCONDITION &contition ,bool needConsume)
{
	if ( pActor->GetType() != enActor )
	{
		return tpNoGuildTech;
	}
	CGuild* pGuild = ((CActor*)pActor)->GetGuildSystem()->GetGuildPtr();
	if ( !pGuild )
	{
		return tpNoGuildTech;
	}
	if (pGuild->m_btTechLevel < contition.nValue)
	{
		return tpNoGuildTech;
	}
	return tpNoError;
}
