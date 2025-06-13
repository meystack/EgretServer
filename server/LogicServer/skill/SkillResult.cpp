#include "StdAfx.h"
#include "SkillResult.h"
#include <algorithm>
#include "config/HallowsConfig.h"

//CSkillResult::SkillResultMgr *CSkillResult::s_skillResultMgr = new CSkillResult::SkillResultMgr;


//Buff计算属性的时候比率的计算规则
#define BUFF_RATE_TYPE_PHYSICAL_ATTACK 0  //物理攻击
#define BUFF_RATE_TYPE_MAGIC_ATTACK 1     //魔法攻击
#define BUFF_RATE_TYPE_WIZARD_ATTACK 2    //道术攻击
#define BUFF_RATE_TYPE_LEVEL 3			  //基于等级的

#ifdef _DEBUG
std::string s_HurtValueMsg;
char s_HurtValueBuff[100];
#endif

//增加技能效果，请在enSkillResultID中对应增加
const CSkillResult::SkillResultFunc CSkillResult::Handlers[]=
{
	&CSkillResult::IsError,					// 0	占位
	&CSkillResult::PhysicAttack,			// 1	物理攻击
	&CSkillResult::MagicAttack,				// 2	魔法攻击
	&CSkillResult::WizardAttack,			// 3	道术攻击
	&CSkillResult::AddBuff,					// 4	增加buff
	&CSkillResult::DefBuff,					// 5	删除buff
	&CSkillResult::CatchMonster,			// 6	抓怪
	&CSkillResult::BeatBack,				// 7	击退效果
	&CSkillResult::MobFire,					// 8	召唤火墙
	&CSkillResult::MobPet,					// 9	召唤宠物
	&CSkillResult::GoAhead,					// 10	往前跑若干格
	&CSkillResult::GoBack,					// 11	向后退若干格
	&CSkillResult::LastSafeAreaRTransfer,	// 12	上一个安全区随机传送
	&CSkillResult::IsError,					// 13	被动技能
	&CSkillResult::Cure,					// 14	瞬间治疗
	&CSkillResult::HolyAttack,				// 15	神圣攻击
	&CSkillResult::MoveToTarget,			// 16	瞬移到目标旁
	&CSkillResult::MoveBackHome,			// 17	瞬移回巢
	&CSkillResult::Transfer,				// 18	指定场景地点传送
	&CSkillResult::SceneTransfer,			// 19	本场景随机传送
	&CSkillResult::LastSafeAreaTransfer,	// 20	上一个安全区传送

	///////////////////////// 以下暫無用

	&CSkillResult::DeleteEffect,			// 21	删除特效
	&CSkillResult::IgnoreDefence,			// 22	忽略目标的防御
	&CSkillResult::CatchTarget,				// 23	把目标抓到自己身边
	&CSkillResult::SetAttackTarget,			// 24	设置攻击目标
	&CSkillResult::MobEntity,				// 25	刷怪
	&CSkillResult::RapidMove,				// 26	冲锋
	&CSkillResult::SelfHpCureTarget,		// 27	自身的HP的百分比治疗目标
	&CSkillResult::LostTarget,				// 28	丢弃目标
	&CSkillResult::ChangEquipDua,			// 29	改变装备的耐久
	&CSkillResult::DropMonsterItem,			// 30	按怪物的掉率掉落东西
	&CSkillResult::ScreenShake,				// 31	震屏
	&CSkillResult::IsError,					// 32
	&CSkillResult::ChangeModel,				// 33	换个模型
	&CSkillResult::RateChangeProperty,		// 34	百分比改属性
	&CSkillResult::SelfDelSkillCd,			// 35	删除技能的CD
	&CSkillResult::GiveAward,				// 36	给奖励
	&CSkillResult::BeatFly,					// 20	击飞效果
	&CSkillResult::PlayGuide,				// 38	客户端播放引导性特
	&CSkillResult::IgnoreMaxDropHp,			// 39   忽略每次最大伤害限制
	&CSkillResult::PointMove,				// 40	把目标移动到地图一个点
	&CSkillResult::SelfPointMove,			// 41	自己移动到范围内的鼠标指定的点
	&CSkillResult::TargetRapidMove,			// 42	向某目标冲撞
	&CSkillResult::RemovePet,				// 43	删除宠物
	&CSkillResult::ChangeMonsterAttacked,	// 44	改变怪物是否能被攻击
	&CSkillResult::FastSelfCurHp,			// 45	瞬回血
	&CSkillResult::TargetPointMove,			// 46	把目标移动到范围内的鼠标指定的点
	&CSkillResult::CatchAll,				// 47	抓取所有符合条件对象
	&CSkillResult::MonsterMove,				// 48   怪物移动
	&CSkillResult::Shout,					// 49   怪物喊话
};

void CSkillResult::DoResult( PSKILLRESULTPARAM pParam,bool bNoDelay)
{
	if( pParam->pTargetEntity)
	{
		if( pParam->pTargetEntity->GetType() == enNpc) return; //Npc不能受到所有技能的影响
	}

	//技能在2,3,4步 距离误差的时候命中的概率
	static int s_nSkill2StepRate = GetLogicServer()->GetDataProvider()->GetGlobalConfig().n2StepSkillHitRate;
	static int s_nSkill3StepRate = GetLogicServer()->GetDataProvider()->GetGlobalConfig().n3StepSkillHitRate;
	static int s_nSkill4StepRate = GetLogicServer()->GetDataProvider()->GetGlobalConfig().n3StepSkillHitRate;

	INT_PTR nCmdID = pParam->pSkillResult->nResultType;
	if( nCmdID >=0 && nCmdID <ArrayCount(Handlers) )
	{
		if(bNoDelay || pParam->pSkillResult->nDelay ==0)
		{
			int nPosX,nPosY;
			if(pParam->pTargetEntity &&  pParam->pTargetEntity !=pParam->pSrcEntity )
			{
				if(pParam->pTargetEntity->IsInited() ==false) return;

				//不在同1个场景不执行
				//if(pParam->pTargetEntity->GetScene() != pParam->pSrcEntity->GetScene()) return;

				//技能在2,3,4步 距离误差的时候命中的概率
				pParam->pTargetEntity->GetPosition(nPosX,nPosY);
				if(pParam->nTargetPosX && pParam->nTargetPosY )
				{
					int nDisX = abs(nPosX -pParam->nTargetPosX);
					int nDisY = abs(nPosY -pParam->nTargetPosY);
					int nDis = __max(nDisX,nDisY); //取2个的最大值

					if( !pParam->pSkillResult->ignoreTargetDis && nDis >=2) //如果距离大于2格以上
					{
						//怪物维持原来的，只有玩家，宠物，和狗维持原来的
						if(pParam->pSrcEntity && pParam->pSrcEntity->GetType() == enMonster )
						{
							return ;
						}

						int nRand =wrand(10000); //取1万之间的随机数
						bool isHit =false;
						//根据技能和当前的点之间的距离，按概率计算是否命中，2，3,4格有一定概率命中，1格直接命中，5格以上直接不命中了
						switch(nDis)
						{
						case 2:
							if( nRand <= s_nSkill2StepRate )
							{
								isHit =true;
							}
							break;
						case 3:
							if( nRand <= s_nSkill3StepRate )
							{
								isHit =true;
							}
							break;
						case 4:
							if( nRand <= s_nSkill4StepRate )
							{
								isHit =true;
							}
							break;
						default:
							break;
						}
						if(!isHit) return; //没有命中
					}

				}

			}
			( this->*CSkillResult::Handlers[nCmdID] )(pParam);
		}
		else
		{
			INT_PTR nDelay;
			//取施法者的攻击速度
			if(pParam->pSkillResult->nDelay <0)
			{
				//倍率
				nDelay =- pParam->pSkillResult->nDelay *
					pParam->pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_ATTACK_SPEED);

			}
			else
			{
				nDelay = pParam->pSkillResult->nDelay;
			}
			CAnimal * pEntity = pParam->pTargetEntity;
			if(pEntity ==NULL)
			{
				pEntity = pParam->pSrcEntity;
			}
			if(pEntity)
			{
				CEntityMsg msg(CEntityMsg::emSkillResult, pParam->pSrcEntity->GetHandle());
				msg.dwDelay = nDelay;
				msg.nParam1 =pParam->nSkillID;
				msg.nParam2 = pParam->nTargetPosX;
				msg.nParam3= pParam->nTargetPosY;
				msg.nParam4= pParam->btIgnoreDefence;
				msg.nParam5= pParam->btIgnoreMaxDropHp;

				msg.nParam5 = pParam->pSkillResult->nValue;
				msg.nParam6 = pParam->pSkillResult->nId;
				msg.nParam7 = pParam->pSkillResult->nResultType;
				//msg.nParam8 = pParam->wCriticalDamgeAdd;
				//msg.nParam9 = pParam->wHitRateAdd;
				pEntity->PostEntityMsg(msg);
			}
		}
	}
}

int CSkillResult::GetAttackValByType(CAnimal * pSrcActor, int nType, int nParam )
{
	int nAbility = 0;
	switch(nType)
	{
	case BUFF_RATE_TYPE_PHYSICAL_ATTACK :
		nAbility =  pSrcActor->GetAttackValue(TYPE_PHYSICAL_ATTACK) ;
		break;
	case BUFF_RATE_TYPE_MAGIC_ATTACK:
		nAbility = pSrcActor->GetAttackValue(TYPE_MAGIC_ATTACK) ;
		break;
	case BUFF_RATE_TYPE_WIZARD_ATTACK:
		nAbility =  pSrcActor->GetAttackValue(TYPE_WIZARD_ATTACK) ;
		break;
	case BUFF_RATE_TYPE_LEVEL:
		nAbility =  pSrcActor->GetProperty<int>(PROP_CREATURE_LEVEL) ;
		break;
	default:
		break;
	}
	return nAbility;
}

 void  CSkillResult::AddBufProcess(PSKILLRESULTPARAM pParam, float fTimeDesc)
 {
	CBuffProvider::BUFFCONFIG * pBuffCfg = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(pParam->pSkillResult->nId);
	if (pBuffCfg == NULL)
	{
		return;
	}
	GAMEATTRVALUE gameValue;
	memset(&gameValue, 0, sizeof(gameValue));
	int nAbility = 0;

	if(pParam->pSkillResult->nParam1)
	{
		if (!(pParam->pSkillResult->nParam1 >= wrand(10000)))
		{
			return;
		}
	}

	if (AttrDataTypes[pBuffCfg->nType] == adFloat)
	{
		gameValue.fValue += (float)pBuffCfg->dValue;
	}
	else
	{
		gameValue.nValue += (int)pBuffCfg->dValue;
	}
	if(pParam->pTargetEntity ==NULL) return;

	CDynamicBuff dyBuff;
	dyBuff.wBuffId = pParam->pSkillResult->nId;
	dyBuff.value = gameValue;
	dyBuff.dwTwice = pBuffCfg->nTimes;
	dyBuff.wActWait = pBuffCfg->nInterval;
	dyBuff.pConfig = pBuffCfg;
	if (pParam->pSrcEntity != NULL)
	{
		dyBuff.mGiver = pParam->pSrcEntity->GetHandle();
	}
	pParam->pTargetEntity->GetBuffSystem()->Append(dyBuff.wBuffId, &dyBuff);

	//雷霆一击--致残特效、高级致残特效
	if ( pParam->pSkillResult->nId == 324 || pParam->pSkillResult->nId == 325 )
	{
		CEntityMsg msg(CEntityMsg::emShowEntityEffect, pParam->pSrcEntity->GetHandle());
		
		switch ( pParam->pSkillResult->nId )
		{
		case 324: //雷霆一击--致残
			msg.nParam1 = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nCrippleEffectId;
			break;
		case 325: //雷霆一击--高级致残
			msg.nParam1 = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nAdvancedCrippleEffectId;
			break;
		default:
			break;
		}

		pParam->pTargetEntity->PostEntityMsg(msg);
	}
 }

//添加buff
 inline void  CSkillResult::AddBuff(PSKILLRESULTPARAM pParam)
{
	AddBufProcess(pParam,0.0);
}

//删除buff
void  CSkillResult::DefBuff(PSKILLRESULTPARAM pParam)
{
	if(pParam->pTargetEntity && pParam->pTargetEntity->IsInited() && pParam->pTargetEntity->isAnimal())
	{
		pParam->pTargetEntity->GetBuffSystem()->RemoveById(pParam->pSkillResult->nId);
	}
}

//抓怪
void  CSkillResult::CatchMonster(PSKILLRESULTPARAM pParam)
{
	if(pParam->pTargetEntity && pParam->pTargetEntity->IsInited() && pParam->pTargetEntity->isAnimal())
	{
		if (pParam->pTargetEntity->GetType() != enMonster) return;
		if (pParam->pTargetEntity->GetAttriFlag().DenyBeCatch == true) return;

		if(!(pParam->pSrcEntity)) return ;
		CActor *pActor = ((CActor *)(pParam->pSrcEntity));
		if (pActor->GetType()!=enActor) { return ;}
		INT_PTR nSkillLevel = pActor->GetSkillSystem().GetSkillLevel(pParam->nSkillID);
		const SKILLONELEVEL * pLevel = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(pParam->nSkillID,nSkillLevel);
		unsigned int nPetNum = pActor->GetPetSystem().GetPetCount(); //当前宠物数量
		if(pLevel->nMaxPet >=1)
		{  //配置了技能等级相关的宠物数量限制
			if( nPetNum >= pLevel->nMaxPet )
			{
				pActor->SendTipmsgFormatWithId(tmCannotCatchPetMore, tstUI);
				return ;
			}
		}
		int nCatchRate = rand() % 10000;
		if(pParam->pSkillResult->nId > nCatchRate)
		{

			INT_PTR nEntityId = pParam->pTargetEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
			INT_PTR nLevel = pParam->pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
			int nPosX = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSX);
			int nPosY = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSY);

			CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
			em->DestroyEntity(pParam->pTargetEntity->GetHandle());

			int nAiId = pParam->pSkillResult->nParam1;
			int nLiveSecs = pParam->pSkillResult->nValue;


			pActor->GetPetSystem().AddPet(nEntityId, nLevel, nAiId, nLiveSecs, 0, 0, nPosX, nPosY);
		}
	}
}

