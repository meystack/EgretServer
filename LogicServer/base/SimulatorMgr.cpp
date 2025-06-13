#include "StdAfx.h"
#include "SimulatorMgr.h"
#include "Container.hpp" 

CSimulatorMgr::CSimulatorMgr()
{
	m_InitTestActor = false;
}

BOOL CSimulatorMgr::Initialize()
{
	return TRUE;
}

CSimulatorMgr::~CSimulatorMgr()
{
	Destroy();
}
bool CSimulatorMgr::CheckTestActorId()
{
	return m_InitTestActor;
}
void CSimulatorMgr::DoTestActorId()
{
	m_InitTestActor = true;
}
VOID CSimulatorMgr::Destroy()
{
	//释放资源
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif
#endif 
	{	 
		OutputMsg(rmTip, "remove CSimulatorMgr all sucess!"  );
		m_RankActorVec.clear();
		m_RuningActorMap.clear();
	}
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
	}	
#endif 
#endif 
} 
void CSimulatorMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{  
} 
 
VOID CSimulatorMgr::RunOne(TICKCOUNT currTick)
{ 
	if (!GetLogicServer()->GetDataProvider()->GetEditionProvider().GetCanSimulor())
	{
		return;
	} 
	int SimulorSize = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetSimulorSize();
	if (SimulorSize <= 0)
	{
		return;
	}
	if (m_RefreshRank.CheckAndSet(currTick))
	{
		OutputMsg( rmTip, "[CSimulatorMgr] refresh data by rank !"); 
		CRanking* pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(0);
		while(1)
		{ 
			if (!pRank)//找玛法榜 人数多
			{
				break;
			}
			
			int nItemCount = pRank->GetList().count();
			if (nItemCount <= 0)
			{
				break;
			}
			m_RankActorVec.clear();
			// 排行榜数据
			for (size_t i = 0; i < nItemCount; i++)
			{
				CRankingItem * pItem = pRank->GetItem(i);
				if (!pItem)
				{
					break;
				}
				int nActorid = (unsigned int)pItem->GetId(); 
				m_RankActorVec.push_back(nActorid);
				// if (!m_RankActorMap.insert(std::make_pair(nActorid, nActorid)).second)
				// { 
				// 	OutputMsg( rmTip, "[CSimulatorMgr] RankActorMap重复加入!"); 
				// } 
			}
			
			break;
		}
	} 
	
	std::map<int, int> RankActorMap; 
	int count = 0;
	for (size_t i = 0; i < m_RankActorVec.size(); i++)
	{
		if (count >= SimulorSize)
		{
			break;
		}
		
		if (!RankActorMap.insert(std::make_pair(m_RankActorVec[i], m_RankActorVec[i])).second)
		{ 
			OutputMsg( rmTip, "[CSimulatorMgr] RankActorMap重复加入!"); 
			continue;
		} 
		count ++;
	}
	
	if (m_RefreshSimu.CheckAndSet(currTick))
	{
		int run_size = m_RuningActorMap.size();
	
		OutputMsg(rmTip, _T("[simulator], ---------------------------------加入计数的玩家数量 = %d"), run_size);

		//先删除排行榜没有的玩家
		std::map<int, int>::iterator iter_del = m_RuningActorMap.begin();
		for (; iter_del != m_RuningActorMap.end(); ++iter_del)
		{ 
			std::map<int, int>::iterator iterFind = RankActorMap.find(iter_del->first);
			
			if(iterFind != RankActorMap.end())
			{
				continue;
			}
			
			CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(iter_del->first);
			if (!pActor)
			{ 
				continue;
			}
			
			pActor->CloseActor(lwiSimulatorSameActorLogin, false); 
			
			//虚拟玩家下线　
			OutputMsg(rmTip, _T("[simulator], 排行榜玩家 下线 actorID = %u, Actorname = %s"), pActor->GetId(), pActor->GetEntityName());
		
		}

		std::map<int, int>::iterator iter = RankActorMap.begin();
		for (; iter != RankActorMap.end(); ++iter)
		{
			if (m_RuningActorMap.size() >= SimulorSize)
			{
				break;//数量上限
			}
			
			std::map<int, int>::iterator iterFind = m_RuningActorMap.find(iter->first);
			
			CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(iter->first);
			if (pActor)
			{ 
				continue;
			}
			if(iterFind != m_RuningActorMap.end())
			{
				continue;
			}

			std::map<int, int>::iterator iterFind2 = m_RuningCSActorMap.find(iter->first);
			
			if(iterFind2 != m_RuningCSActorMap.end())
			{
				continue;
			}
			
			CEntityManager *pMgr = GetGlobalLogicEngine()->GetEntityMgr();
			//再创建一个实体 
			EntityHandle hHandle;
			pActor = (CActor*) pMgr->CreateEntity(enActor, hHandle);
			if(pActor == NULL) //实体创建失败的话，要通知这个
			{
				OutputMsg(rmError, _T("[CSimulatorMgr] create actor entity failed")); 
			}
			else
			{					
				pActor->SetSimulator(true);//标记是模拟用户
				pActor->SetGateKey(0);
				pActor->SetCreateIndex(0); 
				int nServerId = GetLogicServer()->GetServerIndex();  
				pActor->OnServerLogin(iter->first, (unsigned int)16778032, -1, 0, nServerId, (LONGLONG)2371610996); 
			}  
			if (!m_RuningActorMap.insert(std::make_pair(iter->first, iter->first)).second)
			{ 
				OutputMsg( rmTip, "[CSimulatorMgr] RuningActorMap 重复加入!"); 
			} 
			break;
		} 
	}
}

