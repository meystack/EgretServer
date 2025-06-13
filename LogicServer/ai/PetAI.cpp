#include "StdAfx.h"

void CPetAi::UpdateAI(TICKCOUNT tick)
{
	CPet *pPet = ((CPet*)m_pSelf);
	CActor *pActor= pPet->GetMaster(); //获取宠物的主人
	if(pActor ==NULL) return;
	//如果宝宝是停留状态，那么就不移动了
	if(pPet->GetBattleStatus() == CPet::PET_BATTLE_STATUE_STAY)
	{
		return;
	}

	Inherid::UpdateAI(tick);

	if(m_t1minute.CheckAndSet(tick, true))
	{
		m_avoidMap.clear();
	}

	INT_PTR  nDisSquare = m_pSelf->GetEntityDistanceSquare( pActor); //获取距离的平方

	//如果宠物在战斗中，并且距离超过了一定的值，那么将直接向
	if( m_pSelf->HasState(esStateBattle)  )
	{
		if(nDisSquare >= m_petTransferDisSquare || pActor->GetScene() != m_pSelf->GetScene()) 
		{
			m_pSelf->MoveToEntity(pActor); //直接传送到那个地方
			if ( m_pSelf->GetMoveSystem()->GetMotionType() != CMovementSystem::mtMotionTypeFollow )
			{
				m_pSelf->GetMoveSystem()->MoveFollow(pActor,0);
			}
		}
		/*		
		if(nDisSquare >= m_petBattleFollowDis )
		{
			EnterEvadeMode();//推出战斗
			if( m_pSelf->GetMoveSystem()->GetMotionType() !=  CMovementSystem::mtMotionTypeFollow)
			{
				m_pSelf->GetMoveSystem()->MoveFollow(pActor);
			}
		}	
		*/
	}
	else
	{
		if(m_followMasterTime <= GetLogicCurrTickCount())
		{
			//如果和主人的距离过大，传送到主人的身边 
			if(nDisSquare >= m_petTransferDisSquare || pActor->GetScene() != m_pSelf->GetScene()) 
			{
				m_pSelf->MoveToEntity(pActor,0,0,1); //直接传送到那个地方
				if ( m_pSelf->GetMoveSystem()->GetMotionType() != CMovementSystem::mtMotionTypeFollow )
				{
					m_pSelf->GetMoveSystem()->MoveFollow(pActor,0);
				}
			}
			else
			{
				if( m_pSelf->GetMoveSystem()->GetMotionType() != CMovementSystem::mtMotionTypeFollow )
				{
					m_pSelf->GetMoveSystem()->MoveFollow(pActor,0);
				}
			}
		}
		
		
	}
	
}

void  CPetAi::OnChangeStayToFollow()
{
	CPet *pPet = ((CPet*)m_pSelf);
	CActor *pActor= pPet->GetMaster(); //获取宠物的主人
	if(pActor ==NULL) return;
	INT_PTR  nDisSquare = m_pSelf->GetEntityDistanceSquare( pActor); //获取距离的平方
	//如果和主人的距离过大，传送到主人的身边 
	if(nDisSquare >= m_petTransferDisSquare || pActor->GetScene() != m_pSelf->GetScene()) 
	{
		m_pSelf->MoveToEntity(pActor); //直接传送到那个地方
	}
	if( m_pSelf->HasState(esStateBattle)  )
	{
		if(nDisSquare >= m_petBattleFollowDis )
		{
			EnterEvadeMode();//推出战斗
			m_pSelf->GetMoveSystem()->MoveFollow(pActor,0);
		}
	}
	else
	{
		if( m_pSelf->GetMoveSystem()->GetMotionType() !=  CMovementSystem::mtMotionTypeFollow)
		{
			m_pSelf->GetMoveSystem()->MoveFollow(pActor,0);
		}
	}
}



void CPetAi::initialize(PAICONFIG pAiconfig)
 {
	 //是否是一个宠物的ai
	 Inherid::initialize(pAiconfig);

	 if(m_pSelf->GetType() != enPet)
	 {
		OutputMsg( rmError,_T("Pet ai init error,not a pet"));
	 }
	 //初始化这3个属性
	 m_petBattleFollowDis = pAiconfig->bBattleFollowDis *  pAiconfig->bBattleFollowDis ;
	 m_petTransferDisSquare = pAiconfig->bTransferDis * pAiconfig->bTransferDis ;
	 m_followMasterTime =0;
	 m_avoidMap.clear();
	 m_t1minute.Reset();
 }
void CPetAi::EnterEvadeMode()
 {
	 if( m_pSelf->HasState(esStateDeath) ) return; //死了不回归
	 m_pSelf->RemoveState(esStateBattle); //删除战斗模式
	 m_pSelf->SetTarget(NULL);
	 if (m_pSelf->GetMoveSystem()->GetMotionType() == CMovementSystem::mtMotionTypeChase)
	 {
		 m_pSelf->GetMoveSystem()->PopCurrentMotionState();
	 }
	 m_followMasterTime =GetLogicCurrTickCount() + m_pAiConfig->petFollowMasterTime; //脱离战斗3秒以后将走向主人
 }

void CPetAi::AttackedBy(CAnimal * pAttacker, bool boInherit)
 {
	 CPet *pPet = (CPet*)m_pSelf;
	 if (pAttacker == m_pSelf) return;
	 if (pAttacker == pPet->GetMaster()) return;	

	if(IsInAvoidMap(pAttacker->GetHandle())) return;

	 if(pPet->GetBattleStatus() == CPet::PET_BATTLE_STATUE_STAY)
	 {
		 return;
	 }
	Inherid::AttackedBy(pAttacker);
	/*
	CPet *pSelfPet = ((CPet*)m_pSelf);
	if (!boInherit)
	{
		return;
	}
	INT_PTR nSelfId = pSelfPet->GetPetId();
	CActor *pActor= pSelfPet->GetMaster(); //获取宠物的主人
	if(pActor ==NULL) return;
	CPetSystem & petSystem =pActor->GetPetSystem();
	INT_PTR nPetCount= petSystem.GetPetCount();
	//if(nPetCount <=1) return; //只有一个宠物
	CPetSystem::PETDATA *pPetData =NULL;
	//如果一个宠物被攻击要通知他的伙伴
	for(INT_PTR i=0; i< nPetCount; i++)
	{
		pPetData =  petSystem.GetPetData(i);
		if(pPetData && pPetData->nId != nSelfId)
		{
			CPet *pPet =(CPet*)GetEntityFromHandle(pPetData->handle);
			if(pPet && pPet != pSelfPet && pPet->GetLiveTime() == 0)
			{
				if(!pPet->HasState(esStateBattle))
				{
					pPet->GetAI()->AttackedBy(pAttacker, false);
				}
			}
		}
	}
	*/
 }
