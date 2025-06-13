#include "StdAfx.h"
#include "PetSystem.h"


CPetSystem::~CPetSystem()
{
	Destroy();
}


VOID CPetSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity ==NULL) return;
	if(m_pEntity->GetHandle().GetType() != enActor) return; 
	bool hasSendTipmsg =false;
	int nCurrentStatus =0;
	if(nCmd == cPetSetBattleState) //改变宠物的出战状态
	{
		CPet * pPet;
		unsigned char nStatus = 0;
		packet >> nStatus;

		for(INT_PTR i=0; i< m_pets.count(); i++)
		{
			pPet= (CPet*)GetEntityFromHandle(m_pets[i].handle);
			if(pPet != NULL)//&& pPet->GetLiveTime() == 0)
			{
				nCurrentStatus = pPet->GetBattleStatus();
				if(nCurrentStatus == CPet::PET_BATTLE_STATUE_STAY && nStatus == CPet::PET_BATTLE_STATUE_FOLLOW)
				{
					pPet->SetBattleStatus(CPet::PET_BATTLE_STATUE_FOLLOW); //设置移动状态
				}
				else if(nCurrentStatus == CPet::PET_BATTLE_STATUE_FOLLOW && nStatus == CPet::PET_BATTLE_STATUE_STAY)
				{
					pPet->SetTarget(0);
					pPet->SetBattleStatus(CPet::PET_BATTLE_STATUE_STAY);
					if ( pPet->GetMoveSystem()->GetMotionType() == CMovementSystem::mtMotionTypeFollow )
					{
						pPet->GetMoveSystem()->MoveFollow(m_pEntity,1);
					}
				}
			}	
		}
	}
	else if(nCmd == cRecallPet)
	{
		RecallPet();
	}
	else if(nCmd == cRemoveBattlePet)
	{
		RemoveBattlePet();
	}
}
void CPetSystem::RecallPet()
{
	CPet * pPet = NULL;
	INT_PTR nPox = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nPoy = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	INT_PTR nDir = (m_pEntity->GetProperty<int>(PROP_ENTITY_DIR) + 4) & 7; //玩家反方向
	INT_PTR nNewPox = nPox, nNewPoy = nPoy;
	CEntityGrid::NewPosition(nNewPox,nNewPoy,nDir,1);
	CFuBen * pFb = m_pEntity->GetFuBen();     //副本的指针
	CScene * pScene = m_pEntity->GetScene(); //场景指针
	if (!pScene) return;

	for (INT_PTR i = 0; i < m_pets.count(); i++)
	{
		pPet = (CPet *)GetEntityFromHandle(m_pets[i].handle);
		if (pPet != NULL)// && pPet->GetLiveTime() == 0)
		{
			pPet->SetTarget(0);
			CFuBen * pLasfb = pPet->GetFuBen();
			CScene * pLastScene = pPet->GetScene();
			bool result = pFb->Enter(pPet,pScene->GetSceneId(),nNewPox,nNewPoy,0,0,0,0,-1,pLastScene,pLasfb);
			if (!result)
			{
				pFb->Enter(pPet,pScene->GetSceneId(),nPox,nPoy,0,0,0,0,-1,pLastScene,pLasfb);
			}
			
			//pPet->SetBattleStatus(CPet::PET_BATTLE_STATUE_FOLLOW);
			// if ( pPet->GetMoveSystem()->GetMotionType() != CMovementSystem::mtMotionTypeFollow )
			// {
			// 	pPet->GetMoveSystem()->MoveFollow(m_pEntity);
			// }
			// if (pPet->MoveToEntity(m_pEntity,0,0,1) == false)
			// 	if (pPet->MoveToEntity(m_pEntity,0,0,2) == false)
			// 		if (pPet->MoveToEntity(m_pEntity,0,0,3) == false)
			// 			pPet->MoveToEntity(m_pEntity,0,0,0);
		}
	}
}
void CPetSystem::RemoveBattlePet()
{
	if (m_pEntity == NULL || m_pEntity->GetType() != enActor || ((CActor*)m_pEntity)->IsDeath())
	{
		return;
	}
	Destroy();
}

