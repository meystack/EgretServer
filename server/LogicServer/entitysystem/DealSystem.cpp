#include "StdAfx.h"
#include "DealSystem.h"

CDealSystem::CDealSystem()
{
	ClearDealData();
	m_nActorIdCds.clear();
	m_nApplyCount = 0;
}

void CDealSystem::Destroy()
{
	if (m_boDealing)
		CancelDeal();
	Inherited::Destroy();
	m_nActorIdCds.clear();
}

void CDealSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet)
{
	
	switch(nCmd)
	{
	case cDealSystemRequestDeal: ClientRequestDeal(packet); break;
	case cDealSystemReplyDeal: ClientReplyDeal(packet); break;
	case cDealSystemAddDealItem: ClientDealAddItem(packet); break;
	case cDealSystemAddNumber: ClientChangeDealCoin(packet); break;
	case cDealSystemLockDeal: ClientLockDeal(packet); break;
	case cDealSystemCancelDeal: ClientCancelDeal(packet); break;
	case cDealSystemConfimDeal: ClientConfimDeal(packet); break;
	}
}

bool CDealSystem::FindDealItem(const CUserItem *pUserItem)
{
	INT_PTR i;
	for (i=MaxDealItemCount-1; i>-1; --i)
	{
		if (m_DealItems[i] == pUserItem)
			return true;
	}
	return false;
}

bool CDealSystem::ValidateDealItems()
{
	int nCount;
	for (INT_PTR i=MaxDealItemCount-1; i>-1; --i)
	{
		if(m_DealItemList[i] > 0 && !m_DealItems[i])
			return false;
		if(!m_DealItems[i])
			continue;
		CUserItem *pUserItem = m_pEntity->GetBagSystem().FindItemByGuid(m_DealItems[i]->series);
		if (!pUserItem)
			return false;
	}
	return true;
}

void CDealSystem::CompleteDeal()
{
	if(!m_pDealTarget || !m_pEntity) return;
	INT_PTR i;
	CUserItem *pUserItem;
	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	tagEditionInfo editionInfo = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo();
	CDealSystem &dealSys = m_pDealTarget->GetDealSystem();

	bool isGmDealing = m_pDealTarget->GetGmLevel() >0; //是否是Gm在交易
	
	//取得对方交易的钱币
	char sLogText[127];	
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tmLogDealWith);
	if ( dealSys.m_nDealCoin > 0 )
	{
		int nDues = consignmentProvider.nDues * dealSys.m_nDealCoin / 100;
		if(nDues == 0)
			nDues = 1;
		sprintf_s(sLogText, sizeof(sLogText), sFormat, m_pDealTarget->GetEntityName());
		m_pEntity->ChangeMoney(mtCoin, (int)dealSys.m_nDealCoin - nDues, GameLog::Log_Deal, 0, sLogText, true);

		GetGlobalLogicEngine()->AddDealToLog(GameLog::Log_Deal,
			m_pEntity->GetId(), m_pDealTarget->GetId(), 
			0,0,mtCoin,(int)dealSys.m_nDealCoin, nDues);

		m_pEntity->SendTipmsgFormatWithId(tmCoinTaxRateGet, tstUI);

		sprintf_s(sLogText, sizeof(sLogText), sFormat, m_pEntity->GetEntityName());
		m_pDealTarget->ChangeMoney(mtCoin,(int)(dealSys.m_nDealCoin) * -1, GameLog::Log_Deal, 0, sLogText, true);

		dealSys.m_nDealCoin = 0;
	}

	if ( dealSys.m_nDealYb > 0 )
	{
		int nDuty = consignmentProvider.nDuty * dealSys.m_nDealYb / 100;
		if(nDuty == 0)
			nDuty = 1;

		sprintf_s(sLogText, sizeof(sLogText), sFormat, m_pDealTarget->GetEntityName());
		m_pEntity->ChangeMoney(mtYuanbao, (int)dealSys.m_nDealYb-nDuty, GameLog::Log_Deal, 0, sLogText, true);
		if(editionInfo.nStart && ((CActor*)m_pDealTarget)->GetMiscSystem().GetMaxColorCardLevel() < editionInfo.nPrivilege)
		{
			m_pDealTarget->ChangeMoney(mtJyQuota, (int)((int)dealSys.m_nDealYb*-1*(editionInfo.nProportion/100.0)), GameLog::Log_Deal, 0, sLogText, true);
		}
		m_pEntity->SendTipmsgFormatWithId(tmYbTaxRateGet, tstUI);

		GetGlobalLogicEngine()->AddDealToLog(GameLog::Log_Deal,
			m_pEntity->GetId(), m_pDealTarget->GetId(), 
			0,0,mtYuanbao,(int)dealSys.m_nDealYb,nDuty,((CActor*)m_pEntity)->getOldSrvId());

		sprintf_s(sLogText, sizeof(sLogText), sFormat, m_pEntity->GetEntityName());
		m_pDealTarget->ChangeMoney(mtYuanbao, (int) (dealSys.m_nDealYb) * -1, GameLog::Log_Deal, 0, sLogText, true);

		dealSys.m_nDealYb = 0;
	}

	//取得对方交易的物品
	CUserBag& targetBag = m_pDealTarget->GetBagSystem();
	CUserBag& myBag = m_pEntity->GetBagSystem();
	for (i=dealSys.MaxDealItemCount-1; i>-1; --i)
	{
		if(!dealSys.m_DealItems[i])
			continue;
		pUserItem = targetBag.FindItemByGuid(dealSys.m_DealItems[i]->series);
		if ( pUserItem )
		{
			//只有移除成功才给自己加上物品
			if (targetBag.RemoveItem(dealSys.m_DealItems[i]->series,m_pEntity->GetEntityName(),GameLog::Log_Deal,false))
			{
				const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
				//如果物品需要记录流通日志，则需要向日志服务器发送记录日志的数据
				// if ( pStdItem && pStdItem->m_Flags.recordLog)
				// {
				// 	CUserBag::LogChangeItemCount(pUserItem, pStdItem, -pUserItem->wCount, m_pEntity->GetEntityName(), m_pDealTarget->GetId(), m_pDealTarget->GetEntityName(), GameLog::Log_Deal, ((CActor*)m_pEntity)->GetAccount());
				// }
				myBag.AddItem(pUserItem, m_pDealTarget->GetEntityName(), GameLog::Log_Deal,true,false);
			}
			//和GM交易商城物品，相当于从商城买东西，用于外网的一些托的操作
			if(isGmDealing)
			{
				CGameStore::PMERCHANDISE pMerchandise = GetLogicServer()->GetDataProvider()->GetGameStore().GetShopItem(pUserItem->wItemId);
				if(pMerchandise)
				{
					m_pEntity->m_consumeyb += pMerchandise->dwPrice * pUserItem->wCount;
				}
			}
				
			GetGlobalLogicEngine()->AddDealToLog(GameLog::Log_Deal,
				m_pEntity->GetId(), m_pDealTarget->GetId(),
				pUserItem->wItemId,pUserItem->wCount,0,0,0,((CActor*)m_pEntity)->getOldSrvId());
		}
		dealSys.m_DealItems[i] = NULL;
	}
	dealSys.m_nItemCount = 0;

	//发送交易完成的消息
	CActorPacket ap;
	CDataPacket &pack = m_pEntity->AllocPacket(ap);
	pack << (BYTE)GetSystemID() << (BYTE)sDealSystemDealComplete;
	ap.flush();
	m_pEntity->SendTipmsgFormatWithId(tmDealComplete, tstUI);
}

