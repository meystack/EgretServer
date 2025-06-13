
#include "StdAfx.h"
#include "../base/Container.hpp"
CMailSystem::CMailSystem()
{
	m_mailList.clear();
	m_nMailIndex = 0;
	nInitOnce = false;
}

CMailSystem::~CMailSystem()
{

}

void CMailSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if (!m_pEntity)
	{
		return;
	}
	switch(nCmd)
	{
	case jxInterSrvComm::DbServerProto::dcLoadMail:
		{
			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				OnLoadMails(reader);
			}
		}
		break;
	case jxInterSrvComm::DbServerProto::dcSaveMail:
		break;
	}
}


void CMailSystem::LoadMailDataFromDb()
{
	if(!m_pEntity) return;

	unsigned int nActorId = m_pEntity->GetId();

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadMail);
		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << nActorId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}

}
/*
  将CUserItem结构转换成邮件附件结构
*/
void CMailSystem::SetUseItemToMailAttach(CUserItem& userItem, MAILATTACH& mailAttach)
{
	memcpy(&mailAttach.item,&userItem,sizeof(CUserItem));
	// if (mailAttach.nType > qatEquipment)
	// {
	// 	mailAttach.nResourceCount = MAKELONG(userItem.wDura,userItem.wDuraMax);
	// }
}

/*
将邮件附件结构转换成CUserItem结构,useritem主要是保存到附件数据库
*/
void CMailSystem::SetMailAttachToUseItem(CUserItem& userItem, MAILATTACH& mailAttach)
{
	memcpy(&userItem,&mailAttach.item,sizeof(userItem));
	// if (mailAttach.nType > qatEquipment)
	// {
	// 	userItem.wDura =  LOWORD(mailAttach.nResourceCount);
	// 	userItem.wDuraMax = HIWORD(mailAttach.nResourceCount);
	// }
}
void CMailSystem::OnLoadMails(CDataPacketReader & packet)
{
	// ((CActor*)m_pEntity)->OnFinishOneInitStep();
	//注意子系统读取前，脚本login会发邮件
	WORD nMailCount = 0;
	BYTE nPacketCount = 0;
	packet >> nPacketCount;

	packet >> nMailCount;
	
	BYTE nMailAttachCount = 0;
	static int nExpireSec = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMailExpireDays*3600*24;
	unsigned int nMinDt = time(NULL) - nExpireSec;//GetGlobalLogicEngine()->getMiniDateTime() - nExpireSec;
	for (WORD i = 0; i < nMailCount; i++)
	{
		uint64_t nMailId;
		MAILINFO oneMail;
		packet >> nMailId >> oneMail.nSrcId;

		oneMail.nMailId = nMailId; 
		//OutputMsg(rmTip,"113646565422224------------------------------------------------------------nMailId = %llu, int64 = %lld", nMailId, nMailId); 
		packet.readString(oneMail.sTitle, ArrayCount(oneMail.sTitle));
		packet.readString(oneMail.sContent, ArrayCount(oneMail.sContent));
		packet >> oneMail.nCreateDt >> oneMail.nState;
		packet >> nMailAttachCount;
		int nAttachCount = 0;
		for(int j=0; j < nMailAttachCount && j < MAILATTACHCOUNT; j++)
		{
			// CUserItem userItem;
			packet >> oneMail.mailAttach[j].nType;
			oneMail.mailAttach[j].item << packet;
			// SetUseItemToMailAttach(userItem,oneMail.mailAttach[j]);
			if(oneMail.mailAttach[j].item.wCount > 0)
				nAttachCount++;
		}
		oneMail.nAttachCount = nAttachCount;
		
		if (oneMail.nCreateDt > nMinDt && !isSameIdMail(oneMail))
		{
			m_mailList.push_back(oneMail);
		}		
	}
	bool isEnd = false;
	packet >>isEnd;
	if (isEnd)
	{
		// ((CActor*)m_pEntity)->OnFinishOneInitStep(8);
		// SendAllMail();
	}
	if(!nInitOnce)
	{
		((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_MAIL_SYSTEM);
		nInitOnce = true;
	}
}

void CMailSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	switch(nCmd)
	{
	case cMailSystemMailOpt:
		MailOpt(packet);
		break;
	case cMailSystemDeleteMails:
		DeleteMails(packet);
		break;
	case cMailSystemSomeMails: 
		DeleteSomeMails(packet);
		break;
	case cMailSystemGetMailItems:
		GetMailItems(packet);
		break;
	case cMailSystemDeleteMailAll:
		DeleteMailsAll();
		break;
	case cMailSystemGetMailItemsAll:
		GetMailItemsAll();
		break;
	}
}

