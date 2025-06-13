#include "StdAfx.h"
#include "SkillSubSystem.h"
#include "SkillCondition.h"

//处理网络消息
void  CSkillSubSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity == NULL)
	{
		return;
	}
	WORD nSkillID = 0;
	EntityHandle handle;
	WORD nPosX=0, nPosY=0;
	BYTE bDir,bFlag;
	INT_PTR nErrorCode = tpNoError;
	BYTE  bType  = 0;
	BYTE   bUseYB = 0;
	if (m_pEntity == NULL || m_pEntity->IsDeath())
	{
		m_pEntity->SetCommonOpNextTime(900, false, false);
		((CActor *)m_pEntity)->SendOperateResult(true, 0, 0, false);
		return;
	}
	switch(nCmd)
	{
	//这个留着,下发已经学习的技能列表
	case cGetSkill:
		{
			SendLearnSkill();
		}
		break;
	//近身攻击
	case cNearAttack:
		{
			packet >>  handle >> bDir;
			m_pEntity->SetTarget(handle);

			m_pEntity->SetDir(bDir & 7);  

			const float normal_attack_interval = 750;//ms 客户端是900 
			float attack_accelerate_per = 1 + (((float)m_pEntity->GetProperty<int>(PROP_ACTOR_NORMAL_ATK_ACC)) / 10000);
			int final_interval = static_cast<int>(normal_attack_interval / attack_accelerate_per);
			//近身攻击CD ，时间过短的玩家可能是用的挂
			if(!((CActor *)m_pEntity)->CheckAttackOpTick())
			{
				const float attack_interval = 900;//ms 客户端是900  
				int normal_interval = static_cast<int>(attack_interval / attack_accelerate_per);

				//使用挂 或者发包机
				OutputMsg(rmWaning,"[WG] 玩家 = %d Actorname=%s, Attack for speed check, normal:%d", ((CActor*)m_pEntity)->GetId(), m_pEntity->GetEntityName(), (int)normal_interval);

				m_pEntity->SetCommonOpNextTime(final_interval, false, false);

				((CActor *)m_pEntity)->SendOperateResult(true, 0, 0, false); 
				break;
			}

			//特殊处理：检测战士相关的技能
			CEntity * pEntity = GetEntityFromHandle(m_pEntity->GetTarget());
			if(CheckNextSkillFlag(pEntity))
			{
				nErrorCode = NearAttack(handle,0,0); //有2个参数无效，需要重新计算
			}

			// const float normal_attack_interval = 750;//ms 客户端是900 
			// float attack_accelerate_per = 1 + (((float)m_pEntity->GetProperty<int>(PROP_ACTOR_NORMAL_ATK_ACC)) / 10000);
			// int final_interval = static_cast<int>(normal_attack_interval / attack_accelerate_per);

			if(nErrorCode)
			{
				//m_pEntity->SetCommonOpNextTime((INT_PTR)(1.6* m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_ATTACK_SPEED)),false,false,0,0,true,true,false );
				m_pEntity->SetCommonOpNextTime(final_interval,false,false,0,0,true,true,false );
				((CActor *)m_pEntity)->SendOperateResult(false, 0, 0, false);		
			}
			else
			{			
				//m_pEntity->SetCommonOpNextTime((INT_PTR)(1.6* m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_ATTACK_SPEED)),false,false );
				m_pEntity->SetCommonOpNextTime(final_interval, false, false);
				((CActor *)m_pEntity)->SendOperateResult(true, 0, 0, false);
				//m_pEntity->SetCommonOpNextTime( 1.6* m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_ATTACK_SPEED),true,true );
			}
		}
		break;
	//开启技能
	case cOpenSkill :
		{
			packet >> nSkillID;
			SwitchSkill(nSkillID,1);
		}
		break;
	//关闭技能
	case cCloseSkill :
		{
			packet >> nSkillID;
			SwitchSkill(nSkillID,0);
		}
		break;
	//使用技能
	case cUseSkill :
		{
			packet >> nSkillID;
			packet >> handle >> nPosX >> nPosY >> bDir;//句柄坐标方向都是目标的

			//是否为及时技能
			CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuffBySkillId(nSkillID);
			if(pBuffConf != NULL)
			{
				nErrorCode = tpSkillConfigError;
			}
			else
			{
				nErrorCode = CheckSpellTarget(nSkillID, handle, nPosX, nPosY, bDir);//设置目标，设置方向
			}
			
			if (tpNoError == nErrorCode)
			{
				nErrorCode = LaunchSkill(nSkillID,nPosX,nPosY);
			}
			if (nErrorCode)
			{
				((CActor *)m_pEntity)->SendOperateResult(false,0,0,false);
			}
			else
			{
				((CActor *)m_pEntity)->SendOperateResult(true,0,0,false);
			}
			
		}
		break;
	//升级技能
	case cTrainSkill :
		{
			packet >> nSkillID;
			nErrorCode = LearnSkill(nSkillID, GetSkillLevel(nSkillID) + 1); //学习下一级的技能
			if(nErrorCode){
				((CActor *)m_pEntity)->SendTipmsgWithId(tmItemCantLearnSkill,tstUI);
			}
			
		}
		break;
	//同步CD
	case cGetCd:
		packet >> nSkillID;
		OnGetCd(nSkillID);
		break;
	//设置默认技能
	case cSetDefaultSkill:
		packet >> nSkillID;
		SetDefaultSkill(nSkillID);
		break;

	case cStartGather:
		{	
			packet >>  handle >> nPosX >> nPosY >> bDir;
			GatherMonster(handle, nPosX, nPosY, bDir);
			return;
		}
	case cSetSkillOpen:  //设置技能开启
		{
			packet >> nSkillID >> bFlag ; 

			HandSetSkillOpen(nSkillID,bFlag);
			return;
		}
	}	

	// if( nErrorCode != tpNoError)
	// {
	// 	if(cTrainSkill == nCmd)
	// 	{
	// 		((CActor *)m_pEntity)->SendTipmsgFormatWithId(nErrorCode, tstUI);
	// 	}
	// 	else
	// 	{
	// 		if(nErrorCode != tpSkillSpellNoTarget)
	// 		{
	// 			((CActor *)m_pEntity)->SendTipmsgFormatWithId(nErrorCode,tstUI);
	// 		}			
	// 	}
	// }
}

void CSkillSubSystem::HandStartSkillCD(INT_PTR nSkillId)
{
	PSKILLDATA pSkillData =  GetSkillInfoPtr(nSkillId);
	if(pSkillData ==NULL )return;

	TICKCOUNT nCurTick = GetGlobalLogicEngine()->getTickCount();
	TICKCOUNT nOldTick = pSkillData->tick;
	if(  nOldTick >= nCurTick )
	{
		return;
	}
}

void CSkillSubSystem::DelSkillCdTemporary(INT_PTR nSkill)
{
    if (!m_pEntity) return;
	PSKILLDATA pSkillData =  GetSkillInfoPtr(nSkill);
	if(pSkillData ==NULL )return;
	//如果本身的cd就已经转到了,就不需要发数据包给客户端处理了
	if(pSkillData->tick > GetGlobalLogicEngine()->getTickCount())
	{
		pSkillData->nCd =0;
		pSkillData->tick =0; //直接删除一个技能的CD
		if(m_pEntity->GetType() ==enActor ) //如果本身是一个玩家的话
		{	
			CActorPacket pack;
			CDataPacket & data = ((CActor*)m_pEntity)->AllocPacket(pack);
			data << (BYTE) enSkillSystemID  << (BYTE) sSendSkillCd;
			data << (WORD) nSkill  ;
			data << (int) 0;
			pack.flush();
		}
	}
}

INT_PTR CSkillSubSystem::CheckSpellTarget(INT_PTR nSkillID,EntityHandle nTarget, WORD& nDestX, WORD& nDestY, INT_PTR nDir)
{	
    if (!m_pEntity) return tpSkillConfigError;
	int nX, nY;

	//检查common操作时间
	const OneSkillData * pData = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
	if (!pData)
	{
		return tpSkillConfigError;//没有学习技能
	}
	
	//检测特殊的buff是否存在
	if(INT_PTR nErrorID =  CheckSpecialBuff(pData))
	{
		return nErrorID;
	}

	//打断正在准备的技能
	if (nTarget != 0 && nTarget != m_pEntity->GetHandle())
	{
		//检查目标是否有效
		CEntity *pTarget = GetEntityFromHandle(nTarget);
		if (!pTarget)
		{
			//如果是玩家的目标为空的话，应该是没有向客户端下发实体消失的消息，向其下发一个
			if(m_pEntity->GetType() == enActor)
			{
				m_pEntity->GetObserverSystem()->EntityDisappear(nTarget);
			}
			return tpSkillSpellNoTarget;//无效的目标
		}

		//是否处于同一场景判断
		if ( !pTarget->GetFuBen() || !m_pEntity->GetFuBen() || pTarget->GetFuBen()->GetFbId() != m_pEntity->GetFuBen()->GetFbId() 
			|| !pTarget->GetScene() || !m_pEntity->GetScene() || pTarget->GetScene()->GetSceneId() != m_pEntity->GetScene()->GetSceneId())
		{
			return tpSkillSpellNoTarget;//无效的目标
		}

		//检查目标距离
		pTarget->GetPosition(nX, nY);
		//有目标选择时，坐标重置成目标当前位置
	    nDestX = nX;
		nDestY = nY;

		if (abs(nX - nDestX) >MOVE_GRID_ROW_RADIO  || abs(nY - nDestY) > MOVE_GRID_ROW_RADIO)
		{
			return tpSkillTrainDistance;//与目标距离太远
		}
		m_pEntity->SetTarget(nTarget);
	}
	else
	{
		EntityHandle curTarget = m_pEntity->GetTarget();

		if(nTarget != curTarget)
		{
			m_pEntity->SetTarget(nTarget);
		}

		// 特殊处理（治愈术，若目标为空地，则目标设为自己）
		if (nSkillID == 22 && nTarget == 0)
		{
			m_pEntity->SetTarget(m_pEntity->GetHandle());
		}

		//是否处于同一场景判断
		CEntity *pTarget = GetEntityFromHandle(m_pEntity->GetTarget());
		if ( pTarget )
		{
			if ( !pTarget->GetFuBen() || !m_pEntity->GetFuBen() || pTarget->GetFuBen()->GetFbId() != m_pEntity->GetFuBen()->GetFbId() 
			|| !pTarget->GetScene() || !m_pEntity->GetScene() || pTarget->GetScene()->GetSceneId() != m_pEntity->GetScene()->GetSceneId() )
			{
				return tpSkillSpellNoTarget;//无效的目标
			}
		}
			
		m_pEntity->GetPosition(nX, nY);
		
		//施法位置是自己的位置，设置朝向为客户端决定的朝向
		if (nDestX == nX && nDestY == nY)
		{
			m_pEntity->SetDir(nDir & 7);
		}
		else
		{
			if ( pData->btClientDir == 0 )
			{
				//定点施法，设置朝向为施法点的方向
				if(nDestX !=0  || nDestY !=0 )
				{
					nDir = CEntity::GetDir(nX, nY, nDestX, nDestY);
				}
			}	
			
			if(nDir >=0 && nDir < DIR_STOP )
			{
				m_pEntity->SetDir(nDir);
			}			
		}
	}

	return tpNoError;
}

void  CSkillSubSystem::SetDefaultSkill(INT_PTR nSkillID)
{
    if (!m_pEntity) return;
	if(nSkillID ==0) return ;
	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_DEFAULT_SKILL, (UINT)nSkillID);
}

//近身攻击目标
INT_PTR CSkillSubSystem::NearAttack(EntityHandle targetHandle, INT_PTR nActionID ,INT_PTR nEffectID)
{	
    if (!m_pEntity) return tpSkillTrainWithBuff;
	//检测特殊的buff是否存在
	CBuffSystem *pBuff = m_pEntity->GetBuffSystem();
	//麻痹
	INT_PTR ndizzy = m_pEntity->GetProperty<int>(PROP_CREATURE_DIZZY_STATUS);
	if(pBuff && (pBuff->Exists( aDizzy ) || pBuff->Exists(aSlient)) || ndizzy) 
	{
		return tpSkillTrainWithBuff;
	}

	if (m_pEntity->GetType() == enActor)
	{
		if(! m_pEntity->CheckCommonOpTick(((CActor*)m_pEntity)->GetGateNetWorkTickCount() + 50, false) )
		{
			return tpSkillSellCDLimited;
		}
	}
	else
	{
		if(! m_pEntity->CheckCommonOpTick( GetLogicCurrTickCount() ) )
		{
			return tpSkillSellCDLimited;
		}
	}
	
	BroadSpellSkill(0,nActionID,m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR),nEffectID);

	CEntity * pEntity = GetEntityFromHandle(m_pEntity->GetTarget());
	if(pEntity ==NULL || !pEntity->isAnimal()) return tpSkillSpellNoTarget;
	
	//是否处于同一场景判断
	if ( !pEntity->GetFuBen() || !m_pEntity->GetFuBen() || pEntity->GetFuBen()->GetFbId() != m_pEntity->GetFuBen()->GetFbId() 
		|| !pEntity->GetScene() || !m_pEntity->GetScene() || pEntity->GetScene()->GetSceneId() != m_pEntity->GetScene()->GetSceneId())
	{
		return tpSkillSpellNoTarget;
	}
	
	//目标不可攻击
	GAMEATTRVALUE value;
	value.nValue = 1;
	if( !m_targetCondition.CanAttack(m_pEntity,(CAnimal*)pEntity,value ) )
	{
		//OutputMsg(rmTip, _T("不能攻击，但临时改为可攻击。"));
		// if (m_pEntity->GetType() == enActor)
		// {
		// 	((CActor *)m_pEntity)->SendTipmsgWithId(tmTarCantAttack,tstFigthing);
		// }
		return tpNoError;
	}
	
	//攻击距离限制
	if( GetAttackDistance(m_pEntity,pEntity) > 2 )
	{
		return tpSkillTrainDistance;
	}

	SKILLRESULT  result; //技能的结果
	result.nDelay = 300;
	result.nValue =0; 
	result.nId = 10000; //都是100攻击
	result.nResultType = srSkillResultPhysicAttack;
	
	SKILLRESULTPARAM param;

	param.pSrcEntity = m_pEntity;
	param.pTargetEntity = (CAnimal*)pEntity;
	param.pSkillResult =& result;
	m_skillResult.DoResult(&param); //释放肉搏技能
	
	return tpNoError;
}

