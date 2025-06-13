#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif
#include <_ast.h>
#include <_memchk.h>
#include <QueueList.h>
#include <Tick.h>
#include <Lock.h>
#include <Thread.h>
#include <CustomSocket.h>
#include "ServerDef.h"
#include "GateProto.h"
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "DataPacketReader.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerClientSocketEx.h"
#include "CustomServerSocket.h"
#include "CustomServerGateUser.h"
#include "CustomServerGate.h"
#include "CustomGateManager.h"

CCustomServerGate::CCustomServerGate()
{
	m_sGateName[0] = 0;
	m_nGateIndex = 0;
	m_bHasPrintErrInfo = false;
	ZeroMemory(&m_GateUserInfo, sizeof(m_GateUserInfo));
#ifndef _USE_GATE_SEND_PACK_LIST_
	ZeroMemory( &m_SendBufQueue, sizeof(m_SendBufQueue) );
	m_pSendAppendBuffer = &m_SendBufQueue[0];
	m_pSendProcBuffer = &m_SendBufQueue[1];

	InitializeCriticalSection( &m_WriteLock );
#endif
}

CCustomServerGate::~CCustomServerGate()
{
	CloseAllUser();
	FreeBuffers();
#ifndef _USE_GATE_SEND_PACK_LIST_
	DeleteCriticalSection( &m_WriteLock );
#endif
}

VOID CCustomServerGate::FreeBuffers()
{
#ifndef _USE_GATE_SEND_PACK_LIST_
	//释放发送数据缓冲区内存
	SafeFree( m_SendBufQueue[0].pBuffer );
	m_SendBufQueue[0].nSize = 0;
	m_SendBufQueue[0].nOffset = 0;    
	SafeFree( m_SendBufQueue[1].pBuffer );
	m_SendBufQueue[1].nSize = 0;
	m_SendBufQueue[1].nOffset = 0;
#else
	clearSendList();
#endif
}
	
VOID CCustomServerGate::SocketError(INT nErrorCode)
{
	OutputError(nErrorCode, _T("[GATE]%s(%s:%d)socket error"), m_sGateName, GetRemoteHost(), GetRemotePort());
	Inherited::SocketError(nErrorCode);
}

VOID CCustomServerGate::Disconnected()
{
	Inherited::Disconnected();
	OutputMsg(rmWaning, _T("[GATE]%s(%s:%d) closed"), m_sGateName, GetRemoteHost(), GetRemotePort());
	FreeBuffers();
	CloseAllUser();
	m_sGateName[0] = 0;
}

VOID CCustomServerGate::SendKeepAlive()
{
#ifndef _USE_GATE_SEND_PACK_LIST_
	GATEMSGHDR	MsgHdr;
	MsgHdr.dwGateCode = RUNGATECODE;
	MsgHdr.nSocket = 0;
	MsgHdr.wIdent = GM_CHECKCLIENT;
	MsgHdr.nDataSize = 0;
	AddGateBuffer( &MsgHdr, sizeof(MsgHdr) );
#else	
	CDataPacket &packet = AllocGateSendPacket(0, 0);
	PGATEMSGHDR pMsgHdr = (PGATEMSGHDR)packet.getMemoryPtr();
	pMsgHdr->wIdent = GM_CHECKCLIENT;
	pMsgHdr->tickCount = _getTickCount();
	FlushGateSendPacket(packet);
#endif
}