void CDealSystem::CancelDeal(bool boComplete)
{
	if ( !m_boDealing )
		return;
	m_boDealing = false;
	if(m_pEntity ==NULL) return ;
	//让对方取消交易
	if (m_pDealTarget && m_pDealTarget->IsInited())
	{
		m_pDealTarget->GetDealSystem().CancelDeal(boComplete);
	}
	
	//清除交易数据
	ClearDealData();

	if (!boComplete)
	{
		//发送交易取消的消息
		CActorPacket ap;
		CDataPacket& pack = m_pEntity->AllocPacket(ap);
		pack << (BYTE)GetSystemID() << (BYTE)sDealSystemDealCanceled;
		ap.flush();
		m_pEntity->SendTipmsgFormatWithId(tmDealCancel, tstUI);
	}

	
}

void CDealSystem::ClientRequestDeal(CDataPacketReader &packet)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	
	EntityHandle handle;
	bool boCanDeal = false;
	CActor *pTarget = NULL;
	if (GetLogicServer()->IsCommonServer())
	{
		m_pEntity->SendTipmsgFormatWithId(tmAreaCannotTrade, tstUI);
		return;
	}
	
	unsigned int nActorId = 0;
	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	//我是否正在交易
	if (m_boDealing)
	{
		//你已经在交易了，请先完成本次交易
		m_pEntity->SendTipmsgFormatWithId(tmYouAreDealingNow, tstUI);
		return;
	}
	else
	{
		packet >>nActorId;
		char name[32];
		packet.readString(name,sizeof(name));
		name[sizeof(name) -1]=0;
		//查找目标玩家
		if(nActorId > 0)
		{
			pTarget = (CActor*)GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
		}
		if(!pTarget)
		{
			pTarget = (CActor*)GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetActorPtrByName(name);
		}

		

		//如果目标玩家不存在或对象不是一个玩家则返回交易被拒绝
		if ( !pTarget || pTarget->GetType() != enActor )
		{
			//对方不在线，无法请求交易
			m_pEntity->SendTipmsgFormatWithId(tmDealTargetNotExists, tstUI);
			return ;
		}

		if(pTarget->GetId() == ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID))
		{
			m_pEntity->SendTipmsgFormatWithId(tmFriendnotaddSelf, tstUI); 
			return;
		}

		//判断双方距离
		if (!m_pEntity->CheckTargetDistance(pTarget, consignmentProvider.nPrivateDealDistance))
		{
			m_pEntity->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
			return;
		}
		char limit[4];
		sprintf(limit, "%d", consignmentProvider.nMinDealLevel);
		//最低的交易等级
		if(!((CActor*)m_pEntity)->CheckLevel(consignmentProvider.nMinDealLevel, 0))
		{
			m_pEntity->SendTipmsgFormatWithId(tmDealSelfLevelLimit, tstUI, limit);
			return;
		}

		// int nCircleLevel = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
		// int nCardLv = ((CActor*)m_pEntity)->GetMiscSystem().GetMaxColorCardLevel() ;
		// if (nCircleLevel < consignmentProvider.nDealLevel[0] && nCardLv < consignmentProvider.nDealLevel[1])
		// {
		// 	((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmLimitDeal, tstUI);
		// 	return ;
		// }

		if(!((CActor*)pTarget)->CheckLevel(consignmentProvider.nMinDealLevel, 0))
		{
			m_pEntity->SendTipmsgFormatWithId(tmDealTargetLevelLimit, tstUI, limit);
			return;
		}
		if(!DealInCd(pTarget->GetId()))
		{
			m_pEntity->SendTipmsgFormatWithId(tmDealApplyCdLimit, tstUI);
			return;
		}
		// if (m_pEntity->HasState(esStateStall))
		// {
		// 	m_pEntity->SendTipmsgFormatWithId(tmYouInStall, ttFlyTip);
		// 	return;
		// }
		// if (!pTarget->HasMapAttribute(asDealArea))
		// {
		// 	m_pEntity->SendTipmsgFormatWithId(tmDealNotAtSeafArea, tstUI);
		// 	return;
		// }

		// if (!m_pEntity->HasMapAttribute(asDealArea))
		// {
		// 	m_pEntity->SendTipmsgFormatWithId(tmDealNotAtSeafArea, tstUI);
		// 	return;
		// }
		//对方是否正在交易
		if (pTarget->GetDealSystem().IsDealing())
		{
			//对方正在交易，请稍后再试
			m_pEntity->SendTipmsgFormatWithId(tmTargetIsDealing, tstUI);
			return;
		}

		if(pTarget->GetDealSystem().getApplyCount() >= 3)
		{
			m_pEntity->SendTipmsgFormatWithId(tmDealApplyTooMuch, tstUI);
			return;
		}

		boCanDeal = true;
	}
	//返回交易被拒绝
	if ( !boCanDeal )
	{
		CActorPacket ap;
		CDataPacket &pack = m_pEntity->AllocPacket(ap);
		pack << (BYTE)GetSystemID() << (BYTE)sDealSystemDealRefused;
		pack.writeString(pTarget->GetEntityName());
		ap.flush();
	}
	else
	{
		//向对方发送交易请求
		CActorPacket ap;
		CDataPacket &pack = pTarget->AllocPacket(ap);
		pack << (BYTE)GetSystemID() << (BYTE)sDealSystemDealRequest << m_pEntity->GetId();
		pack.writeString(m_pEntity->GetEntityName());
		ap.flush();
		int nCd = time(NULL)+consignmentProvider.nDealCd;
		pTarget->GetDealSystem().AddApplyIdCd(m_pEntity->GetId(), nCd);
		m_nActorIdCds[pTarget->GetId()] =nCd;
		//pTarget->GetDealSystem().m_nApplyCount++;
		//向我发送已经邀请对方进行交易的提示
		m_pEntity->SendTipmsgFormatWithId(tmAlreadyInvaiteDeal, tstUI);
	}
}

