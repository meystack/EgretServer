#include "StdAfx.h"
#include "LocalSender.h"


using namespace jxSrvDef;

#define LOGTIME_FLUCTUATE	60  

CLocalSender::CLocalSender(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName)
{
	this->ServerType = ServerType;
	this->ServerIndex = ServerIndex;
	strcpy(this->ServerName,ServerName);
	this->SetClientName(_T("本地日志"));
	
}

CLocalSender::CLocalSender()
	: ServerIndex(0)
	, ServerType(InvalidServer)
{
	ServerName[0] = 0;
	this->SetClientName(_T("本地日志"));
}

CLocalSender::~CLocalSender(void)
{
}

VOID CLocalSender::SetServerName(LPCTSTR sServerName)
{
	_tcsncpy(ServerName, sServerName, ArrayCount(ServerName) - 1);
	ServerName[ArrayCount(ServerName) - 1] = 0;
}

VOID CLocalSender::SendCurrencyLog(WORD nLogId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop,int nSrvId)
{
	//nAmount = (unsigned long long)nAmount;
	if (nAmount == 0) return;
	if (nAmount < 0) nAmount = nAmount * -1;

	LOGCURRENCY data;
	data.nLogId = nLogId;

	data.nActorid = nActorId;
	if (sActorName != NULL) {_asncpytA((data.szName),sActorName)}
	else data.szName[0] = 0;
	if (sAccount != NULL) {_asncpytA((data.szAccount),sAccount)}
	else data.szAccount[0] = 0;

	data.nLogType = nLogType;
	data.nCurrencyType = nCurrencyType;
	data.nAmount = nAmount;
	data.nBalance = nBalance;
	data.nIsShop = nIsShop;
	
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(LOG_SEND_CURRENCY_CMD); //分配一个 网络包
	pdata << data;
	pdata << (int)GetLogicServer()->GetSpid();
	
	if(nSrvId > 0)
	{
		pdata << (int)nSrvId;
	}else
	{
		pdata << (int)GetLogicServer()->GetServerIndex();
	}
	pdata << (int)GetLogicServer()->GetServerIndex();
	flushProtoPacket(pdata);
}

VOID CLocalSender::SendCurrencyLog(WORD nLogId,CActor *pActor,WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop, int nSrvId)
{
	if (pActor)
	{
		SendCurrencyLog(nLogId,
			pActor->GetId(),(char *)(pActor->GetEntityName()),(pActor->GetAccount()),
			nLogType,nCurrencyType,nAmount,nBalance,nIsShop,pActor->getOldSrvId());
	}
}

VOID CLocalSender::SendShopLog(INT nItemId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum,int nSrvId)	
{
	LOGSHOP data;
	data.nItemId = nItemId;

	data.nActorid = nActorId;
	if (sActorName != NULL) {_asncpytA((data.szName),sActorName);}
	else data.szName[0] = 0;
	if (sAccount != NULL) {_asncpytA((data.szAccount),sAccount);}
	else data.szAccount[0] = 0;

	data.nShopType = nShopType;
	data.nItemType = nItemType;
	data.nItemNum = nItemNum;
	data.nCurrencyType = nCurrencyType;
	data.nConsumeNum = nConsumeNum;
	
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(LOG_SEND_SHOP_CMD); //分配一个 网络包
	pdata << data;
	pdata << (int)GetLogicServer()->GetSpid();
	
	if(nSrvId > 0)
	{
		pdata << (int)nSrvId;
	}else
	{
		pdata << (int)GetLogicServer()->GetServerIndex();
	}
	pdata << (int)GetLogicServer()->GetServerIndex();
	
	flushProtoPacket(pdata);
}

VOID CLocalSender::SendShopLog(INT nItemId,CActor *pActor,WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum,int nSrvId)
{
	if (pActor)
	{
		SendShopLog(nItemId,
			pActor->GetId(),(char *)(pActor->GetEntityName()),(pActor->GetAccount()),
			nShopType,nItemType,nItemNum,nCurrencyType,nConsumeNum,pActor->getOldSrvId());
	}
}

VOID CLocalSender::SendItemLog(INT nItemId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nItemType,WORD nItemNum,WORD nOperType,LPCTSTR sReason,int nSrvId)
{
	LOGITEM data;
	data.nItemId = nItemId;

	data.nActorid = nActorId;
	if (sActorName != NULL) {_asncpytA((data.szName),sActorName);}
	else data.szName[0] = 0;
	if (sAccount != NULL) {_asncpytA((data.szAccount),sAccount);}
	else data.szAccount[0] = 0;

	data.nItemType = nItemType;
	data.nItemNum = nItemNum;
	data.nOperType = nOperType;
	if (sReason != NULL) _asncpytA((data.sReason),sReason)
	else data.sReason[0] = 0;
	
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(LOG_SEND_ITEM_CMD); //分配一个 网络包
	pdata << data;
	pdata << (int)GetLogicServer()->GetSpid();
	
	if(nSrvId > 0)
	{
		pdata << (int)nSrvId;
	}else
	{
		pdata << (int)GetLogicServer()->GetServerIndex();
	}
	pdata << (int)GetLogicServer()->GetServerIndex();
	flushProtoPacket(pdata);
}

