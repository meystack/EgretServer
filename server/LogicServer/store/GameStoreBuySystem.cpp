#include "StdAfx.h"
#include "GameStoreBuySystem.h"
void CGameStoreBuySystem::OnEnterGame()
{
	//ClientQueryYuanBaoCount();
}
void CGameStoreBuySystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet)
{
	switch(nCmd)
	{

	case cStoreSystemShopInfo:
	{
		SendShopLists(packet);
		break;
	}
	case cStoreSystemBuyShop:
	{
		BuyShopItem(packet);
		break;
	}
	default:
		TRACE(_T("invalid GameStore System CMD %d\n"), nCmd);
		break;
	}
}


void CGameStoreBuySystem::SendShopLists(CDataPacketReader& packet)
{
	if(!m_pEntity) return;

	CActor* pActor = (CActor*)m_pEntity;
	if(!pActor) return ;

	BYTE nType = 0;
	packet >> nType;
	BYTE ntab = 0;
	packet >> ntab;
	
	CGameStore& store= GetLogicServer()->GetDataProvider()->GetGameStore();
	std::map<int, std::map<int, std::vector<ShopConfig> > >::iterator ot = store.m_shops.find(nType);
	if(ot != store.m_shops.end())
	{
		std::map<int, std::vector<ShopConfig> >& shopLists = ot->second;
		std::map<int, std::vector<ShopConfig> >::iterator it = shopLists.find(ntab);
		if(it != shopLists.end())
		{
			CActorPacket  ap;
			CDataPacket & dataPacket = pActor->AllocPacket(ap);
			dataPacket <<(BYTE)enStoreSystemID <<(BYTE)sStoreSystemShopInfo;

			int nCount = it->second.size();
			dataPacket << (BYTE) nType;
			dataPacket << (BYTE) ntab;
			int pos = dataPacket.getPosition();
			dataPacket << (BYTE) nCount;

			int nDays = GetLogicServer()->GetDaysSinceOpenServer();

			std::vector<ShopConfig>::iterator ot = it->second.begin();
		
			BYTE nResult = 0;
			for(;ot != it->second.end(); ot++)
			{
				ShopConfig& infoCfg = *ot;
				if(!pActor->CheckLevel(infoCfg.nLimitLv, infoCfg.nReincarnationlimit) || nDays < infoCfg.nDaylimit)
				{
					continue;
				}
				int times =store.getLimitTimes(infoCfg.buyLimit, pActor, nDays);
				if(times != -1) {
					dataPacket << (int)infoCfg.nShopId;
					int nValue = 0;
					if(infoCfg.nStaticType > 0)
						nValue = pActor->GetStaticCountSystem().GetStaticCount(infoCfg.nStaticType);
					dataPacket << (BYTE)(nValue);
					dataPacket << (int)times;
					dataPacket << (BYTE)infoCfg.nFlag;
					dataPacket << (BYTE)infoCfg.nStaticType;

					nResult++;
				}
				
			}
			BYTE* pCount = (BYTE*)dataPacket.getPositionPtr(pos);
			*pCount = nResult;
			ap.flush();
		}
	}
}

