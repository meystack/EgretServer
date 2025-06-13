// #include "StdAfx.h"
// #include "StallSystem.h"
// #include "../base/Container.hpp"

// CStallSystem::CStallSystem(void)
// {
// }


// CStallSystem::~CStallSystem(void)
// {
// }

// void CStallSystem::ProcessNetData( INT_PTR nCmd, CDataPacketReader &packet )
// {
// 	if (GetLogicServer()->IsCommonServer())
// 	{
// 		return;
// 	}
// 	switch(nCmd)
// 	{
// 	case cStartStall:
// 		StartStall(packet);
// 		break;
// 	case cEndStall:
// 		EndStall();
// 		break;
// 	case cLeaveMsg:
// 		LeaveMsg(packet);
// 		break;
// 	case cBuyStallItem:
// 		BuyStallItem(packet);
// 		break;
// 	case cViewStall:
// 		ViewStall(packet);
// 		break;
// 	case cSendAd:
// 		//SendStallAd(packet);
// 		break;
// 	case cStallItemOpt:
// 		OptStallItem(packet);
// 		break;
// 	case cCanStall:
// 		if (CheckStallLevel())
// 			CheckStallArea(packet);
// 		break;
// 	default:
// 		break;
// 	}
// }
// bool CStallSystem::CheckStallLevel()
// {
// 	if (!m_pEntity || m_pEntity->GetType() != enActor)
// 	{
// 		return false;
// 	}
// 	bool ret = true;
// 	int nMinLv = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nStallLevel;
// 	int nMinCircle = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nStallCircle;
// 	if (!m_pEntity->CheckLevel(nMinLv, nMinCircle))
// 	{
// 		ret = false;
// 		m_pEntity->SendOldTipmsgWithId(tpStallLevelLimited, ttFlyTip);
// 	}

// 	return ret;
// }
// void CStallSystem::CheckStallArea(CDataPacketReader & pakcet)
// {
// 	int x,y;
// 	BYTE nResult = CStallData::srStallFail;
// 	m_pEntity->GetPosition(x, y);
// 	//CScene * pScene = m_pEntity->GetScene();
// 	// if (pScene && pScene->HasMapAttribute(x, y, asStallArea))
// 	// {
// 	// 	nResult = CStallData::srStallSucc;
// 	// }
// 	CActorPacket AP;
// 	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
// 	DataPacket << (byte)GetSystemID() << (byte)sStallAreaResult;
// 	DataPacket << (BYTE)nResult;
// 	AP.flush();
// }

// void CStallSystem::SendStallResult(BYTE nStallResult)
// {
// 	CActorPacket AP;
// 	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
// 	DataPacket << (byte)GetSystemID() << (byte)sStallResult;
// 	DataPacket << (BYTE)CStallData::soStartStall;
// 	DataPacket << (BYTE)nStallResult;
// 	AP.flush();
// }

// bool CStallSystem::GetStallInfo(CDataPacketReader& packet)
// {
// 	/*
// 	byte nItemSize;
// 	packet.readString(m_stallInfo.sName);
// 	//packet.readString(m_stallInfo.sAd, ArrayCount(m_stallInfo.sAd));
// 	packet >> nItemSize;
// 	UINT nMyGrid = m_pEntity->GetProperty<UINT>(PROP_ACTOR_STALL_GRID_COUNT);
// 	if (nItemSize > (byte)nMyGrid)
// 	{
// 		m_pEntity->SendOldTipmsgWithId(tpIsMaxGrid, ttFlyTip);
// 		SendStallResult(CStallData::srStallFail);
// 		return false;
// 	}
// 	for (byte i = 0; i < nItemSize; i++)
// 	{
// 		CStallData::STALLITEM item;
// 		packet >> item.series.llId >> item.nCoin >> item.nPrice;
// 		CUserItem* pUserItem = ((CActor*)m_pEntity)->GetBagSystem().FindItemByGuid(item.series);
// 		if (pUserItem)
// 		{
// 			if(pUserItem->binded())
// 			{
// 				const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
// 				if (pStdItem)
// 				{					
// 					m_pEntity->SendOldTipmsgFormatWithId(tpStallBindItem, ttFlyTip, pStdItem->m_sName);
// 				}
// 				m_stallInfo.vItems.clear();
// 				SendStallResult(CStallData::srStallFail);
// 				return false; //绑定的不能卖
// 			}
// 			item.nItemId = pUserItem->wItemId;
// 			item.wNum = pUserItem->wCount;
// 			item.nFlag = 1;
// 			if (item.nPrice <= 0 || item.wNum <= 0 )
// 			{
// 				OutputMsg(rmError, _T("%s:玩家[%s]摆摊价格[%d]或物品数量[%d]非法"), __FUNCTION__,m_pEntity->GetEntityName(),(int)item.nPrice,(int)item.wNum);
// 				SendStallResult(CStallData::srStallFail);
// 				return false;
// 			}
// 			if (item.nCoin != mtCoin && item.nCoin != mtYuanbao)
// 			{
// 				SendStallResult(CStallData::srStallFail);
// 				return false;
// 			}
// 			if (item.nPrice > 99999999)
// 			{
// 				SendStallResult(CStallData::srStallFail);
// 				m_pEntity->SendOldTipmsgWithId(tpDealNumTooLarge, ttFlyTip);	
// 				return false;
// 			}
// 			m_stallInfo.vItems.add(item);
// 		}
// 	}*/
// 	return true;
// }

