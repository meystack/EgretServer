#include "StdAfx.h"
#include "Container.hpp"
static const LPCTSTR g_serverMailFlagVarName = "serverMailFlag";
static const LPCTSTR g_serverMailVar = "serverMailvar";

using namespace jxInterSrvComm::DbServerProto;
CMailMgr::CMailMgr()
{
	m_nMaxServerMailId = 0;
	m_Init = false;
	nToDayTime = 0;
}
CMailMgr::~CMailMgr()
{

}

void CMailMgr::RunOne(CMiniDateTime& minidate,TICKCOUNT tick)
{
	if(m_1minuteTimer.CheckAndSet(tick)) //每分钟执行一次
	{
		LoadNewServerMail();//TODO 应该后台通知
		if(m_Init)
			SendServerMail();
	}
	if(!m_Init)
	{
		CCLVariant& pVarMgr	= GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar();
		unsigned int nValue = 0;
		CCLVariant* pVar = pVarMgr.get(g_serverMailVar);
		if( pVar != NULL )
		{
			nToDayTime = (unsigned int)(double)*pVar;
		}
		m_Init = true;
	}


}
void CMailMgr::Load()
{
	LoadNewServerMail();
}
void CMailMgr::LoadNewServerMail()
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(dcLoadNewServerMail);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId << m_nMaxServerMailId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

void CMailMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	int nRawServerId, nLoginServerId;
	CDataPacketReader inPacket(data,size);
	int nErrorCode;
	inPacket >> nRawServerId >> nLoginServerId >>  nErrorCode;

	if (nCmd == dcLoadNewServerMail && nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	{
		BYTE dataType = 0;
		WORD nCount = 0;
		inPacket >>dataType;
		
		if (dataType == 0)//邮件
		{
			inPacket >>nCount;
			for (int i =0; i < nCount; i++)
			{
				ServerMail mail;
				inPacket >> mail.serverData.nServerMailId;
				inPacket >> mail.serverData.nActorId;
				inPacket.readString(mail.serverData.sTitle);
				inPacket.readString(mail.serverData.sContent);
				inPacket >> mail.serverData.nMinVipLevel >> mail.serverData.nMaxVipLevel >> mail.serverData.nMinLevel >> mail.serverData.nMaxLevel;
				inPacket >> mail.serverData.nMinRegisterTime >> mail.serverData.nMaxRegisterTime >> mail.serverData.nMinLoginTime >> mail.serverData.nMaxLoginTime;
				inPacket >> mail.serverData.btOnline >> mail.serverData.btSex >> mail.serverData.nWay;
				m_serverMailList.add(mail);
				if (mail.serverData.nServerMailId > m_nMaxServerMailId)
				{
					m_nMaxServerMailId = mail.serverData.nServerMailId;
				}
			}
		}
		else if (dataType == 1)//附件
		{
			inPacket >>nCount;
			for (int i =0; i < nCount; i++)
			{
				int nServerMailId = 0;
				inPacket >> nServerMailId;
				ServerMail* pMail = GetServerMail(nServerMailId);
				if (pMail)
				{
					for (int j=0; j < MAILATTACHCOUNT; j++)
					{
						if (pMail->mailAttach[j].nServerMailId ==0)
						{
							pMail->mailAttach[j].nServerMailId =  nServerMailId;
							inPacket >> pMail->mailAttach[j].actorAward.btType;
							inPacket >> pMail->mailAttach[j].actorAward.wId;
							// inPacket >> pMail->mailAttach[j].actorAward.btQuality;
							int nItemCount =0;
							inPacket >> nItemCount;
							pMail->mailAttach[j].actorAward.wCount = nItemCount;
							
							// inPacket >> pMail->mailAttach[j].actorAward.btBind;
							// inPacket >> pMail->mailAttach[j].actorAward.wStar;
							// inPacket >> pMail->mailAttach[j].actorAward.btStarLost;
							// inPacket >> pMail->mailAttach[j].actorAward.btInscriptLevel;
							// inPacket >> pMail->mailAttach[j].actorAward.nAreaId;
							// inPacket >> pMail->mailAttach[j].actorAward.btLuck;	
							// inPacket >> pMail->mailAttach[j].actorAward.nSmith[0];
							// inPacket >> pMail->mailAttach[j].actorAward.nSmith[1];
							// inPacket >> pMail->mailAttach[j].actorAward.nSmith[2];
							// inPacket >> pMail->mailAttach[j].actorAward.nSmith[3];
							// inPacket >> pMail->mailAttach[j].actorAward.nSmith[4];
							// inPacket >> pMail->mailAttach[j].actorAward.wIdentifyslotnum;
							// inPacket >> pMail->mailAttach[j].actorAward.wIdentifynum;
							break;
						}
					}
				}
				else
				{
					OutputMsg(rmWaning,"loadservermailattach error id =%d",nServerMailId);
				}
			}
		}
		else if(dataType == 2)
		{
			HandleAllServerMail();
		}
		
	}

	
}
void CMailMgr::DelServerMail(int nServerMailId)
{
	for (INT_PTR i= m_serverMailList.count()-1; i>=0;i--)
	{
		ServerMail& mail = m_serverMailList[i];
		if (mail.serverData.nServerMailId == nServerMailId)
		{
			SendServerMailComplete(mail.serverData.nServerMailId);
			m_serverMailList.remove(i);
			break;
		}
	}
}
void CMailMgr::OnEnterGame(CActor* pActor)
{
	if (!pActor)
	{
		return ;
	}

	for (INT_PTR i= m_serverMailList.count()-1; i>=0;i--)
	{	
		ServerMail& mail = m_serverMailList[i];
		if(mail.serverData.btOnline == eMo_All || mail.serverData.btOnline == eMo_OffLine)
		{
			if (!IsActorGetServerMail(pActor,mail.serverData.nServerMailId)&& CheckServerMailCond(pActor, mail.serverData,true))
			{
				SendServerMailById(pActor->GetId(), mail);
				SetActorGetServerMail(pActor, mail.serverData.nServerMailId);
			}
		}
	}
}
void CMailMgr::HandleAllServerMail()
{
	for (INT_PTR i= m_serverMailList.count()-1; i>=0;i--)
	{
		ServerMail& mail = m_serverMailList[i];
		if (mail.serverData.nActorId != 0)
		{
			SendServerMailById(mail.serverData.nActorId, mail);
			SendServerMailComplete(mail.serverData.nServerMailId);
			m_serverMailList.remove(i);
			continue;
		}
		else//全服邮件，判断条件
		{
			
			CVector<void*> pEntityList;
			pEntityList.clear();
			GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(pEntityList);
			INT_PTR nCount = pEntityList.count();
			for (INT_PTR j = 0; j < nCount; j++)
			{
				CActor *pActor = (CActor *)pEntityList[j];
				if (pActor && pActor->GetType() == enActor)
				{
					if (mail.serverData.btOnline == eMo_OffLine)
					{
						SetActorGetServerMail(pActor, mail.serverData.nServerMailId);//在线玩家设置已领标志
					}
					else if(mail.serverData.btOnline == eMo_All || mail.serverData.btOnline == eMo_Online)
					{
						//处理全服邮件，发送所有玩家时，在线马上发，其他玩家登录判断条件再发
						if (!IsActorGetServerMail(pActor,mail.serverData.nServerMailId)&& CheckServerMailCond(pActor, mail.serverData))
						{
							SendServerMailById(pActor->GetId(), mail);
							SetActorGetServerMail(pActor, mail.serverData.nServerMailId);
						}
					}	
				}
			}
			if (mail.serverData.btOnline == eMo_Online)
			{
				//只发在线的可以设置完成，不再处理
				SendServerMailComplete(mail.serverData.nServerMailId);
				m_serverMailList.remove(i);
			}
		}
	}
}
bool CMailMgr::CheckServerMailCond(CActor* pActor, ServerMailDef& mail, bool isLogin)
{
	if (!pActor)
	{
		return false;
	}
	int nLevel = pActor->GetLevel();
	unsigned int nCreateTime = pActor->GetCreateTime();
	unsigned int nLoginTime = pActor->GetLastLogoutTime();
	if (!isLogin)
	{
		nLoginTime = pActor->GetLoginTime();
	}
	char nSex = pActor->GetProperty<int>(PROP_ACTOR_SEX);
	if (mail.nMinLevel >0 && nLevel < mail.nMinLevel)
	{ 
		return false;
	}
	if (mail.nMaxLevel >0 && nLevel > mail.nMaxLevel)
	{ 
		return false;
	}
	if (mail.nMinRegisterTime > 0 && nCreateTime < mail.nMinRegisterTime)
	{
		return false;
	}
	if (mail.nMaxRegisterTime >0 && nCreateTime > mail.nMaxRegisterTime)
	{ 
		return false;
	}
	if (mail.nMinLoginTime >0 && nLoginTime < mail.nMinLoginTime)
	{ 
		return false;
	}
	if (mail.nMaxLoginTime >0 && nLoginTime > mail.nMaxLoginTime)
	{ 
		return false;
	}
	if (mail.btSex !=-1 && nSex != mail.btSex)
	{
		return false;
	}
	return true;
}
bool CMailMgr::IsActorGetServerMail(CActor* pActor, int nServerMailId)
{
	if (!pActor)
	{
		return false;
	}
	CCLVariant&var = pActor->GetActorVar();
	bool nClearFalg = false;
	CMiniDateTime combineServerTime  = GetLogicServer()->GetServerCombineTime(); //获取合服的绝对时间

	if(combineServerTime.tv != 0 && (combineServerTime.tv != pActor->GetStaticCountSystem().GetStaticCount(nMAIL_COMBINE_TIME)))
	{
		nClearFalg = true;
		pActor->GetStaticCountSystem().SetStaticCount(nMAIL_COMBINE_TIME, combineServerTime.tv);
	}
	CCLVariant * pMailFlagList	= var.get(g_serverMailFlagVarName);
	if(!pMailFlagList)
	{
		return false;
	}
	else
	{
		if(nClearFalg)
		{
			pMailFlagList->clear();
			return false;
		}
		char varName[64] = {0};
		sprintf(varName,"%d",nServerMailId);
		CCLVariant * pFlag = pMailFlagList->get(varName);
		if (pFlag)
		{
			return true;
		}
	}
	return false;
}
void CMailMgr::SetActorGetServerMail(CActor* pActor, int nServerMailId)
{
	if (!pActor)
	{
		return;
	}
	CCLVariant&var = pActor->GetActorVar();
	CCLVariant * pMailFlagList = var.get(g_serverMailFlagVarName);

	if(!pMailFlagList)
	{
		var.set(g_serverMailFlagVarName);
		pMailFlagList = var.get(g_serverMailFlagVarName);
	}
	if (pMailFlagList)
	{
		char varName[64] = {0};
		sprintf(varName,"%d",nServerMailId);
		pMailFlagList->set(varName,1);
	}
}
void CMailMgr::SendServerMailComplete(int nServerMailId)
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(dcSaveNewServerMail);//主要是设置下完成标志，下次不读
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId << nServerMailId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}
void CMailMgr::SendServerMailById(unsigned int nActorId, ServerMail& mail)
{
	CVector<ACTORAWARD> awardlist;
	for(int i =0; i< MAILATTACHCOUNT; i++)
	{
		ServerMailAttach& attach = mail.mailAttach[i];
		if (attach.actorAward.wId ==0)
		{
			continue;
		}
		awardlist.add(attach.actorAward);
	}
	CMailSystem::SendMailByAward(nActorId, mail.serverData.sTitle, mail.serverData.sContent, awardlist, mail.serverData.nWay);
}
ServerMail* CMailMgr::GetServerMail(int nServerMailId)
{ 
	for(int i =0; i<m_serverMailList.count(); i++)
	{
		ServerMail* mail = &m_serverMailList[i];
		if (mail->serverData.nServerMailId == nServerMailId)
		{
			return mail;
		}
	}
	return NULL;
}

