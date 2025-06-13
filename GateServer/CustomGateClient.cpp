#include "StdAfx.h"

CCustomGateClient::CCustomGateClient()
{
	m_szSrvHost[0]		= _T('\0');
	m_szClientName[0]	= _T('\0');
	m_nSrvPort			= 0;
	m_bStoped			= false;
	m_bConnected		= false;
	m_hConnectThread = INVALID_HANDLE_VALUE;
	m_hSendDataThread = INVALID_HANDLE_VALUE;
	m_hRecvDataThread = INVALID_HANDLE_VALUE;
	m_socket = INVALID_SOCKET;
	m_SendMsgList.setLock(&m_SendMsgLock);
	m_FreeMsgList.setLock(&m_FreeMsgLock);
	m_RecvMsgList.setLock(&m_RecvMsgLock);
	m_RecvFreeMsgList.setLock(&m_RecvFreeMsgLock);
	ZeroMemory(&m_RecvMsg, sizeof(m_RecvMsg));
	m_SendBigPacket.setAllocator(&m_Allocator);
}

CCustomGateClient::~CCustomGateClient()
{
	
	Stop();
	UnintSocketLib();	
	// 删除所有的数据包
	DestroyPacketList(m_SendingPacketList);
	DestroyPacketList(m_FreeMsgList);
	DestroyPacketList(m_RecvMsgList);
	DestroyPacketList(m_RecvFreeMsgList);
}

bool CCustomGateClient::Startup()
{
	OutputMsg(rmTip, _T("CustomGateClient[%s] Start..."), m_szClientName);
	Stop();

	if (!InitSockLib())
		return false;

	m_bStoped = false;
	// 创建连接线程
#ifdef _MSC_VER
	m_hConnectThread = ::CreateThread(NULL, 0, staticConnectThreadRountine, this, 0, NULL);
#else
	pthread_create(&m_hConnectThread, NULL, staticConnectThreadRountine, this);
#endif
	if (m_hConnectThread != INVALID_HANDLE_VALUE)
	{
#ifdef _MSC_VER
		CloseHandle(m_hConnectThread);
#endif
	}
	else
	{
		OutputMsg(rmError, _T("%s Create Connect Thread Failed[errcode:%d]"), __FUNCTION__, GetLastError());
		return false;
	}
	
	// 创建发送线程
#ifdef _MSC_VER
	m_hSendDataThread = ::CreateThread(NULL, 0, staticSendDataThreadRountine, this, 0, NULL);
#else
	pthread_create(&m_hSendDataThread, NULL, staticSendDataThreadRountine, this);
#endif
	if (m_hSendDataThread != INVALID_HANDLE_VALUE)
	{
#ifdef _MSC_VER
		CloseHandle(m_hSendDataThread);
#endif
	}
	else
	{
		OutputMsg(rmError, _T("%s Create SendData Thread Failed[errcode:%d]"), __FUNCTION__, GetLastError());
		return false;
	}

	// 创建接收数据线程
#ifdef _MSC_VER
	m_hRecvDataThread = ::CreateThread(NULL, 0, staticRecvDataThreadRountine, this, 0, NULL);
#else
	pthread_create(&m_hRecvDataThread, NULL, staticRecvDataThreadRountine, this);
#endif
	if (m_hRecvDataThread != INVALID_HANDLE_VALUE)
	{
#ifdef _MSC_VER
		CloseHandle(m_hRecvDataThread);
#endif
	}
	else
	{
		OutputMsg(rmError, _T("%s Create SendData Thread Failed[errcode:%d]"), __FUNCTION__, GetLastError());
		return false;
	}

	return true;
}

void CCustomGateClient::Stop()
{
	m_bStoped = true;
	Close();
	Sleep(3000);
}

void CCustomGateClient::Close()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	if (m_bConnected)
	{
		m_bConnected = false;
		// 清空缓存数据数据
		m_RecvMsgList.flush();
		m_RecvFreeMsgList.appendArray(m_RecvMsgList, m_RecvMsgList.count());
		m_RecvMsgList.trunc(0);
		m_SendingPacketList.flush();
		m_FreeMsgList.appendArray(m_SendingPacketList, m_SendingPacketList.count());
		m_SendingPacketList.trunc(0);
		OutputMsg(rmTip, _T("与[%s]连接断开..."), m_szClientName);
	}
	OnClosed();	
}

void CCustomGateClient::SendRegisteClient()
{

}

void CCustomGateClient::SendKeepAlive()
{

}