//通知一个技能数据
void CSkillSubSystem::NoticeOneSkillData(PSKILLDATA pSkillData)
{
    if (!m_pEntity)
	{
		return;
	} 
	if(pSkillData == NULL)
	{
		return;
	} 


	if(((CActor*)m_pEntity)->OnGetIsSimulator() || ((CActor*)m_pEntity)->OnGetIsTestSimulator())
	{
		return;
	} 
	// 通知更新开启关闭
	CActorPacket pack;
	CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)enSkillSystemID << (BYTE)sTrainSkillResult << (WORD)(pSkillData->nSkillID) << (BYTE)(pSkillData->nLevel);
	int nLeftTick = (int)(pSkillData->tick - GetGlobalLogicEngine()->getTickCount());
	if(nLeftTick <0) nLeftTick =0;
	data << (int)nLeftTick;
	data << (BYTE)(!pSkillData->nIsClosed);
	data << (BYTE)pSkillData->bIsClose;
	pack.flush();
}

//开启/关闭战士的附加技能
void CSkillSubSystem::SwitchSkill(INT_PTR nSkillID, bool nFlag)
{
	if(m_pEntity ==NULL) return;
	if (m_pEntity->GetType() != enActor) return;

	CBuffSystem * bufSystem = ((CActor*)m_pEntity)->GetBuffSystem();
	if(!bufSystem) return;

	SKILLDATA * pSkillData = GetSkillInfoPtr(nSkillID);
	if(pSkillData == NULL) return;
	
	//判断区域能否使用技能
	if(CScene* pScene = m_pEntity->GetScene())
	{
		int x,y;
		m_pEntity->GetPosition(x,y);
		if (m_pEntity->GetType() ==enActor && pScene->HasMapAttribute(x,y,aaNotSkillAttri))
		{
			return;
		}
		if (pScene->HasMapAttribute(x,y,aaNotSkillId, (int)nSkillID))
		{
			return;
		}
	}

	//判断是否为 "战士延次技能的标志" 的技能
	CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuffBySkillId(nSkillID);
	if(pBuffConf == NULL) return;

	//技能cd时间判断
	TICKCOUNT nCurTick = GetGlobalLogicEngine()->getTickCount();
	if(nFlag && pSkillData->tick > nCurTick) 
	{
		//((CActor *)m_pEntity)->SendTipmsgWithId(tmInSkillCD, ttFlyTip);
		return;
	}

	//技能的配置
	const OneSkillData * pSkillConf =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
	if(pSkillConf ==NULL) return;
		
	//技能等级配置
	const SKILLONELEVEL * pSLevelConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID, pSkillData->nLevel);
	if( pSLevelConf == NULL) return;

	//对于可控开启关闭的技能，检查是否已开启关闭.
	if(!pSkillConf->bIsSwitch)  return;
	if (pSkillData->nIsClosed != (BYTE) nFlag) //重复操作了，检查一次buff与技能开关标志是否同步
	{
		if (pSkillData->nIsClosed && !bufSystem->Exists(pBuffConf->nId))
		{
			return;
		}
		else if (!pSkillData->nIsClosed && bufSystem->Exists(pBuffConf->nId))
		{
			return;
		}
	}

	//技能使用的条件的判断
	const DataList<SKILLTRAINSPELLCONDITION> & spellConditions = pSLevelConf->spellConditions ;
	INT_PTR nConditionCount = spellConditions.count;
	for (INT_PTR i =0; i< nConditionCount ; i++ )
	{
		INT_PTR nUseErrorCode = m_targetCondition.CheckSpellContion(m_pEntity,nSkillID,spellConditions[i],false);
		if(nUseErrorCode != tpNoError)
		{
			return;
		}
	}

	//设置技能cd时间
	int nSelfCd = pSkillData->nCoolDownTime; //自己的冷却时间
	if(GetEnableCD() && nSelfCd >0)
	{
		TICKCOUNT nCurTick = GetGlobalLogicEngine()->getTickCount();
		pSkillData->tick = nCurTick + nSelfCd; //设置自己的cd
		OnGetCd(nSkillID);
	}

	int nTipMsg = 0;
	switch (nSkillID)
	{
	case 3:
		nTipMsg = nFlag?tmOpenSkill_3:tmCloseSkill_3;
		break;
	case 4:
		nTipMsg = nFlag?tmOpenSkill_4:tmCloseSkill_4;
		break;
	case 6:
		nTipMsg = tmPrepSkill_6;
		break;
	case 8:
		nTipMsg = tmPrepSkill_8;
		break;
	}

	// 增加/删除buff 及 技能开关标志
	if(nFlag) {
		//OutputMsg(rmTip,_T("\n【%s】开启技能：SwitchSkill %d !!"), m_pEntity->GetEntityName(), 
		//	nSkillID
		//);
		pSkillData->nIsClosed = 0;
		if (!bufSystem->Exists(pBuffConf->nId))
		{
			bufSystem->Append(pBuffConf->nId);
		}
	} else {
		//OutputMsg(rmTip,_T("\n【%s】关闭技能：SwitchSkill %d !!"), m_pEntity->GetEntityName(), 
		//	nSkillID
		//);
		pSkillData->nIsClosed = 1;
		bufSystem->RemoveById(pBuffConf->nId,false);
	}

	//存储记录并发提示
	SetDataModifyFlag(true);
	((CActor *)m_pEntity)->SendTipmsgWithId(nTipMsg, tstFigthing);

	//更新下一击附加技能标记
	UpdateNextSkillFlag();

	//通知一个技能数据
	NoticeOneSkillData(pSkillData);
}

void CSkillSubSystem::InitNextSkillFlag()
{
    if (!m_pEntity) return;
	const OneSkillData * pSkillConf;
	CSkillProvider* provider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	CBuffSystem * bufSystem = ((CActor*)m_pEntity)->GetBuffSystem();
	for(INT_PTR i=0;i < m_skillList.count(); i++)
	{
		pSkillConf = provider->GetSkillData(m_skillList[i].nSkillID);
		if(pSkillConf)
		{
			//判断是否为 "战士延次技能的标志" 的技能
			CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuffBySkillId(m_skillList[i].nSkillID);
			if(pBuffConf)
			{
				if (m_skillList[i].bIsPassive)
				{
					if (!bufSystem->Exists(pBuffConf->nId))
					{
						bufSystem->Append(pBuffConf->nId);
					}
				}
				else if (pSkillConf->bIsSwitch)
				{
					if (m_skillList[i].nIsClosed && bufSystem->Exists(pBuffConf->nId))
					{
						bufSystem->RemoveById(pBuffConf->nId,false);
					} 
					else if (!m_skillList[i].nIsClosed && !bufSystem->Exists(pBuffConf->nId))
					{
						bufSystem->Append(pBuffConf->nId);
					}
				}
			}
		}
	}
}

//检测战士的下一击附加技能，并执行，并更新下一击附加技能标记
INT_PTR CSkillSubSystem::CheckNextSkillFlag(CEntity* pTargetEntity)
{
	INT_PTR nErrorCode = tpSkillCannotSpell;
	if(m_pEntity ==NULL ) return tpSkillCannotSpell;
	if (m_pEntity->GetType() != enActor) return tpSkillCannotSpell;
	CBuffSystem * bufSystem = ((CActor*)m_pEntity)->GetBuffSystem();
	if(bufSystem == NULL) return tpSkillCannotSpell;
	
	//特殊处理：攻杀，刺杀，半月，烈火，逐日
	union
	{
		struct
		{
			unsigned short nSkillId;
			unsigned short nSkillLevel;
		}sub;
		int Flag;
	} nNextSkillFlag;
	
	nNextSkillFlag.Flag = (INT_PTR)(m_pEntity->GetProperty<int>(PROP_ACTOR_NEXT_SKILL_FLAG));
	if (nNextSkillFlag.Flag != 0)
	{
		//玩家附加技能数据触发
		int nSkillID = nNextSkillFlag.sub.nSkillId;
		PSKILLDATA pNextSkillData = GetSkillInfoPtr(nSkillID);
		if(pNextSkillData == NULL) return tpSkillCannotSpell;
		if (pTargetEntity && nNextSkillFlag.Flag)
		{
			//是否处于同一场景判断
			if ( !pTargetEntity->GetFuBen() || !m_pEntity->GetFuBen() || pTargetEntity->GetFuBen()->GetFbId() != m_pEntity->GetFuBen()->GetFbId() 
				|| !pTargetEntity->GetScene() || !m_pEntity->GetScene() || pTargetEntity->GetScene()->GetSceneId() != m_pEntity->GetScene()->GetSceneId())
			{
				return tpSkillCannotSpell;
			}

			CScene* pScene = m_pEntity->GetScene();
			int nCurPosX,nCurPosY;
			m_pEntity->GetPosition(nCurPosX,nCurPosY);
			INT_PTR nDir = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);

			//技能的配置
			const OneSkillData * pNextSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nNextSkillFlag.sub.nSkillId);
			if(pNextSkillConf ==NULL) return tpSkillCannotSpell;
				
			//技能等级配置
			const SKILLONELEVEL * pNextSLevelConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nNextSkillFlag.sub.nSkillId, pNextSkillData->nLevel);
			if( pNextSLevelConf == NULL) return tpSkillCannotSpell;

			//触发消耗（先前已判断过是否足够消耗）
			const DataList<SKILLTRAINSPELLCONDITION> & spellConditions = pNextSLevelConf->spellConditions ;
			INT_PTR nConditionCount = spellConditions.count;
			for (INT_PTR i =0; i< nConditionCount; i++ )
			{
				if(spellConditions[i].bConsumed == false || spellConditions[i].nValue ==0) continue; //如果不需要消耗就不消耗
				m_targetCondition.CheckSpellContion( m_pEntity,nSkillID,spellConditions[i],true ); //去消耗
			}

			//遍历范围效果列表
			for(INT_PTR rangeID=0; rangeID < pNextSLevelConf->pranges.count; rangeID++)
			{
				PSKILLONERANGE pRangeConf = pNextSLevelConf->pranges[rangeID];
				if(pRangeConf ==NULL) continue;

				if (pRangeConf->nDelay > 0)
				{
					CEntityMsg msg(CEntityMsg::emSkillApplyOneRange,m_pEntity->GetHandle());
					msg.dwDelay = pRangeConf->nDelay;
					msg.nParam1 = nNextSkillFlag.sub.nSkillId;
					msg.pParam2 = pNextSkillData;
					msg.pParam3 = pRangeConf;
					msg.pParam4 = pTargetEntity;
					msg.pParam5 = pScene;
					msg.nParam6 = nCurPosX;
					msg.nParam7 = nCurPosY;
					msg.nParam8 = 0;
					msg.nParam9 = 0;
					msg.nParam10 = nDir;
					m_pEntity->PostEntityMsg(msg);
				}
				else
				{
					INT_PTR nError = SkillApplyOneRange((int)nNextSkillFlag.sub.nSkillId,pNextSkillData,pRangeConf,pTargetEntity,pScene,nCurPosX,nCurPosY,0,0,nDir);
					if (nError > tpNoError)
					{
						return nError;
					}
				}
			}
		}

		// 使用之后要删除buff（无限时buff不删）
		CBuffProvider::BUFFCONFIG* buffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuffBySkillId(nNextSkillFlag.sub.nSkillId);
		if (buffConf && buffConf->nTimes != -1)
		{
			pNextSkillData->nIsClosed = 1;
			SetDataModifyFlag(true);
			bufSystem->RemoveById(buffConf->nId,false);
		}

		nErrorCode = tpNoError;

		//通知一个技能数据
		NoticeOneSkillData(pNextSkillData);

		//播放攻击
		BroadSpellSkill(0,0,m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR),0);
	}

	// 更新下一击附加技能标记
	UpdateNextSkillFlag();

	return nErrorCode;
}

