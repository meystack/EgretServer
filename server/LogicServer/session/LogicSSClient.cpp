#include <new>
#include "StdAfx.h"
#include "LogicSSClient.h"


using namespace jxSrvDef;
using namespace jxInterSrvComm;
using namespace jxInterSrvComm::SessionServerProto;

CLogicSSClient::CLogicSSClient(CLogicServer *lpLogicServer)
	:Inherited()
{
	m_pLogicServer = lpLogicServer;

	m_vFreeList.setLock(&m_vFreeListLock);
}

VOID CLogicSSClient::OnConnected()
{
	Inherited::OnConnected();
	SendServerIndex();
}

//向服务器下发服务器index

VOID CLogicSSClient::SendServerIndex()
{
	int nServerIndex = GetLogicServer()->GetServerIndex();
	int nCommonServerIndex =GetLogicServer()->GetCommonServerId();
	bool isCommonServer = GetLogicServer()->IsCommonServer();

	if(connected())
	{
		CDataPacket &packet = allocProtoPacket(cSetCommonServerId);
		if(isCommonServer)
		{
			packet << (int) 0;
		}
		else
		{
			packet << (int) nCommonServerIndex;
		}
		packet << nServerIndex;
		flushProtoPacket(packet);
	}	
}


CLogicSSClient::~CLogicSSClient()
{
	Stop();
	FreeSessionBuffers();

	m_vFreeList.flush();
	//清除包
	for(int i = 0; i < m_vFreeList.count(); i++)
	{
		CDataPacket* dp = m_vFreeList[i];
		dp->setPosition(0);
		flushSendPacket(*dp);
	}
	m_vFreeList.clear();
}

int CLogicSSClient::getLocalServerIndex()
{
	return m_pLogicServer->GetServerIndex();
}


LPCSTR CLogicSSClient::getLocalServerName()
{
	return m_pLogicServer ? m_pLogicServer->getServerName() : "";
}

CCustomGlobalSession* CLogicSSClient::CreateSession()
{
	static const int SessionMemBlockSize = 1024;

	CLogicSession *pSession;
	INT_PTR nCount = m_FreeSessionList.count();
	//如果空闲会话列表为空则申请一批全局会话对象
	if ( nCount <= 0 )
	{
		//申请全局会话内存块
		pSession = (CLogicSession *)m_Allocator.AllocBuffer(sizeof(*pSession) * SessionMemBlockSize);
		//将内存块指针添加到全局会话内存指针列表中
		m_SessionMemList.add(pSession);
		//循环调用每个会话的构造函数并将会话添加到空闲用户列表中
		for (int i=0; i<SessionMemBlockSize; ++i)
		{
			new(pSession)CLogicSession();
			m_FreeSessionList.add(pSession);
			pSession++;
		}
		nCount = m_FreeSessionList.count();
	}
	//返回空闲会话列表尾端的会话指针
	nCount--;
	pSession = (CLogicSession*)m_FreeSessionList[nCount];
	m_FreeSessionList.trunc(nCount);

	return pSession;
}

VOID CLogicSSClient::DestroySession(CCustomGlobalSession* pSession)
{
	m_FreeSessionList.add(pSession);
}

VOID CLogicSSClient::OnOpenSession(CCustomGlobalSession* pSession, BOOL boIsNewSession)
{
	CLogicSession *pLogicSession = (CLogicSession*)pSession;
	//OutputMsg(rmTip,"CLogicSSClient::OnOpenSession, account=%u",pSession->nSessionId); //输出一下sessionid
	pLogicSession->pGamePlayer = NULL;
	
	OutputMsg(rmTip,_T("[Login] (7) 账号验证成功，开启玩家会话user： logickey(%lld)，SessId(%d), CurrentThreadId(%d)。"),
		pSession->lKey, pSession->nSessionId,GetCurrentThreadId());
	
	CDataPacket* outPacket = AllocSendPacket();
	if(outPacket ==NULL) return;
	outPacket->setPosition(0);

	*outPacket << *pSession; //直接把这个数据结构拷贝过去
	m_pLogicServer->GetGateManager()->PostInternalMessage(CCustomGateManager::SGIM_RECV_LOGIN_DATA,
		jxInterSrvComm::SessionServerProto::sCheckPasswdResult,(Uint64)outPacket,pSession->nGateIndex,0);	
}


