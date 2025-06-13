#include "StdAfx.h"
#include "PropertySystem.h"
#include "../base/Container.hpp"

//重载这个函数,初始化玩家的一些2级属性
bool CPropertySystem::Initialize(void *data,SIZE_T size)
{
	if(m_pEntity == NULL ) return false;
	INT_PTR nEntityType =  m_pEntity->GetHandle().GetType();
	bool bIsUser = nEntityType == enActor;
	bool isNewUser = false; //是否是新角色
	
	if(bIsUser)
	{
		if(data == NULL) return false;
		PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
		if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
		{
			OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
			return false;
		}
		GLOBALCONFIG& globalConfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
		UINT_PTR nVocation = pActorData->nVocation;
		if(nVocation >= enMaxVocCount || nVocation<=0) 
		{

			OutputMsg(rmError,_T("Vocation =%d ,error"), nVocation);
			pActorData->nVocation = 1;
		}

		UINT_PTR nLevel = pActorData->nLevel;
		CFuBen * pFb = NULL;
		
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_VOCATION,pActorData->nVocation);
		m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_ICON,pActorData->nIcon);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_SEX,pActorData->nSex);
		m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_ID,pActorData->nID);
		m_pEntity->SetProperty<int>(PROP_ENTITY_MODELID,pActorData->nModelId);
		//m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_STALL_GRID_COUNT, pActorData->nStallGridCount);

		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_RECOVERSTATE,pActorData->nRecoverState);
		//m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_HONOUR_USED, HIWORD(pActorData->nHonourLv));
		//m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_HONOUR_BUFFDL, pActorData->nHonourBuffDl);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL, pActorData->nSupperPlayLvl);

		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK,	pActorData->nSocialMask);

		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_CREAT_TIME_MIN,	pActorData->nCreateTime);
 

		PVOCATIONINITCONFIG pConfig =&( GetLogicServer()->GetDataProvider()->GetVocationConfig()->vocationTable[nVocation]);

		((CActor*)m_pEntity)->SetAttackType(pConfig->nNearAttackType,false); //设置攻击类型

		if(nLevel == 0) //第1次登陆,需要设置它的一些属性
		{
			((CActor*)m_pEntity)->m_isFirstLogin =true;
			//新创建的角色的朝向是左边和右边随机取一个的
			if(wrandvalue() %2)
			{
				m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_DIR,DIR_LEFT);
			}
			else
			{
				m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_DIR,DIR_RIGHT);
			}

			int nCreateIndex = GetLogicServer()->GetGmCreateIndex();
			if(nCreateIndex == 0)
			{
				nCreateIndex = ((CActor*)m_pEntity)->GetCreateIndex();
			}

			int nScenceID = 0, nPosX = 0, nPosY = 0;
			GetGlobalLogicEngine()->GetFuBenMgr()->GetRandomCreatePos(nCreateIndex, nScenceID, nPosX, nPosY);
			pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(nCreateIndex);
			if (!pFb)
			{
				pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
			}
			((CActor*)m_pEntity)->SetEnterFuBenMapPos(nScenceID, nPosX, nPosY);

			isNewUser = true;
			m_pEntity->SetSceneID(nScenceID); 

			m_pEntity->SetPosition(nPosX, nPosY); //
			m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_LEVEL,(unsigned int)(1)); //1级别
			m_pEntity->SetProperty<Uint64>(PROP_ACTOR_EXP,(Uint64)0);

			m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_CIRCLE, (unsigned int)(0)); //0转
			m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL, (unsigned int)(0)); //0转生灵魄

			//商城播报默认关闭
			unsigned int nProp = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK);
			nProp |= (1 << smStoreBroadcastClose);
			m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK, nProp); 
			
			//if (pConfig->nFbID != 0 )
			//{
			//	pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(pConfig->nFbID);	
			//}
			//m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_ENERGY,nMaxEnergy); //开始都加满

			if(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SEX) == 0)
				m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_ICON,pConfig->nManIcon);
			else
				m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_ICON,pConfig->nFemaleIcon);
			
			ResertProperty();
		}
		else
		{
			CActor* pActor = (CActor*)m_pEntity;
			pActor->m_isFirstLogin =false;	
					
			// 玩家上次存盘的场景位置信息
			m_pEntity->SetSceneID(pActorData->nScenceID); 
			m_pEntity->SetPosition(pActorData->nPosX, pActorData->nPosY);
 
			// 角色上次进入副本前所处的场景位置信息
			pActor->m_nEnterFbScenceID = pActorData->nEnterFbScenceID;
			pActor->m_nEnterFbPosx = LOWORD( pActorData->nEnterFbPos);
			pActor->m_nEnterFbPosy = HIWORD( pActorData->nEnterFbPos);

			// 如果上次在FB里头下线，并且副本当前已经关闭，则设置角色场景位置信息为进入副本前的场景位置信息
			if (pActorData->nFbHandle != 0)
			{
				//pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(pActorData->nFbHandle);		
				//if (!pFb || !pFb->IsFb() || !pFb->HasScene(pActorData->nScenceID))
				//{
					m_pEntity->SetSceneID(pActorData->nEnterFbScenceID);
					m_pEntity->SetPosition(pActor->m_nEnterFbPosx, pActor->m_nEnterFbPosy); //
				//}
			}
			m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_HP,pActorData->nHp);
			m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_MP,pActorData->nMp);
			m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_LEVEL,pActorData->nLevel);
			m_pEntity->SetProperty<Uint64>(PROP_ACTOR_EXP,pActorData->lExp);
			m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_DEPOT_COIN, pActorData->nDepotCoin);
			m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_OFFICE, pActorData->nOffice);

			
			//处理虚拟玩家
			if (((CActor*)m_pEntity)->OnGetIsSimulator())
			{
				int nScenceID = 0, nPosX = 0, nPosY = 0; 
				CSimulatorMgr *pSimuMgr = GetGlobalLogicEngine()->GetSimulatorMgr(); 
				pSimuMgr->OnGetRandSimuPos(nScenceID, nPosX, nPosY);
				if (nPosX != 0 && nPosY != 0)
				{
					//((CActor*)m_pEntity)->SetEnterFuBenMapPos(nScenceID, nPosX, nPosY);
					//m_pEntity->SetPosition(pActorData->nPosX, pActorData->nPosY);
					m_pEntity->SetSceneID(nScenceID);  
					m_pEntity->SetPosition(nPosX, nPosY); //
				} 
				OutputMsg(rmTip, _T("[simulator], 修改的登录点 actorID=%u Scene: %d X: %d Y: %d"),	pActor->GetId(), nScenceID, nPosX, nPosY);
 
			} 
		}

		m_pEntity->SetFuBen(pFb);
		
	}

	if(nEntityType ==enPet || nEntityType == enMonster || nEntityType == enGatherMonster || nEntityType ==enHero )
	{
		ResertProperty();
	}

	//如果不是玩家,那么直接不用刷属性
	
	if(isNewUser || nEntityType ==enPet || nEntityType ==enMonster || nEntityType == enGatherMonster)
	{
		//满血,满蓝
		unsigned int nMaxHp = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP);
		unsigned int nHp = nMaxHp;
		if (nEntityType == enMonster)
		{
			// CMonster *pMonster = static_cast<CMonster *>(m_pEntity);
			// if(pMonster->GetBossReferId() == 0)
			// {
			// 	unsigned int nPercent = m_pEntity->GetProperty<unsigned int>(PROP_MONSTER_INITHPPERCENT);
			// 	float fPercent = nPercent / 100.f;
			// 	nHp = (unsigned int)(nMaxHp * fPercent);
			// }

			// unsigned int nPercent = m_pEntity->GetProperty<unsigned int>(PROP_MONSTER_INITHPPERCENT);
			// float fPercent = nPercent / 100.f;
			// nHp = (unsigned int)(nMaxHp * fPercent);	
		}
		m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_HP,nHp);
		m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_MP,m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP));

	}
	if(nEntityType ==enHero)
	{
		
		CHero *pPet =(CHero *)m_pEntity; //宠物的指针
		//pPet->SetColor()
		
		const CHeroSystem::HERODATA *pPetData = pPet->GetHeroPtr();
		if(pPetData ==NULL) 
		{
			OutputMsg(rmError,_T("%s, pet Data is nil"),__FUNCTION__);
			return false;
		}
		//宠物的属性
		unsigned int nHp = pPetData->data.nHp;
		unsigned int nMp = pPetData->data.nMp;
		//最小的HP
		float fRate = GetLogicServer()->GetDataProvider()->GetGlobalConfig().fPetDeathHpRate ;
		unsigned int nMinHp = (unsigned int)( pPetData->cal.m_AttrValues[aMaxHpAdd].uValue * fRate);
		unsigned int nMinMp = (unsigned int)( pPetData->cal.m_AttrValues[aMaxMpAdd].uValue * fRate );
		if(nHp < nMinHp)
		{
			nHp = nMinHp;
		}
		if(nMp < nMinMp)
		{
			nMp = nMinMp;
		}
		m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_HP,nHp);
		m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_MP,nMp);
		m_pEntity->SetProperty<int>(PROP_CREATURE_LEVEL,pPetData->data.bLevel); //设置宠物的等级
	}
	
	return true;
}