void CMailSystem::SaveOfflineMail(unsigned int nActorId, MAILINFO & oneMail)
{
	int nServerId = GetLogicServer()->GetServerIndex();
	CDataPacket & dbPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveOfflineMail);
	dbPacket << nServerId << nServerId << nActorId;
	
	uint64_t nMailId = oneMail.nMailId; //邮件id 
	//OutputMsg(rmTip,"6646135312312------------------------------------------------------------nMailId = %llu, int64 = %lld", nMailId, nMailId); 
	dbPacket << nMailId << oneMail.nSrcId;
	dbPacket.writeString(oneMail.sTitle);
	dbPacket.writeString(oneMail.sContent);
	dbPacket << oneMail.nCreateDt << oneMail.nState <<oneMail.nIsDel;
	CUserItem userItem;
	memset(&userItem, 0, sizeof(CUserItem));
	BYTE mailAttachCount = 0;
	for (int index = 0; index < MAILATTACHCOUNT; index++)
	{
		MAILATTACH & mailAtt = oneMail.mailAttach[index];
		if (mailAtt.nType == qatEquipment && mailAtt.item.wItemId == 0 )
		{
			break;
		}
		mailAttachCount++;
	}
	dbPacket << mailAttachCount;
	for (int index = 0; index < mailAttachCount; index++)
	{
		MAILATTACH & mailAtt = oneMail.mailAttach[index];
		dbPacket << mailAtt.nType;
		SetMailAttachToUseItem(userItem,oneMail.mailAttach[index]);
		userItem >> dbPacket;
	}
	
	GetLogicServer()->GetDbClient()->flushProtoPacket(dbPacket); 
}


void CMailSystem::SaveMailToDb(std::vector<MAILINFO>& mailList, BYTE nDelType)
{
	int nServerId = GetLogicServer()->GetServerIndex();
	unsigned int nMyId = m_pEntity->GetId();
	INT_PTR nMailCount	 = mailList.size();
	INT_PTR packetCount = (nMailCount-1)/MAIL_DATA_SAVE_COUNT_EACH_TIME+1;
	INT_PTR nNeedSendMailCount =0;
	for (INT_PTR curPacketCount =1; curPacketCount <= packetCount; curPacketCount++)
	{
		if (curPacketCount* MAIL_DATA_SAVE_COUNT_EACH_TIME > nMailCount)
		{
			nNeedSendMailCount =  nMailCount - (curPacketCount-1)*MAIL_DATA_SAVE_COUNT_EACH_TIME;
		}
		else
		{
			nNeedSendMailCount = MAIL_DATA_SAVE_COUNT_EACH_TIME;
		}
		CDataPacket & dbNetPack = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveMail);
		dbNetPack << ((CActor*)m_pEntity)->GetRawServerIndex() << nServerId << nMyId;
		dbNetPack << (BYTE)curPacketCount << (BYTE)eMailSaveType_Mail <<(BYTE)nDelType;
		dbNetPack << (BYTE)nNeedSendMailCount;//这个包里有多少邮件
		for (INT_PTR i = (curPacketCount-1)* MAIL_DATA_SAVE_COUNT_EACH_TIME; i < curPacketCount* MAIL_DATA_SAVE_COUNT_EACH_TIME && i < nMailCount; i++)
		{
			MAILINFO & oneMail = mailList[i]; 
			uint64_t nMailId = oneMail.nMailId; //邮件id 
			//OutputMsg(rmTip,"68511463113-------------------------------------------------------------nMailId = %llu, int64 = %lld", nMailId, nMailId); 
			dbNetPack << nMailId << oneMail.nSrcId;
			dbNetPack.writeString(oneMail.sTitle);
			dbNetPack.writeString(oneMail.sContent);
			dbNetPack << oneMail.nCreateDt << oneMail.nState << oneMail.nIsDel;
		}
		GetLogicServer()->GetDbClient()->flushProtoPacket(dbNetPack);

		//先发邮件，再发附件
		CDataPacket & attachPack = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveMail);
		attachPack << ((CActor*)m_pEntity)->GetRawServerIndex() << nServerId << nMyId;
		attachPack << (BYTE)curPacketCount << (BYTE)eMailSaveType_Attach <<(BYTE)nDelType;
		size_t nNeedSendAttachCountPos = attachPack.getPosition();
		WORD nNeedSendAttachCount = 0;
		attachPack << (WORD)nNeedSendAttachCount;//这个包里有多少附件
		for (INT_PTR i = (curPacketCount-1)* MAIL_DATA_SAVE_COUNT_EACH_TIME; i < curPacketCount* MAIL_DATA_SAVE_COUNT_EACH_TIME && i < nMailCount; i++)
		{
			MAILINFO & oneMail = mailList[i];
			for (INT_PTR index = 0; index < oneMail.nAttachCount; index++)
			{
				MAILATTACH & mailAtt = oneMail.mailAttach[index];
				// CUserItem userItem;
				uint64_t nMailId = oneMail.nMailId; //邮件id
				
				//OutputMsg(rmTip,"657353553-------------------------------------------------------------nMailId = %llu, int64 = %lld", nMailId, nMailId);

				attachPack << nMailId;
				attachPack << mailAtt.nType;
				//SetMailAttachToUseItem(userItem, mailAtt);
				mailAtt.item >> attachPack;		
			}
			nNeedSendAttachCount += oneMail.nAttachCount;
		}
		if(nNeedSendAttachCount != 0)
		{
			WORD* pCount = (WORD*)attachPack.getPositionPtr(nNeedSendAttachCountPos);
			*pCount = nNeedSendAttachCount;
		}
		GetLogicServer()->GetDbClient()->flushProtoPacket(attachPack);
	}	
}
void CMailSystem::Save(PACTORDBDATA pData)
{
	SaveMailToDb(m_mailList,eMailSaveDelType_Normal);
	SaveMailToDb(m_mailDelList,eMailSaveDelType_Del);
	m_mailDelList.clear();//已删除的邮件保存之后就清空
}