void CCustomGateClient::SendDataRoutine()
{
	while (!m_bStoped)
	{			
		// 如果已经连接上，发送数据
		if (m_bConnected)  
		{
			TICKCOUNT nStartTick = _getTickCount();
			m_SendMsgList.flush();
			INT_PTR nCount = m_SendMsgList.count();	
			if (nCount > 0)
			{
				for (INT_PTR i = 0; i < nCount; i++)
				{
					CDataPacket* packet = m_SendMsgList[i];
					size_t nAvailableLen = packet->getAvaliableLength();				
					if (nAvailableLen > 0)
					{
						//if (!SendPointedData(packet->getOffsetPtr(), nAvailableLen)) // 发送数据出错，退出
						//	break;
						m_SendBigPacket.writeBuf(packet->getOffsetPtr(), nAvailableLen);
					}
				}

				// 采取打包的方式发送给服务器
				m_SendBigPacket.setPosition(0);
				INT_PTR nAvailableLen = m_SendBigPacket.getAvaliableLength();
				if (nAvailableLen > 0)
				{
					SendPointedData(m_SendBigPacket.getOffsetPtr(), nAvailableLen);
					m_SendBigPacket.setLength(0);
				}

				m_FreeMsgList.appendArray(m_SendMsgList, nCount);
				m_SendMsgList.trunc(0);
			}		

			Sleep(3);
		}	
		else
		{
			Sleep(100);		
		}
	}
}

bool CCustomGateClient::SendPointedData(const char* pData, size_t nLen)
{	
	DECLARE_FUN_TIME_PROF()
	int nRet;
	while (nLen > 0)
	{
		nRet = ::send(m_socket, pData, (int)nLen, 0);
		if (SOCKET_ERROR == nRet)
		{			
			Close();
			return false;
		}
		pData += nRet;
		nLen -= nRet;
	}
	return true;
}

void CCustomGateClient::RecvDataRoutine()
{	
	nPacketCount = 0;
	while (!m_bStoped)
	{
		if (!m_bConnected)
		{
			Sleep(20);
			continue;
		}
				
		// 读取消息头
		bool bRet = ReadPointedData((char *)&(m_RecvMsg.header), sizeof(GATEMSGHDR));
		if (!bRet)
		{
			OutputMsg(rmError, _T("Recv MsgHeader failed req=%d"), sizeof(GATEMSGHDR));
			continue;
		}		

		// 读取消息体
		int nLen = m_RecvMsg.header.nDataSize;
		if (m_RecvMsg.nBuffLen < nLen)
		{	
			if (m_RecvMsg.data != NULL)
				m_Allocator.FreeBuffer(m_RecvMsg.data);
			m_RecvMsg.data = (char *)m_Allocator.AllocBuffer(nLen);
			m_RecvMsg.nBuffLen = nLen;
		}
		
		bRet = ReadPointedData(m_RecvMsg.data, nLen);
		if (!bRet)
		{
			OutputMsg(rmError, _T("Recv Msg Data Failed Req=%d"), nLen);
			continue;
		}		

		nPacketCount++;	

		// 拷贝数据到接收队列中, 不直接处理数据
		CDataPacket* packet = AllocRecvPacket();
		*packet << m_RecvMsg.header;
		if (nLen > 0)
			packet->writeBuf(m_RecvMsg.data, nLen);
		*packet << (int)0xeeff;
		packet->setPosition(0);
		m_RecvMsgList.append(packet);

#ifdef _DEBUG
		memset(&m_RecvMsg, 0, sizeof(m_RecvMsg));
#endif
		//ProcessRecvBuffers(&m_RecvMsg);
	}
}

bool CCustomGateClient::ReadPointedData(char* pBuff, size_t len)
{
	DECLARE_FUN_TIME_PROF()
	int nBytesRead;	
	while (len)
	{
		nBytesRead = recv(m_socket, pBuff, (int)len, 0);
		if (nBytesRead <= 0)
		{
			Close();			
			return false;
		}
		pBuff += nBytesRead;
		len -= nBytesRead;
	}

	return true;
}