bool CSkillResult::isHit( PSKILLRESULTPARAM pParam ,INT_PTR  nAttackType )
{
	//延时作用，有可能人物已下线了
	if (!pParam->pSrcEntity || !pParam->pTargetEntity)
	{
		return false;
	}
	if(pParam->pSrcEntity ==pParam->pTargetEntity  ) return true;

	/*
		情况一：攻击者准确>目标的敏捷，100%命中
		情况二：攻击者准确<=目标的敏捷，闪避率公式=（敏捷-准确+1）/（敏捷+1）
		注意，无论是物理还是魔法都要通过这个判断才能命中目标
	*/
	bool isHit =false;
	unsigned int HitValue = pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_HITVALUE); //准确
	unsigned int nDogValue = pParam->pTargetEntity->GetProperty<int>(PROP_CREATURE_DODVALUE); //敏捷
	if(HitValue ==0) return false; //准确为0就没法名字了

	if(HitValue > nDogValue )
	{
		isHit =true;
	}
	else
	{
		if(   HitValue >= (unsigned int)( wrand (nDogValue+1) +1)  )
		{
			isHit =true;
		}
	}
	/*
	注意：先判断准确与敏捷是否能够命中，如果命中，再判断魔法的闪避
	魔法命中几率 = 1 - （魔法命中-魔法闪避）
	中毒命中几率 = 1 - （魔法命中-毒物闪避）
	*/

	/*
		如果是法师或者道士的攻击，需要根据攻击类型判断命中，
		道士和法师的攻击分为的魔法攻击和读物攻击
			魔法命中几率 = 1 - （魔法命中-魔法闪避）
			中毒命中几率 = 1 - （魔法命中-毒物闪避）
	*/

	if(!isHit || nAttackType == TYPE_PHYSICAL_ATTACK || nAttackType == TYPE_HOLY_ATTACK) //这里没有命中的话，那么就直接不命中了
	{
		return isHit;
	}

	int nRate;

	if(nAttackType == TYPE_MAGIC_ATTACK || nAttackType == TYPE_WIZARD_ATTACK) //如果是魔法攻击的话
	{
		nRate= 10000+ pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_MAGIC_HITRATE) -
			 pParam->pTargetEntity->GetProperty<int>(PROP_CREATURE_MAGIC_DOGERATE) ;

	}

	// unsigned int nRate = (unsigned int)(fRate * 10000);
	isHit = nRate <= 0 ? false : wrand(10000) < nRate;	//这里躲闪超过

	if (!isHit)
		return isHit;	//这里没有命中的话，那么就直接不命中了

	//火墙伤害判定防火率
	if (pParam->pSkillResult->btHitType == htFire)
	{
		WORD nDefenseRate = pParam->pTargetEntity->m_nFireDefenseRate;
		if (nDefenseRate > 0 && nDefenseRate <= 10000)	//超过1W就就找策划吧
		{
			isHit = wrand(10000) < (unsigned int)(10000 - nDefenseRate);
		}
		else
		{
			isHit = true;
		}
	}

	return isHit;

}

//内功攻击
void  CSkillResult::PhysicAttack(PSKILLRESULTPARAM pParam)
{
	NormalAttack(pParam,TYPE_PHYSICAL_ATTACK);
}

void CSkillResult::HolyAttack(PSKILLRESULTPARAM pParam)
{
	NormalAttack(pParam,TYPE_HOLY_ATTACK);
}


void CSkillResult::MoveToTarget(PSKILLRESULTPARAM pParam)
{
	if(!pParam->pSrcEntity || !pParam->pTargetEntity)
	{
		return;
	}
	unsigned int nSceneID =  pParam->pSrcEntity->GetSceneID();
	unsigned int nTargetSceneId = pParam->pTargetEntity->GetSceneID();
	if (nSceneID != nTargetSceneId)
	{
		return;
	}

	CScene* pScene = pParam->pSrcEntity->GetScene();
	if (!pScene) return;
	INT_PTR nEntityType = pParam->pSrcEntity->GetType();

	INT_PTR nDist = 1;//pParam->pSkillResult->nId;
	INT_PTR nMovePosX,nMovePosY;
	INT_PTR nTarPosX,nTarPosY;
	pParam->pSrcEntity->GetPosition(nTarPosX,nTarPosY);
	pParam->pTargetEntity->GetPosition(nMovePosX,nMovePosY);
	INT_PTR nDir = CEntity::GetDir(nTarPosX,nTarPosY,nMovePosX,nMovePosY);
	CEntityGrid::NewPosition(nMovePosX,nMovePosY,(nDir+4)%DIR_STOP,1);
	if (pScene->CanCross(nEntityType,nMovePosX,nMovePosY,true))
	{
		pParam->pTargetEntity->GetPosition(nTarPosX,nTarPosY);
		nDir = CEntity::GetDir(nMovePosX,nMovePosY,nTarPosX,nTarPosY);
		//pParam->pSrcEntity->SetDir(nDir);//GetAttriFlag().DenySetDir
		pParam->pSrcEntity->SetProperty<unsigned int>(PROP_ENTITY_DIR,(unsigned int)nDir);
	}
	else
	{
		pParam->pTargetEntity->GetPosition(nTarPosX,nTarPosY);
		int nLeftRight[2] = { nTarPosX - nDist, nTarPosX + nDist };
		int nTopDown[2] = { nTarPosY - nDist, nTarPosY + nDist };
		bool flag = false;
		// 左右两边
		for (nTarPosX = nLeftRight[0]; nTarPosX <= nLeftRight[1] && !flag; nTarPosX++)
		{
			for (int i = 0; i < 2 && !flag; i++)
			{
				nTarPosY = nTopDown[i];
				if(pScene->CanCross(nEntityType,nTarPosX,nTarPosY,true))
				{
					flag = true;
					break;
				}
			}
		}
		// 上下两边
		for (nTarPosY = nTopDown[0]; nTarPosY <= nTopDown[1] && !flag; nTarPosY++)
		{
			for (int i = 0; i < 2 && !flag; i++)
			{
				nTarPosX = nLeftRight[i];
				if(pScene->CanCross(nEntityType,nTarPosX,nTarPosY,true))
				{
					flag = true;
					break;
				}
			}
		}

		if (flag)
		{
			nMovePosX = nTarPosX;
			nMovePosY = nTarPosY;
			pParam->pTargetEntity->GetPosition(nTarPosX,nTarPosY);
			nDir = CEntity::GetDir(nMovePosX,nMovePosY,nTarPosX,nTarPosY);
			pParam->pSrcEntity->SetProperty<unsigned int>(PROP_ENTITY_DIR,(unsigned int)nDir);
		}
		else
		{
			pParam->pTargetEntity->GetPosition(nMovePosX,nMovePosY);
		}
	}

	//CFuBen* hFb = pParam->pSrcEntity->GetFuBen();
	//GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(pParam->pSrcEntity,hFb,nSceneID,nMovePosX,nMovePosY);
	int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
	((CAnimal*)pParam->pSrcEntity)->GetMoveSystem()->InstantMove(nMovePosX,nMovePosY,1,enDefaultTelePort,nEffId);
}

void CSkillResult::MoveBackHome(PSKILLRESULTPARAM pParam)
{
	if(!pParam->pSrcEntity || pParam->pSrcEntity->GetType() != enMonster)
	{
		return;
	}

	unsigned int nSceneID =  pParam->pSrcEntity->GetSceneID();
	INT_PTR nTargetPosX,nTargetPosY;
	((CMonster*)pParam->pSrcEntity)->GetBornPoint(nTargetPosX,nTargetPosY);
	CFuBen* hFb = pParam->pSrcEntity->GetFuBen();
	//GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(pParam->pSrcEntity,hFb,nSceneID,nTargetPosX,nTargetPosY);
	if (pParam->pSrcEntity->GetAttriFlag().DenySetDir)
	{
		unsigned int nMonsterId = pParam->pSrcEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
		PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
		if (pMonsterConfig)
		{
			if (pMonsterConfig->btDir >= 0 && pMonsterConfig->btDir <=7)
			{
				pParam->pSrcEntity->SetProperty<unsigned int>(PROP_ENTITY_DIR,(unsigned int)pMonsterConfig->btDir);
			}
		}
	}

	int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
	((CAnimal*)pParam->pSrcEntity)->GetMoveSystem()->InstantMove(nTargetPosX,nTargetPosY,1,enDefaultTelePort,nEffId);
}

void CSkillResult::OnDodge(CEntity * pSrcEntity,CEntity * pTargetEntity)
{
	Uint64 uTargetHandle = (Uint64)pTargetEntity->GetHandle();
	if(pSrcEntity&& pSrcEntity->GetType() == enActor)
	{
		CActorPacket pack;
		CDataPacket & data = ((CActor*)pSrcEntity)->AllocPacket(pack);
		data << (BYTE)enDefaultEntitySystemID << (BYTE)sDoDge << uTargetHandle;
		pack.flush();
	}

	if(pTargetEntity && pTargetEntity->GetType() == enActor)
	{
		CActorPacket pack;
		CDataPacket & data = ((CActor*)pTargetEntity)->AllocPacket(pack);
		data << (BYTE)enDefaultEntitySystemID << (BYTE)sDoDge << uTargetHandle;
		pack.flush();
	}
}

//魔法攻击
void  CSkillResult::MagicAttack(PSKILLRESULTPARAM pParam)
{
	NormalAttack(pParam,TYPE_MAGIC_ATTACK);
}


//设定攻击目标
void CSkillResult::SetAttackTarget(PSKILLRESULTPARAM pParam)
{
	if(pParam->pTargetEntity == NULL || pParam->pSrcEntity == NULL) return;
	if(pParam->pTargetEntity == pParam->pSrcEntity) return;
	if(pParam->pTargetEntity->GetAI() == NULL) return;

	pParam->pTargetEntity->GetAI()->AttackStart( pParam->pSrcEntity);
}

//把目标抓到自己身边
void  CSkillResult::CatchTarget(PSKILLRESULTPARAM pParam)
{
	CEntity * pEntity = GetEntityFromHandle(pParam->pSrcEntity->GetTarget());
	if(pEntity && pEntity->isAnimal() )
	{
		if(pEntity->GetAttriFlag().DenyBeMove) return; //如果不能移动的话就不能抓取
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		((CAnimal*)pEntity)->GetMoveSystem()->InstantMove(pParam->nTargetPosX,pParam->nTargetPosY,1,enDefaultTelePort,nEffId); //瞬间移动
	}
}

//瞬间回血
void CSkillResult::Cure(PSKILLRESULTPARAM pParam)
{
	if(pParam->pTargetEntity ==NULL || pParam->pSrcEntity ==NULL) return;
	if(pParam->pTargetEntity->IsDeath()) return; //死亡的不能加血

	// 回血量
	int nValue = pParam->pSrcEntity->GetAttackValue(TYPE_WIZARD_ATTACK);
	nValue = (float)pParam->pSkillResult->nId/10000 * nValue  + pParam->pSkillResult->nValue;

	// 回复上限
	if( pParam->pSkillResult->nParam1 && pParam->pSkillResult->nParam1 > nValue)
	{
		nValue = pParam->pSkillResult->nParam1;
	}

	// 加血
	pParam->pTargetEntity->ChangeHP(nValue);
}

//冲锋
void CSkillResult::RapidMove(PSKILLRESULTPARAM pParam)
{
	INT_PTR nStep = pParam->pSkillResult->nId; //跳跃的距离
	if(nStep ==0) return;
	pParam->pSrcEntity->GetMoveSystem()->RapidMove((int)nStep);
}

void CSkillResult::LostTarget(PSKILLRESULTPARAM pParam)
{
	if( NULL == pParam->pTargetEntity ) return ;

	if( pParam->pTargetEntity->GetType() ==enActor)
	{
		CActorPacket pack;
		((CActor*)pParam->pTargetEntity)->AllocPacket(pack);
		pack <<(BYTE) enDefaultEntitySystemID << (BYTE) sLostTarget << Uint64(pParam->pTargetEntity->GetTarget());
		pack.flush();
	}
	pParam->pTargetEntity->SetTarget(EntityHandle(0));
}


