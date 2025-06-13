
// #include "StdAfx.h"

// void CWalkRideAI::UpdateAI( TICKCOUNT nCurrentTick )
// {
// 	Inherid::UpdateAI(nCurrentTick);
// 	CActor * pActor = ((CWalkRide *)m_pSelf)->GetMaster();
// 	if(pActor == NULL)
// 	{
// 		return;
// 	}
// 	INT_PTR nDis = m_pSelf->GetEntityDistanceSquare(pActor);
// 	if (nDis >= 20 && m_pSelf->GetSceneID() != pActor->GetSceneID())
// 	{
// 		m_pSelf->MoveToEntity(pActor);
// 	}
// 	else
// 	{
// 		m_pSelf->GetMoveSystem()->MoveFollow(pActor);
// 	}
// }

// void CWalkRideAI::initialize( PAICONFIG pAiconfig )
// {

// }

