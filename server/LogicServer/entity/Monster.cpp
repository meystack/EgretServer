#include "StdAfx.h"

//const int monsterVectResetInterval = 6000;			// 怪物归属重置间隔(6s没有打怪，清除怪物原来归属)
CUpdateMask *CMonster::s_monsterBroadcastMask = NULL;
 void  CMonster::InitAi()
 {
	 unsigned int nID = GetProperty<unsigned int>(PROP_ENTITY_ID);
	 PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	 if(pConfig ==NULL) return;	 
	 m_pAI = GetGlobalLogicEngine()->GetAiMgr().CreateAi(pConfig->wAiConfigId,(CAnimal*)this);
	 	 
	 /*if (pConfig->monsterSayList.vecSayTypeInfo[mstHpLow].nRate > 0)
		 m_bHasHPChangeSayConfig = true;
	 else
		 m_bHasHPChangeSayConfig = false;*/
	 MonsterSayTypeConfig& hpTypeConfig = pConfig->monsterSayList.vecSayTypeInfo[mstHpLow];
	 if (hpTypeConfig.nCount > 0)
		 m_bHasHPChangeSayConfig = true;
	 else
		 m_bHasHPChangeSayConfig = false;
	
	 MonsterSayTypeConfig& idleTypeConfig = pConfig->monsterSayList.vecSayTypeInfo[mstIdle];
	 if (idleTypeConfig.nCount > 0)
	 {
		PostponeMonsterIdleSay();
	 }
	 //if (pConfig->monsterSayList.vecSayTypeInfo[mstIdle].nRate > 0)
	 //{
		// if (pConfig->monsterSayList.nUpperInterval - pConfig->monsterSayList.nLowerInterval < 10)  // 防止策划配置错误导致发言太频繁
		//	 pConfig->monsterSayList.nUpperInterval = pConfig->monsterSayList.nLowerInterval + 10;
		// PostponeMonsterIdleSay();
	 //}
	 /*
	 if(pConfig->btMonsterType ==tagMonsterConfig::mtBoss ) //只有boss怪才使用A*寻路
	 {
		 GetMoveSystem()->SetUseAstar(true);
	 }
	 */

	 m_nMonsterType = pConfig->btMonsterType;
	 m_nAttackType = pConfig->btAttackType;
	 m_attriFlag = *(pConfig->pflags);
	 SetAttackInterval(pConfig->nAttackInterval); //设置攻击间隔
	 SetMaxDropHp(pConfig->nMaxDropHp); //每次掉落的HP
	 SetAttackLevel(pConfig->nattackLevel);
}

void CMonster::PostponeMonsterIdleSay()
{
	unsigned int nID = GetProperty<unsigned int>(PROP_ENTITY_ID);
	PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	if(pConfig ==NULL) return;	
	MonsterSayTypeConfig& idleTypeCfg = pConfig->monsterSayList.vecSayTypeInfo[mstIdle];
	if (idleTypeCfg.nCount > 0)
	{
		MonsterSayInfo& idle_info = idleTypeCfg.pMonsterSayCfg[0]; // Idle只处理第一个
		if (idle_info.nRate > 0)
		{
			int nRange = pConfig->monsterSayList.nUpperInterval - pConfig->monsterSayList.nLowerInterval+1;
			int nNextInterval = wrand(nRange) + pConfig->monsterSayList.nLowerInterval;		
			m_nNextIdleSayTime	= GetGlobalLogicEngine()->getTickCount() + nNextInterval * 1000;
		}
	}
	//MonsterSayInfo &idle_info = pConfig->monsterSayList.vecSayInfo[mstIdle];
	/*if (idle_info.nRate > 0)
	{
		int nRange = pConfig->monsterSayList.nUpperInterval - pConfig->monsterSayList.nLowerInterval+1;
		int nNextInterval = wrand(nRange) + pConfig->monsterSayList.nLowerInterval;		
		m_nNextIdleSayTime	= GetGlobalLogicEngine()->getTickCount() + nNextInterval * 1000;
	}*/
}

 //怪物暂时只下发少量的数据
 VOID CMonster::InitMonsterBroadcastmask()
 {
	 if (!s_monsterBroadcastMask)
	 {
		 s_monsterBroadcastMask = new CUpdateMask();
	 }
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_HP);
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MP);
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MAXHP);
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MOVEONESLOTTIME); //移动速度改变了需要广播的
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_ATTACK_SPEED);
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_COLOR);
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_ENTITY_MODELID);
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_ENTITY_DIR); //朝向
	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_DIZZY_STATUS);

	 s_monsterBroadcastMask->SetUpdateMaskFlag(PROP_MONSTER_BELONG_ID);
 }

bool CMonster::CanSee(CEntity* pEntity)
{
	if(CEntity::CanSee(pEntity))
	{
		return true;
	}
	else
	{
		if(pEntity->isAnimal())
		{		
			if( ((CAnimal*)pEntity)->GetBuffSystem()->Exists(aHide))
			{
				if (GetMonsterHetred(pEntity) > 0 || GetAttackerHandle() == pEntity->GetHandle() )
				{
					return true;
				}
			}
		}
	}
	return false;
}

