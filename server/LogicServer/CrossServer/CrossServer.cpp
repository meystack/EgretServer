#include "StdAfx.h"

CCrossServer::CCrossServer(CCrossServerManager *lpSSManager)
:Inherited()
{
	m_pCSManager = lpSSManager;
	m_nEngineClientCount = 0;
	m_dwCheckOnlineLogTick = 0;
	m_boOnlineLoged = FALSE;
	m_dwCheckOnlineLogTick =0;

	m_SessionList.setLock(&m_SessionListLock);
	SetServiceName(_T("会话"));
}

CCrossServer::~CCrossServer()
{

}

VOID CCrossServer::SendLogOnlineCount()
{
}

typedef struct tagIpCount
{
	long long  lIp; //IP
	long long nCount; //登陆的数目

} IPCOUNT;
	
//发送当前ip的状况
void CCrossServer::SendOnlineIpStatus()
{
}



CCustomServerClientSocket* CCrossServer::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	CCrossClient * pClient = new CCrossClient ;
	pClient->SetClientSocket(nSocket, pAddrIn);
	pClient->SetParam(this,false);
	return pClient;
}

VOID CCrossServer::ProcessClients()
{
	INT_PTR i;
	INT_PTR nDBCount = 0, nEngineCount = 0;
	CCrossClient *pClient;
	TICKCOUNT dwCurTick = _getTickCount();

	//循环处理每个客户端连接
	m_ClientList.flush();
	//必须降序循环，因为列表中的数据可能在循环中被移除
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CCrossClient*)m_ClientList[i];
		//处理活动的连接
		pClient->Run();
		//删除断开的连接
		if ( !pClient->connected() )
		{
			//连接在断开后的5分钟后删除
			if ( dwCurTick - pClient->m_dwDisconnectedTick >= 5 * 60 * 1000 )
			{
				m_ClientList.lock();
				m_ClientList.remove(i);
				m_ClientList.unlock();
				delete pClient;
			}
			continue;
		}
	}
}

BOOL CCrossServer::DoStart()
{
	return TRUE;
}

VOID CCrossServer::DoStop()
{

}