//攻击别人
void CPetSystem::OnAttackOther(CAnimal * pEntity)
{
	if (!pEntity || pEntity->IsDeath())
	{
		return;
	}
	CPet * pPet;
	for(INT_PTR i=0; i< m_pets.count(); i++)
	{
		pPet= (CPet*)GetEntityFromHandle(m_pets[i].handle);
		if ( pPet )
		{
			CEntity* pTarget = GetEntityFromHandle(pPet->GetTarget());
			if ( pPet->GetAI() && //pPet->GetBattleStatus() != CPet::PET_BATTLE_STATUE_STAY && 
				 //pPet->GetLiveTime() == 0 &&
				 (!pTarget || pTarget->GetHandle() != pEntity->GetHandle()) )
			{
				pPet->GetAI()->AttackedBy(pEntity);
			}
		}
	}
}

//被攻击
void CPetSystem::OnAttacked(CAnimal *pEntity)
{
	if (!pEntity || pEntity->IsDeath())
	{
		return;
	}
	CPet * pPet;
	for(INT_PTR i=0; i< m_pets.count(); i++)
	{
		pPet= (CPet*)GetEntityFromHandle(m_pets[i].handle);
		if ( pPet )
		{
			CEntity* pTarget = GetEntityFromHandle(pPet->GetTarget());
			if ( pPet->GetAI() &&// pPet->GetBattleStatus() != CPet::PET_BATTLE_STATUE_STAY && 
				 //pPet->GetLiveTime() == 0 &&
				 (!pTarget || pTarget->GetHandle() != pEntity->GetHandle()))
			{
				pPet->GetAI()->AttackedBy(pEntity);
			}
		}
	}
}

//死亡
void CPetSystem::OnDeath()
{
	Destroy(); //把所有的宠物干掉
}

//Runtime
VOID CPetSystem::OnTimeCheck( TICKCOUNT curTick )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	if(m_10Sec.CheckAndSet(curTick, true))
	{
		unsigned int nNow = GetGlobalLogicEngine()->getMiniDateTime();
		INT_PTR nLen = m_pets.count();
		for (INT_PTR i = nLen -1; i >=0; i--)
		{
			CEntity *pEntity =GetEntityFromHandle(m_pets[i].handle); //实体的句柄
			if(pEntity && !((CPet*)pEntity)->IsLiveTime(nNow))
			{
				char sPetName[128];
				strncpy(sPetName, pEntity->GetEntityName(), sizeof(sPetName));
				if(RemovePet(m_pets[i].nId))
				{
					char sText[1024] = {0};
					LPCTSTR sNotice = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpPetRemove);
					sprintf_s(sText, sizeof(sText), sNotice, sPetName);
					((CActor*)m_pEntity)->SendTipmsg(sText, ttFlyTip);
				}
			}
		}
	}
}


