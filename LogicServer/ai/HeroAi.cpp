#include "StdAfx.h"

void CHeroAi::UpdateAI(TICKCOUNT tick)
{
	Inherid::UpdateAI(tick);
	CActor *pActor= ((CHero*)m_pSelf)->GetMaster(); //获取宠物的主人
	if(pActor ==NULL) return;
	INT_PTR  nDisSquare = m_pSelf->GetEntityDistanceSquare( pActor); //获取距离的平方
	//如果宠物在战斗中，并且距离超过了一定的值，那么将直接向
	if( m_pSelf->HasState(esStateBattle) )
	{
		if(nDisSquare >= m_heroBattleFollowDis  || pActor->GetScene() != m_pSelf->GetScene())
		{
			EnterEvadeMode();//退出战斗
			if(pActor->GetScene() != m_pSelf->GetScene())
			{
				m_pSelf->MoveToEntity(pActor); //直接传送到那个地方
			}else{
				m_pSelf->GetMoveSystem()->MoveFollow(pActor );
			}
			
		}

		CScene* pActorScene = pActor->GetScene();
		int  x = 0,y = 0;
		pActor->GetPosition(x,y);
		CScene* pHeroScene = m_pSelf->GetScene();
		int  x1 = 0, y1 = 0;
		m_pSelf->GetPosition(x1,y1); 
		if (pActorScene && pActorScene->HasMapAttribute(x,y,aaSaft) && pHeroScene && !pHeroScene->HasMapAttribute(x1,y1,aaSaft) )
		{ 
			EnterEvadeMode();//退出战斗
			m_pSelf->GetMoveSystem()->MoveFollow(pActor );
		}
	}
	else
	{
		//if(m_followMasterTime > 0 &&  m_followMasterTime <  GetLogicCurrTickCount()) return;	
		//如果和主人的距离过大，传送到主人的身边 
		if(nDisSquare >= m_heroTransferDisSquare || pActor->GetScene() != m_pSelf->GetScene()) 
		{
			m_pSelf->MoveToEntity(pActor); //直接传送到那个地方
		}else  if( m_pSelf->GetMoveSystem()->GetMotionType() !=  CMovementSystem::mtMotionTypeFollow)
		{
			m_pSelf->GetMoveSystem()->MoveFollow(pActor );
		}
	}
}


void CHeroAi::initialize(PAICONFIG pAiconfig)
{
	//是否是一个宠物的ai
	Inherid::initialize(pAiconfig);

	if(m_pSelf->GetType() != enHero)
	{
		OutputMsg( rmError,_T("Hero ai init error,not a Hero"));
	}
	//初始化这3个属性
	m_heroBattleFollowDis = pAiconfig->bBattleFollowDis *  pAiconfig->bBattleFollowDis ;
	m_heroTransferDisSquare = pAiconfig->bTransferDis * pAiconfig->bTransferDis ;
	m_followMasterTime =0;
}

void CHeroAi::EnterEvadeMode()
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