//跨服逻辑处理
VOID CCrossServer::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	INT_PTR nIndex;
	// Inherited::DispatchInternalMessage(uMsg,uParam1,uParam2,uParam3,uParam4); //先执行父类的消息处理
	
	if(uMsg == SSM_CROSS_2_LOGIC_DATA)
	{
		//客户端请求逻辑服，逻辑服请求跨服产生玩家对应的跨服唯一id
		if(uParam1 == jxInterSrvComm::CrossServerProto::sGetCrossActorId) //
		{
			char sBuffer[64];
			int nCrossServerIndex = uParam2;
			int nActorId = uParam3;
			int nCrossActorId = uParam4;
			CDataPacket data(sBuffer,sizeof(sBuffer));
			PDATAHEADER pheader;
			data.setLength(sizeof(*pheader));

			data.setPosition(sizeof(*pheader));
			data <<(WORD)jxInterSrvComm::CrossServerProto::sGetCrossActorId;
			data << nActorId << nCrossActorId;	
			pheader = (PDATAHEADER)data.getMemoryPtr();
			pheader->tag=DEFAULT_TAG_VALUE ;
			pheader->len = data.getLength() - sizeof(*pheader) ;
			SendMsg2LogicClient(nCrossServerIndex,data.getMemoryPtr(), data.getLength());
		} 
		else if(uParam1 == jxInterSrvComm::CrossServerProto::sSendCSMail ||
			uParam1 == jxInterSrvComm::CrossServerProto::sCSGetRankList) //  
		{
			char sBuffer[7168];
			CDataPacket data(sBuffer,sizeof(sBuffer));
			PDATAHEADER pheader;
			data.setLength(sizeof(*pheader));

			data.setPosition(sizeof(*pheader));
			data <<(WORD)uParam1;
			CDataPacket *outPacket = (CDataPacket*)uParam2;
			int nCrossServerIndex = 0;
			if(outPacket) {
				outPacket->setPosition(0);
				// printf("size:%d\n",outPacket->getLength() - outPacket->getPosition());
				if(uParam1 == jxInterSrvComm::CrossServerProto::sSendCSMail)
				{
					int nActorId = 0;
					(*outPacket) >> nActorId >> nCrossServerIndex;
					// printf("cSendCSMail:nActorId:%d,nCrossServerIndex:%d\n",nActorId, nCrossServerIndex);
					data << nActorId;
				}
				else if(uParam1 == jxInterSrvComm::CrossServerProto::sCSGetRankList)
				{
					(*outPacket) >> nCrossServerIndex;
				}
			}
			data.writeBuf(outPacket->getOffsetPtr(),outPacket->getLength() - outPacket->getPosition());
			pheader = (PDATAHEADER)data.getMemoryPtr();
			pheader->tag=DEFAULT_TAG_VALUE ;
			pheader->len = data.getLength() - sizeof(*pheader) ;
			SendMsg2LogicClient(nCrossServerIndex,data.getMemoryPtr(), data.getLength());
			CLocalCrossClient *pClient = (CLocalCrossClient *)(uParam4);
			if (pClient)
				pClient->FreeBackUserDataPacket(outPacket);
		}
		else if (uParam1 == jxInterSrvComm::CrossServerProto::sGetActorOfflineData)
		{
			CDataPacket *outPacket = (CDataPacket*)uParam2;
			if(outPacket)
			{
				outPacket->setPosition(0);
				int nCrossServerIndex = 0;
				(*outPacket) >> nCrossServerIndex;
				CDataPacket &data = allocSendPacket();
				PDATAHEADER pheader;
				data.setLength(sizeof(*pheader));
				data.setPosition(sizeof(*pheader));
				data <<(WORD)uParam1;
				data.writeBuf(outPacket->getOffsetPtr(),outPacket->getLength() - outPacket->getPosition());
				pheader = (PDATAHEADER)data.getMemoryPtr();
				pheader->tag = DEFAULT_TAG_VALUE;
				pheader->len = data.getLength() - sizeof(*pheader);
				SendMsg2LogicClient(nCrossServerIndex, data.getMemoryPtr(), data.getLength());
			 }
		}
		else if(uParam1 == jxInterSrvComm::CrossServerProto::sSendReqChat) //
		{
			//char sBuffer[1024];
			CDataPacket *outPacket = (CDataPacket*)uParam2;
			if(outPacket)
			 {
				outPacket->setPosition(0);
				int nLogicServerIndex = 0;
				(*outPacket) >> nLogicServerIndex;

				printf("size:%d\n", outPacket->getLength()- outPacket->getPosition());
				 
				printf("sSendReqChat: nCrossServerIndex:%d\n", nLogicServerIndex); 

				//CDataPacket data(sBuffer, sizeof(sBuffer)); 
				CDataPacket &data = allocSendPacket();
				PDATAHEADER pheader;
				data.setLength(sizeof(*pheader));
				data.setPosition(sizeof(*pheader));
				data << (WORD)uParam1;
				data.writeBuf(outPacket->getOffsetPtr(), outPacket->getLength() - outPacket->getPosition());
				pheader = (PDATAHEADER)data.getMemoryPtr();
				pheader->tag = DEFAULT_TAG_VALUE;
				pheader->len = data.getLength() - sizeof(*pheader);
				SendMsg2LogicClient(nLogicServerIndex, data.getMemoryPtr(), data.getLength());
				//GetLogicServer()->GetCrossClient()->FreeBackUserDataPacket(outPacket);
			}
			CLocalCrossClient *pClient = (CLocalCrossClient *)(uParam4);
			if (pClient)
			{
				pClient->FreeBackUserDataPacket(outPacket);
			}
		}
		else if( uParam1 == jxInterSrvComm::CrossServerProto::sSendBroadTipmsg
				|| uParam1 == jxInterSrvComm::CrossServerProto::sSendBroadSysTipmsg)
		{
			//char sBuffer[1024];
			CDataPacket *outPacket = (CDataPacket*)uParam2;
			if(outPacket)
			 {
				outPacket->setPosition(0);
				//int nCrossServerIndex = 0;
				//(*outPacket) >> nCrossServerIndex;

				printf("size:%d\n", outPacket->getLength()- outPacket->getPosition()); 
   
				//CDataPacket data(sBuffer, sizeof(sBuffer)); 
				CDataPacket &data = allocSendPacket();
				PDATAHEADER pheader;
				data.setLength(sizeof(*pheader));
				data.setPosition(sizeof(*pheader));
				data << (WORD)uParam1;
				data.writeBuf(outPacket->getOffsetPtr(), outPacket->getLength() - outPacket->getPosition());
				pheader = (PDATAHEADER)data.getMemoryPtr();
				pheader->tag = DEFAULT_TAG_VALUE;
				pheader->len = data.getLength() - sizeof(*pheader); 
				SendMsg2AllClient(GameServer, data.getMemoryPtr(), data.getLength());
				//GetLogicServer()->GetCrossClient()->FreeBackUserDataPacket(outPacket);
			}
			CLocalCrossClient *pClient = (CLocalCrossClient *)(uParam4);
			if (pClient)
			{
				pClient->FreeBackUserDataPacket(outPacket);
			}
		} 
	}
}