// /*玩家摆摊
// */
// void CStallSystem::StartStall( CDataPacketReader& packet)
// {
// 	if(m_pEntity->HasState(esStateStall))
// 	{
// 		m_pEntity->SendOldTipmsgWithId(tpHaveStall,ttFlyTip);
// 		SendStallResult(CStallData::srStallFail);
// 		return;
// 	}
// 	if (m_pEntity->GetDealSystem().IsDealing())
// 	{
// 		m_pEntity->SendOldTipmsgWithId(tpDealNoStall, ttScreenCenter);
// 		return;
// 	}
// 	CScene * pScene = m_pEntity->GetScene();
// 	if (pScene == NULL)
// 	{
// 		SendStallResult(CStallData::srStallFail);
// 		return;
// 	}
// 	if (!CheckStallLevel())
// 		return;

// 	int x,y;
// 	m_pEntity->GetPosition(x, y);
// 	// if(!pScene->HasMapAttribute(x, y, asStallArea))
// 	// {	//不是摆摊区域		
// 	// 	//m_pEntity->SendOldTipmsgWithId(tpNotStallArea,ttDialog);
// 	// 	//主要是为了让客户端弹出非模态窗口
// 	// 	BYTE nResult = CStallData::srStallFail;
// 	// 	CActorPacket AP;
// 	// 	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
// 	// 	DataPacket << (byte)GetSystemID() << (byte)sStallAreaResult;
// 	// 	DataPacket << (BYTE)nResult;
// 	// 	AP.flush();
// 	// 	SendStallResult(CStallData::srStallFail);
// 	// 	return;
// 	// }
// 	int nRadio = 2, nDir = 4;
// 	EntityVector& vec = *(CFuBenManager::m_pVisibleList);
// 	vec.clear();
// 	pScene->GetVisibleList(((CEntity*)m_pEntity),vec,-nRadio,nRadio,-nRadio,nRadio);
// 	if (vec.count() > 0)
// 	{
// 		CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
// 		for (int i = 0; i < vec.count(); i++)
// 		{
// 			CEntity* pe = pMgr->GetEntity(vec[i]);
// 			if (pe && pe->GetType() == enActor && ((CActor*)pe)->HasState(esStateStall))
// 			{
// 				m_pEntity->SendOldTipmsgWithId(tpNeaerHaveStall, ttFlyTip);
// 				return;
// 			}
// 		}
// 	}

// 	m_stallInfo.vItems.clear();
// 	m_stallInfo.vMsgs.clear();
// 	if(!GetStallInfo(packet))
// 	{
// 		return;
// 	}
	
// 	m_pEntity->AddState(esStateStall);
// 	m_pEntity->SetDir(nDir);					//设置摊主朝向
// 	SendStallResult(CStallData::srStallSucc);
// 	m_pEntity->CollectOperate(CEntityOPCollector::coRefFeature); 
// 	char buff[128];
// 	CDataPacket data(buff,sizeof(buff));
// 	data << (BYTE) enDefaultEntitySystemID << (BYTE) sTurnAround
// 		<<	(Uint64)m_pEntity->GetHandle()  << (BYTE) nDir;
// 	CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
// 	if(pSystem != NULL)
// 	{
// 		pSystem->BroadCast(data.getMemoryPtr(),data.getPosition(),true);
// 	}
// 	m_pEntity->ResetShowName();