void CMonster::Destroy()
{
	Inherited::Destroy(); 
	m_ranking = NULL;
	//m_digActorIdList.clear();
	//m_BeAccactedActorList.clear();
	m_sVestEntityName[0] = 0;
	m_hVestEntity = EntityHandle();
	m_nVestAttackTime = 0;
	m_hasDrop = false;

} //销毁一个实体

 //怪物死亡，HP为0的地方调用了这个函数
 void CMonster::OnEntityDeath()
 {
	 //加到场景的死亡列表中，会定期清理
	 Inherited::OnEntityDeath();	 
 }

 //实体销毁
 void CMonster::OnEntityDestroy()
 {
	 MonsterSay(mstDisappear);
	 if (m_pAI)
	 {
		 m_pAI->EntityDestroy();
	 }
 }

 int CMonster::GetKillMonsterAwardAnger(CActor *pActor,PMONSTERCONFIG pMonster)
 {
	 /*
	 int nActorLevel = pActor->GetProperty<int>(PROP_CREATURE_LEVEL); //玩家等级 
	 //玩家等级达到45级才能获得怒气值
	 int minLevel = GetLogicServer()->GetDataProvider()->GetCultureConfig().m_GetAngerMinLevel;
	 if(nActorLevel < minLevel) return 0;

	 int nLevel =pMonster->nLevel; //怪物等级

	 float nDisValue = GetLogicServer()->GetDataProvider()->GetCultureConfig().GetAngerRate(nActorLevel-nLevel);	//等级修正
	 int nAngerRate = GetLogicServer()->GetDataProvider()->GetCultureConfig().m_KillMonsterAngerRate;					//调整系数

	 float nTypeRate = (pMonster->btMonsterType - 1) * GetLogicServer()->GetDataProvider()->GetCultureConfig().m_fAddAngerByMonsterType + 1.0;

	 int nValue = (int)(nDisValue * nAngerRate * nTypeRate);

	 int nResult = 0;

	 if(nValue > 0)
		 nResult = (int)wrand(nValue+1);

	 return nResult;
	 */
	 //先屏蔽怒气
	 return 1;
 }

 float CMonster::GetKillMonsterExpRate(int nActorLevel,PMONSTERCONFIG pMonster)
 {	 
	if( !pMonster->pflags->DenyDieSubExp) 
	{
		int nMonsterLevel = pMonster->nLevel; //怪物等级
		//unsigned int nActorLevel = pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //玩家等级
		int nLevelDis = nMonsterLevel - nActorLevel; //怪物的等级减玩家的等级
		return GetLogicServer()->GetDataProvider()->GetKillMonsterExpReduce().GetKillMonsterExp(nLevelDis); //经验的衰减
	}
	else
	{
		return 1.0;
	}
}

 void CMonster::RealDropItemExp(int nID,CActor * pActor,CScene *pScene, int nPosX,int nPosY,CMonster * pMonster, int nGrowLv) {

	DECLARE_TIME_PROF("CMonster::RealDropItemExp");
	CTeam * pTeam = NULL;
	CActor *member[MAX_TEAM_MEMBER_COUNT]; //附近的队员列表,最大只能是MAX_TEAM_MEMBER_COUNT个
	INT_PTR nTeamMemberCount = 0;
	PMONSTERCONFIG pConfig  = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	//static int nNeedpickTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemNeedTime*1000;
	static int nItemTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemExpireTime;
	//static int nActorpickTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemMasterProtectTime*1000;
	if(NULL == pConfig ) return;
	if(!pScene) return;
	if (pActor != NULL) {

		pTeam = pActor->GetTeam();
		if(pTeam != NULL) {
			nTeamMemberCount = pTeam->GetNearTeamMember(pActor,member);
		}
		//奖励怒气值
		pActor->AddAnger(GetKillMonsterAwardAnger(pActor,pConfig));
	}

	const char* pMonsterName = pMonster ? pMonster->GetEntityName() : pConfig->szName; //怪物的名字
	unsigned int nExp = pConfig->nExp;				//怪物的经验
	int nMonsterLevel = pConfig->nLevel;			//怪物的等级
	int nExpMonsLv = nMonsterLevel;

	int nMonsterCircleLevel = pMonster ? pMonster->GetProperty<unsigned int>(PROP_MONSTER_CIRCLE) : pConfig->nCircle; //怪物的转生等级
	
	if (pConfig->pflags->DenyDieSubExp) {
		nExpMonsLv = 0;			//不进行经验衰减
	}

	EntityFlags* pFlag = pConfig->pflags;

	if(pActor != NULL && nExp > 0) {
		//如果不进行经验衰减的话就直接给原来的经验，否则要进行经验的衰减
		//没有队伍，或者没人和自己共享经验，直接添加给自己
		int nSelfLevel =   pActor->GetProperty<int>(PROP_CREATURE_LEVEL);//玩家的等级
		if(pTeam ==NULL || nTeamMemberCount <= 0 ) {
			pActor->AddExp( nExp ,GameLog::Log_KillMonster, (INT_PTR)nExpMonsLv, 0, (INT_PTR)nMonsterCircleLevel);
		}
		else {

			//float times = GetLogicServer()->GetDataProvider()->GetTeamKillMonsterExp().GetTeamKillMonsterExp(nTeamMemberCount);
			float times = 1/(float)(nTeamMemberCount +1) ;
			nExp = (unsigned int )(nExp * times);
			CActor* pCActor = pTeam->GetCaptin();								//获得队长
			//队伍经验的计算公式为
			//组队打怪获得经验=怪物经验/附近可共享经验的人
			//杀怪者自己获得经验
			pActor->AddExp( nExp ,GameLog::Log_KillMonster, (INT_PTR)nExpMonsLv, 0, (INT_PTR)nMonsterCircleLevel);
			//旁边的人加经验
			for (INT_PTR i = 0; i< nTeamMemberCount; i++) {

				if(member[i]) {
					member[i]->AddExp( nExp,GameLog::Log_TeamKillMonster, (INT_PTR)nExpMonsLv, 0, (INT_PTR)nMonsterCircleLevel);
				}
			}
		}
	}

	//给玩家物品
	INT_PTR dropid= pConfig->nDropid;//掉落列表
	if(dropid <=0 )return;		//没有配置掉落
	const CStdItemProvider& itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	std::vector<DROPGOODS> dropInfos;
	GetLogicServer()->GetDataProvider()->GetDropCfg().GetGiftDropInfoByDropGroupId(dropid, dropInfos);

	int count  = dropInfos.size();
	std::string broadcastmsg = ""; //散财鹿提示
	std::string broadcastCoinMsg = ""; //金币tips
	if(count > 0) {
		for(int i = 0; i < count; i++) {

			DROPGOODS& dropGoods = dropInfos[i];
			GIFTDROPTABLE* pDropTable = GetLogicServer()->GetDataProvider()->GetDropCfg().GetDropTableConfig(dropGoods.info.nDropId);
			if(!pDropTable)
				continue;
			//道具奖励

			if(broadcastmsg != "")
				broadcastmsg += ",";
			
			if(dropGoods.info.nType == qatEquipment) {
				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(dropGoods.info.nId);
				if(!pStdItem)
					continue;
				
				char    cBestAttr[200];  //极品属性
				memset(&cBestAttr, 0, sizeof(cBestAttr));
				int num = dropGoods.v_bestAttr.size();
				for(int j = 0; j < num; j++) {

					char buf[10];
					if(j != 0)
						strcat(cBestAttr, "|");

					DropAttribute data = dropGoods.v_bestAttr[j];
					sprintf(buf, "%d,%d", data.nType, data.nValue);
					strcat(cBestAttr, buf);
				}
				cBestAttr[sizeof(cBestAttr)-1]= '\0';

				INT_PTR nCount= dropGoods.info.nCount;
				pMonster->setHasDrop(true);
				int nNowtime = GetGlobalLogicEngine()->getMiniDateTime();
				if(pActor) //掉落无归属
					((CActor*)pActor)->GetAchieveSystem().ActorAchievementComplete(nAchieveDropItem, dropGoods.info.nCount, pStdItem->m_nIndex);
				//如果掉出多个，要一个一个掉出 --一律掉在地上
				for(int j = 0; j < nCount; j++) {
					CUserItem *pUserItem = GetGlobalLogicEngine()->AllocUserItem(true); //申请一个道具
					if(pUserItem ==NULL) continue;

					pUserItem->wItemId =pStdItem->m_nIndex ;
					pUserItem->wCount = 1;
					pUserItem->btQuality = pStdItem->b_showQuality;
					pUserItem->wPackageType = pStdItem->m_nPackageType;
					memcpy(&pUserItem->cBestAttr, cBestAttr, sizeof(pUserItem->cBestAttr));
					//-----
					// memcpy(&pUserItem->cSourceName, pMonsterName, sizeof(pUserItem->cSourceName));
					if(pActor)
						pUserItem->setSource(CStdItem::iqKillMonster,nNowtime,  pScene->GetSceneId(), nID, pActor->GetEntityName());
					else
						pUserItem->setSource(CStdItem::iqKillMonster,nNowtime,  pScene->GetSceneId(), nID);
					
					CStdItem::AssignInstance(pUserItem,pStdItem);
					CDropItemEntity *pDropItem= CDropItemEntity::CreateDropItem(pScene,nPosX,nPosY,GameLog::clKillMonsterItem,pMonsterName, nItemTime); //
					if(pDropItem) {
						pDropItem->SetItem(pUserItem);
						
						pDropItem->DealDropItemTimeInfo(pActor, pDropTable->nLootType);
						//统计日志
						if(pConfig && pConfig->nMonLog && pActor)
						{
							GetGlobalLogicEngine()->AddKillDropToLog((int)(pActor->GetId()), pActor->GetAccount(), 
							pActor->GetEntityName(), pMonsterName,pScene->GetSceneName(),pStdItem->m_sName,pUserItem->wCount,nPosX,nPosY,pActor->getOldSrvId());
						}
						
						if (pActor && nID != 161 && pStdItem->m_nDropBroadcast != 0 && (pStdItem->m_nDropBroadcast == -1 || pStdItem->m_nDropBroadcast >= GetLogicServer()->GetDaysSinceOpenServer())) {
							
							LPTSTR desc = CUserEquipment::getItemColorDesc(pStdItem);
							if(desc) {
								char descname[100] = {0};
								sprintf_s(descname, sizeof(descname), desc,pStdItem->m_sName);
								char s2[1024] = {0};
								LPCTSTR sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDropMonsterBroadItem);
								if(sFormat2)
								{
									char id2str[1024] = {0};
									sprintf(id2str,"%lld",pUserItem->series.llId); //actotID,guid
									sprintf_s(s2, sizeof(s2), sFormat2, dropGoods.info.nId,id2str,(char*)(descname+1));
								}
								char s1[1024] = {0};
								LPCTSTR sFormat1 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDropMonsterBroad);
								if(sFormat1)
								{
									sprintf_s(s1, sizeof(s1), sFormat1, pActor->GetEntityName(),pScene->GetSceneName(), pMonsterName, s2);
								}
								GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(s1,tstChatSystem,0);
								GetGlobalLogicEngine()->GetChatMgr().addShowItem(pUserItem);
								GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmDropMonsterBroad, tstKillDrop, pActor->GetEntityName(),
								pScene->GetSceneName(), pMonsterName, descname);
							}
						}
					} else {
						GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
					}
				}

				broadcastmsg += pStdItem->m_sName;
				
			} else if( (dropGoods.info.nType == qatMoney) || (dropGoods.info.nType == qatBindMoney) ) {
				INT_PTR nMoneyType = mtCoin;
				int nMonsterDropMaxMoney = 0;

				if (dropGoods.info.nType == qatMoney) {
					nMoneyType = mtCoin;
					nMonsterDropMaxMoney = GetLogicServer()->GetDataProvider()->GetDropItemConfig().nMonsterMaxCoinOneDrop;//GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMonsterMaxCoinOneDrop;
				
				} else if (dropGoods.info.nType == qatBindMoney) {
					nMoneyType = mtBindCoin;
					nMonsterDropMaxMoney = GetLogicServer()->GetDataProvider()->GetDropItemConfig().nMonsterMaxBindCoinOneDrop;
				}

				if (nMonsterDropMaxMoney == 0)	// 防止除数为0
					continue;

				int nTotalCount = dropGoods.info.nCount;
				INT_PTR nDumpCount = nTotalCount/nMonsterDropMaxMoney; //有多少堆最大的
				INT_PTR nLeftCount = nTotalCount - nMonsterDropMaxMoney * nDumpCount; //剩下的
				CDropItemEntity *pDropItem;
				INT_PTR nDropCount = 0;
				pMonster->setHasDrop(true);

				for(INT_PTR i=0; i<= nDumpCount; i++) {

					if(i == nDumpCount ) {
						nDropCount = nLeftCount;
					} else {
						nDropCount = nMonsterDropMaxMoney;
					}

					if(nDropCount ==0) continue; //没有掉落

					pDropItem= CDropItemEntity::CreateDropItem(pScene,nPosX,nPosY,GameLog::clKillMonsterItem,pMonsterName); //
					if(pDropItem) {
						pDropItem->SetMoneyCount((int)nDropCount, (int)nMoneyType);
						pDropItem->DealDropItemTimeInfo(pActor, pDropTable->nLootType);
					}
				}
				LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmMoreCoin);
				if(sFormat)
					broadcastCoinMsg = sFormat;
			} else {

				if (pActor == NULL) continue;

				pActor->GiveAward(dropGoods.info.nType, dropGoods.info.nId, dropGoods.info.nCount, 0,0,0,0,GameLog::Log_KillMonster);
				
			}
		}
		//处理散财鹿tips
		if(broadcastmsg != "" && broadcastCoinMsg != "")
			broadcastmsg += ", " + broadcastCoinMsg;
		
		if(broadcastmsg != "" && nID == 161 && pActor && pScene)
		{
			GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmDropMonsterBroad, tstChatSystem, pActor->GetEntityName(),
				pScene->GetSceneName(), pMonsterName, broadcastmsg.c_str());
			GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmDropMonsterBroad, tstKillDrop, pActor->GetEntityName(),
				pScene->GetSceneName(), pMonsterName, broadcastmsg.c_str());
		}
	}
 }


			
 bool CMonster::RealDropItemByDropGroupId(CScene *pScene, int nPosX,int nPosY,int dropGroupId,int pick_time, int nDropTips) {

	DECLARE_TIME_PROF("CMonster::RealDropItemByDropGroupId");

	
	//static int nNeedpickTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemNeedTime*1000;
	static int nItemTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemExpireTime;
	//static int nActorpickTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemMasterProtectTime*1000;

	const char* pLogName = "Activity" ;
	
	//给玩家物品
	INT_PTR dropid= dropGroupId ;//掉落列表
	if(dropid <=0 )return false ;		//没有配置掉落
	if(!pScene) return false;
	const CStdItemProvider& itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	std::vector<DROPGOODS> dropInfos;
	GetLogicServer()->GetDataProvider()->GetDropCfg().GetGiftDropInfoByDropGroupId(dropid, dropInfos);

	int count  = dropInfos.size();

	if(count > 0) {
		for(int i = 0; i < count; i++) {

			DROPGOODS& dropGoods = dropInfos[i];
			GIFTDROPTABLE* pDropTable = GetLogicServer()->GetDataProvider()->GetDropCfg().GetDropTableConfig(dropGoods.info.nDropId);
			if(!pDropTable)
				continue;
			//道具奖励
			if(dropGoods.info.nType == qatEquipment) {
				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(dropGoods.info.nId);
				if(!pStdItem)
					continue;
				
				char    cBestAttr[200];  //极品属性
				memset(&cBestAttr, 0, sizeof(cBestAttr));
				int num = dropGoods.v_bestAttr.size();
				for(int j = 0; j < num; j++) {

					char buf[10];
					if(j != 0)
						strcat(cBestAttr, "|");

					DropAttribute data = dropGoods.v_bestAttr[j];
					sprintf(buf, "%d,%d", data.nType, data.nValue);
					strcat(cBestAttr, buf);
				}
				cBestAttr[sizeof(cBestAttr)-1]= '\0';

				INT_PTR nCount= dropGoods.info.nCount;


				//如果掉出多个，要一个一个掉出 --一律掉在地上
				int nNowtime = GetGlobalLogicEngine()->getMiniDateTime();
				for(int j = 0; j < nCount; j++) {
					CUserItem *pUserItem = GetGlobalLogicEngine()->AllocUserItem(true); //申请一个道具
					if(pUserItem ==NULL) continue;

					pUserItem->wItemId =pStdItem->m_nIndex ;
					pUserItem->wCount = 1;
					pUserItem->btQuality = pStdItem->b_showQuality;
					pUserItem->wPackageType = pStdItem->m_nPackageType;
					memcpy(&pUserItem->cBestAttr, cBestAttr, sizeof(pUserItem->cBestAttr));
					//-----
					pUserItem->setSource(CStdItem::iqNPCAward,  nNowtime, pScene->GetSceneId());
					CStdItem::AssignInstance(pUserItem,pStdItem);
					CDropItemEntity *pDropItem= CDropItemEntity::CreateDropItem(pScene,nPosX,nPosY,GameLog::cNpcBigTreasure,pLogName, nItemTime,1); 
					//创建item，设置多久以后消失
					if(pDropItem) {
						pDropItem->SetItem(pUserItem);
						pDropItem->DealDropItemTimeInfo(NULL, 666,pick_time);//设置任何人都可以捡取，时间120秒后可以捡取
						
						if (nDropTips && pStdItem->m_nDropBroadcast != 0 && (pStdItem->m_nDropBroadcast == -1 || pStdItem->m_nDropBroadcast >= GetLogicServer()->GetDaysSinceOpenServer())) {
							
							LPTSTR desc = CUserEquipment::getItemColorDesc(pStdItem);
							if(desc) {
								char descname[100] = {0};
								sprintf_s(descname, sizeof(descname), desc,pStdItem->m_sName);
								char s2[1024] = {0};
								LPCTSTR sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDropMonsterBroadItem);
								if(sFormat2)
								{
									char id2str[1024] = {0};
									sprintf(id2str,"%lld",pUserItem->series.llId); //actotID,guid
									sprintf_s(s2, sizeof(s2), sFormat2, dropGoods.info.nId,id2str,(char*)(descname+1));
								}

								char s1[1024] = {0};
								LPCTSTR sFormat1 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nDropTips);
								if(sFormat1)
								{
									sprintf_s(s1, sizeof(s1), sFormat1, s2);
								}
								GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(s1,tstChatSystem,0);

								GetGlobalLogicEngine()->GetChatMgr().addShowItem(pUserItem);
								GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(nDropTips, tstKillDrop,descname);
							}
						}
					} else {
						GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
					}
				}
			} 
		}

	}else return false ;


	return true ;
 }













