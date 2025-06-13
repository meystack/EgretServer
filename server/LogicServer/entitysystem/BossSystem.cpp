

#include "StdAfx.h"
#include "BossSystem.h"

CBossSystem::CBossSystem()
{
}

CBossSystem::~CBossSystem()
{

}

void CBossSystem::OnEnterGame()
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


	SendSZBossTimes(); 
}

bool CBossSystem::Initialize(void *data,SIZE_T size)
{
	return true;
}

VOID CBossSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	switch(nCmd)
	{
		case cRequestBossInfo:
			RequestBossInfo(packet);
			break;
		
		case cTeleToBoss:
		{
			int nBossId = 0;
			int nTab = 0;
			packet >> nTab>> nBossId;
			int nKey = nTab*BOSSSOCAIL_MUTIPLY+ nBossId;
			TeleportToBoss(nKey);
			break;
		}
	}
}

VOID CBossSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
}

VOID CBossSystem::Save(PACTORDBDATA pData)
{

}


VOID CBossSystem::RequestBossInfo(CDataPacketReader & packet)
{
	if(!m_pEntity) return;

	CBossManager& bossMgr = GetGlobalLogicEngine()->GetBossMgr();
	CMonsterProvider& monsterCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig();

	std::map<int,BOSSCONFIGINFO>& bossCfg = monsterCfg.GetBossInfoList();

	if(bossCfg.size() == 0)
		return;

	CActorPacket ap;
	int nCount = 0;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(ap);
	DataPacket << (BYTE)enBossSystemID <<(BYTE)sSendBossInfo;
	int pos = DataPacket.getPosition();
	DataPacket << nCount;

	int nDays = GetLogicServer()->GetDaysSinceOpenServer();

	std::map<int,BOSSCONFIGINFO>::iterator it = bossCfg.begin();
	for(; it!= bossCfg.end(); it++)
	{

		BOSSCONFIGINFO& cfg = it->second;
		if(cfg.nIsOpen == 0)
			continue;
		bool canShow = false;
		if(cfg.nTab != 1)
		{
			if (bossMgr.CheckCondition(cfg.nOpenCircle, cfg.nShowDay))
				canShow = true;
		}else
		{
			if (GetLogicServer()->CheckOpenServer(cfg.nShowDay))
				canShow = true;
		}
		
		if(canShow)
		{
			DataPacket << cfg.nSerial;
			int nReferTime = 0;
			int nId = cfg.nBossId* BOSS_ID_MUTIPLY + cfg.nMapId;
			bool bossLive = false; //boss 是否死亡
			if(cfg.nReborn != 0)
			{
				if(bossMgr.GetBossInfo(it->first))
				{
					if(bossMgr.GetBossInfoIsDeath(cfg.nBossId))
						nReferTime = bossMgr.GetBossMinReferTime(cfg.nBossId);
					else
						bossLive = true;
					
				}
				else
				{
					int nleftday = cfg.nOpenDay - nDays;
					if(nleftday > 0)
						nReferTime = GetGlobalLogicEngine()->getMiniDateTime() + nleftday*24*3600 + 1000; //+1000 让客户端处理 判断是一天
				}
			}
			else
			{
				nId = cfg.nSerial*BOSSSOCAIL_MUTIPLY;
			}
			DataPacket << nReferTime;
			int nLimitTimes = 0;
			if(cfg.nDaliyLimit < 0)
			{
				//处理动态副本逻辑  当前计数器表示可进总数 
				if(cfg.nDalitStaticId != 0)
				{
					nLimitTimes = m_pEntity->GetStaticCountSystem().GetStaticCount(cfg.nDalitStaticId);
				}
				else
				{
					nLimitTimes = -1; //没有计数器限制 表示没限制
				}
			}
			else
			{
				int ntimes = 0;
				if(cfg.nDalitStaticId != 0)
					ntimes = m_pEntity->GetStaticCountSystem().GetStaticCount(cfg.nDalitStaticId);
				
				nLimitTimes = monsterCfg.GetBossTeleCfgTimes(nId) - ntimes;
				if(nLimitTimes < 0) nLimitTimes = 0;
			}
			DataPacket << nLimitTimes;
			nCount++;
			if (bossLive)
			{
				if(cfg.nFubenId > 0) {
					static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
					CScriptValueList paramList;
					paramList.clear();
					int nMonsterType = 1;

					PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(cfg.nBossId);
					if(pMonsterConfig)
					{
						nMonsterType = pMonsterConfig->btMonsterType;
					}
					paramList << (int)nMonsterType;
					paramList << (int)CBossSystem::enOnReqData;
			
					paramList << (int)cfg.nFubenId;
					paramList << (int)cfg.nMapId;
					paramList << (int)cfg.nBossId;
					paramList << &ap;
					if (!globalNpc->GetScript().CallModule("MonDispatcher", "OnEvent", paramList, paramList))
					{
						OutputMsg(rmError,"[MonDispatcher] OnEvent错误 enOnExitArea");
					}
				}
				else
				{
					DataPacket << (int)0;
					DataPacket << (int)0;
				}
				
				// if(cfg.nFubenId > 0)
				// 	GetGlobalLogicEngine()->GetBossMgr().BossCallLuaScript(it->first, CBossSystem::enOnReqData, NULL, &DataPacket);
				
					
			}
			else
			{
				DataPacket << (int)0;
				DataPacket << (int)0;
			}
		}
	}
	int* pInt = (int*)(DataPacket.getPositionPtr(pos));
	*pInt = nCount;
	ap.flush();
}