bool CMailSystem::Initialize(void *data,SIZE_T size)
{


	return TRUE;
}

void CMailSystem::MailOpt( CDataPacketReader & packet )
{
	byte nOptType = 0;
	INT64 nMailId = 0;
	packet >> nOptType >> nMailId;
	switch(nOptType)
	{
	case moReadFlag:
		SetMailReaded(nMailId);
		break;
	}
}

void CMailSystem::SetMailReaded(INT64 nMailId)
{
	MAILINFO * mail = GetMail(nMailId);
	if (mail != NULL)
	{
		if(mail->nState == eMailState_No)
		{
			mail->nState = eMailState_Read;
		}
	}
}
void CMailSystem::GetMailItemsAll()
{
	CVector<MAILINFO *> pMailList;
	pMailList.clear();
	bool bBag = false;
	INT_PTR nCount = m_mailList.size();
	if(nCount <= 0)
		return;
	CUserBag & bag = ((CActor *)m_pEntity)->GetBagSystem();
	int nMailMaxCount = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMailMaxCount;
	if (nCount > nMailMaxCount)
	{
		nCount = nMailMaxCount;//只处理150封
	}
	int x,y;
	m_pEntity->GetPosition(x,y);
	CScene * pScene = m_pEntity->GetScene();
	if (pScene == NULL)
	{
		return;
	}
	// if(!pScene->HasMapAttribute(x,y,aaSaft))
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmNoSafeAreaGetMail, tstUI);
	// 	return;
	// }
	do
	{
		for (WORD i = 0; i < nCount; i++)
		{
			int nNeedGrids = 0;
			bool bCanGet = false;
			MAILINFO * oneMail = &m_mailList[i];
			if (oneMail != NULL)
			{
				if(oneMail->nState == eMailState_Get)
					continue;

				if(oneMail->nAttachCount == 0)
					continue;

				for (int itIdx = 0; itIdx < MAILATTACHCOUNT; itIdx++)
				{
					MAILATTACH * pMailAtt = &oneMail->mailAttach[itIdx];
					if (pMailAtt->nType == qatEquipment && pMailAtt->item.wItemId > 0 && pMailAtt->item.wCount > 0)
					{
						const CStdItem * pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pMailAtt->item.wItemId);
						if (pItem)
						{
							CUserItemContainer::ItemOPParam itemPara;
							AddAttachToItemParam(*pMailAtt,itemPara);
							nNeedGrids += (int)bag.GetAddItemNeedGridCount(itemPara);
						}
					}
					bCanGet = true;
				}
				if(nNeedGrids < 6)
					nNeedGrids = 6;
				
				if (!bag.bagIsEnough(BagMail))
				{
					bBag = true;
					m_pEntity->SendTipmsgFormatWithId(tmGiftNoBagNum, tstUI);
					break;
				}
				else
				{
					if (oneMail->nState == eMailState_No)
					{
						oneMail->nState = eMailState_Read;
					}
					if(bCanGet)
					{
						pMailList.add(oneMail);
						LogGetMailAttach(*oneMail);
						GetOneMailAttach(*oneMail);
					}
				}
			}
		}
	}while(false);
	
	INT_PTR nItMailListCount = pMailList.count();	// 邮件数量
	if ( nItMailListCount > 0)
	{
		CActorPacket pack;
		CDataPacket & netPack = m_pEntity->AllocPacket(pack);
		netPack << (byte)GetSystemID() << (byte)sMailSystemSendMailItems;
		netPack << (WORD)nItMailListCount;
		for (INT_PTR i = 0; i < nItMailListCount; i++)
		{
			netPack << (INT64)(pMailList[i]->nMailId);
		}
		pack.flush();
		m_pEntity->SendTipmsgFormatWithId(tmMailGetItemSuccess, tstUI);
		return;
	}	
	if(!bBag)
		m_pEntity->SendTipmsgFormatWithId(tmMailNOItem, tstUI);
}
void CMailSystem::GetOneMailAttach(MAILINFO& oneMail)
{
	CUserBag & bag = ((CActor *)m_pEntity)->GetBagSystem();
	for (int itIdx = 0; itIdx < MAILATTACHCOUNT; itIdx++)
	{
		MAILATTACH& pMailAtt = oneMail.mailAttach[itIdx];
		bool bSuccess = false;
		if (pMailAtt.nType == qatEquipment && pMailAtt.item.wItemId == 0)
		{
			continue;
		}
		if (pMailAtt.nType == qatEquipment && pMailAtt.item.series > 0)//附件是整个UserItem
		{
			CUserItem* pUserItem = GetGlobalLogicEngine()->CopyAllocUserItem(false,&(pMailAtt.item));
			if(bag.AddItem(pUserItem,m_pEntity->GetEntityName(),GameLog::Log_Mail))
			{
				bSuccess = true;
			}	
		}
		else if (pMailAtt.nType == qatEquipment && pMailAtt.item.wItemId > 0 && pMailAtt.item.wCount > 0)//附件是奖励一个物品
		{
			CUserItemContainer::ItemOPParam iParam;
			AddAttachToItemParam(pMailAtt, iParam);
			if(bag.AddItem(iParam, m_pEntity->GetEntityName(), GameLog::Log_Mail))
			{
				bSuccess = true;
			}					
		}
		else if (pMailAtt.nType == qatYuanbao && oneMail.nSrcId == eMailSource_BackStageWelFare)//后台发元宝,当充值处理
		{
			m_pEntity->ChangeYuanbao(pMailAtt.item.wCount, GameLog::Log_Mail, 0, "backstageMail", true);
			bSuccess = true;
		}
		else if (pMailAtt.nType == qatAddExp)//经验库
		{
			if(((CActor *)m_pEntity)->GiveAward(pMailAtt.nType,pMailAtt.item.wItemId,pMailAtt.item.wCount,pMailAtt.item.btQuality,pMailAtt.item.wStar,pMailAtt.item.btFlag,0,GameLog::Log_Mail,oneMail.sTitle))
			{	
				bSuccess = true;
			}
		}
		else if(pMailAtt.nType > qatEquipment)//附件是奖励资源
		{
			if(((CActor *)m_pEntity)->GiveAward(pMailAtt.nType,pMailAtt.item.wItemId,pMailAtt.item.wCount,pMailAtt.item.btQuality,pMailAtt.item.wStar,pMailAtt.item.btFlag,0,GameLog::Log_Mail,oneMail.sTitle))
			{	// 已经领取，删掉该附件
				bSuccess = true;
			}
		}
		// if (bSuccess)
		// {
		// 	memset(&pMailAtt, 0, sizeof(pMailAtt));
		// 	if (oneMail.nAttachCount > 0)
		// 	{
		// 		oneMail.nAttachCount --;
		// 	}
		// 	if(oneMail.nAttachCount == 0)
		// 		oneMail.nState = eMailState_Get;
		// }
	}
	oneMail.nState = eMailState_Get;
}
void CMailSystem::GetMailItems(CDataPacketReader & packet)
{
	INT64 nMailSeries;
	packet >> nMailSeries;
	INT_PTR nCount = m_mailList.size();
	if(nCount <= 0)
		return;
		
	CUserBag & bag = ((CActor *)m_pEntity)->GetBagSystem();

	MAILINFO * oneMail = GetMail(nMailSeries);
	if(!oneMail)
	{
		m_pEntity->SendTipmsgFormatWithId(tmMailError, tstUI);
		return;
	}


	int x,y;
	m_pEntity->GetPosition(x,y);
	CScene * pScene = m_pEntity->GetScene();
	if (pScene == NULL)
	{
		return;
	}
	// if(!pScene->HasMapAttribute(x,y,aaSaft))
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmNoSafeAreaGetMail, tstUI);
	// 	return;
	// }

	if(oneMail->nState == eMailState_Get)
	{
		m_pEntity->SendTipmsgFormatWithId(tmMailItemGetReady, tstUI);
		return;
	}

	if(oneMail->nAttachCount == 0)
	{
		m_pEntity->SendTipmsgFormatWithId(tmMailNOItem, tstUI);
		return;
	}
	int nNeedGrids = 0;
	bool boMaxForce = false;
	for (int itIdx = 0; itIdx < MAILATTACHCOUNT; itIdx++)
	{
		MAILATTACH * pMailAtt = &oneMail->mailAttach[itIdx];
		if (pMailAtt->nType == qatEquipment && pMailAtt->item.wItemId > 0)
		{
			const CStdItem * pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pMailAtt->item.wItemId);
			if (pItem)
			{
				CUserItemContainer::ItemOPParam itemPara;
				AddAttachToItemParam(*pMailAtt,itemPara);
				nNeedGrids += (int)bag.GetAddItemNeedGridCount(itemPara);
			}
		}
	}
	if(nNeedGrids < 6)
		nNeedGrids = 6;

	if(boMaxForce)
	{
		m_pEntity->SendTipmsgFormatWithId(tmMailForceIsFull, tstUI);
		return;
	}else if (!bag.bagIsEnough(BagMail))
	{
		m_pEntity->SendTipmsgFormatWithId(tmGiftNoBagNum, tstUI);
		return;
	}
	if(oneMail)
	{
		LPCSTR lpSender = m_pEntity->GetEntityName();

		if (oneMail->nState == eMailState_No)
		{
			oneMail->nState = eMailState_Read;
		}
		LogGetMailAttach(*oneMail);	
		GetOneMailAttach(*oneMail);
		CActorPacket pack;
		CDataPacket & netPack = m_pEntity->AllocPacket(pack);
		netPack << (byte)GetSystemID() << (byte)sMailSystemSendMailItems;
		netPack << (WORD)1;
		netPack << (INT64)(oneMail->nMailId);
		pack.flush();
		m_pEntity->SendTipmsgFormatWithId(tmMailGetItemSuccess, tstUI);
	}
} 
void CMailSystem::SendAllMail()
{
	if (m_pEntity == NULL)
	{
		return;
	}
	 
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return; 
    }
	int nMailMaxCount = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMailMaxCount;
	INT_PTR nCount = m_mailList.size();
	if (nCount > nMailMaxCount)
	{
		nCount = nMailMaxCount;
	}
	INT_PTR nPacketCount = (nCount-1)/MAIL_DATA_SEND_COUNT_EACH_TIME +1;
	for (INT_PTR i =1; i <=nPacketCount; i++)
	{
		CActorPacket pack;
		CDataPacket & netPack = m_pEntity->AllocPacket(pack);
		netPack << (byte)GetSystemID() << (byte)sMailSystemSendAllMail;
		INT_PTR nSendCount = 0;
		if (i* MAIL_DATA_SEND_COUNT_EACH_TIME > nCount)
		{
			nSendCount =  nCount - (i-1)*MAIL_DATA_SEND_COUNT_EACH_TIME;
		}
		else
		{
			nSendCount = MAIL_DATA_SEND_COUNT_EACH_TIME;
		}
		if (i == 1)
		{
			netPack << (BYTE)eMailBatchSendType_DelOld << (BYTE)nSendCount;
		}
		else
		{
			netPack << (BYTE)eMailBatchSendType_Add<< (BYTE)nSendCount;
		}
		for (INT_PTR index = (i-1)* MAIL_DATA_SEND_COUNT_EACH_TIME; index < i* MAIL_DATA_SEND_COUNT_EACH_TIME && index < nCount; index++)
		{
			MAILINFO & info = m_mailList[index];
			AddOneMailToPacket(netPack, info);
			info.isSendToClient = true;
		}
		pack.flush();
	}  
}
void CMailSystem::AddMailAttach(MAILATTACH& attach, int nType, int nId, int nQuality, int nStrong, bool bind, int smith[CUserItem::MaxSmithAttrCount])
{
	attach.nType = nType;
	attach.item.wItemId = nId;
	attach.item.btQuality = nQuality;
	attach.item.wStar = nStrong;
	attach.item.btFlag = bind;
	memcpy(attach.item.smithAttrs, smith, sizeof(smith));
}