VOID CPropertySystem::Level1ValuePropertyTransfer(CAttrCalc& cal,PLEVEL1PROPERTYTRANSFER pProp )
{
	/*
	下面计算5个1级属性相当于多少2级属性，映射过去,
	1级属性:  力量,身法,体魄,聪慧,精神
	2级属性:  生命,内力,外功攻击,外功防御,外功爆击,外功闪避,外功命中,内功攻击,内功防御,内功爆击(会心),内功闪避(御气)
		内功命中(内准),治疗,HP恢复,MP恢复
	*/
	/*
	for(INT_PTR i=0  ; i< MAX_LEVEL1_PROPERTY_COUNT   ; i++)
	{
		UINT_PTR nLevel1PropId = (i << 1) +1;
		unsigned int nLevel1Value =cal.m_AttrValues[nLevel1PropId].uValue; //1级属性的值，用这个值去乘以一个比例
		if(nLevel1Value ==0) continue; //1级属性的ID
		for(INT_PTR j =0; j <MAX_LEVEL2_PROPERTY_COUNT; j++)
		{
			float fRate = pProp->m_level1PropertyTransTable[i][j]; //这是一个比率，也就是1点1级属性相当于多少2级属性
			if( fRate > 0.0 )
			{
				GAMEATTR attr;
				INT_PTR nLevel2PropId =(j <<1 )+ aMaxHpAdd; //2级属性的ID
				attr.type = (unsigned char)(nLevel2PropId);

				if(AttrDataTypes[nLevel2PropId] == adFloat)
				{
					attr.value.fValue =fRate * nLevel1Value;
				}
				else if(AttrDataTypes[nLevel2PropId] == adUInt)
				{
					attr.value.uValue =(unsigned int) (fRate * nLevel1Value);
				}
				else 
				{
					attr.value.nValue =(int) (fRate * nLevel1Value);
				}
				cal << attr; //把这些1级属性加进去
			}
		}
	}
	*/
}

//初始化怪物的属性
VOID CPropertySystem::InitMonsterProperty(CAttrCalc& cal)
{
	INT_PTR  nMonsterID = m_pEntity->GetId(); //
	
	const PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterID);
	if(pConfig ==NULL)
	{
		OutputMsg(rmWaning,_T("Monster id=%d error,ptr =NULL"),nMonsterID);
		return;
	}
	INT_PTR j =0;
	GAMEATTR attr;
	unsigned int *ptr =(unsigned int*)pConfig ; //指针
	for (INT_PTR i= Level2PropertyStart ; i< Level2PropertyEnd ; i++)
	{
		if (i%2==0) continue;
		attr.type =(unsigned char) i;
		if(AttrDataTypes[i] == adFloat)
		{
			attr.value.fValue = *(float*)(ptr +j); //取值
			if(attr.value.fValue)
			{
				cal << attr;
			}
		}
		else if(AttrDataTypes[i] == adInt)
		{
			attr.value.nValue = *(int*)(ptr +j); //取值
			if(attr.value.nValue)
			{
				cal << attr;
			}
		}
		else if(AttrDataTypes[i] == adUInt)
		{
			attr.value.uValue = *(unsigned int*)(ptr +j); //取值
			if(attr.value.uValue)
			{
				cal << attr;
			}
		}
		j++;
	}
	//monsterConfig[]
}