VOID CLocalSender::SendItemLog(INT nItemId,CActor *pActor,WORD nItemType,WORD nItemNum,WORD nOperType,LPCTSTR sReason,int nSrvId)
{
	if (pActor)
	{
		SendItemLog(nItemId,
			pActor->GetId(),(char *)(pActor->GetEntityName()),(pActor->GetAccount()),
			nItemType,nItemNum,nOperType,sReason, pActor->getOldSrvId());
	}
}

VOID CLocalSender::SendTaskLog(INT nTaskId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nTaskType,WORD nStatu, int nSrvId)
{
	LOGTASK data;
	data.nTaskId = nTaskId;

	data.nActorid = nActorId;
	if (sActorName != NULL) {_asncpytA((data.szName),sActorName);}
	else data.szName[0] = 0;
	if (sAccount != NULL) {_asncpytA((data.szAccount),sAccount);}
	else data.szAccount[0] = 0;

	data.nTaskType = nTaskType;
	data.nStatu = nStatu;
	
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(LOG_SEND_TASK_CMD); //分配一个 网络包
	pdata << data;
	pdata << (int)GetLogicServer()->GetSpid();
	
	if(nSrvId > 0)
	{
		pdata << (int)nSrvId;
	}else
	{
		pdata << (int)GetLogicServer()->GetServerIndex();
	}
	pdata << (int)GetLogicServer()->GetServerIndex();
	flushProtoPacket(pdata);
}

VOID CLocalSender::SendTaskLog(INT nTaskId,CActor *pActor,WORD nTaskType,WORD nStatu, int nSrvId)
{
	if (pActor)
	{
		SendTaskLog(nTaskId,
			pActor->GetId(),(char *)(pActor->GetEntityName()),(pActor->GetAccount()),
			nTaskType,nStatu,pActor->getOldSrvId());
	}
}

VOID CLocalSender::SendActivityLog(INT nAtvId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nAtvType,WORD nStatu, int nSrvId)
{
	LOGACTIVITY data;
	data.nAtvId = nAtvId;

	data.nActorid = nActorId;
	if (sActorName != NULL) {_asncpytA((data.szName),sActorName);}
	else data.szName[0] = 0;
	if (sAccount != NULL) {_asncpytA((data.szAccount),sAccount);}
	else data.szAccount[0] = 0;

	data.nAtvType = nAtvType;
	data.nStatu = nStatu;
	
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(LOG_SEND_ACTIVITY_CMD); //分配一个 网络包
	pdata << data;
	pdata << (int)GetLogicServer()->GetSpid();
	// pdata << (int)GetLogicServer()->GetServerIndex();
	if(nSrvId > 0)
	{
		pdata << (int)nSrvId;
	}else
	{
		pdata << (int)GetLogicServer()->GetServerIndex();
	}
	pdata << (int)GetLogicServer()->GetServerIndex();
	
	flushProtoPacket(pdata);
}

VOID CLocalSender::SendActivityLog(INT nAtvId,CActor *pActor,WORD nAtvType,WORD nStatu, int nSrvId)
{
	if (pActor)
	{
		SendActivityLog(nAtvId,
			pActor->GetId(),(char *)(pActor->GetEntityName()),(pActor->GetAccount()),
			nAtvType,nStatu,pActor->getOldSrvId());
	}
}


VOID CLocalSender::SendCreateLocalLog(int nSpid,int Serverindex)
{
	CMiniDateTime curTime;
	curTime.tv = GetGlobalLogicEngine()->getMiniDateTime();
	if (curTime.tv >= m_tomorrowDateTime.tv)
	{
		// 更新Next New Day Timepoint
		//m_tomorrowDateTime.tv += 2 * TIME_FLUCTUATE;
		m_tomorrowDateTime.tv = m_tomorrowDateTime.rel_tomorrow() + wrand(LOGTIME_FLUCTUATE);	// 24点后TIME_FLUCTUATE秒分布

		CDataPacket &pdata = allocProtoPacket(LOG_CREATETABLE_CMD); //分配一个 网络包
		pdata << (int)nSpid;
		pdata << (int)Serverindex;
		flushProtoPacket(pdata);
	}
}

VOID CLocalSender::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	//空函数，本身不处理日志服务器下发的消息
}

jxSrvDef::SERVERTYPE CLocalSender::getLocalServerType()
{
	return ServerType;
}

LPCSTR CLocalSender::getLocalServerName()
{
	return ServerName;
}

int CLocalSender::getLocalServerIndex()
{
	return ServerIndex;
}
