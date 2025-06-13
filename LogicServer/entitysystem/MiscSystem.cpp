#include "StdAfx.h"
#include "MiscSystem.h"
#include "FvMask.h"
#include "GameSets.h" 
//static const char* szBuyBanneretTimes[32] = {"buyBanneretTimes0", "buyBanneretTimes1"};
 
//网络数据处理
void CMiscSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &inPack )
{
	if(m_pEntity ==NULL) return;
	//这里进行数据处理
	BYTE nId, buyIdx;
	LPCSTR sName ;
	CScriptValueList  paramList; 
	switch(nCmd)
	{
	case cBuyMonthCard:// 购买月卡
		{
			BYTE mtype = 0;
			inPack >> mtype;
			OnBuyMonthCard(mtype);
			break;
		}
	case cGetMonthCardAward:// 获取月卡奖励
		{
			BYTE mtype = 0;
			inPack >> mtype;
			OnGetMonthCardAward(mtype);
			break;
		}
	case cBuyFrenzy:// 购买狂暴
		{
			CActor* pActor = (CActor *)m_pEntity;
			FrenzyCfg const& cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetFrenzyCfg();
			
			// 已激活则提示
			int nIsFrenzy = m_pEntity->GetProperty<int>(PROP_ACTOR_FRENZY);
			if (nIsFrenzy > 0)
			{
				pActor->SendTipmsgFormatWithId(tmFrenzyReOpen,tstUI);
				break;
			}


			// 检查会员等级
			if ( pActor->GetMiscSystem().GetMaxColorCardLevel() < cfg.nCardLv )
			{
				pActor->SendTipmsgFormatWithId(tmtmFrenzyCardLvLimit,tstUI);
				return;
			}


			// 检查消耗
			for(int  i = 0; i < cfg.vecConsum.size(); i++)
			{
				GOODS_TAG item;
				std::tie(item.type, item.id, item.count) = cfg.vecConsum[i];
				if( !(pActor->CheckActorSource(item.type, item.id, item.count,tstUI)) )
				{
					return;
				}
			}
			
			// 消耗
			for(int  i = 0; i < cfg.vecConsum.size(); i++)
			{
				GOODS_TAG item;
				std::tie(item.type, item.id, item.count) = cfg.vecConsum[i];
				pActor->RemoveConsume(item.type, item.id, item.count,-1,-1,-1,0, GameLog::Log_BuyFrezy);
			}
			
			// 提示
			pActor->SendTipmsgFormatWithId(tmFrenzyOpenSucc,tstUI);

			// 激活之
			pActor->SetProperty<int>(PROP_ACTOR_FRENZY,1);
			pActor->GetPropertySystem().ResertProperty(); //重置属性
		}
		break;
	case cUpgreadOffice:// 升级官阶
		{
			CActor* pActor = (CActor *)m_pEntity;
			int nOfficeLvl = pActor->GetProperty<int>(PROP_ACTOR_OFFICE) + 1;
			std::vector<OfficeCfg> const& vecCfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetOfficeCfg();

			if (nOfficeLvl >= vecCfg.size())
			{
				//满级提示
				pActor->SendTipmsgFormatWithId(tmMaxOffice,tstUI);
				return;
			}

			OfficeCfg const& cfg = vecCfg[nOfficeLvl];

			// 等级检查
			int level = pActor->GetLevel();
			int circle = pActor->GetCircleLevel();
			if (cfg.nLvlLimit > level)
			{
				pActor->SendTipmsgFormatWithId(tmLevelLimited,tstUI);
				return;
			}
			if (cfg.nCircleLimit > circle)
			{
				pActor->SendTipmsgFormatWithId(tmCircleNotEnough,tstUI);
				return;
			}
			
			// 检查消耗
			for(int  i = 0; i < cfg.vecConsum.size(); i++)
			{
				GOODS_TAG item;
				std::tie(item.type, item.id, item.count) = cfg.vecConsum[i];
				if( !(pActor->CheckActorSource(item.type, item.id, item.count, tstUI)) )
				{
					return;
				}
			}
			
			// 消耗
			for(int  i = 0; i < cfg.vecConsum.size(); i++)
			{
				GOODS_TAG item;
				std::tie(item.type, item.id, item.count) = cfg.vecConsum[i];
				pActor->RemoveConsume(item.type, item.id, item.count,-1,-1,-1,0, GameLog::Log_Office);
			}
			
			// 提示
			pActor->SendTipmsgFormatWithId(tmUpgreadOffice,tstUI);

			// 走马灯
			if (cfg.nIsNotice)
			{
				GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmOfficeBroad,tstBigRevolving,
					pActor->GetEntityName(), cfg.sName);	
			}

			// 激活之
			pActor->SetProperty<int>(PROP_ACTOR_OFFICE, nOfficeLvl);
			pActor->GetPropertySystem().ResertProperty(); //重置属性
		}
		break;
	case cGetActorInfo:
		OnViewActorInfo(inPack);
		break;
	case cFcmExitGame:
		m_pEntity->CloseActor(lwiFcmExitTick, false); 
		break;
	case cRankingData:
		//sName= NULL;
		// inPack >> sName;
		// SendRankingData(sName);
		//GetGlobalLogicEngine()->GetTopTitleMgr().SendTopRankData(m_pEntity);
		break;
	case cGetNearActor:
		{
			if (m_pEntity == NULL)
			{
				return;
			}

			const CVector<EntityHandleTag>& entityList = m_pEntity->GetObserverSystem()->GetVisibleList();
			unsigned int nActorCount = 0;
			CActorPacket pack;
			CDataPacket &data =  m_pEntity->AllocPacket(pack);
			data << (BYTE)enMiscSystemID << (BYTE)sGetNearActor;
			size_t pos = data.getPosition();
			data << nActorCount;				//先随便写个数字，后面会写个正确的进来
			for (INT_PTR i = 0; i < entityList.count(); i++)
			{
				CEntity* pEntity = GetEntityFromHandle(entityList[i].m_handle);
				if (pEntity && pEntity->GetType() == enActor)
				{
					CActor* pActor = (CActor*)pEntity;
					data << (unsigned int)(pEntity->GetProperty<int>(PROP_ENTITY_ID));
					data.writeString(pEntity->GetEntityName());
					data << (BYTE)(pEntity->GetProperty<int>(PROP_ACTOR_VOCATION));
					data << (unsigned int)(pEntity->GetProperty<int>(PROP_CREATURE_LEVEL));
					data << (BYTE)(pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE));
					data << (BYTE)(pEntity->GetProperty<int>(PROP_ENTITY_ICON));
					data << (BYTE)(pEntity->GetProperty<int>(PROP_ACTOR_SEX));
					CGuild* pMyGuild =((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
					if(NULL == pMyGuild)
					{
						data.writeString("");
					}
					else
					{
						data.writeString(pMyGuild->m_sGuildname);	//自己的公会名字
					}
					data << (unsigned int)(pEntity->GetProperty<int>(PROP_ACTOR_SUPPER_PLAY_LVL));
					nActorCount++;
				}
				
			}
			unsigned int* pCount = (unsigned int*)data.getPositionPtr(pos);
			*pCount = nActorCount;
			pack.flush();
		}
		break;
	case cGetServerTime:
		OnGetServerTime();
		break;
	case cCircleCount:
		SendCircleCount();
		break;

	case cEveryDayCheck:				//签到
		{
			break;
		}

	case cSaveGameSetData:		//保存游戏设置的数据
		{
			SaveGameSetData(inPack);
			break;
		}

	case cLoginBYLoginTools:	//是否使用登陆器登陆
		{
			BYTE nType;
			inPack >> nType;
			if(nType == 1)
			{
				CActor * pActor = (CActor *)m_pEntity;
				//pActor->GetAchieveSystem().OnEvent(eAchieveEventLogin,nType,pActor->GetLoginDaysValue());
				LogSender* pLogger = GetLogicServer()->GetLogClient();
				if (pLogger)
				{
					//记录日志
					pLogger->SendLoginLog(ltEntryGameByTools,pActor->GetAccountID(),pActor->GetAccount(),"",pActor->GetEntityName(),0,0,0,pActor->GetId(), pActor->getOldSrvId());
				}
				// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtClientEvent, CQuestData::ceDownloadLauncher, 1);
				m_isLauncherLogic = true;
			}
			break;
		}

	case cGetActorBuffData:		//下发玩家buff数据
		{
			INT64 nHnadle = 0;
			inPack >> nHnadle;
			CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(nHnadle);
			if(pEntity && (pEntity->GetType() == enActor || pEntity->GetType() == enPet || pEntity->GetType() == enHero))
			{
				CActorPacket pack;
				CDataPacket &data = m_pEntity->AllocPacket(pack);
				data <<BYTE(enMiscSystemID) <<(BYTE)sSendActorBuffData; //是别的玩家出现了
				if(pEntity->GetType() == enActor)
				{
					((CActor*)pEntity)->GetBuffSystem()->WriteBuffData(data); //把玩家的buff数据写进去
				}
				else
				{
					((CAnimal*)pEntity)->GetBuffSystem()->WriteBuffData(data);
				}
				pack.flush();
			}
			break;
		}

	case cChangeNameOp:
		{
			OnChangeActorName(inPack);
			break;
		}
	case cGetActorInfoByNameFuzzy:
		{
			unsigned char nType;
			char sName[32] = {0};
			inPack >> nType;
			inPack.readString(sName,ArrayCount(sName));
			sName[sizeof(sName)-1] = 0;
			SendActorInfoByNameFuzzy(nType, sName);
			break;
		}
	case cGetActorIdInRank:
		{
			int nRankid = 0;
			inPack >> nRankid;
			unsigned int nActorId = 0;
			CRanking* pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(nRankid);
			if (pRank)
			{
				if(nRankid == enMoBaiRankList)
				{
					char nCondition[32] = {0};
					inPack.readString(nCondition,ArrayCount(nCondition));
					nActorId = pRank->GetActorIdBySexJob(nCondition);
				}
			}
			CActorPacket pack;
			CDataPacket &data = m_pEntity->AllocPacket(pack);
			data <<BYTE(enMiscSystemID) <<(BYTE)sGetActorIdInRank;
			data << (unsigned int )nActorId;
			pack.flush();
			break;
		}
	case cGetMoBaiInfo:
	{
		short nRankId;
		BYTE nCount;
		inPack >> nRankId;
		inPack >> nCount;

		if (CRanking* pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(nRankId))
		{
			CActorPacket actorPack;
			CDataPacket& netPack = m_pEntity->AllocPacket(actorPack);
			netPack << (byte)enMiscSystemID << (byte)sGetMoBaiInfo;
			netPack << nRankId;
			pRank->PushRankToPack(nCount, &netPack);
			actorPack.flush();
		}
		break;
	}
	case cGetCenterMoBaiInfo:
	{
		short nRankId;
		BYTE nCount;
		inPack >> nRankId;
		inPack >> nCount;
		if((short)enCSMoBaiRank != nRankId)
		{
			break; 
		}
		if (CRanking* pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(enCSMoBaiRank))
		{
			// if((int nItemCount = pRank->GetList().count())  <= 0) {
			// 	GetGlobalLogicEngine()->GetRankingMgr().LoadCenterRankData(enCSMoBaiRank);
			// }
			CActorPacket actorPack;
			CDataPacket& netPack = m_pEntity->AllocPacket(actorPack);
			netPack << (byte)enMiscSystemID << (byte)sGetCenterMoBaiInfo;
			netPack << nRankId;
			pRank->PushRankToPack(nCount, &netPack);
			actorPack.flush();
		}
		break;
	}

	case cGetSbkRank:
	{
		short nRankId;
		BYTE nCount;
		inPack >> nRankId;
		inPack >> nCount;

		if (CRanking* pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(nRankId))
		{
			CActorPacket actorPack;
			CDataPacket& netPack = m_pEntity->AllocPacket(actorPack);
			netPack << (byte)enMiscSystemID << (byte)sGetSbkRank;
			netPack << nRankId;
			pRank->PushEspecialRankToPack(nCount, &netPack);
			actorPack.flush();
		}
		break;
	}

	case cGetsbkOfflineInfo:
	{
		CActorPacket ap;
		CDataPacket& data = m_pEntity->AllocPacket(ap);
		data << (BYTE)enMiscSystemID << (BYTE)sGetsbkOfflineInfo;
		GetGlobalLogicEngine()->GetGuildMgr().getsbkOfflineData(data);
		ap.flush();
		break;
	}
	default:
		break;
	}
}

