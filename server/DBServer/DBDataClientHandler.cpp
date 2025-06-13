#include "StdAfx.h"
#include "SysApi.h"

int CDBDataClientHandler::s_nDBHandleSleep = 6;

CDBDataClientHandler::CDBDataClientHandler()
{	
	m_lStop			= 1;
	m_lHasStart     = 0; 
	m_bHasReadOp	= FALSE;
	m_lLogicIsRunning =0; //逻辑处理线程是否在运行中
#ifdef WIN32
	m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else 
	sem_init( &m_hStartEvent, 0, 0 ) ;
	sem_init( &m_hStopEvent, 0, 0 ) ;
#endif
	StartWorkThread();
	m_sMsgList.setLock(&m_sMsgListLock);
	m_sFreeList.setLock(&m_sFreeListLock);
	m_memoryTimer =0;
}

CDBDataClientHandler::~CDBDataClientHandler()
{
	StopWorkThread();
#ifdef WIN32
	CloseHandle(m_hStartEvent);
	CloseHandle(m_hStopEvent);
#else 
	sem_destroy(&m_hStartEvent);
	sem_destroy(&m_hStopEvent);
#endif

	DestroyPacketList(m_sMsgList);
	DestroyPacketList(m_sFreeList);
}

void CDBDataClientHandler::AddMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader& packet)
{
	if (nCmd <= 0 || m_lStop)
		return;

	if (IsReadCmd(nCmd))
		SetReadOpFlag(TRUE);
	
	// construct cache data packet
	CDataPacket *cache_pkg = AllocPacket();	
	if (!cache_pkg)
	{
		OutputMsg(rmError, _T("DBDataCacheError allocPacket failed"));
		return;
	}
	size_t nLen = packet.getAvaliableLength();
	cache_pkg->setLength(nLen + sizeof(WORD));
	(*cache_pkg) << nCmd;	
	cache_pkg->writeBuf(packet.getOffsetPtr(), nLen);
	cache_pkg->setPosition(0);
	const char* pData = cache_pkg->getMemoryPtr();

	// add packet to append list of m_sMsgList
	m_sMsgList.append(cache_pkg);
}


void CDBDataClientHandler::Trace()
{
	OutputMsg(rmNormal,"--------------CDBDataClientHandler start----------");

	m_LogicDBReqHandler.Trace();

	m_sActorCacheData.Trace();
	INT_PTR msgSize=0,nCount=0;

	OutputMsg(rmNormal,"---msg start---");
	CBufferAllocator::ALLOCATOR_MEMORY_INFO memoryInfo;
	GetBuffAlloc().GetMemoryInfo(&memoryInfo);
	OutputMsg(rmNormal,"super alloc=%d,free=%d",(int)memoryInfo.SuperBuffer.dwAllocSize,(int)memoryInfo.SuperBuffer.dwFreeSize);
	OutputMsg(rmNormal,"large alloc=%d,free=%d",(int)memoryInfo.LargeBuffer.dwAllocSize,(int)memoryInfo.LargeBuffer.dwFreeSize);
	OutputMsg(rmNormal,"middle alloc=%d,free=%d",(int)memoryInfo.MiddleBuffer.dwAllocSize,(int)memoryInfo.MiddleBuffer.dwFreeSize);
	OutputMsg(rmNormal,"small alloc=%d,free=%d",(int)memoryInfo.SmallBuffer.dwAllocSize,(int)memoryInfo.SmallBuffer.dwFreeSize);
	

	//消息的长度
	for(INT_PTR i=0;i <m_sMsgList.count(); i++ )
	{	
		// get first packet
		CDataPacket* packet = m_sMsgList[i];	
		if(packet)
		{
			nCount ++;
			msgSize += packet->getMemorySize();
		}
	}
	OutputMsg(rmNormal,"msg List Size=%d,packCount=%d",(int)msgSize,(int)nCount);

	//freelist长度
	msgSize=0;
	nCount=0;

	

	for(INT_PTR i=0;i <m_sFreeList.count(); i++ )
	{	
		// get first packet
		CDataPacket* packet = m_sFreeList[i];	
		if(packet)
		{
			msgSize += packet->getMemorySize();
			nCount ++;
		}
	}
	OutputMsg(rmNormal,"free list Size=%d,packCount=%d",(int)msgSize,(int)nCount);

	OutputMsg(rmNormal,"---msg end---");

	OutputMsg(rmNormal,"--------------CDBDataClientHandler end----------");
}