void CDealSystem::ClientReplyDeal(CDataPacketReader &packet)
{
	if(!m_pEntity) return;
	
	// EntityHandle handle;
	unsigned int nActorId = 0;
	bool boStartDeal = false;
	CActor *pTarget = NULL;
	if (GetLogicServer()->IsCommonServer())
	{
		m_pEntity->SendTipmsgFormatWithId(tmAreaCannotTrade, tstUI);
		return;
	}
	// if (!m_pEntity->HasMapAttribute(asDealArea))
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmAreaCannotTrade, tstUI);
	// 	return;
	// }

	//如果我已经在交易了，则返回正在交易的提示
	if ( m_boDealing )
	{
		//你已经在交易了，请先完成本次交易
		m_pEntity->SendTipmsgFormatWithId(tmYouAreDealingNow, tstUI);
		return;
	}
	else
	{
		BYTE boAccept = 0;
		packet >> nActorId >> boAccept;
		ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
		//查找目标玩家
		pTarget = (CActor*)GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
		//如果目标玩家不存在或对象不是一个玩家则返回申请人已经离开
		if ( !pTarget || pTarget->GetType() != enActor )
		{
			//交易对方已经离开，交易被取消
			m_pEntity->SendTipmsgFormatWithId(tmDealRequesterHasLeaved, tstUI);
			return; 
		}
		if(pTarget->GetId() == ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID))
		{
			m_pEntity->SendTipmsgFormatWithId(tmFriendnotaddSelf, tstUI); 
			return;
		}
		
		//是否拒绝与对方进行交易
		if ( !boAccept )
		{
			//交易对方已经离开，交易被取消
			pTarget->SendTipmsgFormatWithId(tmTargetDoNotDealWithYou, tstUI, m_pEntity->GetEntityName());

			AddApplyIdCd(pTarget->GetId(), 0);
		}
		//如果目标玩家正在交易则不能再进行交易
		else if ( pTarget->GetDealSystem().IsDealing() )
		{
			//对方正在交易，请稍后再试
			m_pEntity->SendTipmsgFormatWithId(tmTargetIsDealing, tstUI);
		}
		else
		{
			//必须在同一场景且在一定坐标范围内才允许交易
			//int x, y, tx, ty;
			//m_pEntity->GetPosition(x, y);
			//pTarget->GetPosition(tx, ty);
			//if ( pTarget->GetPosInfo().pScene != m_pEntity->GetPosInfo().pScene || abs(x - tx) > 12 || abs(y - ty) > 12)
			//{
			//	//交易对方已经离开，交易被取消
			//	m_pEntity->SendOldTipmsgWithId(tpDealRequesterHasLeaved, ttDialog);
			//}
			// if (m_pEntity->HasState(esStateStall))
			// {
			// 	return;
			// }
			// if (pTarget->HasState(esStateStall))
			// {
			// 	m_pEntity->SendOldTipmsgWithId(tpTargetInStall, ttFlyTip);
			// 	return;
			// }
			// if (!pTarget->HasMapAttribute(asDealArea))
			// {
			// 	m_pEntity->SendTipmsgFormatWithId(tmDealNotAtSeafArea, tstUI);
			// 	return;
			// }

			if (!m_pEntity->CheckTargetDistance(pTarget,consignmentProvider.nPrivateDealDistance))
				m_pEntity->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
			else 
				boStartDeal = true;
		}
	}
	if ( !boStartDeal )
	{
		if ( pTarget )
		{
			CActorPacket ap; 
			CDataPacket &pack = pTarget->AllocPacket(ap);
			pack << (BYTE)GetSystemID() << (BYTE)sDealSystemDealRefused << m_pEntity->GetEntityName();
			ap.flush();

			CDataPacket &datapack = m_pEntity->AllocPacket(ap);
			datapack << (BYTE)GetSystemID() << (BYTE)sDealSystemDealRefused << m_pEntity->GetEntityName();
			ap.flush();
			AddApplyIdCd(pTarget->GetId(), 0);
		}
	}
	else
	{
		m_nApplyCount = 0;
		//开始交易
		pTarget->GetDealSystem().ClearDealData();
		pTarget->GetDealSystem().m_boDealing = true;
		pTarget->GetDealSystem().m_pDealTarget = m_pEntity;
		ClearDealData();
		m_boDealing = true;
		m_pDealTarget = pTarget;

		// unsigned int nTargetActorId = pTarget->GetProperty<unsigned int>(PROP_ENTITY_ID);
		// unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
		// BYTE nRelation = 0;
		// if (m_pEntity->GetFriendSystem().GetSocialFlag(nTargetActorId, SOCIAL_FRIEND)
		// 	&& pTarget->GetFriendSystem().GetSocialFlag(nActorId, SOCIAL_FRIEND) )
		// {
		// 	nRelation = 1;
		// }
		//向双方发送开始交易的消息
		CActorPacket ap; 
		byte nSex = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SEX);
		CDataPacket &pack = pTarget->AllocPacket(ap);
		pack << (BYTE)enDealSystemID << (BYTE)sDealSystemStartDeal;
		pack << (unsigned int)(m_pEntity->GetId());
		pack.writeString(m_pEntity->GetEntityName());
		pack << (int)(m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL));
		pack << (BYTE)(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));
		ap.flush();
		CActorPacket AP;
		CDataPacket &packet = m_pEntity->AllocPacket(AP);
		packet << (BYTE)enDealSystemID << (BYTE)sDealSystemStartDeal;
		packet << (unsigned int)(pTarget->GetId());
		packet.writeString(pTarget->GetEntityName());
		packet << (int)(pTarget->GetProperty<unsigned int>(PROP_CREATURE_LEVEL));
		packet << (BYTE)(pTarget->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));
		AP.flush();
	}
}

