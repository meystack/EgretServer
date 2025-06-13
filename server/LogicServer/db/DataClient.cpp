#include "StdAfx.h"
#include "DataClient.h"
#include "../LogicServer.h"

using namespace jxSrvDef;
using namespace jxInterSrvComm;
using namespace jxInterSrvComm::DbServerProto;


CDataClient::CDataClient(CLogicServer * pLogicServer)
{
	m_pLogic = pLogicServer; 
	SetClientName(_T("数据"));
	m_vFreeList.setLock(&m_vFreeListLock);

	InitializeCriticalSection( &m_SessionLock );
}


CDataClient::~CDataClient()
{
	m_vFreeList.flush();
	//清除包
	for(int i = 0; i < m_vFreeList.count(); i++)
	{
		CDataPacket* dp = m_vFreeList[i];
		dp->setPosition(0);
		flushSendPacket(*dp);
	}
	m_vFreeList.clear();
	DeleteCriticalSection( &m_SessionLock );

}

VOID CDataClient::OnLoadActorMsg(CDataPacketReader &inPacket)
{	
	LOADACTORSTRUCT tagSession;
	inPacket >> tagSession.nSeesionId;
	inPacket >> tagSession.nActorid;
	tagSession.dwTime = _getTickCount() + KEEPSESSIONDATA;
	OutputMsg(rmNormal, _T("%s account:%d actor:%d"), __FUNCTION__, tagSession.nSeesionId, tagSession.nActorid);
	//OutputMsg(rmNormal,_T("OnLoadActorMsg, accountid=%d,expird time=%d"), tagSession.nSeesionId,(int)tagSession.dwTime );

	EnterCriticalSection( &m_SessionLock );
	m_vSeesionData.add(tagSession);
	LeaveCriticalSection(&m_SessionLock);

	//告诉数据服务器，玩家的登陆数据已经收到了

	CDataPacket& DataPacket = allocProtoPacket(dcResActorDbSessionData);
	DataPacket << tagSession.nSeesionId;
	DataPacket << tagSession.nActorid;
	flushProtoPacket(DataPacket);

}


VOID CDataClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if(nCmd <=0) return; //是心跳包

	//获取角色列表或者创建账户
	if(dcGetActorList == nCmd   || dcCreateActor == nCmd || dcDeleteActor ==  nCmd || dcGetRandName == nCmd ||
		dcCheckSecondPsw == nCmd || dcCreateSecondPsw == nCmd)
	{
		CDataPacket* outPacket = AllocSendPacket();
		if(outPacket ==NULL) return;
		outPacket->setPosition(0);
		BYTE bGateIndex =0;
		int nRawServerindex=0,nServerindex =0;

		inPacket >> nRawServerindex >> nServerindex >> bGateIndex;

		outPacket->writeBuf(inPacket.getOffsetPtr(),inPacket.getLength() - inPacket.getPosition());
		GetLogicServer()->GetGateManager()->PostInternalMessage(CCustomGateManager::SGIM_RECV_LOGIN_DATA,
			nCmd,(Uint64)outPacket,bGateIndex,0);
	}
	else if(nCmd == dcLoadActor)  //这个是数据服务器发过来的，表示玩家登陆了
	{
		OnLoadActorMsg(inPacket);
	}
	else if(nCmd == dcGetCSActorId || nCmd == dcCreateCSActorId)  //这个是数据服务器发过来的
	{
		// int nRawServerindex=0,nServerindex =0;
		// int nActorId = 0;
		// int nCSActorId = 0;
		// inPacket >> nRawServerindex >> nServerindex;
		CDataPacket* outPacket = AllocSendPacket();
		if(outPacket ==NULL) return;
		outPacket->setPosition(0);
		// (*outPacket) << nRawServerindex >> nServerindex;
		outPacket->writeBuf(inPacket.getOffsetPtr(),inPacket.getLength() - inPacket.getPosition());
		CLogicServer::s_pLogicEngine->GetNetWorkHandle()->PostInternalMessage(SSM_CROSS_SERVER_DB_DATA,
			nCmd,(Uint64)outPacket);
	}
	else
	{
		CDataPacket* outPacket = AllocSendPacket();
		if(outPacket ==NULL) 
		{
			//Assert(FALSE);
			return;
		}
		outPacket->writeBuf(inPacket.getOffsetPtr(),inPacket.getLength() - inPacket.getPosition());
		//这里把cmd和数据包转发过去
		CLogicServer::s_pLogicEngine->GetNetWorkHandle()->PostInternalMessage(SSM_DB_RET_DATA,nCmd,(INT_PTR)outPacket);
	}
}