void CMiscSystem::OnBuyMonthCard(BYTE mtype)
{
	//（1.免费特权，2.月卡，3.大药月卡，4.永久卡）
	// (5/11.白卡元宝银两，6.绿卡，7.蓝卡，8.紫卡，9.橙卡)
	if (m_pEntity == NULL) return;
	//int  flag = 0; 
	//if(mtype == 5) flag = 1 ;//白卡, 消耗银两或者元宝,配置配了银两,优先消耗银两,支持消耗元宝

	// 获取配置
	MONTHCARDCONFIG *pConf = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetMonthCardConfig(mtype);
	if (pConf)
	{
		// if( !flag )//非白卡正常按配置消耗
		// {
			// 检查消耗
			if( !(((CActor*)m_pEntity)->CheckActorSource(pConf->Consume.type, pConf->Consume.id, pConf->Consume.count, tstUI)) )
			{
				return;
			}
		// }
		// else //白卡先按配置消耗银两，不够再用元宝消耗
		// {
		// 	bool returnFlag = true ;
		// 	// 检查消耗
		// 	if( (((CActor*)m_pEntity)->CheckConsume(pConf->Consume.type, pConf->Consume.id, pConf->Consume.count))  )
		// 	{	
		// 		returnFlag = false ;
		// 		flag = 2 ;//耗银两
		// 	}
		// 	//银两不够检查元宝
		// 	if( returnFlag && (((CActor*)m_pEntity)->CheckConsume(4, 4, pConf->Consume.count))  )
		// 	{	
		// 		returnFlag = false ;
		// 		flag = 3 ;//耗元宝
		// 	}
			
		// 	if(returnFlag )
		// 	{
		// 		if (pConf->Consume.type >= 0)
		// 		{
		// 			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmNeedItemNotEnough,tstUI);
		// 			return;
		// 		}

		// 	}
		// }
		
		

		//检测背包格子
		if(!((CActor*)m_pEntity)->GetBagSystem().bagIsEnough(BagAtvCommon))
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmGiftNoBagNum,tstUI);
			return;
		}

		int nCardLogType = GameLog::Log_MonthCard;
		std::string sLogComment = "激活月卡";
		// 设置月卡标志
		switch (mtype)
		{
		case 1://免费特权
			{
				if (IsHasFreePrivilege()) return;
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				val |= 1;
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);
				nCardLogType = GameLog::Log_FreeCard;
				sLogComment = "初心契约";
			}
			break;
		case 2://月卡
			{
				if (IsHasMonthCard()) return;
				unsigned int expire = CMiniDateTime::today() + (30 * 24 * 3600);
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_MONTH_CARD,(unsigned int)expire);
				sLogComment = "月卡";
			}
			break;
		case 3://大药月卡
			{
				if (IsHasMedicineCard()) return;
				unsigned int expire = CMiniDateTime::today() + (30 * 24 * 3600);
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_MEDICINE_CARD,(unsigned int)expire);
				m_pEntity->SendTipmsgFormatWithId(tmDaYaoYueKaGongGao, tstRevolving);
				nCardLogType = GameLog::Log_MEDICINECard;
				sLogComment = "大药月卡";
			}
			break;
		case 4://永久卡：永恒契约
			{
				if (IsHasForverCard()) return;
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				val |= 2;
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);
				m_pEntity->SendTipmsgFormatWithId(tmYongHengGongGao, tstRevolving);
				m_pEntity->SendTipmsgFormatWithId(tmYongHengGongGao, tstChatNotice);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "永久契约";
			}	
			break;
		case 11://永久卡：白卡会员./bu
		case eMembershipComm_White://永久卡：白卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				if (FvMask::HasAny(val, _MASK_(eMembershipLv_White)))
				{
					m_pEntity->SendTipmsg("您已是白卡会员", tstUI); 
					return;
				}
				FvMask::Add(val, _MASK_(eMembershipLv_White));
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);

				if ( m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_RECOVERSTATE) == 0 )
				{
					m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_RECOVERSTATE,2);
				}
				
				char* sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmBaiKaGongGao);
				if(strlen(sFormat) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmBaiKaGongGao, tstRevolving,m_pEntity->GetEntityName());
				}

				char* sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmBaiKaLiaoTianXiTongGongGao);
				if(strlen(sFormat2) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmBaiKaLiaoTianXiTongGongGao, tstChatSystem,m_pEntity->GetEntityName());
				}
				
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "白卡会员";
			}	
			break;
		case eMembershipComm_Green://永久卡：绿卡会员
			{ 
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				if (FvMask::HasAny(val, _MASK_(eMembershipLv_Green)))
				{
					m_pEntity->SendTipmsg("您已是绿卡会员", tstUI); 
					return;
				} 
				else 
				if(!FvMask::HasAny(val, _MASK_(eMembershipLv_White)))
				{ 
					m_pEntity->SendTipmsg("仅对白卡会员开放", tstUI);
					return;
				}
				//if (IsHasGreenCard() ) { m_pEntity->SendTipmsg("您已是绿卡会员", tstUI); return;}
				//	else if(!IsHasWhiteCard()) { m_pEntity->SendTipmsg("仅对白卡会员开放", tstUI); return;}
				//unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				//val |= (1<<4);
				FvMask::Add(val, _MASK_(eMembershipLv_Green));
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);

				char* sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmLvKaGongGao);
				if(strlen(sFormat) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmLvKaGongGao, tstRevolving,m_pEntity->GetEntityName());
				}
				
				char* sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmLvKaLiaoTianXiTongGongGao);
				if(strlen(sFormat2) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmLvKaLiaoTianXiTongGongGao, tstChatSystem,m_pEntity->GetEntityName());
				}
				
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "绿卡会员";
			}	
			break;
		case eMembershipComm_Blue://永久卡：蓝卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				if (FvMask::HasAny(val, _MASK_(eMembershipLv_Blue)))
				{
					m_pEntity->SendTipmsg("您已是蓝卡会员", tstUI); 
					return;
				} 
				else 
				if(!FvMask::HasAny(val, _MASK_(eMembershipLv_Green)))
				{ 
					m_pEntity->SendTipmsg("仅对绿卡会员开放", tstUI);
					return;
				}
				FvMask::Add(val, _MASK_(eMembershipLv_Blue));
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);

				char* sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmLanKaGongGao);
				if(strlen(sFormat) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmLanKaGongGao, tstRevolving,m_pEntity->GetEntityName());
				}

				char* sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmLanKaLiaoTianXiTongGongGao);
				if(strlen(sFormat2) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmLanKaLiaoTianXiTongGongGao, tstChatSystem,m_pEntity->GetEntityName());
				}
				
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "蓝卡会员";
			}	
			break;
		case eMembershipComm_Violet://永久卡：紫卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				if (FvMask::HasAny(val, _MASK_(eMembershipLv_Violet)))
				{
					m_pEntity->SendTipmsg("您已是紫卡会员", tstUI); 
					return;
				} 
				else 
				if(!FvMask::HasAny(val, _MASK_(eMembershipLv_Blue)))
				{ 
					m_pEntity->SendTipmsg("仅对蓝卡会员开放", tstUI);
					return;
				}
				FvMask::Add(val, _MASK_(eMembershipLv_Violet));
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);

				char* sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmZiKaGongGao);
				if(strlen(sFormat) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmZiKaGongGao, tstRevolving,m_pEntity->GetEntityName());
				}

				char* sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmZiKaLiaoTianXiTongGongGao);
				if(strlen(sFormat2) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmZiKaLiaoTianXiTongGongGao, tstChatSystem,m_pEntity->GetEntityName());
				}
				
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "紫卡会员";
			}	
			break;
		case eMembershipComm_Orange://永久卡：橙卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				if (FvMask::HasAny(val, _MASK_(eMembershipLv_Orange)))
				{
					m_pEntity->SendTipmsg("您已是橙卡会员", tstUI); 
					return;
				} 
				else 
				if(!FvMask::HasAny(val, _MASK_(eMembershipLv_Violet)))
				{ 
					m_pEntity->SendTipmsg("仅对紫卡会员开放", tstUI);
					return;
				}

				FvMask::Add(val, _MASK_(eMembershipLv_Orange));
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);

				char* sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmChengKaGongGao);
				if(strlen(sFormat) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmChengKaGongGao, tstRevolving,m_pEntity->GetEntityName());
				}

				char* sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmChengKaLiaoTianXiTongGongGao);
				if(strlen(sFormat2) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmChengKaLiaoTianXiTongGongGao, tstChatSystem,m_pEntity->GetEntityName());
				}
				
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "橙卡会员";
			}	
			break; 
		case eMembershipComm_OrangeStar://永久卡：橙星会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				
				if (FvMask::HasAny(val, _MASK_(eMembershipLv_OrangeStar)))
				{
					m_pEntity->SendTipmsg("您已是橙星会员", tstUI); 
					return;
				} 
				else 
				if(!FvMask::HasAny(val, _MASK_(eMembershipLv_Orange)))
				{ 
					m_pEntity->SendTipmsg("仅对橙卡会员开放", tstUI);
					return;
				}
				FvMask::Add(val, _MASK_(eMembershipLv_OrangeStar));
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);

				char* sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmChengXinGongGao);
				if(strlen(sFormat) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmChengXinGongGao, tstRevolving,m_pEntity->GetEntityName());
				}

				char* sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmChengXinLiaoTianXiTongGongGao);
				if(strlen(sFormat2) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmChengXinLiaoTianXiTongGongGao, tstChatSystem,m_pEntity->GetEntityName());
				} 
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "橙星会员";
			}	
			break;
		case eMembershipComm_OrangeMoon://永久卡：橙月会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				
				if (FvMask::HasAny(val, _MASK_(eMembershipLv_OrangeMoon)))
				{
					m_pEntity->SendTipmsg("您已是橙月会员", tstUI); 
					return;
				} 
				else 
				if(!FvMask::HasAny(val, _MASK_(eMembershipLv_OrangeStar)))
				{ 
					m_pEntity->SendTipmsg("仅对橙星会员开放", tstUI);
					return;
				}
				FvMask::Add(val, _MASK_(eMembershipLv_OrangeMoon));
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);

				char* sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmChengYueGongGao);
				if(strlen(sFormat) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmChengYueGongGao, tstRevolving,m_pEntity->GetEntityName());
				}

				char* sFormat2 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmChengYueLiaoTianXiTongGongGao);
				if(strlen(sFormat2) > 0)
				{
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmChengYueLiaoTianXiTongGongGao, tstChatSystem,m_pEntity->GetEntityName());
				} 
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "橙月会员";
			}	
			break;
		default:
			break;
		}
		CScriptValueList paramList;
		paramList << ((CActor*)m_pEntity);
		GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("UpdateTreeTimes", paramList, paramList, 0);


		// if( !flag )//非白卡正常按配置消耗
		// {
			// 扣除消耗
			if (pConf->Consume.type >= 0)
			{
				((CActor*)m_pEntity)->RemoveConsume(pConf->Consume.type, pConf->Consume.id, pConf->Consume.count,-1,-1,-1,0,
					nCardLogType,sLogComment.c_str());
			}

		// }
		// else
		// {	
		// 	if(flag == 2 )
		// 	{	
		// 		// 按配置扣银两
		// 		if (pConf->Consume.type >= 0)
		// 		{
		// 			((CActor*)m_pEntity)->RemoveConsume(pConf->Consume.type, pConf->Consume.id, pConf->Consume.count,-1,-1,-1,0,
		// 				nCardLogType,sLogComment.c_str());
		// 		}
		// 	}
		// 	if(flag == 3 )
		// 	{	
		// 		// 银两不够扣元宝
		// 		if (pConf->Consume.type >= 0)
		// 		{
		// 			((CActor*)m_pEntity)->RemoveConsume(4, 4, pConf->Consume.count,-1,-1,-1,0,
		// 				nCardLogType,sLogComment.c_str());
		// 		}

		// 	}

		// }
		


		// 扣除消耗
		// if (pConf->Consume.type >= 0)
		// {
		// 	((CActor*)m_pEntity)->RemoveConsume(pConf->Consume.type, pConf->Consume.id, pConf->Consume.count,-1,-1,-1,0,
		// 		nCardLogType,sLogComment.c_str());
		// }

		// 首次购买的奖励
		if (pConf->BuyAwardList[0].type >= 0)
		{
			for (int i = 0; i < 6; i++)
			{	
				GOODS_TAG& award = pConf->BuyAwardList[i];
				if (award.type >=0)
				{
					((CActor*)m_pEntity)->GiveAward(award.type, award.id, award.count, 0,0,0,0,
						nCardLogType,sLogComment.c_str());
				}
			}
			((CActor*)m_pEntity)->SendTipmsgWithId(tmConsiBuySuccAddToBag,tstUI);
		}
		((CActor*)m_pEntity)->UpdateUseDeopt();

		// 推送月卡信息
	}
}

