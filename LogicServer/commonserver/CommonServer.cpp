#include "StdAfx.h"
//暂时作废

CCommonServer::CCommonServer() : Inherited()
{
	/*m_SessionList.setLock(&m_SessionListLock);*/
	SetServiceName(_T("普通逻辑服务器Server"));
}

CCommonServer::~CCommonServer()
{
}

CCustomServerClientSocket* CCommonServer::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{	
	OutputMsg(rmTip, _T("接收普通逻辑服务器连接"));
	return new CCommonServerClient(this, nSocket, pAddrIn);
}

VOID CCommonServer::DestroyClientSocket(CCustomServerClientSocket *pClientSocket)
{	
	//m_CloseClientList.add(pClientSocket);
	OutputMsg(rmTip, _T("断开普通逻辑服务器连接"));
	Inherited::DestroyClientSocket(pClientSocket);	
}

VOID CCommonServer::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	/*switch(uMsg)
	{		
	}*/
}

VOID CCommonServer::ProcessClients()
{
	Inherited::ProcessClients();
	ProcessClosedClients();
}

VOID CCommonServer::SingleRun()
{
	TICKCOUNT dwCurTick = _getTickCount();

	////提交新开启的会话数据
	//if ( m_SessionList.appendCount() > 0 )
	//{
	//	m_SessionList.flush();
	//	TRACE(_T("当前会话数量：%d"), m_SessionList.count());
	//}

	//调用父类例行执行
	Inherited::SingleRun();
}

VOID CCommonServer::ProcessClosedClients()
{
	//INT_PTR i;
	//CDBDataClient *pClient;
	//TICKCOUNT dwCurTick = _getTickCount();

	////必须降序循环，因为连接可能在循环中被移除
	//for (i = m_CloseClientList.count() - 1; i > -1; --i)
	//{
	//	pClient = m_CloseClientList[i];
	//	//连接被关闭后依然要调用Run函数，因为可能有重要的网络数据或逻辑数据没有处理完
	//	pClient->Run();
	//	//连接关闭5分钟后再释放连接对象
	//	if ( dwCurTick - pClient->m_dwClosedTick >= 5 * 60 * 1000 )
	//	{
	//		m_CloseClientList.remove(i);
	//		delete pClient;
	//	}
	//}
}

VOID CCommonServer::OnRun()
{

}

void CCommonServer::SendData(int nServerIndex, const void *pData, const size_t nLen)
{	
	static char szData[81960];
	if (nLen + sizeof(DATAHEADER) > sizeof(szData))
	{
		OutputMsg(rmError, _T("%s send too big packet(len=%d)"), __FUNCTION__, nLen);
		return;
	}

	// INT_PTR i, nSendCount = 0;
	CCommonServerClient *pClient;
	for (INT_PTR i = m_ClientList.count() - 1; i > -1; --i)
	{
		pClient = (CCommonServerClient *)m_ClientList[i];
		if (pClient && pClient->connected() && pClient->registed() && pClient->getClientType() == GameServer)
		{			
			// 封装一个消息头进去
			/*PDATAHEADER pHeader;
			ZeroMemory(pHeader, sizeof(0));
			pHeader->tag = DEFAULT_TAG_VALUE;
			CopyMemory((void *)(pHeader+1), pData, nLen);
			pClient->AppendSendBuffer(pHeader, nLen + sizeof(DATAHEADER));*/
			return;
		}
	}

	return;
}

//
//BOOL CCommonServer::GetSession(const INT_PTR nSessionId, OUT PGAMESESSION pSession)
//{
//	BOOL boResult = FALSE;
//	CSafeLock sl(&m_SessionListLock);
//
//	PGAMESESSION pSessionPtr = GetSessionPtr(nSessionId, NULL);
//	if ( pSessionPtr )
//	{
//		*pSession = *pSessionPtr;
//		boResult = TRUE;
//	}
//
//	return boResult;
//}
//
//PGAMESESSION CCommonServer::GetSessionPtr(const INT_PTR nSessionId, PINT_PTR lpIndex)
//{
//	INT_PTR i;
//	PGAMESESSION *pSessionList;
//
//	pSessionList = m_SessionList;
//	for (i= m_SessionList.count() - 1; i > -1; --i)
//	{
//		if ( pSessionList[i]->nSessionID == nSessionId )
//		{
//			if (lpIndex) *lpIndex = i;
//			return pSessionList[i];
//		}
//	}
//
//	return NULL;
//}