#include "StdAfx.h"

CCommonServerClient::CCommonServerClient(CCommonServer *lpCommonServer, SOCKET nSocket, PSOCKADDR_IN pAddrIn) 
	: Inherited()
{
	SetClientSocket(nSocket, pAddrIn);
	m_vFreeList.setLock(&m_vFreeListLock);
}

CCommonServerClient::~CCommonServerClient()
{

}

VOID CCommonServerClient::OnDisconnected()
{
	OutputMsg(rmError, _T("%s =%d"), __FUNCTION__);
}

VOID CCommonServerClient::OnError(INT errorCode)
{
	OutputMsg(rmError, _T("%s 与[%s][serverindex=%d]的连接错误，errcode=%d"), 
		__FUNCTION__, 
		getClientName(), 
		getClientServerIndex(),
		errorCode);
}

VOID CCommonServerClient::OnRun()
{

}

VOID CCommonServerClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	// nCmd ==0 的是心跳包
	if (nCmd <= 0) 
	{
		if (nCmd < 0)
			OutputMsg(rmWaning, _T("%s recv unknown msg[id=%d]"), __FUNCTION__, (int)nCmd);

		return;
	}
	
	CDataPacket *outPacket = AllocSendPacket();
	if (!outPacket) 
	{
		OutputMsg(rmError, _T("%s Alloc Packet Failed"), __FUNCTION__);
		return;
	}

	// TODO 这里还需要能找到是哪个逻辑服务器发过来的数据。要不然后面逻辑线程处理数据的时候没法释放数据包
	// 最好数据包头部加上Client的标记，这样就能在逻辑线程处理完数据包的时候正常释放这里分配的数据包对象。
	outPacket->writeBuf(inPacket.getOffsetPtr(), inPacket.getAvaliableLength());		
	GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(
		SSM_LOGIC_2_COMMONLOGIC_DATA,	// 逻辑服务器 -> 公共服务器 
		nCmd,							// 消息ID
		(INT_PTR)outPacket,				// 消息数据
		(UINT_PTR)this);				// 用于逻辑线程处理完消息后释放数据包

}

CDataPacket* CCommonServerClient::AllocSendPacket()
{
	if (m_vFreeList.count() <= 0)
	{
		m_vFreeList.flush();
	}
	if (m_vFreeList.count() <= 0)
	{
		allocSendPacketList(m_vFreeList,512);
	}

	CDataPacket* freePacket = m_vFreeList.pop();
	freePacket->setLength(0);
	return freePacket;
}

void CCommonServerClient::FreeBackUserDataPacket(CDataPacket* pPacket)
{
	m_vFreeList.append(pPacket);
}

bool CCommonServerClient::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
{
	if (SERVER_REGDATA::GT_JianXiaoJiangHu == pRegData->GameType &&
		jxSrvDef::GameServer == pRegData->ServerType)
	{
		OutputMsg(rmTip, _T("建立同普通逻辑服务器[目标服务器ServerIndex=%d]的连接"), pRegData->ServerIndex);
		return true;
	}

	return false;
}