// 	ACTORNAME sName;
// 	sprintf(sName, "%s", m_pEntity->GetEntityName());
// 	SendLeaveMsg((CActor*)m_pEntity,sName);
// }

// void CStallSystem::EndStall()
// {
// 	if(m_pEntity->HasState(esStateStall))
// 	{
// 		m_pEntity->RemoveState(esStateStall);
// 		CActorPacket AP;
// 		CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
// 		DataPacket << (byte)GetSystemID() << (byte)sStallResult;
// 		DataPacket << (BYTE)CStallData::soEndStall;
// 		DataPacket << (BYTE)CStallData::srStallSucc;
// 		AP.flush();
// 		NoticeNearBuyers(0);
// 		m_pEntity->ResetShowName();
// 		m_pEntity->CollectOperate(CEntityOPCollector::coRefFeature); 
// 	}
	
// }

// void CStallSystem::LeaveMsg( CDataPacketReader & packet )
// {
// 	ACTORNAME sName;
// 	packet.readString(sName, ArrayCount(sName));
// 	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
// 	if(!pActor)
// 	{
// 		m_pEntity->SendOldTipmsgWithId(tpStallerOnffline, ttFlyTip);
// 		return;
// 	}
// 	CStallData::STALLMSG msg;
// 	msg.nMsgType = 1;
// 	msg.nId = m_pEntity->GetId();
// 	sprintf(msg.sPlayerName, "%s", m_pEntity->GetEntityName());
// 	packet.readString(msg.sMsg, ArrayCount(msg.sMsg));
// 	GetGlobalLogicEngine()->GetChatMgr().Filter(msg.sMsg);
// 	//pActor->GetStallSystem().OnLeaveMsg(msg);
// 	//存起来留言
// 	if (20 <pActor->GetStallSystem().m_stallInfo.vMsgs.count())
// 	{
// 		pActor->GetStallSystem().m_stallInfo.vMsgs.clear();
// 	}
// 	pActor->GetStallSystem().m_stallInfo.vMsgs.add(msg);
// 	SendLeaveMsg(m_pEntity, sName);			//留言者的
// 	if(m_pEntity != pActor)
// 	{
// 		SendLeaveMsg(pActor, sName);		//店主的
// 		((CActor *)m_pEntity)->SendOldTipmsgWithId(tpLeaveWord, ttFlyTip);
// 	}
// }

// bool CStallSystem::OnLeaveMsg( CStallData::STALLMSG& msg, BYTE nCoin, int nPrice )
// {
// 	if(msg.nMsgType)
// 	{
// 	}
// 	else
// 	{
// 		CActorPacket AP;
// 		CDataPacket& netPack = m_pEntity->AllocPacket(AP);
// 		netPack <<(BYTE)GetSystemID();
// 		netPack << (BYTE)sAddBuyLog;
// 		netPack.writeString(msg.sPlayerName);
// 		netPack << (BYTE)nCoin << (int)nPrice;
// 		netPack.writeString(msg.sMsg);
// 		AP.flush();
// 	}
// 	return true;
// }

// void CStallSystem::SendLeaveMsg(CActor * pActor,ACTORNAME sName)
// {
// 	if(pActor == NULL)
// 		return;
// 	CActor * pHostActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
// 	if(!pHostActor)
// 	{
// 		m_pEntity->SendOldTipmsgWithId(tpStallerOnffline, ttFlyTip);
// 		return;
// 	}
// 	CActorPacket AP;
// 	CDataPacket& netPack = pActor->AllocPacket(AP);
// 	netPack <<(BYTE)GetSystemID();
// 	netPack << (BYTE)sAddLeaveMsg;
// 	INT_PTR nSize = pHostActor->GetStallSystem().m_stallInfo.vMsgs.count();
// 	netPack << (BYTE)nSize;
// 	for (INT_PTR i = 0; i < nSize; i++)
// 	{
// 		const CStallData::STALLMSG & msg = pHostActor->GetStallSystem().m_stallInfo.vMsgs.get(i);
// 		netPack.writeString(msg.sPlayerName);
// 		netPack.writeString(msg.sMsg);
// 	}
// 	netPack.writeString(sName);
// 	AP.flush();
// }