VOID CCustomServerGate::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
{
	INT_PTR nLen, nCheckMsgLen;
	char *pBuf, *pMsgBuf;
	PGATEMSGHDR pMsgHdr;
	BOOL boTipNotRunGateCode;

	//如果连接已断开则丢弃所有数据
	if ( !connected() )
	{
		OutputMsg( rmWaning, _T("ProcessRecvBuffers,socket is close ,drop data") );
		Inherited::ProcessRecvBuffers(pDataBuffer);
		SwapRecvProcessBuffers();
		return;
	}

	nLen = 0;
	pBuf = NULL;

	//nLen = pDataBuffer->nOffset;
	pBuf = pDataBuffer->pPointer;
	nLen = pDataBuffer->nOffset - (pBuf - pDataBuffer->pBuffer);
	if (!m_bHasPrintErrInfo && pBuf != pDataBuffer->pBuffer)
	{
		m_bHasPrintErrInfo = true;
		OutputMsg(rmError, _T("CCustomServerGate::ProcessRecvBuffers pPointer != pBuffer"));
	}
	//处理数据包的条件是接收缓冲的长度不少于一个网关通信消息头
	if ( nLen >= sizeof(*pMsgHdr) )
	{
		boTipNotRunGateCode = FALSE;
		while ( TRUE )
		{
			pMsgHdr = (PGATEMSGHDR)pBuf;
			nCheckMsgLen = pMsgHdr->nDataSize + sizeof(*pMsgHdr);
			//检查协议头是否有效（dwGateCode必须是RUNGATECODE）
			if ( pMsgHdr->dwGateCode == RUNGATECODE && nCheckMsgLen < 0x8000 )//丢弃长度大于0x8000的数据包
			{
				boTipNotRunGateCode = FALSE;
				if ( nLen < nCheckMsgLen )
					break;
				pMsgBuf = pBuf + sizeof(*pMsgHdr);
				DispathRecvMessage( pMsgHdr, pMsgBuf, pMsgHdr->nDataSize );
				pBuf += sizeof(*pMsgHdr) + pMsgHdr->nDataSize;
				nLen -= sizeof(*pMsgHdr) + pMsgHdr->nDataSize;
			}
			else
			{
				pBuf++;
				nLen--;
				if ( !boTipNotRunGateCode )
				{
					boTipNotRunGateCode = TRUE;
					OutputMsg( rmWaning, _T("%s droped a packed(not RUNGATECODE)"), _T(__FUNCTION__) );
				}
			}
			if ( nLen < sizeof(*pMsgHdr) )
				break;
		}

		pDataBuffer->pPointer = pBuf;
	}
	SwapRecvProcessBuffers();
}

VOID CCustomServerGate::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	switch (uMsg)
	{
	//按用户SOCKET关闭用户
	case SGIM_CLOSE_USER_SOCKET:
		OutputMsg(rmTip,"CCustomServerGate::DispatchInternalMessage, close socket=%d",(int)uParam1);
		CloseUser(uParam1);
		break;
	//按用户SOCKET以及用户在服务器中的会话编号关闭用户
	case SGIM_CLOSE_USER_SOCKET_SIDX:
		OutputMsg(rmTip,"CCustomServerGate::DispatchInternalMessage, CloseUser socket=%d",(int)uParam1);
		CloseUser(uParam1, (int)uParam2);
		break;
	//按用户的全局会话ID关闭用户，Param1=全局会话ID
	case SGIM_CLOSE_USER_SOCKET_GSID:
		OutputMsg(rmTip,"CCustomServerGate::DispatchInternalMessage, CloseUser account=%d",(int)uParam1);
		CloseUserByGlobalSessionId((int)uParam1);
		break;
	case SGIM_KICK_USER_SOCKET_GSID:
		OutputMsg(rmTip,"CCustomServerGate::DispatchInternalMessage, kick user account=%d",(int)uParam1);
		KickUserByGlobalSessionId((int)uParam1);
		break;
	}
}