void CMiscSystem::OnGetMonthCardAward(BYTE mtype)
{
	
	//（1.免费特权，2.月卡，3.大药月卡，4.永久卡）
	// (5.白卡，6.绿卡，7.蓝卡，8.紫卡，9.橙卡)
	if (m_pEntity == NULL) return;
	int nCardLogType = GameLog::Log_MonthCard;
	std::string sLogComment = "激活月卡";
	// 获取配置
	MONTHCARDCONFIG *pConf = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetMonthCardConfig(mtype);
	if (pConf)
	{
		// 检查月卡有效
		switch (mtype)
		{
		case 1://永久卡（初心契约）
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_FreeCard;
				sLogComment = "初心契约";
				if ((val & 1) == 0) return;
			}
			break;
		case 2://月卡
			{
				unsigned int expire = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_MONTH_CARD);
				sLogComment = "月卡";
				if (expire < GetGlobalLogicEngine()->getMiniDateTime()) return;
			}
			break;
		case 3://大药月卡
			{
				unsigned int expire = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_MEDICINE_CARD);
				nCardLogType = GameLog::Log_MEDICINECard;
				sLogComment = "大药月卡";
				if (expire < GetGlobalLogicEngine()->getMiniDateTime()) return;
			}
			break;
		case 4://永久卡（永恒契约）
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "永久契约";
				if ((val & 2) == 0) return;
			}
			break;
		case eMembershipComm_White://永久卡 白卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "白卡会员";
				if ((val & (1<<3)) == 0) return;
			}
			break;
		case eMembershipComm_Green://永久卡 绿卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "绿卡会员";
				if ((val & (1<<4)) == 0) return;
			}
			break;
		case eMembershipComm_Blue://永久卡 蓝卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "蓝卡会员";
				if ((val & (1<<5)) == 0) return;
			}
			break;
		case eMembershipComm_Violet://永久卡 紫卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "紫卡会员";
				if ((val & (1<<6)) == 0) return;
			}
			break;
		case eMembershipComm_Orange://永久卡 橙卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "橙卡会员";
				if ((val & (1<<7)) == 0) return;
			}
		case eMembershipComm_OrangeStar://永久卡橙星卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "橙星卡会员";
				if ((val & (1<<8)) == 0) return;
			}
			break;
		case eMembershipComm_OrangeMoon://永久卡橙月卡会员
			{
				unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
				nCardLogType = GameLog::Log_ForeverCard;
				sLogComment = "橙月卡会员";
				if ((val & (1<<9)) == 0) return;
			}
			break;
		default:
			break;
		}
		//检测背包格子
		if(!((CActor*)m_pEntity)->GetBagSystem().bagIsEnough(BagAtvCommon))
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmGiftNoBagNum,tstUI);
			return;
		}

		// 检查当日领取标记
		unsigned int flag = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CARD_AWARD_FLAG);
		if ((flag & (1 << (mtype - 1)))) return;

		// 设置标记
		flag |= (1 << (mtype - 1));
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_CARD_AWARD_FLAG,(unsigned int)flag);	

		// 获取日常奖励
		if (pConf->DailyAwardList[0].type >= 0)
		{
			for (int i = 0; i < 6; i++)
			{	
				GOODS_TAG& award = pConf->DailyAwardList[i];
				if (award.type >=0)
				{
					((CActor*)m_pEntity)->GiveAward(award.type, award.id, award.count, 0,0,0,0,nCardLogType,sLogComment.c_str());
				}
			}
			((CActor*)m_pEntity)->SendTipmsgWithId(tmMailGetItemSuccess,tstUI);
		}
	}
}