void CDealSystem::ClientDealAddItem(CDataPacketReader &packet)
{
	bool boSuccess = false;
	CUserItem::ItemSeries series;
	BYTE nType = 0;
	BYTE nPos= 0;
	packet >> nType >> nPos;
	CUserItem* pUserItem = NULL;
	if((nPos < 0) || (nPos >= MaxDealItemCount))
	{
		m_pEntity->SendTipmsgFormatWithId(tmDataError, tstUI);
		return;
	}
	if(!m_pEntity || !m_pDealTarget) return;
	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	do
	{
		if (!m_boDealLocked && m_boDealing)
		{
			if (!m_pEntity->CheckTargetDistance(m_pDealTarget, consignmentProvider.nPrivateDealDistance))
			{
				m_pEntity->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
				m_pDealTarget->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
				CancelDeal();
				return;
			}
			//添加物品
			if(	nType == 1)
			{
				packet >> series;
				//交易物品数量是否已满
				if (m_nItemCount >= MaxDealItemCount)
				{
					m_pEntity->SendTipmsgFormatWithId(tmDealBlankWasFull, tstUI);
					break;
				}
				else
				{
					//从背包中查找物品
					pUserItem = ((CActor*)m_pEntity)->GetBagSystem().FindItemByGuid(series);
					if (!pUserItem)
					{
						OutputMsg(rmError, _T("unexpected item data! can't find item by pointed series"));
						m_pEntity->SendTipmsgFormatWithId(tmItemNoDeal, tstUI);
						break;
					}
					int nCircleLevel = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
					int nCardLv = ((CActor*)m_pEntity)->GetMiscSystem().GetMaxColorCardLevel() ;
					if (nCircleLevel < consignmentProvider.nDealLevel[0] && nCardLv < consignmentProvider.nDealLevel[1])
					{
						((CActor*)m_pEntity)->SendTipmsg(consignmentProvider.sDealLimitTips, tstUI);
						return ;
					}

					// 任务物品无法交易
					bool isQuestItem = false;
					const CStdItem* pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
					if (pItem)
						isQuestItem = (Item::itQuestItem == pItem->m_btType) ? true : false;
					
					if (isQuestItem)
					{
						m_pEntity->SendTipmsgFormatWithId(tmItemIsQuestItem, tstUI);
						break;
					}

					// 配置了不可交易
					if (pItem && pItem->hasSceneProp())
					{
						m_pEntity->SendTipmsgFormatWithId(tmItemNoDeal, tstUI);
						break;
					}

					if (pItem && pItem->m_Flags.denyDeal && strlen(pUserItem->cBestAttr) == 0)
					{
						m_pEntity->SendTipmsgFormatWithId(tmItemNoDeal, tstUI);
						break;
					}

					//如果物品存在且尚未放入交易栏中则添加
					if ( pUserItem && !FindDealItem(pUserItem) )
					{
						if(m_DealItemList[nPos] == 0)
							m_nItemCount++;
						
						m_DealItems[nPos] = pUserItem;
						m_DealItemList[nPos] = nPos+1;
						m_DealItemCount[nPos] = pUserItem->wCount;
						
						boSuccess = true;
						break;
					}else
					{
						m_pEntity->SendTipmsgFormatWithId(tmDealItemExist, tstUI);
						break;
					}
					
				}
			}
			else //删除物品
			{
				m_DealItems[nPos] = NULL;
				m_DealItemList[nPos] = 0;
				m_DealItemCount[nPos] = 0;
				m_nItemCount--;
				boSuccess = true;
				break;
			}
			
		}
		else
		{
			m_pEntity->SendTipmsgFormatWithId(tmDealHasLocked, tstUI);
		}
	
	} while (false);

	CActorPacket ap;
	//向交易对方发送添加交易物品的消息
	if(m_pDealTarget && boSuccess)
	{
		CDataPacket& pack = m_pDealTarget->AllocPacket(ap);
		pack << (BYTE)GetSystemID() << (BYTE)sDealSystemTargetAddDealItem;
		pack << nType;
		pack << nPos;
		if(pUserItem)
		{
			*pUserItem >> pack;
		}
		ap.flush();	
	}
	

	//向客户端回复添加交易物品成功与否的消息
	CDataPacket& Packet = m_pEntity->AllocPacket(ap);
	Packet << (BYTE)GetSystemID() << (BYTE)sDealSystemAddDealItemRet << boSuccess;
	Packet << nType;
	Packet << nPos;
	if(pUserItem)
	{
		*pUserItem >> Packet;
	}
	ap.flush();
}