//刷新属性、计算属性评分
bool  CPropertySystem::ResertProperty()
{
	if(!m_pEntity) return false;

	CAttrCalc cal;
	INT_PTR nEntityType = m_pEntity->GetHandle().GetType(); //实体的类型

	int nDefaultMoveSpeed = GetDefaultMoveSpeed();
	//移动速度和攻击速度放进去要放里边计算
	GAMEATTR attr;
	attr.type = aMoveSpeedAdd;
	attr.value.uValue = nDefaultMoveSpeed;
	cal << attr;

	attr.type = aAttackSpeedAdd;
	attr.value.uValue = GetDefaultAttackSpeed();
	cal << attr;

	attr.type = aFireDefenseRate;	//防火率
	attr.value.uValue = GetFireDefenseRate();
	cal << attr;

	if( nEntityType==enActor)
	{
		CActor* pActor = (CActor *)m_pEntity;
		//装备的属性
		//CAttrCalc equipCal; 
		
		//装备的原始属性，用来计算装备的评分的
		//CAttrCalc equipBaseCal; 
		
		CAttrCalc  basicCal; //等级属性 +装备属性，百分比的基于等级的和装备的属性计算

		#ifdef _DEBUG
			basicCal << m_Debug;
		#endif

		pActor->GetEquipmentSystem().CalcAttributes(basicCal); 
		//OutputMsg(rmTip,_T("ResertProperty1 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		InitActorInitProperty(basicCal);	//初始人物属性
		//OutputMsg(rmTip,_T("ResertProperty2 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		InitActorLevelProperty(basicCal); 	//等级的数据
		//OutputMsg(rmTip,_T("ResertProperty3 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		InitActorCircleProperty(basicCal);	//计算转生属性
		//OutputMsg(rmTip,_T("ResertProperty4 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		InitActorBlessProperty(basicCal);
		//OutputMsg(rmTip,_T("ResertProperty5 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		InitActorMeridianProperty(basicCal);//计算经脉属性
		//OutputMsg(rmTip,_T("ResertProperty6 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		InitActorFrenzyProperty(basicCal);
		//OutputMsg(rmTip,_T("ResertProperty7 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		InitActorOfficeProperty(basicCal);
		//OutputMsg(rmTip,_T("ResertProperty8 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		


		pActor->GetSkillSystem().CalcAttributes(basicCal);		//技能系统
		//OutputMsg(rmTip,_T("ResertProperty9 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		pActor->GetAlmirahSystem().CalcAttr(basicCal);			//翅膀系统
		//OutputMsg(rmTip,_T("ResertProperty10 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		//CalcActorHeadTitleProperty(cal);					//头衔属性
		pActor->GetMiscSystem().CalcRankPropProperty(basicCal);	//属性排行榜属性加成
		//OutputMsg(rmTip,_T("ResertProperty11 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		pActor->GetHeroSystem().CalcAttributes(basicCal);		//英雄附体出战加成
		//OutputMsg(rmTip,_T("ResertProperty12 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		pActor->GetNewTitleSystem().CalcAttributes(basicCal);	//新头衔系统刷新属性
		//OutputMsg(rmTip,_T("ResertProperty13 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		pActor->GetStrengthenSystem().CalcAttributes(basicCal);	//强化属性
		//OutputMsg(rmTip,_T("ResertProperty14 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		pActor->GetGhostSystem().CalcAttributes(basicCal);	 
		//OutputMsg(rmTip,_T("ResertProperty15 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		pActor->GetHallowsSystem().CalcAttributes(basicCal);	 
		//OutputMsg(rmTip,_T("ResertProperty16 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		pActor->GetLootPetSystem().CalcAttributes(basicCal);////宠物系统非 ai宠物 刷新属性	 
		//OutputMsg(rmTip,_T("ResertProperty17 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),basicCal.m_AttrValues[aMaxHpAdd].nValue,basicCal.m_AttrValues[aMaxHpPower].nValue,basicCal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,basicCal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//basicCal.m_AttrValues[aWizardAttackMaxAdd].nValue,basicCal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,basicCal.m_AttrValues[aMagicDefenceMaxAdd].nValue );

		cal << basicCal;

		// pActor->GetSkillSystem().CalcAttributes(cal);		//技能系统
		// pActor->GetAlmirahSystem().CalcAttr(cal);			//翅膀系统
		// //CalcActorHeadTitleProperty(cal);					//头衔属性
		// pActor->GetMiscSystem().CalcRankPropProperty(cal);	//属性排行榜属性加成
		// pActor->GetHeroSystem().CalcAttributes(cal);		//英雄附体出战加成
		// pActor->GetNewTitleSystem().CalcAttributes(cal);	//新头衔系统刷新属性
		// pActor->GetStrengthenSystem().CalcAttributes(cal);	//强化属性
		
		INT_PTR nJob = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION); //玩家的职业

		//buff的属性不计如战力

		CAttrCalc battleCal = cal; //计算战力的属性计算器，buff不计算到战力里边
		CAttrEval *pCal = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetEval(nJob);
		if(pCal)
		{
			int nBattleValue = (int)pCal->GetAttrSetScore(battleCal);	//根据EquipValuation的配置进行评分
			pActor->SetProperty<unsigned int>(PROP_ACTOR_BATTLE_POWER,(unsigned int)nBattleValue);//战力
			// pActor->GetAchieveSystem().OnEvent(aAchiveBattle,nBattleValue);
		}
		battleCal.applyPowerAttributes(basicCal);
		CBuffSystem * pBuffSystem = ((CAnimal*)m_pEntity)->GetBuffSystem();
		if(pBuffSystem)
		{
			pBuffSystem->CalcAttributes(cal);
		}

		m_pEntity->SetProperty(PROP_ACTOR_HP_RATE,basicCal.m_AttrValues[aMaxHpPower].nValue);

		cal.applyPowerAttributes(basicCal); //吧
		//OutputMsg(rmTip,_T("ResertProperty18 nActorId:%d, aMaxHpAdd:%d, aMaxHpPower:%d,aPhysicalAttackMaxAdd:%d,aMagicAttackMaxAdd:%d,aWizardAttackMaxAdd:%d,aPhysicalDefenceMaxAdd:%d,aMagicDefenceMaxAdd:%d"),
		//m_pEntity->GetId(),cal.m_AttrValues[aMaxHpAdd].nValue,cal.m_AttrValues[aMaxHpPower].nValue,cal.m_AttrValues[aPhysicalAttackMaxAdd].nValue,cal.m_AttrValues[aMagicAttackMaxAdd].nValue,
		//cal.m_AttrValues[aWizardAttackMaxAdd].nValue,cal.m_AttrValues[aPhysicalDefenceMaxAdd].nValue,cal.m_AttrValues[aMagicDefenceMaxAdd].nValue );
		
		//Level1ValuePropertyTransfer(cal);
		//如果人物属性
		unsigned int nMoveSpeedLimt = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMoveSpeedLimit;
		if ( cal.m_AttrValues[aMoveSpeedAdd].uValue <= nMoveSpeedLimt)
		{
			cal.m_AttrValues[aMoveSpeedAdd].uValue = nDefaultMoveSpeed;
		}
		SetEntityProperty(cal);
		//记录base_max_hp,base_mas_mp，用作技能的扣除，这2个属性就是等级带来的最大的血和最大的蓝
		pActor->m_nBase_MaxHp = cal.m_AttrValues[aMaxHpAdd].uValue;
		pActor->m_nBase_MaxMp = cal.m_AttrValues[aMaxMpAdd].uValue;
		
		unsigned int nCurHp = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP);
		unsigned int nMaxHp = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP);
		if (nCurHp > nMaxHp)
			m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_HP,nMaxHp);
	}
	else if(nEntityType == enMonster || nEntityType == enGatherMonster ) //怪物的移动属性
	{
		unsigned int nMonsterLevel = m_pEntity->GetProperty<UINT>(PROP_CREATURE_LEVEL);

		PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(
			m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));

		if(pConfig ==NULL ) return false;
		
		CAttrCalc cal; //副本的这些属性加成
		memcpy(&cal,&pConfig->cal,sizeof(CAttrCalc)); //直接把怪物的属性拷贝过来
		updateMonsterDynProp(cal);
		//cal.applyPowerAttributes(pConfig->cal);
		//cal << pConfig->cal;
		// if (pConfig->pflags->CanGrowUp)
		// {
		// 	CAttrCalc originalCal ; //原始的属性
		// 	unsigned int nBornLevel = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
		// 	if(calcMonsterLevelHard(pConfig,originalCal,nBornLevel))
		// 	{
		// 		cal << originalCal;
		// 	}
		// }
		CBuffSystem * pBuffSystem = ((CAnimal*)m_pEntity)->GetBuffSystem();
		if(pBuffSystem)
		{
			pBuffSystem->CalcAttributes(cal);
		}
		cal.applyPowerAttributes(cal); //吧
		SetEntityProperty(cal);		
	}
	else if(nEntityType == enPet ) //宠物
	{
		
		CPet * pPet =(CPet*) m_pEntity;
		/*
		const CPetSystem::PETDATA * pPetData = pPet->GetPetPtr();
		if(pPetData ==NULL)
		{
			OutputMsg(rmError,_T("%s INIT error"),__FUNCTION__);
			return false;
		}
		*/

		//m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_BASE_MAXHP, pPetData->nBaseMaxHp )  ;
		//m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_BASE_MAXMP, pPetData->nBaseMaxMp)  ;

		//cal << pPetData->cal;

		int nLevel = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
		CalcPetBaseProperty(cal, m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID), nLevel);

		/*
		PMONSTERCONFIG pConfig=GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData();
		if(pConfig != NULL)
		{
			int nLevel = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
			if (nLevel > 0 && nLevel < MAX_PET_LEVEL_COUNT)
			{
				unsigned int nColor = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nPetLevels[nLevel - 1];
				pPet->SetColor(nColor);
			}

			cal.applyPowerAttributes(pConfig->cal);
			cal << pConfig->cal;
			CAttrCalc originalCal ; //原始的属性
			if(calcMonsterLevelHard(pConfig,originalCal,nLevel))
			{
				cal << originalCal;
			}
		}
		*/
		//计算buff的属性
		CAttrCalc buffCal; //副本的这些属性加成
		CBuffSystem * pBuffSystem = ((CAnimal*)m_pEntity)->GetBuffSystem();
		if(pBuffSystem)
		{
			pBuffSystem->CalcAttributes(cal);
		}
		//buffCal.applyPowerAttributes(pPetData->basicCal);
		//cal << buffCal;
		SetEntityProperty(cal);		
		

	}
	else if(nEntityType == enHero ) //英雄属性计算
	{
		CHero * pHero =(CHero*) m_pEntity;
		const CHeroSystem::HERODATA * pHeroData = pHero->GetHeroPtr();
		if(pHeroData ==NULL)
		{
			OutputMsg(rmError,_T("%s INIT error"),__FUNCTION__);
			return false;
		}
		cal << pHeroData->cal;
		CAttrCalc &HeroCal = cal;

		//计算buff的属性
		CAttrCalc buffCal; //副本的这些属性加成
		CBuffSystem * pBuffSystem = ((CAnimal*)m_pEntity)->GetBuffSystem();
		if(pBuffSystem)
		{
			pBuffSystem->CalcAttributes(cal);
		}
		cal.applyPowerAttributes(HeroCal);

		SetEntityProperty(cal);	
	}
	return true;
}

/*
计算可成长怪物的成长后的属性
*/
bool CPropertySystem::calcMonsterLevelHard( PMONSTERCONFIG pConfig,CAttrCalc &levelCalc,INT_PTR nBornLevel, const INT_PTR nLevel)
{
// 	if (!pConfig)
// 	{
// 		return false;
// 	}
// 	float fResult ;
// 	INT_PTR nFLevel = nBornLevel;

// 	if ( pConfig->btMonsterType == tagMonsterConfig::mtBoss && pConfig->nMaxHardLv > 0 )
// 	{
// 		nFLevel = pConfig->nLevel;
// 	}

// 	int nLevelDis = (int)nFLevel - pConfig->nOriginalLevel; //等级差
// // 	if(m_pEntity->GetHandle().GetType() == enPet)
// // 	{
// // 		nLevelDis = m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) - 1;
// // 	}
// 	if (nLevel > 0)
// 	{
// 		nLevelDis = (int)nLevel - 1;
// 	}
// 	if(nLevelDis >0)
// 	{
		
// 		fResult = pConfig->fPropA * (float)(nLevelDis * nLevelDis) + pConfig->fPropB * (float)nLevelDis + pConfig->fPropC;
// 	}
// 	else
// 	{
// 		return false;
// 	}


// 	for(INT_PTR i= Level2PropertyStart; i<= aMagicDefenceMaxPower; i++)
// 	{
// 		if (i % 2 == 0)
// 		{
// 			continue;
// 		}

// 		switch(AttrDataTypes[i])
// 		{
// 		case adSmall://有符号1字节类型
// 			levelCalc.m_AttrValues[i].nValue =(char)( pConfig->cal.m_AttrValues[i].nValue * fResult); 
// 			break;
// 		case adUSmall://无符号1字节类型
// 			levelCalc.m_AttrValues[i].nValue =(unsigned char) (pConfig->cal.m_AttrValues[i].nValue * fResult); 
// 			break;
// 		case adShort://有符号2字节类型
// 			levelCalc.m_AttrValues[i].nValue = (short)(pConfig->cal.m_AttrValues[i].nValue * fResult); 
// 			break;
// 		case adUShort://无符号2字节类型
// 			levelCalc.m_AttrValues[i].nValue =(WORD)( pConfig->cal.m_AttrValues[i].nValue * fResult); 
// 			break;
// 		case adInt://有符号4字节类型
// 			levelCalc.m_AttrValues[i].nValue= (int)(pConfig->cal.m_AttrValues[i].nValue * fResult);
// 			break;
// 		case adUInt://无符号4字节类型
// 			levelCalc.m_AttrValues[i].nValue = (unsigned int)(pConfig->cal.m_AttrValues[i].nValue * fResult);
// 			break;
// 		case adFloat://单精度浮点类型值
// 			//baseAttrs.m_AttrValues[i-1].fValue  *= (1+ m_AttrValues[i].fValue); 
// 			levelCalc.m_AttrValues[i].fValue = pConfig->cal.m_AttrValues[i].nValue *  fResult;
// 			break; 
// 		}
		
// 	}
	return true;
}