void CCustomGateClient::PutMessage(int nIdent, 
								  int nSessionIdx, 
								  SOCKET nSocket, 
								  int nServerIdx, 
								  unsigned long long param, 
								  char *pBuffer, 
								  int nBufSize)
{
	DECLARE_FUN_TIME_PROF()
	if (!m_bConnected)
		return;

	//这个是消息头
	RUNGATEMSGHDR Hdr;
	Hdr.dwGateCode	= RUNGATECODE;
	Hdr.nSocket		= nSocket;
	Hdr.wSessionIdx = nSessionIdx;
	Hdr.wIdent		= nIdent;
	Hdr.wServerIdx	= nServerIdx;
	Hdr.nDataSize	= nBufSize;
	Hdr.tickCount	= param + _getTickCount();
	SIZE_T headSize =  sizeof(Hdr);
	CDataPacket* packet = AllocPacket();
	if (packet->getPosition() != 0)
	{
		OutputMsg(rmError, _T("-------------------------------"));
	}
	if (packet->getAvaliableLength() != 0)
	{
		OutputMsg(rmError, _T("--------------|||-----------------"));
	}

	packet->operator<<(Hdr);
	if (nBufSize > 0)
	{
		packet->writeBuf(pBuffer, nBufSize);		
	}
	packet->setPosition(0);	
	if (packet->getAvaliableLength() != sizeof(Hdr) + nBufSize)
	{
		OutputMsg(rmError, _T("--------------packet error[%d]--------------"), nBufSize);
	}
	m_SendMsgList.append(packet);
}

CDataPacket* CCustomGateClient::AllocPacket()
{	
	if (m_FreeMsgList.count() > 0)
	{
		CDataPacket* pkg = m_FreeMsgList.pop();		
		pkg->setLength(0);		
		return pkg;
	}

	if (m_FreeMsgList.appendCount() > 0)
		m_FreeMsgList.flush();
	if (m_FreeMsgList.count() <= 0 )
		allocSendPacketList(m_FreeMsgList, 512);
	CDataPacket* pkg = m_FreeMsgList.pop();
	pkg->setLength(0);
	return pkg;
}

CDataPacket* CCustomGateClient::AllocRecvPacket()
{
	if (m_RecvFreeMsgList.count() > 0)
	{
		CDataPacket* pkg = m_RecvFreeMsgList.pop();		
		pkg->setLength(0);		
		return pkg;
	}

	if (m_RecvFreeMsgList.appendCount() > 0)
		m_RecvFreeMsgList.flush();
	if (m_RecvFreeMsgList.count() <= 0 )
		allocSendPacketList(m_RecvFreeMsgList, 512);
	CDataPacket* pkg = m_RecvFreeMsgList.pop();
	pkg->setLength(0);
	return pkg;
}

bool CCustomGateClient::CreateSocket()
{
	if (m_socket != INVALID_SOCKET)
	{
		OutputMsg(rmError, _T("Socket is createdc already"));
		return false;
	}

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	return m_socket != INVALID_SOCKET ? true : false;
}

bool CCustomGateClient::ConnectToServerImpl()
{
	int nErr;
	TICKCOUNT dwCurTick = _getTickCount();

	//创建套接字
	if (INVALID_SOCKET == m_socket)
	{
		if (!CreateSocket())
		{
			OutputMsg(rmError, _T("[%s]Create Socket Failed"), __FUNCTION__);
			return false;
		}
	}
		
	hostent *phost;
#ifdef UNICODE
	wylib::string::CWideString ws(addr);
	wylib::string::CAnsiString *as = ws.toAStr();
	phost = gethostbyname(*as);	
	delete as;
#else
	phost = gethostbyname(m_szSrvHost);
#endif
	if (phost)
	{
		in_addr addr;
		addr.s_addr = *(u_long*)phost->h_addr_list[0];
		SOCKADDR_IN addrin;
		ZeroMemory(&addrin, sizeof(addrin));
		addrin.sin_family = AF_INET;
		addrin.sin_addr.s_addr = addr.s_addr;
		addrin.sin_port = (htons((u_short)m_nSrvPort));
		nErr = ::connect(m_socket, (sockaddr*)&addrin, sizeof(addrin));
		if ( nErr == 0 )
		{			
			OnConnected();
			return true;
		}		
		else
		{
			OutputMsg(rmNormal, _T("连接到%s服务器失败..."), m_szClientName);
			return false;
		}
	}
	else
	{
		OutputMsg(rmError, _T("GetHostByName resolve failed"));
		return false;
	}

	return true;
}

void CCustomGateClient::OnConnected()
{
	m_bConnected = true;
}

void CCustomGateClient::OnClosed()
{
	
}