void CBossSystem::BossFubenOpt(CDataPacketReader & packet)
{
	/*
	byte nOptType, nFubenId = 0;
	packet >> nOptType >> nFubenId;
	CScriptValueList paramList;
	((CActor*)m_pEntity)->InitEventScriptVaueList(paramList, aeBossFubenOpt);
	paramList << nOptType;
	paramList << nFubenId;
	m_pEntity->OnEvent(paramList,paramList);
	*/
}


int CBossSystem::GetMonsGrowInfo(int nMonsterId, int & nGrowLv, int & nCurExp)
{
	// PMONSTERCONFIG pMonConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
	// if (NULL == pMonConfig || !pMonConfig->pflags->CanGrowUp)
	// {
	// 	return 0;
	// }
	// if (pMonConfig->nLevel < pMonConfig->nOriginalLevel)
	// {
	// 	pMonConfig->nLevel = pMonConfig->nOriginalLevel;
	// }
	// if (pMonConfig->nMaxDeadTimes == 0)
	// {
	// 	int nDis = pMonConfig->nLevel - pMonConfig->nOriginalLevel;
	// 	pMonConfig->nMaxDeadTimes =(int)(pMonConfig->fLevelA * (float)(nDis * nDis) + pMonConfig->fLevelB * (float)(nDis) + pMonConfig->fLevelC);
	// }
	// nGrowLv = pMonConfig->nLevel - pMonConfig->nOriginalLevel, nCurExp = 0;
	// int nMaxExp = 0;
	// if(pMonConfig->nMaxHardLv > 0 && (pMonConfig->nOriginalLevel +  pMonConfig->nMaxHardLv) > pMonConfig->nLevel )
	// {
	// 	nMaxExp = pMonConfig->nMaxDeadTimes;
	// 	nCurExp = pMonConfig->nDeadTimes;
	// }
	// return nMaxExp;
}

