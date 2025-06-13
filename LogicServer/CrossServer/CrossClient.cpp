
#include "StdAfx.h"
VOID CCrossClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	InHandle::OnPacket(nCmd,inPacket);
}

VOID CCrossClient::OnDisconnected()
{
	m_dwDisconnectedTick = _getTickCount();
	if ( registed() )
	{
		OutputMsg( rmWaning, _T("%s服务器(%s)(%s:%d)连接已断开"), getServerTypeName(getClientType()),
			getClientName(), GetRemoteHost(), GetRemotePort() );
	}
	else
	{
		OutputMsg( rmWaning, _T("%s服务器(%s:%d)连接已断开"), getServerTypeName(getClientType()),
			GetRemoteHost(), GetRemotePort() );
	}
	Inherited::OnDisconnected();
}

VOID CCrossClient::OnError(INT errorCode)
{
	
	OutputError(errorCode, _T("%s服务器(%s)(%s:%d)套接字错误"), getServerTypeName(getClientType()),
		getClientName(), GetRemoteHost(), GetRemotePort() );
	Inherited::OnError(errorCode);
}

VOID CCrossClient::OnRun()
{

}
bool CCrossClient::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
{
	//游戏服和db能够登陆
	if ( pRegData->GameType == SERVER_REGDATA::GT_JianXiaoJiangHu && 
		( pRegData->ServerType == GameServer || pRegData->ServerType == DBServer) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CCrossClient::OnRegDataValidated()
{

}

void CCrossClient::OnSendReqCommonPlatformResult(CDataPacketReader &inPacket)
{
	// unsigned int nAccountId = 0, nActorId = 0;
	// int nServerIndex = 0,nDestServerId = 0, nType = 0, nResult = 0;
	// inPacket >> nDestServerId;
	// inPacket >> nAccountId >> nActorId;
	// inPacket >> nType >> nResult;

	// CDataPacket &packet = allocProtoPacket(SessionServerProto::sReqestPlatformAck);
	// packet << (unsigned int)nActorId << (BYTE)SessionServerProto::neSuccess;
	// packet << (unsigned int)nAccountId << (int)nDestServerId << (int)nResult << (int)nType;
	// flushProtoPacket(packet);
}