#include "StdAfx.h"

using namespace jxSrvDef;

CCommonClient::CCommonClient()
{
	SetClientName(_T("普通逻辑"));
	m_vFreeList.setLock(&m_vFreeListLock);
}

CCommonClient::~CCommonClient()
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

VOID CCommonClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
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
		SSM_COMMONLOGIC_2_LOGIC_DATA,	// 公共服务器 -> 逻辑服务器
		nCmd,							// 消息ID
		(INT_PTR)outPacket);			// 消息数据
}

jxSrvDef::SERVERTYPE CCommonClient::getLocalServerType()
{
	return jxSrvDef::GameServer;
}

LPCSTR CCommonClient::getLocalServerName()
{
	return GetLogicServer()->getServerName();
}

int CCommonClient::getLocalServerIndex()
{
	return GetLogicServer()->GetServerIndex();
}

CDataPacket* CCommonClient::AllocSendPacket()
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

void CCommonClient::FreeBackUserDataPacket(CDataPacket *pPacket)
{
	m_vFreeList.append(pPacket);
}