/*击杀成长BOSS
某些BOSS随着被击杀的次数增加，等级需要增加

*/
int CBossSystem::OnKillGrowBoss(int nMonsterId, LPCTSTR sName, PMONSTERCONFIG pConfig)
{
	// int nGrowLv = 0;
	// if (pConfig == NULL)
	// {
	// 	pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
	// }
	// if (pConfig == NULL)
	// {
	// 	return nGrowLv;
	// }
	// //当前等级和初始的等级
	// int nCurrentLevel = pConfig->nLevel;
	// int nOriginalLevel =pConfig->nOriginalLevel;

	// if(nCurrentLevel < nOriginalLevel)
	// {
	// 	pConfig->nLevel = nOriginalLevel;
	// }
	// nGrowLv = pConfig->nLevel - nOriginalLevel;
	// if (pConfig->nMaxDeadTimes == 0)
	// {
	// 	pConfig->nMaxDeadTimes = (int)( pConfig->fLevelA * (float)(nGrowLv * nGrowLv) + pConfig->fLevelB * (float)(nGrowLv) + pConfig->fLevelC);
	// }

	// //nMaxHardLv大于0，才能
	// if(pConfig->nMaxHardLv > 0 && (nOriginalLevel +  pConfig->nMaxHardLv) >nCurrentLevel )
	// {
	// 	pConfig->nDeadTimes ++;
	// 	if (pConfig->nDeadTimes >= pConfig->nMaxDeadTimes)
	// 	{
	// 		pConfig->nDeadTimes = 0;
	// 		int nNewLevel = nGrowLv + 1;
	// 		if(nNewLevel > pConfig->nMaxHardLv)
	// 		{
	// 			pConfig->nMaxDeadTimes = -1;
	// 		}
	// 		else
	// 		{
	// 			pConfig->nLevel ++;
	// 			pConfig->nMaxDeadTimes = (int)( pConfig->fLevelA * (float)(nNewLevel * nNewLevel) + pConfig->fLevelB * (float)(nNewLevel) + pConfig->fLevelC);
	// 		}
	// 	}
	// 	GetLogicServer()->GetDataProvider()->GetMonsterConfig().SetBossGrowFlag(true);		//设置是否进行了成长
	// }

	// /* CMonsterProvider::BOSSINFO 已经不使用了
	// CMonsterProvider::BOSSINFO * pOneBoss = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(nMonsterId);
	// if (pOneBoss != NULL)
	// {
	// 	INT_PTR nDiffCount = pOneBoss->KillList.count() - MAXBOSSGROWROW;
	// 	if (nDiffCount > 0 )
	// 	{
	// 		for (INT_PTR i = 0; i < nDiffCount; i++)
	// 		{
	// 			pOneBoss->KillList.remove(i);
	// 		}
	// 	}
	// 	CMonsterProvider::BOSSFLUSHTIME flushTime;
	// 	flushTime.nTime = GetGlobalLogicEngine()->getMiniDateTime();
	// 	_asncpytA(flushTime.sName, sName);
	// 	pOneBoss->KillList.add(flushTime);
	// }
	// */


	// return nGrowLv;
}

void CBossSystem::RequestOneBossInfo( CDataPacketReader & packet )
{
	// WORD nBossId = 0;
	// packet >> nBossId;
	// CActorPacket pack;
	// CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// netPack << (byte)GetSystemID() << (byte)sSendOneBossInfo;
	// netPack << nBossId;
	// CMonsterProvider & monsterPdr = GetLogicServer()->GetDataProvider()->GetMonsterConfig();
	// CMonsterProvider::BOSSCONFIGINFO * pOneBoss = monsterPdr.GetBossInfoById(nBossId);
	// if (pOneBoss != NULL)
	// {
	// 	INT_PTR nKillCount = pOneBoss->KillList.count();
	// 	netPack << (byte)nKillCount;
	// 	for (INT_PTR i = 0; i < nKillCount; i++)
	// 	{
	// 		CMonsterProvider::BOSSFLUSHTIME & flushTime = pOneBoss->KillList[i];
	// 		netPack << flushTime.nTime;
	// 		netPack.writeString(flushTime.sName);
	// 	}
	// }
	// else
	// {
	// 	netPack << (byte)0;
	// }
	// pack.flush();
}