// void CStallSystem::ViewStall( CDataPacketReader& packet )
// {
// 	unsigned int nActorId = 0;
// 	packet >> nActorId;
// 	ACTORNAME sName;
// 	packet.readString(sName, ArrayCount(sName));
// 	CActor * pActor = NULL;
// 	if( nActorId > 0)
// 	{
// 		pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
// 	}
// 	else
// 	{
// 		pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
// 	}	
// 	if(NULL == pActor)
// 	{
// 		m_pEntity->SendOldTipmsgWithId(tpStallerOnffline, ttFlyTip);
// 		return;
// 	}
// 	CActorPacket AP;
// 	CDataPacket& netPack = m_pEntity->AllocPacket(AP);
// 	netPack <<(BYTE)GetSystemID() << (BYTE)sStallInfo;
// 	pActor->GetStallSystem().FillStallData(netPack);
// 	AP.flush();
// 	SendLeaveMsg(m_pEntity, sName);
// }

// bool CStallSystem::FillStallData( CDataPacket & netPack )
// {
// 	CUserBag & userBag = ((CActor*)m_pEntity)->GetBagSystem();
// 	const char * sName = m_pEntity->GetEntityName();
// 	netPack.writeString(sName);
// 	netPack.writeString(m_stallInfo.sName);
// 	//netPack.writeString(m_stallInfo.sAd);
// 	size_t pos = netPack.getPosition();
// 	BYTE nItemCount = 0;
// 	INT_PTR nSize = m_stallInfo.vItems.count();
// 	netPack << (BYTE)nSize;
// 	CVector<INT_PTR> DelItemList;
// 	for (INT_PTR i = 0; i < nSize; i++)
// 	{
// 		const CStallData::STALLITEM & item = m_stallInfo.vItems.get(i);
// 		if(item.nFlag != -1)
// 		{
// 			CUserItem * pUserItem = userBag.FindItemByGuid(item.series);
// 			if (pUserItem)
// 			{
// 				if(pUserItem->wCount == item.wNum)
// 				{
// 					netPack << *pUserItem << item.nCoin << item.nPrice;
// 					nItemCount++;
// 				}
// 				else
// 				{
// 					DelItemList.add(i);
// 				}
// 			}
// 		}		
// 	}
// 	for(INT_PTR i=DelItemList.count(); i>=0; i--)
// 	{
// 		m_stallInfo.vItems.remove(DelItemList[i]);
// 	}
// 	BYTE* pCount = (BYTE*)netPack.getPositionPtr(pos);
// 	*pCount = (BYTE)nItemCount;
// 	return true;
// }