void CDealSystem::ClientChangeDealCoin(CDataPacketReader &packet)
{
	
	bool boSuccess = false;
	BYTE nMoneyType =0;
    int nMoneyCount =0;
	if(!m_pEntity || !m_pDealTarget) return;
	do
	{
		ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
		if(m_boDealLocked)
		{
			m_pEntity->SendTipmsgFormatWithId(tmDealHasLocked, tstUI);
			return;
		}

		if (m_boDealing)
		{
			if (!m_pEntity->CheckTargetDistance(m_pDealTarget, consignmentProvider.nPrivateDealDistance))
			{
				m_pEntity->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
				m_pDealTarget->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
				CancelDeal();
				break;
			}

			packet >> nMoneyType >> nMoneyCount;
			if(nMoneyType != mtCoin &&  nMoneyType != mtYuanbao) //不是元宝就去掉
				break;
			if (m_pDealTarget)
			{
				if (nMoneyCount < 0 || 0xFFFFFFFF - (unsigned int)m_pDealTarget->GetMoneyCount(eMoneyType(nMoneyType)) < (UINT32)nMoneyCount)
				{
					m_pEntity->SendTipmsgFormatWithId(tmTargetMoneyIsFull, tstUI);				
					break;
				}
			}
			
			if(nMoneyCount >=50000000)
			{
				m_pEntity->SendTipmsgFormatWithId(tmDealNumTooLarge,tstUI);
				if(nMoneyType == mtCoin)
					m_nDealCoin = 0;
				else if(nMoneyType == mtYuanbao)
					m_nDealYb = 0;

				nMoneyCount = 0;
				boSuccess = true;	
				break;
			}
			//判断钱币是否足够，必须判断nCoin是否非负数，否则会造成刷钱的BUG
			if ( nMoneyCount >= 0 && m_pEntity->GetMoneyCount(eMoneyType(nMoneyType)) >= nMoneyCount )
			{
				if(nMoneyType == mtCoin)
				{
					m_nDealCoin = nMoneyCount;
				}
				else if(nMoneyType == mtYuanbao)
				{
					m_nDealYb = nMoneyCount;
				}
				boSuccess = true;
			}			
		}

	}while (false);
	
	//向交易对方发送我交易的钱币数量变更的消息
	CActorPacket ap;
	if(m_pDealTarget)
	{
		CDataPacket& pack = m_pDealTarget->AllocPacket(ap);
		pack << (BYTE)GetSystemID() << (BYTE)sDealSystemTargetChgDealCoin <<boSuccess<<nMoneyType << nMoneyCount;
		ap.flush();
	}
	
	//向客户端回复改变交易钱币数量成功与否的消息
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data << (BYTE)GetSystemID() << (BYTE)sDealSystemChangeDealCoinRet << boSuccess <<nMoneyType<< nMoneyCount ;
	ap.flush();
	
}