/*
将附件格式转换为Item操作格式
*/
void CMailSystem::AddAttachToItemParam(MAILATTACH& attach, CUserItemContainer::ItemOPParam& itemPara)
{
	itemPara.wItemId			= (WORD)attach.item.wItemId;
	itemPara.wCount				= (WORD)attach.item.wCount;
	itemPara.btBindFlag			= attach.item.btFlag;
	itemPara.btQuality			= attach.item.btQuality;
	//itemPara.btStrong			= attach.btStrong;
	itemPara.wStar				= attach.item.wStar;					//强化星级
	itemPara.bLostStar			= attach.item.bLostStar;				//强化损失星级
	itemPara.bInSourceType		= attach.item.bInSourceType;		//铭刻等级
	itemPara.nAreaId		= attach.item.nAreaId;			
	itemPara.btLuck				= attach.item.btLuck;				//幸运
	itemPara.wIdentifyslotnum   = attach.item.wIdentifySlotNum;		//鉴定槽数量
	// itemPara.nDropMonsterId       = attach.item.nDropMonsterId;			//怪物
	memcpy(itemPara.nSmith, attach.item.smithAttrs,sizeof(attach.item.smithAttrs));
}
void CMailSystem::AddOneMailToPacket(CDataPacket& pack, MAILINFO& oneMail)
{
	uint64_t nMailId = oneMail.nMailId;
	//OutputMsg(rmTip, "5687466986844------------------------------------------------------------nMailId = %llu, int64 = %lld", nMailId, nMailId); 
	pack << nMailId;
	pack << oneMail.nSrcId;
	pack.writeString(oneMail.sTitle);
	pack.writeString(oneMail.sContent);
	pack << oneMail.nCreateDt;
	pack << oneMail.nState;
	BYTE nMailAttachCount = 0;
	// for (int i = 0; i < MAILATTACHCOUNT; i++)
	// {
	// 	if (oneMail.mailAttach[i].item.wCount == 0)
	// 	{
	// 		continue;
	// 	}
	// 	nMailAttachCount++;
	// }
	int pos = pack.getPosition();
	pack << nMailAttachCount;
	if(oneMail.nState !=  eMailState_Get)
	{
		for (int i =0; i < MAILATTACHCOUNT; i++)
		{
			MAILATTACH& attach = oneMail.mailAttach[i];

			if(attach.item.wCount == 0)
			{
				continue;
			}
			pack << attach.nType;

			(CUserItem)(attach.item) >> pack;
			nMailAttachCount++;
		}
	}
	
	BYTE* pCount = (BYTE*)pack.getPositionPtr(pos);
	*pCount = (BYTE)nMailAttachCount;

}
bool CMailSystem::SendMail( MAILINFO & oneMail )
{
	/*
		超过150封，
		A) 如果玩家邮箱中有已领取了附件的邮件或无附件邮件，会自动删除最旧的该类邮件，接收并显示新的邮件
		B) 如果没有以上的邮件，则玩家只接收，但显示新的邮件，并给予玩家提示
	*/
	
	if (m_pEntity == NULL || !m_pEntity->IsInited() || m_pEntity->IsDestory())
	{
		return false;
	}
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator() )
	{
		return false;
	}
	
	int nMailMaxCount = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMailMaxCount;
	INT_PTR nCount = m_mailList.size();
	if (nCount >= nMailMaxCount)//超过150
	{
		//删除没附件的
		std::vector<MAILINFO>::iterator it = m_mailList.begin();
		for (; it != m_mailList.end(); it++)
		{
			MAILINFO& oneMail = *it;
			if (oneMail.nAttachCount == 0)
			{
				m_mailList.erase(it);
				nCount--;
			}
			if (nCount + 1 <= nMailMaxCount)//可以容纳新邮件
			{
				break;
			}
		}
	}
	
	if(isSameIdMail(oneMail))
	{
		return true;
	}
	if (m_mailList.size() +1 <= nMailMaxCount)
	{
		SendNewMail(oneMail);
	}
	else
	{
		m_pEntity->SendTipmsgFormatWithId(tmMailFullCantAdd, tstUI);
	}
	
	//放在SendNewMail后
	m_mailList.push_back(oneMail);
	
	return true;
}
void CMailSystem::SendNewMail(MAILINFO& oneMail)
{
	if (m_pEntity == NULL || !m_pEntity->IsInited() || m_pEntity->IsDestory())
	{
		return;
	}
	if (oneMail.isSendToClient)
	{
		return ;
	}
	// m_nMailIndex++;
	// oneMail.nMailId = m_nMailIndex;
	oneMail.isSendToClient = true;
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (byte)enMailSystemID<< (byte)sMailSystemNewMail;
	AddOneMailToPacket(netPack, oneMail);
	
	pack.flush();
}
void CMailSystem::SendMailByAward(unsigned int nActorId, LPCSTR sTitle, LPCSTR sContent,CVector<ACTORAWARD>& awardList, int nSrcId)
{
	CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
	if (awardList.count() > 0)
	{
		int mailAttachIndex = 0;
		for (int i =0 ; i < awardList.count(); i++)
		{
			ACTORAWARD& award = awardList[i];
			mailAttach[mailAttachIndex].nType = award.btType;
			mailAttach[mailAttachIndex].item.wItemId = award.wId;
			if(award.btType != qatEquipment)
				mailAttach[mailAttachIndex].item.wItemId = award.btType;
			mailAttach[mailAttachIndex].item.wCount = award.wCount;
			mailAttachIndex++;
			if (mailAttachIndex >= MAILATTACHCOUNT || (i == awardList.count()-1))
			{
				SendMail(nActorId,sTitle, sContent, mailAttach, nSrcId);
				mailAttachIndex = 0;
			}
		}
	}
	else
	{
		SendMail(nActorId,sTitle, sContent, NULL, nSrcId);
	}

	
}
bool CMailSystem::SendMail( unsigned int nActorId, LPCSTR sTitle, LPCSTR sContent ,MAILATTACH pAttach[MAILATTACHCOUNT], int nSrcId)
{
	CUserItem::ItemSeries mailSeries = GetGlobalLogicEngine()->BuildMailSn();
	MAILINFO oneMail;
	uint64_t nMailId = mailSeries.llId; 
	//OutputMsg(rmTip,"68713777777------------------------------------------------------------nMailId = %llu, int64 = %lld", nMailId, nMailId); 
	oneMail.nMailId = nMailId;
	if (nActorId == 0)
	{
		return false;
	}
	if (!sContent || !sTitle)
	{
		return false;
	}
	_asncpytA(oneMail.sTitle, sTitle);
	_asncpytA(oneMail.sContent, sContent);
	oneMail.nSrcId = nSrcId;
	if (pAttach)
	{
		memcpy(oneMail.mailAttach, pAttach, sizeof(oneMail.mailAttach));
	}

	oneMail.nCreateDt = time(NULL); //GetGlobalLogicEngine()->getMiniDateTime();

	//生成极品属性
	for (int i =0; i< ArrayCount(oneMail.mailAttach);i++)
	{
		MAILATTACH& attach = oneMail.mailAttach[i];
		if (attach.wQualityDataIndex > 0 && attach.item.btQuality > 0)
		{
			//zgame没有极品属性
			//RandAttrSelector::InitSmithById(attach.item.wItemId, attach.item.btQuality, attach.wQualityDataIndex, attach.item.smithAttrs);	
			attach.wQualityDataIndex = 0;
		}
		if ((attach.nType != 0 || attach.item.wItemId !=0) && attach.item.wCount > 0)
		{
			 oneMail.nAttachCount++;//顺便记录附件数量
		}
	}

	
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	LPCSTR sName = "";
	int nOldMailCount = 0;
 
	if (pActor == NULL)
	{
		//发送离线邮件消息
		SaveOfflineMail(nActorId ,oneMail);	
		return true;
	}
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		//发送离线邮件消息
		SaveOfflineMail(nActorId ,oneMail);
		return true;
	} 
	sName = pActor->GetEntityName();
	nOldMailCount = (int)pActor->GetMailSystem().GetMailCount();
	pActor->GetMailSystem().SendMail(oneMail);
 
	// if(GetLogicServer()->GetLocalClient())
	// {
	// 	LPCSTR tFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpSendMailFormat);
	// 	char stMailItemLog1[1024] = {0};
	// 	char stMailItemLog2[1024] = {0};
	// 	char stMailItemLog3[1024] = {0};
	// 	for (int i =0; i < ArrayCount(oneMail.mailAttach); i++)
	// 	{
	// 		MAILATTACH attch = oneMail.mailAttach[i];
	// 		if (attch.item.wCount > 0)
	// 		{
	// 			if (i <2)
	// 			{
	// 				sprintf(stMailItemLog1, tFormat, stMailItemLog1,attch.nType,attch.item.wItemId,attch.item.wCount);
	// 			}
	// 			else if (i<4)
	// 			{
	// 				sprintf(stMailItemLog2, tFormat, stMailItemLog2,attch.nType,attch.item.wItemId,attch.item.wCount);
	// 			}
	// 			else
	// 			{
	// 				sprintf(stMailItemLog3, tFormat, stMailItemLog3,attch.nType,attch.item.wItemId,attch.item.wCount);
	// 			}
	// 		}
	// 	}
	// 	char sSeries[64]; 
	// 	sprintf(sSeries,"%lld",oneMail.nMailId); //序列号
		
	// }
	return true;
}