//怪物掉落一个包裹给玩家，如果是宠物杀死的，就掉落给宠物的主人
void CMonster::DropItemExp(CActor * pActor, int nGrowLv)
{
	DECLARE_TIME_PROF("CMonster::DropItemExp");
	unsigned int nID = GetProperty<unsigned int>(PROP_ENTITY_ID);
	int nPosX,nPosY; //位置
	GetPosition(nPosX,nPosY); //获取坐标
	unsigned int lv = GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	if (CScene* pScene = GetScene())
		RealDropItemExp(nID,pActor,pScene,nPosX,nPosY,this, nGrowLv);
}


bool CMonster::AddDropTaskItemToVester(CActor *pActor, CUserItemContainer::ItemOPParam *pItem)
{ 
	DECLARE_TIME_PROF("CMonster::AddDropTaskItemToVester");
	 CQuestSystem *pQuestSystem = pActor->GetQuestSystem();
	//  if (pQuestSystem->IsCurrQuestItem(pItem->wItemId))
	//  {
	// 	INT_PTR added_count = pActor->GetBagSystem().AddItem(*pItem, GetEntityName(), GameLog::clKillMonsterItem);
	// 	if (added_count != pItem->wCount)
	// 		return false;
	//  }

	 return true;
}


 /*杀怪通知周围玩家，触发周围玩家的任务事件
 注意，
 1、此方法是高频方法，要注意性能；
 2、这里会导致如果死亡见证玩家也是队友，会再计算一次，不过为了性能考虑，且新开服表现不明显，暂时不管
 */
 void CMonster::QuestMonsterToPlayerNearby(CActor* pDoActor)
 {
	DECLARE_TIME_PROF("CMonster::QuestMonsterToPlayerNearby");
	INT_PTR nCount = m_BeAccactedActorList.count();
	if(NULL == pDoActor || nCount <= 0)
	{
		return;
	}
	unsigned int nMyId = pDoActor->GetId();
	unsigned int nMonId = GetId();
	GLOBALCONFIG & gloCfg = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		unsigned int nPlyId = m_BeAccactedActorList[i];
		if(nPlyId == nMyId)
		{
			continue;
		}
		CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nPlyId);
		//以怪物为中心点，判断角色距离此怪物的距离（的平方），过远也不算
		int nDistSqare = (int)GetEntityDistanceSquare(pActor);
		// if (pActor != NULL && nDistSqare <= gloCfg.nSeeKilledDistance)
		// {
		// 	// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtMonster, nMonId, 1);
		// 	// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtSceneKillAnyMon, nMonId, 1);			
		// }
	}
	//m_BeAccactedActorList.clear();
 }

 //被杀死的时候调用的
