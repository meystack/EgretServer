
#include "StdAfx.h"
#include "HandleMgr.h"
#include "TeamManager.h"

bool CTeamManager::Initialize()
{
	//CTeam::m_nMaxTeamLootDistanceSquare = GetLogicServer()->GetDataProvider()->GetGlobalConfig().teamLootMaxDistanceSquare;
	return true;
}
 
void CTeamManager::updateTeamMemFubenAveragePlayerLvl(unsigned int nTeamID)
{
	//CTeam *pTeam = GetTeam(nTeamID);
	//if (!pTeam)
	//	return;

	//CFuBen *pFB = 0;
	//int nTotalLvl = 0, nTeamMemCount = 0;	
	//const TEAMMEMBER* pMember=  pTeam->GetMemberList();
	//for(int i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
	//{
	//	if(pMember[i].pActor) 
	//	{
	//		CScene* pScene = pMember[i].pActor->GetScene();
	//		if (pScene)
	//		{
	//			CFuBen *pFuBen = pScene->GetFuBen();
	//			if (pFuBen && pFuBen->IsFb())
	//			{
	//				if (!pFB)
	//					pFB = pFuBen;

	//				// 只计算副本中队伍成员的平均等级
	//				nTeamMemCount++;
	//				nTotalLvl += pMember[i].pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	//			}
	//		}		
	//	}
	//}
	//int nAverageLvl = 0;
	//if (nTeamMemCount > 0)
	//	nAverageLvl = nTotalLvl / nTeamMemCount;
	//if (pFB)
	//{
	//	pFB->updateFubenPlayerAverageLvl(nAverageLvl);
	//	pFB->updateFubenPlayerCount(nTeamMemCount);
	//	OutputMsg(rmNormal, _T("--- update fb player info: averageLevel=%d playercount=%d"), nAverageLvl, nTeamMemCount);
	//}
}

void CTeamManager::RunOne( TICKCOUNT curTick )
{
	if(m_timer.CheckAndSet(curTick))
	{
		TeamHandle hIndex;
		CTeam* pTeam = m_mgr.First(hIndex);
		while(pTeam != NULL)
		{
			pTeam->RunOne();			
			pTeam = m_mgr.Next(hIndex);
		}		
	}
}