//简单给测试GM命令测试 --未做线程安全
bool CMailMgr::TestAddServerMail(ServerMail& serverMail)
{ 
	AddserverMailToDb(serverMail);
	// HandleAllServerMail();
	return true;
}



void CMailMgr::AddserverMailToDb(ServerMail& serverMail)
{
	CDataPacket & DataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcAddNewServerMail);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (int)0;
	DataPacket << (unsigned int)serverMail.serverData.nActorId;
	DataPacket.writeString( serverMail.serverData.sTitle);
	DataPacket.writeString( serverMail.serverData.sContent);
	DataPacket << (int)serverMail.serverData.nMinVipLevel << (int)serverMail.serverData.nMaxVipLevel <<  (int)serverMail.serverData.nMinLevel <<  (int)serverMail.serverData.nMaxLevel;
	DataPacket <<(unsigned int)serverMail.serverData.nMinRegisterTime << (unsigned int)serverMail.serverData.nMaxRegisterTime << (unsigned int)serverMail.serverData.nMinLoginTime << (unsigned int)serverMail.serverData.nMaxLoginTime;
	DataPacket <<  (int)serverMail.serverData.btOnline <<  (int)serverMail.serverData.btSex <<  (int)serverMail.serverData.nWay;

	int pos = DataPacket.getPosition();
	BYTE nCount = 0;
	DataPacket << (BYTE)nCount;
	for(int i =0; i< MAILATTACHCOUNT; i++)
	{
		ServerMailAttach& attach = serverMail.mailAttach[i];
		DataPacket << attach.actorAward.btType;
		DataPacket << attach.actorAward.wId;
		DataPacket << attach.actorAward.wCount;
		nCount++;
	}
	BYTE* pPos = (BYTE*)DataPacket.getPositionPtr(pos);
	*pPos = nCount;

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}