void CMonster::OnKilledByEntity(CEntity * pKillerEntity)
{
	DECLARE_TIME_PROF("CMonster::OnKilledByEntity");
	if(!m_hasDeathCall)
	{
		CAnimalAI * pAi = GetAI();
		if(pAi)
		{
			pAi->KillBy(); //被杀死了
		}
		else
		{
			OutputMsg(rmError,"OnKilledByEntity pAi=NULL");
		}
		m_hasDeathCall = true;
	}
	
	if(pKillerEntity == NULL) return;
	INT_PTR nType = pKillerEntity->GetType();
	if( nType==enPet)
	{
		CPet * pPet = ((CPet*)pKillerEntity);
		CActor * pOwner = pPet->GetMaster();
		if(pOwner && pOwner->IsInited())
		{
			OnKilledByEntity(pOwner); // 递归
		}
		pPet->AddExp(1); //每次杀死一个怪物，给自己加一点经验
		return;
	}
	else if(nType == enHero) //被英雄杀死的
	{
		CHero * pHero = ((CHero*)pKillerEntity);
		CActor * pOwner = pHero->GetMaster();
		if(pOwner && pOwner->IsInited())
		{
			OnKilledByEntity(pOwner); // 递归
		}
		return;
	}
	else if(nType == enMonster)
	{
		EntityHandle handle = ((CMonster*)pKillerEntity)->GetOwner();
		CEntity *pOwner = GetEntityFromHandle(handle);
		if(pOwner && pOwner->IsInited())
		{
			OnKilledByEntity(pOwner); // 递归
			return;
		}
		/*
		else
		{
			INT_PTR nFubenId = pKillerEntity->GetFuBen()->GetFbId();
			CScene *pScene = pKillerEntity->GetScene();
			if (nFubenId > 0 && pScene)	//如果在副本中,则寻找副本其中某位玩家为击杀者
			{
				CEntityList& list = pScene->GetPlayList();
				CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
				CLinkedNode<EntityHandle> *pNode;
				CLinkedListIterator<EntityHandle> it(list);
				for (pNode = it.first(); pNode; pNode = it.next())
				{
					CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
					if (pEntity && pEntity->GetType() == enActor)
					{
						OnKilledByEntity(pEntity);
						return;
					}		
				}
			}
		}
		*/
	}

	Inherited::OnKilledByEntity(pKillerEntity);
	MonsterSay(mstDead);	
	//处理boss死亡
	int nBossReferId = GetBossReferId();
	GetGlobalLogicEngine()->GetBossMgr().BossDeath(nBossReferId);
	
	if(m_isInited ==false) return;
	//调用怪物死亡触发脚本
	int nMonId = GetId();
	PMONSTERCONFIG pMonConf = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonId);

	if(pMonConf == NULL) return;

	// 这里计算归属者实体		 
	CEntity *pVester = NULL;
	if( m_nForceVesterId > 0 )			//存在强制归属玩家
	{
		pVester = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(m_nForceVesterId);
		if( !pVester )   //强制归属玩家不在线
		{
			//无归属，即便一刀不砍的玩家也能拿掉落物品
			ChangeShowName(12, pMonConf->szName);			//恢复怪物本名
		}
	}
	else
	{
		pVester = GetEntityFromHandle(m_hVestEntity);
	}

	if ( m_nForceVesterId == 0 &&( !pVester || pVester->GetType() != enActor ))		//前提是没有指定强制归属玩家
	{
		pVester = pKillerEntity;	
	}
	//计算boss成长 ---不需要了
	int nGrowLv = 0;
	// if (pMonConf->pflags->CanGrowUp && pMonConf->btMonsterType ==tagMonsterConfig::mtBoss)
	// {
	// 	nGrowLv = CBossSystem::OnKillGrowBoss(nMonId, pVester->GetEntityName(), pMonConf);
	// }

	// 应策划修改，直接最后一刀杀死怪物，就算作谁杀死怪
	//CEntity *pVester = pKillerEntity;

	if (pVester && pVester->GetType() == enActor) // 要给玩家经验（先通知队友，再通知周围玩家）
	{					 		 
		CActor * pActor =(CActor *)pVester;	
		DropItemExp(pActor, nGrowLv);
		pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtMonster, 1, GetProperty<unsigned int> (PROP_ENTITY_ID));
		// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtKillLevelMonster, pMonConf->nLevel, 1, TRUE, ((CActor *)pActor));
		// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtSceneKillAnyMon, pActor->GetSceneID(), 1, TRUE, ((CActor *)pActor));
		if( GetAttriFlag().AttackSeeKill )   //怪物已配置死亡见证
		{
			QuestMonsterToPlayerNearby(pActor);
		}
		pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveKillMonster,1, pMonConf->btMonsterType); //杀怪成就
		pActor->GetAchieveSystem().ActorAchievementComplete(nAchievekillOneMonster,1, pMonConf->nEntityId); //击杀某种怪物
		// 更新宝物命盘XP值
		//((CActor *)pKillerEntity)->GetGemSystem().OnKillEntity(enMonster, (tagMonsterConfig::tagMonsterType)pMonConf->btMonsterType);
	}
	else
	{
		DropItemExp(NULL, nGrowLv);	//掉落无归属
	}

	if (nType == enActor)
	{
		CActor* pActor = (CActor*)pKillerEntity;
		//计算副本中击杀的怪物数量
		CFuBen* pfb = pActor->GetFuBen();
		if (pfb && pfb->IsFb())
		{
			//如果是boss，记录玩家的名字
			if (pMonConf->btMonsterType ==  MONSTERCONFIG::mtBoss || pMonConf->btMonsterType ==  MONSTERCONFIG::mtToumu  || pMonConf->btMonsterType ==  MONSTERCONFIG::mtWorldBoss)
			{
				pfb->AddKillBossName(pActor->GetEntityName());
			}
			pfb->AddKillMonsterCount();
		}
		pActor->ChangeRecordData(rRecordKillMonster);
		pActor->GetGuildSystem()->OnGuildTask(gttKillMonster, nMonId, 1);	//怪可能是被其他的怪打死的，此次必須判定是玩家
		if (pVester && pVester->GetType() == enActor) {
			CActor * pvActor =(CActor *)pVester;	
			if(pActor != pvActor) {
				pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveKillMonster,1, pMonConf->btMonsterType); //杀怪成就
				pActor->GetAchieveSystem().ActorAchievementComplete(nAchievekillOneMonster,1, pMonConf->nEntityId); //击杀某种怪物
			}
			
		} 
		
	}
	
	//这段代码放在OnMonsterKilled之前
	if( pMonConf->bRecordKillTime)
	{
		unsigned int nNow = GetGlobalLogicEngine()->getMiniDateTime();
		GetLogicServer()->GetDataProvider()->GetMonsterConfig().SetBossLastKillTime(nMonId, nNow);		//设置BOSS被击杀时间
	}

	//怪物死亡，需要触发LUA脚本
	if (pMonConf && pMonConf->pflags->bDeathTriggerScript)
	{
		CScriptValueList arg; 
		arg << this;			
		arg << pVester;			//怪物归属者
		arg << nMonId;
		arg <<pKillerEntity;	//怪物击杀者
		GetGlobalLogicEngine()->GetMonFuncNpc()->GetScript().Call("OnMonsterKilled", arg, arg, 0);
	}

	if( pMonConf->btMonsterType == tagMonsterConfig::mtBoss ||
		pMonConf->btMonsterType ==  MONSTERCONFIG::mtToumu ||
		pMonConf->btMonsterType ==  MONSTERCONFIG::mtWorldBoss )		//BOSS被击杀，记录日志
	{
		//记录日志
		if(GetLogicServer()->GetLocalClient())
		{
		}
	}
		
	m_bHasLiveTimeOut = true;			//怪物被击杀，就不再执行生命周期到的逻辑
}


