#include "StdAfx.h"
#include "ChatSystem.h"

#define NEXTSUBMITTIME 30					//提交建议的时间间隔限制
#define CHAT_BUFF_SIZE	256					//聊天读取buff大小
bool CChatSystem::CheckChannelCD(tagChannelID cid)
{	
	if ((cid < 0) || (cid >= ciChannelMax)) return false;
	TICKCOUNT nCurrentTime = GetLogicCurrTickCount();	
	if (nCurrentTime >= m_nNextHitTime[cid])
	{	
		int nChatCD = 0;
		CLogicDataProvider *pDataProvider = GetLogicServer()->GetDataProvider();
		/*
		if (cid == ciChannelZhenying)
		{
			int nCampTitle = m_pEntity->GetProperty<int>(PROP_ACTOR_CAMP_TITLE);
			if (nCampTitle != INVALID_CAMP_TITLE_ID)
			{
				const CampTitleNode* pNode = pDataProvider->GetCampConfig().m_CampTitleTree.GetTitleNode(nCampTitle);
				if (pNode)
				{
					nChatCD = pNode->m_self.m_nChatCD;
					m_nNextHitTime[cid] = nCurrentTime + nChatCD;
					return true;
				}
			}
		}
		*/
		//GLOBALCONFIG &gc = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
		CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
		int nChannelCD = gc.nChatChannelCD[cid];
		m_nNextHitTime[cid] = nCurrentTime + nChannelCD;		
		return true;
	}

	return false;
}

void CChatSystem::OnEnterGame()
{
	if(m_pEntity) {
		((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_BANCHAT, m_nFreePostTime);
		GetGlobalLogicEngine()->GetChatMgr().sendHistoryChatMsg(((CActor*)m_pEntity));
	}

}

bool CChatSystem::CheckLimit(tagChannelID cid)
{
	if ((cid < 0) || (cid >= ciChannelMax)) return false;
	//GLOBALCONFIG &gc = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
	if(((CActor*)m_pEntity)->GetRechargeNum() > 0 && gc.nRechargeopen[cid] >= 1)
		return true;
	if( !( ((CActor*)m_pEntity)->CheckLevel((int)gc.ChatLimit[cid].wLevel, 0) && ((CActor*)m_pEntity)->CheckRecharge(gc.ChatLimit[cid].nRechargeAmount) ) )
	{
		m_pEntity->SendTipmsgFormatWithId(tmchatLevelLimit, tstFigthing, gc.ChatLimit[cid].wLevel, gc.ChatLimit[cid].nRechargeAmount);
		return false;
	}
	return true;
}

void  CChatSystem::SetGambleFlag(bool isClose)
{
	//开启赌博
	CDataPacket& dataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSetGambleFlag);
	dataPacket <<(unsigned int ) (isClose ?1:0); //玩家ID
	GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
}

bool CChatSystem::IsDigit(char * str)
{
	if(str ==NULL) return false;
	for (INT_PTR i=0; i < (INT_PTR)strlen(str); i++)
	{
		if( i==0 &&  str[i] == '-') continue;
		if( str[i] < '0' || str[i] > '9'  )
		{
			return false;
		}
	}
	return true;
}