//获取该实体的移动速度
int  CPropertySystem::GetDefaultMoveSpeed()
{
	INT_PTR nEntityType = m_pEntity->GetHandle().GetType(); //实体的类型
	if(nEntityType == enActor  ||  nEntityType == enHero) //人或者宠物的移动速度是一样的
	{
		return GetLogicServer()->GetDataProvider()->GetVocationConfig()->nDefaultMoveSpeed; 
	}
	else if(nEntityType ==enMonster || nEntityType == enPet)
	{
		return 0;
		//int nMonsterID = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //怪物的ID
		//unsigned int nMonsterLevel = m_pEntity->GetProperty<UINT>(PROP_CREATURE_LEVEL);
		////const PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterID);
		//PMONSTERCONFIG pConfig = 
		//	GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(
		//	nMonsterID);
		//if(pConfig ==NULL )
		//{
		//	return 0;
		//}
		//else
		//{
		//	return pConfig->cal.m_AttrValues[aMoveSpeedAdd].uValue; //返回移动速度
		//}
	}
	else
	{
		return 0;
	}
}
int CPropertySystem::GetDefaultAttackSpeed()
{
	INT_PTR nEntityType = m_pEntity->GetHandle().GetType(); 
	if(nEntityType == enActor) 
	{
		return GetLogicServer()->GetDataProvider()->GetVocationConfig()->nDefaultAttackSpeed; 
	}else if(nEntityType == enHero)
	{
	   return  GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroAttackSpeed();
	}
	else if(nEntityType ==enMonster || nEntityType == enPet)
	{
		return 0;
		//int nMonsterID = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //怪物的ID
		////const PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterID);
		//PMONSTERCONFIG pConfig  = 
		//	GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(
		//	nMonsterID);
		//if(pConfig ==NULL)
		//{
		//	return 0;
		//}
		//else
		//{
		//	return pConfig->cal.m_AttrValues[aAttackSpeedAdd].uValue; //返回攻击速度
		//}
	}
	else 
	{
		return 10000000; //这是个很大的数
	}
}

VOID CPropertySystem::InitMonsterCalculator( CREATURBATTLEEDATA & data, CAttrCalc& cal )
{
	unsigned int *ptr =(unsigned int*)&data.nMaxHp ; //指针，属性的第1个
	GAMEATTR attr;
	INT_PTR	 j=0;
	for (INT_PTR i= Level2PropertyStart ; i< Level2PropertyEnd ; i++)
	{
		if ( (i -Level2PropertyStart)%2==1) continue;
		attr.type =(unsigned char) i;
		if(AttrDataTypes[i] == adFloat)
		{
			attr.value.fValue = *(float*)(ptr +j); //取值
			if(attr.value.fValue)
			{
				cal << attr;
			}
		}
		else if(AttrDataTypes[i] == adInt)
		{
			attr.value.nValue = *(int*)(ptr +j); //取值
			if(attr.value.nValue)
			{
				cal << attr;
			}
		}
		else if(AttrDataTypes[i] == adUInt)
		{
			attr.value.uValue = *(unsigned int*)(ptr +j); //取值
			if(attr.value.uValue)
			{
				cal << attr;
			}
		}
		j++;
	}


	for (INT_PTR i= Level2PropertyEnd ; i< BattlePropertyCount ; i++)
	{
		attr.type =(unsigned char) i;
		if(AttrDataTypes[i] == adFloat)
		{
			attr.value.fValue = *(float*)(ptr +j); //取值
			if(attr.value.fValue)
			{
				cal << attr;
			}
		}
		else if(AttrDataTypes[i] == adInt)
		{
			attr.value.nValue = *(int*)(ptr +j); //取值
			if(attr.value.nValue)
			{
				cal << attr;
			}
		}
		else if(AttrDataTypes[i] == adUInt)
		{
			attr.value.uValue = *(unsigned int*)(ptr +j); //取值
			if(attr.value.uValue)
			{
				cal << attr;
			}
		}
		j++;
	}
}