void CCrossServer::ProcessKickCrossActor(const unsigned int nSessionId, const int nServerIndex)
{
	char sBuffer[64];
	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << 1;//(WORD)SessionServerProto::sKickCrossServerUser ;
	data << (unsigned int)nSessionId;
	
	
	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;

	OutputMsg(rmNormal,"send Kick cross server actor,accountid=%d,serverindex=%d",(int)nSessionId,(int)nServerIndex);

	SendMsg2LogicClient(nServerIndex,data.getMemoryPtr(), data.getLength());
	
}

VOID CCrossServer::SingleRun()
{
	TICKCOUNT dwCurTick = _getTickCount();

	//提交新开启的会话数据
	if ( m_SessionList.appendCount() > 0 )
	{
		m_SessionList.flush();
		TRACE(_T("当前会话数量：%d"), m_SessionList.count());
	}
	//调用父类例行执行
	Inherited::SingleRun();

}



bool CCrossServer::InitRankMsg()
{
	
	return true;
}

bool CCrossServer::SaveRankMsg()
{

	return true;
}




//初始化账户
bool CCrossServer::InitACCountId()
{
	return true;
}

PGAMECROSS CCrossServer::GetSessionPtr(const unsigned int nSessionId,PINT_PTR lpIndex ,Uint64 lKey,int nServerIndex)
{
	INT_PTR i;
	PGAMECROSS *pSessionList;

	bool bNeedCheckKey = (lKey == UINT64(-1));

	pSessionList = m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nSessionID == nSessionId && ( bNeedCheckKey==true || lKey ==pSessionList[i]->lKey ) && (nServerIndex ==-1 || nServerIndex ==pSessionList[i]->nServerIndex ))
		{
			if (lpIndex) *lpIndex = i;
			return pSessionList[i];
		}
	}
	return NULL;
}


//返回1个ip登陆了多少个号
int CCrossServer::GetLoginAccountCount(LONGLONG lIp)
{
	INT_PTR i;
	PGAMECROSS *pSessionList;

	int nCount =0;
	pSessionList = m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nIPAddr == lIp &&   pSessionList[i]->nIsRawLogin)
		{
			nCount ++;
		}
	}
	return nCount;
}


PGAMECROSS CCrossServer::GetSessionPtrByAccount(LPCSTR sAccount, PINT_PTR lpIndex)
{
	INT_PTR i;
	PGAMECROSS *pSessionList;

	pSessionList = m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( _stricmp(pSessionList[i]->sAccount, sAccount) == 0 )
		{
			if (lpIndex) *lpIndex = i;
			return pSessionList[i];
		}
	}
	return NULL;
}

BOOL CCrossServer::CloseSessionAtListIndex(const INT_PTR nIndex)
{
	unsigned int nSessionId;
	PGAMECROSS pSession;
	BOOL boResult = FALSE;
	
	//OutputMsg(rmNormal,"Current Thread =%d",(int)GetCurrentThreadId());

	// if ( m_SessionList.count() > nIndex )
	// {
	// 	boResult = TRUE;

	// 	pSession = m_SessionList[nIndex];
	// 	nSessionId = (unsigned int)pSession->nSessionID;
		
	// 	//关闭会话
	// 	OutputMsg(rmTip,_T("close Session,ID=%u,account=%s,serverin=%d"),nSessionId,pSession->sAccount,(int)pSession->nServerIndex);

	// 	//发给自己的服务器，需要删除这个会话
	// 	BroadCastCloseSession(nSessionId,pSession->lKey,pSession->nServerIndex);

		
	// 	//删除会话指针
	// 	m_SessionList.lock();
	// 	m_SessionList.remove(nIndex);
	// 	m_SessionList.unlock();
	// 	m_Allocator.FreeBuffer(pSession);
		


	return boResult;
}

INT_PTR CCrossServer::SendMsg2AllClient(const SERVERTYPE eServerType, LPCSTR sMsg, const size_t dwSize)
{
	INT_PTR i, nSendCount = 0;
	CCrossClient *pClient;

	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CCrossClient*)m_ClientList[i];
		if ( pClient && pClient->connected() && pClient->registed() )
		{
			if ( eServerType == InvalidServer || pClient->getClientType() == eServerType )
			{
				pClient->AppendSendBuffer(sMsg, dwSize );
				nSendCount++;
			}
		}
	}
	return nSendCount;
}