//网络数据处理
void  CChatSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &inPack )
{
	bool result = false;
	do
	{
		if (m_pEntity ==NULL) break;
		
		unsigned int nSex = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SEX); //获取玩家的性别
		unsigned int nCircle = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE); //获取玩家的转数

		if (nCmd == cSendReqCsChat)
		{ 
			OnCustomReqCsChat(inPack);
			// BYTE nChannleID = 0;
			// int nSvID = 0;
			// unsigned int nMsgTime = 0;
			// inPack >> nChannleID;
			// inPack >> nSvID;
			// inPack >> nMsgTime;

			// char strBuff[10240];
			// memset(strBuff, 0, sizeof(strBuff));
			// CActorPacket ap;
			// CDataPacket& outPack = m_pEntity->AllocPacket(ap);
			// //CDataPacket outPack(strBuff, sizeof(strBuff));
			// outPack << (BYTE) enChatSystemID << (BYTE)sSendReqCsChat <<(BYTE)ciChannelTipmsg; 
			// int ccount = 0;
            // INT_PTR nOffer = outPack.getPosition(); 
			// outPack << ccount; 
			// std::vector<CChatManager::CHATRECORD> & pCsRecord = GetGlobalLogicEngine()->GetChatMgr().GetChatCsRecord();
			// for (size_t i = 0; i < pCsRecord.size(); i++)
			// { 
			// 	if(nMsgTime != 0 && pCsRecord[i].msgTime <= nMsgTime)
			// 	{
			// 		continue;
			// 	}
			// 	outPack << (unsigned int) pCsRecord[i].msgTime; 
			// 	std::string sstr = pCsRecord[i].msg; 
			// 	outPack.writeString(sstr.c_str());
			// 	ccount++;
			// }
			// INT_PTR nOffer2 = outPack.getPosition();  
			// outPack.setPosition(nOffer);
			// outPack << ccount;  
			// outPack.setPosition(nOffer2); 
			// ap.flush();  
		}
		else if (nCmd == cSendClearMsgRefresh)
		{ 
			OnEnterGame(); 
		}
		else if (nCmd == cSendChat)
		{
			BYTE nChannleID =0;
			inPack >> nChannleID;
			char msg[CHAT_BUFF_SIZE];
			size_t nReadSize= inPack.readString( msg,sizeof(msg) );
			if (!CheckChatLen(msg))
			{
				m_pEntity->SendTipmsgFormatWithId(tmchaToLength, tstFigthing);
				break;
			}
			if (HaveCutChar(msg))
			{
				m_pEntity->SendTipmsgFormatWithId(tmchatillegal, tstFigthing);
				break;
			}
			if('@' == msg[0]) //GM命令来了
			{
				if (m_pEntity->GetGmLevel() >= 0)
				{
					ProcessGmCommand(msg);
				}
				break;	// 不是GM，直接忽略该聊天
			}
			if (!CheckLimit((tagChannelID)nChannleID))
			{			
				break;
			}

			if (IsForbit(nChannleID))
			{
				m_pEntity->SendTipmsgFormatWithId(tmThisChanalForbit, tstUI);
				break;
			}

			if (!CheckChannelCD((tagChannelID)nChannleID) && !m_bIsTalkFree )
			{
				m_pEntity->SendTipmsgFormatWithId(tmchatCDLimit, tstFigthing);
				break;
			}
			if (IsShutup())
			{
				m_pEntity->SendTipmsgFormatWithId(tmBanChat, tstChatSystem);
				break;
			}
			
			byte nParam = 0;
			std::string sendName = "";
			ACTORNAME name;
			inPack.readString(name,sizeof(name)); // 读取名字
			sendName = name;
			if(nChannleID == ciChannelSecret) //私聊
			{
				CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sendName.c_str());		//对方玩家
				if(pActor ==NULL)
				{
					m_pEntity->SendTipmsgFormatWithId(tmpChatTargetOffLine, tstFigthing); 
					break;
				}
				if(pActor && pActor->GetId() == m_pEntity->GetId())
				{
					m_pEntity->SendTipmsgFormatWithId(tmChatnotaddSelf, tstFigthing); 
					break;
				}
			}
			BYTE isItem = 0;
			CUserItem::ItemSeries itemGuid;
			inPack >>isItem;
			if(isItem == 1)
			{
				inPack >> itemGuid.llId;
				CUserItem* pUserIem= m_pEntity->GetBagSystem().FindItemByGuid(itemGuid);
				if(pUserIem) {
					GetGlobalLogicEngine()->GetChatMgr().addShowItem(pUserIem);
				}
			}
			
			//发送消息
			result = SendChat(nChannleID, msg, true, nParam, sendName, itemGuid.llId);
			//发送聊天信息到日志
			if (!IsShutup() && GetLogicServer()->GetLogClient())
			{
				LPCSTR sIp = ((CActor *)m_pEntity)->GetIp();
				char sBuff[500];
				sprintf_s(sBuff, ArrayCount(sBuff), "%s\b%s\b%s", sIp,m_pEntity->GetAccount(), msg);
				GetLogicServer()->GetLogClient()->SendChatLog(nChannleID, (char*)m_pEntity->GetEntityName(),m_pEntity->GetId(), sBuff);
			}
		}
		else if( nCmd == cGmBroadCast) //GM发公告
		{
			if(m_pEntity->GetGmLevel() <=0 ) break; //不是gm不能发
			int nMask =0;
			char buff[512];
			inPack.readString(buff,sizeof(buff)); //读取消息
			if (HaveCutChar(buff))
			{
				break;
			}
			inPack >> nMask;  //需要显示的位置
			GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(buff,nMask);
		}
		else if(nCmd == cSendHelp)	//求救
		{
			BroadcastSendHelp(inPack);
		}
		else if(nCmd == cGetChatShowItem)
		{
			CUserItem::ItemSeries itemGuid;
			inPack >> itemGuid.llId;
			CActorPacket ap;
			CDataPacket& data = m_pEntity->AllocPacket(ap);
			data << (BYTE)enChatSystemID << (BYTE)sGetChatShowItem;
			CUserItem userItem;
			GetGlobalLogicEngine()->GetChatMgr().GetShowItem(userItem, itemGuid.llId);
			if(userItem.series.llId > 0)
			{
				data << (BYTE)1;
				userItem >>data;
			}else 
			{
				data <<(BYTE)0;
			}
			ap.flush();
			return;
			
		}

	}while (false);

	CActorPacket pack;
	CDataPacket & data = m_pEntity->AllocPacket(pack);
	data << (BYTE) enChatSystemID << (BYTE) sChatResult;
	data<<(BYTE)result;
	pack.flush();
}