bool CMiscSystem::IsHasFreePrivilege()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & 1);
}
bool CMiscSystem::IsHasMonthCard()
{
	unsigned int expire = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_MONTH_CARD);
	return (expire > GetGlobalLogicEngine()->getMiniDateTime());
}
bool CMiscSystem::IsHasMedicineCard()
{
	unsigned int expire = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_MEDICINE_CARD);
	return (expire > GetGlobalLogicEngine()->getMiniDateTime());
}
bool CMiscSystem::IsHasForverCard()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & 2);
}

bool CMiscSystem::IsHasShouChongCard()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & (1<<2));
}

bool CMiscSystem::IsHasWhiteCard()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & (1<<3));
}
bool CMiscSystem::IsHasGreenCard()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & (1<<4));
}
bool CMiscSystem::IsHasBlueCard()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & (1<<5));
}
bool CMiscSystem::IsHasPurpleCard()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & (1<<6));
}
bool CMiscSystem::IsHasOrangeCard()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	return (val & (1<<7));
}

int CMiscSystem::GetMaxColorCardLevel()
{
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_OrangeMoon)))
	{
		return 7;
	} 
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_OrangeStar)))
	{
		return 6;
	} 
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_Orange)))
	{
		return 5;
	} 
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_Violet)))
	{
		return 4;
	} 
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_Blue)))
	{
		return 3;
	}
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_Green)))
	{
		return 2;
	}
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_White)))
	{
		return 1;
	} 
	return 0 ;
	/*
	if(IsHasOrangeCard())
		return 5 ;
	else if(IsHasPurpleCard())
		return 4 ;
	else if(IsHasBlueCard())
		return 3 ;
	else if(IsHasGreenCard())
		return 2 ;
	else if(IsHasWhiteCard())
		return 1 ;
	return 0 ;
	*/
}

int CMiscSystem::GetMaxExpDampColorCard()
{
	//非会员或者使用默认衰减的返回0 
	int colorCardLevel = GetMaxColorCardLevel()  ;
	if( !colorCardLevel ) return 0 ;
	MONTHCARDCONFIG *pConf =GetLogicServer()->GetDataProvider()->GetMiscConfig().GetMonthCardConfigbySuperLV(colorCardLevel);
	int nDampNum = 0 ;
	if(pConf && pConf->nSuperRightLV == colorCardLevel)
	{
		nDampNum = pConf->nMaxExpDampNum;	
	}
	return nDampNum ;
}


int CMiscSystem::GetJiShouHangMaxCount()
{
	//非会员或者使用默认衰减的返回0 
	int colorCardLevel = GetMaxColorCardLevel()  ;
	if( !colorCardLevel ) return 0 ;
	MONTHCARDCONFIG *pConf =GetLogicServer()->GetDataProvider()->GetMiscConfig().GetMonthCardConfigbySuperLV(colorCardLevel);
	int nJiShouNum = 0 ;
	if(pConf && pConf->nSuperRightLV == colorCardLevel)
	{
		nJiShouNum = pConf->nJiShouNum;	
	}
	return nJiShouNum ;
}


int CMiscSystem::GetSuperRightAtvPaoDianPlus()
{
	int colorCardLevel = GetMaxColorCardLevel()  ;
	int nRetNum =  0 ;
	if( colorCardLevel ) 
	{
		MONTHCARDCONFIG *pConf =GetLogicServer()->GetDataProvider()->GetMiscConfig().GetMonthCardConfigbySuperLV(colorCardLevel);
		if(pConf && pConf->nSuperRightLV == colorCardLevel)
		{
			nRetNum = pConf->nAtvPaoDianPlus;	
		}
	}
	else if(nRetNum ==0 && IsHasShouChongCard())
	{
		MONTHCARDCONFIG *pConf =GetLogicServer()->GetDataProvider()->GetMiscConfig().GetShouChongCardConfig();
		if(pConf)
		{
			nRetNum = pConf->nAtvPaoDianPlus;	
		}
	}
	return nRetNum;
}




void CMiscSystem::CheckLauncherLogin()
{
	if (m_pEntity == NULL)
	{
		return;
	}
	if (m_isLauncherLogic)
	{
		// m_pEntity->GetQuestSystem()->OnQuestEvent(CQuestData::qtClientEvent, CQuestData::ceDownloadLauncher, 1);
	}
}

//通过金钱的类型获取金钱的名字
LPCSTR CMiscSystem::GetMoneyName(INT_PTR nMoneyType)
{
	INT_PTR nTipMsgId =0;
	switch(nMoneyType)
	{
	case mtBindCoin:
		nTipMsgId = tpMoneyTypeBindCoin;
		break;
	case mtCoin:
		nTipMsgId = tpMoneyTypeCoin;
		break;
	case mtBindYuanbao:
		nTipMsgId = tpMoneyTyeBindYuanbao;
		break;
	case mtYuanbao:
		nTipMsgId = tpMoneyTypeYuanbao;
		break;
	}
	if(nTipMsgId)
	{
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipMsgId);
	}
	else
	{
		return NULL;
	}
}

//通过奖励的类型获取奖励的名字
LPCSTR CMiscSystem::GetAwardName(INT_PTR nType)
{	
	INT_PTR nTipMsgId =0;
	switch(nType)
	{
	case qatBindMoney:
		nTipMsgId = tpMoneyTypeBindCoin;
		break;
	case qatMoney:
		nTipMsgId = tpMoneyTypeCoin;
		break;
	case qatBindYb:
		nTipMsgId = tpMoneyTyeBindYuanbao;
		break;
	case qatYuanbao:
		nTipMsgId = tpMoneyTypeYuanbao;
		break;
	case qatCombatScore:
		nType = tpCombatScore;
		break;
	}
	if(nTipMsgId)
	{
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipMsgId);
	}
	else
	{
		return NULL;
	}
}