//删除特效
void CSkillResult::DeleteEffect(PSKILLRESULTPARAM pParam)
{
	if( NULL == pParam->pTargetEntity ) return ;
	if( pParam->pTargetEntity->isAnimal())
	{
		((CAnimal*)pParam->pTargetEntity)->GetEffectSystem().DelEffect(pParam->pSkillResult->nValue,
			pParam->pSkillResult->nId);
	}
}
//刷怪
void CSkillResult::MobEntity(PSKILLRESULTPARAM pParam)
{
	if(pParam->nTargetPosX ==0 || pParam->nTargetPosY ==0) return;
	if(pParam->pTargetEntity ==NULL) return;

	CScene * pScene = pParam->pTargetEntity->GetScene();
	if(pScene ==NULL )return;
	INT_PTR nPosX = (INT_PTR)pParam->nTargetPosX ;
	INT_PTR nPosY = (INT_PTR)pParam->nTargetPosY;
	INT_PTR nEntityType = pParam->pSkillResult->nParam1; //实体的类型
	if(nEntityType == enActor || nEntityType == enNpc)return;
	INT_PTR nTotalCount = pParam->pSkillResult->nValue;  //刷出怪的数量
	//x,y,如果策划配置了点，那就在策划的点上刷
	int nConfigX = pParam->pSkillResult->nParam2;
	int nConfigY = pParam->pSkillResult->nParam3;
	if(nConfigX && nConfigY )
	{
		nPosX = nConfigX;
		nPosY = nConfigY;
	}
	for(INT_PTR i = 0; i < nTotalCount;i ++)
	{
		bool flag = false;
		if( pParam->pSkillResult->nParam4 >0) //在一个范围里随机
		{
			unsigned int nTwoR =2*  pParam->pSkillResult->nParam4 +1; //这个半径
			int       nR = pParam->pSkillResult->nParam4;

			INT_PTR nCenterX = nPosX;
			INT_PTR nCenterY = nPosY;

			for(INT_PTR j=0; j< 10000; j++)
			{
				INT_PTR x = (INT_PTR)wrand(nTwoR) - nR + nPosX;
				INT_PTR y =(INT_PTR)wrand(nTwoR) - nR + nPosY;
				if(pScene->canEntityMove((int)x,(int)y, (int)nEntityType,0,0,0))
				{
					nPosX = x;
					nPosY = y;
					flag = true;
					break;
				}
			}
		}
		else
		{
			if(nConfigX && nConfigY)
			{
				flag = pScene->GetGrid().GetMoveablePoint(NULL,pScene,nConfigX,nConfigY,nPosX,nPosY,false,nEntityType);
				if(flag ==false)
				{
					if( pScene->CanMove(nConfigX,nConfigY))
					{
						nPosX = nConfigX;
						nPosY = nConfigY;
						flag =true;
					}
				}
			}
			else
			{
				flag = true;
			}
		}
		if(flag ==false)
		{
			OutputMsg(rmError,_T("Skill MobEntity Error,position error,skillid=%d,mid=%d,sid=%d,x=%d,y=%d"),
				(int)pParam->nSkillID,pParam->pSkillResult->nId,pScene->GetSceneId(),(int)nPosX,(int)nPosY);
			continue;
		}

		if(nEntityType == enMonster || nEntityType == enGatherMonster)
		{
			int nInitPercent = pParam->pSkillResult->nParam6;
			if (nInitPercent == 0)
			{
				nInitPercent = 100;
			}
			CEntity* pEntity = pScene->CreateEntityAndEnterScene(pParam->pSkillResult->nId,
				nEntityType,nPosX ,nPosY,-1,NULL, pParam->pSkillResult->nParam5,0,NULL,0,nInitPercent);
			if (pEntity == NULL)
			{
				OutputMsg(rmError,_T("Skill MobEntity Error,create entity error,skillid=%d,mid=%d,sid=%d,x=%d,y=%d"),
					(int)pParam->nSkillID,pParam->pSkillResult->nId,pScene->GetSceneId(),(int)nPosX,(int)nPosY);
			}
		}
	}

}
 void CSkillResult::ScreenShake(PSKILLRESULTPARAM pParam)
{
	if(pParam->pTargetEntity ==NULL )return;
	if(pParam->pTargetEntity->GetType() != enActor) return;
	CActorPacket pack;
	((CActor*)pParam->pTargetEntity)->AllocPacket(pack);
	pack <<(BYTE) enDefaultEntitySystemID << (BYTE) sScreenShake << (WORD) pParam->pSkillResult->nId;
	pack << (WORD) pParam->pSkillResult->nValue;
	pack.flush();
}

//自身的HP的比例治疗目标
void CSkillResult::SelfHpCureTarget(PSKILLRESULTPARAM pParam)
{
	if(pParam->pSrcEntity == pParam->pTargetEntity ) return; //自己不能治疗自己，否则就是白费
	if(pParam->pSrcEntity->IsDeath() || pParam->pTargetEntity->IsDeath()) return ; //有一方死了都不行
	//如果对方的血是满的话，那么就不浪费自己的血
	if(pParam->pTargetEntity->GetProperty<int>(PROP_CREATURE_HP) == pParam->pTargetEntity->GetProperty<int>(PROP_CREATURE_MAXHP))
	{
		return ;
	}
	if(pParam->pSkillResult->nId <=0) return;
	float rate = (float)pParam->pSkillResult->nId /10000;  //牺牲自己的血量
	float addRate = (float)pParam->pSkillResult->nValue /10000;  //给别人加的血量

	int nHpAdd =(int) ( pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_HP) *rate);

	pParam->pSrcEntity->ChangeHP(-nHpAdd);
	nHpAdd = (int) (nHpAdd * addRate); //按比例给对方
	pParam->pTargetEntity->ChangeHP(nHpAdd);
}

//删除技能的CD
void CSkillResult::SelfDelSkillCd(PSKILLRESULTPARAM pParam)
{
	if(pParam->pSrcEntity ==NULL) return;
	int nSkillId = pParam->pSkillResult->nId;
	if(nSkillId <=0) return ; //配置不合法

	pParam->pSrcEntity->GetSkillSystem().DelSkillCdTemporary(nSkillId);
}

//击飞
void CSkillResult::BeatFly(PSKILLRESULTPARAM pParam)
{
	INT_PTR nStep = pParam->pSkillResult->nId;
	if(nStep ==0) return;
	INT_PTR nTargetPosX = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nTargetPosY = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSY);

	//如果怪物自己把自己击退的话，就往后退几格
	CEntity *pSrc = pParam->pSrcEntity ;
	if(pParam->pSrcEntity ==pParam->pTargetEntity )
	{
		pSrc =GetEntityFromHandle(pParam->pSrcEntity->GetTarget());
	}

	if(pSrc ==NULL) return;

	//当前的坐标
	INT_PTR nCurrentPosx =  pSrc->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  pSrc->GetProperty<int>(PROP_ENTITY_POSY);


	//如果坐标不变的话
	if(nCurrentPosx == nTargetPosX && nCurrentPosy == nTargetPosY) return;

	INT_PTR nDir = CEntity::GetDir(nCurrentPosx,nCurrentPosy,nTargetPosX,nTargetPosY);
	pParam->pTargetEntity->GetMoveSystem()->BeatBack(nDir,nStep,false);
}

//击退
 void  CSkillResult::BeatBack(PSKILLRESULTPARAM pParam)
{
	INT_PTR nStep = pParam->pSkillResult->nId; //跳跃的距离
	if(nStep ==0) return;
	INT_PTR nTargetPosX = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nTargetPosY = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSY);

	// 非怪物，在安全区不能弹出去
	INT_PTR entityType = pParam->pTargetEntity->GetType();
	if (entityType != enMonster)
	{
		CScene* pActorScene = pParam->pTargetEntity->GetScene();
		if (pActorScene && pActorScene->HasMapAttribute(nTargetPosX,nTargetPosY,aaSaft))
		{
			return;
		}
	}

	//如果怪物自己把自己击退的话，就往后退几格
	CEntity *pSrc = pParam->pSrcEntity ;
	if(pParam->pSrcEntity ==pParam->pTargetEntity )
	{
		pSrc =GetEntityFromHandle(pParam->pSrcEntity->GetTarget());
	}

	if(pSrc ==NULL) return;

	//当前的坐标
	INT_PTR nCurrentPosx =  pSrc->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  pSrc->GetProperty<int>(PROP_ENTITY_POSY);

	//如果坐标不变的话
	if(nCurrentPosx == nTargetPosX && nCurrentPosy == nTargetPosY) return;

	INT_PTR nDir = CEntity::GetDir(nCurrentPosx,nCurrentPosy,nTargetPosX,nTargetPosY);
	pParam->pTargetEntity->SetDir((nDir +4)  %DIR_STOP); //取相反的方向
	pParam->pTargetEntity->GetMoveSystem()->BeatBack(nDir,nStep,true);
}