//更新下一击附加技能标记
void CSkillSubSystem::UpdateNextSkillFlag()
{
    if (!m_pEntity) return;
	CBuffSystem * pBufSystem = ((CActor*)m_pEntity)->GetBuffSystem();
	if (!pBufSystem) return;

	//特殊处理：攻杀，刺杀，半月，烈火，逐日
	union
	{
		struct
		{
			unsigned short nSkillId;
			unsigned short nSkillLevel;
		}sub;
		int Flag;
	} nNextSkillFlag;
	nNextSkillFlag.Flag = 0;

	// 战士技能附加技能标志更新
	int priority = 0;
	CVector<CDynamicBuff>& AllBuff =  pBufSystem->GetAllBuff();
	for (size_t i = 0; i < AllBuff.count(); i++)
	{
		CDynamicBuff& buff = AllBuff[i];
		int skillid = (int)(buff.pConfig->dValue);
		int nRate10000 = 0;

		// 获取触发概率万分比
		CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuffBySkillId(skillid);
		SKILLONELEVEL *pLevelSkill = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(skillid,GetSkillLevel(skillid));
		if (pBuffConf && pLevelSkill)
		{
			// 获取概率
			for(INT_PTR rangeID=0; rangeID < pLevelSkill->pranges.count; rangeID++)
			{
				PSKILLONERANGE pRange = pLevelSkill->pranges[rangeID];
				INT_PTR nResultCount = pRange->skillResults.count;
				for(INT_PTR resultID =0; resultID < nResultCount; resultID++)
				{
					PSKILLRESULT pResult = &(pRange->skillResults[resultID]);
					if(pResult->nResultType != srSkillResultPhysicAttack ) continue;
					nRate10000 = pResult->nParam2;
				}
				if (nRate10000) break;
			}

			// 触发
			if(buff.pConfig->nType == aNextSkillFlag && (nRate10000?nRate10000:10000) > wrand(10000))
			{
				
				PSKILLDATA pSkill = GetSkillInfoPtr(skillid);

				if (pSkill && priority < pSkill->bPriority)
				{
					nNextSkillFlag.sub.nSkillId = skillid;
					nNextSkillFlag.sub.nSkillLevel = pSkill->nLevel;
					priority = pSkill->bPriority;
				}
			}
		}
	}
	m_pEntity->SetProperty<int>(PROP_ACTOR_NEXT_SKILL_FLAG, nNextSkillFlag.Flag);
	m_pEntity->GetObserverSystem()->UpdateActorEntityProp();
#ifdef _DEBUG_GATEMSG
	OutputMsg(rmTip,_T("\n【%s】检查更新技能标志：UpdateNextSkillFlag (%d , %d)"), m_pEntity->GetEntityName(), 
		nNextSkillFlag.sub.nSkillId, nNextSkillFlag.sub.nSkillLevel
	);
#endif
}

//技能标记的BUff过期回调
INT_PTR CSkillSubSystem::OnBuffExpire(CDynamicBuff *pBuff)
{
	//玩家技能数据
	int nSkillId = (int)(pBuff->pConfig->dValue);
	PSKILLDATA pSkillData = GetSkillInfoPtr(nSkillId);
	if(pSkillData == NULL) return tpNoError;

	// 关闭技能
	pSkillData->nIsClosed = 1;
	SetDataModifyFlag(true);

	// 更新下一击附加技能标记
	UpdateNextSkillFlag();

	// 更新数据信息
	NoticeOneSkillData(pSkillData);
}

//发射技能
INT_PTR CSkillSubSystem::LaunchSkill(INT_PTR nSkillID,INT_PTR nPosX, INT_PTR nPosY, bool bNoCd)
{	
	if(m_pEntity ==NULL ) return tpSkillCannotSpell;
	
	//场景
	CScene *pScene = m_pEntity->GetScene();
	if(pScene == NULL) //出现了比较严重的错误
	{
		OutputMsg(rmWaning,_T("释放技能失败，场景为空,技能ID=%d"),nSkillID);
		return tpNoError; 
	}

	//SKILLDATA skillData;
	
	//判断是否为 "战士延次技能的标志" 的技能
	CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuffBySkillId(nSkillID);
	if(pBuffConf != NULL) 
	{
		// TODO 提示
		OutputMsg(rmWaning,_T("技能使用错误！"));
		return tpNoError;
	}

	//玩家技能数据
	PSKILLDATA pSkillData = GetSkillInfoPtr(nSkillID);
	if(pSkillData == NULL) return tpSkillSpellNotLeared;	//技能不存在
	if(pSkillData->bIsClose) return tpSkillIsClosed; 		//该技能已经关闭了

	//技能的配置
	const OneSkillData * pSkillConf =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
	if(pSkillConf ==NULL) return tpSkillConfigError;
	
	//检测特殊的buff是否存在
	CBuffSystem *pBuff = m_pEntity->GetBuffSystem();
	//麻痹
	if(pSkillConf->nSkillType != stTransferSkill) {
		
		INT_PTR ndizzy = m_pEntity->GetProperty<int>(PROP_CREATURE_DIZZY_STATUS);
		if(pBuff && (pBuff->Exists( aDizzy ) || pBuff->Exists(aSlient)) || ndizzy) 
		{
			OutputMsg(rmWaning,_T("麻痹或沉默中，无法使用技能！"));
			return tpSkillTrainWithBuff;
		}
	}
	

	//技能等级配置
	const SKILLONELEVEL * pSLevelConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID, pSkillData->nLevel);
	if( pSLevelConf == NULL)
	{
		OutputMsg(rmWaning,_T("技能配置错误！"));
		return tpSkillSpellNotLeared;
	}

	//判断区域能否使用技能
	if(CScene* pScene = m_pEntity->GetScene())
	{
		int x,y;
		m_pEntity->GetPosition(x,y);
		if (m_pEntity->GetType() == enActor && pScene->HasMapAttribute(x,y,aaNotSkillAttri))
		{
			OutputMsg(rmWaning,_T("该区域禁止使用任何技能！"));
			return tpSkillCannotSpell;
		}
		if (pScene->HasMapAttribute(x,y,aaNotSkillId, (int)nSkillID))
		{
			OutputMsg(rmWaning,_T("该区域禁止使用该技能技能！"));
			return tpSkillCannotSpell;
		}
	}

	//对于玩家/怪物/宠物的相关限制
	INT_PTR nType = m_pEntity->GetType();
	if(nType == enMonster)
	{
		if(pSkillConf->bSkillClass != scSkillClassMonster )
		{
			return tpSkillCannotSpell;
		}
	}
	else if(nType == enActor )
	{
		if ( pSkillConf->bSkillClass == scSkillClassMonster )
		{
			return tpSkillTrainCoinLimited;
		}

		//被动技能和生活技能无法释放
		if(pSkillConf->nSkillType == stPassiveSkill || pSkillConf->nSkillType == stLifeSkill)
		{
			return tpSkillCannotSpell;
		}
		else if(pSkillConf->nSkillType != stOtherSpecialSkill && pSkillConf->bSkillClass != scSkillHideAttach && pSkillConf->nSkillType != stTransferSkill) //特殊技能不检查cd的
		{
			TICKCOUNT commTickCheck = ((CActor*)m_pEntity)->GetGateNetWorkTickCount() +50;
			// 这里打个补丁
			((CActor*)m_pEntity)->CheckCommonOpTickBefore(commTickCheck);
			////因存在移动后进入使用技能时会出现公共CD没到而技能使用不出来的情况，所以这里改为允许30毫秒的误差
			if (!bNoCd && !((CActor*)m_pEntity)->CheckCommonOpTick(commTickCheck,false))
			{
				return tpSkillSellCDLimited; //攻击速度
			}
		}
	}
	else if(nType == enPet)
	{
		if(pSkillConf->nSkillType == stPassiveSkill )
		{
			return tpSkillCannotSpell;
		}
	}

	//技能冷却判断
	TICKCOUNT nCurTick = GetGlobalLogicEngine()->getTickCount();
	if(!bNoCd && pSkillData->tick  > nCurTick ) 
	{
		return tpSkillSellCDLimited;
	}

	//技能使用的条件的判断
	const DataList<SKILLTRAINSPELLCONDITION> & spellConditions = pSLevelConf->spellConditions ;
	INT_PTR nConditionCount= spellConditions.count;
	for (INT_PTR i =0; i< nConditionCount ; i++ )
	{
		INT_PTR nUseErrorCode = m_targetCondition.CheckSpellContion(m_pEntity,nSkillID,spellConditions[i],false);
		if(nUseErrorCode != tpNoError)
		{
			return nUseErrorCode;
		}
	}

	//广播开始释放技能
	BroadSpellSkill(nSkillID, pSkillData->nLevel, m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR), m_pEntity->GetTarget(), nPosX, nPosY);

	//下面进行技能的目标筛选
	INT_PTR nSkillType = pSkillConf->nSkillType;  //技能的类型
	CEntity * pTargetEntity = GetEntityFromHandle(m_pEntity->GetTarget()); //目标实体指针
	
	int nCurPosX,nCurPosY;
	m_pEntity->GetPosition(nCurPosX,nCurPosY); //当前的坐标
	INT_PTR nDir = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);

	//广播施法,特殊的技能是不能广播的，比如队伍的辐射光环buff
	if(nSkillType != stOtherSpecialSkill)
	{
		//特殊的技能不影响
		if(m_pEntity->GetType() == enActor)
		{				
			((CActor*)m_pEntity)->SetCommonOpNextTime( m_pEntity->GetProperty<unsigned int> (PROP_CREATURE_ATTACK_SPEED),true,true);
		}
	}

	//发射技能（或延时）
	if (pSLevelConf->nSingTime > 0)
	{
		CEntityMsg msg(CEntityMsg::emRealLaunchSkill,m_pEntity->GetHandle());
		msg.dwDelay = pSLevelConf->nSingTime;
		msg.nParam1 = nSkillID;
		msg.pParam2 = pScene;
		msg.nParam3 = nDir;
		msg.nParam4 = nCurPosX;
		msg.nParam5 = nCurPosY;
		msg.pParam6 = pTargetEntity;
		msg.nParam7 = nPosX;
		msg.nParam8 = nPosY;
		m_pEntity->PostEntityMsg(msg);
	}
	else
	{
		INT_PTR nError = RealLaunchSkill(nSkillID, pScene, nDir, nCurPosX, nCurPosY, pTargetEntity, nPosX, nPosY);
		if (nError > tpNoError) return nError;
	}
	
	//触发消耗（先前已判断过是否足够消耗）
	for (INT_PTR i =0; i< nConditionCount; i++ )
	{
		if(spellConditions[i].bConsumed == false || spellConditions[i].nValue ==0) continue; //如果不需要消耗就不消耗
		m_targetCondition.CheckSpellContion( m_pEntity,nSkillID,spellConditions[i],true ); //去消耗
	}

	//技能cd时间
	int nSelfCd = pSkillData->nCoolDownTime; //自己的冷却时间
	if(/*nType == enActor && */GetEnableCD() && nSelfCd >0)
	{
		TICKCOUNT nCurTick = GetGlobalLogicEngine()->getTickCount();
		pSkillData->tick = nCurTick + nSelfCd; //设置自己的cd
		if (nType == enActor)
		{
			OnGetCd(nSkillID);
		}
	}

	//触发了公共的cd
	int nCommonCd =  pSkillConf->nCommonCdTime ;
	if(nCommonCd >0 && m_pEntity->GetType()!=enHero)
	{
		ULONGLONG nextTick = nCommonCd + GetGlobalLogicEngine()->getTickCount(); //获取下次公告cd的时间
		for (INT_PTR i=0; i < m_skillList.count(); i++ )
		{
			if(((ULONGLONG)m_skillList[i].tick < nextTick) && m_skillList[i].nSkillType != stTransferSkill)
			{
				m_skillList[i].tick = nextTick; 
			}
		}
	}
	return tpNoError;
}

//实际发射技能
INT_PTR  CSkillSubSystem::RealLaunchSkill(INT_PTR nSkillID,
	CScene* pScene, INT_PTR nDir, INT_PTR nCurPosX, INT_PTR nCurPosY,
	CEntity* pTargetEntity, INT_PTR nMousesPosX, INT_PTR nMousesPosY)
{
	//玩家技能数据
	PSKILLDATA pSkillData = GetSkillInfoPtr(nSkillID);
	if(pSkillData == NULL) return tpSkillSpellNotLeared;

	//技能的配置
	const OneSkillData * pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
	if(pSkillConf ==NULL) return tpSkillConfigError;
		
	//技能等级配置
	const SKILLONELEVEL * pSLevelConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID, pSkillData->nLevel);
	if( pSLevelConf == NULL) return tpSkillSpellNotLeared;

	//场景特效（一般是群体技能）
	if (pSLevelConf->isSceneEffect != 0)
	{
		CEntityMsg msg(CEntityMsg::emShowSceneEffect);
		msg.nSender = m_pEntity->GetHandle();
		msg.nParam1 = pSLevelConf->nHitId;
		msg.nParam2 = nMousesPosX;
		msg.nParam3 = nMousesPosY;
		m_pEntity->PostEntityMsg(msg); //施法者自己发送消息，稍候广播
	}
	else if(!pTargetEntity && pSkillConf->bSingleEmptyEffect)//单体无目标，则空地发射
	{
		CEntityMsg msg(CEntityMsg::emShowSceneEffect);
		msg.nSender = m_pEntity->GetHandle();
		msg.nParam1 = pSLevelConf->nHitId;
		msg.nParam2 = nMousesPosX;
		msg.nParam3 = nMousesPosY;
		m_pEntity->PostEntityMsg(msg);
	}
	
	//遍历范围效果列表
	for(INT_PTR rangeID=0; rangeID < pSLevelConf->pranges.count; rangeID++)
	{
		PSKILLONERANGE pRangeConf = pSLevelConf->pranges[rangeID];
		if(pRangeConf ==NULL) continue;

		if (pRangeConf->nDelay > 0)
		{
			CEntityMsg msg(CEntityMsg::emSkillApplyOneRange,m_pEntity->GetHandle());
			msg.dwDelay = pRangeConf->nDelay;
			msg.nParam1 = nSkillID;
			msg.pParam2 = pSkillData;
			msg.pParam3 = pRangeConf;
			msg.pParam4 = pTargetEntity;
			msg.pParam5 = pScene;
			msg.nParam6 = nCurPosX;
			msg.nParam7 = nCurPosY;
			msg.nParam8 = nMousesPosX;
			msg.nParam9 = nMousesPosY;
			msg.nParam10 = nDir;
			m_pEntity->PostEntityMsg(msg);
		}
		else
		{
			INT_PTR nError = SkillApplyOneRange((int)nSkillID,pSkillData,pRangeConf,pTargetEntity,pScene,nCurPosX,nCurPosY,(int)nMousesPosX,(int)nMousesPosY,nDir);
			if (nError > tpNoError)
			{
				return nError;
			}
		}
	}

	return tpNoError;
}