VOID CCustomServerGate::DispathRecvMessage(PGATEMSGHDR pMsgHdr, char *pBuffer, SIZE_T nBufSize)
{
	int nUserIndex;
	if  (pMsgHdr->wIdent  != GM_CHECKSERVER)
	{
		//OutputMsg( rmTip, _T("收到网关的数据!, cmd=%d,nBufSize=%d"),  pMsgHdr->wIdent,nBufSize );
	}
	
	switch ( pMsgHdr->wIdent )
	{
	//打开新用户
	case GM_OPEN:
		{
			nUserIndex = (int)OpenNewUser( pMsgHdr->nSocket, pMsgHdr->wSessionIdx, pBuffer );
			SendAcceptUser( pMsgHdr->nSocket, pMsgHdr->wSessionIdx, nUserIndex + 1 );
		}
		break;
	//关闭用户
	case GM_CLOSE:
		OutputMsg(rmTip,"CCustomServerGate::DispathRecvMessage GM_CLOSE,SOCKET=%d ",(int)pMsgHdr->nSocket);
		CloseUser( pMsgHdr->nSocket,false ); //网关通知关闭的就不回消息过去了
		break; 
	//网关发送的心跳包
	case GM_CHECKSERVER:
		{
			nUserIndex = m_sGateName[0];
			_asncpytA( m_sGateName, pBuffer );
			if ( !nUserIndex )
			{
				OutputMsg(rmTip, _T("[GATE]%s:%d registed(%s)"), GetRemoteHost(), GetRemotePort(), m_sGateName);
			}

			SendKeepAlive();
		}
		break;
	case GM_GATEUSERINFO:
		if (nBufSize == sizeof(GATEUSERINFO))
		{
			CopyMemory(&m_GateUserInfo, pBuffer, nBufSize);			
		}
		break;
	//网关转发用户数据
	case GM_DATA:
	case GM_APPKEEPALIVEACK:	
		PostUserData( pMsgHdr, pBuffer, nBufSize );	
		break;
	}
}

VOID CCustomServerGate::SingleRun()
{
	Inherited::SingleRun();
	if ( connected() )
	{
#ifndef _USE_GATE_SEND_PACK_LIST_
		SendGateBuffers();
#endif
	}
}

INT_PTR  CCustomServerGate::OpenNewUser(UINT64 nSocket, int nGateSessionIndex, char *sIPAddr)
{
	INT_PTR i, nIndex;
	CCustomServerGateUser *pUser, **pUserList;

	pUser = CreateGateUser();

	//从用户列表中找到一个空闲的用户位置（指针为NULL的位置）
	//如果有空闲的位置则将用户放置在空闲处
	nIndex = -1;
	pUserList = m_UserList;
	for ( i=m_UserList.count()-1; i>-1; --i )
	{
		if ( !pUserList[i] )
		{
			pUserList[i] = pUser;
			nIndex = i;
			break;
		}
	}
	//未能将用户放置再空闲处则添加到用户列表的末尾
	if ( nIndex < 0 )
	{
		nIndex = m_UserList.add( pUser );
	}

	//初始化网关用户对象
	pUser->nSocket				= nSocket;
	pUser->nGateSessionIndex		= nGateSessionIndex;
	pUser->nServerSessionIndex	= (int)nIndex;
	_asncpytA(pUser->sIPAddr, sIPAddr);
	pUser->dwConnectTick			= pUser->dwLastMsgTick = _getTickCount();
	pUser->dwDelayCloseTick		= 0;
	pUser->boMarkToClose			= FALSE;
	//TRACE(_T("创建连接Socket=%d,nGateSessionIndex=%d,nServerIndex=%d\n"),nSocket,nGateSessionIndex,nIndex +1);
	//回调打开用户的函数
	OnOpenUser(pUser);

	return nIndex;
}

CCustomServerGateUser* CCustomServerGate::CreateGateUser()
{
	return new CCustomServerGateUser();
}

VOID CCustomServerGate::DestroyGateUser(CCustomServerGateUser* pUser)
{
	SafeDelete(pUser);
}

VOID CCustomServerGate::OnOpenUser(CCustomServerGateUser* pUser)
{
}

VOID CCustomServerGate::OnCloseUser(CCustomServerGateUser* pUser)
{
}

VOID CCustomServerGate::OnGateClosed()
{
}