inline void CSkillResult::NormalAttack(PSKILLRESULTPARAM pParam, int nAttackType)
{
	//某些怪物只能被特定技能伤害
	if (pParam->pSrcEntity && pParam->pSrcEntity->GetType() == enActor && pParam->pTargetEntity && pParam->pTargetEntity->GetType() == enMonster)
	{
		PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(pParam->pTargetEntity->GetId());
		if ( pMonsterConfig->wDamageSkill > 0  && pMonsterConfig->wDamageSkill != pParam->nSkillID)
		{
			LPCSTR sSkillName = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(pMonsterConfig->wDamageSkill);
			if (sSkillName != NULL)
			{
				((CActor*)pParam->pSrcEntity)->SendOldTipmsgFormatWithId(tpNotDamageBySkill, ttFlyTip, sSkillName);
			}
			return;
		}
	}

	bool isMonster = bool(pParam->pSrcEntity? (pParam->pSrcEntity->GetType() == enMonster) : false);
	bool isActor = bool(pParam->pSrcEntity? (pParam->pSrcEntity->GetType() == enActor) : false);

	//命中
	if(isHit(pParam, nAttackType))
	{
		//职业攻击
		int nValue = pParam->pSrcEntity->GetAttackValue(nAttackType);

		if ((nValue >= 10000) && isMonster)
		{
			int nScene = pParam->pSrcEntity->GetSceneID();
			int nEntityId= pParam->pSrcEntity->GetProperty<int>(PROP_ENTITY_ID);
			if (nEntityId != 171)
			{
				int nX= pParam->pSrcEntity->GetProperty<int>(PROP_ENTITY_POSX);
				int nY= pParam->pSrcEntity->GetProperty<int>(PROP_ENTITY_POSY);
				int nPhysAttackMax= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MIN);
				int nPhysAttackMin= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MAX);
				int nMagicAttackMax= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MAX);
				int nMagicAttackMin= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MIN);
				OutputMsg(rmError,_T("[Monster Attack1] 怪物攻击过高（ID:%d Atk:%d） 技能参数（SId:%d, AtkType:%d）  坐标（%d,%d,%d）  物理（%d,%d）  魔法（%d,%d）；"),
				nEntityId,nValue,
				pParam->nSkillID,nAttackType,
				nScene,nX,nY,
				nPhysAttackMin,nPhysAttackMin,
				nMagicAttackMax,nMagicAttackMin);
			}
		}

		#ifdef _DEBUG
		// S1
		s_HurtValueMsg.clear();
		SNPRINTFA(s_HurtValueBuff,100,"S1::%d",nValue);
		s_HurtValueMsg += s_HurtValueBuff;
		#endif

		//伤害值
		float fValue = 0;

		//技能攻击上限和下限
		if(pParam->pSkillResult->nParam1 && pParam->pSkillResult->nParam1 >pParam->pSkillResult->nValue)
		{
			int nAddValue = pParam->pSkillResult->nValue + wrand(pParam->pSkillResult->nParam1 - pParam->pSkillResult->nValue +1);
			fValue = (float)pParam->pSkillResult->nId/10000 * nValue  + nAddValue;
		}
		else
		{
			fValue = (float)pParam->pSkillResult->nId/10000 * nValue  + pParam->pSkillResult->nValue;
		}

		//兵魂-霜之哀伤
		if ((nAttackType == TYPE_PHYSICAL_ATTACK || nAttackType == TYPE_MAGIC_ATTACK || nAttackType == TYPE_WIZARD_ATTACK ))
		{
			if( pParam->pSkillResult->btHitType != htFire )
			{
				if(isActor)
				{
					HallowsLorderInfo *pHallowsLorderInfo = ((CActor*)pParam->pSrcEntity)->GetHallowsSystem().GetHallowsLoderv(0, eHallowsSSC_SZAS);
					if(pParam->pSrcEntity->m_nFrozenStrength > 0 && pHallowsLorderInfo && pHallowsLorderInfo->nLv > 0)
					{
						int nFrozenRate = 0;
						//触发几率：施法者冰冻强度 - 目标冰冻强度(施法者冰冻强度与目标冰冻强度 相等时，施法者增加冰冻强度(万分比))；最高几率8%
						if(pParam->pSrcEntity->m_nFrozenStrength >= pParam->pTargetEntity->m_nFrozenStrength)
						{
							nFrozenRate = std::max(pParam->pSrcEntity->m_nFrozenStrength - pParam->pTargetEntity->m_nFrozenStrength, GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nFrozenAddValue);
							nFrozenRate = std::min(nFrozenRate, GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nMaxFrozen);

							if(wrand(10000) < nFrozenRate)
							{
								//目标为怪物时,怪物攻击降低10%,
								if(pParam->pTargetEntity->GetType() == enMonster)
								{
									std::vector<int>::iterator iter = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vReduceAttackMon.begin();
									for(; iter != GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vReduceAttackMon.end(); ++iter)
									{
										pParam->pTargetEntity->GetBuffSystem()->Append(*iter);
									}
								}

								//已处于冰冻效果时不可再次被冰冻
								static int nFrozenBuffId = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nFrozenBuffId;
								CBuffProvider::BUFFCONFIG * pBUFFCONFIG = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nFrozenBuffId);
								if (pBUFFCONFIG)
								{
									CDynamicBuff *pDynamicBuff = pParam->pTargetEntity->GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
									if(!pDynamicBuff)
									{
										pParam->pTargetEntity->GetBuffSystem()->Append(nFrozenBuffId);

										//触发冰冻时额外扣除与施法者冰冻值相等的额外伤害
										int nFrozenTimes = 5;
										if ( GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nFrozenTimes > 0 )
										{
											nFrozenTimes = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nFrozenTimes;
										}
										pParam->pTargetEntity->m_nSZASAttckValue = pParam->pSrcEntity->m_nFrozenStrength * nFrozenTimes;
									}
								}
							}
						}
					}
				}
			}
		}

		// 这里将法/道的职业的普攻速万分比加到伤害增益上
		if (isActor && (nAttackType == TYPE_MAGIC_ATTACK || nAttackType == TYPE_WIZARD_ATTACK))
		{
			// 必须属性伤害增益万分比有值， 普攻增速万分比才有效
			if (pParam->pSkillResult->nId > 0)
			{
				float atk_acc = pParam->pSrcEntity->GetProperty<int>(PROP_ACTOR_NORMAL_ATK_ACC);
				fValue = fValue + (atk_acc/10000 * nValue);
			}
		}

		//兵魂-剧毒裁决
		if ((nAttackType == TYPE_PHYSICAL_ATTACK || nAttackType == TYPE_MAGIC_ATTACK || nAttackType == TYPE_WIZARD_ATTACK ))
		{
			if( pParam->pSkillResult->btHitType != htFire )
			{
				if(isActor)
				{
					HallowsLorderInfo* pHallowsLorderInfo = ((CActor*)pParam->pSrcEntity)->GetHallowsSystem().GetHallowsLoderv(0, eHallowsSSC_JDCJ);
					if(pHallowsLorderInfo && pHallowsLorderInfo->nLv > 0)
					{
						//触发几率
						if(wrand(10000) < pParam->pSrcEntity->m_nToxicRate)
						{
							//触发效果：额外绝对值伤害
							pParam->pTargetEntity->m_nToxicAttckValue = pParam->pSrcEntity->m_nToxicDamage;

							std::vector<int>::iterator iter = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vPoisonBuff.begin();
							for(; iter < GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vPoisonBuff.end(); ++iter)
							{
								pParam->pTargetEntity->GetBuffSystem()->Append(*iter);
							}
						}
					}
				}
			}
		}

		//内功
		if ( isActor )
		{
			bool isInternalWork = false;
			switch (pParam->nSkillID)
			{
			case enInternalWorkSkillsId_HalfMoon:				//半月弯刀
			case enInternalWorkSkillsId_Fire:					//烈火剑法
			case enInternalWorkSkillsId_DayByDay:				//逐日剑法
			case enInternalWorkSkillsId_Thunder:				//雷电术
			case enInternalWorkSkillsId_IceStorm:				//冰咆哮
			case enInternalWorkSkillsId_RainFlowSpark:			//流星火雨
			case enInternalWorkSkillsId_SoulFireSign:			//灵魂火符
			case enInternalWorkSkillsId_BloodBite:				//嗜血术
				isInternalWork = true;
				break;
			default:
				break;
			}

			if ( isInternalWork )
			{
				if (pParam->pTargetEntity->GetType() == enActor)
				{
					fValue = fValue + pParam->pSrcEntity->GetProperty<unsigned int>(InternalWorkIncreaseDamageMap[pParam->nSkillID]) - pParam->pTargetEntity->GetProperty<unsigned int>(InternalWorkReduceDamageMap[pParam->nSkillID]);
				} else {
					fValue = fValue + pParam->pSrcEntity->GetProperty<unsigned int>(InternalWorkIncreaseDamageMap[pParam->nSkillID]);
				}
			}
		}


		#ifdef _DEBUG
		// S2
		SNPRINTFA(s_HurtValueBuff,100," //S2::%d",(int)fValue);
		s_HurtValueMsg += s_HurtValueBuff;
		#endif

		if ((fValue >= 10000.0f) && isMonster)
		{
			int nScene = pParam->pSrcEntity->GetSceneID();
			int nEntityId= pParam->pSrcEntity->GetProperty<int>(PROP_ENTITY_ID);
			if (nEntityId != 171)
			{
				int nX= pParam->pSrcEntity->GetProperty<int>(PROP_ENTITY_POSX);
				int nY= pParam->pSrcEntity->GetProperty<int>(PROP_ENTITY_POSY);
				int nPhysAttackMax= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MIN);
				int nPhysAttackMin= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MAX);
				int nMagicAttackMax= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MAX);
				int nMagicAttackMin= pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MIN);
				OutputMsg(rmError,_T("[Monster Attack2] 怪物攻击过高（ID:%d Atk:%f） 技能参数（SId:%d, AtkType:%d）  坐标（%d,%d,%d）  物理（%d,%d）  魔法（%d,%d）；"),
				nEntityId,fValue,
				pParam->nSkillID,nAttackType,
				nScene,nX,nY,
				nPhysAttackMin,nPhysAttackMin,
				nMagicAttackMax,nMagicAttackMin);
			}
		}

		//触发附加效果(麻痹)
		if ((nAttackType == TYPE_PHYSICAL_ATTACK || nAttackType == TYPE_MAGIC_ATTACK || nAttackType == TYPE_WIZARD_ATTACK ))
		{
			//概率触发麻痹逻辑 --对方如果是麻痹中这不触发麻痹
			if(!((CAnimal*)pParam->pTargetEntity)->GetDizzyState() &&pParam->pSrcEntity->m_nDizzyValue >0) {
				if( pParam->pSkillResult->btHitType != htFire  /*&&  pParam->pSrcEntity->m_nDizzyBuffInterval <= GetGlobalLogicEngine()->getMiniDateTime() */)  {

					//M1=(（麻痹几率-抗麻几率）/10000)*职业麻痹系数
					//M1=Min{M1,30%}
					//M2=（麻痹时长增加-麻痹时长减免）
					//M2=Min{M2,7000}
					int nCoefficient = 10000;
					if(pParam->pSrcEntity->GetType() == enActor) {
						CActor* pActor = (CActor*)pParam->pSrcEntity;
						if(pActor) {
							nCoefficient = GetLogicServer()->GetDataProvider()->GetGlobalConfig().GetJobDizzyRate(pActor->GetJob()-1);
						}
					}

					int nDizzyRate = (pParam->pSrcEntity->m_nDizzyValue - pParam->pTargetEntity->m_nDeductDizzyValue)/* nCoefficient/10000.0*/;
					int nDizzyBuffMaxRate = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nDizzyBuffMaxRate;
					nDizzyRate = std::min(nDizzyRate, nDizzyBuffMaxRate);
					//麻痹命中了
					if( nDizzyRate >= 0 && wrand(10000) <= nDizzyRate) {
						int nDizzyTime = pParam->pSrcEntity->m_nDizzyTimeAdd - pParam->pTargetEntity->m_nDizzyTimeAbsorbAdd;
						int nDizzyBuffMaxTime = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nDizzyBuffMaxTime;
						nDizzyTime = std::min(nDizzyTime, nDizzyBuffMaxTime);
						if(nDizzyTime > 0) {
							((CAnimal*)pParam->pTargetEntity)->SetDizzyTime(nDizzyTime);
							if(pParam->pSrcEntity && pParam->pTargetEntity) {
								static int nParalyEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nParalyEffId;
								if(nParalyEffId > 0 ) {
									CEntityMsg msg(CEntityMsg::emShowEntityEffect, pParam->pSrcEntity->GetHandle());
									msg.nParam1 = nParalyEffId;
									pParam->pTargetEntity->PostEntityMsg(msg);
								}
							}
						}
						// if (pParam->pSrcEntity->GetType() == enActor)
						// {
						// 	((CActor *)(pParam->pSrcEntity))->SendOldTipmsgWithId(tpDizzyActive, ttChatWindow);
						// }
					}

				}
			}
		}

		if (nAttackType == TYPE_HOLY_ATTACK)
		{
			//神圣攻击
			((CCreature *)pParam->pTargetEntity)->ChangeHP(-1 * fValue, pParam->pSrcEntity, true, false, true);
		}
		else
		{
			CalculateDamage( pParam->nSkillID, pParam->pSrcEntity, pParam->pTargetEntity, pParam, fValue, nAttackType);
		}


#ifdef _DEBUG
		if (GetGlobalLogicEngine()->IsPrintDamageInfo() && (pParam->pSrcEntity->GetType() == enActor))
		{
			s_HurtValueMsg += ";;";
			CActor *pActor = ((CActor*)pParam->pSrcEntity);
			CActorPacket pack;
			CDataPacket &data = pActor->AllocPacket(pack);
			data << (BYTE)enChatSystemID << (BYTE)sSendChat << (BYTE)1;
			data.writeString(pParam->pTargetEntity->GetShowName());
			data.writeString(s_HurtValueMsg.c_str());
			data << pActor->GetLevel();
			data << Uint64(pActor->GetHandle());
			data <<(unsigned int)(0);
			data << (BYTE)0;
			pack.flush();
		}
#endif

	}
	//无命中
	else OnDodge(pParam->pSrcEntity,pParam->pTargetEntity);
}

inline void CSkillResult::WizardAttack(PSKILLRESULTPARAM pParam)
{
	NormalAttack(pParam,TYPE_WIZARD_ATTACK);
}

//忽略防御
inline void CSkillResult::IgnoreDefence(PSKILLRESULTPARAM pParam)
{
	pParam->btIgnoreDefence =1; //忽略目标的防御
}

//召唤火出来
inline void CSkillResult::MobFire(PSKILLRESULTPARAM pParam)
 {
	 if(pParam->pSkillResult->nValue <=0) return;
	 if ( !pParam->pSrcEntity )
	 {
		 return;
	 }
	 int nCurPosX,nCurPosY;
	 pParam->pSrcEntity->GetPosition(nCurPosX,nCurPosY);
	 if (abs(nCurPosX - pParam->nTargetPosX) >MOVE_GRID_ROW_RADIO  || abs(nCurPosY - pParam->nTargetPosY) > MIN_MOVE_GRID_COL_RADIO)
	 {
		return;
	 }
	 float rate = (float)pParam->pSkillResult->nParam2 /10000;
	 float addValue = (float)(pParam->pSkillResult->nParam3 * 1000);

	 //火墙的存活时间，单位ms
	 int nLiveTime =(int) ( (int)(pParam->pSrcEntity->GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MAX) *rate) + addValue );

	/*
	*
	*	2021-08-20 火墙 暂时性修改
	
	// 清除上一次召唤的火墙
	std::deque<EntityHandle>& LastFireSkillEntitys = ((CActor*)pParam->pSrcEntity)->m_deqLastFireSkillEntity;
	for ( ;LastFireSkillEntitys.size() > 0; LastFireSkillEntitys.pop_front() )
	{
		EntityHandle hLastFireEntity =  LastFireSkillEntitys.front();
		CFire* pFire = (CFire*)GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(hLastFireEntity);
		CActor* pMaster = NULL;
		if (pFire && (pMaster = (CActor*)pFire->GetMasterPtr()))
		{
			if (pParam->pSrcEntity->GetHandle() == pMaster->GetHandle())
			{
				GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(hLastFireEntity,false);
			}
		}
	}

	*/ 

	// 创建9个火的位置
	static int s_matrix[][2] =
	{
		{-1,-1}, {0,-1}, {1,-1},
		{-1,0},  {0,0},  {1,0},
		{-1,1},  {0,1},  {1,1}
	};

	for(INT_PTR j=0; j< ArrayCount(s_matrix); j++)
	{
		int x = s_matrix[j][0];
		int y = s_matrix[j][1];

		 //创建法师的火
		CFire * pFire = CFire::CreateFire(pParam->pSrcEntity->GetScene(),pParam->nTargetPosX + x,pParam->nTargetPosY + y,pParam->pSrcEntity,
			pParam->pSkillResult->nId,
			nLiveTime,
			pParam->pSkillResult->nValue,
			pParam->pSkillResult->nParam1);

		/*
		*
		*	2021-08-20 火墙 暂时性修改

		//记录火
		if (pFire)
		{
			((CActor*)pParam->pSrcEntity)->m_deqLastFireSkillEntity.push_back(pFire->GetHandle());
		} else
		{
			((CActor*)pParam->pSrcEntity)->m_deqLastFireSkillEntity.push_back(0);
		}

		*/
	}
 }

//召唤宠物
inline void  CSkillResult::MobPet(PSKILLRESULTPARAM pParam)
{
	int nPetId = pParam->pSkillResult->nId;
	int nLevel = pParam->pSkillResult->nValue;
	int nAiId = pParam->pSkillResult->nParam1;
	int nInheritPercent = pParam->pSkillResult->nParam2;
	if(nPetId <=0 || nLevel<=0) return;
	if( pParam->pSrcEntity ==NULL) return;
	if(pParam->pSrcEntity->GetType() != enActor) return;
	CActor *pActor = (CActor*)pParam->pSrcEntity;
	pActor->GetPetSystem().AddPet(nPetId,nLevel, nAiId, 0, nInheritPercent,pParam->nSkillID);
}

//删除自身宠物
inline void CSkillResult::RemovePet(PSKILLRESULTPARAM pParam)
{
	int nPetId = pParam->pSkillResult->nId;

	if (nPetId <=0) return;

	if (pParam->pSrcEntity ==NULL) return;
	if (pParam->pSrcEntity->GetType() != enActor) return;

	((CActor*)pParam->pSrcEntity)->GetPetSystem().RemoveAllPetByConfigId(nPetId);

}