INT_PTR CSkillSubSystem::SkillApplyOneRange(int nSkillID, PSKILLDATA pSkillData,PSKILLONERANGE pRange,
	CEntity* pTargetEntity,CScene* pScene,int nCurPosX,int nCurPosY,int nMousesPosX, int nMousesPosY, int nSpellDir)
{
	//设置中心点，及朝向
	INT_PTR nCenterPosX = 0; 						//技能中心点的X的位置
	INT_PTR nCenterPosY = 0; 						//技能中心点的Y的位置
	INT_PTR nDir = DIR_STOP;  						//获取本身的朝向
	switch(pRange->rangeCenter)
	{
		//以主角为中心
		case stSkillActorCenter:
		{
			nCenterPosX = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSX);
			nCenterPosY = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);
			nDir = m_pEntity->GetProperty< int>(PROP_ENTITY_DIR);
			break;
		}
		//以目标为中心
		case stSkillTargetCenter:
		{
			if(pTargetEntity)
			{
				nCenterPosX = pTargetEntity->GetProperty<unsigned int>(PROP_ENTITY_POSX);
				nCenterPosY = pTargetEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);
				nDir = pTargetEntity->GetProperty<int>(PROP_ENTITY_DIR);
			}
			else
			{
				return tpSkillSpellNoTarget;
			}
			break;
		}
		//以施法点为中心
		case stSkillSpellPointCenter:
		{
			if(nMousesPosX >0 && nMousesPosY > 0)
			{
				nCenterPosX =  (int)nMousesPosX;
				nCenterPosY =  (int)nMousesPosY;
				nDir = DIR_LEFT; //随便搞个方向，这个没意义
			}
			else
			{
				return tpSkillSpellNoTarget;
			}
			break;
		}
		//以主角瞬时坐标方向为中心
		case stSkillActorCenterDir:
		{
			nCenterPosX = nCurPosX;
			nCenterPosY = nCurPosY;
			nDir = nSpellDir;
			break;
		}
	}

	//攻击实例列表
	EntityVector& entityList = *(CFuBenManager::m_pVisibleList);
	entityList.clear(); 

	//技能方位调整及落实技能效果
	INT_PTR nRangeType = pRange->rangeType;
	if(stSkillRangeSingle == nRangeType)
	{	//单体目标不用筛选
		if(pTargetEntity)
		{
			entityList.add(m_pEntity->GetTarget());
		}else
		{
			entityList.add(m_pEntity->GetHandle());
		}
		SkillApplyOneGrid(pRange,entityList,nCenterPosX,nCenterPosY,nSkillID,pSkillData,nCurPosX,nCurPosY);
	}
	else if(stSkillRangeNoTarget == nRangeType)
	{
		INT_PTR nNewPosX , nNewPosY;
		for(INT_PTR	nRelPosX = pRange->nStartX; nRelPosX <= pRange->nEndX; nRelPosX ++ )
		{
			SKILLRESULTPARAM param;
			param.nSkillID = nSkillID;
			param.pSrcEntity = m_pEntity;
			for(INT_PTR	nRelPosY = pRange->nStartY; nRelPosY <= pRange->nEndY; nRelPosY ++ )
			{	
				nNewPosX  = nCenterPosX + nRelPosX;
				nNewPosY  = nCenterPosY + nRelPosY; 
				param.nTargetPosX = (WORD)nNewPosX;
				param.nTargetPosY = (WORD)nNewPosY;
				
				//直接技能的结果
				INT_PTR nResultCount= pRange->skillResults.count;
				for(INT_PTR resultID = 0;  resultID < nResultCount; resultID ++)
				{
					param.pSkillResult = &(pRange->skillResults[resultID]);
					m_skillResult.DoResult(&param);
				}
			}
		}

	}
	else
	{	//群体目标一一筛选
		INT_PTR nNewPosX , nNewPosY;
		for(INT_PTR	nRelPosX = pRange->nStartX; nRelPosX <= pRange->nEndX; nRelPosX ++ )
		{
			for(INT_PTR	nRelPosY = pRange->nStartY; nRelPosY <= pRange->nEndY; nRelPosY ++ )
			{
				entityList.clear();

				if(stSkillRangeRotationSingle == nRangeType ||  stSkillRangeRotationGroup == nRangeType)
				{
					PositionRotation(0,0,nRelPosX,nRelPosY,nDir,nNewPosX,nNewPosY);
					nNewPosX += nCenterPosX;
					nNewPosY += nCenterPosY;
					pScene->GetVisibleList((int)nNewPosX,(int)nNewPosY,entityList,0,0,0,0);
					SkillApplyOneGrid(pRange,entityList,nNewPosX,nNewPosY,nSkillID,pSkillData,nCurPosX,nCurPosY);
				}
				else if(stSkillRangeAccurateRotation == nRangeType)
				{
					//以中心点与鼠标的位置的连线作为旋转轴旋转，比线性旋转更精确，用于法师的火球术
					//int nNewRotatePosX=0,  nNewRotatePosY =0; //新的旋转相对中心点

					INT_PTR nMPosRelX = nMousesPosX- nCenterPosX;
					INT_PTR nMPosRelY = nMousesPosY- nCenterPosY;
					AccurateRotation(nMPosRelX,nMPosRelY,nRelPosX,nRelPosY,nNewPosX,nNewPosY);
					nNewPosX += nCenterPosX;
					nNewPosY += nCenterPosY;
					if(nNewPosX <0) nNewPosX =0;
					if(nNewPosY <0) nNewPosY =0;
					pScene->GetVisibleList((int)nNewPosX,(int)nNewPosY,entityList,0,0,0,0);
					SkillApplyOneGrid(pRange,entityList,nNewPosX,nNewPosY,nSkillID,pSkillData,nCurPosX,nCurPosY);
				}
				else if(stSkillRangePointCenter == nRangeType)
				{
					nNewPosX  = nCenterPosX + nRelPosX;
					nNewPosY  = nCenterPosY + nRelPosY;
					pScene->GetVisibleList((int)nNewPosX,(int)nNewPosY,entityList,0,0,0,0);
					//if (entityList.count() > 0)
					SkillApplyOneGrid(pRange,entityList,nCenterPosX,nCenterPosY,nSkillID,pSkillData,nCurPosX,nCurPosY); 
				}
				else
				{
					nNewPosX  = nCenterPosX + nRelPosX;
					nNewPosY  = nCenterPosY + nRelPosY; 
					pScene->GetVisibleList((int)nNewPosX,(int)nNewPosY,entityList,0,0,0,0);
					SkillApplyOneGrid(pRange,entityList,nNewPosX,nNewPosY,nSkillID,pSkillData,nCurPosX,nCurPosY);
				}
			}
		}
	}
	return tpNoError;
}

void CSkillSubSystem::SkillApplyOneGrid(PSKILLONERANGE pRange, EntityVector& entityList,
	INT_PTR nPosX,INT_PTR nPosY,INT_PTR nSkillID,PSKILLDATA pSkillData,int nCurPosX,int nCurPosY)  
{
	if(m_pEntity == NULL) return;

	SKILLRESULTPARAM param;
	param.nSkillID = nSkillID;
	param.nTargetPosX = (WORD)nPosX;
	param.nTargetPosY = (WORD)nPosY;
	param.pSrcEntity = m_pEntity;

	//技能的配置
	const OneSkillData * pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
	if(pSkillConf ==NULL) return;
		
	//技能等级配置
	const SKILLONELEVEL * pSLevelConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID, pSkillData->nLevel);
	if( pSLevelConf == NULL) return;

	//遍历这些实体
	INT_PTR nEntityCount = entityList.count();
	for(INT_PTR nEnityIndex = 0; nEnityIndex < nEntityCount;)
	{
		CEntity * pEntity = GetEntityFromHandle(entityList[nEnityIndex++]) ;
		if(pEntity == NULL ) 
		{
			continue; //实体已经删除
		} 
		
		if(pEntity->GetType() == enNpc || pEntity->GetType() == enDropItem || pEntity->GetType() == enFire || pEntity->GetType() == enTransfer)
		{
			continue;
		}
		//怪物不能被攻击的情况
		// if(m_pEntity->GetType() == enActor && pEntity->GetType() == enMonster && m_pEntity->CanAttack((CAnimal*)pEntity) == false)	
		// {
		// 	if (stSkillRangeRotationSingle == pRange->rangeType && m_pEntity->GetType() == enActor)
		// 	{
		// 		((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmTarCantAttack, tstFigthing);
		// 	}
		// 	continue;
		// }

		//目标死亡状态的处理
		if (((CAnimal *)pEntity)->HasState(esStateDeath) && pEntity != m_pEntity ) // fix bug: 解决怪物打不死的问题。后面考虑增加一个SkillCondition
		{
			// if (stSkillRangeRotationSingle == pRange->rangeType && m_pEntity->GetType() == enActor)
			// {
			// 	((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmTarCantAttack, tstFigthing);
			// }
			continue;
		}	

		//目标条件筛选
		if(INT_PTR nCondCount = pRange->targetConditions.count)
		{
			bool flag = false;
			for(INT_PTR nCondIndex = 0; nCondIndex < nCondCount; nCondIndex ++)
			{
				TARGETSELCONDITION &condition = pRange->targetConditions[nCondIndex];
				INT_PTR nEntityType = pEntity->GetType();
				if( enFire == nEntityType || enNpc == nEntityType || enDropItem == nEntityType || enTransfer == nEntityType
					|| !m_targetCondition.Check(m_pEntity,(CAnimal *)pEntity,condition) )
				{
							
					// if (stSkillRangeRotationSingle == pRange->rangeType && m_pEntity->GetType() == enActor && condition.nKey != 15) //策划要求野蛮冲撞 目标等级大于自身等级 无tip
					// {
					// 	((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmTarCantAttack, tstFigthing);
					// }
					
					flag = true;
					break;
				}
			}
			if (flag) continue;
		}

		//单体特效
		if ((m_pEntity != pEntity) && pSLevelConf->isSceneEffect == 0)
		{
			if (pEntity && pSLevelConf->nHitId)
			{
				CEntityMsg msg(CEntityMsg::emShowEntityEffect, m_pEntity->GetHandle());
				msg.nParam1 = pSLevelConf->nHitId;
				pEntity->PostEntityMsg(msg);
			}
		}
		// 对方在安全区，受限制
		int x = 0, y = 0;
		CScene* pScene = pEntity->GetScene();
		pEntity->GetPosition(x,y);
		if ((pScene && pScene->HasMapAttribute(x,y,aaSaft)) && m_pEntity->GetType() == enPet) continue;
		// {
			// ((CActor*)m_pEntity)->SendTipmsg(_T("对方在安全区中，不能攻击"),tstFigthing);
			// return false;
			//这里是技能的结果 
		// } 

		INT_PTR nResultCount= pRange->skillResults.count;
		param.pTargetEntity = (CAnimal*)pEntity;
		for(INT_PTR resultID = 0;  resultID < nResultCount; resultID ++)
		{
			param.pSkillResult = &(pRange->skillResults[resultID]);
			m_skillResult.DoResult(&param);
		}
		

		//单体技能只能攻击1人
		if (stSkillRangeRotationSingle == pRange->rangeType || stSkillRangeSingle == pRange->rangeType)
			break;
	}
}

//检测技能在特殊的buff下能够使用
INT_PTR CSkillSubSystem::CheckSpecialBuff(const OneSkillData * pSkill)
{
	if(pSkill)
	{
		if(pSkill->bSpecialBuffCond == stSkillSpecialBuffAll) //在任何的封断都能够使用
		{
			return tpNoError;
		}
	}
	else
	{
		return tpNoError; //普通攻击不受这个影响
	}
	CBuffSystem *pBuff = m_pEntity->GetBuffSystem();
	if(pBuff ==NULL) return tpNoError;
	//麻痹
	INT_PTR ndizzy = m_pEntity->GetProperty<int>(PROP_CREATURE_DIZZY_STATUS);
	if( pBuff->Exists( aDizzy ) || pBuff->Exists(aSlient) || ndizzy) 
	{
		if(pSkill->bSpecialBuffCond == stSkillSpecialBuffDizzy) //在晕眩的时候可以用
		{
			return tpNoError;
		}
		else
		{
			return tpSkillTrainWithBuff;
		}
	}
	/*
	if( pBuff->Exists( GAMEATTRTYPE(aInnnerSkillForbid) ) )//被封了
	{
		if(pSkill->nSkillType == stInnerSingleAttackSkill ||  pSkill->nSkillType == stInnerGroupAttackSkill) //如果被封断了
		{
			if(pSkill->bSpecialBuffCond == stSkillSpecialBuffFengDuan) //在封的时候可以用
			{
				return tpNoError;
			}
			else
			{
				return tpSkillTrainWithBuff;
			}
		}
	}
	

	if( pBuff->Exists( GAMEATTRTYPE(aOutSkillForbid) ) )
	{
		if(pSkill->nSkillType == stOutSingleAttackSkill ||  pSkill->nSkillType == stOutGroupAttackSkill) //如果被封断了
		{
			if(pSkill->bSpecialBuffCond == stSkillSpecialBuffFengDuan) //在封的时候可以用
			{
				return tpNoError;
			}
			else
			{
				return tpSkillTrainWithBuff;
			}
		}
	}
	*/
	
	return tpNoError;
}