bool CPetSystem::AddPet(INT_PTR  PetId,INT_PTR nLevel, INT_PTR nAiId,INT_PTR nLiveSecs, INT_PTR nInheritPercent,INT_PTR nSkillId,int nTarPosX,int nTarPosY)
{
	CScene *pScene = m_pEntity->GetScene();
	if(pScene ==NULL) return false;

	const PMONSTERCONFIG pConfig =  GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(PetId);
	if(pConfig ==NULL)
	{
		OutputMsg(rmError,"no pet id=%d",PetId);
		return false;
	}
	 
	if (m_pEntity->HasState(esStateDeath))
	{
		OutputMsg(rmNormal, _T("在创建道士宠物时，玩家是死亡状态"));
		return false;
	}
	INT_PTR nJobCount = GetJobPetCount();
	if (nLiveSecs == 0 && nJobCount >= 1)
	{
		OutputMsg(rmNormal, _T("这里手动判断道士宠物是否大于2个，目前来看已大于2个啦"));
		return false;
	}
	 
	//一共48个，从里边到外边遍历，找到一个合适的位置
	static int s_matrix[][2] =
	{
		// 原点
		{0,0},

		//第1圈8个
		
		{0,-1},
		{0,-2},
		{0,-3},
		{-1,-1},
		{-1,-2},
		{-1,-3},
		{1,-1},
		{1,-2},
		{1,-3},

		//第2圈16个
		{-2,-1},
		{-2,-2},
		{-2,-3},
		{2,-1},
		{2,-2},
		{2,-3},

		{-1,0},
		{-2,0},
		{1,0},
		{2,0},

		{0,1},
		{-1,-1},
		{-2,-1},

		{1,1},
		{2,1},

		{0,2},

		//第3圈24个
		{-1,-2},
		{-2,-2},
		{1,2},
		{2,2},

		{0,3},
		{-1,-3},
		{-3,-2},

		{1,3},
		{2,3},
		{0,0},
	};

	int nPosX,nPosY;
	m_pEntity->GetPosition(nPosX,nPosY); //获取玩家当前的位置
	if (nTarPosX && nTarPosY)
	{
		nPosX = nTarPosX;
		nPosY = nTarPosY;
	}
	
	//优先刷在自己的前面，如果自己的前面有怪物了，则刷在旁边的一个点

	INT_PTR nNewRelX,nNewRealY; //相对坐标x,y
	int nDir = m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);

	

	//计算出玩家正前方一个点新坐标
	//在周围找一个可以刷的地方
	INT_PTR   nDX, nDY;

	//这里先轮一次，如果旁边48个格子都有了，那么进行第2次遍历，不过这种情况是比较少的
	
	for(INT_PTR j=0; j< ArrayCount(s_matrix); j++)
	{
		CSkillSubSystem::PositionRotation( 0, 0,s_matrix[j][0], s_matrix[j][1], nDir, nNewRelX, nNewRealY);

		nDX = nPosX + nNewRelX;
		nDY = nPosY + nNewRealY;
		if(pScene->CanCross(enPet,nDX,nDY,false,false))
		{
			goto lb_mob;
		}
	}

	return false;

lb_mob:

	unsigned int nId = GenPetId(); //产生一个宠物的id
	//创建一个宠物
	CEntity *pEntity = pScene->CreateEntityAndEnterScene(pConfig->nEntityId,enPet,nDX,nDY,nLevel,m_pEntity);
	if(pEntity ==NULL) return false;
	CPet *pPet = (CPet*) pEntity;

	char sAllName[512];
	_asncpytA( sAllName, pConfig->szName);
	// if(pEntity->GetAttriFlag().ShowNameByNumber == false)//宠物也根据配置，不显示最后的数字
	// {
	// 	INT_PTR nLen = strlen(sAllName);
	// 	if (nLen > 2)
	// 	{
	// 		if (sAllName[nLen-1] >='0' && sAllName[nLen-1] <='9')
	// 		{
	// 			sAllName[nLen-1]=0;
	// 		}
	// 		if (sAllName[nLen - 2] >= '0' && sAllName[nLen - 2] <='9')
	// 		{
	// 			sAllName[nLen - 2] = 0;
	// 		}
	// 	}
	// }
	//sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",sAllName, m_pEntity->GetEntityName());
	// pPet->ChangeName(sAllName);
	pPet->SetPetId(nId);
	pEntity->SetEntityName(sAllName);
	pEntity->SetProperty<int>(PROP_ENTITY_MODELID,pConfig->nModelId);
	pEntity->SetProperty<int>(PROP_ENTITY_ICON, 1);
	//pEntity->SetProperty<int>(PROP_CREATURE_LEVEL,nLevel);
	pPet->SetMaster(m_pEntity);

	pPet->SetInheritPercent((int)nInheritPercent);
	pPet->GetPropertySystem().ResertProperty();//要计算人物继承属性，所以设置master之后需要重新计算

	pPet->InitAi((int)nAiId);
	if(NULL == pPet->GetAI())
	{
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(pEntity->GetHandle(),false);
		return false; 
	}

	pEntity->SetInitFlag(true);//如果是创建备用的，这个设置成false
	 
	if( pPet->GetAI())
	pPet->GetAI()->RefreshSkillDis(); //重新刷新技能 
	//pPet->GetAttriFlag().DenyUseNearAttack =true; //宠物都不肉搏
	pPet->SetLiveTime((int)nLiveSecs);
	int nAttackSpd = pPet->GetProperty<int>(PROP_CREATURE_ATTACK_SPEED);
	if(nAttackSpd > 0)
	{
		pPet->SetAttackInterval(nAttackSpd);
	}
	if (nLiveSecs == 0 && !m_pEntity->HasState(esStateOwnPet))
	{
		m_pEntity->AddState(esStateOwnPet);
	}
	CSkillSubSystem & skillSystem =  pPet->GetSkillSystem();
	const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();
	INT_PTR nCount = skills.count();
	for(INT_PTR i=0; i< nCount;i++)
	{
		CSkillSubSystem::SKILLDATA *pSkill = &skills[i];
		if(pSkill->bEvent == mSkillEventBorn)
		{
			skillSystem.LaunchSkill(pSkill->nSkillID);
		}
	}
	 
	//获得宠物的升级经验
	int nLevelUpExp =GetLogicServer()->GetDataProvider()->GetPetProvider().GetMaxExp(nLevel,0);  

	PETDATA data;
	data.nConfigId = (int)pConfig->nEntityId;

	data.nId = nId; //宠物的ID
	data.handle = pEntity->GetHandle();
	data.nSkillId = (int)nSkillId;
	m_pets.add(data);
	SendPetCount();
	int nStatus = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_PET_STATUS);
	pPet->SetBattleStatus(nStatus);
	SendPetSkillCountChange(data.nConfigId, data.nSkillId);
	return true;

}