// void CStallSystem::BuyStallItem( CDataPacketReader & packet )
// {
// 	// if (m_pEntity->HasState(esStateStall))
// 	// {
// 	// 	//return;
// 	// }
// 	// ACTORNAME sName;
// 	// packet.readString(sName, ArrayCount(sName));
// 	// CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
// 	// if(!pActor)
// 	// {
// 	// 	m_pEntity->SendOldTipmsgWithId(tpStallerOnffline, ttFlyTip);
// 	// 	return;
// 	// }
// 	// if (pActor == (CActor *)m_pEntity)
// 	// {
// 	// 	return;
// 	// }
// 	// //检查摊主是否在摆摊
// 	// if (!pActor->HasState(esStateStall))
// 	// {
// 	// 	m_pEntity->SendOldTipmsgFormatWithId(tpNotStallState, ttFlyTip, pActor->GetEntityName());
// 	// 	return;
// 	// }
// 	// CUserItem::ItemSeries series;
// 	// packet >> series.llId;
// 	// const CStallData::STALLITEM* sItem = pActor->GetStallSystem().GetStallItemByGuid(series);
// 	// if( sItem && sItem->nFlag != -1)
// 	// {
// 	// 	CUserItem * pUserItem = pActor->GetBagSystem().FindItemByGuid(series);
// 	// 	if (pUserItem != NULL)
// 	// 	{
// 	// 		//判断钱的物品和位置
// 	// 		byte bItCoin = sItem->nCoin;
// 	// 		int nPrice = sItem->nPrice;
// 	// 		int nTipId = tpMoneyTypeCoin;
// 	// 		if (bItCoin == mtYuanbao)
// 	// 		{
// 	// 			nTipId = tpMoneyTypeYuanbao;
// 	// 		}
// 	// 		TIPMSGCONFIG& tipConfig = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig();	
// 	// 		char * sMoneyDesc = tipConfig.GetTipmsg(nTipId);
// 	// 		int nMyMoney = (int)m_pEntity->GetMoneyCount(eMoneyType(bItCoin));
// 	// 		if (nPrice <= 0 || nMyMoney < nPrice)
// 	// 		{							
// 	// 			m_pEntity->SendOldTipmsgFormatWithId(tpBuyStallNeedMoney, ttFlyTip, sItem->nPrice,sMoneyDesc,nMyMoney,sMoneyDesc);
// 	// 			return;
// 	// 		}
// 	// 		unsigned int nMasterMoney = (unsigned int)pActor->GetMoneyCount(eMoneyType(bItCoin));
// 	// 		if (0xFFFFFFFF - nMasterMoney < (UINT32)nPrice)
// 	// 		{
// 	// 			m_pEntity->SendOldTipmsgWithId(tpTargetMoneyIsFull, ttFlyTip);
// 	// 			return;
// 	// 		}
// 	// 		//位置
// 	// 		CUserItemContainer::ItemOPParam itemOp;
// 	// 		itemOp.wItemId = pUserItem->wItemId;
// 	// 		itemOp.wCount = pUserItem->wCount;
// 	// 		itemOp.btQuality = pUserItem->btQuality;
// 	// 		itemOp.btStrong = pUserItem->btStrong;
// 	// 		itemOp.btBindFlag = pUserItem->btFlag;
// 	// 		INT_PTR nNeedGrids = m_pEntity->GetBagSystem().GetAddItemNeedGridCount(itemOp);
// 	// 		if( nNeedGrids > 0 && nNeedGrids > m_pEntity->GetBagSystem().availableCount(pUserItem->wPackageType))
// 	// 		{
// 	// 			m_pEntity->SendOldTipmsgFormatWithId(tpNeedBagGrid, ttFlyTip, nNeedGrids);
// 	// 			return;
// 	// 		}
// 	// 		LPCTSTR sBuyFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpLogBuyStallItem);
// 	// 		if(!m_pEntity->ChangeMoney(bItCoin, -nPrice, GameLog::clStallBuyItemMoney, 0, sBuyFormat, true))
// 	// 		{
// 	// 			OutputMsg(rmNormal, _T("摆摊系统买家[%u]扣钱[类型:%d][数量:%d]出错,卖家[%u]"), m_pEntity->GetId(), sItem->nCoin,sItem->nPrice, pActor->GetId());
// 	// 			return;
// 	// 		}
// 	// 		LPCTSTR sSellFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpLogSellStallItem);
// 	// 		if(!pActor->ChangeMoney(bItCoin, nPrice, GameLog::clStallSellItemMoney, 0, sSellFormat, true))
// 	// 		{
// 	// 			OutputMsg(rmNormal, _T("摆摊系统卖家[%u]添加钱[类型:%d][数量:%d]出错,买家是[%u]"),pActor->GetId(),sItem->nCoin,sItem->nPrice,m_pEntity->GetId());
// 	// 			return;
// 	// 		}
// 	// 		int nOldCount = pUserItem->wCount;
// 	// 		pActor->GetStallSystem().OnBuyItem(series);	//先通知客户端,sItem指针就从列表中移除，后面不能用了。
// 	// 		pActor->GetBagSystem().RemoveItem(series, m_pEntity->GetEntityName(), GameLog::clStallSellItem, false);
// 	// 		const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
// 	// 		//如果物品需要记录流通日志，则需要向日志服务器发送记录日志的数据
// 	// 		if ( pStdItem && pStdItem->m_Flags.recordLog)
// 	// 		{
// 	// 			CUserBag::LogChangeItemCount(pUserItem, pStdItem, nOldCount, m_pEntity->GetEntityName(), pActor->GetId(), pActor->GetEntityName(), GameLog::clStallSellItem, pActor->GetAccount());
// 	// 		}

// 	// 		if(((CActor*)m_pEntity)->GetBagSystem().AddItem(pUserItem, pActor->GetEntityName(), GameLog::clStallBuyItem, true, false) > 0 )
// 	// 		{
// 	// 			CStallData::STALLMSG msg;
// 	// 			msg.nMsgType = 0;
// 	// 			msg.nId = m_pEntity->GetId();
// 	// 			sprintf(msg.sPlayerName, "%s", m_pEntity->GetEntityName());
// 	// 			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpStallBuyLog);				
// 	// 			sprintf(msg.sMsg, sFormat, nPrice, sMoneyDesc, itemOp.wCount, pStdItem->m_sName );
// 	// 			pActor->GetStallSystem().OnLeaveMsg(msg, bItCoin, nPrice);
				