INT_PTR CCrossServer::SendMsg2LogicClient(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize)
{
	INT_PTR i, nSendCount = 0;
	CCrossClient *pClient;

	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CCrossClient*)m_ClientList[i];
		if ( pClient && pClient->connected() && pClient->registed() && pClient->getClientType() == GameServer && pClient->getClientServerIndex() == (int)nServerIndex)
		{
			pClient->AppendSendBuffer(sMsg, dwSize );
			nSendCount++;
		}
	}
	return nSendCount;
}

INT_PTR CCrossServer::SendGroupLogicClientMsg(const int nCommServerId, LPVOID data, const SIZE_T nSize)
{
	INT_PTR i, nSendCount = 0;
	// CCrossClient *pClient;
	// for (i = m_ClientList.count() - 1; i > -1; --i)
	// {
	// 	pClient = (CCrossClient*)m_ClientList[i];
	// 	if ( pClient && pClient->registed() 
	// 		&& GameServer == pClient->getClientType()
	// 		&& nCommServerId == pClient->GetCommonServerId())
	// 	{
	// 		pClient->AppendSendBuffer(data, nSize);
	// 		nSendCount++;
	// 	}
	// }

	return nSendCount;
}


INT_PTR CCrossServer::SendAllDBAndIndexEngineMsg(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize)
{
	INT_PTR i, nSendCount = 0;
	CCrossClient *pClient;

	m_ClientList.lock();
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CCrossClient*)m_ClientList[i];
		if ( !pClient || !pClient->connected() || !pClient->registed() )
			continue;

		if ( pClient->getClientType() == DBServer || 
			(pClient->getClientType() == GameServer && pClient->getClientServerIndex() == nServerIndex) )
		{
			pClient->AppendSendBuffer(sMsg, dwSize );
			nSendCount++;
		}
	}
	m_ClientList.unlock();
	return nSendCount;
}

INT_PTR CCrossServer::BroadCastUpdateSessionState(PGAMECROSS pSession)
{
	char sBuffer[32];
	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << 1;//(WORD)SessionServerProto::sUpdateSession ;
	data << (int)pSession->nSessionID;
	data << (int)pSession->nState;

	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;

	//向所有数据服务器发送会话状态变更消息
	INT_PTR nResult = SendMsg2AllClient(DBServer,data.getMemoryPtr(), data.getLength());

	//如果会话登录到某个逻辑服务器，则向此逻辑服务器发送会话状态变更消息
	if ( pSession->nServerIndex > 0 )
	{
		nResult += SendMsg2LogicClient(pSession->nServerIndex,data.getMemoryPtr(), data.getLength());
	}

	return nResult;
}

INT_PTR CCrossServer::BroadCastCloseSession(const UINT64 nSessionId,Uint64 lKey,const int nServerIndex)
{
	char sBuffer[64];
	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	//关闭号
	data.setPosition(sizeof(*pheader));
	data << 1;//(WORD)SessionServerProto::sCloseSession ;
	data <<(unsigned int) nSessionId;
	data <<(Uint64) lKey;

	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;
	//向所有数据服务器、引擎服务器发送删除会话消息
	
	if(nServerIndex >0 )
	{
		return SendMsg2LogicClient(nServerIndex, data.getMemoryPtr(), data.getLength()) ;
	}
	else
	{
		return SendMsg2AllClient(LogServer, data.getMemoryPtr(), data.getLength() );
	}
	
	//return  SendAllClientMsg(InvalidServer, data.getMemoryPtr(), data.getLength());
}

INT_PTR CCrossServer::BroadCastSessionConfim(const UINT64 nSessionId, const INT_PTR nServerIndex)
{

	INT_PTR nResult =0;
	char sBuffer[32];

	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << 1;//(WORD)SessionServerProto::sQuerySessionExist ;
	data << (unsigned int)nSessionId;

	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;
	
	//如果nServerIndex为0，则向所有数据服务器、引擎服务器发送确认会话是否在线的消息
	if ( !nServerIndex )
	{
		nResult = SendMsg2AllClient(InvalidServer, data.getMemoryPtr(), data.getLength());
	}
	else 
	{
		nResult = SendAllDBAndIndexEngineMsg(nServerIndex, data.getMemoryPtr(), data.getLength());
	}

	return nResult;
}