void CPetSystem::Destroy()
{
	RemoveAllPets();
}

void CPetSystem::SendPetCount()
{
	if (m_pEntity && m_pEntity->IsInited())
	{
		CActorPacket pack;
		CDataPacket &data=((CActor*)m_pEntity)->AllocPacket(pack);
		data << (BYTE)(enPetSystemID) << (BYTE)sPetCount << (BYTE)GetPetCount();
		pack.flush();
	}
	
}

void CPetSystem::RemoveAllPets()
{
	INT_PTR nCount = m_pets.count();
	for (INT_PTR i = nCount - 1; i > -1; --i)
	{
		RemovePetByPos(i);
	}
	m_pets.clear();
	SendPetCount();
}

void CPetSystem::OnActorLoginOut()
{
	GetGlobalLogicEngine()->GetEntityMgr()->CachePet(m_pEntity,&m_pets,10*60*1000);
}

void CPetSystem::OnEnterGame()
{
	GetGlobalLogicEngine()->GetEntityMgr()->UnCachePet(m_pEntity,&m_pets,m_pEntity->GetScene());
}

void CPetSystem::OnEnterScene()
{ 
	int nActorType = m_pEntity->GetHandle().GetType();
	if (nActorType == enActor)
	{
		if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
			|| ((CActor*)m_pEntity)->OnGetIsSimulator())
		{
			return; 
		} 
	}
	SendPetCount();
}

unsigned int CPetSystem::GenPetId()
{
	unsigned int ID=0;
	for(INT_PTR i=0; i< m_pets.count();i ++)
	{
		if(m_pets[i].nId > ID)
		{
			ID = m_pets[i].nId;
		}
	}
	ID ++;
	return ID;
}