void CGameStoreBuySystem::BuyShopItem(CDataPacketReader& packet)
{
	if(!m_pEntity) return;

	CActor* pActor = (CActor*)m_pEntity;
	if(!pActor) return;

	BYTE nType = 0;
	BYTE nShopId = 0;
	BYTE ntab = 0;
	int nNum = 0;
	packet >> nType >> ntab >> nShopId;
	
	
	EntityHandle targetHandle;
	packet >> targetHandle;
	packet >> nNum;
	if(nNum == 0)
		nNum = 1;
	CGameStore& store= GetLogicServer()->GetDataProvider()->GetGameStore();
	std::map<int, std::map<int, std::vector<ShopConfig> > >::iterator it = store.m_shops.find(nType);
	if(it != store.m_shops.end())
	{
		std::map<int, std::vector<ShopConfig> >& shopLists = it->second;
		std::map<int, std::vector<ShopConfig> >::iterator iter = shopLists.find(ntab);
		if(iter != shopLists.end())
		{
			int nDays = GetLogicServer()->GetDaysSinceOpenServer();

			std::vector<ShopConfig>::iterator ot = iter->second.begin();
		
			BYTE nResult = 0;
			for(;ot != iter->second.end(); ot++)
			{
				ShopConfig& infoCfg = *ot;
				if(infoCfg.nShopId == nShopId)
				{
					if(!pActor->CheckLevel(infoCfg.nLimitLv, infoCfg.nReincarnationlimit) || nDays < infoCfg.nDaylimit)
					{
						pActor->SendTipmsgFormatWithId(tmComPoseLimit, tstUI);
						return ;
					}
					if( !pActor->CheckGuildLevel(infoCfg.nGuildLevelLimit) )
					{
						pActor->SendTipmsgFormatWithId(tmComPoseLimit, tstUI);
						return ;
					}
					int nValue = infoCfg.nStaticType > 0 ?pActor->GetStaticCountSystem().GetStaticCount(infoCfg.nStaticType):0;
					int times = store.getLimitTimes(infoCfg.buyLimit, pActor, nDays);
					if(times == -1 || (times > 0 && (nValue +nNum) > times))
					{
						pActor->SendTipmsgFormatWithId(tmCircleLimtiTimes, tstUI);
						return ;
					}

					if(infoCfg.nNpc.size() != 0)
					{
						CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(targetHandle);
						if (!pEntity || !infoCfg.InNpc(pEntity->GetId()))
						{
							pActor->SendTipmsgFormatWithId(tmDataError, tstUI);
							return ;
						}
						if(!pActor->CheckTargetDistance((CActor*)pEntity, 15))
						{
							pActor->SendTipmsgFormatWithId(tmTagetDistanceTooFar, tstUI);
							return ;
						}

					}
					int nZk = 100;
					CUserBag& bag = m_pEntity->GetBagSystem();
					if(infoCfg.shop.nType == 0 ) {
						const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(infoCfg.shop.nItemId);
						if(pStdItem ) {
							int nCostBag = 0;
							if (pStdItem->m_wDupCount >= 1) {
								nCostBag = (infoCfg.shop.nCount*nNum) / pStdItem->m_wDupCount +1;
							}

							if(bag.availableCount(pStdItem->m_nPackageType) < nCostBag)
							{
								pActor->SendTipmsgFormatWithId(tmDefNoBagNum,tstUI);
								return;
							}
						}
					}
					else{
						if(!bag.bagIsEnough(BagShop))
						{
							pActor->SendTipmsgFormatWithId(tmDefNoBagNum,tstUI);
							return;
						}
					}
					

					if(!pActor->CheckActorSource(infoCfg.price.nType, infoCfg.price.nId, (infoCfg.price.nCount*nNum), tstUI))
					{
						// pActor->SendTipmsgFormatWithId(tmNomoreYubao, tstUI);
						return ;
					}
					// if()
					char logs[64] = {0};
					if(infoCfg.shop.nType == 0 ) {
						const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(infoCfg.shop.nItemId);
						if(pStdItem) {
							sprintf_s(logs,64,"%s|%d|%d",pStdItem->m_sName,nNum,nShopId);
							logs[63] = 0;
						}
					}else
					{
						const SourceConfig* pConfig = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetNumericalConfig(infoCfg.shop.nItemId);
						if(pConfig) {
							sprintf_s(logs,64,"%s|%d|%d",pConfig->name,nNum,nShopId);
							logs[63] = 0;
						}
					}
					

					int reslut = pActor->RemoveConsume(infoCfg.price.nType, infoCfg.price.nId, (infoCfg.price.nCount*nNum), -1, -1, -1, 0,GameLog::Log_ShopBuy,logs);
					if(reslut)
					{
						if(infoCfg.nStaticType > 0)
							pActor->GetStaticCountSystem().SetStaticCount(infoCfg.nStaticType, nValue+nNum);
						
						//购买商品广播消息
						if (strlen(infoCfg.tips) > 0)
						{
							char temp[1024];

							//获取商品信息
							const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(infoCfg.shop.nItemId);

							sprintf(temp, infoCfg.tips, pActor->GetEntityName(), pStdItem->m_sName);
							GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(temp, tstChatSystem);
						}
						
						pActor->GiveAward(infoCfg.shop.nType, infoCfg.shop.nItemId, infoCfg.shop.nCount*nNum,0,0,0,0,GameLog::Log_ShopBuy);
					}
					//统计商品购买
					GetLogicServer()->SendShopLocalLog(infoCfg.shop.nItemId, pActor, nType, infoCfg.shop.nType, infoCfg.shop.nCount*nNum, infoCfg.price.nType, infoCfg.price.nCount*nNum);
					CActorPacket ap;
					CDataPacket & dataPacket = pActor->AllocPacket(ap);
					dataPacket <<(BYTE)enStoreSystemID <<(BYTE)sStoreSystemBuyShop;
					dataPacket << (BYTE)nShopId << (BYTE)(reslut != 0 ?1:0);
					dataPacket << (int)times;
					int buyTimes = nValue + nNum;
					dataPacket << (int)buyTimes;
					ap.flush();
				}
			}	
		}
	}
}

void CGameStoreBuySystem::ShowNpcShop(BYTE nType, BYTE ntab)
{
	CActor* pActor = (CActor*)m_pEntity;
	if(!pActor) return ;

	CGameStore& store= GetLogicServer()->GetDataProvider()->GetGameStore();
	std::map<int, std::map<int, std::vector<ShopConfig> > >::iterator ot = store.m_shops.find(nType);
	if(ot != store.m_shops.end())
	{
		std::map<int, std::vector<ShopConfig> >& shopLists = ot->second;
		std::map<int, std::vector<ShopConfig> >::iterator it = shopLists.find(ntab);
		if(it != shopLists.end())
		{
			CActorPacket  ap;
			CDataPacket & dataPacket = pActor->AllocPacket(ap);
			dataPacket <<(BYTE)enStoreSystemID <<(BYTE)sStoreSystemShopInfo;

			int nCount = it->second.size();
			dataPacket << (BYTE) nType;
			dataPacket << (BYTE) ntab;
			int pos = dataPacket.getPosition();
			dataPacket << (BYTE) nCount;

			int nDays = GetLogicServer()->GetDaysSinceOpenServer();

			std::vector<ShopConfig>::iterator ot = it->second.begin();
		
			BYTE nResult = 0;
			for(;ot != it->second.end(); ot++)
			{
				ShopConfig& infoCfg = *ot;
				if(!pActor->CheckLevel(infoCfg.nLimitLv, infoCfg.nReincarnationlimit) || nDays < infoCfg.nDaylimit)
				{
					continue;
				}
				int times =store.getLimitTimes(infoCfg.buyLimit, pActor, nDays);
				dataPacket << (int)infoCfg.nShopId;
				int nValue = 0;
				if(infoCfg.nStaticType > 0)
					nValue = pActor->GetStaticCountSystem().GetStaticCount(infoCfg.nStaticType);
				dataPacket << (BYTE)(nValue);
				dataPacket << (int)times;
				dataPacket << (BYTE)infoCfg.nFlag;
				dataPacket << (BYTE)infoCfg.nStaticType;

				nResult++;
			}
			BYTE* pCount = (BYTE*)dataPacket.getPositionPtr(pos);
			*pCount = nResult;
			ap.flush();
		}
	}
}