//自己往前方走一格
void  CSkillResult::GoAhead(PSKILLRESULTPARAM pParam)
{
	int nStep = pParam->pSkillResult->nValue;
	int nDir = pParam->pSrcEntity->GetProperty<int>(PROP_ENTITY_DIR); //按当前的方向往前面走
	int nInterval   =pParam->pSkillResult->nParam1; //表示移动一格的速度(单位ms)
	int nSimulateReboundSpeed = 1; //如果是1表示需要失败模拟

	CScene  *pScene = pParam->pSrcEntity->GetScene();
	if(pScene ==NULL) return;

	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();

	// 当前点
	int nCurrentPosx ,nCurrentPosy;
	pParam->pSrcEntity->GetPosition(nCurrentPosx,nCurrentPosy);
	int nCurPosX = nCurrentPosx, nCurPosY = nCurrentPosy;
	//bool hasEntityBlock	 =false;

	int nRealStep = 0;

	for (size_t i = 0; i < nStep; i++)
	{
		// 新的点
		INT_PTR nNewPox = nCurPosX;
		INT_PTR nNewPoy = nCurPosY;
		CEntityGrid::NewPosition(nNewPox,nNewPoy,nDir,1);

		// //是否有传送门
		// bool hasTransPoint =false;
		// if(pScene->IsTelePort(nNewPox,nNewPoy))
		// {
		// 	hasTransPoint = true;
		// }

		//不能穿过实体
		bool hasEntityBlock = false;
		const MapEntityGrid *pGrid=pScene->GetMapEntityGrid((int)nNewPox);

#ifndef GRID_USE_STL_MAP
		for(INT_PTR i=0; i< pGrid->m_entitys.count(); i++)
		{
			if(pGrid->m_entitys[i].pos != nNewPoy) continue;
			EntityHandle& hHandle = pGrid->m_entitys[i].hd;
#else
		const EntityPosVector& entitys = pGrid->m_entitys;
		typedef typename EntityPosVector::const_iterator Iter;
		Iter it = entitys.begin(), end = entitys.end();
		for (; it != end; it++)
		{
			if((*it).second != nNewPoy) continue;
			const EntityHandle& hHandle = (*it).first;
#endif
			switch(hHandle.GetType())
			{
			case enActor:
			case enMonster:
			case enNpc:
			case enPet:
			case enHero:
				CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
				if (pEntity && !pEntity->IsDeath())
					hasEntityBlock = true;
				break;
			}
		}
		if(hasEntityBlock || !pScene->CanMove(nNewPox,nNewPoy)) break;

		nRealStep++;
		nCurPosX = nNewPox;
		nCurPosY = nNewPoy;
	}
	pParam->pSrcEntity->GetMoveSystem()->Move(nDir,nCurrentPosx,nCurrentPosy,nRealStep,0,true,nInterval,sMoveAhead,0,nSimulateReboundSpeed);
}

//往后走若干格
void  CSkillResult::GoBack(PSKILLRESULTPARAM pParam)
{
	INT_PTR nStep = pParam->pSkillResult->nValue;
	int nInterval   =pParam->pSkillResult->nParam1;
	if(nStep ==0)
	{
		return;
	}
	INT_PTR nTargetPosX = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nTargetPosY = pParam->pTargetEntity->GetProperty<int>(PROP_ENTITY_POSY);

	// 非怪物，在安全区不能弹出去
	INT_PTR entityType = pParam->pTargetEntity->GetType();
	if (entityType != enMonster)
	{
		CScene* pActorScene = pParam->pTargetEntity->GetScene();
		if (pActorScene && pActorScene->HasMapAttribute(nTargetPosX,nTargetPosY,aaSaft))
		{
			return;
		}
	}

	int nId = pParam->pSkillResult->nId;  //如果失败了添加值为id的技能标记
	int nTime = pParam->pSkillResult->nParam2;  //技能标记的时间
	//如果怪物自己把自己击退的话，就往后退几格
	CEntity *pSrc = pParam->pSrcEntity ;
	if(pParam->pSrcEntity ==pParam->pTargetEntity )
	{
		return ;
	}

	if(pSrc ==NULL)
	{
		return;
	}

	//当前的坐标
	INT_PTR nCurrentPosx =  pSrc->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  pSrc->GetProperty<int>(PROP_ENTITY_POSY);

	//如果坐标不变的话
	if(nCurrentPosx == nTargetPosX && nCurrentPosy == nTargetPosY)
	{
		return;
	}

	INT_PTR nDir = CEntity::GetDir(nCurrentPosx,nCurrentPosy,nTargetPosX,nTargetPosY);

	CScene *pScene = pParam->pTargetEntity->GetScene();
	if(pScene ==NULL) return;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// INT_PTR nNewPox=nTargetPosX ;
	// INT_PTR nNewPoy=nTargetPosY;
	// CEntityGrid::NewPosition(nNewPox,nNewPoy,nDir,nStep);
	// if(pScene->IsTelePort(nNewPox,nNewPoy)) //如果目标点有传送门则不可以推出去
	// {
	// 	return;
	// }
	// pParam->pTargetEntity->SetDir((nDir +4)  %DIR_STOP); //取相反的方向
	// pParam->pTargetEntity->GetMoveSystem()->Move(nDir,nTargetPosX,nTargetPosY,nStep,0,true,nInterval,sMoveBack);


	// 当前点
	int nTarPosX = nTargetPosX, nTarPosY = nTargetPosY;
	int nRealStep = 0;
	for (size_t i = 0; i < nStep; i++)
	{
		// 新的点
		INT_PTR nNewPox = nTarPosX;
		INT_PTR nNewPoy = nTarPosY;
		CEntityGrid::NewPosition(nNewPox,nNewPoy,nDir,1);

		//如果目标点有传送门则不可以推出去
		if(pScene->IsTelePort(nNewPox,nNewPoy))
		{
			return;
		}

		//不能穿过实体
		bool hasEntityBlock = false;
		const MapEntityGrid *pGrid=pScene->GetMapEntityGrid((int)nNewPox);

#ifndef GRID_USE_STL_MAP
		for(INT_PTR i=0; i< pGrid->m_entitys.count(); i++)
		{
			if(pGrid->m_entitys[i].pos != nNewPoy) continue;
			EntityHandle& hHandle = pGrid->m_entitys[i].hd;
#else
		const EntityPosVector& entitys = pGrid->m_entitys;
		typedef typename EntityPosVector::const_iterator Iter;
		Iter it = entitys.begin(), end = entitys.end();
		for (; it != end; it++)
		{
			if((*it).second != nNewPoy) continue;
			const EntityHandle& hHandle = (*it).first;
#endif
			switch(hHandle.GetType())
			{
			case enActor:
			case enMonster:
			case enNpc:
			case enPet:
			case enHero:
				CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
				if (pEntity && !pEntity->IsDeath())
					hasEntityBlock = true;
				break;
			}
		}
		if(hasEntityBlock || !pScene->CanMove(nNewPox,nNewPoy)) break;

		nRealStep++;
		nTarPosX = nNewPox;
		nTarPosY = nNewPoy;
	}
	pParam->pTargetEntity->SetDir((nDir +4)  %DIR_STOP); //取相反的方向
	pParam->pTargetEntity->GetMoveSystem()->Move(nDir,nTargetPosX,nTargetPosY,nRealStep,0,true,nInterval,sMoveBack);
}

void CSkillResult::RandTransfer(CFuBen *pFb,CScene *pScene,CEntity *pEntity)
{
	if(pScene ==NULL) return;
	int nWidth=0,nHeight=0; //长宽


	int nSceneId  = pScene->GetSceneId();

	pScene->GetMapSize(nWidth,nHeight);
	if(nWidth <=0 || nHeight <=0) return; //场景非法
	int nPosX,nPosY;
	CFuBenManager* pMgr = GetGlobalLogicEngine()->GetFuBenMgr();


	int nCurrentPosX,nCurrentPosY;
	pEntity->GetPosition(nCurrentPosX,nCurrentPosY);

	//尝试一万次，在这个地方找到一个点可以传送过去,找个点传送过去
	for(INT_PTR i=0; i< 10000; i++)
	{
		nPosX = wrand(nWidth);
		nPosY = wrand(nHeight);

		if(pScene->CanMove(nPosX,nPosY) )
		{
			//自己当前的点不算
			if(nCurrentPosX == nPosX &&nCurrentPosY ==  nPosY)
			{
				continue;
			}
			int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
			if( pMgr->EnterFuBen(pEntity,pFb,nSceneId,nPosX,nPosY,enDefaultTelePort,nEffId))
			{
				char broadcastBuff[64];
				CDataPacket broadcastPack(broadcastBuff, sizeof(broadcastBuff)); //下发的数据包
				broadcastPack << (BYTE) enDefaultEntitySystemID <<(BYTE)sEnterSceneEffect;
				broadcastPack << (Uint64)pEntity->GetHandle();
				broadcastPack << (unsigned short)(nEffId);
				CObserverSystem * pSystem = ((CAnimal*)pEntity)->GetObserverSystem();
				pSystem->BroadCast(broadcastPack.getMemoryPtr(),broadcastPack.getPosition(),false);
				break;
			}
		}
	}
}

void CSkillResult::FixTransfer(CFuBen *pFb,CScene *pScene,CEntity *pEntity, int nPosX,int nPosY)
{
	if(pScene ==NULL) return;
	int nWidth=0,nHeight=0; //长宽


	int nSceneId  = pScene->GetSceneId();

	pScene->GetMapSize(nWidth,nHeight);
	if(nWidth <=0 || nHeight <=0) return; //场景非法

	CFuBenManager* pMgr = GetGlobalLogicEngine()->GetFuBenMgr();


	int nCurrentPosX,nCurrentPosY;
	pEntity->GetPosition(nCurrentPosX,nCurrentPosY);

	if(pScene->CanMove(nPosX,nPosY) )
	{
		//自己当前的点不算
		if(nCurrentPosX == nPosX &&nCurrentPosY ==  nPosY)
		{
			return ;
		}
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		bool bEnterResult = false;

		int nPkValue = pEntity->GetProperty<unsigned int>(PROP_ACTOR_PK_VALUE);
		if (pEntity->GetScene() == pScene && GetLogicServer()->GetDataProvider()->GetPkConfig().redName > nPkValue)
		{
			bEnterResult = ((CAnimal*)pEntity)->GetMoveSystem()->InstantMove(nPosX,nPosY,1,enDefaultTelePort,nEffId);
		}
		else
		{
			bEnterResult = pFb->Enter(pEntity,nSceneId,nPosX,nPosY,0,0,enDefaultTelePort,nEffId);
		}

		if(bEnterResult)
		{
			char broadcastBuff[64];
			CDataPacket broadcastPack(broadcastBuff, sizeof(broadcastBuff)); //下发的数据包
			broadcastPack << (BYTE) enDefaultEntitySystemID <<(BYTE)sEnterSceneEffect;
			broadcastPack << (Uint64)pEntity->GetHandle();
			broadcastPack << (unsigned short)(nEffId);
			CObserverSystem * pSystem = ((CAnimal*)pEntity)->GetObserverSystem();
			pSystem->BroadCast(broadcastPack.getMemoryPtr(),broadcastPack.getPosition(),false);
		}
	}

}



void CSkillResult::PostponeMoveTime(PSKILLRESULTPARAM pParam)
{
	if( pParam->pTargetEntity ==NULL) return;
	pParam->pTargetEntity->GetMoveSystem()->PostponeMoveTime(pParam->pSkillResult->nValue);
}

void  CSkillResult::LastSafeAreaRTransfer(PSKILLRESULTPARAM pParam)
{
	if(pParam->pSrcEntity ==NULL) return;
	if(pParam->pSrcEntity->GetType() != enActor) return; //只有玩家才有
	CActor *pActor = (CActor*)pParam->pSrcEntity; //玩家的指针

	if( !pActor->IsInRawServer() ) return; //不在跨服里是没法使用的

	if (wrand(10000) <= pParam->pSkillResult->nId)
	{
		int nSceneId = pActor->GetCitySceneId();
		if(nSceneId ==0)
		{
			nSceneId = pActor->GetSceneID();
		}

		CFuBen * pFb=GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if(pFb ==NULL) return;

		CScene *pScene= pFb->GetScene(nSceneId);
		RandTransfer(NULL,pScene,pActor);
	}
}

//指定场景地点传送
void  CSkillResult::Transfer(PSKILLRESULTPARAM pParam)
{
	if(pParam->pSrcEntity ==NULL) return;
	if(pParam->pSrcEntity->GetType() != enActor) return; //只有玩家才有
	CActor *pActor = (CActor*)pParam->pSrcEntity; //玩家的指针
	CFuBenManager *pFBMgr = GetGlobalLogicEngine()->GetFuBenMgr();

	if( !pActor->IsInRawServer() ) return; //不在跨服里是没法使用的

	int nSceneId = pParam->pSkillResult->nId;
	int nX = pParam->pSkillResult->nParam1;
	int nY = pParam->pSkillResult->nParam2;

	CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);

	if (pFb)
	{
		CScene *pScene= pFb->GetScene(nSceneId);
		FixTransfer(pFb, pScene,pParam->pSrcEntity,nX,nY);
		// if (pScene && pScene->CanMove(nX,nY))
		// {
		// 	if (pActor->GetScene() == pScene)
		// 	{
		// 		((CAnimal*)pParam->pSrcEntity)->GetMoveSystem()->InstantMove(nX,nY);
		// 	}
		// 	else
		// 	{
		// 		pFb->Enter(pParam->pSrcEntity,nSceneId,nX,nY,0,0);
		// 	}
		// }
	}
}

//本场景随机传送
void  CSkillResult::SceneTransfer(PSKILLRESULTPARAM pParam)
{
	//往这里随机传送
	if(pParam->pSrcEntity ==NULL) return;
	//CScene *pScene=  pParam->pSrcEntity->GetScene();

	CEntity *pEntity = (CEntity*) pParam->pSrcEntity;
	CFuBen * pFb = pEntity->GetFuBen();
	int nSceneId =  pEntity->GetSceneID();
	if(nSceneId ==0) return; //别传送进了测试场景

	if(pEntity ==NULL || pEntity->GetScene() ==NULL) return;

	if(pEntity->GetType() != enActor)
	{
		OutputMsg(rmTip,"Skill SceneTransfer ,name=[%s],scene=[%s]",pEntity->GetEntityName(), pEntity->GetScene()->GetSceneName());
	}

	RandTransfer(pFb,pEntity->GetScene(),pEntity);
}

//最后一个安全区域传送
void  CSkillResult::LastSafeAreaTransfer(PSKILLRESULTPARAM pParam)
{
	if(pParam->pSrcEntity ==NULL) return;
	if(pParam->pSrcEntity->GetType() != enActor) return; //只有玩家才有
	CActor *pActor = (CActor*)pParam->pSrcEntity; //玩家的指针

	WORD nSceneId, nX, nY;

	// if( !pActor->IsInRawServer() ) return; //不在跨服里是没法使用的

	pActor->GetCityPoint(nSceneId,nX,nY);
	if(nSceneId==0)
	{
		nSceneId = pActor->GetSceneID();
	}

	int nPkValue = pParam->pSrcEntity->GetProperty<unsigned int>(PROP_ACTOR_PK_VALUE);

	if (GetLogicServer()->GetDataProvider()->GetPkConfig().redName <= nPkValue)
	{
		nSceneId = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonSceneId;//红名地图
		nX = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonPosX;
		nY = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonPosY;
		//nWeight = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonWeight;
		//nHeight = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonHeight;
	}

	CFuBen * pFb=GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	if(pFb == NULL) return;
	CScene *pScene= pFb->GetScene(nSceneId);

	int nDiffX,nDiffY;
	for(INT_PTR i=0; i< 50; i++)
	{
		nDiffX = (int)wrand(6) - 3;
		nDiffY = (int)wrand(6) - 3;

		if(pScene->CanMove(nX+nDiffX, nY+nDiffY) )
		{
			FixTransfer(pFb, pScene, pParam->pSrcEntity,nX+nDiffX,nY+nDiffY);
			return;
		}
	}

	FixTransfer(pFb, pScene, pParam->pSrcEntity,nX,nY);
}

void  CSkillResult::ChangEquipDua(PSKILLRESULTPARAM pParam)
{
	if(pParam->pSrcEntity ==NULL) return;

	if(pParam->pSrcEntity->GetType() != enActor)
	{
		return ;
	}
	CUserEquipment&   equips= ((CActor *) pParam->pSrcEntity)->GetEquipmentSystem();
	//获取武器的指针
	CUserItem *pUserItem =equips.GetItemByType(pParam->pSkillResult->nId);


	if(pUserItem ==NULL  )return ;

	equips.ChangeEquipDua(pUserItem,pParam->pSkillResult->nValue);


}

//怪物掉落东西
void CSkillResult::DropMonsterItem(PSKILLRESULTPARAM pParam)
{
	CActor * pActor = NULL;
	if(pParam->pTargetEntity != NULL )
	{
		if(pParam->pTargetEntity->GetType() == enPet)
		{
			pActor = ((CPet*)pParam->pTargetEntity)->GetMaster();
		}
		else if(pParam->pTargetEntity->GetType() == enHero)
		{
			pActor = ((CHero*)pParam->pTargetEntity)->GetMaster();
		}
		else if(pParam->pTargetEntity->GetType() == enActor)
		{
			pActor = (CActor*)pParam->pTargetEntity;
		}
	}
	CMonster::RealDropItemExp(pParam->pSkillResult->nId,pActor,pParam->pSrcEntity->GetScene(),pParam->nTargetPosX,pParam->nTargetPosY);

}


void CSkillResult::ChangeModel(PSKILLRESULTPARAM pParam)
{
	if( pParam->pSrcEntity == NULL ) return;
	pParam->pSrcEntity->SetProperty<int>(PROP_ENTITY_MODELID, pParam->pSkillResult->nId);

}

//按百分比扣属性,rate表示扩除的万分比，1000表示10%，id=1表示扣血，1表示扣蓝
void CSkillResult::RateChangeProperty(PSKILLRESULTPARAM pParam)
{
	if( pParam->pTargetEntity == NULL || !pParam->pTargetEntity->isAnimal()) return;
	INT_PTR nEntityType = pParam->pTargetEntity->GetType();
	int nType = pParam->pSkillResult->nId;
	int nRate = pParam->pSkillResult->nValue;
	float fRate=  (float)nRate/10000;
	int nValue =0;
	switch (nType)
	{
	case 1:	//最大血
		{
			nValue =  pParam->pTargetEntity->GetProperty< int>(PROP_CREATURE_MAXHP);
			nValue = (int )(fRate * nValue);
			if (nValue < 0 && pParam->pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) == 0)	//如果挂了就没血可扣了
			{
				return;
			}
			pParam->pTargetEntity->ChangeHP(nValue);

			if (nValue < 0 && pParam->pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) == 0)
			{
				pParam->pTargetEntity->OnKilledByEntity(pParam->pSrcEntity);
			}
		}
		break;
	case 2:	//最大蓝
		{
			nValue =  pParam->pTargetEntity->GetProperty< int>(PROP_CREATURE_MAXMP);
			nValue = (int )(fRate * nValue);
			pParam->pTargetEntity->ChangeMP(nValue);
		}
		break;
	case 3:	//最大基本血
		{
			if (nEntityType == enActor)
			{
				nValue = ((CActor *)(pParam->pTargetEntity))->m_nBase_MaxHp;
			}
			else if (nEntityType == enHero)
			{
				const CHeroSystem::HERODATA *pData = ((CHero *)(pParam->pTargetEntity))->GetHeroPtr();
				if (pData)
				{
					nValue = pData->nBaseMaxHp;
				}
			}
			else
				nValue =  pParam->pTargetEntity->GetProperty< int>(PROP_CREATURE_MAXHP);

			nValue = (int )(fRate * nValue);

			if (nValue < 0 && pParam->pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) == 0)	//如果挂了就没血可扣了
			{
				return;
			}

			pParam->pTargetEntity->ChangeHP(nValue);

			if (nValue < 0 && pParam->pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) == 0)
			{
				pParam->pTargetEntity->OnKilledByEntity(pParam->pSrcEntity);
			}
		}
		break;
	case 4:	//最大基本蓝
		{
			if (nEntityType == enActor)
			{
				nValue = ((CActor *)(pParam->pTargetEntity))->m_nBase_MaxMp;
			}
			else if (nEntityType == enHero)
			{
				const CHeroSystem::HERODATA *pData = ((CHero *)(pParam->pTargetEntity))->GetHeroPtr();
				if (pData)
				{
					nValue = pData->nBaseMaxMp;
				}
			}
			else
				nValue =  pParam->pTargetEntity->GetProperty< int>(PROP_CREATURE_MAXMP);

			nValue = (int )(fRate * nValue);
			pParam->pTargetEntity->ChangeMP(nValue);
		}
		break;
	default:
		break;
	}

}

