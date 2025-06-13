#include "StdAfx.h"

using namespace jxSrvDef;
using namespace jxInterSrvComm;

CDBCenterClient::CDBCenterClient(CDBServer *pDBEngine)
{
	m_pDBEngine = pDBEngine;
	SetClientName(_T("DBCenter服务器"));
	m_vFreeList.setLock(&m_vFreeListLock);
}

CDBCenterClient::~CDBCenterClient()
{
	m_vFreeList.flush();	
	for (INT_PTR i = 0; i < m_vFreeList.count(); i++)
	{
		CDataPacket* dp = m_vFreeList[i];
		dp->setPosition(0);
		flushSendPacket(*dp);
	}
	
	m_vFreeList.clear();
}

CDataPacket& CDBCenterClient::AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd)
{
	return allocProtoPacket(nCmd);
}

void CDBCenterClient::FlushDataPacket(CDataPacket &packet)
{
	return flushProtoPacket(packet);
}

void CDBCenterClient::Init()
{
	//m_pGateMgr = m_pDBEngine->getGateManager();
	//m_pSQLConn = m_pGateMgr->GetSQLConnectionPtr();
	CDBDataServer *pDataServer = m_pDBEngine->getDataServer();
	m_pSQLConn = pDataServer->GetSQLConnection();
	
	m_sDataHandler.Start(this, m_pSQLConn);
	m_loginHandler.Init(m_pSQLConn, pDataServer);
}

void CDBCenterClient::Stop()
{
	m_sDataHandler.Stop();
}

VOID CDBCenterClient::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3, UINT64 uParam4)
{
	//Inherited::DispatchInternalMessage(uMsg, uParam1, uParam2, uParam3);	
	switch (uMsg)
	{
	case DBC_INTERNAL_UPDATECLIENTLIST:
		UpdateLogicClientList((INT_PTR)uParam1, (int *)uParam2);
		break;

	default:
		break;
	}
}

void CDBCenterClient::PostUpdateLogicClientList(const CBaseList<int> &serverIdList)
{
	int nCount = (int)serverIdList.count();
	int *data = NULL;
	if (nCount > 0)
	{
		data = (int *)m_Allocator.AllocBuffer(sizeof(int) * nCount);
		int *pIter = data;
		CopyMemory(pIter, (int *)serverIdList, sizeof(int)*nCount);
	}	

	PostInternalMessage(DBC_INTERNAL_UPDATECLIENTLIST, nCount, (UINT_PTR)data, 0);
}

void CDBCenterClient::UpdateLogicClientList(INT_PTR nCount, int *idList)
{	
	if (connected())
	{
		CDataPacket &packet = AllocDataPacket(DB2DC_UpdateLogicClientList);
		packet << (int)nCount;
		//OutputMsg(rmTip, _T("%s 更新连接的逻辑服务器客户端列表，数量为:%d"), __FUNCTION__, (int)nCount);	
		if (nCount > 0)
		{
			packet.writeBuf(idList, sizeof(int) * nCount);			
		}
		FlushDataPacket(packet);
	}

	if (nCount > 0)
	{
		m_Allocator.FreeBuffer(idList);
	}
}

VOID CDBCenterClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{

	if (nCmd < 0) 
	{
		OutputMsg(rmWaning, _T("%s recv unknown msg[id=%d]"), __FUNCTION__, (int)nCmd);
		return;
	}
	if(nCmd == DBCenterHeartBeat )
	{
		return;
	}
	//是登陆的消息包，要最优先处理，让玩家迅速能够拿到角色列表
	if(nCmd >= dcGetActorList && nCmd < dcLoginMsgMax)
	{

		//CDataPacket& out = allocProtoPacket(nCmd);
		m_loginHandler.OnRecvLoginMsg(nCmd,inPacket,this); //处理登陆消息，不做缓存，立刻处理
		//flushProtoPacket(out);
		return;
	}
	else
	{
		m_sDataHandler.AddMessage(nCmd, inPacket);
	}
	
	/*
	switch (nCmd)
	{
	case DBCenterHeartBeat:
		break;

	case DBCenterGateRequest:
		HandleDBGateRequest(inPacket);
		break;

	case DBCenterLogicRequest:
		HandleLogicRequest(inPacket);
		break;

	default:
		OutputMsg(rmError, _T("%s recv unknown recv unknown msg[id=%d]"), __FUNCTION__, (int)nCmd);
		break;
	}
	*/
	
}