void CCustomGateClient::ProcessRecvDataImpl()
{
	DECLARE_FUN_TIME_PROF()
	m_RecvMsgList.flush();
	CDataPacket* packet;
	INT_PTR nCount = m_RecvMsgList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		packet = m_RecvMsgList[i];				
		GATEMSGHDR* pHeader = (GATEMSGHDR *)packet->getMemoryPtr();
		ProcessRecvBuffers(pHeader, (LPCSTR)(pHeader+1), pHeader->nDataSize);
		int *pVerify = (int *)((char *)pHeader + sizeof(GATEMSGHDR) + pHeader->nDataSize);
		Assert(*pVerify == 0xeeff);
	}
	m_RecvFreeMsgList.appendArray(m_RecvMsgList, m_RecvMsgList.count());
	m_RecvMsgList.trunc(0);		
}

void CCustomGateClient::ConnectToServerRoutine()
{
	TICKCOUNT dwReconnectTick = _getTickCount();
	while (!m_bStoped)
	{
		if (m_bConnected)
		{			
			ProcessRecvDataImpl();
			Sleep(3);
		}
		else
		{
			// Try Connect To Server
			if (_getTickCount() >= dwReconnectTick)
			{			
				if (ConnectToServerImpl())
				{
					OutputMsg(rmNormal, _T("连接到服务器[%s]成功"), m_szClientName);
				}
				dwReconnectTick = _getTickCount() + 5000;
			}

			Sleep(200);
		}
	}

	return;
}

void CCustomGateClient::SingleRun()
{

}

bool CCustomGateClient::InitSockLib()
{
#ifdef WIN32
	WSADATA wsaData;
	int nErr = WSAStartup(0x0202, &wsaData);
	if (nErr != 0)
	{
		OutputMsg(rmError, _T("%s Init Socket Lib Failed, errcode=%d"), __FUNCTION__, nErr);
		return false;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
	{
		OutputMsg(rmError, "Could not find a usable version[2.2] of Winsock.dll\n");
		return false;
	}
#endif
	return true;
}

void CCustomGateClient::UnintSocketLib()
{
#ifdef WIN32
	WSACleanup();
#endif
}

#ifdef _MSC_VER
	DWORD CCustomGateClient::staticConnectThreadRountine(LPVOID pParam)
#else
	void* CCustomGateClient::staticConnectThreadRountine(void* pParam)
#endif
{
	CCustomGateClient* pThis = (CCustomGateClient *)pParam;
	if (pThis)
		pThis->ConnectToServerRoutine();

	return 0;
}

#ifdef _MSC_VER
	DWORD CCustomGateClient::staticSendDataThreadRountine(LPVOID pParam)
#else
	void* CCustomGateClient::staticSendDataThreadRountine(void* pParam)
#endif
{
	CCustomGateClient* pThis = (CCustomGateClient *)pParam;
	if (pThis)
		pThis->SendDataRoutine();

	return 0;
}

#ifdef _MSC_VER
	DWORD CCustomGateClient::staticRecvDataThreadRountine(LPVOID pParam)
#else
	void* CCustomGateClient::staticRecvDataThreadRountine(void* pParam)
#endif
{
	CCustomGateClient* pThis = (CCustomGateClient *)pParam;
	if (pThis)
		pThis->RecvDataRoutine();
	return 0;
}

void CCustomGateClient::SetClientName(LPCTSTR sClientName)
{
	_tcsncpy(m_szClientName, sClientName, ArrayCount(m_szClientName) - 1);
	m_szClientName[ArrayCount(m_szClientName) - 1] = 0;
}

void CCustomGateClient::SetServerHost(LPCTSTR sHost)
{
	_tcsncpy(m_szSrvHost, sHost, ArrayCount(m_szSrvHost) - 1);
	m_szSrvHost[ArrayCount(m_szSrvHost) - 1] = 0;
}

void CCustomGateClient::SetServerPort(const INT_PTR nPort)
{
	m_nSrvPort = nPort;
}

void CCustomGateClient::Dump()
{	
	OutputMsg(rmTip, _T("等待发送数据包数量: %d个\r\n"), (int)(m_SendMsgList.count() + m_SendMsgList.appendCount()));	
}

void CCustomGateClient::DestroyPacketList(CQueueList<CDataPacket*>& pkgList)
{
	if (pkgList.appendCount() > 0)
		pkgList.flush();

	for (INT_PTR i = pkgList.count()-1; i >= 0; i--)
	{
		CDataPacket* pkg = pkgList[i];
		pkg->~CDataPacket();
	}
	pkgList.clear();
}