// 	// 			if (pActor->HasState(esStateStall))
// 	// 			{
// 	// 				pActor->GetStallSystem().NoticeNearBuyers(1);
// 	// 			}

// 	// 			char sItemInfo[32];
// 	// 			sprintf_s(sItemInfo, ArrayCount(sItemInfo), "%s,%d", m_pEntity->GetEntityName(), pUserItem->wItemId);
// 	// 		}
// 	// 	}
// 	// }
// }

// bool CStallSystem::OnEnlargeStallGrid( byte nGrid )
// {/*
// 	if (m_stallInfo.nGridCount + nGrid > GetLogicServer()->GetDataProvider()->GetGlobalConfig().nStallMaxGrids)
// 	{
// 		return false;
// 	}
// 	m_stallInfo.nGridCount += nGrid;
// 	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_STALL_GRID_COUNT, m_stallInfo.nGridCount);
// 	*/
// 	return true;
// }

// bool CStallSystem::OnBuyItem( CUserItem::ItemSeries& series )
// {
// 	bool boResult = false;
// 	INT_PTR nSize = m_stallInfo.vItems.count();
// 	for (INT_PTR i = 0; i < nSize; i++)
// 	{
// 		const CStallData::STALLITEM & item = m_stallInfo.vItems.get(i);
// 		if (item.nFlag != -1 && item.series == series)
// 		{
// 			m_stallInfo.vItems.remove(i);
// 			CActorPacket AP;
// 			CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
// 			DataPacket << (byte)GetSystemID() << (byte)sNoticeSaleItem;
// 			DataPacket <<(INT64)series.llId;
// 			AP.flush();
// 			boResult = true;
// 			break;
// 		}
// 	}
// 	if(m_stallInfo.vItems.count() == 0)
// 	{
// 		EndStall();
// 	}
// 	return boResult;
// }

// const CStallData::STALLITEM* CStallSystem::GetStallItemByGuid( CUserItem::ItemSeries series )
// {
// 	INT_PTR nSize = m_stallInfo.vItems.count();
// 	for (INT_PTR i = 0; i < nSize; i++)
// 	{
// 		const CStallData::STALLITEM & item = m_stallInfo.vItems.get(i);
// 		if (item.nFlag != -1 && item.series == series)
// 		{
// 			return &item;
// 		}
// 	}
// 	return NULL;
// }

// void CStallSystem::OptStallItem( CDataPacketReader & pakcet )
// {/*
// 	byte nOptType;
// 	CStallData::STALLITEM item;
// 	CUserItem::ItemSeries itSeries;
// 	pakcet >> nOptType >> itSeries.llId;
// 	if (m_pEntity->HasState(esStateStall))
// 	{
// 		BYTE nResult = 1;
// 		if (nOptType)
// 		{
// 			CUserItem* pUserItem = ((CActor*)m_pEntity)->GetBagSystem().FindItemByGuid(itSeries);
// 			if (pUserItem)
// 			{
// 				UINT nMyGrid = m_pEntity->GetProperty<UINT>(PROP_ACTOR_STALL_GRID_COUNT);
// 				if(pUserItem->binded())
// 				{
// 					const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
// 					if (pStdItem)
// 					{					
// 						m_pEntity->SendOldTipmsgFormatWithId(tpStallBindItem, ttFlyTip, pStdItem->m_sName);
// 					}
// 					return; //绑定的不能卖
// 				}
				
// 				if (m_stallInfo.vItems.count() + 1 > (byte)nMyGrid)
// 				{
// 					m_pEntity->SendOldTipmsgWithId(tpIsMaxGrid,ttFlyTip);
// 					return;
// 				}
// 				item.series = itSeries;
// 				pakcet >> item.nCoin >> item.nPrice; 
// 				item.nItemId = pUserItem->wItemId;
// 				item.wNum = pUserItem->wCount;
// 				item.nFlag = 1;
// 				m_stallInfo.vItems.add(item);
// 				NoticeNearBuyers(1);
// 				nResult = 0;
// 			}
// 		}
// 		else
// 		{
// 			INT_PTR nIndex = GetStallItemIdxByGuid(itSeries);
// 			if (nIndex != -1)
// 			{
// 				m_stallInfo.vItems.remove(nIndex);
// 				NoticeNearBuyers(1);
// 				nResult = 0;
// 			}
// 			if (m_stallInfo.vItems.count() == 0)
// 			{
// 				EndStall();
// 			}
// 		}
		