/*
设置怪物的生命周期，
注意，服务器启动时，GetGlobalLogicEngine()->getMiniDateTime()尚未赋值，可能返回0
所以需要判断一下，
	服务器启动后，GetGlobalLogicEngine()->getMiniDateTime()就>0了
VOID CLogicEngine::OnRountine() 方法是先启动运行其他的，然后才设置系统时间
*/
void CMonster::SetLiveTime( UINT nTime )
 {
	 if (nTime > 0)
	 {
		 unsigned int nNow = GetGlobalLogicEngine()->getMiniDateTime();
		 if( nNow > 0 )
		 {
			m_liveTime = nNow + nTime;
		 }
		 else
		 {
			 SYSTEMTIME mCurSysTime;
			 GetLocalTime(&mCurSysTime);
			 CMiniDateTime mCurMiniDateTime;
			 mCurMiniDateTime = mCurMiniDateTime.encode(mCurSysTime);
			 m_liveTime = mCurMiniDateTime + nTime;
		 }
		 m_BossLiveTime = nTime;
	 }
	 else
	 {
		m_liveTime =0;
		m_BossLiveTime = 0;
	 }
 }

 VOID CMonster::LogicRun( TICKCOUNT nCurrentTime )
 {	 
	DECLARE_TIME_PROF("CMonster::LogicRun");
	 Inherited::LogicRun(nCurrentTime);
	 
	 INT_PTR nType = GetType();

	 //精英怪和boss定时定量回血
	 if (nType == enMonster && (m_nMonsterType == tagMonsterConfig::mtBoss || m_nMonsterType == tagMonsterConfig::mtElite))
	 {
		 if( m_t5s.CheckAndSet(nCurrentTime, true))
		 {
			 if (!IsDeath())
			 {
				 if(!GetAttriFlag().DenyAutoAddHp)
				 {
					 int nAddValue = 1;//GetProperty<int>( PROP_MONSTER_HP_RENEW_VALUE);
					 ChangeHP(nAddValue);
				 }
			 }
		 }
	 }

	// 显示怪物归属名字
	if (nType == enMonster || nType == enGatherMonster)
	{
		static const int s_monsterVectResetInterval = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMonsterVectResetInterval;
		if (m_nVestAttackTime > 0 && GetGlobalLogicEngine()->getTickCount() - m_nVestAttackTime  >= s_monsterVectResetInterval)
		{
			m_nVestAttackTime = 0;
			SetVestEntity(EntityHandle());
			if (GetAttriFlag().boShowVestEntityName)
			{
				SetVestEntityName("");
			}
		}
	}

	// 500ms精度刷新
	if (m_500Timer.CheckAndSet(nCurrentTime, true))
	{			 
		if(nType == enMonster  || nType == enGatherMonster )
		{
			if (m_updateFBMonsterProp.CheckAndSet(nCurrentTime, true))
			{
				// 添加到死亡列表
				if (m_liveTime != 0 && m_liveTime < GetGlobalLogicEngine()->getMiniDateTime())
				{
					CScene* pSc = GetScene();
					if (pSc)
					{
					pSc->AddDeathEntity(this);

					//怪物生命期到，需要触发LUA脚本
					if( !m_bHasLiveTimeOut )
					{
						m_bHasLiveTimeOut = true;
						int nMonId = GetId();
						PMONSTERCONFIG pMonConf = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonId);
						if (pMonConf && pMonConf->pflags->bLiveTimeOutTriggerScript)
						{
							CScriptValueList arg; 
							arg << this;
							arg << nMonId;
							arg << GetSceneID();
							GetGlobalLogicEngine()->GetMonFuncNpc()->GetScript().Call("OnMonsterLiveTimeOut", arg, arg, 0);
						}

						if( pMonConf->btMonsterType == tagMonsterConfig::mtBoss ||
							pMonConf->btMonsterType == tagMonsterConfig::mtToumu ||
							pMonConf->btMonsterType == tagMonsterConfig::mtWorldBoss)		//BOSS生命周期到，记录日志
						{
							//记录日志
							if(GetLogicServer()->GetLocalClient())
							{
							}
						}
					}
					}
					//m_BossLiveTime = 0;
					return;
				}

				//死亡10秒以后，如果还没有加到死亡列表里去的话
				if(IsDeath() && !GetAttriFlag().CanReuse )
				{
					if( !IsInDeathList() )
					{
						CScene* pSc = GetScene();
						if (pSc )
						{
							pSc->AddDeathEntity(this);				 
						}
					}
					else
					{
					if(GetDeathTime() +10 < GetGlobalLogicEngine()->getMiniDateTime())
					{
						CScene* pSc = GetScene();
						if (pSc)
						{
							pSc->ClearDeathEntity(GetHandle());
						}
						GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(GetHandle());
						return;
					}
					}
				
					return;
				}

				CScene *pScene = GetScene();
				if (!pScene) return;
				CFuBen *pFB = pScene->GetFuBen();
				if (!pFB || !pFB->IsFb()) return;
				COMMONFUBENCONFIG *pfc = pFB->GetConfig();
				
				// 动态调整怪物属性
				if (!pfc->bDisableMonsterPropAdjust)
				{
				int fubenRecommendLvl = pfc->nRecommandLevel;
				int fubenPlayerAverageLvl = pFB->getPlayerAverageLvl();
				int fubenPlayerCount = pFB->getPlayerCount();
				bool bNeedUpdateProp = false;

				// 检测副本玩家平均等级，如果大于
				if (fubenPlayerAverageLvl > fubenRecommendLvl)  // 避免等级不够推荐等级条件时的队伍平均等级变化导致的刷新 
				{
					int levelDiff = fubenPlayerAverageLvl - GetProperty<unsigned int>(PROP_CREATURE_LEVEL); // 可能为负数
					int oldLevelDiff = GetProperty<unsigned int>(PROP_MONSTER_LEVEL_DIFF);
					if (levelDiff != oldLevelDiff)
					{
						SetProperty<unsigned int>(PROP_MONSTER_LEVEL_DIFF, levelDiff);			// 更新怪物所在副本的玩家平均等级	 
						bNeedUpdateProp = true;
					}
				}

				if (fubenPlayerCount != m_nTeamPlayerCnt)
				{
					m_nTeamPlayerCnt	= fubenPlayerCount;	// 更新怪物所在副本的玩家数量
					bNeedUpdateProp	= true;
				}		

				if (bNeedUpdateProp)
				{			 			 
					CollectOperate(CEntityOPCollector::coRefMonsterDynProp);
				}
				}
			}
		}
	}
}

 void CMonster::Reuse()
 {
	DECLARE_TIME_PROF("CMonster::Reuse");
	 CScene *pScene = GetScene();
	 if (!pScene)
	 {
		 OutputMsg(rmError, _T("Monster reuse error, get scene is null"));
		 return;
	 }
	 if (enNpc == GetType())
	 {
		 OutputMsg(rmError, _T("%s:重用出错，不应该为NPC[%s]"), __FUNCTION__, GetEntityName());
		 return;
	 }
	 pScene->ExitScene(this);
	 // 复用怪物清除怪物的观察者列表
	 GetObserverSystem()->Clear();

	 Inherited::Reuse();
	 m_liveTime = 0;	 
	 int nMonsterId = GetProperty<int>(PROP_ENTITY_ID);
	 PMONSTERCONFIG pConfig =
		 GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
	 if (pConfig)
	 {
		 //((CAnimal*)this)->AddState(esStateDeath);  // do a trick for following property change
		 SetProperty<unsigned int>(PROP_CREATURE_HP,pConfig->cal.m_AttrValues[aMaxHpAdd].uValue);
		 SetProperty<unsigned int>(PROP_CREATURE_MP,pConfig->cal.m_AttrValues[aMaxMpAdd].uValue);
		 SetProperty<unsigned int>(PROP_CREATURE_STATE,0);
		 m_attriFlag = *(pConfig->pflags);//属性标志位
		 if (m_attriFlag.DenyAttackedByActor)
		 {
			 AddState(esStateDenyAttackedByActor);
		 }
		 RefreshStateMask();
	 }
	 SetVestEntity(EntityHandle());
	 if (GetAttriFlag().boShowVestEntityName)
	 {
		SetVestEntityName("");
	 }
	 SetVestAttackTime(0);
	 CEntityManager *em = GetGlobalLogicEngine()->GetEntityMgr();
	 if (em)
		 em->updateMonsterHandle(this);
	 
	 // 检测此怪物所在场景中的刷新时间为0
	 SCENECONFIG *pSceneData = pScene->GetSceneData();
	 unsigned int nBornPointID = GetProperty<unsigned int>(PROP_MONSTER_BORNPOINT);	  
	 if (pSceneData && nBornPointID >=0 && nBornPointID < (unsigned int)pSceneData->vRefreshList.nCount)
	 {				
		 REFRESHCONFIG* pSceneRefreshConfig = pSceneData->vRefreshList.pList + nBornPointID;
		 Assert(pSceneRefreshConfig->nNextRefreshTime == 0);
		 //INT_PTR seed = pSceneRefreshConfig->nPointsCount;
		 //if(seed >0) seed =wrand((unsigned long)seed);	 //随机点
		 //INT_PTR nMonsterPosX = pSceneRefreshConfig->Points[seed].x;
		 //INT_PTR nMonsterPosY = pSceneRefreshConfig->Points[seed].y;
 
		 INT_PTR nMonsterPosX = 0;
		 INT_PTR nMonsterPosY = 0;
		 if ( pScene->GetMobMonsterXY(pSceneRefreshConfig,nMonsterPosX,nMonsterPosY) )
		 {
			 pScene->EnterScene(this, nMonsterPosX, nMonsterPosY);		 
		 }		 
	 }
	 if (m_pAI)
		 m_pAI->EnterInitAI();
	m_hasDeathCall =false;	
	GetAttriFlag().CanReuse = true;

 }


 void CMonster::SetVestEntity(const EntityHandle& handler)
 {
	m_hVestEntity = handler;

	// 设置怪物的归属属性
	const PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(GetProperty<unsigned int>(PROP_ENTITY_ID));
	if ( pMonsterConfig )
	{
		if ( pMonsterConfig->nAscriptionOpen )
		{
			CEntity *pVestEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(m_hVestEntity);
			if ( pVestEntity )
			{
				if ( enActor == pVestEntity->GetType() )
				{
					SetProperty<unsigned int>(PROP_MONSTER_BELONG_ID, pVestEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
					return;
				}
			}
		}
	}

	SetProperty<unsigned int>(PROP_MONSTER_BELONG_ID, 0);
 }

const EntityHandle& CMonster::GetVestEntity()const
{
	return m_hVestEntity;
}

 bool CMonster::InBeAttackActorList(unsigned int nActorId)
 {
	 for(int i=0; i<m_BeAccactedActorList.count(); i++)
	 {
		 if( nActorId == m_BeAccactedActorList[i] )
		 {
			 return true;		//已经在列表中
		 }
	 }
	 return false;
 }

 void CMonster::SetBeAttackActorList(unsigned int nActorId)
 {
	 if(!InBeAttackActorList(nActorId))
	 {
		 m_BeAccactedActorList.add(nActorId);
	 }
 }

 /*
 怪物被攻击
 */
 void CMonster::OnAttacked(CAnimal * pEntity, bool bSetVest)
 {
	 //OutputMsg(rmNormal, "CMonster::OnAttacked: nDemage=%d", nDemage);
	 Inherited::OnAttacked(pEntity);
	 if (pEntity->GetType() == enPet)
	 {
		 CPet * pPet = ((CPet*)pEntity);
		 CActor * pOwner = pPet->GetMaster();
		 if(pOwner && pOwner->IsInited())
		 {
			 OnAttacked(pOwner);
		 }
		 return;
	 }
	 if (pEntity->GetType() == enHero)
	 {
		 CHero* pHero = ((CHero*)pEntity);
		 CActor * pOwner = pHero->GetMaster();
		 if(pOwner && pOwner->IsInited())
		 {
			 OnAttacked(pOwner);
		 }
		 return;
	 }
	 if (pEntity->GetType() == enActor && bSetVest)		//被玩家攻击
	 {
		 if (GetVestEntity().IsNull())		// 第一次被攻击时设置怪物归属
		 {	
			 SetVestEntity(pEntity->GetHandle());
			 if (GetAttriFlag().boShowVestEntityName)
			 {
				 SetVestEntityName(pEntity->GetEntityName());
			 }
			 m_nVestAttackTime = GetGlobalLogicEngine()->getTickCount();
		 }
		 else
		 {
			 EntityHandle hEntity = pEntity->GetHandle();
			 if (hEntity != m_hVestEntity)
			 {
				 CEntity *pVester = GetEntityFromHandle(m_hVestEntity);
				 if (!pVester || pVester->GetScene() != GetScene())
				 {
					 SetVestEntity(hEntity);
					 if (GetAttriFlag().boShowVestEntityName)
					 {
						 SetVestEntityName(pEntity->GetEntityName());
					 }
				 }
			 }
			 else
			 {
				 m_nVestAttackTime = GetGlobalLogicEngine()->getTickCount();
			 }
		 }
		 m_nAttackedTime =  GetGlobalLogicEngine()->getMiniDateTime();
		 CEntity *pMaster = GetEntityFromHandle(GetOwner());
		 if (pMaster && pMaster->GetType() == enActor) 
		 {
		     ((CActor*)pMaster)->GetPkSystem().SendPkToClient();
		 }
		 SetBeAttackActorList(pEntity->GetId());
	 }
 }

void CMonster::MonsterSay(MonsterSayType sayType, INT_PTR nSayIdx)
{
	/*DECLARE_FUN_TIME_PROF()*/
	if (sayType == mstInvalid || sayType == mstMax) return;
	unsigned int nMonsterId = GetProperty<unsigned int>(PROP_ENTITY_ID);
	PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
	if (!pConfig) return;

	MonsterSayList &sayList = pConfig->monsterSayList;
	MonsterSayTypeConfig &sayTypeInfo = sayList.vecSayTypeInfo[sayType];
	if (sayTypeInfo.nCount <= 0 || nSayIdx >= sayTypeInfo.nCount)
		return;

	MonsterSayInfo *pSayInfo = NULL;
	if (sayType != mstHpLow)
		pSayInfo = &sayTypeInfo.pMonsterSayCfg[0];
	else
		pSayInfo = &sayTypeInfo.pMonsterSayCfg[nSayIdx];
	if (pSayInfo->nRate <= 0)
		return;

	// Idle time limit
	if (mstIdle == sayType && GetGlobalLogicEngine()->getTickCount() < m_nNextIdleSayTime)
		return;

	if (sayType == mstIdle)
		PostponeMonsterIdleSay();

	// random rate limit		
	if (wrand(101) > (unsigned int)pSayInfo->nRate)
		return;

	unsigned int nContentCnt = pSayInfo->nCount;
	if (nContentCnt <= 0) return;
	int nIdx = 0;
	if (nContentCnt <= 1)
		nIdx = 0;
	else
	{
		nIdx = wrand(nContentCnt);
		if (sayType == m_nPreSayType)
		{
			if (nIdx == m_nPreSayContentIdx)
			{
				nIdx++;
				if (nIdx >= (int)nContentCnt) 
					nIdx = 0;
			}		
		}
		else
		{
			m_nPreSayType = sayType;		
		}
	}
	
	m_nPreSayContentIdx = nIdx;	
	// 发言内容
	const char *pMsg = pSayInfo->pMsgList[nIdx];
	char szTransmit[80] = {0};
	if (pSayInfo->bTransmit)
	{
		int nLimitTime = pSayInfo->nLimitTime;
		if (0 != nLimitTime)
			nLimitTime = nLimitTime + GetGlobalLogicEngine()->getMiniDateTime();
		int x = pSayInfo->nPosX; 
		int y = pSayInfo->nPosY;
		if (0 == x || 0 == y)
			GetPosition(x, y);
		CScene *pScene = GetScene();
		if (pScene)
		{
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpTransmitPattern);
			sprintf(szTransmit, sFormat, pScene->GetSceneName(), x, y, GetEntityName(), nLimitTime);
		}
	}
	char szMsg[1024] = {0};
	sprintf(szMsg, "%s%s", pMsg, szTransmit);
	Say(pSayInfo->nBCType, szMsg, (tagTipmsgType)(pSayInfo->nTipType), (unsigned int)pSayInfo->nLimitLev);	
}


