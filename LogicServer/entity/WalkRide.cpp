
// #include "StdAfx.h"

// CUpdateMask * CWalkRide::s_pWalkRideMask = NULL;

// VOID CWalkRide::LogicRun( TICKCOUNT nCurrentTime )
// {
// 	Inherited::LogicRun(nCurrentTime);
// }

// void CWalkRide::InitWalkRideBroadcastMask()
// {
// 	if (!s_pWalkRideMask)
// 	{
// 		s_pWalkRideMask = new CUpdateMask;
// 	}
// 	s_pWalkRideMask->SetUpdateMaskFlag(PROP_CREATURE_MOVEONESLOTTIME);
// }

// void CWalkRide::DestroyWalkRideBroadcastMask()
// {
// 	SafeDelete(s_pWalkRideMask);
// }

// void CWalkRide::InitAi(int nAiId)
// {
// 	if (nAiId > 0)
// 	{
// 		m_moveSystem.SetUseAstar(true); //使用A*
// 		m_pAI = GetGlobalLogicEngine()->GetAiMgr().CreateAi(nAiId,(CAnimal*)this);
// 	}
// }