void CSkillResult::GiveAward(PSKILLRESULTPARAM pParam)
{
	if( pParam->pTargetEntity == NULL ||  pParam->pTargetEntity->GetType() != enActor) return;
	CActor *pActor = (CActor*)pParam->pTargetEntity;
	pActor->GiveAward(pParam->pSkillResult->nParam1,pParam->pSkillResult->nId,pParam->pSkillResult->nValue,0,0,pParam->pSkillResult->nParam2,0,GameLog::clSkillResultGiveAward,"Skill");

}

void CSkillResult::NoticeRemMasterAsset( LPCTSTR sName, bool boItems, int nItemId, int nCount)
{
	char sText[1024] = {0};
	if (boItems)
	{
		if (nCount > 0)
		{
			const CStdItem  * pItemConfig = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
			if (pItemConfig != NULL)
			{
				LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpPetDropMasterItems);
				sprintf_s(sText, sizeof(sText), sFormat, sName, pItemConfig->m_sName, nCount);
			}
		}
		else
		{
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpPetNotDropMstItems);
			sprintf_s(sText, sizeof(sText), sFormat, sName, nCount);
		}
	}
	else
	{
		if (nCount > 0)
		{
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpPetDropMasterMoney);
			sprintf_s(sText, sizeof(sText), sFormat, sName, nCount);
		}
		else
		{
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpPetNotDropMstMoney);
			sprintf_s(sText, sizeof(sText), sFormat, sName);
		}
	}
	GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(sText, ttChatWindow + ttScreenCenter);
}

//客户端播放引导性特效
void CSkillResult::PlayGuide(PSKILLRESULTPARAM pParam)
{
	if (NULL == pParam->pTargetEntity) return;

	if (pParam->pTargetEntity->GetType() == enActor)
	{
		CActorPacket pack;
		CDataPacket &data = ((CActor*)pParam->pTargetEntity)->AllocPacket(pack);
		data << (BYTE)enSkillSystemID << (BYTE)sPlayGuide;
		data << (unsigned)pParam->pSkillResult->nId;
		data << (unsigned)pParam->pSkillResult->nValue;
		pack.flush();
	}
}

//忽略每次最大伤害限制
void CSkillResult::IgnoreMaxDropHp(PSKILLRESULTPARAM pParam)
{
	pParam->btIgnoreMaxDropHp =1; //忽略每次最大伤害限制
}



//丢到一个点移动
void CSkillResult::PointMove(PSKILLRESULTPARAM pParam)
{
	if(pParam->pTargetEntity ==NULL) return;
	int  nPosX,nPosY;
	nPosX = pParam->pSkillResult->nId ;  //x,y
	nPosY = pParam->pSkillResult->nValue;

	if(nPosX >0 && nPosY >0)
	{
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		pParam->pTargetEntity->GetMoveSystem()->InstantMove(nPosX,nPosY,1,enDefaultTelePort,nEffId); //瞬间移动
	}
	else
	{
		//如果坐标没有配置则随机传
		CFuBen *pFb    = pParam->pTargetEntity->GetFuBen();
		CScene *pScene =pParam->pTargetEntity->GetScene();
		RandTransfer(pFb,pScene,pParam->pTargetEntity);
	}

}


//自己移动到范围内的一个点
void CSkillResult::SelfPointMove(PSKILLRESULTPARAM pParam)
{
	/*
	if (pParam->pSrcEntity ==NULL) return;

	int nPosX,nPosY,nMaxDis,isUseRand,isUseLine;
	nPosX = pParam->nTargetPosX;  //x,y
	nPosY = pParam->nTargetPosY;
	nMaxDis = pParam->pSkillResult->nValue;
	isUseRand = pParam->pSkillResult->nId;  //是否使用回字查找
	isUseLine = pParam->pSkillResult->nParam1;  //是否优先选择距离与玩家原来所在地较近的位置

	if (nPosX >0 && nPosY >0 && isUseLine)
	{
		//点不可移动时，是否优先选择距离与玩家原来所在地较近的位置
		CScene *pScene = pParam->pSrcEntity->GetScene();
		if (pScene)
		{
			int nStartX, nStartY;
			pParam->pSrcEntity->GetPosition(nStartX, nStartY);
			//距离的平方
			INT_PTR nDisSquare = CMovementSystem::GetDisSqare(nStartX,nStartY, nPosX,nPosY);
			//获取方向
			INT_PTR nDir = CEntity::GetDir(nPosX,nPosY,nStartX,nStartY);
			INT_PTR nNewPosX = nPosX;
			INT_PTR nNewPosY = nPosY;
			INT_PTR nStep = ((int)sqrt((double)nDisSquare));
			for (INT_PTR i = 0; i < nStep; i++)
			{
				if (!pParam->pSrcEntity->GetMoveSystem()->CanJumpTo(pScene,nNewPosX,nNewPosY))
				{
					CEntityGrid::NewPosition(nNewPosX, nNewPosY, nDir, 1);
					continue;
				}

				if (nMaxDis > 0)	//限制一下移动最大的距离
				{
					nDisSquare = CMovementSystem::GetDisSqare(nStartX,nStartY, nNewPosX,nNewPosY);
					if (nDisSquare > (nMaxDis * nMaxDis))
					{
						CEntityGrid::NewPosition(nNewPosX, nNewPosY, nDir, 1);
						continue;
					}
				}

				//找到了,如果一直找不到会在InstantMove()再进行回字查找
				nPosX = (int)nNewPosX;
				nPosY = (int)nNewPosY;
				break;

			}
		}
	}

	pParam->pSrcEntity->GetMoveSystem()->InstantMove(nPosX,nPosY,isUseRand); //瞬间移动
	*/

	if (pParam->pSrcEntity ==NULL) return;

	int nPosX,nPosY,nMaxDis,isUseRand,isUseLine;
	nPosX = pParam->nTargetPosX;  //x,y
	nPosY = pParam->nTargetPosY;
	nMaxDis = pParam->pSkillResult->nValue;
	isUseRand = pParam->pSkillResult->nId;  //是否使用回字查找
	isUseLine = pParam->pSkillResult->nParam1;  //是否优先选择距离与玩家原来所在地较近的位置

	if (nPosX >0 && nPosY >0 && isUseLine)
	{
		CScene *pScene = pParam->pSrcEntity->GetScene();
		if (pScene)
		{
			int nStartX, nStartY;
			pParam->pSrcEntity->GetPosition(nStartX, nStartY);
			//距离的平方
			INT_PTR nDisSquare = CMovementSystem::GetDisSqare(nStartX,nStartY, nPosX,nPosY);
			//获取方向
			INT_PTR nDir = CEntity::GetDir(nStartX,nStartY,nPosX,nPosY);
			INT_PTR nNewPosX = nPosX;
			INT_PTR nNewPosY = nPosY;
			if (nDisSquare > nMaxDis * nMaxDis)
			{
				nDisSquare = nMaxDis * nMaxDis;
				nNewPosX = nStartX;
				nNewPosY = nStartY;
				CEntityGrid::NewPosition(nNewPosX, nNewPosY, nDir, nMaxDis);
				nPosX = (int)nNewPosX;
				nPosY = (int)nNewPosY;
			}
		}
	}
	int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
	pParam->pSrcEntity->GetMoveSystem()->InstantMove(nPosX,nPosY,isUseRand,enDefaultTelePort,nEffId); //瞬间移动

}