bool CDBDataClientHandler::IsReadCmd(jxSrvDef::INTERSRVCMD nCmd)
{
	bool bResult = false;
	switch (nCmd)
	{
	case dcInitDB:		
	case dcSetGambleFlag:		
	case dcUpdateGateUserInfo:
	case dcResActorDbSessionData:			// 逻辑服务器收到了玩家的数据返回		
	case dcQuery:							// 查询玩家角色数据		
	case dcQuerySubSystemData:		
	case dcLoadQuest:						// 装载任务数据		
	case dcLoadDepotItem:					// 装载仓库物品数据	
	case dcLoadBagItem:						// 装载背包物品数据
	case dcLoadEquipItem:					// 装载装备物品数据		
	case dcLoadActiveBag:					// 装载活动背包物品数据		
	case dcGetActiveItem:					// 提取活动物品		
	case dcDeleteActiveItem:				// 删除获取物品		
	case dcLoadVar:							// 加载角色脚本变量数据		
	case dcLoadGuildList:					// 帮派组件读入所有帮派的数据		
	case dcLoadActorApplyResult:			// 玩家申请结果
	case dcLoadGuildDetail:					// 帮派的详细数据
	case dcLoadGuildApplyLists:             // 行会申请列表
	case dcDealGuildApply:              	// 处理行会申请
	case dcLoadGuildSkill:					// 帮派技能的数据
	case dcAddGuild:						// 增加帮派		
	case dcDeleteGuild:						// 解散帮派		
	//case dcSaveGuild:						// 保存帮派的信息		
	case dcAddMember:						// 增加帮派成员		
	case dcLoadActorGuild:					// 读入个人帮派信息		
	case dcDeleteMember:					// 删除某个成员		
	case dcAddMemberOffLine:				// 帮派增加一个不在线的成员,数据服务器要返回这个成员的详细数据	
	//case dcSaveGuildSkill:					//保存帮派技能信息
	case dcInitGuildSkill:					//初始化帮派技能
	case dcLoadMsg:							// 读取用户的消息数据		
	case dcDeleteMsg:						// 删除某条消息		
	case dcAddMsg:							// 增加用户一条信息		
	case dcAddMsgByActorName:		
	case dcLoadBaseRank:
	//case dcSaveBaseRank:	
	case dcLoadPetDetailData:
	case dcLogout:
	case dcLoadRelation:
	case dcLoadOfflineUserInfo:
	case dcLoadOfflineUserHeroInfo:
	case dcLoadGuildDepotItem:
	case dcLoadGuildEvent:
	case dcLoadGuildDepotRecord:
	//case dcSaveGuildDepotRecord:
	case dcLoadCombatRankInfo:
	case dcLoadCmobatGameInfo:
	//case dcSaveCombatGameInfo:
	case dcLoadCombatRecord:
	//case dcSaveCombatRecord:
	case dcUpdateCombatBaseInfo:
	case dcLoadFriendsData:
	//case dcSaveFriendsData:
	case dcDeleteFriendData:
	case dcAddFriendChatMsg:
	case dcLoadFriendChatMsg:
	case dcLoadTopLevelActor:
	case dcLoadTopLevelActorData:
	case dcLoadActorConsumeRank:
	case dcLoadTopicBattleData:
	case dcQueryActorExists:
	case dcLoadGameSetData:
	case dcLoadSwingLevelData:
	case dcLoadTopicHeroData:
	case dcLoadAllActivityData:
	case dcAddFilterWordsMsg:
	case dcBackRemoveItem:
	case dcBackRemoveMoney:
	case dcSendDbServerStopCmd:
	case dcLoadCombinLevelActor:
	case dcLoadCombinBattleActor:
	case dcUpdateActorState:
	case dcReloadCrossConfig:
	case dcLoadActorBuildingData:
	case dcLoadMail:				//邮件
	case dcSaveOfflineMail:
	case dcLoadFriendOffline:		//装载N天不上线的玩家
	case dcLoadNewTitleData:
	case dcLoadDeathData:
	case dcLoadDeathDropData:
	case dcLoadActivityList:
	case dcLoadPersonActivity:
	case dcLoadReviveDurationData:   
		bResult = true;
		break;
	}
	return bResult;
}