bool CMonster::SetOwnerName( LPCTSTR sName)
{
	if (sName == NULL)
		return false;
	
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	if(pActor ==NULL) return false;
	m_nOwnerActorId = pActor->GetId();
	m_hOwner = pActor->GetHandle();

	LPCTSTR sShowName = GetShowName();
	char name[128];
	strcpy(name,sName);
	strcat(name,"-");
	strcat(name,sShowName);
	SetShowName(name); //设置名字

	if(IsInited())
	{
		//ChangeName(GetEntityName());
		ChangeName(GetShowName());
		return true;
	}
	else
	{
		return false;
	}
}


void CMonster::ResetShowName(LPCTSTR sName)
{
	if (sName == NULL)
		return;

	char *pShowName = (char*)GetShowName();

	const char *sPtr = strchr(pShowName, '-');
	if (sPtr != NULL)
	{
		char name[128];
		INT_PTR nLen = sPtr - pShowName + 1;
		strncpy(name, pShowName, nLen);
		name[nLen] = _T('\0');
		strcat(name, sName);
		SetShowName(name); //设置名字
	}
	else
	{
		SetShowName(sName); //设置名字
	}
	
	//如果已经进入游戏了，需要广播一下
	if(IsInited())
	{
		ChangeName(GetShowName());
	}
}