//求救
void CChatSystem::BroadcastSendHelp(CDataPacketReader &inPack)
{
}

bool CChatSystem::SendChat(int nChannleID, char *msg, bool boSave, byte nParam, std::string sendName, LONG64 itemId)
{
	if (m_pEntity ==NULL) return false;
	
	char buff[1024];
	CDataPacket outPack(buff, sizeof(buff)); //下发的数据包

	unsigned int nSex =m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SEX); //获取玩家的性别
	unsigned int nCircle = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE); //获取玩家的转数

	/// 禁言
	CheckAutoSilencing(msg);

	GetGlobalLogicEngine()->GetChatMgr().Filter(msg) ;
	outPack << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)nChannleID;

	outPack.writeString(m_pEntity->GetEntityName()); //把自己的名字写上去
	outPack.writeString(msg);

	outPack << m_pEntity->GetLevel();
	Uint64 actorid = Uint64(m_pEntity->GetHandle());
	outPack <<actorid;
	outPack<<(unsigned int)(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL));
	outPack << (BYTE)(itemId != 0? 1:0);
	if(itemId != 0)
	{
		outPack << itemId;
	}

	bool boCanSendToLog = true;
	if (IsShutup())
	{
		((CActor *)m_pEntity)->SendData(outPack.getMemoryPtr(), outPack.getPosition());
	}
	else
	{	
		switch(nChannleID)
		{
			case ciChannelSecret: //私聊
			{
				CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sendName.c_str());		//对方玩家
				if(pActor ==NULL)
				{
					m_pEntity->SendTipmsgFormatWithId(tmpChatTargetOffLine, tstFigthing); 
					return false;
				}


				if(pActor->GetFriendSystem().GetSocialFlag(m_pEntity->GetId(),SOCIAL_BLACK))
				{
					m_pEntity->SendTipmsgFormatWithId(tmpChatTargetOffLine, tstFigthing); 
					return false;
				}

				if (!pActor->OnGetIsSimulator() 
					&& !pActor->OnGetIsTestSimulator())
				{
					CActorPacket pack;
					CDataPacket & data = pActor->AllocPacket(pack);
					data.writeBuf(outPack.getMemoryPtr(),outPack.getPosition());
					pack.flush();
				}

				// ((CActor*)m_pEntity)->GetFriendSystem().AddLinkManLately(pActor);		//本人的“私聊”添加对方
				// pActor->GetFriendSystem().AddLinkManLately((CActor*)m_pEntity);			//对方的“私聊”添加本人
			}
			break;
			case ciChannelNear: //附近
			{
				m_pEntity->GetObserverSystem()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition(),true); //附近广播
				break;
			}
		
			case ciChannelGuild: //工会			
				m_pEntity->GetGuildSystem()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
				break;
			case ciChannelTeam: //队伍聊天
				m_pEntity->GetTeamSystem().BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
				break;
		
			case ciChannelWorld: //世界频道
			{
				// CUserItemContainer::ItemOPParam param;
				// param.wItemId = (WORD) (GetLogicServer()->GetDataProvider()->GetChatSystemConfig().nWorldChatItemItem); //世界喊话的物品的ID
				// param.btBindFlag = -1;
				// CUserBag& bag = m_pEntity->GetBagSystem();
				// param.wCount = 1;
				// if (param.wCount <= bag.GetItemCount(param.wItemId))
				// {
				// 	//ms.SendLackItem(param.wItemId, param.wCount);
				// 	if(bag.DeleteItem(param,"laba",GameLog::clWorldTalkItem/*, -1, m_pEntity*/) < 1)
				// 		return false;
				// }

				// int boratNum = m_pEntity->GetProperty<int>(PROP_ACTOR_BROATNUM);
				// if(boratNum ==0)
				// {
				// 	m_pEntity->SendTipmsgFormatWithId(tmBoratNotEnough, tstFigthing); 
				// 	return false;
				// }
				// m_pEntity->RemoveConsume(qatBroat, 0, 1, -1, -1,-1,0, GameLog::Log_WorldChat);
				//全服聊天限制
				int nVip = ((CActor*)m_pEntity)->GetMiscSystem().GetMaxColorCardLevel();
				CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
				if(nVip >= gc.nVipChats.size()) {
					return false;
				}
				int nMaxTimes = gc.nVipChats[nVip];
				int nUseTimes = ((CActor*)m_pEntity)->GetStaticCountSystem().GetStaticCount(gc.nCounter);
				if((nMaxTimes != -1) && (nUseTimes >= nMaxTimes)) {
					m_pEntity->SendTipmsgFormatWithId(tmChatVipLimit, tstUI); 
					return false;
				}
				((CActor*)m_pEntity)->GetStaticCountSystem().AddStaticCount(gc.nCounter,1);
				
				GetGlobalLogicEngine()->GetChatMgr().AddChatRecord(outPack.getMemoryPtr(), outPack.getPosition());
				GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
				
				if(nMaxTimes != -1 && nMaxTimes-nUseTimes-1 <= gc.nDisplayLeftTimesLimit ) {
					m_pEntity->SendTipmsgFormatWithId(tmChatVipLeftTimes, tstFigthing,nMaxTimes-nUseTimes-1);
				}

				// LPCTSTR sLogText = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpLogWorldChat);
				// if (nCount == 0 || m_pEntity->ChangeMoney(mtCoin,-1*nCount,GameLog::clChatMoney,0, sLogText,true)) //先扣除银两才能进行世界聊天
				// {
				// 	GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
				// }
			}
			break;
			default:
			return false;
		}
	}
	if (boCanSendToLog && boSave && GetLogicServer()->GetDataProvider()->GetChatSystemConfig().nChatLogFlag > 0)
	{
		SaveChatMsg(nChannleID, (char *)m_pEntity->GetEntityName(), msg);
	}

	// 贪玩聊天上报
	static char *pSPID = (char *)GetLogicServer()->GetVSPDefine().GetDefinition("SPID");
	char cSPID[CHAT_BUFF_SIZE] = {"27"};
	if ( strcmp(pSPID, cSPID) == 0 )
	{
		TanWanChatReport(m_pEntity->GetAccount(), m_pEntity->GetId(), m_pEntity->GetEntityName(), nChannleID, msg, sendName.c_str());
	}

	return true;
}