/*处理来自于LogicalServer的DB操作请求
有的请求直接操作，有的请求可能先放入缓存，待一段时间后执行
*/
void CDBDataClientHandler::HandleDBMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacket &packet)
{
	unsigned int nActorID = 0;	
	switch (nCmd)
	{
		//////////////////////////////////////////////////////////////////////////
		// read request
		//////////////////////////////////////////////////////////////////////////
	case dcInitDB:
		{
			/*if (m_pClient)
				m_pClient->ExecuInitDb();	*/		
			m_LogicDBReqHandler.ExecuInitDb(packet);
			m_LogicDBReqHandler.UpdateCharOnlineStatus(0, 0);//启动时清空在线标识，防宕机有留存，暂没支持多服共用一个数据库的情况
			break;
		}
	case dcSetGambleFlag:
		{
			int nFlag; 
			packet >> nFlag ;
			CDBDataServer::m_bGableIsClose = nFlag ?true:false; 
			break;
		}
	case dcUpdateGateUserInfo:
		{
			//m_pClient->UpdateLogicGateUserInfo(packet);
			//m_LogicDBReqHandler.UpdateLogicGateUserInfo(packet);
			break;
		}
	case dcResActorDbSessionData:			// 逻辑服务器收到了玩家的数据返回
		{			
			/*if (m_pClient)
				m_pClient->OnResponseDbSessionData(packet);*/
			m_LogicDBReqHandler.OnResponseDbSessionData(packet);
			break;
		}

	case dcSendDbServerStopCmd:
		{
			m_LogicDBReqHandler.LogicSendStopDbServerCmd(packet);
			break;
		}

	case dcReloadCrossConfig:
		{
			m_LogicDBReqHandler.OnReloadCrossConfig(packet);
			break;
		}

	case dcQuery:							// 查询玩家角色数据
		{	
			unsigned int nActorId = m_LogicDBReqHandler.QueryActorFromDB(packet);
			
			if (CDBDataServer *pDataServer = CDBServer::s_pDBEngine->getDataServer())
			{
				pDataServer->m_ActorIdMapLock.Lock();
				pDataServer->m_ActorIdMap[nActorId] = true;
				pDataServer->m_ActorIdMapLock.Unlock();
			}

			//if (m_pClient)
			//{
			//	m_pClient->m_LogicDBRequestHandler.QueryActorFromDB(packet);
			//}
			//	//m_pClient->QueryActorFromDB(packet);
			//else
			//	OutputMsg(rmError, _T("dcQuery error, m_pClient is null"));
			break;
		}

		//获取玩家战力排行前1000名的
	case dcLoadCombatRankInfo:
		{
			m_LogicDBReqHandler.LoadActorCombatRank(packet);
			break;

		}
		//职业宗师竞技基本信息
	case dcLoadCmobatGameInfo:
		{
			m_LogicDBReqHandler.LoadActorCombatBaseInfo(packet);
			break;
		}

	case dcUpdateCombatBaseInfo:
		{
			m_LogicDBReqHandler.UpdateActorCombatAward(packet);
			break;
		}

	//保存职业宗师竞技基本信息
	case dcSaveCombatGameInfo:
		{
			m_sActorCacheData.CacheData(enCT_CombatGameData, packet);
			break;
		}
		//战力竞技日志
	case dcLoadCombatRecord:
		{
			m_LogicDBReqHandler.LoadActorCombatLog(packet);
			break;
		}
		//保存战力竞技日志
	case dcSaveCombatRecord:
		{
			m_sActorCacheData.CacheData(enCT_CombatRecordData, packet);
			break;
		}

	case dcLoadFriendsData:
		{
			//m_LogicDBReqHandler.LoadActorFriendsData(packet);
			break;
		}
	case dcLoadDeathData:
		{
			//m_LogicDBReqHandler.LoadActorDeathData(packet);
			break;
		}
	case dcLoadCircleCount:
		{
			m_LogicDBReqHandler.LoadCircleCount();
			break;
		}
	case dcLoadDeathDropData:
		{
			m_LogicDBReqHandler.LoadActorDeathDropData(packet);
			break;
		}
	case dcAddFilterWordsMsg:
		{
			m_LogicDBReqHandler.AddFilterWordsDbMsg(packet);
			break;
		}
	case dcBackRemoveItem:
		{
			m_LogicDBReqHandler.BackRemoveItem(packet);
			break;
		}
	case dcBackRemoveMoney:
		{
			m_LogicDBReqHandler.BackRemoveMoney(packet);
			break;
		}
	case dcSaveFriendsData:
		{
			//m_sActorCacheData.CacheData(enCT_FriendData, packet);
			m_LogicDBReqHandler.SaveActorFriendsData(packet);
			break;
		}
	case dcSaveDeathData:
		{
			//
			//m_sActorCacheData.CacheData(enCT_DeathData, packet);
			m_LogicDBReqHandler.SaveActorDeathData(packet);
			break;
		}
	case dcSaveDeathDropData:
		{
			m_sActorCacheData.CacheData(enCT_DeathDropData, packet);
			break;
		}
	case dcDeleteFriendData:
		{
			m_LogicDBReqHandler.DeleteActorFriendData(packet);
			break;
		}

	case dcAddFriendChatMsg:
		{
			m_LogicDBReqHandler.AddFriendChatMsg(packet);
			break;
		}

	case dcLoadFriendChatMsg:
		{
			m_LogicDBReqHandler.LoadFriendChatMsgData(packet);
			break;
		}

	case dcLoadGameSetData:
		{
			m_LogicDBReqHandler.LoadActorGameSetData(packet);
			break;
		}

	case dcSaveGameSetData:
		{
			m_sActorCacheData.CacheData(enCT_GameSetData, packet);
			//m_LogicDBReqHandler.SaveActorGameSetData(packet);
			break;
		}

	case dcLoadTopLevelActor:
		{
			m_LogicDBReqHandler.LoadTopLevelActorData(packet);
			break;
		}
	case dcLoadTopLevelActorData:
		{
			m_LogicDBReqHandler.LoadTopLevelActorOneData(packet);
			break;
		}
	case dcLoadCombinLevelActor:
		{
			m_LogicDBReqHandler.LoadCombineLevelActorData(packet);
			break;
		}

	case dcLoadCombinBattleActor:
		{
			m_LogicDBReqHandler.LoadCombineBattleActorData(packet);
			break;
		}

	case dcLoadSwingLevelData:
		{
			m_LogicDBReqHandler.LoadActorSwingLevelData(packet);
			break;
		}

	case dcLoadTopicHeroData:
		{
			m_LogicDBReqHandler.LoadActorHeroTopicRankData(packet);
			break;
		}

	case dcLoadAllActivityData:
		{
			m_LogicDBReqHandler.LoadAllActivityData(packet);
			break;
		}

	case dcLoadActorConsumeRank:
		{
			m_LogicDBReqHandler.LoadActorConsumeRank(packet);
			break;
		}

	case dcLoadTopicBattleData:
		{
			m_LogicDBReqHandler.LoadActorTopicBattle(packet);
			break;
		}


	case dcQuerySubSystemData:
		{
			//if (m_pClient)
			//	m_pClient->QueryActoSubSystemData(packet); // 装载玩家子系统数据
			m_LogicDBReqHandler.QueryActoSubSystemData(packet);
			break;
		}
	case dcLoadQuest:						// 装载任务数据
		{			
			/*if (m_pClient)
				m_pClient->QueryQuestFromDB(packet);*/
			m_LogicDBReqHandler.QueryQuestFromDB(packet);
			break;
		}
	case dcLoadFriendOffline:		//装载N天没上线的玩家
		{
			m_LogicDBReqHandler.LoadFiendOfflineData(packet);
			break;
		}
	case dcLoadDepotItem:					// 装载仓库物品数据
		{
			int nRawServerId = 0, nLoginServerId;
			packet >> nRawServerId >> nLoginServerId >> nActorID;
			/*if (m_pClient)
				m_pClient->QueryItems(nActorID,itItemDepot);*/
			m_LogicDBReqHandler.QueryItems(nActorID, itItemDepot, nRawServerId, nLoginServerId);
			break;
		}
	case dcLoadBagItem:						// 装载背包物品数据
	case dcLoadEquipItem:					// 装载装备物品数据
		{
			break;
		}
	case dcLoadActiveBag:					// 装载活动背包物品数据
		{
			m_LogicDBReqHandler.LoadUserActiveItem(packet);
			/*if (m_pClient)
				m_pClient->LoadUserActiveItem(packet);*/
			break;
		}
	case dcGetActiveItem:					// 提取活动物品
		{
			/*if (m_pClient)
				m_pClient->GetUserActiveItem(packet);*/
			m_LogicDBReqHandler.GetUserActiveItem(packet);
			break;
		}
	case dcDeleteActiveItem:				// 删除获取物品
		{
			/*if (m_pClient)
				m_pClient->RemoveUserActiveItem(packet);*/
			m_LogicDBReqHandler.RemoveUserActiveItem(packet);
			break;
		}
	case dcLoadVar:							// 加载角色脚本变量数据
		{			
			/*if (m_pClient)
				m_pClient->QueryActorScriptData(nActorID);*/
			m_LogicDBReqHandler.QueryActorScriptData(packet);
			break;
		}
	case dcLoadActivityList:
		{
			m_LogicDBReqHandler.LoadActivityList(packet);
			break;
		}
	case dcSaveActivityList:
		{
			m_LogicDBReqHandler.SaveActivityList(packet);
			break;
		}
	case dcLoadPersonActivity:
		{
			int nRawServerId = 0, nLoginServerId;
			packet >> nRawServerId >> nLoginServerId >> nActorID;
			m_LogicDBReqHandler.LoadActorActivity(packet, nActorID, nRawServerId, nLoginServerId);
			break;
		}
		
	case dcSavePersonActivity:
		{
			int nRawServerId = 0, nLoginServerId;
			packet >> nRawServerId >> nLoginServerId >> nActorID;
			m_LogicDBReqHandler.SaveActorActivity(packet, nActorID, nRawServerId, nLoginServerId);
			break;
		}
	case dcSaveofflineAchieve:
		{
			m_LogicDBReqHandler.SaveOfflineAchieveMentInfo(packet);
			break;
		}
	case dcLoadofflineAchieve:
	{
		m_LogicDBReqHandler.LoadOfflineAchieveMentInfo(packet);
		break;
	}
	case dcLoadGuildList:					// 帮派组件读入所有帮派的数据
	{
		/*if (m_pClient)
			m_pClient->LoadGuildList(packet);*/
		m_LogicDBReqHandler.LoadGuildList(packet);
		break;
	}
	case dcLoadActorApplyResult:					// 全局玩家行会申请结果
	{
		m_LogicDBReqHandler.LoadActorApplyResult(packet);
		break;
	}
	case dcSaveActorApplyResult:
	{
		m_LogicDBReqHandler.SaveActorApplyResult(packet);
		break;
	}
	case dcLoadGuildDetail:					// 帮派的详细数据
	{
		/*if (m_pClient)
			m_pClient->LoadGuildDetail(packet);*/
		m_LogicDBReqHandler.LoadGuildDetail(packet);
		break;
	}
	case dcLoadGuildApplyLists:						//加载帮派申请
	{
		m_LogicDBReqHandler.LoadGuildApplyList(packet);
		break;
	}
	case dcDealGuildApply:						//处理帮派申请
	{
		m_LogicDBReqHandler.DealGuildApply(packet);
		break;
	}
	case dcLoadGuildSkill:					//帮派技能的数据
	{
		/*if (m_pClient)
			m_pClient->LoadGuildSkillDb(packet);*/
		m_LogicDBReqHandler.LoadGuildSkillDb(packet);
		break;
	} 
	case dcLoadActorBuildingData:
	{
		m_LogicDBReqHandler.LoadActorBuildingDb(packet);
		break;
	} 
	case dcAddGuild:						// 增加帮派
	{
		/*if (m_pClient)
			m_pClient->AddGuild(packet);*/
		m_LogicDBReqHandler.AddGuild(packet);
		break;
	}
	case dcDeleteGuild:						// 解散帮派
	{
		/*if (m_pClient)
			m_pClient->DeleteGuild(packet);*/
		m_LogicDBReqHandler.DeleteGuild(packet);
		break;
	}
	case dcSaveGuild:						// 保存帮派的信息
	{
		/*if (m_pClient)
			m_pClient->SaveGuild(packet);*/
		m_LogicDBReqHandler.SaveGuild(packet);
		break;
	}
	case dcAddMember:						// 增加帮派成员
	{
		/*if (m_pClient)
			m_pClient->AddGuildMember(packet);*/
		m_LogicDBReqHandler.AddGuildMember(packet);
		break;
	}
	case dcSaveGuildSkill:						// 增加帮派成员
	{
		/*if (m_pClient)
			m_pClient->SaveGuildSkill(packet);*/
		m_LogicDBReqHandler.SaveGuildSkill(packet);
		break;
	}
	case dcSaveGuildEvent:
	{
		/*if (m_pClient)
			m_pClient->SaveGuildEventDb(packet);*/
		m_LogicDBReqHandler.SaveGuildEventDb(packet);
		break;
	}
	case dcSaveGuildDepotItem:
	{
		m_LogicDBReqHandler.SaveGuildDepotDb(packet);
		break;
	} 
	case dcLoadGuildDepotItem:
	{
		m_LogicDBReqHandler.LoadGuildDepoyFromDb(packet);
		break;
	}
	case dcLoadGuildDepotRecord:
	{
		m_LogicDBReqHandler.LoadGuildDepotRecordDb(packet);
		break;
	}
	case dcSaveGuildDepotRecord:
	{
		m_LogicDBReqHandler.SaveGuildDepotRecordDb(packet);
		break;
	} 
	case dcLoadGuildEvent:
	{
		/*if (m_pClient)
			m_pClient->LoadGuildEventDb(packet);*/
		m_LogicDBReqHandler.LoadGuildEventDb(packet);
		break;
	}
	case dcInitGuildSkill:						//初始化帮派技能
	{
		/*if (m_pClient)
			m_pClient->InitGuildSkillInfo(packet);*/
		m_LogicDBReqHandler.InitGuildSkillInfo(packet);
		break;
	}
	case dcLoadActorGuild:					// 读入个人帮派信息
	{
		break;
	}
	case dcDeleteMember:					// 删除某个成员
	{
		/*if (m_pClient)
			m_pClient->DeleteMember(packet);*/
		m_LogicDBReqHandler.DeleteMember(packet);
		break;
	}
	case dcAddMemberOffLine:				// 帮派增加一个不在线的成员,数据服务器要返回这个成员的详细数据
	{
		/*if (m_pClient)
			m_pClient->AddMemberOffLine(packet);*/
		m_LogicDBReqHandler.AddMemberOffLine(packet);
		break;
	}
	case dcLoadMsg:							// 读取用户的消息数据
	{
		/*if (m_pClient)
			m_pClient->LoadMsgList(packet);*/
		m_LogicDBReqHandler.LoadMsgList(packet);
		break;
	}
	case dcDeleteMsg:						// 删除某条消息
	{
		/*if (m_pClient)
			m_pClient->DeleteMsg(packet);*/
		m_LogicDBReqHandler.DeleteMsg(packet);
		break;
	}
	case dcAddMsg:							// 增加用户一条信息
	{
		/*if (m_pClient)
			m_pClient->AddMsg(packet);*/
		m_LogicDBReqHandler.AddMsg(packet);
		break;
	}
	case dcAddMsgByActorName:
	{
		/*if (m_pClient)
			m_pClient->AddMsgByName(packet);*/
		m_LogicDBReqHandler.AddMsgByName(packet);
		break;
	}
	case dcLoadBaseRank:
	{
		/*if (m_pClient)
			m_pClient->LoadBaseRank(packet);	*/		
		m_LogicDBReqHandler.LoadBaseRank(packet);
		break;
	}
	case dcSaveBaseRank:
	{
		/*if (m_pClient)
			m_pClient->SaveBaseRank(packet);		*/	
		m_sActorCacheData.SaveAllActorDataImmediately();
		m_LogicDBReqHandler.SaveBaseRank(packet);
		break;
	} 
	case dcSaveBaseRankCS://跨服的排行榜保存数据
	{ 
		OutputMsg(rmTip,"[dcSaveBaseRankCS]1 跨服的排行榜保存数据 !" );
		m_sActorCacheData.SaveAllActorDataImmediately();
		m_LogicDBReqHandler.SaveBaseRankCS(packet);
		break;
	} 
	case dcLoadPetDetailData:
	{
		//QueryPetDetailData(inPacket);
		break;
	} 
	case dcSaveRelation:
	{
		/*if(m_pClient)
			m_pClient->SaveRelation(packet);*/
		m_LogicDBReqHandler.SaveRelation(packet);
		break;
	}
	case dcSaveOfflineInfo:
	{
		m_LogicDBReqHandler.SaveOfflineActorProperty(packet);
		break;
	}
	case dcSaveCustomInfo:
	{
		m_LogicDBReqHandler.SaveActorCustomInfo(packet);
		break;
	} 
	case dcSaveCustomInfoUnForbid:
	{
		m_LogicDBReqHandler.SaveActorCustomInfoUnForbid(packet);
		break;
	}
	case dcSaveCustomInfoCustomTitleId:
	{
		m_LogicDBReqHandler.SaveActorCustomInfoCustomTitleId(packet);
		break;
	}   
	case dcLoadCustomInfo:
	{
		m_LogicDBReqHandler.QueryActorCustomInfo(packet);
		break;
	}   
	case dcLoadCustomInfoUnForbid:
	{
		m_LogicDBReqHandler.QueryActorCustomInfoUnForbid(packet);
		break;
	}
	case dcLoadCustomInfoCustomTitleId:
	{
		m_LogicDBReqHandler.QueryActorCustomInfoCustomTitleId(packet);
		break;
	}   
	case dcLoadOfflineUserInfo:
	{
		/*if(m_pClient)
			m_pClient->QueryOfflineUserInfo(packet);*/
		m_LogicDBReqHandler.QueryOfflineUserInfo(packet);
		break;
	}
	case dcLoadOfflineUserHeroInfo:
	{
		/*if(m_pClient)
			m_pClient->QueryOfflineUserInfo(packet);*/
		m_LogicDBReqHandler.QueryOfflineUserHeroInfo(packet);
		break;
	}
	case dcUpdateChallengeData:
	{
		//if (m_pClient)
		//{
		//	// 即时保存数据
		//	m_sActorCacheData.SaveAllActorDataImmediately();
		//	m_pClient->UpdateChallengeData(packet);
		//}
		m_sActorCacheData.SaveAllActorDataImmediately();
		m_LogicDBReqHandler.UpdateChallengeData(packet);
		break;
	} 
	case dcSaveStaticCount:
	{
		m_LogicDBReqHandler.SaveActorStaticCounts(packet);
		break;
	} 
	case dcSaveActorStrengthenInfo:
	{
		m_LogicDBReqHandler.SaveActorStrengthenInfo(packet);
		break;
	} 

	//////////////////////////////////////////////////////////////////////////
	// write request
	//////////////////////////////////////////////////////////////////////////
	case dcLogout:
	{
		m_sActorCacheData.OnActorLogout(packet);
		break;
	}
	case dcSave:							// 保存角色基本数据
	{	
		m_sActorCacheData.CacheData(enCT_ActorBasicData, packet);
		break;
	}
	case dcSaveQuest:						// 保存角色任务数据
	{
		m_sActorCacheData.CacheData(enCT_QuestData, packet);			
		break;
	}
	case dcSaveSkill:						// 保存角色技能数据
	{
		m_sActorCacheData.CacheData(enCT_SkillData, packet);			
		break;
	} 
	case dcSaveOtherGameSets:						// 保存游戏新设置数据
	{
		m_sActorCacheData.CacheData(enCT_GameOtherSetData, packet);			
		break;
	}
	case dcSaveDepotItem:					// 保存仓库物品，13
	{
		m_sActorCacheData.CacheData(enCT_DepotItemData, packet);
		break;
	}
	case dcSaveBagItem:						// 保存背包物品数据
	{
		m_LogicDBReqHandler.SaveItems(packet);
		//m_sActorCacheData.CacheData(enCT_BagItemData, packet);			
		break;
	}
	case dcSaveEquipItem:					// 保存装备物品
	{
		// m_sActorCacheData.CacheData(enCT_EquipItemData, packet);
		m_LogicDBReqHandler.SaveItems(packet);
		break;
	}
	case dcSaveHeroItem:						// 保存宠物的装备
	{
		m_sActorCacheData.CacheData(enCT_PetItemData, packet);			
		break;
	}
	case dcSaveVar:							// 保存角色脚本变量
	{
		m_sActorCacheData.CacheData(enCT_ScriptData, packet);			
		break;
	}
	case dcSaveActorGuild:					// 保存个人帮派信息
	{
		m_sActorCacheData.CacheData(enCT_GuildData, packet);
		break;
	}
	case dcSaveHeroData:
	{
		m_sActorCacheData.CacheData(enCT_PetData, packet);
		break;
	}
	case dcSaveHeroSkillData:
	{
		m_sActorCacheData.CacheData(enCT_PetSkillData, packet);
		break;
	}
	case dcQueryActorExists:
	{
		m_LogicDBReqHandler.QueryActorExists(packet);
		break;
	} 
	case  dcUpdateActorState:
	{
		m_LogicDBReqHandler.UpdateCharStatus(packet);
		break;
	}
	case dcLoadMail:
	{
		m_LogicDBReqHandler.LoadActorMail(packet);
		break;
	}
	case dcSaveMail:
	{
		m_LogicDBReqHandler.SaveActorMail(packet);
		break;
	}
	case dcSaveOfflineMail:
	{
		m_LogicDBReqHandler.AddActorOfflineMail(packet);
		break;
	}
	case dcLoadNewServerMail:
	{
		int nServerId = 0, nRawServerId = 0;
		packet >> nRawServerId >> nServerId;
		int nMaxServerMailId = 0;
		packet >> nMaxServerMailId;
		m_LogicDBReqHandler.LoadNewServerMail(nServerId, nRawServerId, nMaxServerMailId);
		break;
	}
	case dcSaveNewServerMail:
	{
		m_LogicDBReqHandler.SaveNewServerMail(packet);
		break;
	}
	case dcAddNewServerMail:
	{
		m_LogicDBReqHandler.AddNewServerMail(packet);
		break;
	}
	case dcSaveAlmirahItem:					// 保存装备物品
	{
		m_sActorCacheData.CacheData(enCT_AlmirahItemData, packet);
		break;
	}
	case dcLoadNewTitleData:
	{
		m_LogicDBReqHandler.LoadActorNewTitleData(packet);
		break;
	}
	case dcSaveNewTitleData:
	{
		m_LogicDBReqHandler.SaveActorNewTitleData(packet);
		break;
	}
	case dcSaveConsignmentItem:			
	{
		m_LogicDBReqHandler.SaveConsignmentItem(packet);
		break;
	}
	case dcLoadConsignmentItem:			
	{
		m_LogicDBReqHandler.LoadConsignmentItem(packet);
		break;
	}
	case dcSaveActorConsignment:
	{
		m_sActorCacheData.CacheData(enCT_ConsignData, packet);
		break;
	}
	case dcLoadConsignmentIncome:
	{
		m_LogicDBReqHandler.LoadConsignmentIncome(packet);
		break;
	}
	case dcSaveAchieveMent:
	{
		m_LogicDBReqHandler.SaveActorAchieveMentInfo(packet);
		break;
	}
	case dcLoadFee:
	{
		int nRawServerId = 0, nLoginServerId;
		packet >> nRawServerId >> nLoginServerId >> nActorID;
		m_LogicDBReqHandler.LoadFee(packet, nActorID, nRawServerId, nLoginServerId);
		break;
	}
	case dcClearFee:
	{
		int nRawServerId = 0, nLoginServerId;
		packet >> nRawServerId >> nLoginServerId >> nActorID;
		m_LogicDBReqHandler.DelFee(packet, nActorID, nRawServerId, nLoginServerId);
		break;
	}

	case dcLoadGhostInfo:
	{
		m_LogicDBReqHandler.LoadActorGhost(packet);
		break;
	}
	case dcSaveGhostInfo:
	{
		m_LogicDBReqHandler.SaveActorGhost(packet);
		break;
	}
	case dcLoadCrossGuildId:
	{
		m_LogicDBReqHandler.LoadCrossGuildId(packet);
		break;
	}
	case dcSaveHallowsSystemInfo:
	{
		m_LogicDBReqHandler.SaveHallowsSystemInfo(packet);
		break;
	} 
	case dcLoadHallowsSystemInfo:
	{
		m_LogicDBReqHandler.LoadHallowsSystemInfo(packet);
		break;
	}
	case dcLoadReviveDurationData:
	{
		m_LogicDBReqHandler.LoadReviveDurationSystemInfo(packet);
		break;
	} 
	case dcSaveReviveDurationData:
	{
		m_LogicDBReqHandler.SaveReviveDurationSystemInfo(packet);
		break;
	}
	case dcSaveLootPetData:
	{
		m_LogicDBReqHandler.SaveLootPetToDB(packet);
		break;
	}
	case dcLoadRebateData:
	{
		m_LogicDBReqHandler.QueryRebateData(packet);
		break;
	}
	case dcLoadNextDayRebateData:
	{
		m_LogicDBReqHandler.QueryNextDayRebateData(packet);
		break;
	}
	case dcUpdateRebateData:
	{
		m_LogicDBReqHandler.UpdateRebateData(packet);
		break;
	}
	case dcSaveRebateData:
	{
		m_LogicDBReqHandler.SaveRebateToDB(packet);
		break;
	}
	case dcClearRebateData:
	{
		m_LogicDBReqHandler.ClearRebateData(packet);
		break;
	}
	case dcBackChangeTradingQuota:
	{
		m_LogicDBReqHandler.ChangeActorTradingQuota(packet);
		break;
	}
	case dcLoadCustomTitleData:
	{
		m_LogicDBReqHandler.LoadActorCustomTitleData(packet);
		break;
	}
	case dcSaveCustomTitleData:
	{
		m_LogicDBReqHandler.SaveActorCustomTitleData(packet);
		break;
	}
	// case dcLoadBossInfo:
	// 	{
	// 		m_LogicDBReqHandler.LoadBossList(packet);
	// 		break;
	// 	}
	// case dcSaveBossInfo:
	// 	{
	// 		m_LogicDBReqHandler.SaveBossList(packet);
	// 		break;
	// 	}
	default:
		OutputMsg(rmError, _T("%s recv unknown dbrequest cmd[id=%d]"), __FUNCTION__, (int)nCmd);
		break;
	}
}

