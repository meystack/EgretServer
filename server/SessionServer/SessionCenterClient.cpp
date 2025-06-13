#include "StdAfx.h"

using namespace jxSrvDef;
using namespace jxInterSrvComm;

CSessionCenterClient::CSessionCenterClient()
{
	
	SetClientName(_T("SessionCenterClient"));
	m_vFreeList.setLock(&m_vFreeListLock);
}

CSessionCenterClient::~CSessionCenterClient()
{
	m_vFreeList.flush();	
	for (INT_PTR i = 0; i < m_vFreeList.count(); i++)
	{
		CDataPacket* dp = m_vFreeList[i];
		dp->setPosition(0);
		flushSendPacket(*dp);
	}
	
	m_vFreeList.clear();
}




VOID CSessionCenterClient::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3, UINT64 uParam4)
{

}



VOID CSessionCenterClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{

	
	
	
}



void CSessionCenterClient::HandleLogicRequest(CDataPacketReader &inPacket)
{
	
}

SERVERTYPE CSessionCenterClient::getLocalServerType()
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!修改
	return DBServer;
}

int CSessionCenterClient::getLocalServerIndex()
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!修改
	return 1;
}

LPCSTR CSessionCenterClient::getLocalServerName()
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!修改
	return "";
}

CDataPacket* CSessionCenterClient::AllocSendPacket()
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

void CSessionCenterClient::FreeBackUserDataPacket(CDataPacket *pPacket)
{
	m_vFreeList.append(pPacket);
}