void CMiscSystem::OnGetServerTime()
{
	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data << (BYTE)GetSystemID() << (BYTE)sSendServerTime ;
	data << (unsigned int)GetGlobalLogicEngine()->getMiniDateTime();
	data << (unsigned int)GetLogicServer()->GetServerOpenTime();
	ap.flush();
}
void CMiscSystem::OnEnterGame()
{	
	DECLARE_TIME_PROF("CMiscSystem::OnEnterGame");

	InitRankPropData();	//初始化属性排行榜

	LoadGameSetData();

	SendTempCombineTime();

	CMiniDateTime nLastLogOutTime = m_pEntity->GetLastLogoutTime();
	CMiniDateTime nLoginTime = m_pEntity->GetLoginTime();
	if (!nLastLogOutTime.isSameDay(nLoginTime))	
	{
		
	}
	if(! GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen) return;

	//嘟嘟传奇后台只做经验限制,其他的平台来做
	// if(!m_hasSendLoginTipmsg )
	// {
	// 	//玩家第一次登陆时，系统提示他已经纳入防沉迷了
	// 	m_pEntity->SendOldTipmsgWithId(tpFcmNotifyBeFcm, tstRevolving);

	// 	if(! m_isAdult) //	账号已纳入防沉迷系统，是否需要进行身份信息填写
	// 	{ 
	// 		// if(m_pEntity->m_isFirstLogin) deled by caphone 2011.5.9 未成年每次登陆都需给出提示
	// 		{
	// 			CActorPacket ap;
	// 			CDataPacket& data = m_pEntity->AllocPacket(ap);
	// 			data << (BYTE)GetSystemID() << (BYTE)sWriteFcmInfo ;
	// 			ap.flush();
	// 		}
	// 	}
	// 	//第1次登陆需要提示玩家，去填写资料
	// 	m_hasSendLoginTipmsg =true;
	// }

	if(m_isAdult ) 
		return ;		//成年人
	int nTimePass  = GetGlobalLogicEngine()->getMiniDateTime() - m_fcmStartTime;
	nTimePass =nTimePass/60; //先转换为分
	
	if( nTimePass >= (int)GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFcmHalfExpMoneyTime )
	{
		//告诉玩家收益减半
		if(nTimePass >= (int)GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFcmZeroExpMoneyTime)
		{
			//m_pEntity->SendOldTipmsgWithId(tpFcmZero,ttDialog);
			m_hasSend5HourTipmsg =true;
			
		}
		else //告诉玩家收益为0
		{
			//m_pEntity->SendOldTipmsgWithId(tpFcmHalfLogin,ttDialog);
			m_hasSend3HourTipmsg =true;
		} 
	} 
	 
	CActor* pActor = (CActor*)m_pEntity;
	//盛趣防沉迷 online 
	/*盛趣防沉迷屏蔽
	if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsSqKidsFcmOpen)
	{ 
		std::string strSendBuff = "";
		strSendBuff.resize(200);   
		CDataPacket pack((char *)strSendBuff.data(), 200); 
		pack << (byte)m_isAdult;
		pack << (byte)enMiscSystemID;
		pack << (byte)sFcmUseOnline;
		//pack << (int)GetLogicServer()->GetServerIndex(); 
		//pack.writeString(pActor->GetAccount().c_str());//账号名字
		pack.setPosition(0);
		CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength()); 
		static CNpc* gp = GetGlobalLogicEngine()->GetGlobalNpc();
		CScriptValueList paramList, retList;
		paramList << enMiscSystemID;
		paramList << sFcmUseOnline;
		paramList << pActor;
		paramList << &dp; 

		bool ret = gp->GetScript().Call("OnNetMsg", paramList, retList);
		if (!ret)
		{
			//脚本错误，将以模态对话框的形式呈现给客户端
			const RefString& s = gp->GetScript().getLastErrorDesc();
			if (pActor) pActor->SendTipmsg((const char*)s, ttDialog);
		}
	}
	*/
}

void CMiscSystem::OnActorLoginOut()
{
	CActor* pActor = (CActor*)m_pEntity;
	//盛趣防沉迷offline
	/*盛趣防沉迷屏蔽
	if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsSqKidsFcmOpen)
	{ 
		std::string strSendBuff = "";
		strSendBuff.resize(200);   
		CDataPacket pack((char *)strSendBuff.data(), 200); 
		pack << (byte)m_isAdult;
		pack << (byte)enMiscSystemID;
		pack << (byte)sFcmUseOffline; 
		pack.setPosition(0);
		CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength());  
		static CNpc* gp = GetGlobalLogicEngine()->GetGlobalNpc();
		CScriptValueList paramList, retList;
		paramList << enMiscSystemID;
		paramList << sFcmUseOffline;
		paramList << pActor;
		paramList << &dp; 

		bool ret = gp->GetScript().Call("OnNetMsg", paramList, retList);
		if (!ret)
		{
			//脚本错误，将以模态对话框的形式呈现给客户端
			const RefString& s = gp->GetScript().getLastErrorDesc();
			if (pActor) pActor->SendTipmsg((const char*)s, ttDialog);
		}
	} 
	*/
}
void CMiscSystem::Save(PACTORDBDATA  pActorData)
{
	if(bNeedSaveGameSetData)
		SaveGameSetDataToDb();
	
}

bool CMiscSystem::ReInitialFCM(BYTE FcmTag)
{
	//发来成年人
	if(FcmTag == 1) 
	{
		m_isAdult = true ;
		return true ;
	}

	//平台发来未成年
	if(FcmTag == 0 || FcmTag == 2)
	{	
		m_isAdult = false ;
		m_pEntity->SendTipmsgFormatWithId(tmFCMOpened,tstUI);
	}

	if(!GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen )
		return true;	//这个系统没开启
	if(m_isAdult ) 
		return true;		//成年人
	if( m_pEntity->m_nFcmTime >=0 ) //这个玩家开启了防沉迷
	{
		//认为他往前面推了一段时间登陆，因为防沉迷是基于账户的
		unsigned int nOnlineSecond =  (unsigned int)m_pEntity->m_nFcmTime ; //已经连续在线的时间
		if( nOnlineSecond >= 3600) //大于了一个小时
		{
			m_hasSend1HourTipmsg =true;
		}

		if( nOnlineSecond >= 7200) //大于了2个小时
		{
			m_hasSend2HourTipmsg =true;
		}
		//2小时55分的
		if(nOnlineSecond >= 10500)
		{
			m_hasSend2Hour55MinuteTipmsg =true;
		}
		/*盛趣防沉迷屏蔽
		if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsSqKidsFcmOpen)
		{
			m_fcmStartTime = GetGlobalLogicEngine()->getMiniDateTime();//盛趣直接用这个做开始时间，然后接口时间相减
		}
		else
		*/
		{ 
			//防沉迷开始的时间
			m_fcmStartTime = GetGlobalLogicEngine()->getMiniDateTime() - nOnlineSecond ;
		}
	}

	return true; 
}
bool CMiscSystem::Initialize(void *data,SIZE_T size)
{
	
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
	{
		OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
		return false;
	}

	m_isAdult = true ; //默认是成年人，实际判断等平台消息过来
	if(!GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen )
		return true;	//这个系统没开启
	if(m_isAdult ) 
		return true;		//成年人
	// if( m_pEntity->m_nFcmTime >=0 ) //这个玩家开启了防沉迷
	// {
	// 	//认为他往前面推了一段时间登陆，因为防沉迷是基于账户的
	// 	unsigned int nOnlineSecond =  (unsigned int)m_pEntity->m_nFcmTime ; //已经连续在线的时间
	// 	if( nOnlineSecond >= 3600) //大于了一个小时
	// 	{
	// 		m_hasSend1HourTipmsg =true;
	// 	}
	// 	if( nOnlineSecond >= 7200) //大于了2个小时
	// 	{
	// 		m_hasSend2HourTipmsg =true;
	// 	}
	// 	//2小时55分的
	// 	if(nOnlineSecond >= 10500)
	// 	{
	// 		m_hasSend2Hour55MinuteTipmsg =true;
	// 	}
	// 	//防沉迷开始的时间
	// 	m_fcmStartTime = GetGlobalLogicEngine()->getMiniDateTime() - nOnlineSecond ;
	// }

	return true;
}

void CMiscSystem::Destroy()
{
	m_RankPropData.empty();
}

int CMiscSystem::GetFcmExpMoneyRate()
{
	if(m_isAdult || !m_fcmIsOpen)
		return 100;
	if(m_hasSend5HourTipmsg)
	{
		//防沉迷5小时了，没有收入了
		if(m_pEntity && m_pEntity->IsInited())
		{
			m_pEntity->SendOldTipmsgWithId(tpFcmZeroIncome,ttTipmsgWindow); //给自己发消息
		}
		return 0;
	}
	else if(m_hasSend3HourTipmsg)
	{
		//防沉迷3小时了，收入只有以前的一半了
		if(m_pEntity && m_pEntity->IsInited())
		{
			m_pEntity->SendOldTipmsgWithId(tpFcmHalfIncome,ttTipmsgWindow); //给自己发消息
		}
		return 50;
	}
	else
	{
		return 100;
	}
}
void CMiscSystem::OnTimeCheck(TICKCOUNT  nTickCount)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	if(! GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen)  
		return;
	if( m_isAdult )
	{
		return;
	}
	unsigned int nTimePass  = GetGlobalLogicEngine()->getMiniDateTime() - m_fcmStartTime;
	nTimePass =nTimePass / 60; //取分钟
 
	//盛趣平台走这里
	/*盛趣防沉迷屏蔽
	if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsSqKidsFcmOpen)
	{  
		return;
	}
	*/
	//其他平台走这里
	if( nTimePass >= GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFcmZeroExpMoneyTime )
	{
		if(nTimePass % 15 ==0)
		{
			
			//m_pEntity->SendOldTipmsgWithId(tpFcmZero,ttDialog);
			m_pEntity->SendTipmsgFormatWithId(tmFcmZeroExp,tstUI);
		}
		m_hasSend5HourTipmsg =true;
		return;
	}

	if( nTimePass >= (int)GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFcmHalfExpMoneyTime)
	{
		m_hasSend3HourTipmsg =true;
		//播报
		if(nTimePass % 15 ==0)
		{
			//m_pEntity->SendOldTipmsgFormatWithId(tpFcmHalfInt,ttDialog,(int)(nTimePass /60));
			m_pEntity->SendTipmsgFormatWithId(tmFcmHalfExp,tstUI);
		}
		// else if(nTimePass % 30==0)
		// {
		// 	//您已累计在线3.5小时，您已进入疲劳游戏时间，游戏收益变成正常的50%，请您下线休息。
		// 	//m_pEntity->SendOldTipmsgFormatWithId(tpFcmHalfFloat,ttDialog,(int)(nTimePass /60));
		// 	m_pEntity->SendTipmsgFormatWithId(tmFcmHalfExp,tstUI);
		// }
		return;
	}

	if( nTimePass %15 ==0)
	{
		//m_pEntity->SendOldTipmsgFormatWithId(tpFcmNotify,ttDialog,(int)(nTimePass /60));
		m_pEntity->SendTipmsgFormatWithId(tmFcmPerHour,tstUI);
		return; 
	}
 
}

