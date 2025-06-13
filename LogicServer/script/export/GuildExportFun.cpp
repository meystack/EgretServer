#include "StdAfx.h"
#include "../interface/GuildExportFun.h"

namespace Guild
{
	
	char* getGuildName(unsigned int nGuidID)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuidID);
		if(NULL ==pGuild )
		{
			return NULL;
		}
		else
		{
			return pGuild->m_sGuildname;
		}
	}

	void* getGuildLeader( unsigned int nGuildID )
	{
		// CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildID);
		// if(NULL ==pGuild )
		// {
		// 	return NULL;
		// }
		// else
		// {
		// 	if (!pGuild->m_nLeaderId) return NULL;
		// 	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pGuild->m_pLeader->hActorHandle);
		// 	if (pEntity && pEntity->GetType() == enActor)
		// 	{
		// 		return pEntity;
		// 	}
		// }
		return NULL;
	}

	unsigned int getGuildLeaderId( unsigned int nGuildID )
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildID);
		if(NULL ==pGuild )
		{
			return 0;
		}
		else
		{
			return pGuild->m_nLeaderId;
			
		}
		return 0;
	}

	bool changeGuildFr( unsigned int nGuildId, int nFrVar )
	{
		CGuild *pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			pGuild->SetGuildFr(nFrVar);
			return true;
		}
		return false;
	}

	bool changeGuildCoin( unsigned int nGuildId, int nCoinVar, int nLogId,const char* pStr)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			pGuild->ChangeGuildCoin(nCoinVar, nLogId,pStr);
			return true;
		}
		return false;
	}

	const char* getGuildLeaderName( unsigned int nGuildId )
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			if(pGuild->m_nLeaderId)
			{
				return pGuild->FindGuildMemberName(pGuild->m_nLeaderId);
			}
			else
			{
				return NULL;
			}
		}
		return NULL;
	}
	
	int getGuildMemberNum(unsigned int nGuildId)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			return (int)(pGuild->GetMemberCount());
		}

		return 0;
	}

	unsigned int getCityOwnMasterId()
	{
		return GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnMasterId();
	}

	


	// 震天龙弹
	unsigned int getThunderPower(unsigned int nGuildId)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			return pGuild->m_nThunderPower;
		}
		return 0;
	}

	void setThunderPower(unsigned int nGuildId, unsigned int nValue)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			pGuild->m_nThunderPower = nValue;
			pGuild->m_boUpdateTime = true;
		}
	}

	unsigned int getFillThunderTimes(unsigned int nGuildId)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			return pGuild->m_nFillThunderTimes;
		}
		return 0;
	}

	void setFillThunderTimes(unsigned int nGuildId, unsigned int nValue)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			pGuild->m_nFillThunderTimes = nValue;
			pGuild->m_boUpdateTime = true;
		}
	}
	
	bool isGuildSiegeStart()
	{
		return GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart();
	}

	void autoSignGuildSiege()
	{
		 GetGlobalLogicEngine()->GetGuildMgr().AutoSignGuildSiege();
	}

	//下发行会战指挥面板到全体参战的成员
	void sendGuildSiegeCmdPanel(unsigned char nFlag)
	{
		 GetGlobalLogicEngine()->GetGuildMgr().SendGuildSiegeCmdPanel(nFlag);
	}

	//会长下发指令
	void setGuildSiegeCommand(void* pEntity, int nGuildId, int nCmdIdx, int nCmdMsgIdx)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			pGuild->SetGuildSiegeCommand((CActor*)pEntity, nCmdIdx, nCmdMsgIdx);
		}
	}

	unsigned int getGuildSiegeCommandTime(int nGuildId)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			return pGuild->GetGuildSiegeCommandTime();
		}
		return 0;
	}

	void resetSignGuildsiege()
	{
		GetGlobalLogicEngine()->GetGuildMgr().ResetSignGuildsiege();
	}

	void clearGuildSiegeCommand()
	{
		GetGlobalLogicEngine()->GetGuildMgr().ClearGuildSiegeCommand();
	}

	bool isGuildSiegeTodayOpen()
	{
		return GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeTodayOpen();
	}
	void startGuildSiege()
	{
		GetGlobalLogicEngine()->GetGuildMgr().StartGuildSiege();
	}
	void guildSiegeEnd(unsigned int nGuildId)
	{
		GetGlobalLogicEngine()->GetGuildMgr().GuildSiegeEnd(nGuildId);
	}
	
	char* getCityPostionInfo(int nPos,int &nJob,int &nSex)
	{
		return GetGlobalLogicEngine()->GetGuildMgr().GetCityPostionInfo(nPos,nJob,nSex);
	}
	void clearGuildSiegeData()
	{
		GetGlobalLogicEngine()->GetGuildMgr().ClearGuildSiegeData();
	}

	bool isLimitGuildCoin(unsigned int nGuildId)
	{
		CGuildComponent& guildMgr = GetGlobalLogicEngine()->GetGuildMgr();
		CGuild* pGuild = guildMgr.FindGuild(nGuildId);
		if (!pGuild)
		{
			return false;
		}
		return false;
	}
	

	void sendGuildMsg(unsigned int nGuildId, char* szMsg,int nLevel, int nCircle)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		pGuild->BroadCastMsgInGuildChannel(szMsg, nLevel, nCircle);
	}


	void getUpgradeItemCount(unsigned int nGuildId, int& nItemCount1,int& nItemCount2, int& nItemCount3)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			nItemCount1 = -1;
			nItemCount2 = -1;
			nItemCount3 = -1;
			return;
		}
		return pGuild->GetUpgradeItemCount(nItemCount1, nItemCount2, nItemCount3);
	}

	void setUpgradeItemCount(unsigned int nGuildId, int nItemCount1,int nItemCount2, int nItemCount3)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		pGuild->SetUpgradeItemCount(nItemCount1, nItemCount2, nItemCount3);
	}

	void getGuildTreeData(unsigned int nGuildId, int &nTreeLevel, int &nTreeDegree)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		return pGuild->GetGuildTreeData(nTreeLevel, nTreeDegree);
	}

	void setGuildTreeData(unsigned int nGuildId, int nTreeLevel, int nTreeDegree)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		pGuild->SetGuildTreeData(nTreeLevel, nTreeDegree);
	}

	unsigned int getGuildTreeFruitTime(unsigned int nGuildId)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return 0;
		}
		return pGuild->GetGuildTreeFruitTime();
	}

	void setGuildTreeFruitTime(unsigned int nGuildId, unsigned int nFruitTime)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		pGuild->SetGuildTreeFruitTime(nFruitTime);
	}

	void getGuildFruitData(unsigned int nGuildId, int &nGuildFruitLevel, int &nGuildFruitDegree, int &nGuildFruitNum)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		return pGuild->GetGuildFruitData(nGuildFruitLevel, nGuildFruitDegree, nGuildFruitNum);
	}

	void setGuildFruitData(unsigned int nGuildId, int nGuildFruitLevel, int nGuildFruitDegree, int nGuildFruitNum)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		pGuild->SetGuildFruitData(nGuildFruitLevel, nGuildFruitDegree, nGuildFruitNum);
	}
	

	void* getGuildPtr(unsigned int nGuildId)
	{
		return GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
	}
	void getGuildMemberBasicData(void* pGuild, unsigned int nActorId, int &nSex, int &nJob)
	{
		if (!pGuild)
		{
			return;
		}
		((CGuild*)pGuild)->GetGuildMemberBasicData(nActorId, nSex, nJob);
	}
	int getGuildMemberIdList(lua_State* L)
	{
		CGuild* pGuild = (CGuild*)lua_touserdata(L, 1);
		CVector<unsigned int> pIdList;
		pIdList.clear();
		if (pGuild)
		{
			int nSize = pGuild->m_ActorOffLine.size();
			for(int i = 0; i < nSize; i++)
			{
				ActorCommonNode& Node = pGuild->m_ActorOffLine[i];
				pIdList.add(Node.BasicData.nActorId);
			}
			if (pIdList.count() > 0)
			{
				LuaHelp::PushNumberVector(L,&(pIdList[0]),pIdList.count());
			}
			else
			{
				return 0;
			}
		}
		return 1;
	}
	int getGuildOnlineMemberList( lua_State *L )
	{
		CGuild* pGuild = (CGuild*)lua_touserdata(L, 1);
		CVector<void*> pEntityList;
		pEntityList.clear();
		if (pGuild)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(pGuild->m_ActorOnLine);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				EntityHandle& hHandle = pNode->m_Data;
				CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
				if (pEntity && pEntity->GetType() == enActor)
				{
					pEntityList.add((void*)pEntity);
				}
			}
			if (pEntityList.count() > 0)
			{
				LuaHelp::PushDataPointerToTable(L,(void**)pEntityList,pEntityList.count());
			}
			else
			{
				LuaHelp::PushDataPointerToTable(L,NULL,0);
			}

		}
		return 1;
	}

	void setCityOwnGuildId(unsigned int nGuildId)
	{
		GetGlobalLogicEngine()->GetGuildMgr().SetCityOwnGuildId(nGuildId);
	}

	//获取皇城职位
	unsigned int getCityPostionById(unsigned int nActorId)
	{
		return GetGlobalLogicEngine()->GetGuildMgr().getCityPostionById(nActorId);
	}

	unsigned int getCityOwnGuildId()
 	{
		return GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId();
	}
	unsigned int getPalaceOccoupyTime()
	{
		return 0;//GetGlobalLogicEngine()->GetGuildMgr().m_OccupyTime;
	}



	void procGuildTreeToSapling()
	{
		GetGlobalLogicEngine()->GetGuildMgr().ProcGuildTreeToSapling();
	}

	void procGuildDailyData()
	{
		GetGlobalLogicEngine()->GetGuildMgr().ProcDailyData();
	}


	void getGuildTaskData(unsigned int nGuildId, int &nTaskId, int &nTaskSche)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		pGuild->GetGuildTaskData(nTaskId, nTaskSche);
	}

	int getDailyGuildCoinDonated(unsigned int nGuildId)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return 0;
		}
		return pGuild->GetDailyGuildCoinDonated();
	}

	void addDailyGuildCoinDonated(unsigned int nGuildId, int nAddGuildCoin)
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
		pGuild->AddDailyGuildCoinDonated(nAddGuildCoin);
	}

	void guildBroadCastMsg(void* packet, void* guildPtr, int nLevel, int nCircle)
	{
		if (packet && guildPtr)
		{
			CGuild* pGuild = (CGuild*)guildPtr;
			CActorPacket* pPack = (CActorPacket *)packet;
			pGuild->BroadCast(pPack->packet->getMemoryPtr(), pPack->packet->getPosition(), nLevel, nCircle);
		}
	}

	void sendMemberList(void* pEntity, unsigned int nActorId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CActor* pActor = (CActor*)pEntity;
		pActor->GetGuildSystem()->SendMemberList(nActorId);
	}

	void setSbkGuildId(unsigned int nGuildId)
	{
		int guildId = GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId();
		if(guildId != nGuildId) 
			GetGlobalLogicEngine()->GetGuildMgr().SetCityOwnGuildId(nGuildId);
	}
};