void CBossSystem::SendWildBossSts()
{
	// if( !m_pEntity )
	// {
	// 	return;
	// }

	// CActor* pActor = (CActor*)m_pEntity;
	// CMonsterProvider & monsterProvider = GetLogicServer()->GetDataProvider()->GetMonsterConfig();
	// CVector<CMonsterProvider::WILDBOSS>& wildBossList = monsterProvider.GetWildBossList();

	// CActorPacket pack;
	// CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// netPack << (byte)enBossSystemID << (byte)sSendWildBossSts;
	// netPack << (short)wildBossList.count();
	// for( INT_PTR i=0; i<wildBossList.count(); i++ )
	// {
	// 	CMonsterProvider::WILDBOSS &stBoss = wildBossList[i];
	// 	CScene* pScene = GetGlobalLogicEngine()->GetMiscMgr().GetScenePtrById(stBoss.nSceneId);
	// 	if( pScene )
	// 	{
	// 		int nBossNum = pScene->GetMonsterCount(stBoss.nBossId,0);
	// 		netPack << (int)stBoss.nBossId;
	// 		char nSts = 0;
	// 		if( nBossNum > 0 )		//BOSS尚未被击杀
	// 		{
	// 			nSts = 1;
	// 			netPack << (char)nSts;
	// 		}
	// 		else
	// 		{
	// 			nSts = 0;
	// 			netPack << (char)nSts;
	// 		}
	// 		netPack << (unsigned int) monsterProvider.GetBossLastKillTime( stBoss.nBossId );
	// 		//OutputMsg(rmNormal,"SendWildBossSts, BossId=%d, nSts=%d", stBoss.nBossId, nSts);
	// 	}
	// }
	// pack.flush();
}

void CBossSystem::SendWorldBossSts()
{
	// if( !m_pEntity )
	// {
	// 	return;
	// }

	// CActor* pActor = (CActor*)m_pEntity;
	// CVector<CMonsterProvider::WORLDBOSS>& worldBossList = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetWorldBossList();

	// CActorPacket pack;
	// CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// netPack << (byte)enBossSystemID << (byte)sSendWorldBossSts;
	// netPack << (short)worldBossList.count();
	// for( INT_PTR i=0; i<worldBossList.count(); i++ )
	// {
	// 	CMonsterProvider::WORLDBOSS &stBoss = worldBossList[i];
	// 	CScene* pScene = GetGlobalLogicEngine()->GetMiscMgr().GetScenePtrById(stBoss.nSceneId);
	// 	if( pScene )
	// 	{
	// 		int nBossNum = pScene->GetMonsterCount(stBoss.nBossId,0);
	// 		netPack << (int)stBoss.nBossId;
	// 		if( nBossNum > 0 )		//BOSS尚未被击杀
	// 		{
	// 			netPack << (char)1;
	// 		}
	// 		else
	// 		{
	// 			netPack << (char)0;
	// 		}
	// 		//OutputMsg(rmNormal,"SendWildBossSts, BossId=%d", stBoss.nBossId);
	// 	}
	// }
	// pack.flush();
}

/*下发某个BOSS的最后一次被击杀的时间
此BOSS的recordKillTime要配置为true
*/
void CBossSystem::SendBossLastKillTime( WORD nBossId )
{
// 	unsigned int nBossKillTime = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossLastKillTime(nBossId);
// 	if( !m_pEntity )
// 	{
// 		return;
// 	}

// 	CActor* pActor = (CActor*)m_pEntity;
// 	CActorPacket pack;
// 	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
// 	netPack << (byte)enBossSystemID << (byte)sSendBossLastKillTime;
// 	netPack << (int)nBossId;
// 	netPack << (unsigned int)nBossKillTime;
// 	pack.flush();
}