bool CDBDataClientHandler::Start(ILogicDBRequestHost* pHost, CSQLConenction *lpSQLConnection)
{	
	m_pHost = pHost;	
	m_LogicDBReqHandler.SetRequestHost(m_pHost);
	m_LogicDBReqHandler.SetSQLConnection(lpSQLConnection);
	m_sActorCacheData.SetDBClient(m_pHost, this);	
	if (TRUE == ::InterlockedCompareExchange(&m_lStop, FALSE, TRUE))
	{
#ifdef WIN32
		ResetEvent(m_hStopEvent);
#endif

		//如果已经启动了，则不需要等待
		if(FALSE== ::InterlockedCompareExchange(&m_lHasStart, TRUE, FALSE) )
		{
			//如果已经启动了，那么就不等待了
			if( TRUE== ::InterlockedCompareExchange(&m_lLogicIsRunning, TRUE, TRUE) )
			{

			}
			else
			{
				OutputMsg(rmNormal, _T("Mgr Start: wait for StartEvent 1 minutes ,CurThreadId=%d"), GetCurrentThreadId());
#ifdef WIN32
				WaitForSingleObject(m_hStartEvent, 60000);
#else
				struct timespec tm ;
				clock_gettime(CLOCK_REALTIME, &tm);
				tm.tv_sec += 60;
				sem_timedwait(&m_hStartEvent, &tm);
#endif
			}
			
		}
		else
		{
			OutputMsg(rmNormal, _T("Mgr Start: ThreadLogicProc has Start already. CurThreadId=%d"), GetCurrentThreadId());
			
		}
		OutputMsg(rmNormal, _T("Mgr Start: finish and return, CurThreadId=%d"), GetCurrentThreadId());
		return true;
	}
	else
	{
		::InterlockedCompareExchange(&m_lHasStart, TRUE, FALSE);
		OutputMsg(rmNormal, _T("Mgr Start: ThreadLogicProc Is Running Already,CurThreadId=%d"),GetCurrentThreadId());
		return false;
	}
}