void CMiscSystem::OnViewActorInfo(CDataPacketReader &packet)	
{
	if (m_pEntity->HasMapAttribute(aaCannotViewOther))
	{
		(m_pEntity)->SendOldTipmsgWithId(tpChatDisableOperator);
		return;
	}

	char msg[80];
	packet.readString(msg,sizeof(msg)); //获取玩家的名字
	msg[sizeof(msg)-1] =0; //防止溢出
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(msg);
	//对方已经离线
	if(NULL ==pActor)
	{
		m_pEntity->SendOldTipmsgWithId( tpChatTargetOffLine);
		return;
	}
	if(pActor == m_pEntity) return; //查看自己，很无聊吗
	
	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data << (BYTE)GetSystemID() << (BYTE)sSendActorInfo ;
	data.writeString(msg);
	data << (BYTE)pActor->GetProperty<unsigned int>(PROP_ENTITY_ICON);
	data << (BYTE)pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	data << (BYTE)pActor->GetProperty<unsigned int>(PROP_ACTOR_VOCATION);
	data << (unsigned int)pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);
	//帮派名字
	CGuild * pGuild= pActor->GetGuildSystem()->GetGuildPtr();
	if(NULL ==pGuild )
	{
		data.writeString("");//帮派的名字
		data << (BYTE)0;//帮派的位置
	}
	else
	{
		data.writeString(pGuild->m_sGuildname);//帮派的名字
		data << (BYTE)pActor->GetGuildSystem()->GetGuildPos(); //帮派的位置
	}
	data.writeString("");//原师傅数据
	data << (BYTE)( pActor->GetProperty<unsigned int>(PROP_ACTOR_TEAM_ID) ?1:0);
	data << (BYTE)pActor->GetProperty<unsigned int>(PROP_ACTOR_SEX); //性别
	data << (BYTE ) pActor->GetCampId();
	data << (BYTE)0 ; //阵营里的职位
	data << (BYTE)0; //兄弟位
	
	ap.flush();
	pActor->PlayerViewNotice(m_pEntity);
}

void CMiscSystem::SendRankingData( LPCSTR sName )
{
	// if (!m_pEntity)
	// {
	// 	return;
	// }
	// if (sName == NULL || sName[0] == 0)
	// {
	// 	CActorPacket ap;
	// 	CDataPacket& pack = m_pEntity->AllocPacket(ap);
	// 	CVector<CRanking*>& List = GetGlobalLogicEngine()->GetRankingMgr().GetList();
	// 	ap << (BYTE)enMiscSystemID << (BYTE)sRankList;
	// 	INT_PTR pos = pack.getPosition();
	// 	ap << (WORD)0;
	// 	WORD result = 0;

	// 	for (INT_PTR i = 0; i < List.count(); i++)
	// 	{
	// 		CRanking* rank = List[i];
	// 		if (rank && rank->IsDisplay(m_pEntity))
	// 		{
	// 			pack.writeString(rank->GetName());	
	// 			pack.writeString(rank->GetDisplayName());
	// 			result++;
	// 		}
	// 	}
	// 	WORD* pCount = (WORD*)pack.getPositionPtr(pos);
	// 	*pCount = result;
	// 	ap.flush();	
	// }else
	// {
	// 	CRanking* rank = NULL;
	// 	CActorPacket ap;
	// 	CDataPacket& pack = m_pEntity->AllocPacket(ap);
	// 	pack << (BYTE)enMiscSystemID << (BYTE)sRankData;
	// 	pack.writeString(sName);
	// 	if ( (rank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(sName)) && rank->IsDisplay(m_pEntity))
	// 	{
	// 		CVector<CRankingItem*>& itemList = rank->GetList();
	// 		int nCount = __min(rank->GetMax(), (int)(itemList.count()));
	// 		WORD nColCount = rank->GetColCount();
	// 		pack << (int)nCount;
	// 		INT_PTR pos = pack.getPosition();
	// 		pack << (WORD)0;
	// 		WORD resultColCount = 0;
	// 		//发标题的名称
	// 		for (INT_PTR i = 0; i < nColCount; i++)
	// 		{
	// 			if (rank->isColDisplay(i))
	// 			resultColCount++;
	// 		}
	// 		WORD* pCount = (WORD*)pack.getPositionPtr(pos);
	// 		*pCount = resultColCount+2;
	// 		//发id和point的标题
	// 		pack.writeString(rank->GetIdTitle());
	// 		pack.writeString(rank->GetPointTitle());

	// 		for (INT_PTR i = 0; i < nColCount; i++)
	// 		{
	// 			if (rank->isColDisplay(i))
	// 			{
	// 				CRankingItem& title = rank->GetTitle();
	// 				pack.writeString(title.GetSubData(i));
	// 			}
	// 		}
	// 		for (INT_PTR i = 0; i < nCount; i++)
	// 		{
	// 			CRankingItem* item = itemList[i];
	// 			pack << (int)(i+1) << (int)item->GetId() << (int)item->GetPoint();//第一个是排名
	// 			for (INT_PTR j = 0; j < nColCount; j++)
	// 			{
	// 				if (rank->isColDisplay(j))
	// 				pack.writeString(item->GetSubData(j));
	// 			}
	// 		}
	// 	}
	// 	else
	// 	{
	// 		ap << (int)-1;
	// 	}
	// 	ap.flush();	
	// }
}

void CMiscSystem::SendCountdown( INT_PTR nTime, int  nFlag, int nType )
{
	if (!m_pEntity)
	{
		return;
	}
	CActorPacket ap;
	m_pEntity->AllocPacket(ap);
	ap << (BYTE)enMiscSystemID << (BYTE)sCountDown << (unsigned)nTime << (BYTE) nFlag << (nType);
	ap.flush();	
}

void CMiscSystem::SaveGameSetData(CDataPacketReader &packet)
{
	int nCount = 0;
	packet >> nCount;

	if(nCount <= 0) return;

	if(nCount > MAXGAMESETTING)
		nCount = MAXGAMESETTING;
	
	bNeedSaveGameSetData = true;

	for(int i=0;i<nCount;i++)
	{
		BYTE nIndex = 0;
		packet >> nIndex;
		int nNum = 0;
		packet >> nNum;

		if(nIndex > 0 && nIndex <= MAXGAMESETTING)
		{
			m_nGameSetData[nIndex-1] = nNum;
		}
	}
}

void CMiscSystem::LoadGameSetData()
{
	if (!m_pEntity) 
		return;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);

		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGameSetData);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << m_pEntity->GetRawServerIndex() << nServerId;
		DataPacket << nActorId;
 
		if (!((CActor*)m_pEntity)->OnGetIsTestSimulator())
		{   		
			GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket); 	
		}
	}
}


void CMiscSystem::LoadGameSetDataResult(CDataPacketReader &reader)
{
	if (!m_pEntity)
	{ 
		return;
	}
	
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator())
	{
        return;
	}
	int nCount = 0;
	reader >> nCount;
	
	if(nCount > 0)
	{
		for(int i=0;i<nCount;i++)
		{
			reader >> m_nGameSetData[i];
		}
	}

	SendGameSetRsult();
}

void CMiscSystem::SendGameSetRsult()
{
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enMiscSystemID << (BYTE)sSendGameSetData;

	int nCount = 0;
	size_t pos = DataPacket.getPosition();
	DataPacket << (int)0;//未必是真实的数量

	for(int i=0;i<MAXGAMESETTING;i++)
	{
		if(m_nGameSetData[i] > 0)
		{
			nCount++;
			DataPacket << (BYTE)(i+1);
			DataPacket << (int)m_nGameSetData[i];
		}
	}

	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nCount;

	AP.flush();
}

VOID CMiscSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	switch(nCmd)
	{
	case jxInterSrvComm::DbServerProto::dcLoadGameSetData:
		{
			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				LoadGameSetDataResult(reader);
			}
			else
			{
				SendGameSetRsult();
			}
			break;
		}

	case jxInterSrvComm::DbServerProto::dcChangeActorName:
		{
			OnChangeActorNameResult(nErrorCode,reader);
			break;
		}
	}
}

