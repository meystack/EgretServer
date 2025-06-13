#include "StdAfx.h"

CCrossServerManager::CCrossServerManager() {
	m_sServerName[0] = 0;
	m_pCrossServer = new CCrossServer(this);
}

CCrossServerManager::~CCrossServerManager() {
	Shutdown();
	SafeDelete(m_pCrossServer);
}


VOID CCrossServerManager::SetServerName(LPCSTR sSrvName) {
	_asncpytA(m_sServerName, sSrvName);
}

VOID CCrossServerManager::SetCrossServiceAddress(LPCTSTR sHost, const int nPort) {
	m_pCrossServer->SetServiceHost(sHost);
	m_pCrossServer->SetServicePort(nPort);
}


BOOL CCrossServerManager::Startup() {
	//初始化网络套接字
	int nError = CCustomWorkSocket::InitSocketLib();
	if ( nError ) {
		OutputError( nError, _T("初始化网络库失败") );
		return FALSE;
	}
	//启动会话客户端
	if ( !m_pCrossServer->Startup() ) {
		OutputMsg(rmError, _T("!m_pCrossServer->Startup() 失败"));
		return FALSE;
	}
	
	return TRUE;
}

VOID CCrossServerManager::Shutdown()
{
	//停止会话客户端
	if( m_pCrossServer ) {
		m_pCrossServer->Stop();
	}
	SafeDelete(m_pCrossServer);
}