//踢掉在线的用户
VOID CCrossServer::PostKickCrossActor(const INT_PTR nSessionId, const int nServerIndex)
{
	PostInternalMessage(CSIM_KICK_ONLINE_CROSSSERVER_ACTOR, nSessionId,nServerIndex,0);
}



VOID CCrossServer::PostAllClientMsg(const SERVERTYPE eServerType, LPCSTR sMsg)
{
	size_t dwSize = strlen(sMsg);
	LPVOID lpData = m_Allocator.AllocBuffer(dwSize);
	memcpy(lpData, sMsg, dwSize);
	PostInternalMessage(CSIM_POST_ALL_CLIENT_MSG, eServerType, (UINT_PTR)lpData, dwSize);
}

BOOL CCrossServer::GetSession(const INT_PTR nSessionId, OUT PGAMECROSS pSession,int nServerIndex)
{
	BOOL boResult = FALSE;
	CSafeLock sl(&m_SessionListLock);

	PGAMECROSS pSessionPtr = GetSessionPtr((unsigned int)nSessionId, NULL,Uint64(-1),nServerIndex);
	if ( pSessionPtr )
	{
		*pSession = *pSessionPtr;
		boResult = TRUE;
	}

	return boResult;
}

PGAMECROSS CCrossServer::GetSpecialServerSession(const INT_PTR nSessionId, const int nServerIndex)
{
	INT_PTR i;
	PGAMECROSS *pSessionList= m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nSessionID == nSessionId && pSessionList[i]->nServerIndex == nServerIndex)
		{
			return pSessionList[i];
			
		}
	}
	return NULL;
}
//logic server count
INT_PTR CCrossServer::GetLogicClientCount(const INT_PTR nServerIndex)
{
	OutputMsg(rmTip,_T("GetLogicClientCount:%d"),nServerIndex);

	if ( !nServerIndex )
		return m_nEngineClientCount;

	INT_PTR i, nResult = 0;
	CCrossClient *pClient;

	m_ClientList.lock();
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CCrossClient*)m_ClientList[i];
		////debug
		//if (pClient)
		//{
		//	OutputMsg(rmTip,_T("GetLogicClientCount pClient:connected=%d,registed=%d,clienttype=%d,serverindex=%d,clientName=%s"),
		//		(int)(pClient->connected()),(int)(pClient->registed()),pClient->getClientType(),
		//		pClient->getClientServerIndex(),pClient->getClientName());
		//}
		////end debug
		if ( pClient && pClient->connected() && pClient->registed() 
			&& pClient->getClientType() == GameServer && pClient->getClientServerIndex() == nServerIndex )
		{
			nResult ++;
			//OutputMsg(rmTip,_T("--------------------------%d"),pClient->getClientServerIndex());
			////debug
			//if (pClient)
			//{
			//	OutputMsg(rmTip,_T("GetLogicClientCount pClient2:connected=%d,registed=%d,clienttype=%d,serverindex=%d,clientName=%s"),
			//		(int)(pClient->connected()),(int)(pClient->registed()),pClient->getClientType(),
			//		pClient->getClientServerIndex(),pClient->getClientName());
			//}
			////end debug
		}
	}
	m_ClientList.unlock();

	return nResult;
}

INT_PTR CCrossServer::UserCanLoginToServer(const INT_PTR nServerIndex)
{
	INT_PTR i, nResult = -1;
	CCrossClient *pClient;

	// m_ClientList.lock();
	// for ( i=m_ClientList.count() - 1; i>-1; --i )
	// {
	// 	pClient = (CCrossClient*)m_ClientList[i];
	// 	if ( pClient && pClient->connected() && pClient->registed() 
	// 		&& pClient->getClientType() == GameServer && pClient->getClientServerIndex() == nServerIndex )
	// 	{
	// 		//判断服务器人员是否已满,
	// 		if ( pClient->m_nUserCount < pClient->m_nUserLimit )
	// 			nResult = 0;
	// 		else nResult = -1;
	// 		break;
	// 	}
	// }
	// m_ClientList.unlock();

	return nResult;
}