jxSrvDef::SERVERTYPE CDataClient::getLocalServerType()
{
	return GameServer;
}

LPCSTR CDataClient::getLocalServerName()
{
	return m_pLogic->getServerName();
}

int CDataClient::getLocalServerIndex()
{
	return m_pLogic->GetServerIndex();
}

VOID CDataClient::OnConnected()
{
	CLogicEngine* pEngine = GetGlobalLogicEngine();
	if (pEngine)
	{
		CDataPacket& DataPacket = allocProtoPacket(dcInitDB);
		DataPacket << GetLogicServer()->GetServerIndex();
		flushProtoPacket(DataPacket);

		pEngine->InitComponent();
		pEngine->GetGuildMgr().Load();		//加载行会数据

		//如果 启动好了会自己去连接，防止断开重连的情况
		if( !GetLogicServer()->IsCommonServer() )
		{
			if(pEngine->IsEngineerRuning()) //如果已经启动了 
			{
				GetLogicServer()->ReloadCrossConfig();
			}
		}
		
	}
}

BOOL CDataClient::OnQuerySessionData(UINT_PTR nSessionId,unsigned int nActorId)
{
	BOOL result = FALSE;
	Uint64 dwCurrTime = _getTickCount();
	//这里要加锁，并且遍历搜索，性能不高。不过这个队列内容应该不多
	EnterCriticalSection( &m_SessionLock );
	INT_PTR nCount = m_vSeesionData.count();
	bool isFind = false; //是否找到了
	for(INT_PTR i = nCount-1; i >= 0; i--)
	{
		LOADACTORSTRUCT& data = m_vSeesionData[i];
		
		if (data.dwTime < dwCurrTime && !IsDebuggerPresent() )
		{
			//清除掉过期的
			if(data.nActorid == nActorId && data.nSeesionId == nSessionId)
			{
				OutputMsg(rmError,_T(" sessionid=%d OnQuerySessionData ,找到时候,但是超时 %d，curretTime=%d"),data.nSeesionId,int(dwCurrTime - data.dwTime), (int)dwCurrTime );
				m_vSeesionData.remove(i);
				break;
			}
			else
			{
				OutputMsg(rmError,_T(" sessionid=%d OnQuerySessionData ,超时时间%d过期，curretTime=%d"),data.nSeesionId,(int)(dwCurrTime - data.dwTime), (int)dwCurrTime );
			}
			
		}
		else if (data.nActorid == nActorId && data.nSeesionId == nSessionId)
		{
			//命中
			result = TRUE;
			m_vSeesionData.remove(i);
			isFind =true;
			break;
		}
		

	}
	LeaveCriticalSection(&m_SessionLock);
	return result;
}

void CDataClient::WaitFor()
{
	OutputMsg(rmTip, "WAITING FOR FLUSHING ACTOR DATA...");
	size_t nCount;
	TICKCOUNT dwLastPrintMsgTick = 0;
	CLogicEngine* pEngine = GetGlobalLogicEngine();
	if (pEngine)
	{
		pEngine->SaveComponent();
	}
	
	while(nCount = getPacketCount() > 0)
	{
		Sleep(1);
		if (dwLastPrintMsgTick <= _getTickCount())
		{
			dwLastPrintMsgTick = _getTickCount() + 10000;
			OutputMsg(rmNormal, "%d PACKET(S) REMAIN", nCount);
		}
	}

	OutputMsg(rmTip, "FLUSH ACTOR DATA COMPLETE!");
}


void CDataClient::FreeBackUserDataPacket(CDataPacket* pDataPacket)
{
	m_vFreeList.append(pDataPacket);
}

CDataPacket* CDataClient::AllocSendPacket()
{
	if (m_vFreeList.count() <= 0)
	{
		m_vFreeList.flush();
	}
	if (m_vFreeList.count() <= 0)
	{
		allocSendPacketList(m_vFreeList,512);
	}

	CDataPacket* m_TempData = m_vFreeList.pop();//得到一个空闲的Datapacket
	m_TempData->setPosition(0);
	return m_TempData;
}