bool CChatSystem::SendChatMessageByChannel(int channel, char * outPack,SIZE_T nSize)
{
	
	switch(channel)
	{
	case ciChannelNear: //附近				
		m_pEntity->GetObserverSystem()->BroadCast(outPack, nSize,true); //附近广播
		break;
	case ciChannelGuild: //工会			
		m_pEntity->GetGuildSystem()->BroadCast(outPack, nSize);
		break;
	case ciChannelTeam: //队伍聊天
		m_pEntity->GetTeamSystem().BroadCast(outPack, nSize);
		break;
	case ciChannelWorld: //世界频道
		GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(outPack, nSize);
		break;

	default:
		return false;
	}
}

void CChatSystem::SetShutup(bool bForbid, INT_PTR nDuration)
{	
	if (bForbid)
	{
		m_nFreePostTime = (unsigned int)(GetGlobalLogicEngine()->getMiniDateTime() + nDuration);
	}
	else
	{
		m_nFreePostTime = GetGlobalLogicEngine()->getMiniDateTime();
		m_nChatHitCount = 0;
	}
	if(m_pEntity)
		((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_BANCHAT, m_nFreePostTime);
	m_bShutup = bForbid;
}


bool CChatSystem::IsShutup()
{
	unsigned int nCurrTm = GetGlobalLogicEngine()->getMiniDateTime();
	if (m_nFreePostTime <= nCurrTm)
	{
		m_nFreePostTime = nCurrTm;
		m_nChatHitCount = 0;
		return false;
	}
	return true;
}

unsigned int CChatSystem::GetFreePostTime()
{
	return m_nFreePostTime;
}
void CChatSystem::SetOffLineShutUp(unsigned int nActorId, bool bForbid, INT_PTR nDuration)
{
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	if(pActor)
	{
		pActor->GetChatSystem()->SetShutup(bForbid, nDuration);
	}
	else
	{
		if (bForbid)
		{
			unsigned int nTime = (unsigned int)(GetGlobalLogicEngine()->getMiniDateTime() + nDuration);
			SaveShutUpData(nActorId, nTime);
		}
		else
		{
		    SaveShutUpData(nActorId, 0);
		}		
	}
}
void CChatSystem::SaveShutUpData(unsigned int nActorId, unsigned int nFreePostTime)
{
	CCLVariant &var =  GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar();
	CCLVariant* pList = var.get("FreePostTimeList");
	if (!pList)
	{
		pList = &var.set("FreePostTimeList");
	}
	if (pList)
	{
		char sId[64];
		sprintf(sId,"%u",nActorId);
		CCLVariant* pActorVar = pList->get(sId);
		if (!pActorVar)
		{
			pActorVar = &pList->set(sId);
		}
		if (pActorVar)
		{
			pActorVar->set("FreePostTime",	(double)(nFreePostTime));
		}
	}
}
void CChatSystem::SaveToScriptData(PACTORDBDATA pData)
{
	pData->nShutUpTime = m_nFreePostTime;
	//SaveShutUpData(m_pEntity->GetId(), GetFreePostTime());
}

void CChatSystem::LoadFromScriptData()
{	
	CCLVariant &var =  GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar();
	CCLVariant* pList = var.get("FreePostTimeList");
	if (pList)
	{
		char sId[64];
		sprintf(sId,"%u",m_pEntity->GetId());
		CCLVariant* pActorVar = pList->get(sId);
		if (pActorVar)
		{
			CCLVariant* pVar = pActorVar->get("FreePostTime");
			if(pVar)
			{
				unsigned int nFreePostTime = (unsigned int)((double)(*pVar));
				unsigned int nCurrMiniTime = GetGlobalLogicEngine()->getMiniDateTime();
				if (nFreePostTime && nFreePostTime > nCurrMiniTime)
				{
					SetShutup(true, nFreePostTime - nCurrMiniTime);
				}
			}
		}
	}
}

char* CChatSystem::GetChannelName(int ChannelId)
{
	switch(ChannelId)
	{
	case ciChannelSecret:
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelSecret);
	case ciChannelNear:
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelNear);
	// case ciChannelHorn:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelHorn);
	case ciChannelGuild:
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelGuild);
	case ciChannelTeam:
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelTeam);
	// case ciChannelBigTeam:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelBigTeam);
	// case ciChannelMap:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelMap);
	// case ciChannelTipmsg:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelTipmsg);
	// case ciChannelZhenying:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelZhenying);
	// case ciChannelSystem:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelSystem);
	case ciChannelWorld:
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelWorld);
	// case ciChannelHelp:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelHelp);
	// case ciChannelFriend:
	// 	return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChannelZhenying);  //复用成好友
	default:
		break;
	}

	return NULL;
}