bool CMailSystem::RemoveMail(INT64 nMailId)
{
	std::vector<MAILINFO>::iterator it = m_mailList.begin();
	for (; it != m_mailList.end();)
	{
		MAILINFO & info = *it;
		if (info.nMailId == nMailId)
		{
			info.nIsDel = 1;
			m_mailDelList.push_back(info);
			it = m_mailList.erase(it);
			return true;
		}
		else
		{
			it++;
		}
		
	}
	return false;
}
void CMailSystem::DeleteMailsAll()
{
	INT_PTR nCount = m_mailList.size();
	if(nCount <= 0)
		return;
	// int nMailMaxCount = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMailMaxCount;
	// CActorPacket pack;
	// CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// netPack << (byte)GetSystemID() << (byte)sMailSystemDeleteMails;
	// int nPos = netPack.getPosition();
	int nDelCount = 0;
	// netPack << (WORD)nDelCount;
	//只删除已提取附件的
	// for (int i = nCount -1; i > -1; i--)
	// {
	// 	MAILINFO& oneMail = m_mailList[i];
	// 	if (oneMail.nAttachCount == 0)
	// 	{
	// 		uint64_t nMailId = oneMail.nMailId;
	// 		if(RemoveMail(oneMail.nMailId))
	// 		{
	// 			netPack << (INT64)nMailId;
	// 			nDelCount++;
	// 		}
	// 	}
	// }
	for (std::vector<MAILINFO>::iterator it = m_mailList.begin(); it != m_mailList.end();)
	{
		MAILINFO & info = *it;
		if (info.nAttachCount == 0 || (info.nAttachCount > 0 && eMailState_Get == info.nState))
		{
			info.nIsDel = 1;
			// netPack << (INT64)info.nMailId;
			m_mailDelList.push_back(info);
			it = m_mailList.erase(it);
			nDelCount++;
		}else
		{
			it++;
		}
	}
	// WORD* pCount = (WORD*)netPack.getPositionPtr(nPos);
	// *pCount = (WORD)nDelCount;
	// pack.flush();
	if(nDelCount > 0)
	{
		m_pEntity->SendTipmsgFormatWithId(tmDelMailSuccess, tstUI);
	}
	
	SendAllMail();
}
void CMailSystem::DeleteMails(CDataPacketReader & packet)
{
	INT64  nMailSerie = 0;
	packet >> nMailSerie;
	
	INT_PTR nNum = m_mailList.size();
	if(nNum <= 0)
		return;

	MAILINFO* pOneMail =  GetMail(nMailSerie);
	int nDeleteCount = 0;
	// if (pOneMail && GetLogicServer()->GetLocalClient())
	// {
	// 	LogDelMail(*pOneMail);
	// }
	if(pOneMail && pOneMail->nAttachCount > 0 && eMailState_Get != pOneMail->nState)
	{
		m_pEntity->SendTipmsgFormatWithId(tmMailHasItem, tstUI);
		return;
	}
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (byte)GetSystemID() << (byte)sMailSystemDeleteMails;
	int pos = netPack.getPosition();
	netPack << (WORD)nDeleteCount;
	if(RemoveMail(nMailSerie))
	{
		netPack << (INT64)nMailSerie;
		nDeleteCount++;
	}
	WORD* nCount = (WORD*)netPack.getPositionPtr(pos);
	*nCount = (WORD)nDeleteCount;
	pack.flush();

}