// 		CActorPacket AP;
// 		CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
// 		DataPacket << (byte)GetSystemID() << (byte)sStallItemOpt;
// 		DataPacket << (BYTE)nOptType << (BYTE)nResult <<(INT64)itSeries.llId;
// 		if (nOptType)
// 		{
// 			DataPacket << (BYTE)item.nCoin << (int)item.nPrice;
// 		}
// 		AP.flush();
// 	}*/
// }

// INT_PTR CStallSystem::GetStallItemIdxByGuid( CUserItem::ItemSeries series )
// {
// 	INT_PTR nSize = m_stallInfo.vItems.count();
// 	for (INT_PTR i = 0; i < nSize; i++)
// 	{
// 		const CStallData::STALLITEM & item = m_stallInfo.vItems.get(i);
// 		if (item.nFlag != -1 && item.series == series)
// 		{
// 			return i;
// 		}
// 	}
// 	return -1;
// }

// void CStallSystem::NoticeNearBuyers(byte nMsgType)
// {
// 	char buff[1024];
// 	CDataPacket netPack(buff,sizeof(buff));
// 	const char * sName = m_pEntity->GetEntityName();
// 	netPack << (BYTE)GetSystemID() << (BYTE) sStallBroadCast;
// 	netPack.writeString(sName);
// 	netPack << nMsgType;
// 	bool boMyself = (nMsgType > 0);
// 	m_pEntity->GetObserverSystem()->BroadCast(netPack.getMemoryPtr(), netPack.getPosition(), boMyself);
// }

// void CStallSystem::OnActorLogout()
// {
// 	if(m_pEntity ==NULL) return;

// 	if (m_pEntity->HasState(esStateStall))
// 	{
// 		m_stallInfo.vItems.clear();
// 		m_stallInfo.vMsgs.clear();
// 		EndStall();
// 	}
// }
// //
// //void CStallSystem::SendStallAd(CDataPacketReader & packet)
// //{
// //	//扣钱
// //	//发出售频道
// //	if (m_pEntity->HasState(esStateStall))
// //	{
// //		packet.readString(m_stallInfo.sAd, ArrayCount(m_stallInfo.sAd));
// //		const int nMoney = 5000;		//需要配置化
// //		int nMyCoin = (int)m_pEntity->GetMoneyCount(mtCoin);
// //		if (nMyCoin < nMoney)
// //		{
// //			m_pEntity->SendOldTipmsgFormatWithId(tpSendStallAdMoney, ttDialog, nMoney, nMyCoin );
// //			return;
// //		}
// //		if(m_pEntity->ChangeMoney(mtCoin, -nMoney, GameLog::clStallBuyItemMoney, 0, m_pEntity->GetEntityName(), true))
// //		{
// //			char sBuff[512], sMsg[512];
// //			sprintf(sMsg, "[%s]:[%s]", m_stallInfo.sName, m_stallInfo.sAd);
// //			CDataPacket outPack(sBuff, sizeof(sBuff));
// //			outPack << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)ciChannelNear;
// //			outPack.writeString(m_pEntity->GetEntityName());
// //			outPack.writeString(sMsg);
// //			outPack << (BYTE)m_pEntity->GetProperty<int>(PROP_ACTOR_SEX);//nSex; //玩家的性别	
// //			outPack << (BYTE)0;		
// //			outPack << (BYTE)1; // 阵营Id
// //			outPack << (BYTE)0; // 阵营职位Id
// //			m_pEntity->GetObserverSystem()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition(),true);
// //		}
// //	}
// //}

// void CStallSystem::OnEnterGame()
// {/*
// 	unsigned int nCurGrid =  m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_STALL_GRID_COUNT);
// 	unsigned int nMinGrid = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nStallMinGrids;
// 	if (nCurGrid < nMinGrid)
// 	{
// 		nCurGrid = nMinGrid;
// 		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_STALL_GRID_COUNT, nMinGrid);
// 	}
// 	m_stallInfo.nGridCount = nCurGrid;*/
// }