VOID CLogicSSClient::OnCloseSession(CCustomGlobalSession* pSession)
{
	CLogicSession *pLogicSession = (CLogicSession*)pSession;

	//如果会话关联的逻辑用户存在，则向网关管理器投递按全局会话ID关闭用户的消息，继而在关闭网关用户的时候关闭逻辑玩家
	if ( pLogicSession->pGamePlayer )
	{
		OutputMsg(rmTip,"CLogicSSClient::OnCloseSession, account=%d",pSession->nSessionId); //输出一下sessionid
		//m_pLogicServer->PostKickGateUserByGlobalSessionId(pSession->nSessionId);
		pLogicSession->pGamePlayer = NULL;
	}
	else
	{
		OutputMsg(rmTip,"CLogicSSClient::OnCloseSession, account=%u,pGamePlayer =NULL",pSession->nSessionId); //输出一下sessionid
	}
}

VOID CLogicSSClient::OnUpdateSessionState(CCustomGlobalSession* pSession, jxSrvDef::GSSTATE oldState, jxSrvDef::GSSTATE newState)
{
}

VOID CLogicSSClient::OnQuerySessionExists(INT_PTR nSessionId)
{
	// 此处实现查询全局会话关联的逻辑玩家是否在线)
	CLogicGateManager *pGateManager = m_pLogicServer->GetGateManager();
	if(pGateManager )
	{
		pGateManager->PostQueryGateUserExistsBySessionId(nSessionId);
	}
}

/*
void CLogicSSClient::CatchCheckPasswdResult(CDataPacketReader &inPacket)
{
	UINT64 lKey =0;
	BYTE   bErrorCode =0;
	inPacket >> lKey;
	inPacket >> bErrorCode;
	if(bErrorCode)
	{

	}
}
*/


VOID CLogicSSClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	BYTE  nErroeCode;
	int nGateIndex ;
	Uint64 lKey ;
	int nSessionID;

	switch (nCmd)
	{
	case jxInterSrvComm::SessionServerProto::sUserItemList:
	case jxInterSrvComm::SessionServerProto::sGetUserItem:
	case jxInterSrvComm::SessionServerProto::sDeleteUserItem:
	case jxInterSrvComm::SessionServerProto::sAddValueCard:
	case jxInterSrvComm::SessionServerProto::sQueryAddValueCard:
	case jxInterSrvComm::SessionServerProto::sQueryYuanbaoCount:
	case jxInterSrvComm::SessionServerProto::sWithdrawYuanbao:
	case jxInterSrvComm::SessionServerProto::sReqestRransmitAck:
	case jxInterSrvComm::SessionServerProto::sRecvGroupMessage:
	case jxInterSrvComm::SessionServerProto::sOtherLogicMessage:
	case jxInterSrvComm::SessionServerProto::sLoadCsRank:
		{
			CDataPacket* outPacket = AllocSendPacket();
			if(outPacket ==NULL) return;
			outPacket->writeBuf(inPacket.getOffsetPtr(),inPacket.getLength() - inPacket.getPosition());
			CLogicServer::s_pLogicEngine->GetNetWorkHandle()->PostInternalMessage(SSM_SS_RET_DATA,nCmd,(Uint64)outPacket);

		}
		break;
		//校验密码的结果
	case jxInterSrvComm::SessionServerProto::sCheckPasswdResult:
		{
			//CDataPacket* outPacket = AllocSendPacket();
			//if(outPacket ==NULL) return;
			//outPacket->writeBuf(inPacket.getOffsetPtr(),inPacket.getLength() - inPacket.getPosition());
			
			inPacket >> nGateIndex ;
			inPacket >> lKey ;
			inPacket >> nErroeCode ;
			UINT64 nParam =  (Uint64)nGateIndex;
			
			m_pLogicServer->GetGateManager()->PostInternalMessage(CCustomGateManager::SGIM_RECV_LOGIN_CMD,
				nCmd,nParam,lKey,nErroeCode); //账户id是0

			break;
		}

		//这个不需要回复的
	case jxInterSrvComm::SessionServerProto::sCreateAccountResult:
		break;
	case jxInterSrvComm::SessionServerProto::sKickCrossServerUser:
		nSessionID =0;
		inPacket >> nSessionID;
		m_pLogicServer->PostKickGateUserByGlobalSessionId(nSessionID);
		
		break;
	default:
		Inherited::OnDispatchRecvPacket(nCmd,inPacket);
		break;
	}
}