void CMailMgr::AddserverMailToDb(const char * sTitle, const char * sContent,unsigned int nActorId, std::vector<ACTORAWARD> vAwards, int nMinVipLevel, int nMaxVipLevel,
int nMinLevel, int nMaxLevel,int nMinRegisterTime,int nMaxRegisterTime,int nMinLoginTime,int nMaxLoginTime,int btOnline,int btSex,
int nWay)
{
	CDataPacket & DataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcAddNewServerMail);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (int)0;
	DataPacket << (unsigned int)nActorId;
	DataPacket.writeString(sTitle);
	DataPacket.writeString(sContent);
	DataPacket <<  (int)nMinVipLevel << (int) nMaxVipLevel <<  (int)nMinLevel <<  (int)nMaxLevel;
	DataPacket <<  (unsigned int)nMinRegisterTime << (unsigned int)nMaxRegisterTime <<  (unsigned int)nMinLoginTime <<  (unsigned int)nMaxLoginTime;
	DataPacket <<  (int)btOnline <<  (int)btSex << (int)nWay;


	int pos = DataPacket.getPosition();
	BYTE nCount = 0;
	DataPacket << (BYTE)nCount;
	for(int i =0; (i < vAwards.size()) && (i < MAILATTACHCOUNT); i++)
	{
		ACTORAWARD& actorAward = vAwards[i];
		DataPacket << actorAward.btType;
		DataPacket << actorAward.wId;
		DataPacket << actorAward.wCount;
		nCount++;
	}
	BYTE* pPos = (BYTE*)DataPacket.getPositionPtr(pos);
	*pPos = nCount;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}


void CMailMgr::SendServerMail()
{
	// int nNowTime = CMiniDateTime::now();
	// if( nNowTime > nToDayTime)
	// {
	// 	nToDayTime = CMiniDateTime::tomorrow();
	// 	int openday = GetLogicServer()->GetDaysSinceOpenServer() ;
	// 	static  std::map<int, std::vector<int> >& mails = GetLogicServer()->GetDataProvider()->GetMailConfig().m_nOpenDayMails;
	// 	std::map<int, std::vector<int> >::iterator it = mails.find(openday);
	// 	if(it != mails.end())
	// 	{
	// 		std::vector<int>& mail = it->second;
	// 		for(int i = 0; i < mail.size(); i++)
	// 		{
	// 			MailCfg* cfg = GetLogicServer()->GetDataProvider()->GetMailConfig().getOneMailCfg(mail[i]);
	// 			if(cfg)
	// 			{
	// 				AddserverMailToDb(cfg->cTitle, cfg->cContent, 0, cfg->vAwards, 0, 0, 0, 0, 0, nToDayTime);
	// 			}
	// 		}
	// 	}

	// 	// nToDayTime = CMiniDateTime::tomorrow();
	// 	CCLVariant& pVarMgr	= GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar();
	// 	pVarMgr.set(g_serverMailVar, (double)nToDayTime);
	// 	GetGlobalLogicEngine()->GetGlobalVarMgr().Save();
	// }
}