//添加跨服的玩家
void CSimulatorMgr::OnAddRuningCsActor(int actorId)
{  
	if (!m_RuningCSActorMap.insert(std::make_pair(actorId, actorId)).second)
	{ 
		OutputMsg( rmTip, "[CSimulatorMgr] AddRuningCsActor 添加重复的跨服 玩家 %d!", actorId); 
	}  
}
 void CSimulatorMgr::OnRemoveRuningCsActor(int actorId)
{  
	std::map<int, int>::iterator iter = m_RuningCSActorMap.find(actorId);
	if (iter != m_RuningCSActorMap.end())
	{
		iter = m_RuningCSActorMap.erase(iter); 
	}  
}
//删除自动管理里的玩家
void CSimulatorMgr::OnRemoveRuningActor(int actorId)
{ 
	std::map<int, int>::iterator iter = m_RuningActorMap.find(actorId);
	if (iter != m_RuningActorMap.end())
	{
		iter = m_RuningActorMap.erase(iter); 
		
		OutputMsg(rmTip, _T("[simulator], ----------delete-----------------------加入计数的玩家数量 = %d"), m_RuningActorMap.size());
	} 
}

//获得随机坐标
void CSimulatorMgr::OnGetRandSimuPos(int& SenceId, int& posX, int& posY)
{ 
	if (!GetLogicServer()->GetDataProvider()->GetEditionProvider().GetCanSimulor())
	{
		return;
	} 
	int SimulorSize = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetSimulorSize();
	if (SimulorSize <= 0)
	{
		return;
	}
	
	std::vector<SimulorMapPosInfo>& info = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetSimulorInfo();
	int index = wrandvalue() % info.size();
	SimulorMapPosInfo &temp = info[index];
	SenceId = temp.nSimulorMapSceneId; 
	int nRandDestPosX = wrandvalue() % (temp.nSimulorMaxPosX - temp.nSimulorMinPosX);
	int nRandDestPosY = wrandvalue() % (temp.nSimulorMaxPosY - temp.nSimulorMinPosY);
	posX = temp.nSimulorMinPosX + nRandDestPosX;
	posY = temp.nSimulorMinPosY + nRandDestPosY;
}