void CDealSystem::ClientLockDeal(CDataPacketReader &packet)
{
	if (GetLogicServer()->IsCommonServer())
	{
		m_pEntity->SendTipmsgFormatWithId(tmAreaCannotTrade, tstUI);
		return;
	}
	// if (!m_pEntity->HasMapAttribute(asDealArea))
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmAreaCannotTrade, tstUI);
	// 	return;
	// }
	if (!m_boDealing)
		return;
	if(! m_pDealTarget) return;
	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	if (!m_pEntity->CheckTargetDistance(m_pDealTarget, consignmentProvider.nPrivateDealDistance))
	{
		if(! m_pDealTarget) return;
		m_pEntity->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
		CancelDeal();
		return;
	}

	m_boDealLocked = true;

	//向我发送双方的交易锁定状态
	CActorPacket ap;
	CDataPacket& pack = m_pEntity->AllocPacket(ap);
	pack << (BYTE)GetSystemID() << (BYTE)sDealSystemLockDeal << m_boDealLocked << m_pDealTarget->GetDealSystem().m_boDealLocked;
	ap.flush();

	//向对方发送双方的交易锁定状态
	m_pDealTarget->AllocPacket(ap);
	ap << (BYTE)GetSystemID() << (BYTE)sDealSystemLockDeal << m_pDealTarget->GetDealSystem().m_boDealLocked << true;
	ap.flush();

	if ( m_pDealTarget->GetDealSystem().m_boDealLocked )
	{
		m_pEntity->SendTipmsgFormatWithId(tmYouMayClickDealBtnNow, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmYouMayClickDealBtnNow, tstUI);
	}
	else
	{
		m_pEntity->SendTipmsgFormatWithId(tmWaitTargetLockDeal, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmTargetHasLockedDeal, tstUI);
	}
}