void CPetSystem::SendPetSkillCountChange(int nPetConfigId, int nSkillId)
{
	//改用技能通知,用来客户端挂机检查
	INT_PTR nCount = GetPetCountByConfigId((int)nPetConfigId);
	m_pEntity->GetSkillSystem().SendPetSkillCountChange(nSkillId,nCount);
}
void CPetSystem::RemovePetByPos(INT_PTR nPos)
{
	PPETDATA pPetData = GetPetData(nPos);
	if (!pPetData)
	{
		return;
	}
	EntityHandle petHandle = pPetData->handle;
	int nPetConfigId = pPetData->nConfigId;
	int nSkillId = pPetData->nSkillId;
	m_pets.remove(nPos);//注意pPetData指向已经变了,不能直接用
	CEntity *pEntity =GetEntityFromHandle(petHandle); //实体的句柄
	if(pEntity)
	{
		if (pEntity->GetType() == enPet)
		{
			SendPetSkillCountChange(nPetConfigId, nSkillId);
		}
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(petHandle, true); //玩家下线的时候要把自己创建的宠物删除
	}
	
	if (m_pets.count() == 0 && m_pEntity->HasState(esStateOwnPet))
	{
		m_pEntity->RemoveState(esStateOwnPet);
	}
	
	SendPetCount();
}
bool CPetSystem::RemovePet(int nPetId)
{
	if (m_pEntity == NULL || m_pEntity->GetType() != enActor || ((CActor*)m_pEntity)->IsDeath())
	{
		return false;
	}
	INT_PTR nPos = GetPetPos(nPetId) ;
	if (nPos <0) return false;
	RemovePetByPos(nPos);
	return true;
}

INT_PTR CPetSystem::RemoveAllPetByConfigId(int nConfigId)
{
	if (m_pEntity == NULL || m_pEntity->GetType() != enActor || ((CActor*)m_pEntity)->IsDeath())
	{
		return 0;
	}
	INT_PTR nRemoveCount = 0;
	for (INT_PTR i = m_pets.count() - 1; i > -1; --i)
	{
		PETDATA &data = m_pets[i];
		if (data.nConfigId != nConfigId)
		{
			continue;
		}
		RemovePetByPos(i);
		nRemoveCount++;
	}
	
	SendPetCount();
	return nRemoveCount;
}

int CPetSystem::GetPetCountByConfigId( int nPetId )
{
	int nCount = 0;
	for(INT_PTR i=0; i< m_pets.count();i ++)
	{
		if(m_pets[i].nConfigId == nPetId)
		{
			nCount++;
		}
	}
	return nCount;
}

int CPetSystem::GetPetCountBySkillId(int nSkillId)
{
	int nCount = 0;
	for(INT_PTR i=0; i< m_pets.count();i ++)
	{
		if(m_pets[i].nSkillId == nSkillId)
		{
			nCount++;
		}
	}
	return nCount;
}

int CPetSystem::GetJobPetCount()
{
	int nCount = 0;
	CPet * pPet = NULL;
	INT_PTR nAllPetCount = m_pets.count();
	for (INT_PTR i = 0; i < nAllPetCount; i++)
	{
		pPet = (CPet *)GetEntityFromHandle(m_pets[i].handle);
		if (pPet != NULL && pPet->GetLiveTime() == 0)
		{
			nCount++;
		}
	}
	return nCount;
}

CPet *CPetSystem::GetTopJobPet()
{
	CPet *pPet = NULL, *pRetPet = NULL;
	INT_PTR nAllPetCount = m_pets.count();
	INT_PTR nCurLevel = 0, nMaxLevel = 0;
	for (INT_PTR i = 0; i < nAllPetCount; i++)
	{
		pPet = (CPet *)GetEntityFromHandle(m_pets[i].handle);
		if (pPet == NULL || pPet->GetLiveTime() != 0)
		{
			continue;
		}

		nCurLevel = pPet->GetProperty<int>(PROP_CREATURE_LEVEL);
		if (nCurLevel > nMaxLevel)
		{
			nMaxLevel = nCurLevel;
			pRetPet = pPet;
		}
	}

	return pRetPet;
}

bool CPetSystem::IsMyJobPet( CPet * pOutPet )
{
	INT_PTR nAllPetCount = m_pets.count();
	for (INT_PTR i = 0; i < nAllPetCount; i++)
	{
		CPet * pPet = (CPet *)GetEntityFromHandle(m_pets[i].handle);
		if (pPet == NULL || pPet->GetLiveTime() != 0)
		{
			continue;
		}
		if (pOutPet == pPet )
		{
			return true;
		}
	}
	return false;
}