// void CSkillSubSystem::AutoLearnSkill()
// {
// 	if(m_pEntity->GetType() != enActor) return;

// 	int nVocation = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);//玩家的职业

// 	CSkillProvider * pProvider =GetLogicServer()->GetDataProvider()->GetSkillProvider();
// 	for(INT_PTR i=1; i< pProvider->count(); i++)
// 	{
// 		const OneSkillData *pData= pProvider->GetSkillData(i);
// 		if(pData)
// 		{
// 			if (pData->bVocation == nVocation && pData->nSkillType  == stPassiveSkill &&
// 				pData->boNotAutoLearn ) //能自动升级本职业的被动技能(必须配置自动学习)
// 			{
// 				INT_PTR nSkillLevel =  GetSkillLevel(i);
// 				for(INT_PTR j=nSkillLevel+1; j <= pData->levels.count; j++)
// 				{
// 					if(tpNoError != LearnSkill( i, j ))
// 					{
// 						break;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	//pProvider->[]
// }

void CSkillSubSystem::AutoLearnVocSkill(int nLevel)
{
	if (m_pEntity->GetType() != enActor) return;

	int nVocation = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);//玩家的职业

	CSkillProvider *pProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	for (INT_PTR i = 1; i < pProvider->count(); i++)
	{
		const OneSkillData *pData= pProvider->GetSkillData(i);
		if(pData)
		{
			if(pData->bVocation == nVocation &&
				(pData->bSkillClass  == scSkillClassVocBasic))
			{
				// if (pData->nSkillType == stPassiveSkill)
				// {
				// 	nLevel = 3;
				// }
				INT_PTR nError = LearnSkill(pData->nSkillID, nLevel, false, true);
				if (nError != 0 )
				{
					OutputMsg(rmWaning,_T("Learnskill:%d,error:%d"),pData->nSkillID,nError);
				}
			}
		}
	}
}

//获取cd
void CSkillSubSystem::OnGetCd(INT_PTR nSkillID)
{

	PSKILLDATA pSkillInfo = GetSkillInfoPtr(nSkillID); 
	if( NULL == pSkillInfo ) return ;
	if (m_pEntity->GetType() != enActor) return;
	
	if ( ((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator())
	{  
		return;
	}

	CActorPacket pack;
	CDataPacket & data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE) enSkillSystemID  << (BYTE) sSendSkillCd;
	data << (WORD) nSkillID ;
	int nLeftTick = (int)(pSkillInfo->tick - GetGlobalLogicEngine()->getTickCount());
	if(nLeftTick <0) nLeftTick =0;
	data << (int) nLeftTick;
	pack.flush();
}

//初始化技能数据
bool CSkillSubSystem::Initialize(void *data,SIZE_T size)
{
	
	if(m_pEntity ==NULL ) return false;
	bool bIsUser = m_pEntity->GetHandle().GetType() == enActor;
	
	if(bIsUser)
	{
		if(data ==NULL) return false;
		PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
		if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
		{
			OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
			return false;
		}
	}
	return true;
}

//判断是否启用技能CD
inline bool CSkillSubSystem::GetEnableCD()
{ 
	return !m_pEntity->HasState(esDisableSkillCD); 
}

//设置技能CD启用与否
void CSkillSubSystem::SetEnableCD(bool boEnable)
{
	boEnable ? m_pEntity->RemoveState(esDisableSkillCD) : m_pEntity->AddState(esDisableSkillCD);
}

//
void CSkillSubSystem::ResetSkillCd(PSKILLDATA  pSkill,const SKILLONELEVEL * pLevel)
{
	if(pSkill ==NULL || pLevel ==NULL) return;
	INT_PTR nTime = pLevel->nCooldownTimes; //技能的冷却时间
	if(nTime != pSkill->nCoolDownTime)
	{
		if(m_pEntity->GetType() ==enActor ) //如果本身是一个玩家的话
		{	
			CActorPacket pack;
			CDataPacket & data = ((CActor*)m_pEntity)->AllocPacket(pack);
			data << (BYTE) enSkillSystemID  << (BYTE) sSetSkillCdTime;
			data << (WORD) pSkill->nSkillID  ;
			data <<(BYTE) pSkill->nLevel;
			data << (int) nTime;
			pack.flush();
		}
		pSkill->nCoolDownTime = (int)nTime;
	}
}

VOID CSkillSubSystem::OnTimeCheck(TICKCOUNT nTick)
{	
	if (!m_pEntity)
	{
		return;
	}
	if(m_currentSingSkillID ==0 ) return; 

	if(m_singTimer.Check(nTick))
	{
		if(m_currentSingSkillID == GATHER_SKILL_ID) //如果是采集怪
		{
			//到时间了就杀死这个怪物
			CMonster *pMonster =(CMonster *) GetEntityFromHandle(m_hSingTarget);
			if(pMonster !=NULL && !pMonster->IsDeath())
			{
				bool boResult = true;
				int nMonsterType = pMonster->GetMonsterType();
				if ( nMonsterType == MONSTERCONFIG::mtFubenCollect)
				{
					CActor * pActor = (CActor *)m_pEntity;
					static int nItemId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nHammerItemId;
					CUserItemContainer::ItemOPParam itemPara;
					itemPara.wItemId = nItemId;
					itemPara.wCount = 1;//设置一个大的数字，不用先看背包有多少这个物品
					itemPara.btStrong = -1;
					itemPara.btQuality = -1;
					if (pActor->GetBagSystem().GetItemCount(nItemId, -1,-1, 1) > 0 )
					{
						itemPara.btBindFlag = 1;
					}
					else
						itemPara.btBindFlag = -1;
					
					if(pActor->GetBagSystem().DeleteItem(itemPara,m_pEntity->GetEntityName(),GameLog::clFubenCollectRemItem) <= 0)
					{
						pActor->SendOldTipmsgWithId(tpFubenCollectNotItem, ttFlyTip);
						boResult = false;
					}
				}
				else if(nMonsterType == MONSTERCONFIG::mtCollectOnce)
				{
					unsigned int nMyId = m_pEntity->GetId();
					CMiscMgr & miscMgr = GetGlobalLogicEngine()->GetMiscMgr();
					pMonster->SetBeAttackActorList(nMyId);
				}
				if (boResult)
				{
					pMonster->SetVestEntity(m_pEntity->GetHandle());
					pMonster->SetVestAttackTime(GetGlobalLogicEngine()->getTickCount());
					if (pMonster->GetAttriFlag().boShowVestEntityName)
					{
						pMonster->SetVestEntityName(m_pEntity->GetEntityName());
					}
					pMonster->ChangeHP (-1 *pMonster->GetProperty<int>(PROP_CREATURE_HP));
					if( pMonster->GetProperty<unsigned int>(PROP_CREATURE_HP) ==0)
					{
						pMonster->OnKilledByEntity(m_pEntity);	
					}
					else
					{
						if (nMonsterType == MONSTERCONFIG::mtGirl || nMonsterType == MONSTERCONFIG::mtCollectOnce)
						{
							pMonster->OnKilledByEntity(m_pEntity);
						}
					}
					}
			}
		}
		else
		{
			OutputMsg(rmWaning,_T("Gath is not SingskillId"));
		}
		m_currentSingSkillID =0;
		m_pEntity->RemoveState(esStateSing);
	}
	
}

void CSkillSubSystem::EndSingSpell(bool isPositive)
{
	//结束吟唱
	if( m_pEntity->HasState(esStateSing) ==false) return;
	
	CActorPacket pack;
	CDataPacket & data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE) enSkillSystemID  << (BYTE) sGatherMonsterProgress;

	data << (BYTE)m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);
	data << (int) 0;

	pack.flush();
	m_pEntity->RemoveState(esStateSing);
	m_currentSingSkillID =0;
}

INT_PTR CSkillSubSystem::GetSpellTargetDistance( INT_PTR nSkillID, INT_PTR nSkillLevel)
{
	const SKILLONELEVEL * pLevel = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID,nSkillLevel);
	if(  pLevel == NULL)
	{
		return 0; 
	}
	const DataList<SKILLTRAINSPELLCONDITION> & spellConditions = pLevel->spellConditions ;
	INT_PTR nConditionCount= spellConditions.count;

	for (INT_PTR i =0; i< nConditionCount ; i++ )
	{
		if(spellConditions[i].nConditionID == scSkillConditionTargetDistance)
		{
			return spellConditions[i].nValue;
		}
	}
	return -1; //没有距离限制

}

void  CSkillSubSystem::BroadSpellSkill(INT_PTR nSkillID,INT_PTR nLevel,INT_PTR nDir,INT_PTR nAuxParam, unsigned short nX, unsigned short nY)
{	
	//播放怪物声音
	int nMusicId =0; //播放的声音

	if(m_pEntity->IsMonster())
	{
		if(m_attackMusicRate ==BYTE(-1))
		{
			const PMONSTERCONFIG pMonster= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
			if( pMonster !=NULL )
			{
				m_attackMusicRate = pMonster->bAttackMusicRate;
				m_attackMusicID   = pMonster->bAttackMusicId;
			}
		}
		if(m_attackMusicID !=0 && m_attackMusicRate !=0)
		{
			//需要播放声音了
			if(m_attackMusicRate >= wrand(101))
			{
				nMusicId = m_attackMusicID;
			}
		}
	}
	
	char buff[128];
	CDataPacket pack(buff,sizeof(buff));
	pack << (BYTE) enDefaultEntitySystemID ;
	if(nSkillID) //这个是释放技能，如果是0表示是普通攻击
	{
		pack << (BYTE) sStartSpellSkill;
		pack << (Uint64) m_pEntity->GetHandle() << (WORD) nSkillID << (BYTE) nLevel << (BYTE) nDir << nAuxParam << nX << nY;
	}
	else //肉搏
	{
		pack << (BYTE) sNearAttack;
		pack << (Uint64) m_pEntity->GetHandle() << (BYTE) nLevel  << (BYTE) nDir <<(WORD)nAuxParam <<nMusicId;
	}
	m_pEntity->GetObserverSystem()->BroadCast(pack.getMemoryPtr(),pack.getPosition(),true);
}

void CSkillSubSystem::CheckSkillOnConditionChange(INT_PTR nSkillClass)
{
	if(m_pEntity ==NULL || m_pEntity->GetType() != enActor) return;

	
	CSkillProvider* pProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	INT_PTR nCount =  pProvider->count(); //技能的数量

	 
	for (INT_PTR i =1 ; i < nCount ; i ++)
	{
		const OneSkillData *pSkill = pProvider->GetSkillData(i);
		if(pSkill ==NULL ) continue; //如果这个技能已经删除的话也不能学习
		
		INT_PTR nSkillId =pSkill->nSkillID; //技能的ID
		
		SKILLONELEVEL * pLevel = pProvider->GetSkillLevelData(nSkillId,1); 
		if(pLevel ==NULL) continue;

		if(pSkill->bSkillClass ==  nSkillClass) //技能的分类是一样的
		{
			PSKILLDATA pSkillLevel= GetSkillInfoPtr(nSkillId);
			bool isInvalid =true;			
			for(INT_PTR j=0; j< pLevel->trainConditions.count; j++ ) //检查有哪些条件无法满足了，不满足的就要遗忘技能
			{
				PSKILLTRAINSPELLCONDITION pCondition = pLevel->trainConditions.pData  +j;
				//条件不满足了
				if(tpNoError != m_targetCondition.CheckUpGradeContion((CActor*)m_pEntity,nSkillId,*pCondition,false))
				{
					isInvalid =false;
					break;
				}
			}
	
			if(isInvalid ==false ) //该技能可以遗忘了
			{
				if(pSkillLevel != NULL)
				{
					ForgetSkill(nSkillId); //遗忘这个技能了
				}
			}
			else //能够学习这个技能了
			{
				//这些技能都只有1级
				if( pSkillLevel ==NULL)
				{
					LearnSkill(nSkillId,1); //学习1级技能
				}
			}		
		}		
	}
}