void CChatSystem::SaveChatMsg(int ChannelId,char * userName,char * strMsg)
{
	LPCTSTR channelName = GetChannelName(ChannelId);
	if(channelName == NULL) return;

	switch(ChannelId)
	{
	//case ciChannelSecret:
	//case ciChannelNear:
	case ciChannelGuild:
	//case ciChannelTeam:
	case ciChannelWorld:
	case ciChannelSystem:
	case ciChannelIntegrated:
		{
			//记录日志
			if(GetLogicServer()->GetLocalClient())
			{
			}
			break;
		}

	default:
		break;
	}
}

void CChatSystem::GetSortChatMsg(const char* msg, char* all, char* word)
{
	LPCSTR sptr;
	INT_PTR nLen = 0;
	unsigned char ch;
	char letter[CHAT_BUFF_SIZE] = {'\0'};
	char other[CHAT_BUFF_SIZE] = {'\0'};
	for ( sptr = msg; *sptr; sptr= sptr + nLen)
	{
		ch = (unsigned char)*sptr;
		if((ch & 0xF8) == 0xF8) //5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			nLen =5;	
		}
		else if( (ch & 0xF0) == 0xF0 )//4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			nLen =4;
		}
		else if( (ch & 0xE0) == 0xE0)//3个字节1110xxxx 10xxxxxx 10xxxxxx
		{
			nLen =3;
		}
		else if( (ch & 0xC0) == 0xC0 )//2个字节110xxxxx 10xxxxxx
		{
			nLen =2;
		}
		else
		{
			nLen =1;
		}

		if (nLen >=2)
		{
			strncat(all,sptr,nLen);
			strncat(word,sptr,nLen);
		}
		else if  (nLen == 1)
		{
			if (ch > 'A' && ch <'Z' || ch >'a'&& ch < 'z')
			{
				strncat(letter,sptr,nLen);
			}
			else
			{
				strncat(other,sptr,nLen);
			}
		}
	}
	strcat(all,letter);
	strcat(all,other);
}
// 检测禁言
void CChatSystem::CheckAutoSilencing(const char* msg)
{
	//GLOBALCONFIG& gc(GetLogicServer()->GetDataProvider()->GetGlobalConfig());
	CHATSYSTEMCONFIG& gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
	if( ! m_pEntity->CheckLevel((int)gc.nAutoSilenceLevel+1, 0))
	{ // 包含nAutoSilenceLevel，所以+1
		char all[CHAT_BUFF_SIZE] = {'\0'};//排序下中文字母，提高屏蔽字识别率
		char word[CHAT_BUFF_SIZE] = {'\0'};//中文
		GetSortChatMsg(msg,all,word);
		if (! GetGlobalLogicEngine()->GetChatMgr().IsStrInvalid((char*)all, 2))
		{
			++m_nChatHitCount;
		}

		if (strcmp(m_lastChatMsg,word) == 0)//只记录中文
		{
			m_nSameChatMsgCount++;
		}
		else
		{
			m_nSameChatMsgCount = 0;
		}
		strcpy(m_lastChatMsg,word);
		if ( !IsShutup())
		{
			CChatManager &mgr= GetGlobalLogicEngine()->GetChatMgr();
			unsigned long nIp = inet_addr(m_pEntity->GetIp());
			int nShutupIpCount = mgr.GetChatShutUpIpCount(nIp);
			if (nShutupIpCount >= gc.nAutoSilenceSameIpShutup)//ip禁言
			{
				SetShutup(true, gc.nAutoSilencingSecond ); 
			}
			if (m_nChatHitCount > gc.nAutoSilencing)//forbitword禁言
			{
				SetShutup(true, gc.nAutoSilencingSecond ); 
				// mgr.AddChatShutUpIpCount(nIp);
			}
			if (m_nSameChatMsgCount >= gc.nSameChatSliencingCount)//发相同内容禁言
			{
				SetShutup(true, gc.nSameChatSliencingSecond );		
				// mgr.AddChatShutUpIpCount(nIp);
				m_nSameChatMsgCount =0;
			}
		}
	}
}
bool CChatSystem::CheckChatLen(char* msg)
{
	size_t nLen = strlen(msg);
	if (strncmp(msg,"[<#",3)==0)//展示装备功能
	{
		if (nLen > 300)
		{
			return false;
		}
	}

	//字符长度过长的话，多半是模拟发言了
	else if( nLen > 120)//客户端最多40字符（中文)
	{
		//m_pEntity->CloseActor(false);
		return false;
	}
	return true;
}
bool CChatSystem::HaveCutChar(char* InputStr)
{
	bool boResult = false;
	if( strstr(InputStr,"<url>") || strstr(InputStr, "font") || strstr(InputStr, "color") || strstr(InputStr, "<<") || strstr(InputStr, ">>") 
		||strstr(InputStr,"\r") || strstr(InputStr, "\n"))	//客户端传入的消息，存在非法内容
	{
		boResult = true;
	}
	return boResult;
}

