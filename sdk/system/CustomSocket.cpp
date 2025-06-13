#include "SocketConfig.h"

using namespace wylib::inet::socket;

CCustomSocket::CCustomSocket()
{
	m_nSocket = INVALID_SOCKET;
	m_LocalAddr.sin_family = AF_INET;
	m_LocalAddr.sin_addr.s_addr = 0;
	m_LocalAddr.sin_port = 0;
	m_RemoteAddr = m_LocalAddr;
	m_boBlock = true;
	m_boConnected = false;
	m_boConnecting = false;
}

CCustomSocket::~CCustomSocket()
{
	close();
}

VOID CCustomSocket::OnConnected()
{
}

VOID CCustomSocket::OnDisconnected()
{
}

VOID CCustomSocket::OnError(INT errorCode)
{
}

VOID CCustomSocket::Connected()
{
	OnConnected();
}

VOID CCustomSocket::Disconnected()
{
	OnDisconnected();
}

VOID CCustomSocket::SocketError(INT errorCode)
{
	OnError(errorCode);
	close();
}

VOID CCustomSocket::setSocket(const SOCKET socket)
{
	m_nSocket = socket;

	//设置了套接字后变更为制定的阻塞模式
	if ( m_nSocket != INVALID_SOCKET )
	{
		m_boConnected = true;
		m_boBlock = !m_boBlock;
		setBlockMode( !m_boBlock );
	}
	else m_boConnected = false;
}