void CBossSystem::AnheiBossInfo(unsigned char nIndex)
{
	// if( !m_pEntity )
	// {
	// 	return;
	// }
	// CActor* pActor = (CActor*)m_pEntity;
	// GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor, "SendAnheiBoss", nIndex);
}

void CBossSystem::TeleportToBoss(int nBossId)
{
	if( !m_pEntity )
	{
		return;
	}
	int nErrorCode = 0;
	int nLeftTimes = 0;
	do
	{
		CActor* pActor = (CActor*)m_pEntity;
		
		BOSSCONFIGINFO* pOneBoss = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetBossInfoById(nBossId);
		if(!pOneBoss)
		{
			m_pEntity->SendTipmsgFormatWithId(tmDataError, tstUI);
			nErrorCode = 1;
			break;
		}

		if(pOneBoss->nDelivery == 0)
		{
			m_pEntity->SendTipmsgFormatWithId(tmMapLimitTeleport, tstUI);
			nErrorCode = 10;
			break;
		}

		CBossManager& bossMgr = GetGlobalLogicEngine()->GetBossMgr();
		if(!bossMgr.CheckCondition(pOneBoss->nOpenCircle, pOneBoss->nOpenDay))
		{
			m_pEntity->SendTipmsgFormatWithId(tmNoOpen, tstUI);
			return;
		}
		if(!((CActor*)m_pEntity)->CheckLevel(pOneBoss->nLevellimit, pOneBoss->nBerebornlimit))
		{
			m_pEntity->SendTipmsgFormatWithId(tmLevelLimited, tstUI);
			return;
		}
		if(((CActor*)m_pEntity)->GetMiscSystem().GetMaxColorCardLevel() < pOneBoss->nVip)
		{
			m_pEntity->SendTipmsgFormatWithId(pOneBoss->nTips, tstUI);
			return;
		}

		if(bossMgr.GetBossInfoIsDeath(pOneBoss->nBossId) && pOneBoss->nNodeliver == 0)
		{
			m_pEntity->SendTipmsgFormatWithId(tmSZBossDeath, tstUI);
			return;
		}

		int nLimitTimes = 0;
		if(pOneBoss->nDalitStaticId != 0)
			nLimitTimes = m_pEntity->GetStaticCountSystem().GetStaticCount(pOneBoss->nDalitStaticId);
		
		//处理静态副本逻辑  当前计数器表示已经进入总数
		if(pOneBoss->nDaliyLimit > 0 && nLimitTimes >= pOneBoss->nDaliyLimit)
		{
			pActor->SendTipmsgFormatWithId(tmNoTimes, tstUI);
			nErrorCode = 3;
			break;
		}
		//处理动态副本逻辑  当前计数器表示可进总数 
		if(pOneBoss->nDalitStaticId && pOneBoss->nDaliyLimit < 0 && nLimitTimes <= 0)
		{
			pActor->SendTipmsgFormatWithId(tmNoTimes, tstUI);
			nErrorCode = 3;
			break;
		}
		
		int nMonsterType = 1;
        PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(pOneBoss->nBossId);
		if(pMonsterConfig)
        {
            nMonsterType = pMonsterConfig->btMonsterType;
        }
        CScriptValueList paramList,retList;
        paramList << (int)nMonsterType;//(int)EnBossEvent::enOnCancel;
		paramList << (int)enCheckEnterFuben;//(int)EnBossEvent::enCheckEnterFuben;
		paramList << (int)pOneBoss->nFubenId;
		paramList << (int)pOneBoss->nMapId;
		paramList << (int)nBossId;
		paramList << pActor;
        //次数检查
		if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("MonDispatcher","OnEvent", paramList, retList, 1))
		{
			OutputMsg(rmError,"[MonDispatcher] OnEvent nType 错误，页签类型ID=%d，场景=%d， bossId= %d  !",pOneBoss->nTab,pOneBoss->nMapId, nBossId);
		}

		if(retList.count() >= 0 && (bool)retList[0] == false)
		{
			// 条件不满足
			nErrorCode = 10;
			break;
		}

		if(pOneBoss->nCosts.size() > 0)
		{
			for(int i = 0; i < pOneBoss->nCosts.size(); i++)
			{
				KILLBOSSCOST& cost = pOneBoss->nCosts[i];
				if(!pActor->CheckActorSource(cost.nType, cost.nId, cost.nCount, tstUI))
				{
					nErrorCode = 2;
					break;
				}
			}
			
			if(nErrorCode)
				break;

		}
		int nId = pOneBoss->nBossId*BOSS_ID_MUTIPLY + pOneBoss->nMapId;
		if(pOneBoss->nReborn == 0)
		{
			nId = pOneBoss->nSerial*BOSSSOCAIL_MUTIPLY;
		}
		bool success = false;
		if(pOneBoss->nFubenId > 0)
		{
			if(m_pEntity->GetScene() && m_pEntity->GetScene()->GetFuBen()->IsFb())
			{
				pActor->SendTipmsgFormatWithId(tmInFuben, tstUI);
				nErrorCode = 4;
				break;
			}
			// CScene* pScene = NULL;
			// CFuBen * pRetFb =NULL;
			// GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneId(pOneBoss->nFubenId, pOneBoss->nMapId, pRetFb, pScene);
			// //如果原来在副本中，则不做处理
			// CFuBen* pDyFb = m_pEntity->GetFuBen();
			// CScene* pThisScene = m_pEntity->GetScene();
			// if(pRetFb && pRetFb->Enter(m_pEntity,pOneBoss->nMapId,pOneBoss->nDeliveryX, pOneBoss->nDeliveryY,0,0,0,0,-1,pThisScene,pDyFb))
			// {
			// 	success = true;
			// }
			paramList.clear();
			retList.clear();
			paramList << (int)nMonsterType;//
			paramList << (int)enReqEnterFuben;//
			paramList << pActor;
			paramList << (int)pOneBoss->nFubenId;
			paramList << (int)pOneBoss->nSerial;
			//次数检查
			if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("MonDispatcher","OnEvent", paramList, retList, 1))
			{
				OutputMsg(rmError,"[MonDispatcher] OnEvent nType 错误，页签类型ID=%d，场景=%d， bossId= %d  !",pOneBoss->nTab,pOneBoss->nMapId, nBossId);
			}
			if(retList.count() >= 0 && retList[0])
			{
					success = true;
			}
		}
		else
		{
			int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
			if( ((CActor*)m_pEntity)->FlyShoseToStaticScene(pOneBoss->nEndmap, pOneBoss->nDeliveryX, pOneBoss->nDeliveryY, pOneBoss->nDeliveryRange, pOneBoss->nDeliveryRange,enDefaultTelePort,nEffId))
			{
				success = true;
				EntityHandle handle = ((CActor*)m_pEntity)->GetHandle();
				CEntity *pEntity = GetEntityFromHandle(handle);
				if(pEntity) {
					((CAnimal*)pEntity)->GetMoveSystem()->nFlyShoeNpcId = pOneBoss->nNpcId;
				}
			}
		}
		
		
		if(success)
		{
			if(pOneBoss->nCosts.size() > 0)
			{
				for(int i = 0; i < pOneBoss->nCosts.size(); i++)
				{
					KILLBOSSCOST& cost = pOneBoss->nCosts[i];
					pActor->RemoveConsume(cost.nType, cost.nId, cost.nCount, -1, -1,-1,0, GameLog::Log_BossSystem);
				}
			}
			if(pOneBoss->nJiontime)
			{	
				//禁地
				if(!pOneBoss->nMapId)
					pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveJoinJDFuben, 1);
				CScene* pScene = NULL;
				CFuBen * pRetFb =NULL;
				GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneId(pOneBoss->nFubenId, pOneBoss->nEndmap, pRetFb, pScene);
				int nMapGroup = 0;
				if(pScene) {
					SCENECONFIG* pdata = pScene->GetSceneData();
					if(pdata)
						nMapGroup = pdata->nMapGgroup;
				}
				// ((CActor*)m_pEntity)->SetJDJoinTime(pOneBoss->nJiontime);
				((CActor*)m_pEntity)->SetJDJoinMap(pOneBoss->nEndmap,pOneBoss->nJiontime ,nMapGroup);
			}
			//boss 非动态副本算累加
			if (pOneBoss->nDalitStaticId != 0 && !pOneBoss->nNoexpel)
				((CActor*)m_pEntity)->GetStaticCountSystem().SetStaticCount(pOneBoss->nDalitStaticId, nLimitTimes+1);
			
		}

		if(pOneBoss->nDaliyLimit < 0)
		{
			//处理动态副本逻辑  当前计数器表示可进总数 
			if(pOneBoss->nDalitStaticId != 0)
			{
				nLeftTimes = m_pEntity->GetStaticCountSystem().GetStaticCount(pOneBoss->nDalitStaticId);
			}
		}
		
	}
	while(false);

	CActorPacket ap;
	CDataPacket & data = m_pEntity->AllocPacket(ap);
	data<<(BYTE)GetSystemID() << (BYTE)sTeleToBoss;
	data<<(BYTE)nErrorCode;
	data<<(unsigned int)(((CActor*)m_pEntity)->GetJDJoinTime());
	data<<(int)nLeftTimes;
	ap.flush();
	
}