VOID CCustomServerGate::SendAcceptUser(UINT64 nSocket, int nGateSessionIndex, int nServerSessionIndex)
{
#ifndef _USE_GATE_SEND_PACK_LIST_
	GATEMSGHDR	MsgHdr;

	MsgHdr.dwGateCode	= RUNGATECODE;
	MsgHdr.nSocket		= nSocket;
	MsgHdr.wSessionIdx	= (WORD)nGateSessionIndex;
	MsgHdr.wIdent		= GM_SERVERUSERINDEX;
	MsgHdr.wServerIdx	= (WORD)nServerSessionIndex;
	MsgHdr.nDataSize	= 0;

	AddGateBuffer( &MsgHdr, sizeof(MsgHdr) );
#else
	CDataPacket &packet = AllocGateSendPacket(nSocket, nGateSessionIndex,nServerSessionIndex);
	PGATEMSGHDR pMsgHdr = (PGATEMSGHDR)packet.getMemoryPtr();
	pMsgHdr->wIdent		= GM_SERVERUSERINDEX;
	//pMsgHdr->wServerIdx	= (WORD)nServerSessionIndex;
	FlushGateSendPacket(packet);

	OutputMsg(rmTip,_T("[Login] (4) 告诉网关已添加玩家gate： socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),
		nSocket,nGateSessionIndex,nServerSessionIndex,GetCurrentThreadId());
#endif
}


VOID CCustomServerGate::PostCloseGateServer()
{


#ifndef _USE_GATE_SEND_PACK_LIST_
	GATEMSGHDR	MsgHdr;
	MsgHdr.dwGateCode = RUNGATECODE;
	MsgHdr.nSocket = 0;
	MsgHdr.wIdent = GM_CLOSE_SERVER;
	MsgHdr.nDataSize = 0
	AddGateBuffer( &MsgHdr, sizeof(MsgHdr) );
#else	
	CDataPacket &packet = AllocGateSendPacket(0, 0);
	PGATEMSGHDR pMsgHdr = (PGATEMSGHDR)packet.getMemoryPtr();
	pMsgHdr->wIdent = GM_CLOSE_SERVER;
	pMsgHdr->tickCount =0;	
	FlushGateSendPacket(packet);
#endif
}
VOID CCustomServerGate::CloseUser(const UINT64 nSocket,bool bNeedTellGate)
{
	INT_PTR i;
	CCustomServerGateUser **pUserList;
	OutputMsg(rmNormal, _T("CCustomServerGate::CloseUser: socket=%d"), (int)nSocket);
	pUserList = m_UserList;
	for ( i=m_UserList.count()-1; i>-1; --i )
	{
		if ( pUserList[i] && pUserList[i]->nSocket == nSocket )
		{
			GateUserClosed( pUserList[i] ,bNeedTellGate);
			pUserList[i] = NULL;
			//TRACE( "被动关闭网关用户连接\n" );
			break;
		}
	}
}

VOID CCustomServerGate::CloseUser(const UINT64 nSocket, const int nServerSessionIndex,bool bNeedTellGate)
{
	OutputMsg(rmNormal, _T("CCustomServerGate::CloseUser2: socket=%d,srvIndex=%d"), (int)nSocket,int(nServerSessionIndex));
	if ( nServerSessionIndex >= 0 && nServerSessionIndex < m_UserList.count() )
	{
		if ( m_UserList[nServerSessionIndex] && m_UserList[nServerSessionIndex]->nSocket == nSocket )
		{
			GateUserClosed( m_UserList[nServerSessionIndex],bNeedTellGate );
			m_UserList[nServerSessionIndex] = NULL;
		}
		else
		{
			//OutputMsg(rmWaning,_T("用户索引中指定位置的用户不存在或套接字句柄未能匹配\n"));
		}
	}
}

VOID CCustomServerGate::CloseUserByGlobalSessionId(const int nGlobalSessionId)
{
	INT_PTR nIndex;
	CCustomServerGateUser *pUserList = UserExistsOfGlobalSessionId(nGlobalSessionId, &nIndex);

	if ( pUserList )
	{
		GateUserClosed( pUserList );
		m_UserList[nIndex] = NULL;
	}
}

VOID CCustomServerGate::KickUserByGlobalSessionId(const int nGlobalSessionId)
{
	INT_PTR nIndex;
	CCustomServerGateUser *pUserList = UserExistsOfGlobalSessionId(nGlobalSessionId, &nIndex);

	if ( pUserList )
	{
		GateUserClosed( pUserList );
		m_UserList[nIndex] = NULL;
	}
}


CCustomServerGateUser* CCustomServerGate::UserExistsOfGlobalSessionId(const int nGlobalSessionId, PINT_PTR pIndex)
{
	INT_PTR i;
	CCustomServerGateUser **pUserList;

	pUserList = m_UserList;
	for ( i=m_UserList.count()-1; i>-1; --i )
	{
		if ( pUserList[i] && pUserList[i]->nGlobalSessionId == nGlobalSessionId )
		{
			if ( pIndex ) *pIndex = i;
			return pUserList[i];
		}
	}
	return NULL;
}

VOID CCustomServerGate::CloseAllUser()
{
	INT_PTR i;
	CCustomServerGateUser **pUserList;
	CCustomServerGateUser *pGateUser;

	pUserList = m_UserList;

	for ( i=m_UserList.count() -1; i>-1; --i )
	{
		if ( pUserList[i] )
		{
			pGateUser = pUserList[i];
			pUserList[i] = NULL;
			GateUserClosed(pGateUser);
		}
	}
	m_UserList.clear();
}

VOID CCustomServerGate::GateUserClosed(CCustomServerGateUser *pGateUser,bool bNeedTellGate)
{
	OutputMsg(rmNormal, _T("CCustomServerGate::GateUserClosed socket=%d, sessionId=%d, serverId=%d"), (int)pGateUser->nSocket, 
				(int)pGateUser->nGateSessionIndex, (int)pGateUser->nServerSessionIndex);
	if(bNeedTellGate)
	{
		SendGateCloseUser(pGateUser->nSocket, pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
	}
	
	OnCloseUser(pGateUser);
	
	pGateUser->nSocket = 0;
	pGateUser->nGlobalSessionId = 0;
	pGateUser->nServerSessionIndex = 0;
	pGateUser->nGateSessionIndex = 0;

	//添加到空闲用户列表中
	DestroyGateUser( pGateUser );
}

VOID CCustomServerGate::SendGateCloseUser(UINT64 nSocket, int nGateSessionIndex,int nServerIndex)
{
#ifndef _USE_GATE_SEND_PACK_LIST_
	GATEMSGHDR	MsgHdr;

	MsgHdr.dwGateCode	= RUNGATECODE;
	MsgHdr.nSocket		= nSocket;
	MsgHdr.wSessionIdx	= (WORD)nGateSessionIndex;
	MsgHdr.wIdent		= GM_CLOSE;
	MsgHdr.wServerIdx	= (WORD)nServerIndex;
	MsgHdr.nDataSize	= 0;

	AddGateBuffer( &MsgHdr, sizeof(MsgHdr) );
#else
	OutputMsg(rmNormal, _T("关闭Socket=%d, nGateSessionIndex=%d\n"),nSocket, nGateSessionIndex);
	CDataPacket &packet = AllocGateSendPacket(nSocket, nGateSessionIndex,nServerIndex);
	PGATEMSGHDR pMsgHdr = (PGATEMSGHDR)packet.getMemoryPtr();
	pMsgHdr->wIdent		= GM_CLOSE;
	//pMsgHdr->wServerIdx =(WORD)nServerIndex;
	FlushGateSendPacket(packet);
#endif
}

VOID CCustomServerGate::PostUserData(PGATEMSGHDR pMsgHdr, char *pData, SIZE_T nDataSize)
{
	INT_PTR i;
	CCustomServerGateUser *pGateUser, **pGateUserList;

	//按用户的服务器索引查找用户
	pGateUser = NULL;
	if ( pMsgHdr->wServerIdx > 0 && pMsgHdr->wServerIdx <= m_UserList.count() )
	{
		pGateUser = m_UserList.get(pMsgHdr->wServerIdx - 1);
		if ( pGateUser && pGateUser->nSocket != pMsgHdr->nSocket )
		{
			pGateUser = NULL;
		}
	}
	//如果没有找到用户则从用户表中遍历查找用户
	if ( !pGateUser )
	{
		pGateUserList = m_UserList;
		for ( i=m_UserList.count()-1; i>-1; --i )
		{
			if ( pGateUserList[i] && pGateUserList[i]->nSocket == pMsgHdr->nSocket )
			{
				pGateUser = pGateUserList[i];
				//找到用户后重新向网关发送设定用户服务端索引的消息
				SendAcceptUser( pMsgHdr->nSocket, pMsgHdr->wSessionIdx, (int)i + 1 );
				TRACE(_T("%s reget user server-index\n"),__FUNCTION__);
				break;
			}
		}
	}

	if ( pGateUser && !pGateUser->boMarkToClose && pGateUser->dwDelayCloseTick == 0 )
	{
		/*
		//减少一个字节！从网关发送过来的数据增加了一个字节，表示为0终止符，此处不需要终止符。
		if ( nDataSize > 0 ) nDataSize--; 
		*/
		//调用处理用户数据的函数
		pGateUser->dwGateTickCount = pMsgHdr->tickCount;//带网关的tickcount过来
		OnDispatchUserMsg(pGateUser, pData, nDataSize);
	}

}

#ifndef _USE_GATE_SEND_PACK_LIST_

VOID CCustomServerGate::AddGateBuffer(LPCVOID pBuffer, SIZE_T nBufferSize)
{
	PDATABUFFER pDataBuf;

	if ( !pBuffer || nBufferSize <= 0 || !connected() ) return ;

	EnterCriticalSection( &m_WriteLock );   
	pDataBuf = m_pSendAppendBuffer;

	if ( pDataBuf->nSize < (INT_PTR)(pDataBuf->nOffset + nBufferSize) )
	{
		pDataBuf->nSize += (pDataBuf->nSize / 8192 + 1) * 8192;
		pDataBuf->pBuffer = (char*)realloc( pDataBuf->pBuffer, pDataBuf->nSize );
	}
	memcpy( &pDataBuf->pBuffer[pDataBuf->nOffset], pBuffer, nBufferSize );
	pDataBuf->nOffset += nBufferSize;

	LeaveCriticalSection( &m_WriteLock );
}

VOID CCustomServerGate::AddUserBuffer(SOCKET nUserSocket, INT_PTR nGateSessionIndex, LPCVOID pBuffer, SIZE_T nBufferSize)
{
	static const size_t MemAlginmentSize = 8192;//发送缓冲内存块边界大小，必须是2的次方数

	PGATEMSGHDR	pMsgHdr;
	INT_PTR nNeedSize;
	char *pBuf;
	PDATABUFFER pDataBuf;

	if ( !pBuffer || nBufferSize <= 0 || !connected() ) return;

	EnterCriticalSection( &m_WriteLock );   

	pDataBuf = m_pSendAppendBuffer;
	nNeedSize = pDataBuf->nOffset + sizeof(*pMsgHdr) + nBufferSize;
	if ( pDataBuf->nSize < nNeedSize )
	{
		pDataBuf->nSize *= 2;
		if ( pDataBuf->nSize < nNeedSize )
		{
			pDataBuf->nSize = (nNeedSize / MemAlginmentSize + 1) * MemAlginmentSize;
		}
		pDataBuf->pBuffer = (char*)realloc( pDataBuf->pBuffer, pDataBuf->nSize );
	}

	pBuf = pDataBuf->pBuffer + pDataBuf->nOffset;
	pMsgHdr = (PGATEMSGHDR)pBuf;
	pBuf += sizeof(*pMsgHdr);

	pMsgHdr->dwGateCode	= RUNGATECODE;
	pMsgHdr->nSocket	= nUserSocket;
	pMsgHdr->wSessionIdx= (WORD)nGateSessionIndex;
	pMsgHdr->wIdent		= GM_DATA;
	pMsgHdr->nDataSize	= (INT)nBufferSize;

	memcpy( pBuf, pBuffer, nBufferSize );
	pDataBuf->nOffset += sizeof(*pMsgHdr) + nBufferSize;

	LeaveCriticalSection( &m_WriteLock );
}

VOID CCustomServerGate::SendGateBuffers()
{
	static const INT_PTR OnceSendBytes = 8192;//每次通信最大发送数据长度

	PDATABUFFER pDataBuf;
	int	nBytesWriten, nSendSize;

	if ( m_pSendProcBuffer->nOffset <= 0 )
	{
		EnterCriticalSection( &m_WriteLock );
		pDataBuf = m_pSendProcBuffer;
		m_pSendProcBuffer = m_pSendAppendBuffer;
		m_pSendAppendBuffer = pDataBuf;
		m_pSendProcBuffer->pPointer = m_pSendProcBuffer->pBuffer;
		LeaveCriticalSection( &m_WriteLock );
	}

	pDataBuf = m_pSendProcBuffer;
	while ( pDataBuf->nOffset > 0 )
	{
		nSendSize = (INT)pDataBuf->nOffset;
		if ( nSendSize > OnceSendBytes )
			nSendSize = OnceSendBytes;

		nBytesWriten = send( pDataBuf->pPointer, nSendSize );
		//连接断开了
		if ( nBytesWriten <= 0 )
			break;

		//调整发包缓冲指针以及剩余包长度
		pDataBuf->pPointer += nBytesWriten;
		pDataBuf->nOffset -= nBytesWriten;
		if ( pDataBuf->nOffset <= 0 )
			pDataBuf->nOffset = 0;

		//如果发送的字节长度少于需求发送的长度则表明发送缓冲已经满了
		if ( nBytesWriten < nSendSize )
			break;
	}
}

#else

CDataPacket& CCustomServerGate::AllocGateSendPacket(UINT64 nUserSocket, int nGateSessionIndex,int nServerIndex)
{
	CDataPacket *pPacket;

	allocSendPacketList(*((CBaseList<CDataPacket*>*)NULL), 0, &pPacket);

	PGATEMSGHDR	pMsgHdr;
	pPacket->setLength(sizeof(*pMsgHdr));
	pPacket->setPosition(sizeof(*pMsgHdr));
	pMsgHdr = (PGATEMSGHDR)pPacket->getMemoryPtr();

	pMsgHdr->dwGateCode	= RUNGATECODE;
	pMsgHdr->nSocket	= nUserSocket;
	pMsgHdr->wSessionIdx= (WORD)nGateSessionIndex;
	pMsgHdr->wIdent		= GM_DATA;
	pMsgHdr->wServerIdx =(WORD )nServerIndex;
	return *pPacket;
}

VOID CCustomServerGate::FlushGateSendPacket(CDataPacket& packet)
{
	//向网关通信头中写入用户数据包长度
	PGATEMSGHDR	pMsgHdr = (PGATEMSGHDR)packet.getMemoryPtr();
	pMsgHdr->nDataSize	= (int)packet.getLength() - sizeof(*pMsgHdr);
	
	flushSendPacket(packet);
}

VOID CCustomServerGate::FlushGateBigPacket(CDataPacket& packet)
{
	flushSendPacket(packet);
}
#endif