/* 
* Comments: 改变HP
* Param int nValue: <0 减HP，>0	加HP
* Param CEntity * pKiller: 
* Param bool bIgnoreDamageRedure: 
* Param bool bIgnoreMaxDropHp: 
* @Return void:  
*/
void CMonster::ChangeHP(int nValue,CEntity * pKiller,bool bIgnoreDamageRedure, bool bIgnoreMaxDropHp,bool boSkillResult,int btHitType)
{
	PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(GetId());
	if (!pConfig)
	{
		return;
	}
	unsigned int nOldHp =  GetProperty<unsigned int >(PROP_CREATURE_HP);
	unsigned int nMaxHp =  GetProperty<unsigned int >(PROP_CREATURE_MAXHP);
	if(nOldHp > nMaxHp)
		nOldHp = nMaxHp;
	if (!m_bHasHPChangeSayConfig)
		Inherited::ChangeHP(nValue,pKiller,bIgnoreDamageRedure,bIgnoreMaxDropHp,boSkillResult,btHitType);
	else
	{
		unsigned int nOldVal =  GetProperty<unsigned int >(PROP_CREATURE_HP);
		Inherited::ChangeHP(nValue,pKiller,bIgnoreDamageRedure,bIgnoreMaxDropHp,boSkillResult,btHitType);
		unsigned int nCurVal =  GetProperty<unsigned int >(PROP_CREATURE_HP);
		if (nOldVal <= nCurVal) return;

		unsigned int nMaxVal =  GetProperty<unsigned int >(PROP_CREATURE_MAXHP);
		unsigned int nID = GetProperty<unsigned int>(PROP_ENTITY_ID);
	
		MonsterSayTypeConfig &sayTypeConfig = pConfig->monsterSayList.vecSayTypeInfo[mstHpLow];
		if (sayTypeConfig.nCount <= 0)
			return;
		for (INT_PTR i = 0; i < sayTypeConfig.nCount; i++)
		{
			MonsterSayInfo &si = sayTypeConfig.pMonsterSayCfg[i];
			if (si.nRate > 0)
			{
				float nRate = si.nParam / 100.0f;
				unsigned int nDivPos = (unsigned int)(nRate * nMaxVal);
				if (nOldVal > nDivPos && nCurVal <= nDivPos)
				{
					MonsterSay(mstHpLow, i);
					break;
				}
			}
		}
		
		/*MonsterSayInfo &si = pConfig->monsterSayList.vecSayInfo[mstHpLow];		
		if (si.nRate > 0)
		{
			float nRate = si.nParam / 100.0f;
			unsigned int nDivPos = unsigned int(nRate * nMaxVal);
			if (nOldVal > nDivPos && nCurVal <= nDivPos)
			{
				MonsterSay(mstHpLow);
			}
		}*/
	}

	unsigned int nCurHp =  GetProperty<unsigned int >(PROP_CREATURE_HP);
	int nDropHp = nOldHp - nCurHp;
	if(m_nMonsterType == tagMonsterConfig::mtWorldBoss && nDropHp > 0)
	{
		if (pKiller && pKiller->GetType() == enActor)
		{
			CActor* pActor = ((CActor*)pKiller);
			CScriptValueList paramList;
			paramList << nDropHp; //damage
			int isKiller = 0;
			if(nCurHp  <= 0)
				isKiller = 1;
			paramList << isKiller; //isKiller
			paramList << GetGlobalLogicEngine()->getMiniDateTime();
			pActor->OnEvent(aeHurtMonster, paramList,paramList);
		}
	}
	//boss 收到伤害 触发
	// if(m_nBossReferId > 0 && nDropHp > 0)
	// {
	// 	CActor *pActor = NULL;
	// 	INT_PTR nEntityType = pEntity->GetType();
	// 	if (nEntityType == enPet)	
	// 	{
	// 		pActor = ((CPet*)pEntity)->GetMaster();	//主人
	// 	}
	// 	if (nEntityType == enHero)
	// 	{
	// 		pActor = ((CHero*)pEntity)->GetMaster();	//主人
	// 	}
	// 	if (nEntityType == enActor)
	// 	{
	// 		pActor  =(CActor*)pEntity;
	// 	}
	// 	if (pActor)
	// 	{
	// 		GetGlobalLogicEngine()->GetBossMgr().BossCallLuaScript(m_nBossReferId, CBossSystem::enOnAttackBoss, pActor);
	// 	}
	// }

	if (m_ranking && nDropHp > 0)
	{
	    UpdateMonsterRanking(pKiller, nDropHp);
	}	
	if ((pConfig->btMonsterType == MONSTERCONFIG::mtBoss || 
		pConfig->btMonsterType == MONSTERCONFIG::mtToumu || 
		pConfig->btMonsterType == MONSTERCONFIG::mtWorldBoss
	)
	&& nDropHp > 0)
	{
		UpdateMonsterHatred(pKiller, nDropHp);
	}
}