//遗忘一个技能
bool CSkillSubSystem::ForgetSkill(INT_PTR nSkillId)
{
	bool bResult = false;
	int nSkillLevel = 0;
	for (INT_PTR i =0 ; i< m_skillList.count();i ++)
	{
		if(m_skillList[i].nSkillID ==nSkillId )
		{
			nSkillLevel = m_skillList[i].nLevel;
			m_skillList.remove(i);
			bResult = true;
			break;
		}
	}
	if(bResult)
	{
		const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillId);
		if(pSkill ==NULL ) return false;
		if(m_pEntity->GetType() == enActor)
		{
			((CActor *)m_pEntity)->SendOldTipmsgFormatWithId(tpSkillForgetSkill,ttFlyTip, pSkill->sSkillName);
			//通知客户端已经遗忘了一个技能
			CActorPacket pack;
			CDataPacket &data=((CActor*)m_pEntity)->AllocPacket(pack);
			data << (BYTE)(enSkillSystemID) << (BYTE)sForgetSkill << (WORD) nSkillId ;
			pack.flush();
		}
		if(pSkill->nSkillType == stPassiveSkill || pSkill->bBeedCalPassiveProperty) //如果是被动技能需要刷属性
		{
			//设置角色刷新属性的标记
			m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);
			OnForgetPassSkill(pSkill,nSkillLevel);
		}

		SetDataModifyFlag(true); //数据发生了改变
	}
	
	return bResult;	
}

bool CSkillSubSystem::ForgetSkillByClass(INT_PTR nSkillClass)
{
	if(m_pEntity ==NULL || m_pEntity->GetType() != enActor) return false;

	CSkillProvider *pProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	INT_PTR nCount = pProvider->count();

	for (INT_PTR i = 0; i < nCount; i++)
	{
		const OneSkillData *pSkill = pProvider->GetSkillData(i);
		if(pSkill == NULL ) continue; //如果这个技能已经删除的话也不能学习

		INT_PTR nSkillId = pSkill->nSkillID; //技能的ID

		SKILLONELEVEL *pLevel = pProvider->GetSkillLevelData(nSkillId,1); 
		if(pLevel == NULL) continue;

		if(pSkill->bSkillClass == nSkillClass) //技能的分类是一样的
		{
			PSKILLDATA pSkillLevel= GetSkillInfoPtr(nSkillId);
			if(pSkillLevel != NULL ) //已经删除的技能，铁定要删除的
			{
				ForgetSkill(nSkillId); //遗忘这个技能
			}
		}
	}
	SetDataModifyFlag(true); //数据发生了改变
	return true;
}


//下发已经学习的技能列表 
void CSkillSubSystem::SendLearnSkill()
{
	if(m_pEntity ==NULL) return;
	CActorPacket pack;
	CDataPacket &data=((CActor*)m_pEntity)->AllocPacket(pack);

	//初始化玩家的技能 BYTE:count,( WORD:nSkillID,char:nLevel,unsigned char:nLearnedSecretSauce,unsigned int nExpOrCd) count个
	data << (BYTE)(enSkillSystemID) << (BYTE)sInitUserSkill;
	INT_PTR nCountOffer = data.getPosition();
	data << (BYTE) m_skillList.count();
	int nCount = 0;

	for (INT_PTR i = 0 ; i < m_skillList.count(); i++)
	{
		SKILLDATA& skillData = m_skillList[i];
		TICKCOUNT nLeftTick = skillData.tick  - GetGlobalLogicEngine()->getTickCount() ;
		if(nLeftTick <0) nLeftTick = 0;

		const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(skillData.nSkillID);
		if (pSkill->boIsDelete) continue;
		
		data << (WORD)skillData.nSkillID << (BYTE)skillData.nLevel;
		data << (int)nLeftTick;
		BYTE nActiveState = 0;//技能是否激活可用
		nActiveState = !skillData.nIsClosed;
		data << (BYTE)nActiveState;
		data << (BYTE)skillData.bIsClose;
		nCount++;
	}
	INT_PTR nFinishOffer = data.getPosition();
	data.setPosition(nCountOffer);
	data << (BYTE)nCount;
	data.setPosition(nFinishOffer);
	pack.flush();

	UpdateNextSkillFlag();
} 

void CSkillSubSystem::SendPetSkillCountChange(INT_PTR nSkillId, INT_PTR nCount)
{
	if (m_pEntity == NULL || !m_pEntity->IsInited())
	{
		return;
	}
	CActorPacket pack;
	CDataPacket &data=((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)(enSkillSystemID) << (BYTE)sSkillSystemPetCountChange <<(WORD)nSkillId  <<(BYTE)nCount;
	pack.flush();
}

void CSkillSubSystem::OnLevelUp(int nLevel)
{
	if (m_pEntity->GetType() == enActor)
	{
		int nVocation = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);//玩家的职业
		CSkillProvider * pProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();

		// 检测职业技能
		{
			std::vector<OneSkillData*>& skills = pProvider->GetVocationSKills(nVocation);
			int nCount = skills.size();
			for(INT_PTR i=0; i< nCount; i++)
			{
				const OneSkillData *pData= skills[i];
				if(pData)
				{
					if (pData->bIsAutoLearn && (pData->bVocation == nVocation))
					{
						if (nLevel >= pData->nAutoLearnSkillLvl)
						{
							INT_PTR nSkillLevel =  GetSkillLevel(pData->nSkillID);
							if(nSkillLevel == 0 && tpNoError != LearnSkill( pData->nSkillID, 1 ))
							{
								break;
							}
						}
					}
				}
			}
		}

		// 检测非职业技能
		{
			std::vector<OneSkillData*>& skills = pProvider->GetVocationSKills(0);
			int nCount = skills.size();
			for(INT_PTR i=0; i< nCount; i++)
			{
				const OneSkillData *pData= skills[i];
				if(pData)
				{
					if (pData->bIsAutoLearn && (pData->bVocation == 0))
					{
						if (nLevel >= pData->nAutoLearnSkillLvl)
						{
							INT_PTR nSkillLevel =  GetSkillLevel(pData->nSkillID);
							if(nSkillLevel == 0 && tpNoError != LearnSkill(pData->nSkillID, 1 ))
							{
								break;
							}
						}
					}
				}
			}
		}
	}
}

void CSkillSubSystem::OnEnterGame()
{
	const SKILLONELEVEL * pLevel;
	CSkillProvider* provider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	for(INT_PTR i=0;i < m_skillList.count(); i++)
	{
		pLevel= provider->GetSkillLevelData(m_skillList[i].nSkillID, m_skillList[i].nLevel);
		if(pLevel)
		{
			ResetSkillCd(&m_skillList[i], pLevel);
		}
	}

	if (((CActor*)m_pEntity)->m_isFirstLogin)
	{
		int nVocation = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);//玩家的职业
		int nLevel = m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);//玩家等级
		CSkillProvider * pProvider =GetLogicServer()->GetDataProvider()->GetSkillProvider();
		for(INT_PTR i=1; i< pProvider->count(); i++)
		{
			const OneSkillData *pData= pProvider->GetSkillData(i);
			if(pData)
			{
				if (pData->bIsAutoLearn && (pData->bVocation == 0 || pData->bVocation == nVocation))
				{
					if (nLevel >= pData->nAutoLearnSkillLvl)
					{
						INT_PTR nSkillLevel =  GetSkillLevel(i);
						for(INT_PTR j=nSkillLevel+1; j <= pData->levels.count; j++)
						{
							if(tpNoError != LearnSkill( i, j ))
							{
								break;
							}
						}
					}
				}
			}
		}

		// 新创建角色，默认技能快捷键
		int maxidx = 10;
		int maxidxApp = 4;
		std::vector<int>& skillList = GetLogicServer()->GetDataProvider()->GetFirstLoginKeySet();
		for (size_t i = 0; i < skillList.size() && i <= maxidx; i++)
		{
			int skillid = skillList[i];
			((CActor*)m_pEntity)->GetGameSetsSystem().DoUpdateGameSetsData(1,maxidx--,skillid,-1);
			((CActor*)m_pEntity)->GetGameSetsSystem().DoUpdateGameSetsDataApp(1,maxidxApp--,skillid,-1);
		}
	}
}
	
//1表示启用，0表示不启用就是停止 
void CSkillSubSystem::HandSetSkillOpen(INT_PTR nSkillId,INT_PTR nFlag)
{
	PSKILLDATA pSkill= GetSkillInfoPtr(nSkillId);
	if(pSkill ==NULL) return;
	//技能的配置的数据
	const OneSkillData * pSkillData= GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillId);
	if(pSkillData ==NULL) return ; //压根就没有这个技能
	//if(pSkillData->bSkillClass != scSkillClassJianghuMulti) return; //不是在江湖绝学无法开启
	
	if( pSkill->nIsClosed != (BYTE) nFlag)  return;  //状态本来就是对的
	
	if(nFlag)
	{
		pSkill->nIsClosed =0;
	}
	else
	{
		pSkill->nIsClosed =1; //关闭
	}
	CActorPacket pack;
	CDataPacket &data=((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)(enSkillSystemID) << (BYTE)sSetSkillOpen <<(WORD)nSkillId <<(BYTE)nFlag;
	pack.flush();	
}

int qsortSkillCmp(const void *a ,const void *b)
{
	CSkillSubSystem::SKILLDATA  *SkillA = (CSkillSubSystem::SKILLDATA*)a;
	CSkillSubSystem::SKILLDATA  *SkillB= (CSkillSubSystem::SKILLDATA*)b;
	if(SkillA->nCoolDownTime  < SkillB->nCoolDownTime)
		return -1;
	else if (SkillA->nCoolDownTime  == SkillB->nCoolDownTime)
		return 0;
	return 1;
}

//学习技能
INT_PTR  CSkillSubSystem::LearnSkill(INT_PTR nSkillID, INT_PTR nLevelID,bool bIsClose, bool bForce ,bool bSort)
{
	INT_PTR nErrorCode = GetLearnSkillErrorCode(nSkillID ,nLevelID, bForce) ;
	if(nErrorCode == tpNoError)
	{
		StartRealLearnSkill(nSkillID,nLevelID,bIsClose);
	}
	if(bSort)
	{
		INT_PTR  nCount = m_skillList.count();
		qsort(&m_skillList[0], nCount, sizeof(m_skillList[0]),qsortSkillCmp);
	}
	return nErrorCode;
}