// void CBossSystem::SendBossBeLong(int nBossId)
// {
// 	if(!m_pEntity) return;

// 	CActorPacket ap;
	
// 	CDataPacket & data = ((CActor*)m_pEntity)->AllocPacket(ap);
// 	data<<(BYTE)GetSystemId() <<(BYTE)sBossBelong;
// 	ap.flush();

// }

void CBossSystem::SendSZBossTimes(int nLevel)
{
	if(!m_pEntity) return;
	// int nLv = 0;
	// int nDay = 0;
	// int nId = -1;
	// if (nType == 1)
	// {
	// 	nLv = GetLogicServer()->GetDataProvider()->GetMonsterConfig().nOpenlevel;
	// 	nDay = GetLogicServer()->GetDataProvider()->GetMonsterConfig().nOpenday;
	// 	nId = GetLogicServer()->GetDataProvider()->GetMonsterConfig().nId;
	// }else
	// {
	// 	nLv = GetLogicServer()->GetDataProvider()->GetMonsterConfig().nPersonOpenlevel;
	// 	nDay = GetLogicServer()->GetDataProvider()->GetMonsterConfig().nPersonOpenday;
	// 	nId = GetLogicServer()->GetDataProvider()->GetMonsterConfig().nPersonId;
	// }
	std::vector<BossOpenConfig>& cfgs = GetLogicServer()->GetDataProvider()->GetMonsterConfig().m_openCfgs;
	for(int i = 0; i < cfgs.size(); i++)
	{
		BossOpenConfig& cfg = cfgs[i];
		if(nLevel == 0)	
		{
			if(!((CActor*)m_pEntity)->CheckLevel(cfg.nOpenlevel, 0)) continue;
		}
		else 
		{
			if(nLevel != cfg.nOpenlevel)
				continue;
		}
		if(GetLogicServer()->GetDaysSinceOpenServer() < cfg.nOpenday)
			continue;
		
		CActorPacket pack;
		CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(pack);
		data << (BYTE)GetSystemID()<< (BYTE)sBossTimes << (BYTE)i;
		int nTimes = m_pEntity->GetStaticCountSystem().GetStaticCount(cfg.nId);
		data <<(int)nTimes;
		pack.flush();
	}
	
}
