/*模拟登陆的客户端连接
* 模拟客户端的连接，发起多个连接到服务器去
*每个客户端都需要维护一个当前的状态机
*/


#include "stdafx.h"

void CRobotClient::ProcData()
{
	if ( !connected() )
	{
		return ;
	}
	ReadSocket();
	//ProcessRecvBuffers(getRecvBuffer());
}
//处理接受到的服务器数据包
 VOID CRobotClient::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
 {
	 //如果连接已断开则丢弃所有数据
	
	 if ( pDataBuffer->nOffset <= 0 )
	 {
		 SwapRecvProcessBuffers();
		 return;
	 }

	 //刚发了到服务器
	
	 //jxSrvDef::INTERSRVCMD nCmd;
	 INT_PTR dwRemainSize;
	 PDATAHEADER pPackHdr;
	 char* pDataEnd = pDataBuffer->pBuffer + pDataBuffer->nOffset;

	 while ( TRUE )
	 {
		 //在连接的过程中可能关闭连接，导致循环里的数据清除，如果没有退出将导致问题
		 if(pDataBuffer->nOffset ==0)
		 {
			 break;
		 }
		 dwRemainSize = (INT_PTR)(pDataEnd - pDataBuffer->pPointer);
		 
		 //如果当前在获取密钥的状态,这个时候服务器将发送unsigned int 数值的服务器的密钥过来
		 if(m_nCurrentStatus == eStatusEncryReq ) 
		 {
			 if( dwRemainSize >= sizeof(short) )
			 {
				 m_nKey = *(unsigned short *)(pDataBuffer->pPointer);
				 pDataBuffer->pPointer += sizeof(unsigned short);
				 m_nCurrentStatus = eStatusCommunication; //处于通信状态
				 
				 CAgent *pAgent = GetCurrentAgent();
				 if(pAgent)
				 {
					 pAgent->OnConnected(); //已经连接上了服务器，基本校验通过
				 }
				 break;
			 }
			 else
			 {
				 OutputMsg(rmError,_T("Recv data leng =%d, may be error"),dwRemainSize);
				 //SwapRecvProcessBuffers();
				 break;
			 }
 
		 }
		 else if( m_nCurrentStatus != eStatusCommunication ) //不在通信状态收到包的话，将不处理
		 {
			 //SwapRecvProcessBuffers();
			 break;
		 }

		 //如果缓冲区中的剩余长度小于通信协议头的长度，则交换缓冲并在以后继续进行处理
		 if ( dwRemainSize < sizeof(*pPackHdr) )
		 {
			 //SwapRecvProcessBuffers();
			 break;
		 }

		 pPackHdr = (PDATAHEADER)pDataBuffer->pPointer;
		 //检查包头标志是否有效，如果包头标志无效则需要遍历数据包查找包头
		 if ( pPackHdr->tag != m_nKey )
		 {
			 char* sCurPtr = pDataBuffer->pPointer;
			 do 
			 {
				 pDataBuffer->pPointer++;
				 pPackHdr = (PDATAHEADER)pDataBuffer->pPointer;
				 //找到包头标记则终止查找
				 if ( pPackHdr->tag == m_nKey )
					 break;
			 } 
			 while (pDataBuffer->pPointer < pDataEnd - 1);
			 //如果无法查找到包头，则保留接收缓冲末尾的最后一个字符并交换接收/处理缓冲以便下次继续连接新接收的数据后重新查找包头
			 if ( pPackHdr->tag != m_nKey )
			 {
				 //SwapRecvProcessBuffers();
				 //找不到协议头标志，输出错误消息
				 OutputMsg(rmError, _T(" recv invalid server data, proto header can not be found"));
				 break;
			 }
			 //输出找到包头的消息
			 OutputMsg(rmError, _T( "recv invalid server data, proto header refound after %d bytes"), 
				 (int)(pDataBuffer->pPointer - sCurPtr));
		 }

		 //如果处理接收数据的缓冲中的剩余数据长度不足协议头中的数据长度，则交换缓冲并在下次继续处理
		 dwRemainSize -= sizeof(*pPackHdr);
		 if ( pPackHdr->len > dwRemainSize )
		 {
			 //SwapRecvProcessBuffers();
			 break;
		 }
		 //将缓冲读取指针调整到下一个通信数据包的位置
		 pDataBuffer->pPointer += sizeof(*pPackHdr) + pPackHdr->len;
		 if ( pPackHdr->len >= 2 )
		 {
			 //将通信数据段保存在packet中
			 CDataPacketReader packet(pPackHdr + 1, pPackHdr->len);
			 CAgent *pAgent = GetCurrentAgent();
			 if(pAgent)
			 {
				 pAgent->OnRecv(packet);
			 }
		 }
		 else
		 {
			 OutputMsg(rmError, _T("recv invalid server packet, packet size to few(%d)"), 
				pPackHdr->len);
		 }
	 }
	 SwapRecvProcessBuffers();
 }


 CAgent * CRobotClient::GetCurrentAgent() 
 {
	 //分派数据包处理
	 switch(m_nCurrentServer)
	 {
	 case eServerLogic:
		 return &m_logicAgent;
		 break;
	 default:
		 return NULL;
	 }
 }
//进行内部消息的处理分派
 VOID CRobotClient::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
 {
	 Inherited::DispatchInternalMessage(uMsg,uParam1,uParam2,uParam3,uParam4);
 }
//每次逻辑循环的例行逻辑处理函数
VOID CRobotClient::OnRun()
{
	Inherited::OnRun();
	if( connected() )
	{
		if(m_nCurrentServer ==eServerLogic)
		{
			m_CurTick = _getTickCount();
			m_logicAgent.OnTimeRun(m_CurTick);
			Sleep(10);
		}
	}
	
}