void CDBCenterClient::HandleDBGateRequest(CDataPacketReader &inPacket)
{
	INTERSRVCMD nCmd;
	inPacket >> nCmd;
	switch (nCmd)
	{
	case DbServerProto::DC2DB_QueryActorList:
		OnRecvQueryActorList(inPacket);
		break;

	case DbServerProto::DC2DB_EnterGame:
		OnRecvEnterGame(inPacket);
		break;

	default:
		break;
	}
	OutputMsg(rmTip, _T("%s recv dbgate msg(id=%d)"), __FUNCTION__, nCmd);
}

void CDBCenterClient::OnRecvEnterGame(CDataPacketReader &inPacket)
{	
	int nAccountId, nLoginServerId, nRawServerId;
	unsigned int nActorId = 0;
	ACCOUNT szAccountName;
	char szIP[32] = {0};
	__int64 nIP;
	inPacket >> nAccountId >> nLoginServerId >> nRawServerId >> nActorId >> nIP;
	inPacket.readString(szAccountName, ArrayCount(szAccountName));
	inPacket.readString(szIP, ArrayCount(szIP));

	int nError;
	CDataPacket &packet = AllocDataPacket(DbServerProto::DB2DC_EnterGameAck);
	packet << nAccountId << nActorId;
	if (!m_pSQLConn->Connected())
	{
		OutputMsg(rmError, _T("%s 数据库连接没就绪!"), __FUNCTION__);
		nError = ERR_SQL;
	}
	else
	{
		nError = m_gateDBReqHandler.StartEnterGame(nLoginServerId, 
												   nRawServerId,
												   nAccountId, 												   
												   nActorId, 
												   szAccountName, 
												   nIP);
		if (nError)
			nError = ERR_NOUSER;
	}

	if (!nError)
	{		
		/*
		char szActorId[32];
		sprintf_s(szActorId, sizeof(szActorId), "%d", nActorId);
		m_pDBEngine->getLogClient()->SendLoginLog(ltEntryGame,
												  nAccountId,
												  szAccountName,
												  szIP,
												  szActorId);
		*/

	}
	
	char szGateIP[128];
	INT_PTR nGatePort;
	if ( !m_pDBEngine->SelectGameServerRoute(nLoginServerId, szGateIP, ArrayCount(szGateIP), &nGatePort))
	{
		OutputMsg( rmError, _T("%s 无匹配的路由数据, ServerId=%d!"), __FUNCTION__, nLoginServerId);
		nError = ERR_NOGATE;
	}

	// Response Ack Msg: AccountId | ActorId | ErrorCode(char) | RouteIP(string) | RoutePort(int)
	packet << (char)nError;
	if (!nError)
	{		
		packet.writeString(szGateIP);
		packet << (int)nGatePort;
	}
	FlushDataPacket(packet);
}

void CDBCenterClient::OnRecvQueryActorList(CDataPacketReader &inPacket)
{
	unsigned int nAccountId;
	int nServerIndex;
	inPacket >> nAccountId >> nServerIndex;
	CDataPacket &packet = AllocDataPacket(DbServerProto::DB2DC_QueryActorListAck);
	packet << (WORD)QUERYACTORLISTRESP_CMD << nAccountId;
	m_gateDBReqHandler.GetActorList(nAccountId, nServerIndex,packet);
	FlushDataPacket(packet);
}

void CDBCenterClient::HandleLogicRequest(CDataPacketReader &inPacket)
{
	WORD nCmd = -1;
	inPacket >> nCmd;	
	m_sDataHandler.AddMessage(nCmd, inPacket);
}

SERVERTYPE CDBCenterClient::getLocalServerType()
{
	return DBServer;
}

int CDBCenterClient::getLocalServerIndex()
{
	return m_pDBEngine->getServerIndex();
}

LPCSTR CDBCenterClient::getLocalServerName()
{
	return m_pDBEngine->getServerName();
}

CDataPacket* CDBCenterClient::AllocSendPacket()
{
	if (m_vFreeList.count() <= 0)
	{
		m_vFreeList.flush();
	}
	if (m_vFreeList.count() <= 0)
	{
		allocSendPacketList(m_vFreeList,512);
	}
		
	CDataPacket* m_TempData = m_vFreeList.pop();
	m_TempData->setLength(0);
	return m_TempData;
}

void CDBCenterClient::FreeBackUserDataPacket(CDataPacket *pPacket)
{
	m_vFreeList.append(pPacket);
}