#include "StdAfx.h"

using namespace jxSrvDef;

CLocalCrossClient::CLocalCrossClient()
	: m_SrvHost("")
	, m_Port(0) 
	, m_isRun(true)
	, m_CurIndex(1)
	, m_CsGateAddr("")
	, m_nCsGatePort(0)  
{
	SetClientName(_T("跨服逻辑"));
	m_vFreeList.setLock(&m_vFreeListLock);
}

CLocalCrossClient::~CLocalCrossClient()
{
	m_vFreeList.flush();	
	for (INT_PTR i = 0; i < m_vFreeList.count(); i++)
	{
		CDataPacket* dp = m_vFreeList[i];
		if(dp) {
			dp->setPosition(0);
			flushSendPacket(*dp);
		}
	}
	
	m_vFreeList.clear();
}

VOID CLocalCrossClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if (nCmd <= 0) 
	{
		if(nCmd <0)
		{
			OutputMsg(rmWaning, _T("%s recv unknown msg[id=%d]"), __FUNCTION__, (int)nCmd);
		}
		return;
	}

	CDataPacket *outPacket = AllocSendPacket();
	if (!outPacket) 
	{
		OutputMsg(rmError, _T("%s Alloc Packet Failed"), __FUNCTION__);
		return;
	}

	outPacket->writeBuf(inPacket.getOffsetPtr(), inPacket.getAvaliableLength());
	GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(
		SSM_CROSS_2_LOGIC_DATA,	// 跨服服务器 -> 逻辑服务器
		nCmd,							// 消息ID
		(INT_PTR)outPacket);			// 消息数据
}

jxSrvDef::SERVERTYPE CLocalCrossClient::getLocalServerType()
{
	return jxSrvDef::GameServer;
}

LPCSTR CLocalCrossClient::getLocalServerName()
{
	return GetLogicServer()->getServerName();
}

int CLocalCrossClient::getLocalServerIndex()
{
	return GetLogicServer()->GetCrossServerId();//m_nCrosssKey* 100000 + GetLogicServer()->GetServerIndex();
}

CDataPacket* CLocalCrossClient::AllocSendPacket()
{
	if (m_vFreeList.count() <= 0)
	{
		m_vFreeList.flush();
	}
	if (m_vFreeList.count() <= 0)
	{
		allocSendPacketList(m_vFreeList,512);
	}
		
	CDataPacket* m_TempData = m_vFreeList.pop();
	m_TempData->setLength(0);
	return m_TempData;
}

void CLocalCrossClient::FreeBackUserDataPacket(CDataPacket *pPacket)
{
	m_vFreeList.append(pPacket);
}

void CLocalCrossClient::OnRecvMessage(int nMsgId, CDataPacketReader &reader)
{
	switch (nMsgId)
	{
	default:
		OutputMsg(rmError, _T("%s Recv unknown message:%d"), __FUNCTION__, nMsgId);
		break;
	}
}
 
//重置服务器
void CLocalCrossClient::OnStopServer()
{
	m_isRun = false;
	//Stop();
	Disconnected();
}
bool CLocalCrossClient::OnRunServer()
{
	//ConnectToServer();
	m_isRun = true;
	//Connected(); 
	//Startup();  
	//跨服 
	if ( !Startup() ) 
	{
		return false;
	} 

	return true;
}
void CLocalCrossClient::OnSetSrvHostPort(std::string SrvHost, int64_t Port)
{
	m_SrvHost = SrvHost;
	m_Port = Port;
	SetServerHost(SrvHost.c_str());
	SetServerPort(Port);
} 
//发送到原服数据包
VOID CLocalCrossClient::SendRawServerData(const jxSrvDef::INTERSRVCMD nCmd, LPCVOID pData, SIZE_T size)
{
	if (nCmd <= 0) 
	{
		if(nCmd <0)
		{
			OutputMsg(rmWaning, _T("%s recv unknown msg[id=%d]"), __FUNCTION__, (int)nCmd);
		}
		return;
	}

	CDataPacket *outPacket = AllocSendPacket();
	if (!outPacket) 
	{
		OutputMsg(rmError, _T("%s Alloc Packet Failed"), __FUNCTION__);
		return;
	}
	// outPacket->setPosition(0);
	printf("len:%d\n",size);
	outPacket->writeBuf(pData,size);
	// outPacket->writeBuf(inPacket.getOffsetPtr(), inPacket.getAvaliableLength());
	// GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(
	// 	SSM_CROSS_2_LOGIC_DATA,	// 跨服服务器 -> 逻辑服务器
	// 	nCmd,							// 消息ID
	// 	(INT_PTR)outPacket,(UINT_PTR)this);			// 消息数据
	GetLogicServer()->GetCrossServerManager()->PostCrossServerMessage(SSM_CROSS_2_LOGIC_DATA,
			nCmd,							// 消息ID
		(Uint64)outPacket,0,(UINT_PTR)this);
}