VOID CLogicSSClient::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	CLogicSession *pSession;

	//设置会话关联的游戏角色对象，Param1=会话ID，Param2=角色对象
	if (uMsg == SCIM_SET_SESSION_PLAYER)
	{
		pSession = (CLogicSession*)GetSessionPtr(uParam1, NULL,uParam3);
		if (pSession)
		{
			pSession->pGamePlayer = (CActor*)uParam2;
		}
	}
	else Inherited::DispatchInternalMessage(uMsg, uParam1, uParam2, uParam3,uParam4);
}

VOID CLogicSSClient::OnWorkThreadStop()
{
	Inherited::OnWorkThreadStop();
	FreeSessionBuffers();
}

VOID CLogicSSClient::FreeSessionBuffers()
{
	INT_PTR i;

	m_FreeSessionList.empty();
	for (i=m_SessionMemList.count()-1; i>-1; --i)
	{
		m_Allocator.FreeBuffer(m_SessionMemList[i]);
	}
	m_SessionMemList.empty();
}

VOID CLogicSSClient::SendOnlinePlayerCount(INT_PTR nCount)
{	
	CDataPacket &data = allocProtoPacket(jxInterSrvComm::SessionServerProto::cOnlineCount); //分配一个 网络包
	data << (int)nCount;
	flushProtoPacket(data);
}

void CLogicSSClient::FreeBackUserDataPacket(CDataPacket* pDataPacket)
{
	m_vFreeList.append(pDataPacket);
}

CDataPacket* CLogicSSClient::AllocSendPacket()
{
	if (m_vFreeList.count() <= 0)
		m_vFreeList.flush();

	if (m_vFreeList.count() <= 0)
	{
		allocSendPacketList(m_vFreeList,512);
	}

	CDataPacket* m_TempData = m_vFreeList.pop();//得到一个空闲的Datapacket
	m_TempData->setPosition(0);
	return m_TempData;
}

void CLogicSSClient::BroadMsgToCommonLogicClient(LPCVOID pData, const SIZE_T nLen)
{
	if (nLen > 0)
	{
		CDataPacket &packet = allocProtoPacket(cSendGroupMessage);
		packet.writeBuf(pData, nLen);
		flushProtoPacket(packet);
	}
}

void CLogicSSClient::SendMsgToDestLogicClient(int nCmd, int nDestServerId, LPCVOID pData, const SIZE_T nLen)
{
	if (nDestServerId <= 0 || nCmd < 0)
		return;
	int nRawServerId = GetLogicServer()->GetServerIndex();
	if (nLen >= 0)
	{
		CDataPacket &packet = allocProtoPacket(cBroadcastOr1SrvMessage);
		packet << (int)nDestServerId;
		packet << (int)nRawServerId;
		packet << (WORD)nCmd;
		packet.writeBuf(pData, nLen);
		flushProtoPacket(packet);
	}
}

void CLogicSSClient::BroadMsgToLogicClient(int nCmd, LPCVOID pData, const SIZE_T nLen)
{
	if (nCmd < 0)
		return;
	int nRawServerId = GetLogicServer()->GetServerIndex();
	if (nLen >= 0)
	{
		CDataPacket &packet = allocProtoPacket(cBroadcastOr1SrvMessage);
		packet << (int)0;	//0则为广播到全部逻辑服
		packet << (int)nRawServerId;
		packet << (WORD)nCmd;
		packet.writeBuf(pData, nLen);
		flushProtoPacket(packet);
	}
}
