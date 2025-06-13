
#include "StdAfx.h"

CDBSessionClient::CDBSessionClient(CDBServer *lpDBServer)
{
	m_pDBServer = lpDBServer;
}

CDBSessionClient::~CDBSessionClient()
{
}

SERVERTYPE CDBSessionClient::getLocalServerType()
{
	return DBServer;
}

int CDBSessionClient::getLocalServerIndex()
{
	return m_pDBServer->getServerIndex();
}
LPCSTR CDBSessionClient::getLocalServerName()
{
	return m_pDBServer->getServerName();
}

CCustomGlobalSession* CDBSessionClient::CreateSession()
{
	return Inherited::CreateSession();
}

VOID CDBSessionClient::DestroySession(CCustomGlobalSession* pSession)
{
	Inherited::DestroySession(pSession);
}

VOID CDBSessionClient::OnOpenSession(CCustomGlobalSession* pSession, BOOL boIsNewSession)
{
	
	//收到打开session的包的时候，向session服务器发一个包，回复一下，session依据这个回复告诉客户端登陆
	//通过这个确认的流程，保证登陆服务器的时候，session的数据是到位的
	//只有自己的数据服务器才会回复，其他的不需要回复
	if( m_pDBServer->getGameReady(pSession->nServerIndex) )
	{
		CDataPacket &data = allocProtoPacket(jxInterSrvComm::SessionServerProto::cConformOpenSession); //分配一个 网络包
		data << (int)pSession->nSessionId ;
		data << (int)pSession->nServerIndex;
		flushProtoPacket(data);
	}
}

VOID CDBSessionClient::OnCloseSession(int nSessionId)
{
	//会话被关闭的同时需要向网关发送按全局会话ID关闭连接的消息
	m_pDBServer->PostCloseGateUserBySessionId(nSessionId);
}

VOID CDBSessionClient::OnUpdateSessionState(CCustomGlobalSession* pSession, jxSrvDef::GSSTATE oldState, jxSrvDef::GSSTATE newState)
{
}

VOID CDBSessionClient::OnQuerySessionExists(INT_PTR nSessionId)
{
	//异步的通过会话ID查询网关用户，网关对此消息的返回将在内部消息分派函数中处理
	m_pDBServer->PostQueryUserExistsBySessionId((int)nSessionId);
}