void CMailSystem::DeleteSomeMails(CDataPacketReader & packet)
{
	BYTE nDelMailCount = 0;
	packet >> nDelMailCount;
	if((int)nDelMailCount <= 0)
	{
		return;
	}
	INT_PTR nNum = m_mailList.size();
	if(nNum <= 0)
	{
		return;
	}

	INT64 nMailSerie = 0;
	std::vector<INT64> nMailSerieList;
	for (size_t i = 0; i < nDelMailCount; i++)
	{
		packet >> nMailSerie; 
		nMailSerieList.push_back(nMailSerie);
	}
	
	int nDeleteCount = 0;
	
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (byte)GetSystemID() << (byte)sMailSystemSomeMails;
	int pos = netPack.getPosition();
	netPack << (byte)nDeleteCount;
	for (size_t i = 0; i < nMailSerieList.size(); i++)
	{
		INT64 nMailSerieItem = nMailSerieList[i]; 
		
		MAILINFO* pOneMail = GetMail(nMailSerieItem);
		
		if(pOneMail && pOneMail->nAttachCount > 0 && eMailState_Get != pOneMail->nState)
		{
			m_pEntity->SendTipmsgFormatWithId(tmMailHasItem, tstUI);
			continue;
		}
		
		if(RemoveMail(nMailSerieItem))
		{
			netPack << (INT64)nMailSerieItem;
			nDeleteCount++;
		}
	}

	byte* nCount = (byte*)netPack.getPositionPtr(pos);
	*nCount = (byte)nDeleteCount;
	pack.flush();

	// MAILINFO* pOneMail = GetMail(nMailSerie);
	// // if (pOneMail && GetLogicServer()->GetLocalClient())
	// // {
	// // 	LogDelMail(*pOneMail);
	// // }
	// if(pOneMail && pOneMail->nAttachCount > 0 && eMailState_Get != pOneMail->nState)
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmMailHasItem, tstUI);
	// 	return;
	// }
	// CActorPacket pack;
	// CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// netPack << (byte)GetSystemID() << (byte)sMailSystemSomeMails;
	// int pos = netPack.getPosition();
	// netPack << (WORD)nDeleteCount;
	// if(RemoveMail(nMailSerie))
	// {
	// 	netPack << (INT64)nMailSerie;
	// 	nDeleteCount++;
	// }
	// WORD* nCount = (WORD*)netPack.getPositionPtr(pos);
	// *nCount = (WORD)nDeleteCount;
	// pack.flush();

}
void CMailSystem::LogDelMail(MAILINFO& mailInfo)
{
	unsigned int nActorId = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID);
	INT_PTR nCount = GetMailCount();
	char sSeries[64];
	sprintf(sSeries,"%d",mailInfo.nMailId);
	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
}
void CMailSystem::LogGetMailAttach(MAILINFO& mailInfo)
{
	unsigned int nActorId = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID);
	INT_PTR nCount = GetMailCount();
	char sSeries[1024];
	sprintf(sSeries,"%lld",mailInfo.nMailId); //序列号
	

	char sLong[4][1024];
	for (int i =0; i < MAILATTACHCOUNT;i++)
	{
		int nCount = mailInfo.mailAttach[i].item.wCount;
		// if (mailInfo.mailAttach[i].nType != qatEquipment)
		// {
		// 	nCount = mailInfo.mailAttach[i].nResourceCount;
		// }
		if (i%2== 0)
		{
			sprintf(sLong[i/2],"%d-%d-%d", mailInfo.mailAttach[i].nType, mailInfo.mailAttach[i].item.wItemId, nCount);
		}
		else
		{
			sprintf(sLong[i/2],"%s-%d-%d-%d", sLong[i/2],mailInfo.mailAttach[i].nType, mailInfo.mailAttach[i].item.wItemId,nCount);
		}
	}
}