//冲锋
void CSkillResult::TargetRapidMove(PSKILLRESULTPARAM pParam)
{
	if (pParam->pTargetEntity == NULL || pParam->pSrcEntity == NULL || !pParam->pTargetEntity->isAnimal()) return;

	if (pParam->pTargetEntity == pParam->pSrcEntity) return;

	INT_PTR nMaxStep = pParam->pSkillResult->nValue > 0 ? pParam->pSkillResult->nValue : 30; //移动的最大步数
	int nId = pParam->pSkillResult->nId;  //如果失败了添加值为id的技能标记
	int nTime = pParam->pSkillResult->nParam1;  //timeParam表示技能标记的时间

	int nSpeed = pParam->pSkillResult->nParam2; //冲锋速度

	int nPosX,nPosY,nTargetPosX,nTargetPosY;
	pParam->pSrcEntity->GetPosition(nPosX,nPosY);	//玩家当前的位置
	nTargetPosX = pParam->nTargetPosX;
	nTargetPosY = pParam->nTargetPosY;
	if (nTargetPosX == 0 || nTargetPosY == 0)
	{
		pParam->pTargetEntity->GetPosition(nTargetPosX,nTargetPosY);	//目标当前的位置
	}


	CMovementSystem *pMoveSys = pParam->pSrcEntity->GetMoveSystem();
	CScene *pScene = pParam->pSrcEntity->GetScene();
	CScene *pTargetScene = pParam->pTargetEntity->GetScene();
	if (pScene == NULL || pScene != pTargetScene) return;

	bool hasHit = false;
	INT_PTR nDir = pParam->pSrcEntity->GetProperty< int>(PROP_ENTITY_DIR);

	INT_PTR nFinishStep = 0, nOneStep = 1;
	INT_PTR nNewPosX = nPosX;
	INT_PTR nNewPosY = nPosY;

	static INT_PTR nDirTbl[3] = {0, -1, 1};	//需先判断与目标点的方向

	INT_PTR nCheckDir = nDir;
	if (nNewPosX == nTargetPosX && nNewPosY == nTargetPosY)
	{
		hasHit = true;
	}
	else
	{
		while (nFinishStep < nMaxStep) //如果还没完成
		{
			nDir = CEntity::GetDir(nNewPosX,nNewPosY,nTargetPosX,nTargetPosY);

			//取新坐标
			INT_PTR nCheckPosX = nNewPosX;
			INT_PTR nCheckPosY = nNewPosY;

			bool bCanMove = false;

			for (INT_PTR i = 0; i < 1; i++)	//判断3方向 现暂判断1方向
			{
				nCheckDir = (nDir + nDirTbl[i]) % DIR_STOP;

				CEntityGrid::NewPosition(nCheckPosX,nCheckPosY,nCheckDir,nOneStep);

				if (nCheckPosX == nTargetPosX && nCheckPosY == nTargetPosY)
				{
					hasHit = true;
					break;
				}

				if (pMoveSys->CanJumpTo(pScene,nCheckPosX,nCheckPosY)) //遇到了一个传说门就不过去了
				{
					nNewPosX = nCheckPosX;
					nNewPosY = nCheckPosY;
					bCanMove = true;
					break;
				}
			}

			if (!bCanMove)	//无法通过
			{
				break;
			}

			nFinishStep ++;		 //已经完成的步子
		}
	}

	INT_PTR nNewDir = CEntity::GetDir(nPosX,nPosY,nNewPosX,nNewPosY);
	if (nNewDir != DIR_STOP)
	{
		pParam->pSrcEntity->SetDir(nNewDir);
	}

	if (nFinishStep > 0)
	{
		if (pScene->MoveTo(pParam->pSrcEntity,nNewPosX,nNewPosY))
		{
			if (pParam->pSrcEntity->GetPosInfo().pScene == pScene)
			{
				char broadcasrBuff[128];
				CDataPacket broadData(broadcasrBuff,sizeof(broadcasrBuff)); //广播的数据
				//广播玩家的c移动
				broadData << (BYTE)enDefaultEntitySystemID << (BYTE)sTargetRapidMove;
				broadData << (Uint64)pParam->pSrcEntity->GetHandle() << (WORD)nPosX << (WORD)nPosY;
				broadData << (WORD)nNewPosX << (WORD)nNewPosY;
				broadData << (BYTE)pParam->pSrcEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);
				broadData << (int)nSpeed;
				CObserverSystem *pSystem = pParam->pSrcEntity->GetObserverSystem();
				if (pSystem)
				{
					pSystem->BroadCast(broadData.getMemoryPtr(),broadData.getPosition(),true);
				}
			}
		}
		else
		{
			hasHit = false;
		}
	}
}

 void CSkillResult::BroadCastHit(CAnimal * pEntity, Uint64 hTargetHandle)
 {
	 char buff[128];
	 CDataPacket pack(buff,sizeof(buff));
	 pack << (BYTE) enDefaultEntitySystemID  << (BYTE) sSkillHit;
	 pack << (Uint64) hTargetHandle  ;
	 pEntity->GetObserverSystem()->BroadCast(pack.getMemoryPtr(),pack.getPosition(),true);
 }

  //造成伤害 = 攻击方攻击伤害 * ( 1 – 防守方护甲减伤比例)
  // 护甲减伤比例 = 防守方防御值 * 15 / （ 防守方防御值 * 15 + 角色等级 * 400）