void CDealSystem::ClientCancelDeal(CDataPacketReader &packet)
{
	bool boComplete = false;
	CancelDeal(boComplete);
}

bool CDealSystem::CheckDealTargetDistance(CActor *pActor1, CActor *pActor2)
{	
	if (!pActor1 || !pActor2)
		return false;
	if(pActor1->IsInited() == false || pActor2->IsInited() ==false) return false;

	int nSelf_X, nSelf_Y, nTarget_X, nTarget_Y;
	pActor1->GetPosition(nSelf_X, nSelf_Y);
	pActor2->GetPosition(nTarget_X, nTarget_Y);
	if (pActor1->GetPosInfo().pScene != pActor2->GetPosInfo().pScene 
			|| abs(nSelf_X - nTarget_X) > 12 
			|| abs(nSelf_Y - nTarget_Y) > 12)
	{
		return false;
	}

	return true;
}
//交易
void CDealSystem::ClientConfimDeal(CDataPacketReader &packet)
{
	CActorPacket ap;
	if (GetLogicServer()->IsCommonServer())
	{
		m_pEntity->SendTipmsgFormatWithId(tmAreaCannotTrade, tstUI);
		return;
	}

	if(! m_pDealTarget) return;
	

	if (!m_boDealing) return;

	// if (!m_pEntity->HasMapAttribute(asDealArea) || !m_pDealTarget->HasMapAttribute(asDealArea))
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmAreaCannotTrade, tstUI);
	// 	return;
	// }

	//判断双方书否均已锁定交易
	if (!m_boDealLocked || !m_pDealTarget->GetDealSystem().m_boDealLocked)
	{
		//只能在双发都锁定交易后才能完成交易
		m_pEntity->SendTipmsgFormatWithId(tmYouMustLockDealFirst, tstUI);
		return;
	}

	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	tagEditionInfo& editionInfo = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo();
	if (!m_pEntity->CheckTargetDistance(m_pDealTarget, consignmentProvider.nPrivateDealDistance))
	{
		if(! m_pDealTarget) return;
		m_pEntity->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmDealTargetTooFar, tstUI);
		CancelDeal();
		return;
	}

	//如果对方尚未按下交易键则向其发送交易确认消息
	bool bHadConfirmed = m_boDealConfimed;
	m_boDealConfimed = true;
	if (!m_pDealTarget->GetDealSystem().m_boDealConfimed)
	{
		if (!bHadConfirmed)
			m_pDealTarget->SendTipmsgFormatWithId(tmDealTargetConfimed, tstUI);
		return;
	}

	//检查双方的交易钱币是否足够
	if ((UINT32)m_pEntity->GetMoneyCount(mtCoin) < m_nDealCoin 
		|| !ValidateDealItems() 
		|| (UINT32)m_pDealTarget->GetMoneyCount(mtCoin) < m_pDealTarget->GetDealSystem().m_nDealCoin
		|| !m_pDealTarget->GetDealSystem().ValidateDealItems())
	{
		//非法数据，外挂嫌疑
		m_pEntity->SendTipmsgFormatWithId(tmDealDataValidateFailure, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmDealDataValidateFailure, tstUI);
		CancelDeal();
		return;
	}

	if (INT_MAX - m_pEntity->GetMoneyCount(mtCoin) < m_pDealTarget->GetDealSystem().m_nDealCoin)		
	{
		m_pEntity->SendTipmsgFormatWithId(tmSelfMoneryIsFull, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmTargetMoneyIsFull, tstUI);		
		CancelDeal();
		return;
	}
	if (INT_MAX - m_pDealTarget->GetMoneyCount(mtCoin) < m_nDealCoin)
	{
		m_pEntity->SendTipmsgFormatWithId(tmTargetMoneyIsFull, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmSelfMoneryIsFull, tstUI);		
		CancelDeal();
		return;
	}

	//检查双方的交易钱币是否足够
	if ((UINT32)m_pEntity->GetMoneyCount(mtYuanbao) < m_nDealYb
		|| !ValidateDealItems() 
		|| (UINT32)m_pDealTarget->GetMoneyCount(mtYuanbao) < m_pDealTarget->GetDealSystem().m_nDealYb
		|| !m_pDealTarget->GetDealSystem().ValidateDealItems())
	{
		//非法数据，外挂嫌疑
		m_pEntity->SendTipmsgFormatWithId(tmDealDataValidateFailure, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmDealDataValidateFailure, tstUI);
		CancelDeal();
		return;
	}
	if(editionInfo.nStart)
	{
		if((UINT32)(m_pEntity->GetMoneyCount(mtJyQuota)*(editionInfo.nProportion/100.0)) < m_nDealYb 
		|| (UINT32)(m_pDealTarget->GetMoneyCount(mtJyQuota)*(editionInfo.nProportion/100.0)) < m_pDealTarget->GetDealSystem().m_nDealYb)
	{
		m_pEntity->SendTipmsgFormatWithId(tmQuotaNoEnough, tstUI);
		m_pDealTarget->SendTipmsgFormatWithId(tmQuotaNoEnough, tstUI);
		CancelDeal();
		return;
	}
	}
	

	if (0xFFFFFFFF - m_pEntity->GetMoneyCount(mtYuanbao) < m_pDealTarget->GetDealSystem().m_nDealYb)		
	{
		m_pEntity->SendTipmsgFormatWithId(tmSelfMoneryIsFull);
		m_pDealTarget->SendTipmsgFormatWithId(tmTargetMoneyIsFull);		
		CancelDeal();
		return;
	}
	if (0xFFFFFFFF - m_pDealTarget->GetMoneyCount(mtYuanbao) < m_nDealYb)
	{
		m_pEntity->SendTipmsgFormatWithId(tmTargetMoneyIsFull);
		m_pDealTarget->SendTipmsgFormatWithId(tmSelfMoneryIsFull);		
		CancelDeal();
		return;
	}

	//判断我的背包空间是否足够
	// if (m_pEntity->GetBagSystem().availableMinCount() < m_pDealTarget->GetDealSystem().m_nItemCount)
	// {
	// 	//由于你的背包无法完全容纳交易对方放入的物品，交易已被取消
	// 	m_pEntity->SendTipmsgFormatWithId(tmDealCanceledOfMyBagWasFull, tstUI);
	// 	//由于对方背包无法完全容纳你放入的物品，交易已被取消
	// 	m_pDealTarget->SendTipmsgFormatWithId(tmDealcanceldOfTargetBagWasFull, tstUI);
	// 	CancelDeal();
	// 	return;
	// }
	CUserItem * pUserItem = NULL;
	for (int i=CDealSystem::MaxDealItemCount-1; i>-1; --i) {
		//判断我的背包空间是否足够
		if(m_pDealTarget->GetDealSystem().m_DealItems[i]) {
			pUserItem = m_pDealTarget->GetBagSystem().FindItemByGuid(m_pDealTarget->GetDealSystem().m_DealItems[i]->series);
			if (pUserItem) {
				if(pUserItem->wCount != m_pDealTarget->GetDealSystem().m_DealItemCount[i])
				{
					CancelDeal();
					return;
				}
				if(! m_pEntity->GetBagSystem().CanAddItem(pUserItem, true)) {
					//由于你的背包无法完全容纳交易对方放入的物品，交易已被取消
					m_pEntity->SendTipmsgFormatWithId(tmDealCanceledOfMyBagWasFull, tstUI);
					//由于对方背包无法完全容纳你放入的物品，交易已被取消
					m_pDealTarget->SendTipmsgFormatWithId(tmDealcanceldOfTargetBagWasFull, tstUI);
					CancelDeal();
					return;
				}
			}
		}
		//判断对方的背包空间是否足够
		if(m_pEntity->GetDealSystem().m_DealItems[i]) {
			pUserItem = m_pEntity->GetBagSystem().FindItemByGuid(m_pEntity->GetDealSystem().m_DealItems[i]->series);
			if ( pUserItem ) {
				if(pUserItem->wCount != m_DealItemCount[i])
				{
					CancelDeal();
					return;
				}
				if(! m_pDealTarget->GetBagSystem().CanAddItem(pUserItem, true)) {
					//由于你的背包无法完全容纳交易对方放入的物品，交易已被取消
					m_pEntity->SendTipmsgFormatWithId(tmDealCanceledOfMyBagWasFull, tstUI);
					//由于对方背包无法完全容纳你放入的物品，交易已被取消
					m_pDealTarget->SendTipmsgFormatWithId(tmDealcanceldOfTargetBagWasFull, tstUI);
					CancelDeal();
					return;
				}
			}
		}
	}

	
	//判断对方的背包空间是否足够
	// if (m_pDealTarget->GetBagSystem().availableMinCount() < m_nItemCount)
	// {
	// 	//由于你的背包无法完全容纳交易对方放入的物品，交易已被取消
	// 	m_pDealTarget->SendTipmsgFormatWithId(tmDealCanceledOfMyBagWasFull, tstUI);
	// 	//由于对方背包无法完全容纳你放入的物品，交易已被取消
	// 	m_pEntity->SendTipmsgFormatWithId(tmDealcanceldOfTargetBagWasFull, tstUI);
	// 	CancelDeal();
	// 	return;
	// }

	CompleteDeal();

	m_pDealTarget->GetDealSystem().CompleteDeal();
	m_pDealTarget->GetDealSystem().ClearDealData();
	m_pDealTarget->SaveDb();
	ClearDealData();
	m_pEntity->SaveDb();
}