/*正式开始学习技能
扣除需要的资源
*/
void  CSkillSubSystem::StartRealLearnSkill(INT_PTR nSkillID, INT_PTR nLevelID, bool bIsClose)
{
	CSkillProvider *provider= GetLogicServer()->GetDataProvider()->GetSkillProvider();
	const OneSkillData * pSkillData = provider->GetSkillData(nSkillID);
	const SKILLONELEVEL * pLevel = provider->GetSkillLevelData(nSkillID,nLevelID);
	if(pLevel ==NULL || pSkillData==NULL ) return;

	DataList<SKILLTRAINSPELLCONDITION> const& spellConditions = pLevel->trainConditions ;
	bool isActor = m_pEntity->GetHandle().GetType() == enActor;
	INT_PTR nNewLevel = 1;
	
	//升级消耗
	if(isActor)
	{
		if (pSkillData->bSkillClass == scSkillClassMonster )
		{
			return;
		}
		//是否删除
		if (pSkillData->boIsDelete)
		{
			return;
		}
		for(INT_PTR i = 0; i< spellConditions.count; i++)
		{
			if(spellConditions[i].bConsumed ==false || spellConditions[i].nValue ==0) continue; //如果不需要消耗就不消耗
			m_targetCondition.CheckUpGradeContion(m_pEntity,nSkillID,spellConditions[i],true ); //去消耗
		}
	}
	
	//以前是否学习过这个技能
	bool hasLearned = false; 
	for (INT_PTR i =0 ; i< m_skillList.count();i ++)
	{
		if(m_skillList[i].nSkillID == nSkillID )
		{
			m_skillList[i].nLevel =(int)nLevelID;
			m_skillList[i].bIsClose = bIsClose ?1:0;
			nNewLevel = m_skillList[i].nLevel; 
			m_skillList[i].nCoolDownTime =  pLevel->nCooldownTimes;
			hasLearned = true;
			break;
		}
	}
	if(hasLearned == false)
	{
		BYTE bConfigPriority = pSkillData->bPriority; //优先级

		if(nLevelID <=0)
		{
			nLevelID =1;
		}
		SKILLDATA data;
		data.nSkillID =(WORD) nSkillID;
		data.nLevel = (BYTE)nLevelID;
		data.nCd =0;
		data.nIsClosed = 1;
		data.tick = GetGlobalLogicEngine()->getTickCount();
		data.nExp =0;
		data.nMijiExpiredTime=0;
		data.bMjMask =0;
		data.nMijiId =0;
		data.bEvent =0;
		data.bPriority =bConfigPriority;
		data.nCoolDownTime = pLevel->nCooldownTimes; //冷却时间
		data.nLastDbCd = 0;
		data.bIsClose = bIsClose ?1:0;
		data.bIsPassive = ((pSkillData->nSkillType == stPassiveSkill)?1:0);
		data.nSkillType = pSkillData->nSkillType;

		if(bConfigPriority >0)
		{
			INT_PTR nPos =0;
			for(INT_PTR i=0; i < m_skillList.count(); i++)
			{
				if( bConfigPriority >= m_skillList[i].bPriority )
				{
					nPos =i;
					break;
				}
			}
			m_skillList.insert(nPos,data);
		}
		else
		{
			m_skillList.add(data); //学习一个技能成功
		}

		if(pSkillData->nSkillType != stPassiveSkill && (pSkillData->bSkillClass == scSkillClassVocBasic) && (m_pEntity->GetHandle().GetType() == enActor) ) //学习技能自动补位
			((CActor*)m_pEntity)->GetGameSetsSystem().AutoGameSetSkill(1, nSkillID);
	}
	
	//向玩家发送数据包，技能学习成功
	if( m_pEntity->GetHandle().GetType() == enActor )  //如果是一个玩家的话，还需要发条消息到客户端
	{
		CActorPacket pack;
		CActor *pActor = ((CActor*)m_pEntity) ;
		CDataPacket &data = pActor->AllocPacket(pack);
		data << (BYTE)enSkillSystemID << (BYTE)sTrainSkillResult << (WORD)nSkillID << (BYTE)nLevelID;
		data << (int)(pLevel->nCooldownTimes);
		SKILLDATA * pSkill = GetSkillInfoPtr(nSkillID);
		data << (BYTE)(!pSkill->nIsClosed);
		data << (BYTE)pSkill->bIsClose;
		pack.flush();
		//pActor->GetQuestSystem()->OnEvent(CQuestData::qtSkillLevel,nSkillID,nNewLevel);
		if(nNewLevel ==1) //通知玩家成功学习一个技能
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmSuccLearnSkill,tstUI,pSkillData->sSkillName);
		}
		else
		{
			//技能等级提升
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmSuccUpgrSkill,tstUI,pSkillData->sSkillName,nNewLevel);
		}
	}
	
	//如果是被动技能需要刷属性
	if(pSkillData->nSkillType == stPassiveSkill || pSkillData->bBeedCalPassiveProperty)
	{
		//设置角色刷新属性的标记
		m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);
	}

	//技能发生了改变
	SetDataModifyFlag(true);

	//累计技能等级
	if(isActor)
	{
		((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtSkillLv, nLevelID,nSkillID);
        //  CActor *pActor = ((CActor*)m_pEntity) ;
		//  int  nTotalSkillLevel = 0;
		//  for (INT_PTR i =0 ; i< m_skillList.count();i ++)
		//  {
		// 	 const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(m_skillList[i].nSkillID);
		// 	 if(pSkill != NULL && pSkill->bSkillClass == scSkillClassVocBasic) //技能的分类是一样的
		// 	 {
		// 		  nTotalSkillLevel +=  m_skillList[i].nLevel;
		// 	 }
		//
		
		// }
	}
	
	//强制开启半月
	if (nSkillID == 4 || nSkillID == 3)
	{
		SwitchSkill(nSkillID,1);
	}
	
	// //技高一筹
	// OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(MAX_TOP_TITLE_SKILL);
	// if(pConfig == NULL || 	pConfig->nConditionCount < 1 || !isActor)	
	// {
	// 	return;
	// }

	// //技惊四座
	// OneNewTitleConfig* pConfig2 = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(MAX_TOP_TITLE_SKILL_2);
	// if(pConfig2 == NULL || 	pConfig2->nConditionCount < 1 || !isActor)	
	// {
	// 	return;
	// }

	// //技能头衔
	// const int maxSkillNum = 6;
	// if(maxSkillNum <= m_skillList.count())
	// {
	// 	int count = 0;
	// 	int count2 = 0;
	// 	for (INT_PTR i =0 ; i< m_skillList.count();i ++)
	// 	{
	// 		if( pConfig->ConditionList[0].value.nValue <= m_skillList[i].nLevel)
	// 		{
	// 			const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(m_skillList[i].nSkillID);
	// 			if(pSkill != NULL && pSkill->bSkillClass == scSkillClassVocBasic) //技能的分类是一样的
	// 			{
	// 				count++;
	// 			}
	// 		}
	// 		if( pConfig2->ConditionList[0].value.nValue <= m_skillList[i].nLevel)
	// 		{
	// 			const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(m_skillList[i].nSkillID);
	// 			if(pSkill != NULL && pSkill->bSkillClass == scSkillClassVocBasic) //技能的分类是一样的
	// 			{
	// 				count2++;
	// 			}
	// 		}
	// 	}
	// 	if(maxSkillNum <= count && ((CActor*)m_pEntity)->GetNewTitleSystem().IsHaveNewTitle(MAX_TOP_TITLE_SKILL) < 0 )
	// 	{
	// 		((CActor*)m_pEntity)->GetNewTitleSystem().addNewTitle(MAX_TOP_TITLE_SKILL);
	// 	}
	// 	if(maxSkillNum <= count2 && ((CActor*)m_pEntity)->GetNewTitleSystem().IsHaveNewTitle(MAX_TOP_TITLE_SKILL_2) < 0)
	// 	{
	// 		((CActor*)m_pEntity)->GetNewTitleSystem().addNewTitle(MAX_TOP_TITLE_SKILL_2);
	// 	}
		
	// }
	
	////////////////////////////////////////////////////////////////////////////////////
}

bool CSkillSubSystem::HasLongCdSkill()
{
	LONGLONG tick = GetGlobalLogicEngine()->getTickCount(); //当前的时间
	unsigned int nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime(); //当前的时间
	size_t count = m_skillList.count();
	for (UINT_PTR i=0; i<count; i ++ )
	{

		LONGLONG nLeftCdTime = m_skillList[i].tick - tick  ; //还剩余多少时间
		
		if(nLeftCdTime > 5000 ) //只有CD时间离当前的时间比较长的时候才需要这么做
		{
			if(  m_skillList[i].nLastDbCd  < nCurrentTime )
			{
				return true;
			}
		}
	}
	return false;

}

//存盘
void CSkillSubSystem::Save(PACTORDBDATA/* pData*/)
{
	if(m_pEntity ==NULL || m_pEntity->GetType() != enActor) return;
	if(HasDbDataInit() ==false)return; //如果DB数据没有初始化就不存盘
	if(HasDataModified() || HasLongCdSkill()) 
	{
		size_t count = m_skillList.count();
		//if(count <=0) return;
		CDataPacket& dataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveSkill);
		dataPacket << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		dataPacket <<(unsigned int ) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //玩家ID
		INT_PTR pos = dataPacket.getPosition();
		dataPacket  << (int)0; //玩家技能的数量
		int nSaveCount = 0;
		ULONGLONG tick = GetGlobalLogicEngine()->getTickCount(); //当前的时间
		unsigned int nCurrentTime= GetGlobalLogicEngine()->getMiniDateTime() ;
		//玩家的技能的冷却是需要存盘的，根据当前的tickcount，然后算出还有多长时间过期
		for (UINT_PTR i=0; i<count; i ++ )
		{

			const OneSkillData * pData =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(m_skillList[i].nSkillID);
			if (!pData || pData->bSkillClass == scSkillClassMonster)
			{
				continue;
			}
			LONGLONG nLeftCdTime = m_skillList[i].tick - tick  ; //还剩余多少时间
			if(nLeftCdTime >0 )
			{
				m_skillList[i].nCd =nCurrentTime+ (int)(nLeftCdTime/1000) ; //获取当前的时间			
			}
			else
			{
				m_skillList[i].nCd =0;
			}
			m_skillList[i].nLastDbCd = m_skillList[i].nCd;
			dataPacket.writeBuf(&m_skillList[i],sizeof( ONESKILLDBDATA) ); //只保持db需要的那部分
			nSaveCount++;
		}
		int *pCount = (int *)dataPacket.getPositionPtr(pos);
		*pCount = (int)nSaveCount;
		GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
		SetDataModifyFlag(false);
	}
}

void CSkillSubSystem::SetSkillClose(INT_PTR nSkillID, bool isClose)
{
	 PSKILLDATA pData = GetSkillInfoPtr(nSkillID);
	 if(pData)
	 {
		 pData->bIsClose = isClose ?1:0;
	 }
}

//DB装载技能
VOID  CSkillSubSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader & reader )
{
	if(m_pEntity ==NULL )
	{
		OutputMsg(rmError,_T("装载玩家技能的数据出错,实体指针为MULL"));
		return;
	}
	//技能的配置的读取器
	CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();  //技能数据读取器
	CStdItemProvider & itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider(); //物品配置器
	
	if(nCmd ==jxInterSrvComm::DbServerProto::dcLoadSkill &&  nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	{
		if(Inherid::HasDbDataInit()) return; //已经装载了这个技能数据
		
		int count = 0;
		reader >> count;
		//if(count <=0) return;
		//如果已经有数据，可能出现了一点问题
		if(m_skillList.count() >0)
		{
			m_skillList.clear();
		}
		
		//这里写玩家的技能数据
		if(count >0)
		{
			m_skillList.reserve(count);

			unsigned int nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime(); //当前的时间
			ULONGLONG tick =  GetGlobalLogicEngine()->getTickCount(); //获取当前的逻辑的tick

			SKILLDATA  data;
			for(INT_PTR i=0; i <count; i++)
			{
				reader.readBuf(&data,sizeof(ONESKILLDBDATA) );
				data.bEvent =0;
				data.bMjMask =0;
				const SKILLONELEVEL * pLevel = pSkillProvider->GetSkillLevelData(data.nSkillID, data.nLevel); 
				const OneSkillData *pConfig =  pSkillProvider->GetSkillData(data.nSkillID);
				if(pLevel ==NULL || pConfig ==NULL) continue;

				if(pLevel)
				{
					data.nCoolDownTime = pLevel->nCooldownTimes; //冷却时间
				}
				else
				{
					continue;
				}
				if(data.nCd  > nCurrentTime)
				{
					data.tick = tick + (data.nCd  - nCurrentTime) * 1000; //这个是新的冷却时间
					
				}
				else
				{
					data.tick =0;
					data.nCd  =0;
					
				}
				data.nLastDbCd = data.nCd;

				if(data.nMijiId) //如果镶嵌了秘籍
				{
					bool isInvalid = false;
					if(data.nMijiExpiredTime  > GetGlobalLogicEngine()->getMiniDateTime()) //如果秘籍还有效的话
					{
						const CStdItem*  pStdItem = itemProvider.GetStdItem(data.nMijiId);
						if(pStdItem )
						{
							isInvalid =true;
							data.bMjMask = (BYTE)pStdItem->m_wSuitID; //这个是真正生效的秘籍的效果
						}
					}
					if(!isInvalid)
					{
						data.nMijiId =0;
						data.nMijiExpiredTime =0; //已经过期了
					}
				}

				data.bIsPassive = ((pConfig->nSkillType == stPassiveSkill)?1:0);
				data.bPriority = pConfig->bPriority;
				data.nSkillType = pConfig->nSkillType;
				m_skillList.add(data);
			}
			
		}
		Inherid::OnDbInitData();
		((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_SKILL_SYSTEM); //完成一个步骤
	}
	else if(nCmd ==jxInterSrvComm::DbServerProto::dcLoadSkill &&  nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
	{
		OutputMsg(rmError,_T("装载玩家[%s]的技能的数据出错,actorid=%d"),((CActor*)m_pEntity)->GetEntityName(),m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
	}
}


INT_PTR CSkillSubSystem::GetLearnSkillErrorCode(INT_PTR nSkillID, INT_PTR nLevelID, bool bForce)
{
	if( GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID) == NULL)
	{
		return tpSkillConfigError;
	}
	SKILLONELEVEL * pLevel = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID,nLevelID);
	if(GetSkillLevel(nSkillID) >= nLevelID) return tpSkillTrainHasLearned; //已经学习过这个技能了
	if(pLevel ==NULL) return tpSkillIsUpMostLevel; //已经是最高等级了
	if( m_pEntity->GetHandle().GetType() !=enActor ) return tpNoError; //怪物的话不需要判断条件
	if (bForce) return tpNoError;	//强制学习
	INT_PTR nErrorCode = tpNoError;
	DataList<SKILLTRAINSPELLCONDITION> & conditions = pLevel->trainConditions ;
	for(INT_PTR i = 0; i< conditions.count; i++)
	{
		nErrorCode = m_targetCondition.CheckUpGradeContion(m_pEntity,nSkillID,conditions[i],false);
		if(nErrorCode != tpNoError) return nErrorCode;
	}
	return nErrorCode; //没有错误
}

/*计算一个技能的属性
skill_x_x.txt的配置，如
results = 
{
	{ resultType = 7, delay = 200, id = 119, value = 0.1, param = 0 },
	{ resultType = 7, delay = 200, id = 120, value = 0.25, param = 0 },
	{ resultType = 7, delay = 200, id = 109, value = 0.03, param = 0 },
},
*/
VOID CSkillSubSystem::CalcOneSkillAttr(INT_PTR nSkillID,INT_PTR nLevel,CAttrCalc &calc)
{
	CSkillProvider *pProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	if ( !pProvider )
	{
		return;
	}
	const OneSkillData * pData = pProvider->GetSkillData(nSkillID);
	if(pData ==NULL) return; 

	if(pData->nSkillType != stPassiveSkill &&  pData->bBeedCalPassiveProperty ==false) return;

	
	if(nLevel <=0 )return;
	SKILLONELEVEL * pLevel = pProvider->GetSkillLevelData(nSkillID,nLevel);
	if(pLevel ==NULL) return;
	GAMEATTR attr;
	for(INT_PTR rangeID=0; rangeID < pLevel->pranges.count; rangeID++)
	{
		PSKILLONERANGE pRange = pLevel->pranges[rangeID];
		INT_PTR nResultCount= pRange->skillResults.count;
		for(INT_PTR resultID =0; resultID < nResultCount; resultID ++)
		{
			PSKILLRESULT pResult = &(pRange->skillResults[resultID]);
			if(pResult->nResultType != srSkillResultChangeProperty ) continue;
			attr.type =  (BYTE)pResult->nId; //属性的ID

			//属性计算
			if(pResult->nParam1 == ptValueTypeFloat ) //如果是浮点数的
			{
				attr.value.fValue = (float)pResult->nValue /10000; // 万分之1
				calc << attr;
			}
			else if(pResult->nParam1 == ptValueTypeInt)
			{
				attr.value.nValue = pResult->nValue; //是整数
				calc << attr;
			}
		}
	}
}
void CSkillSubSystem::OnForgetPassSkill(const OneSkillData* pSkill, int nSkillLevel)
{
    if (!m_pEntity) return;
	if (!pSkill)
	{
		return;
	}
	CSkillProvider *pProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	if ( !pProvider )
	{
		return;
	}
	SKILLONELEVEL * pLevel = pProvider->GetSkillLevelData(pSkill->nSkillID, nSkillLevel);
	if (!pLevel)
	{
		return;
	}
	for(INT_PTR rangeID=0; rangeID < pLevel->pranges.count; rangeID++)
	{
		PSKILLONERANGE pRange = pLevel->pranges[rangeID];

		INT_PTR nResultCount= pRange->skillResults.count;
		for(INT_PTR resultID =0;  resultID < nResultCount; resultID ++)
		{
			PSKILLRESULT pResult = &(pRange->skillResults[resultID]);
			if(pResult->nResultType != srSkillResultChangeProperty ) continue;
			if (pResult->nParam1 == ptValueTypeSkillAddBuff)
			{
				if (m_pEntity->GetBuffSystem()->Exists(pResult->nValue))
				{
					m_pEntity->GetBuffSystem()->RemoveById(pResult->nValue,false);
				}
			}

		}
	}
}

VOID CSkillSubSystem::CalcAttributes(CAttrCalc &calc)
{
	if(m_pEntity ==NULL) return ;

	size_t count = m_skillList.count();
	if(count <=0) return;

	for( size_t i=0; i<count; i ++ )
	{
		INT_PTR nSkillID=  m_skillList[i].nSkillID;
		INT_PTR nLevel	=  m_skillList[i].nLevel;
		CalcOneSkillAttr(nSkillID, nLevel, calc);			//计算技能再results中配置的属性
	}	
}


bool CSkillSubSystem::CheckGather(CMonster *pTarget, int nPosX, int nPosY, BYTE nDir)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor || !pTarget || pTarget->GetType() != enGatherMonster)
		return false;
	CActor* pActor = (CActor*)m_pEntity;
	const PMONSTERCONFIG pConfig  = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(pTarget->GetId());
	if ( pConfig && pConfig->nGatherLevel > 0)
	{
		if (!pActor->CheckLevel(pConfig->nGatherLevel,0))
		{
			pActor->SendOldTipmsgFormatWithId(tpLevelNotEnoughGather, ttFlyTip, pConfig->nGatherLevel);
			return false;
		}
	}

	if( pActor->GetBagSystem().availableMinCount() <= 0 )		//背包满了，不能采集
	{
		((CActor*)m_pEntity)->SendOldTipmsgWithId(tpBagFullCantGather,ttFlyTip);
		return false;
	}

	int nTargetMonsterType = pTarget->GetMonsterType();
	unsigned int nOnwerActorId =  pTarget->GetOwnerActorId() ;
	if(nOnwerActorId) //自己的actorid
	{
		if(nTargetMonsterType == MONSTERCONFIG::mtCollectOnce )
		{
			if(pTarget->InBeAttackActorList(m_pEntity->GetId()))
			{
				((CActor *)m_pEntity)->SendOldTipmsgWithId(tpOnlyCollectOnce, ttFlyTip);
				return false;
			}
		}
		else
		{
			if( m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) != nOnwerActorId )
			{
				((CActor*)m_pEntity)->SendOldTipmsgWithId(tpGatherNotOwner,ttFlyTip);
				return false;
			}
		}
	}
	else
	{
		if ( nTargetMonsterType != MONSTERCONFIG::mtGirl)
		{
			EntityHandle hd=pTarget->GetVestEntity();

			if(!hd.IsNull() && GetEntityFromHandle(hd) != NULL && hd != m_pEntity->GetHandle())
			{
				((CActor*)m_pEntity)->SendOldTipmsgWithId(tpGatherNotOwner,ttFlyTip);
				return false;
			}
			if(nTargetMonsterType == MONSTERCONFIG::mtFubenCollect)
			{
				static int nItemId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nHammerItemId;
				CActor * pActor = (CActor *)m_pEntity;
				if(pActor->GetBagSystem().GetItemCount(nItemId) <= 0)
				{
					pActor->SendOldTipmsgWithId(tpFubenCollectNotItem, ttFlyTip);
					return false;
				}
			}
		}
	}

	//pTarget->OnAttacked(m_pEntity);

	//if(pTarget->Getw)
	// 检测公共操作定时器	
	if (!m_pEntity->CheckCommonOpTick(((CActor *)m_pEntity)->GetGateNetWorkTickCount()+50, false))
		return false;

	// 检测采集距离和朝向
	int nSelfX = 0, nSelfY = 0;
	m_pEntity->GetPosition(nSelfX, nSelfY);
	if (abs(nSelfX - nPosX) > 1 || abs(nSelfY - nPosY) > 1)
	{		
		return false;
	}

	m_pEntity->SetTarget(pTarget->GetHandle());
	INT_PTR nNewDir = CEntity::GetDir(nSelfX, nSelfY, nPosX, nPosY);
	m_pEntity->SetDir(nNewDir);
	return true;
}