void CMiscSystem::SaveGameSetDataToDb()
{
	if(!m_pEntity)
		return;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveGameSetData);

		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)nActorId;

		for(int i=0;i<MAXGAMESETTING;i++)
		{
			DataPacket << (int)m_nGameSetData[i];
		}

		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}

	bNeedSaveGameSetData = false;

}


void CMiscSystem::InitRankPropData()
{
	if (m_pEntity == NULL) return;

	unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
	DataList<RANKPROP>& rankProps = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetRankProps();
	
	// CRankingMgr& rankMgr = GetGlobalLogicEngine()->GetRankingMgr();

	m_RankPropData.clear();
	for (INT_PTR i = 0; i < rankProps.count; i++)
	{
		RANKPROP *pProps = rankProps.pData + i;

		//艹 希望不要出现处理其他榜的情况

		//处理ranking的情况
		if (pProps == NULL || pProps->rankName[0] == '\0') continue;
		//CRanking *pRanking = rankMgr.GetRanking(pProps->rankName);

		CRanking *pRanking = pProps->pRanking;

		if (pRanking == NULL) continue;
		INT_PTR nRank =-1;
		CRankingItem *pItem = pRanking->GetPtrFromId(nActorId,nRank);
		if (pItem == NULL) continue;
		if (nRank >= 0 && nRank < pProps->props.count && pItem->GetPoint() >= pProps->nValue)	//如果在配置排名内且分数达到就添加
		{
			RANKITEM item;
			item.nRankIndex = (WORD)nRank;
			item.nConfigIndex = (WORD)i;
			m_RankPropData.add(item);
		}
	}
}

void CMiscSystem::UpdateRankPropData(int nConfigIndex, int nRankIndex, int nPoint)
{
	if (nConfigIndex < 0 || m_pEntity == NULL) return;
	
	if (nRankIndex >= 0)	//排行>0添加
	{
		bool bHas = false;
		for (INT_PTR i = 0; i < m_RankPropData.count(); i++)
		{
			RANKITEM &item = m_RankPropData[i];
			if (item.nConfigIndex == nConfigIndex)
			{
				item.nRankIndex = nRankIndex;
				item.nPoint = nPoint;
				bHas = true;
				break;
			}
		}

		if (!bHas)	//没有就添加
		{
			RANKITEM item;
			item.nConfigIndex = nConfigIndex;
			item.nRankIndex = nRankIndex;
			item.nPoint = nPoint;
			m_RankPropData.add(item);
		}
	} 
	else	//排行<0删除
	{
		for (INT_PTR i = m_RankPropData.count()-1; i >= 0 ; --i)
		{
			RANKITEM &item = m_RankPropData[i];
			if (item.nConfigIndex == nConfigIndex)	//如果存在 则删除
			{
				m_RankPropData.remove(i);
				break;
			}
		}
	}

	m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	//重新刷能力,玩家的属性发生了改变
}

bool CMiscSystem::CalcRankPropProperty(CAttrCalc &calc)
{
	DECLARE_TIME_PROF("CMiscSystem::CalcRankPropProperty");
	if (m_pEntity == NULL) return false;
	DataList<RANKPROP>& rankProps = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetRankProps();
	for (INT_PTR i = 0; i < m_RankPropData.count(); i++)
	{
		RANKITEM &item = m_RankPropData[i];
		if (item.nConfigIndex >= 0 && item.nConfigIndex < rankProps.count)
		{
			RANKPROP *pProps = rankProps.pData + item.nConfigIndex;
			if (pProps == NULL) continue;

			if (item.nRankIndex >= 0 && item.nRankIndex < pProps->props.count)
			{
				CStdItem::AttributeGroup *pProp = pProps->props.pData + item.nRankIndex;
				//写入属性
				if (pProp->nCount > 0)
				{
					for (INT_PTR i = pProp->nCount-1; i>-1; --i)
					{
						calc << pProp->pAttrs[i];
					}
				}
			}
			
		}
	}
	return true;
}

void CMiscSystem::OnChangeActorName(CDataPacketReader &packet)
{
	if(!m_pEntity) return;

	if(GetLogicServer()->GetchangeNameFlag() == false)
	{
		m_pEntity->SendTipmsgFormatWithId(tmChangeNameNotOpen,tstUI);
		return;
	}

	CUserItem::ItemSeries series ;
	packet >> series;

	char sName[32];
	packet.readString(sName,ArrayCount(sName));

	CUserBag& bag= m_pEntity->GetBagSystem();
	
	INT_PTR nBagIdx = bag.FindIndex(series);
	if ( nBagIdx < 0 )
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoChangeNameItem,tstUI);
		return ;
	}

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcChangeActorName);

		DataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)nActorId;
		DataPacket << (LONG64)series.llId;
		DataPacket.writeString(sName?sName:"");
		DataPacket.writeString(m_pEntity->GetEntityName()?m_pEntity->GetEntityName():"");
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

void CMiscSystem::OnChangeActorNameResult(INT_PTR nErrorCode,CDataPacketReader &reader)
{
	if(!m_pEntity) return;

	if(nErrorCode == 0)	//成功 
	{
		CUserItem::ItemSeries series ;
		reader >> series.llId;

		char sName[32];
		reader.readString(sName,ArrayCount(sName));
		sName[sizeof(sName)-1] = 0;

		CUserBag& bag= m_pEntity->GetBagSystem();

		INT_PTR nBagIdx = bag.FindIndex(series);
		if ( nBagIdx < 0 )
		{
			m_pEntity->SendTipmsgFormatWithId(tmNoChangeNameItem,tstUI);
			return ;
		}

		CUserItem *pUserItem = bag[nBagIdx];
		if(pUserItem ==NULL) return;

		bag.DeleteItem(pUserItem,1,"change name",GameLog::Log_ChangeuseName,true);

		m_pEntity->SendTipmsgFormatWithId(tmChangeNameSuc,tstUI);
		if(sName)
		{
			char oldName[64];
			sprintf(oldName,"%s",m_pEntity->GetEntityName());
			//记录日志
			if(GetLogicServer()->GetLocalClient())
			{
			}
			m_pEntity->SetEntityName(sName);
			m_pEntity->ResetShowName();
			AfterChangeNameOp(oldName);
		}

	}
	else
	{
		if(nErrorCode == 1)  //名称无效
		{
			m_pEntity->SendTipmsgFormatWithId(tmNeInvalidName,tstUI);
		}
		else if(nErrorCode == 4)
		{
			m_pEntity->SendTipmsgFormatWithId(tmNeNameinuse,tstUI);
		}
	}

	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data << (BYTE)enMiscSystemID << (BYTE)sSendChangeNameResult ;
	data << (BYTE)nErrorCode ;
	ap.flush();	
}

bool CMiscSystem::ChangeNameInRank(const char * sRankingName, int nRankSubIdx)
{
	// CRanking *fRanking = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(sRankingName);
	// if(fRanking != NULL)
	// {
	// 	CRankingItem* pItem = fRanking->GetPtrFromId(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
	// 	if(pItem)
	// 	{
	// 		pItem->SetSub(nRankSubIdx,m_pEntity->GetEntityName());
	// 		return true;
	// 	}
	// }
	return false;
}

void CMiscSystem::AfterChangeNameOp(const char* oldName)
{
	if(!m_pEntity) return;
	const char* newName = m_pEntity->GetEntityName();
	if( m_pEntity->GetGuildSystem() )
	{
		m_pEntity->GetGuildSystem()->OnChangeNameGuildOp();
	}
	//ChangeNameInRank 
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	typedef CRankingMgr::RankMap RankMap;
	typedef RankMap::iterator RankIterator;
	RankMap& rankingList = RankingMgr.GetList();
	RankIterator iter = rankingList.begin();
    for (; iter != rankingList.end(); iter++)
    {
		CRanking& rank = (*iter).second;
		if (rank.IsValid())
		{
			CRankingItem* pItem = rank.GetPtrFromId(m_pEntity->GetId());
			if (pItem && strcmp(pItem->GetSubData(0),oldName) == 0)
			{
				pItem->SetSub(0,newName);
			}
		}
    }

	//触发脚本
	CScriptValueList  paramList;
	CScriptValueList  retParamList;
	//((CActor*)this)->InitEventScriptVaueList( paramList,(int)aeChangeName );
	paramList << oldName << newName;
	((CActor*)this)->OnEvent( aeChangeName,paramList,retParamList );
}

void CMiscSystem::SendTempCombineTime()
{
	/*unsigned int beginTime = 0;
	unsigned int nCombineTime = GetGlobalLogicEngine()->GetMiscMgr().GetServerTempCombineTime(beginTime);

	if (m_pEntity == NULL || nCombineTime == 0 || beginTime > 0) return;
	CActorPacket datePack;
	m_pEntity->AllocPacket(datePack);
	datePack << (BYTE)enMiscSystemID << (BYTE)sSendTempCombineTime ;
	datePack << (unsigned int)nCombineTime;
	datePack << (BYTE)CMiscMgr::refresh_cs;
	datePack.flush();*/
}