//例行执行函数
VOID CRobotClient::SingleRun()
{
	Inherited::SingleRun();
}
 VOID CRobotClient::OnError(INT errorCode)
{
	Inherited::OnError(errorCode);
	
	OutputError( errorCode, _T("%s Socket error"),m_account.name);
}

VOID CRobotClient::OnDisconnected()
{
	//OutputMsg(rmNormal,_T("Dis connect"));
	if(m_nCurrentStatus == eStatusEncryReq)
	{
		m_nCurrentStatus = eStatusEncryReqDisConnect; //断开连接了
	}
	else if(m_nCurrentStatus == eStatusCommunication)
	{
		m_nCurrentStatus = eStatusCommuDisConnect;
	}
	else
	{
		OutputMsg(rmWaning,_T("error status=%d disconnect"),m_nCurrentStatus);
	}

	CAgent * pAgent = GetCurrentAgent();
	if(pAgent)
	{
		pAgent->OnDisConnected();
	}
	
}

void CRobotClient::ConnectNextServer()
{
	switch(m_nCurrentServer)
	{
	case eServerNone: //下一个要连接的是session
		m_nCurrentServer = eServerLogic;
		break;

	default:
		return;
		break;
	}
	//这里必须清掉，否则无法重新连接到网关，因为状态已经不正确了
	//m_encryptHander.Reset();
	//m_nCurrentStatus = eStatusIdle;
	
	ConnectToServer();
}
      
VOID CRobotClient::OnConnected()
{
	SwapRecvProcessBuffers();
	m_encryptHander.Reset();
	m_encryptHander.SetSelfSalt(m_encryptHander.GenSalt());
	m_nCurrentStatus = eStatusEncryReq ; //当前的状态改变
	unsigned int nSalt = m_encryptHander.GetSelfSalt();
	//OutputMsg(rmNormal ,_T("client salt=%d"),nSalt );
	CDataPacket &Packet = allocSendPacket();
	Packet << nSalt;
	flushSendPacket(Packet); 
	//AppendSendBuffer(&nSalt,sizeof(nSalt)); //发一个包告诉去服务器获取密钥
	//SendSocketBuffers();
	
}

CDataPacket& CRobotClient::AllocProtoPacket()
{
	CDataPacket &Packet = allocSendPacket();
	//PDATAHEADER pPackHdr;
	//预留通信数据头空间
	Packet.setLength(sizeof(DATAHEADER));
	Packet.setPosition(sizeof(DATAHEADER));
	//写入通信消息号
	return Packet;
}

VOID CRobotClient::FlushProtoPacket(CDataPacket& packet)
{
	PDATAHEADER pPackHdr = (PDATAHEADER)packet.getMemoryPtr();
	//计算并向协议头中写入通信数据长度

	char *pData = (char *)pPackHdr + sizeof(*pPackHdr);  //数据的指针
	pPackHdr->tag = (WORD)m_nKey;
	pPackHdr->len = packet.getLength() - sizeof(*pPackHdr);
	flushSendPacket(packet);
}

BOOL CRobotClient::ConnectToServer()
{
	int nErr;

	//如果已连接则直接返回
	if ( connected() )
	{
		return TRUE;
	}

	TICKCOUNT dwCurTick = _getTickCount();

	//创建套接字
	if ( getSocket() == INVALID_SOCKET )
	{
		SOCKET sock;
		nErr = createSocket(&sock);
		if ( nErr )
		{
			OutputError( nErr, _T("创建%s客户端套接字失败"), m_account.name );
			return FALSE;
		}
		setSocket(sock);

		// 复用socket
		//int optval = 1;
		//setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		
		//调整发送和接收缓冲大小
		nErr = setSendBufSize(4 * 1024);
		if ( nErr )
		{
			OutputError( nErr, _T("调整%s套接字发送缓冲长度失败"),  m_account.name);
			return FALSE;
		}
		nErr = setRecvBufSize(4 * 1024);
		if ( nErr )
		{
			OutputError( nErr, _T("调整%s套接字接收缓冲长度失败"),  m_account.name);
			return FALSE;
		}
	}
	//连接到会话服务器
	//if ( dwCurTick >= m_dwReconnectTick )
	{
		//m_dwReconnectTick = dwCurTick + 1 * 1000;
		nErr = connect(m_serverIp,m_nServerPort );
		if ( nErr )
		{
			OutputError( nErr, _T("连接到%s服务器失败"), m_serverIp );
			return FALSE;
		}
		//调整为非阻塞模式
		nErr = setBlockMode(FALSE);
		if ( nErr )
		{
			OutputError( nErr, _T("调整%s套接字接为非阻塞模式失败"), m_serverIp );
			return FALSE;
		}
		m_dwConnectTick = m_dwMsgTick = dwCurTick;
		return TRUE;
	}

	return FALSE;
}

bool CRobotClient::MoveBySceneId( int nSceneId, int nX, int nY )
{
	SCENECONFIG * pSc = GetRobotMgr()->GetGlobalConfig().GetSceneConfig().GetSceneConfig(nSceneId);
	if (pSc != NULL)
	{
		return MoveBySceneName(pSc->szScenceName, nX, nY);
	}
	return false;
}

bool CRobotClient::MoveBySceneName( LPCTSTR sSceneName, int nX, int nY )
{
	CClientActor & ca = GetActorData();
	if(ca.GetFubenId() > 0)
	{
		CDataPacket & netPack = AllocProtoPacket();
		netPack << (BYTE)enFubenSystemID << (BYTE)cExitFuben;
		netPack << (int)ca.GetFubenId();
		FlushProtoPacket(netPack);
	}
	return true;
}