VOID  CPropertySystem::SetEntityProperty(CAttrCalc& cal)
{
	//设置1级属性,只有玩家才有
	INT_PTR nEntityType =m_pEntity->GetHandle().GetType();

	bool isActor =  (nEntityType== enActor);

	int nVocation = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_VOCATION);	//职业
	if(isActor)
	{
		
		CActor *pActor = (CActor *)m_pEntity;

		int nLevel = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //等级
		//玩家剩余的多倍经验
	//	m_pEntity->SetProperty<unsigned long long>(PROP_ACTOR_MAX_EXP, 
	//		GetLogicServer()->GetDataProvider()->GetLevelUpExp().GetLevelExp(nLevel+1));
		float fValue =1.0; //默认的打怪经验是1.0
		fValue += (float)cal.m_AttrValues[aExpPower].nValue/(float)10000; //经验倍率的增加
		pActor->m_fExpRate = fValue;
		float fCoinValue =1.0; //默认的金币倍率是1.0
		fCoinValue += (float)cal.m_AttrValues[aBuffmtBindCoinAddRate].nValue/(float)10000; //金币加成
		pActor->m_fBindCoinRate = fCoinValue;

		//设置一些面板属性
		m_pEntity->SetProperty<int>(PROP_ACTOR_CRIT_RATE, cal.m_AttrValues[aCritDamageRate].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_CRIT_POWER, cal.m_AttrValues[aCritDamageValue].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_DEDUCT_CRIT, cal.m_AttrValues[aDeductCriteDamageRate].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_CRIT_MUTRATE, cal.m_AttrValues[aCritDamagePower].nValue);
		//m_pEntity->SetProperty<float>(PROP_ACTOR_DEDUCT_DAMAGE, cal.m_AttrValues[aDeductDamagePower].fValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_DEDUCT_DAMAGE, cal.m_AttrValues[aDamageAbsorbRate].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_IGNORDEFENCE, cal.m_AttrValues[aIgnorDefenceRatio].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_SUCKBLOOD, cal.m_AttrValues[aSuckBloodRate].nValue < 0 ? 0 : cal.m_AttrValues[aSuckBloodRate].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_EXP_POWER, cal.m_AttrValues[aExpPower].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_LOOTBINDCOIN, cal.m_AttrValues[aBuffmtBindCoinAddRate].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_CUT, cal.m_AttrValues[aCuttingRate].nValue);
		m_pEntity->SetProperty<int>(PROP_ACTOR_DAMAGEBONUS, cal.m_AttrValues[aDoubleAtkRate].nValue);

		if(cal.m_AttrValues[aNormalAttackAcc].nValue < 0)
		{
			m_pEntity->SetProperty<int>(PROP_ACTOR_NORMAL_ATK_ACC, 0);
		} else {
			m_pEntity->SetProperty<int>(PROP_ACTOR_NORMAL_ATK_ACC, cal.m_AttrValues[aNormalAttackAcc].nValue);
		}
		
		m_pEntity->SetProperty<int>(PROP_ACTOR_PK_DEDUCT_DAMAGE, cal.m_AttrValues[aPkDamageAbsorbRate].nValue);
		
		//英雄多倍经验
       ( (CActor*)m_pEntity)->GetHeroSystem().m_fExpRate = cal.m_AttrValues[aBuffHeroExpRate].fValue; 
	}

	//设置2级属性,如外攻攻击等
	int nBaseId = aMaxHpAdd;
	int nBasePropertyID = PROP_CREATURE_MAXHP;
	for(INT_PTR i =nBaseId;i < Level2PropertyEnd ; i++)
	{
		int nDis= (int)(i - nBaseId);
		if( nDis %2==1) continue;
		INT_PTR nPropID  = ( nDis >> 1 ) + nBasePropertyID;
		//if(cal.m_AttrValues[i].nValue ==0) continue;
		if(AttrDataTypes[i] == adFloat)
		{
			//修复了魔法闪避可能为负数的问题
			//PROP_CREATURE_MAGIC_DOGERATE--改为int了
			if( nPropID == PROP_CREATURE_MAGIC_DOGERATE && cal.m_AttrValues[i].fValue <0)
			{
				m_pEntity->SetProperty<float>((int)nPropID,0.0);
			}
			else
			{
				m_pEntity->SetProperty<float>((int)nPropID,cal.m_AttrValues[i].fValue);
			}
		}
		else if(AttrDataTypes[i] == adInt)
		{
			
			if(
				(nPropID == PROP_CREATURE_PHYSICAL_ATTACK_MIN || 
				nPropID == PROP_CREATURE_PHYSICAL_ATTACK_MAX || 
				nPropID == PROP_CREATURE_MAGIC_ATTACK_MIN || 
				nPropID == PROP_CREATURE_MAGIC_ATTACK_MAX || 
				nPropID == PROP_CREATURE_WIZARD_ATTACK_MIN ||
				nPropID == PROP_CREATURE_WIZARD_ATTACK_MAX ||
				nPropID == PROP_CREATURE_PYSICAL_DEFENCE_MIN ||

				nPropID == PROP_CREATURE_PYSICAL_DEFENCE_MAX ||
				nPropID == PROP_CREATURE_MAGIC_DEFENCE_MIN ||
				nPropID == PROP_CREATURE_MAGIC_DEFENCE_MAX ||
				nPropID == PROP_CREATURE_HP_RENEW 
				
				) && cal.m_AttrValues[i].nValue <0 )
			{
				m_pEntity->SetProperty<int>((int)nPropID,0); //这4个属性最多为0，不能小于0
			}
			else
			{
				m_pEntity->SetProperty<int>((int)nPropID,cal.m_AttrValues[i].nValue);
			}
			
		}
		else if(AttrDataTypes[i] == adUInt)
		{
			m_pEntity->SetProperty<unsigned int>((int)nPropID,cal.m_AttrValues[i].uValue);
		}

		
	}

	if(isActor && m_pEntity->IsInited())//初始化之后才能调用脚本
	{
		CActor *pActor = (CActor *)m_pEntity;

		//战士
		enPropCreature MinValueType = PROP_CREATURE_PHYSICAL_ATTACK_MIN;
		enPropCreature MaxValueType = PROP_CREATURE_PHYSICAL_ATTACK_MAX;
		//法师
		if(nVocation == enVocMagician)
		{
			MinValueType = PROP_CREATURE_MAGIC_ATTACK_MIN;
			MaxValueType = PROP_CREATURE_MAGIC_ATTACK_MAX;
		}
		//道士
		else if(nVocation == enVocWizard)
		{
			MinValueType = PROP_CREATURE_WIZARD_ATTACK_MIN;
			MaxValueType = PROP_CREATURE_WIZARD_ATTACK_MAX;
		}

		int nMinAttrValue = m_pEntity->GetProperty<unsigned int>(MinValueType); 
		int nMaxAttrValue = m_pEntity->GetProperty<unsigned int>(MaxValueType);

		nMinAttrValue += cal.m_AttrValues[aAllAttackMinAdd].nValue;
		nMaxAttrValue += cal.m_AttrValues[aAllAttackMaxAdd].nValue;

		// 全职业属性攻击
		m_pEntity->SetProperty<int>(MinValueType,nMinAttrValue);
		m_pEntity->SetProperty<int>(MaxValueType,nMaxAttrValue);

		// 最大攻击，不知道干嘛的 TODO: 待查
		if( nMaxAttrValue > ACHIEVE_ATTR_VALUE )//此数值由策划定
		{
			//最大攻击达到某只完成一个成就任务
			// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtAttrMax, 0, nMaxAttrValue, false);
		}

		//内功
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_HALFMONTHS_INCREASEDAMAGE, cal.m_AttrValues[aHalfMonthsIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FIRE_INCREASEDAMAGE, cal.m_AttrValues[aFireIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_DAYBYDAY_INCREASEDAMAGE, cal.m_AttrValues[aDayByDayIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_ICESTORM_INCREASEDAMAGE, cal.m_AttrValues[aIceStormIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FIRERAIN_INCREASEDAMAGE, cal.m_AttrValues[aFireRainIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_THUNDER_INCREASEDAMAGE, cal.m_AttrValues[aThunderIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_BLOODBITE_INCREASEDAMAGE, cal.m_AttrValues[aBloodBiteIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FIRESIGN_INCREASEDAMAGE, cal.m_AttrValues[aFireSignIncreaseDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_HALFMONTHS_REDUCEDAMAGE, cal.m_AttrValues[aHalfMonthsReduceDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FIRE_REDUCEDAMAGE, cal.m_AttrValues[aFireReduceDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_DAYBYDAY_REDUCEDAMAGE, cal.m_AttrValues[aDayByDayReduceDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_ICESTORM_REDUCEDAMAGE, cal.m_AttrValues[aIceStormReduceDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FIRERAIN_REDUCEDAMAGE, cal.m_AttrValues[aFireRainReduceDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_THUNDER_REDUCEDAMAGE, cal.m_AttrValues[aThunderReduceDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_BLOODBITE_REDUCEDAMAGE, cal.m_AttrValues[aBloodBiteReduceDamage].nValue);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FIRESIGN_REDUCEDAMAGE, cal.m_AttrValues[aFireSignReduceDamage].nValue);
	}
	

	//设置一些战斗属性
	if(m_pEntity->isAnimal())
	{
		CAnimal * pAnimal = (CAnimal*) m_pEntity;
		
		//pAnimal->SetDeductDamagePower(cal.m_AttrValues[aDeductDamagePower].fValue);  //抵扣伤害
		
		pAnimal->m_nFireDefenseRate = (WORD)(cal.m_AttrValues[aFireDefenseRate].nValue); //抗火率 使用1点表示1万分之1
		if (nEntityType == enActor || nEntityType == enHero)
		{
			CAdvanceAnimal *pAdAnimal = (CAdvanceAnimal *)pAnimal;
			
			pAdAnimal->m_nReduceEquipDropRate =(cal.m_AttrValues[aReduceEquipDropRate].nValue); //奖励装备的爆率

			pAdAnimal->m_nWarriorDamageValueDec =(cal.m_AttrValues[aWarriorDamageValueDec].nValue); //固定值降低战士的伤害
			pAdAnimal->m_nWarriorDamageRateDec = (WORD)(cal.m_AttrValues[aWarriorDamageRateDec].nValue);  //百分比降低战士的伤害
			
			pAdAnimal->m_nMagicianDamageValueDec =(cal.m_AttrValues[aMagicianDamageValueDesc].nValue); //固定值降低法师的伤害
			pAdAnimal->m_nMagicianDamageRateDec = (WORD)(cal.m_AttrValues[aMagicianDamageRateDesc].nValue); //百分比降低法师的伤害

			pAdAnimal->m_nWizardDamageValueDec = (cal.m_AttrValues[aWizardDamageValueDesc].nValue); //固定值降低道士的伤害
			pAdAnimal->m_nWizardDamageRateDec = (WORD)(cal.m_AttrValues[aWizardDamageRateDesc].nValue);  //百分比降低道士的伤害
			
			pAdAnimal->m_nMonsterDamageValueDec = (cal.m_AttrValues[aMonsterDamageValueDesc].nValue); //固定值降低怪物的伤害
			pAdAnimal->m_nMonsterDamageRateDec = (WORD)(cal.m_AttrValues[aMonsterDamageRateDesc].nValue); //百分比降低怪物的伤害


			
			pAdAnimal->m_nDamageReduceRate =(WORD)(cal.m_AttrValues[aDamageReduceRate].nValue); //触发伤害减免的几率
			pAdAnimal->m_nDamageReduceValue = (cal.m_AttrValues[aDamageReduceValue].nValue); //触发伤害减免的值

			pAdAnimal->m_nDamageAddRate = (WORD)(cal.m_AttrValues[aDamageAddRate].nValue); //触发伤害追加的几率
			pAdAnimal->m_nDamageAddValue = (cal.m_AttrValues[aDamageAddValue].nValue);  //触发伤害追加的值

			pAdAnimal->m_nIgnorDefenceRate =(WORD) (cal.m_AttrValues[aIgnorDefenceRate].nValue); //触发无视防御几率
			pAdAnimal->m_nIgnorDefenceRateSkill = (cal.m_AttrValues[aIgnorDefenceRateSkill].nValue); //技能触发无视防御几率

			pAdAnimal->m_nIgnorDefenceRatio = (cal.m_AttrValues[aIgnorDefenceRatio].nValue); //无视防御比例
			pAdAnimal->m_nSuckBloodRatio = (cal.m_AttrValues[aSuckBloodRate].nValue); //吸血

			if (nEntityType == enActor)
			{
				//---神装
				m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR1,pAdAnimal->m_nWarriorDamageValueDec); //降低受战士伤害值
				m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR2,pAdAnimal->m_nMagicianDamageValueDec); //降低受法师伤害值
				m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR3,pAdAnimal->m_nWizardDamageValueDec); //降低受道士伤害值
				m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR4,pAdAnimal->m_nMonsterDamageValueDec); //降低受怪物伤害值
			}
		}
		//攻魔道属性增加 万分比
		int nAttkRatio =cal.m_AttrValues[aAllAttackAddRate].nValue; //
		// if(nAttkRatio > 0) {
			AddPropertyRatioValue(PROP_CREATURE_PHYSICAL_ATTACK_MAX, nAttkRatio);
			AddPropertyRatioValue(PROP_CREATURE_PHYSICAL_ATTACK_MIN, nAttkRatio);
			AddPropertyRatioValue(PROP_CREATURE_MAGIC_ATTACK_MAX, nAttkRatio);
			AddPropertyRatioValue(PROP_CREATURE_MAGIC_ATTACK_MIN, nAttkRatio);
			AddPropertyRatioValue(PROP_CREATURE_WIZARD_ATTACK_MIN, nAttkRatio);
			AddPropertyRatioValue(PROP_CREATURE_WIZARD_ATTACK_MAX, nAttkRatio);
		// }
		//双防加成 万分比
		int nDefRatio =cal.m_AttrValues[aDoubleDefenseRate].nValue; //
		if(nDefRatio > 0) {
			AddPropertyRatioValue(PROP_CREATURE_PYSICAL_DEFENCE_MIN, nDefRatio);
			AddPropertyRatioValue(PROP_CREATURE_PYSICAL_DEFENCE_MAX, nDefRatio);
			AddPropertyRatioValue(PROP_CREATURE_MAGIC_DEFENCE_MIN, nDefRatio);
			AddPropertyRatioValue(PROP_CREATURE_MAGIC_DEFENCE_MAX, nDefRatio);
		}
		
		// //最大血
		// int nMaxHpRatio =cal.m_AttrValues[aMaxHpPower].nValue; //
		// if(nMaxHpRatio > 0) {
		// 	AddPropertyRatioValue(PROP_CREATURE_MAXHP, nMaxHpRatio);
		// 	cal.m_AttrValues[aMaxHpAdd].nValue = cal.m_AttrValues[aMaxHpAdd].nValue*(1+nMaxHpRatio/10000.0);
		// }
		pAnimal->m_nWarriorTargetDamageValue = (int)cal.m_AttrValues[aWarriorTargetDamageValue].nValue;   //固定值增加对战士的伤害
		pAnimal->m_nWarriorTargetDamageRate = (int)cal.m_AttrValues[aWarriorTargetDamageRate].nValue;		//百分比增加对战士的伤害
		pAnimal->m_nMagicianTargetDamageValue = (int)cal.m_AttrValues[aMagicianTargetDamageValue].nValue; //固定值增加对法师的伤害
		pAnimal->m_nMagicianTargetDamageRate = (int)cal.m_AttrValues[aMagicianTargetDamageRate].nValue;	//固定比增加对法师的伤害
		pAnimal->m_nWizardTargetDamageValue = (int)cal.m_AttrValues[aWizardTargetDamageValue].nValue;		//固定值增加对道士的伤害
		pAnimal->m_nWizardTargetDamageRate = (int)cal.m_AttrValues[aWizardTargetDamageRate].nValue;		//固定比增加对法师的伤害
		pAnimal->m_nMonsterTargetDamageValue = (int)cal.m_AttrValues[aMonsterTargetDamageValue].nValue;	//固定值增加对怪物的伤害 
		pAnimal->m_nMonsterTargetDamageRate = (int)cal.m_AttrValues[aMonsterTargetDamageRate].nValue;		//固定比增加对怪物的伤害
		pAnimal->m_nAddAllDamageRate = (int)cal.m_AttrValues[aAddAllDamageRate].nValue;						//全职业攻击伤害追加概率
		pAnimal->m_fAddAllDamagePower = (float)cal.m_AttrValues[aAddAllDamagePower].fValue;					//全职业攻击时附加攻击输出的倍率增加
		pAnimal->m_nDamageReboundRate = (int)cal.m_AttrValues[aAddDamageReboundRate].nValue;					//伤害反弹 概率
		pAnimal->m_fDamageReboundPower = (float)cal.m_AttrValues[aAddDamageReboundPower].fValue;				//伤害反弹 倍率
		pAnimal->m_nAddAllDamageRateEx = (int)cal.m_AttrValues[aAddAllDamageRateEx].nValue;						//会心一击伤害追加概率
		pAnimal->m_nCritDamageRate = (int)cal.m_AttrValues[aCritDamageRate].nValue;				//新版暴击几率
		pAnimal->m_nCritDamageValue = (int)cal.m_AttrValues[aCritDamageValue].nValue;			//新版暴击力
		pAnimal->m_nCritDamagePower = (int)cal.m_AttrValues[aCritDamagePower].nValue;			//新版暴击倍率
		pAnimal->m_nDeductCritDamageRate = (int)cal.m_AttrValues[aDeductCriteDamageRate].nValue;//抗暴
		int fMedicamentRenew = cal.m_AttrValues[aFastMedicamentRenew].nValue;
		if (nEntityType == enActor)
		{
			m_pEntity->SetProperty<int>(PROP_ACTOR_MEDIC_RENEW,fMedicamentRenew); //药品回复率
			//---神装
			m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR5,pAnimal->m_nWarriorTargetDamageValue); //增加对战士伤害值
			m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR6,pAnimal->m_nMagicianTargetDamageValue); //增加对法师伤害值
			m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR7,pAnimal->m_nWizardTargetDamageValue); //增加对道士伤害值
			m_pEntity->SetProperty<int>(PROP_ACTOR_GOLDEQ_ATTR8,pAnimal->m_nMonsterTargetDamageValue); //增加对怪物伤害值

			//护身
			m_pEntity->SetProperty<int>(PROP_ACTOR_HP_2_MP_DAMAGE,cal.m_AttrValues[aMpReplaceHpRate].nValue); //增加对怪物伤害值
			
		}
		pAnimal->m_fDamgeAbsorbRate = (float)(cal.m_AttrValues[aDamageAbsorbRate].nValue)/(float)10000;

		//Hp转换为Mp的比例
		pAnimal->SetHp2MpRate(cal.m_AttrValues[aMpReplaceHpRate].nValue);  
 
		pAnimal->m_nDizzyValue = cal.m_AttrValues[aDizzyRateAdd].nValue;  		//麻痹
		pAnimal->m_nSaviorTime = cal.m_AttrValues[aSaviorTime].nValue;    		//救主灵刃-CD时间
		pAnimal->m_nSaviorRate = cal.m_AttrValues[aSaviorRate].nValue;    		//救主灵刃-回复率
		pAnimal->m_nSaviorValue = cal.m_AttrValues[aSaviorValue].nValue;  		//救主灵刃-回复值
		pAnimal->m_nToxicRate = cal.m_AttrValues[aToxicRate].nValue;  	  		//剧毒裁决-淬毒几率
		pAnimal->m_nToxicDamage = cal.m_AttrValues[aToxicDamage].nValue;  		//剧毒裁决-淬毒伤害
		pAnimal->m_nToxicEffect = cal.m_AttrValues[aToxicEffect].nValue;  		//剧毒裁决-效果
		pAnimal->m_nFrozenStrength = cal.m_AttrValues[aFrozenStrength].nValue;  //霜之哀伤-冰冻强度
		pAnimal->m_nHpRenewAdd = cal.m_AttrValues[aHpRenewAdd].nValue;  		//生命恢复值增加
		pAnimal->m_nDeductDizzyValue = cal.m_AttrValues[aDeductDizzyRate].nValue;  //抗麻
		pAnimal->m_nDizzyTimeAdd = cal.m_AttrValues[aDizzyTimeAdd].nValue;  //麻痹时长增加 
		pAnimal->m_nDizzyTimeAbsorbAdd = cal.m_AttrValues[aDizzyTimeAbsorbAdd].nValue;  //麻痹时长减免  
		pAnimal->m_nGuardRate = cal.m_AttrValues[aGuardRate].nValue;  //守护 
		pAnimal->m_nGuardValue = cal.m_AttrValues[aaGuardValue].nValue;  //守护值
		pAnimal->m_nCuttingRate = cal.m_AttrValues[aCuttingRate].nValue;  //切割 
		pAnimal->m_nDoubleAtkRate = cal.m_AttrValues[aDoubleAtkRate].nValue;  //倍攻 
		//pk伤害减免
		pAnimal->m_nPkDamageAbsorbRatio = cal.m_AttrValues[aPkDamageAbsorbRate].nValue;  //

		//死亡的刷的hp比例
		pAnimal->m_DieRefreshHpPro =(int) (cal.m_AttrValues[aDieRefreshHpPro].fValue *100);

		//治疗的衰减
		//float fValue =1.0;
		//fValue +=  cal.m_AttrValues[aSkillCureRateAdd].fValue; //治疗的增加的
		//	pAnimal->SetCureRate(fValue);

	}
	if(nEntityType == enMonster)
	{
		// CMonster *pMonster = static_cast<CMonster *>(m_pEntity);
		//unsigned int nPercent = m_pEntity->GetProperty<unsigned int>(PROP_MONSTER_INITHPPERCENT); 
		// if(pMonster && pMonster->GetBossReferId() && pMonster->GetBuffPercent() )
		// {
		// 	int nPercent = pMonster->GetBuffPercent();
		// 	pMonster->DealBossGrowAttr();
		// 	cal.m_AttrValues[aMaxHpAdd].nValue = cal.m_AttrValues[aMaxHpAdd].nValue*((float)(nPercent)/100));
			
		// }
		unsigned int nPercent = m_pEntity->GetProperty<unsigned int>(PROP_MONSTER_INITHPPERCENT); 
		if(nPercent > 0 )
		{
			AddPropertyValue(PROP_CREATURE_PHYSICAL_ATTACK_MAX, nPercent);
			AddPropertyValue(PROP_CREATURE_PHYSICAL_ATTACK_MIN, nPercent);
			AddPropertyValue(PROP_CREATURE_MAGIC_ATTACK_MAX, nPercent);
			AddPropertyValue(PROP_CREATURE_MAGIC_ATTACK_MIN, nPercent);
			AddPropertyValue(PROP_CREATURE_WIZARD_ATTACK_MIN, nPercent);
			AddPropertyValue(PROP_CREATURE_WIZARD_ATTACK_MAX, nPercent);
			AddPropertyValue(PROP_CREATURE_MAXHP, nPercent);
			cal.m_AttrValues[aMaxHpAdd].nValue = cal.m_AttrValues[aMaxHpAdd].nValue*((float)(nPercent)/100);
			
		}
	}

	//这里是处理命，蓝加成
	if ((int)m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > cal.m_AttrValues[aMaxHpAdd].nValue)
	{
		m_pEntity->ChangeHP(cal.m_AttrValues[aMaxHpAdd].nValue);
	}
	if ((int)m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MP) > cal.m_AttrValues[aMaxMpAdd].nValue)
	{
		m_pEntity->ChangeMP(cal.m_AttrValues[aMaxMpAdd].nValue);
	}

}

void CPropertySystem::InitEntityLevelProperty(CAttrCalc &cal, PENTITYLEVELPROPERTY pVoc,INT_PTR nLevel)
{
	if (pVoc == NULL) return;

	GAMEATTR attr;
	for(INT_PTR i=0; i< LEVEL_PROPERTY_COUNT; i++)
	{
		//数值由策划直接配置，现直接读取
		attr.value.nValue =  (int)pVoc->levelProp[i];
		attr.type = (unsigned char )((i << 1) +aMaxHpAdd);	
		cal << attr;
	}
}

bool  CPropertySystem::InitActorInitProperty(CAttrCalc &cal)
{
	 AttriGroup& attr = GetLogicServer()->GetDataProvider()->GetVocationConfig()->initAttrs;
	 if (attr.nCount > 0)
	 {
		 for (int i=0; i<attr.nCount; i++)
		 {
			 cal << attr.pAttrs[i];
		 }
	 }
	 return true;
}

//玩家的等级带来的属性
bool  CPropertySystem::InitActorLevelProperty(CAttrCalc &cal)
{
	//PVOCATIONINITTABLE pData = GetLogicServer()->GetDataProvider()->GetVocationConfig();

	//通过这几个属性来初始化1级属性和2级属性
	unsigned nLevel = m_pEntity->GetProperty<unsigned>(PROP_CREATURE_LEVEL); //读取他的等级
	unsigned nSex = m_pEntity->GetProperty<unsigned>(PROP_ACTOR_SEX); //性别
	unsigned nVocation =  m_pEntity->GetProperty<unsigned>(PROP_ACTOR_VOCATION); //职业
	if(nVocation >= enMaxVocCount)
	{
		OutputMsg(rmError,_T("Vocation error =%d"),nVocation); //属性失败了
		return false;
	}
	//PENTITYLEVELPROPERTY pVoc = & pData->vocationTable[nVocation].actorProp;
	PENTITYLEVELPROPERTY pData = GetLogicServer()->GetDataProvider()->GetVocationConfig()->GetVocationLevelPropParam(enActor, nVocation, nLevel);
	InitEntityLevelProperty(cal,pData,nLevel);
	return true;
}

void CPropertySystem::updateMonsterDynProp(CAttrCalc &calc)
{
	INT_PTR nEntityType = m_pEntity->GetHandle().GetType(); 
	if (nEntityType != enMonster)
		return;

	CMonster *pMonster = static_cast<CMonster *>(m_pEntity);
	if (!pMonster)
		return;

	// 如果怪物在副本，检测怪物等级和副本玩家平均等级差值
	CScene *pScene = pMonster->GetScene();
	if (!pScene)
		return;
	CFuBen *pFB = pScene->GetFuBen();
	if (!pFB || !pFB->IsFb())	// 在副本中
		return;
	
	int nPlayerLvlDiff = pMonster->GetProperty<unsigned int>(PROP_MONSTER_LEVEL_DIFF);	// 可能小于0
	if (nPlayerLvlDiff < 0)
		nPlayerLvlDiff = 0;
		/*return;*/
	SceneNpcDynPropConfig &cfg = GetLogicServer()->GetDataProvider()->GetNpcDynPropConfig();
	nPlayerLvlDiff = (int)__min(nPlayerLvlDiff, cfg.vecDynPropRate.count()-1);
	const NpcDynPropListRate &dynPropListRate = cfg.vecDynPropRate.get(nPlayerLvlDiff);
	
	// 获取队伍人数对副本怪物属性的提升
	int nTeamCount = pMonster->GetFubenTeammemberCount();
	nTeamCount = __max(1, __min(nTeamCount, MAX_TEAM_MEMBER_COUNT));	// 副本没人也按照1个人来计算属性提升系数
	const NpcDynPropListRate &nTeamRate = cfg.teamNumAdjustRateInfo[nTeamCount-1];
	GAMEATTR atts[] = {
		{DPCT_HPRate,		0, 0 }, 

		//{DPCT_OuterAttack,	0, 0 }, 
		//{DPCT_InnerAttack,	0, 0 },
		/*{DPCT_OuterDefence, 0, 0},
		{DPCT_InnerDefence, 0, 0},*/
	};

	for (int i = 0; i < NPC_DYN_PROP_COUNT; i++)
	{
		atts[i].value.fValue = ((dynPropListRate.fRate[i]+1.0f) * (nTeamRate.fRate[i]+1.0f) - 1.0f);
		calc << atts[i];
	}
}


bool CPropertySystem::CalcActorHeadTitleProperty(CAttrCalc &calc)
{
	/*	
	CAchieveProvider &provider = GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	//计算每位获取头像
	//unsigned nCurnewTitel = m_pEntity->GetProperty<unsigned>(PROP_ACTOR_CURNEWTITLE); //头衔

	for (INT_PTR i = 0; i < MAX_HEADTITLE_COUNT; ++i)
	{
		if (!((CActor *)m_pEntity)->HasHeadTitle(i))
			continue;

		provider.CalcHeadTitleAttributes(i, calc);
	}
	*/
	return true;
}

//转生属性计算
bool CPropertySystem::InitActorCircleProperty(CAttrCalc &calc)
{
	if (!m_pEntity || ((CEntity *)m_pEntity)->GetType() != enActor) 
		return false;
    CActor* pActor = (CActor*)m_pEntity;
	int  nCircle = pActor->GetProperty<int>(PROP_ACTOR_CIRCLE);
	CIRCLECFG &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_CircleLevelConfig;
	if(nCircle > 0 && nCircle <= cfg.m_lCircleLv.size())
	{
		std::map<int, CIRCLELEVEL>::iterator it = cfg.m_lCircleLv.find(nCircle);
		if(it != cfg.m_lCircleLv.end())
		{
			CIRCLELEVEL &pAttr  = it->second;//cfg.m_lCircleLv[nCircle - 1];
			for(int  i = 0; i < pAttr.attri.nCount; i++)
			{
				calc << pAttr.attri.pAttrs[i];
			}
		}
	}
	return true;
}


//祝福属性计算
bool CPropertySystem::InitActorBlessProperty(CAttrCalc &calc)
{
	if (!m_pEntity || ((CEntity *)m_pEntity)->GetType() != enActor) 
		return false;
    CActor* pActor = (CActor*)m_pEntity;
	int  nValue = pActor->GetProperty<int>(PROP_ACTOR_Bless);
	BlessCfg* cfg = GetLogicServer()->GetDataProvider()->GetBlessCfg().GetBlessCfg(nValue);
	if(nValue > 0 && cfg)
	{
		for(int  i = 0; i < cfg->attri.nCount; i++)
		{
			calc << cfg->attri.pAttrs[i];
		}
	}
	return true;
}


//经脉属性计算
bool CPropertySystem::InitActorMeridianProperty(CAttrCalc &calc)
{
	if (!m_pEntity || ((CEntity *)m_pEntity)->GetType() != enActor) 
		return false;
    CActor* pActor = (CActor*)m_pEntity;
	int  nLv =pActor->GetProperty<int>(PROP_ACTOR_MERIDIALv);
	std::map<int, MeridianUpCfg> &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_MeridianUpCfg;
	if(nLv > 0 && nLv <= cfg.size())
	{
		std::map<int, MeridianUpCfg>::iterator it = cfg.find(nLv);
		if(it != cfg.end())
		{
			MeridianUpCfg &pAttr  = it->second;
			for(int  i = 0; i < pAttr.attri.nCount; i++)
			{
				calc << pAttr.attri.pAttrs[i];
			}
		}
	}
	return true;
}

// 计算狂暴属性
bool CPropertySystem::InitActorFrenzyProperty(CAttrCalc &calc)
{
	if (!m_pEntity || ((CEntity *)m_pEntity)->GetType() != enActor) 
		return false;

    CActor* pActor = (CActor*)m_pEntity;
	if (m_pEntity->GetProperty<int>(PROP_ACTOR_FRENZY) > 0)
	{
		FrenzyCfg const& cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetFrenzyCfg();
		for(int  i = 0; i < cfg.vecPropertys.size(); i++)
		{
			GAMEATTR attr;
			std::tie(attr.type,attr.value.nValue) = cfg.vecPropertys[i];
			calc << attr;
		}
	}
	return true;
}

// 计算官阶属性
bool CPropertySystem::InitActorOfficeProperty(CAttrCalc &calc)
{
	if (!m_pEntity || ((CEntity *)m_pEntity)->GetType() != enActor) 
		return false;

    CActor* pActor = (CActor*)m_pEntity;
	int nOfficeLvl = m_pEntity->GetProperty<int>(PROP_ACTOR_OFFICE);
	if (nOfficeLvl > 0)
	{
		std::vector<OfficeCfg> const& vecCfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetOfficeCfg();
		if (nOfficeLvl < vecCfg.size())
		{
			OfficeCfg const& cfg = vecCfg[nOfficeLvl];
			for(int  i = 0; i < cfg.vecPropertys.size(); i++)
			{
				GAMEATTR attr;
				std::tie(attr.type,attr.value.nValue) = cfg.vecPropertys[i];
				calc << attr;
			}
		}
	}
	return true;
}

unsigned int CPropertySystem::GetFireDefenseRate()
{
	INT_PTR nEntityType = m_pEntity->GetHandle().GetType(); 
	if (nEntityType == enActor ||nEntityType == enHero ) 
	{
		int nVocation = enVocNone;
		if (nEntityType == enActor)
		{
			nVocation = m_pEntity->GetProperty<unsigned>(PROP_ACTOR_VOCATION); //职业
		}
		else
		{
			CHero * pHero =(CHero*) m_pEntity;

			const CHeroSystem::HERODATA * pPetData = pHero->GetHeroPtr();
			if(pPetData ==NULL)
			{
				OutputMsg(rmError,_T("%s INIT error"),__FUNCTION__);
				return 0;
			}
			nVocation = pPetData->data.bVocation;
		}
		if (nVocation <= enVocNone || nVocation >= enMaxVocCount)
		{
			OutputMsg(rmError,_T("Vocation error =%d"),nVocation); //属性失败了
			return 0;
		}
		PVOCATIONINITCONFIG pVoc = &GetLogicServer()->GetDataProvider()->GetVocationConfig()->vocationTable[nVocation];
		CAnimal *pAnimal = (CAnimal *)m_pEntity;
		return pVoc->nFireDefenseRate;
	}
	else 
	{
		return 0; //其他都为0
	}
}

bool CPropertySystem::CalcPetBaseProperty(CAttrCalc &calc, const INT_PTR nMonsterId, const INT_PTR nLevel)
{
	PMONSTERCONFIG pConfig=GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
	if (pConfig == NULL) return false;
	if (m_pEntity->GetType() != enPet)
	{
		return false;
	}
	CPet* pPet = (CPet*)m_pEntity;
	//宠物属性= monster配置（攻击类不算）+ 人物攻击*百分比(攻击只继承道术攻击)
	calc << pConfig->cal;
	calc.applyPowerAttributes(pConfig->cal);
	CActor* pMaster = pPet->GetMaster();
	if (pMaster)
	{
		int nPercent = pPet->GetInheriPercent();
		if (nPercent > 0)
		{
			CAttrCalc masterCalc;
			int nBaseId = aPhysicalAttackMinAdd;
			int nBasePropertyID = PROP_CREATURE_PHYSICAL_ATTACK_MIN;
			for(INT_PTR i = nBaseId; i <= aWizardAttackMaxPower ; i++)
			{
				int nDis= (int)(i - nBaseId);
				if( nDis %2==1) continue;
				INT_PTR nPropID  = ( nDis >> 1 ) + nBasePropertyID;
				if (nPropID== PROP_CREATURE_PHYSICAL_ATTACK_MIN || nPropID==PROP_CREATURE_MAGIC_ATTACK_MIN)
				{
					//攻击只继承道术
					nPropID = PROP_CREATURE_WIZARD_ATTACK_MIN;
				}
				if (nPropID== PROP_CREATURE_PHYSICAL_ATTACK_MAX || nPropID==PROP_CREATURE_MAGIC_ATTACK_MAX)
				{
					//攻击只继承道术
					nPropID = PROP_CREATURE_WIZARD_ATTACK_MAX;
				}

				//if(cal.m_AttrValues[i].nValue ==0) continue;
				if(AttrDataTypes[i] == adFloat)
				{
					masterCalc.m_AttrValues[i].fValue = pMaster->GetProperty<float>((int)nPropID);
				}
				else if(AttrDataTypes[i] == adInt)
				{
					masterCalc.m_AttrValues[i].nValue = pMaster->GetProperty<int>((int)nPropID);

				}
				else if(AttrDataTypes[i] == adUInt)
				{
					masterCalc.m_AttrValues[i].uValue = pMaster->GetProperty<unsigned int>((int)nPropID);
				}
			}
			masterCalc *= ((float)nPercent/100);
			calc << masterCalc;
		}
		
	}
	// CAttrCalc originalCal ; //原始的属性
	// if (calcMonsterLevelHard(pConfig,originalCal,nLevel))
	// {
	// 	calc << originalCal;
	// }
	
	return true;
}

void CPropertySystem::AddPropertyValue(INT_PTR nPropertyType, int nAddValue)
{
	unsigned int nValue = m_pEntity->GetProperty<unsigned int>(nPropertyType);
	nValue = (unsigned int )(nValue*((float)(nAddValue)/100));
	m_pEntity->SetProperty<unsigned int>(nPropertyType, nValue);
}

void CPropertySystem::AddPropertyRatioValue(INT_PTR nPropertyType, int nAddValue)
{
	unsigned int nValue = m_pEntity->GetProperty<unsigned int>(nPropertyType);
	nValue = (unsigned int )(nValue*(1+nAddValue/10000.0));

	if (nAddValue < -10000.0)
	{
		nValue = 0;
	}
	
	m_pEntity->SetProperty<unsigned int>(nPropertyType, nValue);
}