//此函数是被网关线程调用的
BOOL CCrossServer::PostOpenSession(const INT_PTR nServerIndex, 
									 const INT_PTR nRawServerIndex,
									 LPCSTR sAccount,
									 const unsigned int nSessionId,
									 const INT_PTR nIPAddr, 
									 const INT_PTR dwTodayOnlineSec,
									 INT_PTR nGmLevel,
									 UINT64  lKey ,
									 INT_PTR  nGateIndex, 
									 bool   bIsWhiteLogin,
									 bool	bCommonMsg
									 )
{	
	char sBuffer[256];
	
	PGAMECROSS pSession;
	BOOL boNewSession = FALSE;
	
	m_SessionList.lock();


	pSession = GetSessionPtr(nSessionId);
	m_SessionList.unlock();
	if ( !pSession )
	//每次都去new一个
	{
		pSession = (PGAMECROSS)m_Allocator.AllocBuffer(sizeof(*pSession));
		boNewSession = TRUE;
	}

	
	ZeroMemory(pSession, sizeof(*pSession));
	pSession->nSessionID = nSessionId;
	pSession->nServerIndex = (int)nServerIndex;
	
	if(nServerIndex == nRawServerIndex)
	{
		pSession->nIsRawLogin =1 ; //原来的服务器ID
	}
	else
	{
		pSession->nIsRawLogin =0;
	}
	
	pSession->nIPAddr = nIPAddr;
	//会话建立的时候就标记为等待查询角色状态,这个时候将连接数据服务器
	pSession->nState = gsWaitQueryChar; 
	pSession->dwSessionTick = _timeGetTime();
	_asncpytA(pSession->sAccount, sAccount);

	//追加会话数据
	if ( boNewSession )
	{
		m_SessionList.append(pSession);
	}
	//向日志服务器记录登录日志，格式为：USERID/ACCOUNT/GAMETYPE/IPADDR/CHARNAME

	//广播消息	
	GLOBALSESSIONOPENDATA SessionData ;
	SessionData.nSessionId = (INT)nSessionId;
	SessionData.nServerIndex = (INT)nServerIndex; //?	
	SessionData.nRawServerId = (INT)nRawServerIndex;
	SessionData.nClientIPAddr = (LONGLONG)nIPAddr;
	SessionData.dwFCMOnlineSec = (DWORD)dwTodayOnlineSec;
	SessionData.nGmLevel =(int) nGmLevel; //GM等级
	SessionData.lKey =lKey; 
	SessionData.nGateIndex = (int)nGateIndex; //在服务器的index

	SessionData.eState =(GSSTATE) (bIsWhiteLogin?0:1);
	_asncpytA(SessionData.sAccount, sAccount);

	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << 1;//(WORD)SessionServerProto::sOpenSession  ; //cmd
	data << SessionData; //数据
	
	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;

	if(bCommonMsg == false)
	{
		//只向特定的逻辑服发送
		SendMsg2LogicClient(nServerIndex, data.getMemoryPtr(), data.getLength());
	}

	return TRUE;
}


VOID CCrossServer::PostCloseSessionByAccountId(Uint64 nAccountId,UINT64 lKey)
{
	PostInternalMessage(CSIM_CLOSE_SESSION_BY_ACCOUNTID, nAccountId, lKey,0,0);
}

VOID CCrossServer::PostCloseSessionByAccount(const LPCSTR sAccount)
{
	LPSTR sAccountStr = (LPSTR)m_Allocator.AllocBuffer(sizeof(sAccountStr[0]) * (strlen(sAccount) + 1));
	strcpy(sAccountStr, sAccount);
	PostInternalMessage(CSIM_CLOSE_SESSION_BY_ACCOUNT, (UINT_PTR)sAccountStr, 0, 0);
}

VOID CCrossServer::EnumConnections(EnumConnectionFn lpFn, LPCVOID lpParam)
{
	CCrossClient *pClient;
	UINT uRetCode;
	m_ClientList.lock();
	for ( INT_PTR i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CCrossClient*)m_ClientList[i];
		if ( pClient && pClient->connected())
		{
			uRetCode = lpFn(lpParam, pClient);
			if ( uRetCode )
				break;
		}
	}
	m_ClientList.unlock();
}



/*
* Comments:  解封ip
* Parameter: long long lIp:
* @Return  void:
*/

long long  CCrossServer::GetIntIp(char *sIp)
{
	
}

void CCrossServer::OnSendPlatformResultToClient(int nServerIndex,CDataPacketReader &inPacket)
{
	CCrossClient *pClient = 0;
	for (INT_PTR i= m_ClientList.count() - 1; i > -1; --i)
	{
		pClient = (CCrossClient*)m_ClientList[i];
		if (pClient && pClient->connected() && pClient->getClientType() == GameServer)
		{
			if(pClient->GetServerId() == nServerIndex)
			{
				pClient->OnSendReqCommonPlatformResult(inPacket);
				return;
			}
		}
	}
}