bool CSkillSubSystem::GathMonsterCallBack(CEntity* pMonster)
{
	bool boReslut = false;
	if ( !m_pEntity || m_pEntity->GetType() != enActor || !pMonster || pMonster->GetType() != enGatherMonster)
	{
		return boReslut;
	}
	CScriptValueList paramList, retParamList;
	paramList << (CActor*)m_pEntity;
	paramList << (CMonster*)pMonster;
	CNpc * pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc != NULL)
	{
		if(!pNpc->GetScript().Call("OnGatherMonster", paramList, retParamList))
		{
			const RefString & s = pNpc->GetScript().getLastErrorDesc();
			((CActor*)m_pEntity)->SendTipmsg((LPCSTR)s, ttDialog);
			return boReslut;
		}
	}
	if (retParamList.count() > 0)
	{
		boReslut = (bool)retParamList[retParamList.count() - 1];
	}
	return boReslut;
}


void CSkillSubSystem::GatherMonster(EntityHandle hTarget, int nPosX, int nPosY, BYTE nDir)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
		return;
	CMonster *pMonster = (CMonster *)GetEntityFromHandle(hTarget);

	//需要告诉玩家不能采集
	
	//检查采集条件
	if (!CheckGather(pMonster, nPosX, nPosY, nDir))
	{
		((CActor*)m_pEntity)->SendOperateResult(false);
		return;
	}

	//被别人踩死了
	int nMonsterHP	= pMonster->GetProperty<int>(PROP_CREATURE_HP);
	if(nMonsterHP <=0)
	{
		((CActor*)m_pEntity)->SendOperateResult(false);
		return;
	}
	int nGatherVal	= 1;
	
	if(nGatherVal ==0) return ;

	m_pEntity->SetCommonOpNextTime(m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_ATTACK_SPEED), false, true);

	m_hSingTarget = pMonster->GetHandle(); //保存这个目标的handle
	m_currentSingSkillID = GATHER_SKILL_ID ; //正在采集

	int nMaxHp = pMonster->GetProperty<int>(PROP_CREATURE_MAXHP);
	unsigned int nSingTime =1000  * ( nMaxHp/ nGatherVal );

	m_singTimer.SetNextHitTimeFromNow(nSingTime);

	CActorPacket pack;
	CDataPacket & data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE) enSkillSystemID  << (BYTE) sGatherMonsterProgress;
	data << (BYTE)m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);
	data << (int) nSingTime;
	m_pEntity->AddState(esStateSing);//开始吟唱
	pack.flush();
}

void  CSkillSubSystem::BroadGather()
{
	char buff[128];
	CDataPacket pack(buff,sizeof(buff));


	int nSelfX, nSelfY;
	m_pEntity->GetPosition(nSelfX, nSelfY);

	pack << (BYTE) enDefaultEntitySystemID ;
	pack << (BYTE) sGather;
	pack << (Uint64) m_pEntity->GetHandle();
	pack << (unsigned short)nSelfX;
	pack << (unsigned short)nSelfY;
	pack << (BYTE)m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR) ;	
	
	m_pEntity->GetObserverSystem()->BroadCast(pack.getMemoryPtr(), pack.getPosition(), false);
}

void CSkillSubSystem::SetInitiativeAttack(CEntity * myEntity,CEntity * pEntity)
{
	if(myEntity == NULL || pEntity == NULL) return;

	if(pEntity->IsDeath()) return;

	if(myEntity->GetType() ==  enActor && pEntity->GetType() == enActor)
	{
		INT_PTR nPkMode = ((CActor *)myEntity)->GetProperty<int>(PROP_ACTOR_PK_MOD);

		if(nPkMode == fpPeaceful) return;

		if(((CActor *)pEntity)->GetNameColorData() < 2 && ((CActor *)myEntity)->GetNameColorData() != 3)
		{
			if (myEntity != pEntity)
			{
				((CActor *)myEntity)->SetAttackOthersFlag(true);
				((CActor *)myEntity)->GetObserverSystem()->UpdateActorEntityProp();
			}
		}
	}
}

void CSkillSubSystem::StopGather()
{
	if (m_pEntity != NULL && m_currentSingSkillID == GATHER_SKILL_ID)
	{
		m_currentSingSkillID = 0;
		m_pEntity->RemoveState(esStateSing);
	}
}

//跨服相关数据
void CSkillSubSystem::SendMsg2CrossServer(int nType) {
	if(!m_pEntity) return;
	if(m_pEntity->GetType() != enActor)
	{
		OutputMsg(rmError,_T("跨服非玩家技能的数据"));
		return;
	}
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = m_pEntity->GetId();
	packet << nServerId << nActorID;
	packet <<(BYTE)nType;
	packet << ((CActor*)m_pEntity)->GetCrossActorId();
	packet << ((CActor*)m_pEntity)->GetAccountID();
	//数据
	OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "),nType,nActorID);
	packet << (int)(m_skillList.count());
	for(int i = 0; i < m_skillList.count(); i++) {
		packet << m_skillList[i];
	}
	pCrossClient->flushProtoPacket(packet);
}


//跨服初始化
VOID  CSkillSubSystem::OnCrossInitData(std::vector<CSkillSubSystem::SKILLDATA>& skillData)
{
	if(m_pEntity ==NULL )
	{
		OutputMsg(rmError,_T("装载玩家技能的数据出错,实体指针为MULL"));
		return;
	}
	if(m_pEntity->GetType() != enActor)
	{
		OutputMsg(rmError,_T("装载非玩家技能的数据出错"));
		return;
	}
	//技能的配置的读取器
	CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();  //技能数据读取器
	CStdItemProvider & itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider(); //物品配置器
	
	if(Inherid::HasDbDataInit()) return; //已经装载了这个技能数据
	
	int count = skillData.size();
	// reader >> count;
	//如果已经有数据，可能出现了一点问题
	if(m_skillList.count() >0)
	{
		m_skillList.clear();
	}
	
	//这里写玩家的技能数据
	if(count >0)
	{
		m_skillList.reserve(count);

		unsigned int nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime(); //当前的时间
		ULONGLONG tick =  GetGlobalLogicEngine()->getTickCount(); //获取当前的逻辑的tick

		SKILLDATA  data;
		for(INT_PTR i=0; i <count; i++)
		{
			// reader.readBuf(&data,sizeof(ONESKILLDBDATA) );
			data = skillData[i];
			data.bEvent =0;
			data.bMjMask =0;
			const SKILLONELEVEL * pLevel = pSkillProvider->GetSkillLevelData(data.nSkillID, data.nLevel); 
			const OneSkillData *pConfig =  pSkillProvider->GetSkillData(data.nSkillID);
			if(pLevel ==NULL || pConfig ==NULL) continue;

			if(pLevel)
			{
				data.nCoolDownTime = pLevel->nCooldownTimes; //冷却时间
			}
			else
			{
				continue;
			}
			if(data.nCd  > nCurrentTime)
			{
				data.tick = tick + (data.nCd  - nCurrentTime) * 1000; //这个是新的冷却时间
				
			}
			else
			{
				data.tick =0;
				data.nCd  =0;
				
			}
			data.nLastDbCd = data.nCd;

			if(data.nMijiId) //如果镶嵌了秘籍
			{
				bool isInvalid = false;
				if(data.nMijiExpiredTime  > GetGlobalLogicEngine()->getMiniDateTime()) //如果秘籍还有效的话
				{
					const CStdItem*  pStdItem = itemProvider.GetStdItem(data.nMijiId);
					if(pStdItem )
					{
						isInvalid =true;
						data.bMjMask = (BYTE)pStdItem->m_wSuitID; //这个是真正生效的秘籍的效果
					}
				}
				if(!isInvalid)
				{
					data.nMijiId =0;
					data.nMijiExpiredTime =0; //已经过期了
				}
			}

			data.bIsPassive = ((pConfig->nSkillType == stPassiveSkill)?1:0);
			data.bPriority = pConfig->bPriority;
			data.nSkillType = pConfig->nSkillType;
			OutputMsg(rmTip,_T("OnCrossInitData CSkillSubSystem nActorID:%d nSkillID:%d,nLevel:%d"),
			m_pEntity->GetId(),data.nSkillID, data.nLevel);
			m_skillList.add(data);
		}
		
	}
	Inherid::OnDbInitData();
	//存储记录并发提示
	SetDataModifyFlag(true);
	((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CSKILL); //完成一个步骤
}