void CMonster::InitPriorAttackTarget(CVector<int> &monsterList)
{
	unsigned int nID = GetProperty<unsigned int>(PROP_ENTITY_ID);
	PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	if (!pConfig) return;
	for (INT_PTR i = 0; i < pConfig->priorAttackTarget.nCount; i++)
	{
		monsterList.add(pConfig->priorAttackTarget.pTargetIDList[i]);
	}
}

static const LPCTSTR g_szMosterRankPattern = _T("monster_rank_%d.txt");	//怪物排行榜名称	必须与脚本保持一致
static const INT_PTR g_nMosterRankItemCount = 100;			//怪物排行榜排行数量
static const LPCTSTR g_szMosterRankColumn_0 = _T("username");	//怪物排行榜第一列
static const LPCTSTR g_szMosterRankColumn_1 = _T("param");	//怪物排行榜第一列

void CMonster::RegisterMonsterRank()
{
	// unsigned int nID = GetProperty<unsigned int >(PROP_ENTITY_ID);
	// PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	// if (!pConfig || !(pConfig->bCreateRank)) return;
	// TCHAR name[1024] = {0};
	// sprintf(name, g_szMosterRankPattern, nID); //获取玩家的ID,拼接成完整的排行榜名称
	// CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	// CRanking *pRanking = RankingMgr.GetRanking(name);
	// //初始化交给脚本处理
	// if (!pRanking)
	// {
	// 	pRanking = RankingMgr.Add(name, g_nMosterRankItemCount, 0);
	// 	if (pRanking)
	// 	{
	// 		if (!pRanking->Load(name))
	// 		{
	// 			pRanking->AddColumn(g_szMosterRankColumn_0,-1);
	// 			pRanking->AddColumn(g_szMosterRankColumn_1,-1);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		OutputMsg(rmError, _T("create monster rank error! id = %"), nID);
	// 		return;
	// 	}
	// }
	
	// m_ranking = pRanking;
}

void CMonster::UpdateMonsterRanking(CEntity *pEntity, INT_PTR nNewVal)
{
	DECLARE_TIME_PROF("CMonster::UpdateMonsterRanking");
	if (!pEntity || !m_ranking) return;
	CActor *pActor = (CActor *)pEntity;
	if (pEntity->GetType() == enPet)	//如果是宠物打出的伤害 则计算到主人身上
	{
		pActor = ((CPet*)pEntity)->GetMaster();	//主人
	}
	if (pEntity->GetType() == enHero)
	{
		pActor = ((CHero*)pEntity)->GetMaster();	//主人
	}
	if (pActor && pActor->GetType() == enActor)
	{
		unsigned int nActorId = pActor->GetId();
		CRankingItem *pItem = m_ranking->GetPtrFromId(nActorId);
		if (pItem)
		{
			m_ranking->Update(nActorId, nNewVal);
		}
		else
		{
			pItem = m_ranking->AddItem(nActorId, nNewVal);
			if (pItem)
			{
				pItem->SetSub(0,pActor->GetEntityName());
			}
		}
		m_ranking->Save();
	}	
}
INT_PTR CMonster::GetMonsterHetred(CEntity* pEntity)
{
	if (!pEntity) return 0;
	CActor *pActor = NULL;
	INT_PTR nEntityType = pEntity->GetType();
	if (nEntityType == enPet)	
	{
		pActor = ((CPet*)pEntity)->GetMaster();	//主人
	}
	if (nEntityType == enHero)
	{
		pActor = ((CHero*)pEntity)->GetMaster();	//主人
	}
	if (nEntityType == enActor)
	{
		pActor  =(CActor*)pEntity;
	}
	if (pActor)
	{
		unsigned int nActorId = pActor->GetId();
		MONSTERHATRED* pHatred = NULL;
		for (int i =0 ; i< m_hatredList.count(); i++)
		{
			MONSTERHATRED& hatred = m_hatredList[i];
			if (hatred.nActorId == nActorId && hatred.nEntityType == nEntityType)
			{
				pHatred = &hatred;
				break;
			}
		}
		unsigned int nHatredKeepTime = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMonsterHatredKeepTime;
		unsigned int nCurrTime = GetGlobalLogicEngine()->getMiniDateTime();
		if (pHatred && nCurrTime - pHatred->nLastAttackTime <= nHatredKeepTime)
		{
			return pHatred->nDamageValue;
		}
	}
	return 0;
}
void CMonster::UpdateMonsterHatred(CEntity *pEntity, INT_PTR nNewVal)
{
	DECLARE_TIME_PROF("CMonster::UpdateMonsterHatred");
	if (!pEntity) return;
	CActor *pActor = NULL;
	INT_PTR nEntityType = pEntity->GetType();
	unsigned int nHatredKeepTime = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMonsterHatredKeepTime;
	unsigned int nCurrTime = GetGlobalLogicEngine()->getMiniDateTime();
	if (nEntityType == enPet)	
	{
		pActor = ((CPet*)pEntity)->GetMaster();	//主人
	}
	if (nEntityType == enHero)
	{
		pActor = ((CHero*)pEntity)->GetMaster();	//主人
		//战神按比例增加仇恨，其他跟伤害1:1
		//nNewVal = (INT_PTR)(nNewVal* GetLogicServer()->GetDataProvider()->GetGlobalConfig().fMonsterHatredHeroRate);
	}
	if (nEntityType == enActor)
	{
		pActor  =(CActor*)pEntity;
	}
	if (pActor)
	{
		unsigned int nActorId = pActor->GetId();
		bool isFind = false;
		for (int i =0 ; i < m_hatredList.count(); i++)
		{
			MONSTERHATRED& hatred = m_hatredList[i];
			if (hatred.nActorId == nActorId && hatred.nEntityType == nEntityType)
			{
				isFind = true;
				if (nCurrTime - hatred.nLastAttackTime <= nHatredKeepTime)
				{
					hatred.nDamageValue += nNewVal;
				}
				else
				{
					hatred.nDamageValue = nNewVal;
				}
				
				hatred.nLastAttackTime = nCurrTime;
				OutputMsg(rmTip,_T("[%s] 攻击boss:%d，时间 %d"),pActor->GetEntityName(),m_nBossReferId,hatred.nLastAttackTime);
				
				break;
				//pActor->GetBossSystem().SetHatredBossData(this, hatred);
			}
		}
		if (!isFind)
		{
			MONSTERHATRED newHatred;
			newHatred.nActorId = nActorId;
			newHatred.nDamageValue = nNewVal;
			newHatred.nEntityType =(BYTE)nEntityType;
			newHatred.nLastAttackTime = nCurrTime;
			m_hatredList.add(newHatred);

			//pActor->GetBossSystem().SetHatredBossData(this, newHatred);
		}
	}
	
}
void CMonster::SetVestEntityName(LPCTSTR sName)
{
	if(sName ==NULL) return;
	if (m_nForceVesterId > 0) return;
	if (strcmp(sName,m_sVestEntityName) != 0)
	{
		_asncpytA(m_sVestEntityName,sName);
		//OutputMsg(rmSystem,_T("Vest Name: %s"), m_sVestEntityName);
		CVector<EntityHandle> vecNearEntityList;
		GetNearActorList(vecNearEntityList);
		INT_PTR nEntityCount = vecNearEntityList.count();
		if (nEntityCount > 0)
		{
			CEntityManager * pMgr = GetGlobalLogicEngine()->GetEntityMgr();
			for (int i = 0; i < nEntityCount; i++)
			{
				CEntity * pActor = pMgr->GetEntity(vecNearEntityList[i]);
				if (pActor && pActor->GetType() == enActor)
				{
					CActorPacket ap;
					CDataPacket & outPack = ((CActor*)pActor)->AllocPacket(ap);
					outPack << (BYTE)enMiscSystemID << (BYTE)sVestName;
					outPack.writeString(m_sVestEntityName);
					ap.flush();	
				}
			}
		}
	}
}

LPCSTR CMonster::GetVestEntityName()
{
	return m_sVestEntityName;
}


void CMonster::SetForceVesterId(unsigned int nForceVesterId) 
{ 
	if( nForceVesterId > 0 )			
	{
		CEntity *pVester = NULL;
		pVester = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nForceVesterId);
		if( pVester )  
		{
			SetVestEntityName(((CActor*)pVester)->GetEntityName());
			m_nForceVesterId = nForceVesterId;
		}
	}
}