void CChatSystem::OnCustomReqCsChat(CDataPacketReader& inPack)
{
    if(!m_pEntity) 
        return;

	BYTE nChannleID = 0;
	int nSvID = 0;
	unsigned int nMsgTime = 0;
	inPack >> nChannleID;
	inPack >> nSvID;
	inPack >> nMsgTime;
	if (nChannleID == ciChannelTipmsg)
	{
		char strBuff[10240];
		memset(strBuff, 0, sizeof(strBuff));
		CActorPacket ap;
		CDataPacket& outPack = m_pEntity->AllocPacket(ap);
		//CDataPacket outPack(strBuff, sizeof(strBuff));
		outPack << (BYTE) enChatSystemID << (BYTE)sSendReqCsChat <<(BYTE)ciChannelTipmsg; 
		int ccount = 0;
		INT_PTR nOffer = outPack.getPosition(); 
		outPack << ccount; 
		std::vector<CChatManager::CHATRECORD> & pCsRecord = GetGlobalLogicEngine()->GetChatMgr().GetChatCsRecord();
		for (size_t i = 0; i < pCsRecord.size(); i++)
		{ 
			if(nMsgTime != 0 && pCsRecord[i].msgTime <= nMsgTime)
			{
				continue;
			} 
			outPack << (byte) pCsRecord[i].msgType;  
			outPack << (unsigned int) pCsRecord[i].msgTime; 
			std::string sstr = pCsRecord[i].msg; 
			outPack.writeString(sstr.c_str());
			ccount++;
		}
		INT_PTR nOffer2 = outPack.getPosition();  
		outPack.setPosition(nOffer);
		outPack << ccount;  
		outPack.setPosition(nOffer2); 
		ap.flush(); 
	}
}