bool CMailSystem::SendMailFromUserItem( unsigned int nActorId, LPCSTR sTitle, LPCSTR sContent, CUserItem* pUserItem)
{
	if( !pUserItem )
	{
		return false;
	}

	CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
	SetUseItemToMailAttach(*pUserItem, mailAttach[0]);
	return CMailSystem::SendMail( nActorId, sTitle, sContent, mailAttach );
}

bool CMailSystem::isSameIdMail(MAILINFO& mailInfo)
{
	int nCount = m_mailList.size();
	for (int i = 0; i < nCount; i++)
	{
		MAILINFO& one = m_mailList[i];
		if (one.nMailId == mailInfo.nMailId)
		{
			return true;
		}
	}
	return false;
}


void CMailSystem::SendCrossServerMail(unsigned int nActorId,LPCSTR sTitle, LPCSTR sContent,std::vector<ACTORAWARD>& awardList, int nSrcId)
{
	
	int nid = 0; 
	int nServerId = 0;
	CActor::GetCSRealyActorId(nActorId, nid, nServerId);
	if(nid == 0) {
		OutputMsg(rmError,_T("SendCrossServerMail nActorId=%d,sTitle:%s"),nActorId,sTitle);
		return;
	}
	char buff[512];
	CDataPacket outPack(buff, sizeof(buff));
	outPack <<(unsigned int)nid<<(unsigned int)nServerId;
	outPack.writeString(sTitle);
	outPack.writeString(sContent);
	int nCount = awardList.size();
	outPack <<(BYTE)nCount;
	for(auto& it : awardList) {
		outPack << it;
	}
	GetLogicServer()->GetCrossClient()->SendRawServerData(jxInterSrvComm::CrossServerProto::sSendCSMail, outPack.getMemoryPtr(), outPack.getPosition());
}