bool CDBDataClientHandler::Stop()
{	
	
	::InterlockedCompareExchange(&m_lHasStart,FALSE , TRUE);
	if (FALSE == ::InterlockedCompareExchange(&m_lStop, TRUE, FALSE))
	{		
		StopWorkThread();
#ifdef WIN32
		ResetEvent(m_hStartEvent);
#endif
		//如果已经退出了就不等待了,否则等待3分钟，防止线程异常退出的时候没有信号的情况
		//如果已经在运行，则等待，否则直接退出
		if( TRUE== ::InterlockedCompareExchange(&m_lLogicIsRunning, TRUE, TRUE)  )
		{
			OutputMsg(rmNormal, _T("Mgr Stop:  CurThreadId=%d,wait for ThreadLogicProc quit 4 minuts..."), GetCurrentThreadId());
#ifdef WIN32
			WaitForSingleObject(m_hStopEvent, 240000);
#else
			struct timespec tm ;
			clock_gettime(CLOCK_REALTIME, &tm);
			tm.tv_sec += 240;
			sem_timedwait(&m_hStopEvent, &tm);
#endif
		}
		OutputMsg(rmNormal, _T("Mgr Stop: finish and return. CurThreadId=%d"), GetCurrentThreadId());
		return true;
	}
	else
	{
		OutputMsg(rmNormal, _T("Mgr Stop: ThreadLogicProc Is Stopped Already, CurThreadId=%d"),GetCurrentThreadId());
		return false;
	}

}