void CMiscSystem::ClearCheckSignIn()
{
	// if (!m_pEntity)
	// {
	// 	return;
	// }
	// CActor *pActor = (CActor *)m_pEntity;
	// pActor->SetProperty<unsigned int>(PROP_ACTOR_SIGNIN, 0);

	// SYSTEMTIME sysTime;

	// CMiniDateTime nLastLogOutTime = pActor->GetLastLogoutTime();
	// CMiniDateTime nLoginTime = pActor->GetLoginTime();
	// CMiniDateTime nCurrentTime = GetGlobalLogicEngine()->getMiniDateTime();
	// nCurrentTime.decode(sysTime);
	// if ((nLastLogOutTime.tv > 0 && !GetGlobalLogicEngine()->IsSameMonth(nLastLogOutTime, nCurrentTime)) || (sysTime.wDay == 1 && !GetGlobalLogicEngine()->IsSameMonth(nLoginTime, nCurrentTime)))	//判断是否同一个月
	// {
	// 	pActor->SetProperty<unsigned int>(PROP_ACTOR_CHECKINS, 0);
	// }
}

void CMiscSystem::PlaySrcEffect(int nEffId, int nSec, bool boWorld, int nLevel)
{
	if (boWorld)
	{
		char data[128];
		CDataPacket pack(data,sizeof(data));
		pack << (BYTE)enMiscSystemID << (BYTE)sScrEffect << (WORD)nEffId << (int)nSec;
		GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(data,pack.getPosition(), nLevel);
	}
	else
	{
		CActorPacket ap;
		CDataPacket& data = m_pEntity->AllocPacket(ap);
		data << (BYTE)enMiscSystemID << (BYTE)sScrEffect << (WORD)nEffId <<  (int)nSec;
		ap.flush();
	}
}



void CMiscSystem::SendCustomEffect( byte nType, int nParam )
{
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (byte)GetSystemID() << (byte)sSendCustomEffect;
	netPack << nType << nParam;
	pack.flush();
}

void CMiscSystem::SendTurnOnOffIcon( byte nFlag, WORD nIconId )
{
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (byte)GetSystemID() << (byte)sSendTurnOnOffIconFlag;
	netPack << nFlag << nIconId;
	pack.flush();
}

int CMiscSystem::GetGameSetting(int nType, int nIdx)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return 0;
	}
	CActor* pActor = (CActor*)m_pEntity;
	if( nType > 0 &&  nType <= MAXGAMESETTING)
	{
		int nValue= m_nGameSetData[nType-1];
		if(nIdx >=0 && nIdx <=32)
		{
			return (nValue >> nIdx) & 0x1;
		}
	}
	return 0;
}


void CMiscSystem::SendActorInfoByNameFuzzy(unsigned char nType, char* sName)
{
	if( strstr("", sName) )
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpFriendSearchNull,ttFlyTip);
		return;
	}

	CVector<void *> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);

	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enMiscSystemID << (BYTE)sSendActorInfoByNameFuzzy;
	DataPacket << (unsigned char)nType;
	size_t pos = DataPacket.getPosition();
	DataPacket << (int)0;//未必是真实的数量
	int nResult = 0;

	for (INT_PTR i = 0; i < actorList.count(); i++)
	{
		CActor *pActor = (CActor *)actorList[i];
		if( pActor && strstr(pActor->GetEntityName(), sName) )
		{
			DataPacket << (unsigned int)(pActor->GetProperty<unsigned int>(PROP_ENTITY_ID));
			DataPacket.writeString(pActor->GetEntityName());
			DataPacket << (int)(pActor->GetProperty<int>(PROP_ACTOR_VOCATION));
			DataPacket << (int)(pActor->GetProperty<int>(PROP_CREATURE_LEVEL));
			DataPacket << (int)(pActor->GetProperty<int>(PROP_ACTOR_SEX));
			nResult++;
		}
	}

	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nResult;

	AP.flush();
}

void CMiscSystem::OnUserLogin()
{
	//发送开服天数
	SendOpenServerDays();
}
void CMiscSystem::SendOpenServerDays()
{
	if(m_pEntity == NULL) 
	{
		return;
	}
 
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return; 
    } 
	unsigned int nActorId = m_pEntity->GetId();
	int nDays = GetLogicServer()->GetDaysSinceOpenServer();
	int nServerId = GetLogicServer()->GetServerIndex();

	//发送开服天数
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enMiscSystemID << (BYTE)sSendOpenServerDays;

	DataPacket << (int)nServerId;
	DataPacket << (WORD)nDays;
	DataPacket << (unsigned int)GetLogicServer()->GetServerOpenTime();

	int nCombineDays = GetLogicServer()->GetDaysSinceCombineServer();
	DataPacket << (WORD)nCombineDays;

	int nCombineTime = GetLogicServer()->GetServerCombineTime();
	DataPacket << (int)nCombineTime;
	AP.flush(); 
}

void CMiscSystem::TriggerEvent(INT_PTR nEventID,INT_PTR nSubEvent,INT_PTR nParam1,INT_PTR nParam2,INT_PTR nParam3,INT_PTR nParam4)
{
	CScriptValueList paramList;
	//((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,nEventID);
	paramList << (int)nSubEvent;
	if(nParam1 >=0)
	{
		paramList << (int)nParam1;
		if(nParam2 >=0)
		{
			paramList << (int)nParam2;
			if(nParam3 >=0)
			{
				paramList << (int)nParam3;
				if(nParam4 >=0)
				{
					paramList << (int)nParam4;
				}
			}
		}	
	}

	((CActor*)m_pEntity)->OnEvent(nEventID,paramList,paramList);
}

void CMiscSystem::SendCircleCount()
{
	if (!m_pEntity)
	{
		return;
	}
	CActorPacket ap;
	CDataPacket& dataPack = m_pEntity->AllocPacket(ap);
	dataPack <<(BYTE)enMiscSystemID << (BYTE)sCircleCount;
	int nSize = GetGlobalLogicEngine()->GetGlobalVarMgr().m_CircleMap.size();

	dataPack<<(BYTE)nSize;
	std::map<int, int>::iterator it = GetGlobalLogicEngine()->GetGlobalVarMgr().m_CircleMap.begin();
	for(;it != GetGlobalLogicEngine()->GetGlobalVarMgr().m_CircleMap.end(); it++)
	{
		dataPack << (BYTE)(it->first);
		dataPack << (int)GetGlobalLogicEngine()->GetGlobalVarMgr().GetCountByCircleLevel(it->first);
	}
	int nOpenDay = GetLogicServer()->GetDaysSinceOpenServer();
	CIRCLECFG &cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().m_CircleLevelConfig;
	int nCost = 0;//cfg.GetCost(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE)+1, nOpenDay);
	dataPack << (int)nCost;
	ap.flush();
}



int CMiscSystem::GetRecoverGetGold(int nType, int nId)
{
	//非会员或者使用默认衰减的返回0 
	int colorCardLevel = GetMaxColorCardLevel()  ;
	if( !colorCardLevel ) return 0 ;
	MONTHCARDCONFIG *pConf =GetLogicServer()->GetDataProvider()->GetMiscConfig().GetMonthCardConfigbySuperLV(colorCardLevel);
	int nDampNum = 0 ;
	if(pConf && pConf->nSuperRightLV == colorCardLevel)
	{
		for(auto it :pConf->m_recoverGolds) {
			if(it.type == nType && it.id == nId)
				return it.count;
		}
	}
	return nDampNum ;
} 
void CMiscSystem::DealRecover()
{
	//非会员或者使用默认衰减的返回0 
	int colorCardLevel = GetMaxColorCardLevel()  ;
	unsigned int val = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_RECOVERSTATE);
	if( val == 0 && colorCardLevel >= 1 )
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_RECOVERSTATE, 1);
	}
}

void CMiscSystem::SendMsg2CrossServer(int nType)
{
	if(m_pEntity ==NULL)
	{
 		return;
	}
	
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = m_pEntity->GetId();
    OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "),nType, nActorID);
	packet << nServerId << nActorID;
	packet <<(BYTE)nType;
	//数据
	packet << m_pEntity->GetCrossActorId();
    packet << m_pEntity->GetAccountID(); 
	
	//记录数据长度
	int ccount = 0;
	INT_PTR nOffer = packet.getPosition(); 
	packet << ccount; 
	GameUserDataOther tmpData;
	tmpData.nGmLevel = (int)((CActor*)m_pEntity)->GetGmLevel();
	packet.writeBuf(&tmpData, sizeof(GameUserDataOther));
	ccount += sizeof(GameUserDataOther);
 
	INT_PTR nOffer2 = packet.getPosition();  
	packet.setPosition(nOffer);
	packet << ccount;  
	packet.setPosition(nOffer2); 

	pCrossClient->flushProtoPacket(packet);
} 
 
//接受数据
void CMiscSystem::OnCrossInitData(GameUserDataOther &data)
{
    if(!m_pEntity) 
	{ 
		return;
	} 

    ((CActor*)m_pEntity)->SetGmLevel(data.nGmLevel); 

    //m_isInitData = true;
   	//m_isDataModify = true; 
    ((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_GLOBAL_DATA); //完成一个步骤
}