// void CChatSystem::OnSendCsChat(CChatManager::CHATRECORD& record)
// {
//     if(!m_pEntity) 
//         return;
    
// 	char strBuff[10240];
// 	memset(strBuff, 0, sizeof(strBuff));
// 	CActorPacket ap;
// 	CDataPacket& outPack = m_pEntity->AllocPacket(ap);
// 	//CDataPacket outPack(strBuff, sizeof(strBuff));
// 	outPack << (BYTE) enChatSystemID << (BYTE)sSendReqCsChat <<(BYTE)ciChannelTipmsg; 
// 	int ccount = 0;
// 	INT_PTR nOffer = outPack.getPosition(); 
// 	outPack << 1;   
// 	{  
// 		outPack << (byte) record.msgType;  
// 		outPack << (unsigned int) record.msgTime; 
// 		std::string sstr = record.msg; 
// 		outPack.writeString(sstr.c_str()); 
// 	}
// 	ap.flush();  
// }


void CChatSystem::TanWanChatReport(LPCTSTR szAccount, int nActorId, LPCTSTR szActorName, int nChannelId, LPCTSTR szMsg, LPCTSTR szSendToActorName)
{
	if ( nChannelId < 0 || nChannelId > ciChannelMax)
	{
		OutputMsg(rmError,"TanWanChatReport nChannelId 错误 频道Id=%d !", nChannelId);
		return;
	}

	int nTanWanChannleId = 0;
	switch(nChannelId)
	{
		case ciChannelSecret: //私聊
			{
				nTanWanChannleId = 1;
				break;
			}
		case ciChannelNear: //附近频道，同屏
			{
				nTanWanChannleId = 8;
				break;
			}
		case ciChannelGuild: //工会，帮派
			{
				nTanWanChannleId = 6;
				break;
			}
		case ciChannelTeam: //队伍频道,5人
			{
				nTanWanChannleId = 7;
				break;
			}
		case ciChannelWorld: //世界频道
			{
				nTanWanChannleId = 4;
				break;
			}
		default:
		{
			nTanWanChannleId = 0;
			break;
		}
	}

	if ( !nTanWanChannleId )
	{
		OutputMsg(rmError,"TanWanChatReport nTanWanChannleId 错误 频道Id=%d !", nTanWanChannleId);
		return;
	}
	
	int nServerId = GetLogicServer()->GetServerIndex();
	char* szServerName = (char *)GetLogicServer()->getServerName();

	static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if ( globalNpc )
	{
		CScriptValueList paramList;
		paramList << nServerId;
		paramList << szServerName;
		paramList << szAccount;
		paramList << nActorId;
		paramList << szActorName;
		paramList << nTanWanChannleId;
		paramList << szMsg;
		paramList << szSendToActorName;
		if ( !globalNpc->GetScript().CallModule("TanWanChatReport", "OnReqTanWanChatReport", paramList, paramList, 0) )
		{
			OutputMsg(rmError,"TanWanChatReport OnReqTanWanChatReport 错误 玩家=%s !", szActorName);
		}
	}
}