// 角色等级的取值并不是固定的，当攻击方等级比防守方高时，采用防守方的等级进行计算，
//	  当防守方等级比攻击方等级高时，采用攻击方的等级进行伤害减免计算，这样设定的主要目的是建立等级优势。



 void CSkillResult::CalculateDamage(INT_PTR nSkillID,CAnimal * pSrcEntity,CAnimal *pTargetEntity,
	 PSKILLRESULTPARAM pParam,float fAttackValue,INT_PTR  nAttackType)
 {
	 if(pSrcEntity == NULL || pTargetEntity ==NULL || fAttackValue <= 0.0) return; //压根就没有伤害


	 unsigned int nCurrentHp = 	pTargetEntity->GetProperty<unsigned int>( PROP_CREATURE_HP);

	 if( nCurrentHp == 0) return;

	 GLOBALCONFIG &globalConfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();

	 const char * sSkillName=NULL ;
	 const OneSkillData *pConfig =  NULL;
	 if(nSkillID)
	 {
		pConfig = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
		sSkillName = pConfig->sSkillName;
	 }
	 else
	 {
		 sSkillName = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpNearAttackSkillName);
	 }

	 int nDefence = 0;

	 int  nSrcType =  (int)pSrcEntity->GetType();
	 INT_PTR nTargetType = pTargetEntity->GetType(); //获取实体的类型

	 if(nTargetType ==enActor)
	 {
		 if( ((CActor*)pTargetEntity)->m_isSuperman ) return; //如果是无敌状态，则不添加伤害
	 }
	 //防御值
	 float fDrop ;  //掉血的点数
	 int nDrop; //整数的血的掉落
	 bool bIgnoreDamgeRate =false; //是否忽略伤害吸收

	 bool isNotFireDamage = pParam->pSkillResult->btHitType != htFire; //不是火的伤害，部分效果不计算火墙输出

	 if(pParam->btIgnoreDefence) //如果是忽略对方的防御，那么就是0
	 {
		 nDefence =0;
		 bIgnoreDamgeRate =true;
	 }
	 else
	 {
		 int nMaxValue,nMinValue;

		 if( nAttackType == TYPE_PHYSICAL_ATTACK) //如果是物理攻击的话
		 {
			 nMaxValue = pTargetEntity->GetProperty<int>(PROP_CREATURE_PYSICAL_DEFENCE_MAX);
			 nMinValue = pTargetEntity->GetProperty<int>(PROP_CREATURE_PYSICAL_DEFENCE_MIN);
		 }
		 else
		 {
			 nMaxValue = pTargetEntity->GetProperty<int>(PROP_CREATURE_MAGIC_DEFENCE_MAX);
			 nMinValue = pTargetEntity->GetProperty<int>(PROP_CREATURE_MAGIC_DEFENCE_MIN);
		 }
		 if(nMinValue >= nMaxValue)
		 {
			 nDefence = nMaxValue;
		 }
		 else
		 {
			 nDefence = nMinValue + wrand(nMaxValue - nMinValue +1); //随机取一个防御值
		 }
		//无视防御比例--mhl
		if(isNotFireDamage && nSrcType == enActor || nSrcType == enHero ) {
			CAdvanceAnimal* pAdvSrcAnimal =  (CAdvanceAnimal*)pSrcEntity;
			if (pAdvSrcAnimal && pAdvSrcAnimal->m_nIgnorDefenceRatio > 0)
			{
				int currentDef = (int)(nDefence *(pAdvSrcAnimal->m_nIgnorDefenceRatio)/10000.0);
				nDefence -= currentDef;
			}
		}

		 //无视防御,如果施法方有无视防御的效果，则需要免去对方一定的防御
		 if(isNotFireDamage && nSrcType == enActor || nSrcType == enHero )
		 {
			 CAdvanceAnimal* pAdvSrcAnimal =  (CAdvanceAnimal*)pSrcEntity;
			 int nRateSkill = 0;
			 if (pAdvSrcAnimal && pConfig && pConfig->wAttrId == aIgnorDefenceRateSkill)
			 {
				 nRateSkill = pAdvSrcAnimal->m_nIgnorDefenceRateSkill;
			 }
			 if(  pAdvSrcAnimal->m_nIgnorDefenceRate + nRateSkill > 0 )
			 {
				 if(wrand(10000) < (unsigned long)(pAdvSrcAnimal->m_nIgnorDefenceRate + nRateSkill)) //触发了无视防御
				 {
					 nDefence = 0 ;

					 //如果实现了目标无视防御，要给自己放1个特效
					 static int s_nIgnorDefenceEffectId = 0; //这个特效id
					 static int s_nIgnorDefenceEffectKeepTime = 0; //这个特效持续时间

					 if(s_nIgnorDefenceEffectId >0 )
					 {
						 CEntityMsg msg(CEntityMsg::emAppendEntityEffect, pSrcEntity->GetHandle());
						 msg.dwDelay = 0;
						 msg.nParam1 = 0;  //肯定是个挥洒型的特效
						 msg.nParam2 = s_nIgnorDefenceEffectId;
						 msg.nParam3 = s_nIgnorDefenceEffectKeepTime;
						 pSrcEntity->PostEntityMsg(msg); //向目标发送消息，稍候由目标进行广播
					 }

				 }
			 }
		 }
		 if(nDefence <0) //防御不能小于0，小于0了就当成0
		 {
			nDefence =0;
		 }
	 }

	 fDrop =  fAttackValue - nDefence;

	#ifdef _DEBUG
	// S3
	SNPRINTFA(s_HurtValueBuff,100," //S3::%d",(int)fDrop);
	s_HurtValueMsg += s_HurtValueBuff;
	#endif

	 // 攻击事件
	 pTargetEntity->OnAttacked(pSrcEntity);
	 pSrcEntity->OnAttackOther(pTargetEntity);

	//职业伤害减免/加成
	int nDamageValueDesc =0, nDamageRateDesc=0;			//职业伤害减免
	int nTargetDamageValue = 0, nTargetDamageRate = 0;	//职业伤害加成
	{
		CAdvanceAnimal* pAdvAnimal =  (CAdvanceAnimal*)pTargetEntity;
		CAdvanceAnimal* pAdvSrcAnimal =  (CAdvanceAnimal*)pSrcEntity;

		//职业伤害加成  nTargetDamageValue nTargetDamageRate
		int nTargetVocation = 0;
		if (nTargetType == enActor)
		{
			nTargetVocation = pTargetEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
		}
		else if (nTargetType == enHero)
		{
			const CHeroSystem::HERODATA * pTargetHero = ((CHero *)pTargetEntity)->GetHeroPtr();
			if (pTargetHero != NULL)
			{
				nTargetVocation = pTargetHero->data.bVocation;
			}
		}
		else
		{
			nTargetVocation = enVocNone;
		}

		switch(nTargetVocation)
		{
		case enVocNone: //怪物，宠物
			nTargetDamageValue += pAdvSrcAnimal->m_nMonsterTargetDamageValue;
			nTargetDamageRate = pAdvSrcAnimal->m_nMonsterTargetDamageRate;
			break;
		case enVocWarrior:
			nTargetDamageValue += pAdvSrcAnimal->m_nWarriorTargetDamageValue;
			nTargetDamageRate = pAdvSrcAnimal->m_nWarriorTargetDamageRate;
			break;
		case enVocMagician:
			nTargetDamageValue += pAdvSrcAnimal->m_nMagicianTargetDamageValue;
			nTargetDamageRate = pAdvSrcAnimal->m_nMagicianTargetDamageRate;
			break;
		case enVocWizard:
			nTargetDamageValue += pAdvSrcAnimal->m_nWizardTargetDamageValue;
			nTargetDamageRate = pAdvSrcAnimal->m_nWizardTargetDamageRate;
			break;
		default:
			break;
		}

		//职业伤害减免 nDamageValueDesc nDamageRateDesc
		if( nTargetType == enHero || nTargetType == enActor )
		{
			if(nSrcType == enActor || nSrcType == enHero)
			{
				int nVocation = 0;
				if(nSrcType == enActor)
				{
					nVocation = pSrcEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
				}
				else
				{
					const CHeroSystem::HERODATA * pData = ((CHero*)pSrcEntity)->GetHeroPtr();
					if(pData)
					{
						nVocation = pData->data.bVocation;
					}
				}

				switch(nVocation)
				{
				case enVocWarrior:
					nDamageValueDesc += pAdvAnimal->m_nWarriorDamageValueDec;
					nDamageRateDesc  = pAdvAnimal->m_nWarriorDamageRateDec;
					break;
				case enVocMagician:
					nDamageValueDesc += pAdvAnimal->m_nMagicianDamageValueDec;
					nDamageRateDesc  = pAdvAnimal->m_nMagicianDamageRateDec;
					break;
				case enVocWizard:
					nDamageValueDesc += pAdvAnimal->m_nWizardDamageValueDec;
					nDamageRateDesc  = pAdvAnimal->m_nWizardDamageRateDec;
					break;
				}
			}
			else if(nSrcType == enMonster || nSrcType == enPet) //宝宝也算怪物的减免
			{
				nDamageValueDesc += pAdvAnimal->m_nMonsterDamageValueDec;
				nDamageRateDesc  = pAdvAnimal->m_nMonsterDamageRateDec;
			}
		}
	}

	//非职业伤害减免 +nDamageValueDesc
 	if( nTargetType == enHero || nTargetType == enActor )
	{
		CAdvanceAnimal* pAdvAnimal =  (CAdvanceAnimal*)pTargetEntity;
		CAdvanceAnimal* pAdvSrcAnimal =  (CAdvanceAnimal*)pSrcEntity;

		if( isNotFireDamage && pAdvAnimal->m_nDamageReduceRate > 0 && pAdvAnimal->m_nDamageReduceValue >0)
		{
			if(wrand(10000) < pAdvAnimal->m_nDamageReduceRate) //触发了伤害减免
			{
				nDamageValueDesc += pAdvAnimal->m_nDamageReduceValue;

				//如果实现了目标无视防御，要给自己放1个特效
				static int s_nDamageDescEffectId = 0;
				static int s_nDamageDescEffectKeepTime = 0; //这个特效持续时间

				if(s_nDamageDescEffectId >0 )
				{
					CEntityMsg msg(CEntityMsg::emAppendEntityEffect, pSrcEntity->GetHandle());
					msg.dwDelay = 0;
					msg.nParam1 = 0;  //肯定是个挥洒型的特效
					msg.nParam2 = s_nDamageDescEffectId;
					msg.nParam3 = s_nDamageDescEffectKeepTime;
					pTargetEntity->PostEntityMsg(msg); //向目标发送消息，稍候由目标进行广播
				}
			}
		}
	}

	//非职业伤害加成 -nDamageValueDesc
	if(nSrcType == enActor || nSrcType == enHero)
	{
		CAdvanceAnimal* pAdvSrcAnimal =  (CAdvanceAnimal*)pSrcEntity;

		//伤害追加
		if(isNotFireDamage && pAdvSrcAnimal->m_nDamageAddRate > 0 && pAdvSrcAnimal->m_nDamageAddValue >0)
		{
			if(wrand(10000) < pAdvSrcAnimal->m_nDamageAddRate) //触发了伤害追加
			{
				nDamageValueDesc -= pAdvSrcAnimal->m_nDamageAddValue;

				//如果实现了目标无视防御，要给自己放1个特效
				static int s_nDamageAddEffectId = 0; //这个特效id
				static int s_nDamageAddEffectKeepTime = 0; //这个特效持续时间

				if(s_nDamageAddEffectId >0 )
				{
					CEntityMsg msg(CEntityMsg::emAppendEntityEffect, pSrcEntity->GetHandle());
					msg.dwDelay = 0;
					msg.nParam1 = 0;  //肯定是个挥洒型的特效
					msg.nParam2 = s_nDamageAddEffectId;
					msg.nParam3 = s_nDamageAddEffectKeepTime;
					pAdvSrcAnimal->PostEntityMsg(msg); //向目标发送消息，稍候由目标进行广播
				}

			}
		}
	}

	if (nTargetDamageValue > 0) fDrop += nTargetDamageValue;
	if(nDamageValueDesc != 0) fDrop -= nDamageValueDesc;

	#ifdef _DEBUG
	// S4
	SNPRINTFA(s_HurtValueBuff,100," //S4::%d",(int)fDrop);
	s_HurtValueMsg += s_HurtValueBuff;
	#endif

	if (nTargetDamageRate > 0 || nDamageRateDesc >0)
	{
		float nBaseRate = ((float)(10000 + nTargetDamageRate - nDamageRateDesc))/10000;
		fDrop *= nBaseRate;
	}

	#ifdef _DEBUG
	// S5
	SNPRINTFA(s_HurtValueBuff,100," //S5::%d",(int)fDrop);
	s_HurtValueMsg += s_HurtValueBuff;
	#endif

	//新版暴击，最后计算 --火墙除外
	int nCritiEffId = 0;
	if(pParam->pSkillResult && pParam->pSkillResult->btHitType != htFire)
	{
		int nNewCriDamarate = pParam->pSrcEntity->m_nCritDamageRate - pParam->pTargetEntity->m_nDeductCritDamageRate;
		if (nNewCriDamarate > 0 &&  (int)wrand(10000) < nNewCriDamarate)
		{
			fDrop *= (1 + (pParam->pSrcEntity->m_nCritDamagePower/10000.0));
			fDrop += (float)pParam->pSrcEntity->m_nCritDamageValue;
			pSrcEntity->SetProperty<unsigned int>(PROP_ACTOR_CRIT_DAMAGE,(int)fDrop);
			if(pSrcEntity && pTargetEntity) {
				// static int nCritiEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nCritiEffId;
				// if(nCritiEffId > 0) {
					nCritiEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nCritiEffId;
				// }

			}
		}
	}

	//最终的扣血伤害
	if (fDrop <= 0.0 ) fDrop = 1.0f;//return ; //压根就不掉血
	nDrop = (int)fDrop;
	if(nCritiEffId > 0) {
		CEntityMsg msg(CEntityMsg::emShowEntityEffect, pSrcEntity->GetHandle());
		msg.nParam1 = nCritiEffId;
		msg.nParam3 = (int)nDrop;
		pTargetEntity->PostEntityMsg(msg);
	}

	if(fDrop - nDrop > 0.0) nDrop ++;//向上取整，不够一滴的算1滴伤害

	#ifdef _DEBUG
	// S6
	SNPRINTFA(s_HurtValueBuff,100," //S6::%d",nDrop);
	s_HurtValueMsg += s_HurtValueBuff;
	#endif

	//扣血
	bool bIgnoreMaxDropHp = pParam->btIgnoreMaxDropHp > 0 ? true : false; //是否忽略每次最大伤害限制
	///---------应策划要求暂时加一条无敌--理应是buff 怪物打玩家无敌
	// if(nTargetType == enActor && (nSrcType == enMonster || nSrcType == enNpc))
	// 	nDrop = 0;
	// if(((CActor *)pTargetEntity)->GetIsWd())
	// 	nDrop = 0;
	int btHitType = htDefault;
	if(pParam->pSkillResult)
		btHitType = pParam->pSkillResult->btHitType;

	((CCreature *)pTargetEntity)->ChangeHP(-1* nDrop,pSrcEntity,bIgnoreDamgeRate,bIgnoreMaxDropHp,true, btHitType); //掉了多少点血

	//特殊buff
	CDynamicBuff *pBuff = pTargetEntity->GetBuffSystem()->GetFirstBuff(aDamageDropTime);
	if(pBuff)
	{
		pTargetEntity->GetBuffSystem()->ReduceBuffTime(pBuff,nDrop);
	}

	//计算反弹
	if (pTargetEntity->m_nDamageReboundRate > 0 && wrand(10000) < pTargetEntity->m_nDamageReboundRate)
	{
		int nTargetDrop = (int)(nDrop * pTargetEntity->m_fDamageReboundPower);
		if (nTargetDrop > 0)
		{
			pSrcEntity->ChangeHP(-nTargetDrop, pTargetEntity);
			if (nSrcType == enActor)
			{
				((CActor*)pSrcEntity)->GetPkSystem().SendPkToClient();
			}
		}
	}

	//死亡
	if( pTargetEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) == 0)
	{
		((CAnimal*)pTargetEntity)->OnKilledByEntity(pSrcEntity);
	}
	//受到攻击播放后摇
	else
	{
		//{
			BroadCastHit((CAnimal*)pTargetEntity,pTargetEntity->GetHandle());
		//}
	}
 }

 void CSkillResult::ChangeMonsterAttacked( PSKILLRESULTPARAM pParam )
 {
	 if( pParam->pTargetEntity == NULL ||  pParam->pTargetEntity->GetType() != enMonster) return;
	 EntityFlags& flag= pParam->pTargetEntity->GetAttriFlag();
	 if(pParam->pSkillResult->nId ==0)
	 {
		 flag.DenyBeAttack =false;
	 }
	 else
	 {
		 flag.DenyBeAttack =true;
	 }
 }

 void CSkillResult::FastSelfCurHp(PSKILLRESULTPARAM pParam)
 {
	if( pParam->pSrcEntity ==NULL || pParam->pSrcEntity->IsDeath() || pParam->pSkillResult == NULL) return;

	int nRate = pParam->pSkillResult->nValue; //比例
	int nValue = pParam->pSkillResult->nId; //类型：0红，1血，2红和血
	bool boPercent = pParam->pSkillResult->nParam1 == 0 ? true: false;
	float fCureValue= (float)nRate/10000;
	switch(nValue)
	{
		case 0:
			{
				if (boPercent)
				{
					pParam->pSrcEntity->ChangeHP((int)(fCureValue * pParam->pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP)));
				}
				else
				{
					pParam->pSrcEntity->ChangeHP(nRate);
				}

				break;
			}
		case 1:
			{
				if (boPercent)
				{
					pParam->pSrcEntity->ChangeMP((int)(fCureValue * pParam->pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP)));
				}
				else
				{
					pParam->pSrcEntity->ChangeMP(nRate);
				}

				break;
			}
		case 2:
			{
				if (boPercent)
				{
					pParam->pSrcEntity->ChangeHP((int)(fCureValue * pParam->pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP)));
					pParam->pSrcEntity->ChangeMP((int)(fCureValue * pParam->pSrcEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP)));
				}
				else
				{
					pParam->pSrcEntity->ChangeHP(nRate);
					pParam->pSrcEntity->ChangeMP(nRate);
				}

				break;
			}
		default:
			{
				break;
			}
	}
 }

 //把目标移动到范围内的鼠标指定的点
 void CSkillResult::TargetPointMove(PSKILLRESULTPARAM pParam)
 {
      if (pParam->pTargetEntity == NULL || pParam->pSrcEntity == NULL || !pParam->pTargetEntity->isAnimal()) return;
	  int  nPosX, nPosY,isUseRand,nMaxDis ;
	  nPosX  = pParam->nTargetPosX;
	  nPosY  = pParam->nTargetPosY;
	  isUseRand = pParam->pSkillResult->nId;  //是否使用回字查找
	  nMaxDis = pParam->pSkillResult->nValue; //限制移动的最大距离
	  if (nPosX >0 && nPosY >0)
	  {
		  CScene *pScene = pParam->pTargetEntity->GetScene();
		  if (pScene)
		  {
			  int nStartX, nStartY;
			  pParam->pTargetEntity->GetPosition(nStartX, nStartY);
			  //距离的平方
			  INT_PTR nDisSquare = CMovementSystem::GetDisSqare(nStartX,nStartY, nPosX,nPosY);
			  //获取方向
			  INT_PTR nDir = CEntity::GetDir(nPosX,nPosY,nStartX,nStartY);
			  INT_PTR nNewPosX = nPosX;
			  INT_PTR nNewPosY = nPosY;
			  INT_PTR nStep = ((int)sqrt((double)nDisSquare));
			  for (INT_PTR i = 0; i < nStep; i++)
			  {
				  if (!pParam->pTargetEntity->GetMoveSystem()->CanJumpTo(pScene,nNewPosX,nNewPosY))
				  {
					  CEntityGrid::NewPosition(nNewPosX, nNewPosY, nDir, 1);
					  continue;
				  }

				  if (nMaxDis > 0)	//限制一下移动最大的距离
				  {
					  nDisSquare = CMovementSystem::GetDisSqare(nStartX,nStartY, nNewPosX,nNewPosY);
					  if (nDisSquare > (nMaxDis * nMaxDis))
					  {
						  CEntityGrid::NewPosition(nNewPosX, nNewPosY, nDir, 1);
						  continue;
					  }
				  }

				  //找到了,如果一直找不到会在InstantMove()再进行回字查找
				  nPosX = (int)nNewPosX;
				  nPosY = (int)nNewPosY;
				  break;
			  }
		  }
	  }
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
	  pParam->pTargetEntity->GetMoveSystem()->InstantMove(nPosX,nPosY,isUseRand,enDefaultTelePort,nEffId); //瞬间移动
 }

 void CSkillResult::CatchAll(PSKILLRESULTPARAM pParam)
 {
	 if(!pParam->pSkillResult || !pParam->pTargetEntity)
	 {
		 return;
	 }
	 CEntity *  pEntity = pParam->pTargetEntity;
	 if(pEntity->GetAttriFlag().DenyBeMove) return; //如果不能被抓取的话就不能抓取

	 int nPosX,nPosY;
	 pParam->pSrcEntity->GetPosition(nPosX,nPosY);

	 nPosX += pParam->pSkillResult->nId;  //x坐标偏移
	 nPosY += pParam->pSkillResult->nValue; //Y坐标偏移

	 if(pEntity && pEntity->isAnimal() )
	 {
		 ((CAnimal*)pEntity)->GetMoveSystem()->InstantMove(nPosX,nPosY); //瞬间移动
	 }
 }

 void CSkillResult::MonsterMove(PSKILLRESULTPARAM pParam)
 {
	 if(!pParam->pSrcEntity || !pParam->pTargetEntity)
	 {
		 return;
	 }
	 unsigned int nSceneID =  pParam->pSrcEntity->GetSceneID();
	 unsigned int nTargetSceneId = pParam->pTargetEntity->GetSceneID();
	 if (nSceneID != nTargetSceneId)
	 {
		 return;
	 }
	 int nTargetPosX,nTargetPosY;
	 pParam->pTargetEntity->GetPosition(nTargetPosX,nTargetPosY);
	 CFuBen* hFb = pParam->pSrcEntity->GetFuBen();
	 GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(pParam->pSrcEntity,hFb,nSceneID,nTargetPosX,nTargetPosY);
 }

 void   CSkillResult::Shout(PSKILLRESULTPARAM pParam)
 {
	 INT_PTR nShoutType = pParam->pSkillResult->bBuffType;
	 INT_PTR nShoutId =  pParam->pSkillResult->nId;
	 INT_PTR nTipType = pParam->pSkillResult->nValue == 0 ? ttChatWindow : pParam->pSkillResult->nValue;

	 //if(nShoutId)
	 char * pStr = GetLogicServer()->GetDataProvider()->GetMonsterShoutConfig().GetTipmsg(nShoutId); //获取这句话
	 if(pStr ==NULL) return;
	 if(pParam->pSrcEntity ==NULL || nShoutType <= mssInvalid || nShoutType >=mssMax ) return; //参数错误
	 pParam->pSrcEntity->Say((MonsterSayBCType)nShoutType,pStr,(int)nTipType);
 }