void CDBDataClientHandler::OnRountine()
{
	//如果没有启动的话，则长期等待

	Sleep(1000);
	while( FALSE== ::InterlockedCompareExchange(&m_lHasStart,FALSE , FALSE))
	{
		Sleep(50);
	}

#ifdef WIN32
	SetEvent(m_hStartEvent);
#else
	sem_post(&m_hStartEvent);
#endif
	::InterlockedCompareExchange(&m_lLogicIsRunning,TRUE , FALSE);
	ThreadLogicProc();

	::InterlockedCompareExchange(&m_lLogicIsRunning, FALSE, TRUE);
#ifdef WIN32
	SetEvent(m_hStopEvent);
#else
	sem_post(&m_hStopEvent);
#endif

	
}

void CDBDataClientHandler::ThreadLogicProc()
{
	OutputMsg(rmNormal, _T("ThreadLogicProc Start! CurThreadId=%d"), GetCurrentThreadId());
	while (!m_lStop)
	{
		RunOne();
		Sleep(s_nDBHandleSleep);
	}

	// call before quit thread	
	m_sActorCacheData.SaveAllActorDataImmediately();
	OutputMsg(rmNormal, _T("ThreadLogicProc Stop! curThreadId=%d"), GetCurrentThreadId());
}

void CDBDataClientHandler::RunOne()
{
#ifdef _USE_TRY_CATCH
	__try
	{
#endif 
		TICKCOUNT tc = _getTickCount();
		// append write list to read list
		if (m_sMsgList.appendCount() > 0)
			m_sMsgList.flush();

		// iterate all data in list, this is high priority
		while (m_sMsgList.count() > 0)
		{	
			// get first packet
			CDataPacket* packet = m_sMsgList[0];		
			jxSrvDef::INTERSRVCMD nCmd;
			*packet >> nCmd;

			// handle packet
			HandleDBMessage(nCmd, *packet);
						
			// free packet
			m_sMsgList.remove(0);
			FreePacket(packet);			
		}

		// for write handle, this is low priority
		SetReadOpFlag(FALSE);
		m_sActorCacheData.RunOne(tc);	

		if(m_memoryTimer <= tc)
		{
			m_memoryTimer = tc + 300000; //5分钟
			CheckMemory(false);
		}
#ifdef _USE_TRY_CATCH
	}
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
		/*
		CSQLConenction* pConnect = m_LogicDBReqHandler.GetSqlConnection();
		if(pConnect)
		{
			pConnect->ResetQuery(); //释放以避免死锁
		}
		*/

	}
#endif

}

CDataPacket* CDBDataClientHandler::AllocPacket()
{
	// here code is same to following, don't merge because of efficient thinking
	if (m_sFreeList.count() > 0)
	{
		CDataPacket* pkg = m_sFreeList.pop();
		pkg->setLength(0);
		return pkg;
	}

	if (m_sFreeList.appendCount() > 0)
		m_sFreeList.flush();
	if (m_sFreeList.count() <= 0 )
		allocSendPacketList(m_sFreeList, 512);
	CDataPacket* pkg = m_sFreeList.pop();
	pkg->setLength(0);
	return pkg;
}

void CDBDataClientHandler::FreePacket(CDataPacket* packet)
{
	m_sFreeList.append(packet);
}

void CDBDataClientHandler::DestroyPacketList(CQueueList<CDataPacket*>& pkgList)
{
	if (pkgList.appendCount() > 0)
		pkgList.flush();

	for (INT_PTR i = pkgList.count()-1; i >= 0; i--)
	{
		CDataPacket* pkg = pkgList[i];
		pkg->~CDataPacket();
	}
	pkgList.clear();
}
