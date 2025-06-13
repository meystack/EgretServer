#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif
#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"

#include "CommonDef.h"
#include "ServerDef.h"


#include "SendPackPool.h"
#include "CustomWorkSocket.h"

#include "CustomServerClientSocket.h"


#include "CustomClientSocket.h"
#include "CustomJXClientSocket.h"
#include "CustomJXServerClientSocket.h"

#include "InterServerComm.h"
//#include "../encrypt/CRC.h"
//#include "../encrypt/Encrypt.h"
//#include "../dataProcess/NetworkDataHandler.h"
#include "CustomJXClientSocket.h"
#include "CustomGlobalSession.h"
#include "CustomSessionClient.h"

using namespace jxSrvDef;
using namespace jxInterSrvComm;

const CCustomSessionClient::OnHandleSockPacket CCustomSessionClient::SSSockPacketHandlers[] = 
{
	&CCustomSessionClient::CatchDefaultPacket,
	&CCustomSessionClient::CatchOpenSession,
	&CCustomSessionClient::CatchCloseSession,
	&CCustomSessionClient::CatchUpdateSession,
	&CCustomSessionClient::CatchQuerySessionExists,
};

CCustomSessionClient::CCustomSessionClient()
:Inherited()
{
	SetClientName(_T("会话"));
}

CCustomSessionClient::~CCustomSessionClient()
{
}

VOID CCustomSessionClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if ( nCmd >= ArrayCount(SSSockPacketHandlers) )
	{
		OutputMsg(rmWaning, _T("SessionServer[%s:%d]has sent invalid packet(%d)，datalen：%d"),
			GetServerHost(), GetServerPort(), nCmd, inPacket.getLength());
	}
	else
	{
		(this->*SSSockPacketHandlers[nCmd])(inPacket);
	}
}

VOID CCustomSessionClient::CatchDefaultPacket(CDataPacketReader &inPacket)
{
}
	
VOID CCustomSessionClient::CatchOpenSession(CDataPacketReader &inPacket)
{
	BOOL boNewSession = FALSE;
	GLOBALSESSIONOPENDATA SessionData;

	inPacket >> SessionData;
	OutputMsg(rmTip,"%s sessionid=%d",__FUNCTION__,SessionData.nSessionId);
	if ( SessionData.nSessionId && SessionData.sAccount[0] )
	{
		CCustomGlobalSession* pSession = GetSessionPtr(SessionData.nSessionId, NULL,SessionData.lKey);
		if ( !pSession )
		{
			pSession = CreateSession();
			//填充会话基本数据
			pSession->nSessionId = SessionData.nSessionId;
			pSession->nServerIndex = SessionData.nServerIndex;
			pSession->nRawServerId = SessionData.nRawServerId;
			pSession->nClientIPAddr = SessionData.nClientIPAddr;
			pSession->dwSessionTick = _getTickCount();
			pSession->nState = SessionData.eState;
			pSession->dwFCMOnlineSec = SessionData.dwFCMOnlineSec;
			pSession->nGmLevel =  SessionData.nGmLevel; //账户的GM等级
			_asncpytA(pSession->sAccount, SessionData.sAccount);
			pSession->lKey =  SessionData.lKey;
			pSession->nGateIndex =  SessionData.nGateIndex; 

			//将会话添加到全局会话队列中
			m_SessionLock.Lock();
			m_SessionList.add(pSession);
			m_SessionLock.Unlock();
			boNewSession = TRUE;

			//test
			//OutputMsg(rmTip,"CustomSeesionClient Open Session,nSessionId=%d,nServerIndex=%d",
			//	pSession->nSessionId,pSession->nServerIndex);
		}
		else
		{
			//设置成新的值
			pSession->nSessionId = SessionData.nSessionId;
			pSession->nServerIndex = SessionData.nServerIndex;
			pSession->nRawServerId = SessionData.nRawServerId;
			pSession->nClientIPAddr = SessionData.nClientIPAddr;
			pSession->dwSessionTick = _getTickCount();
			pSession->nState = SessionData.eState;
			pSession->dwFCMOnlineSec = SessionData.dwFCMOnlineSec;
			pSession->nGmLevel = SessionData.nGmLevel; 
			_asncpytA(pSession->sAccount, SessionData.sAccount);
			pSession->lKey =  SessionData.lKey;
			pSession->nGateIndex =  SessionData.nGateIndex; 
		}
		
		
			
		OnOpenSession(pSession, boNewSession);
	}
}

VOID CCustomSessionClient::CatchCloseSession(CDataPacketReader &inPacket)
{
	int nSessionId;
	UINT64 lKey =0;  //玩家的key

	inPacket >> nSessionId >>lKey ;//<<----从通信数据包中获取全局会话ID
	OutputMsg(rmTip,"%s sessionid=%d",__FUNCTION__,nSessionId);
	INT_PTR nIdx;
    CCustomGlobalSession *pSession = GetSessionPtr(nSessionId, &nIdx,lKey);
	if ( pSession )
	{
		//调用会话删除的通知函数
		OnCloseSession(pSession);
		//test
		//OutputMsg(rmTip,"CustomSeesionClient Close Session,nSessionId=%d,nServerIndex=%d",
		//	pSession->nSessionId,pSession->nServerIndex);

		m_SessionList.remove(nIdx);
		DestroySession(pSession);
	}
	else
	{
		OutputMsg(rmTip,"CatchCloseSession id=%d, pSession is NULL",nSessionId);
	}
}

VOID CCustomSessionClient::CatchUpdateSession(CDataPacketReader &inPacket)
{
	int nSessionId = 0;
	GSSTATE nNewState = gsSelChar;
	UINT64 lKey =0;
	inPacket >> nSessionId;//<<----从通信数据包中获取全局会话ID
	inPacket >> nNewState;//<----从通信数据包中获取会话的状态值
	inPacket >> lKey;//<----
	CCustomGlobalSession *pSession = GetSessionPtr( nSessionId, NULL,lKey );
	if ( pSession )
	{
		GSSTATE nOldState = pSession->nState;
		pSession->nState = nNewState;
		//调用关于会话状态变更的通知函数
		OnUpdateSessionState(pSession, nOldState, nNewState);
	}
}

VOID CCustomSessionClient::CatchQuerySessionExists(CDataPacketReader &inPacket)
{
	int nSessionId = 0;
	inPacket >> nSessionId;//<<----从通信数据包中获取全局会话ID
	//调用查询会话是否在线的函数
	OnQuerySessionExists(nSessionId);
}

/*
void CCustomSessionClient::SendKeepAlive()
{
#pragma __CPMSG__(发送保持连接的心跳包)
	//flushProtoPacket(allocProtoPacket(？？));
}
*/

CCustomGlobalSession* CCustomSessionClient::GetSessionPtr(const INT_PTR nSessionID, PINT_PTR pIndex,UINT64 lKey)
{
	INT_PTR i;
	CCustomGlobalSession **pSessionList;

	pSessionList = m_SessionList;
	for (i=m_SessionList.count()-1; i>-1; --i)
	{
		if ( pSessionList[i]->nSessionId == nSessionID && pSessionList[i]->lKey ==  lKey)
		{
			if (pIndex) *pIndex = i;
			return pSessionList[i];
		}
	}
	return NULL;
}

VOID CCustomSessionClient::ClearSessionList()
{
	INT_PTR i;
	CCustomGlobalSession **pSessionList;
	CSafeLock sl(&m_SessionLock);

	pSessionList = m_SessionList;
	for (i=m_SessionList.count()-1; i>-1; --i)
	{
		if ( pSessionList[i] )
		{
			DestroySession(pSessionList[i]);
		}
	}
	m_SessionList.clear();
}

VOID CCustomSessionClient::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	CCustomGlobalSession *pSession;
	WORD cmd = 0;
	switch(uMsg)
	{
	case SSIM_RESULT_SESSION_EXISTS:
		cmd = jxInterSrvComm::SessionServerProto::cQueryResult;
		break;

	case SSIM_CHG_SESSION_STATE:
		cmd = jxInterSrvComm::SessionServerProto::cUpdateSession ;
		break;

	case SSIM_CLOSE_SESSION:
		cmd = jxInterSrvComm::SessionServerProto::cCloseSession ;
		break;
	default:
		return;
	}
	
	CDataPacket &data = allocProtoPacket(cmd); //分配一个 网络包
	switch (uMsg)
	{
	case SSIM_RESULT_SESSION_EXISTS://返回查询会话是否在线的结果 uParam1=全局会话ID，uParam2=在线?1:0
		{
			//返回会话服务器用户是否存在
			data << (int) uParam1;
			data << (BYTE) uParam2;
			break;
		}
	case SSIM_CHG_SESSION_STATE://改变会话状态 uParam1=全局会话ID，uParam2=会话状态，值为GSSTATE的枚举值
		{
			pSession = GetSessionPtr(uParam1, NULL,uParam3);
			if ( pSession )
			{
				pSession->nState = (GSSTATE)uParam2;
				//向会话服务器发送改变会话状态的消息
				data << (int) uParam1;
				data << (int) uParam2;
			}
			else
			{
				OutputMsg(rmWaning,_T("SSIM_CHG_SESSION_STATE 改变会话的时候没有找到需要改变状态的会话%d\n"), (int)uParam1);
			}
			
		}
		break;
	case SSIM_CLOSE_SESSION://关闭会话 uParam1=全局会话ID
		{
			pSession = GetSessionPtr(uParam1, NULL,uParam2);
			if ( pSession )
			{
				//如果会话的状态不是等待连接下一个服务器的状态则关闭会话
				//if ( pSession->nState != gsWaitQueryChar && pSession->nState != gsWaitEntryGame )
				data << (int) uParam1 << UINT64(uParam2);
			}
			else
			{
				data << (int) 0 << 0;
				OutputMsg(rmWaning,_T("SSIM_CLOSE_SESSION 没有找到需要关闭的会话%d\n"), (int)uParam1);
			}
		}
		break;
	}

	flushProtoPacket(data);
}

CCustomGlobalSession* CCustomSessionClient::CreateSession()
{
	return (CCustomGlobalSession*)m_Allocator.AllocBuffer(sizeof(CCustomGlobalSession));
}

VOID CCustomSessionClient::DestroySession(CCustomGlobalSession* pSession)
{
	m_Allocator.FreeBuffer(pSession);
}



VOID CCustomSessionClient::OnWorkThreadStop()
{
	ClearSessionList();
}

bool CCustomSessionClient::GetSessionData(const int nSessionID,UINT64 lKey,OUT PGLOBALSESSIONOPENDATA pSessionData)
{
	INT_PTR i;
	bool boResult = false;
	CCustomGlobalSession **pSessionList, *pSession;
	CSafeLock sl(&m_SessionLock);

	pSessionList = m_SessionList;
	for (i=m_SessionList.count()-1; i>-1; --i)
	{
		if ( pSessionList[i]->nSessionId == nSessionID && lKey== pSessionList[i]->lKey)
		{
			pSession = pSessionList[i];
			//填充会话基本数据
			pSessionData->nSessionId = (INT)pSession->nSessionId;
			pSessionData->nServerIndex = (INT)pSession->nServerIndex;
			pSessionData->nRawServerId = pSession->nRawServerId;
			pSessionData->nClientIPAddr = pSession->nClientIPAddr;
			pSessionData->dwFCMOnlineSec = pSession->dwFCMOnlineSec;
			pSessionData->eState = pSession->nState;
			pSessionData->nGmLevel = pSession->nGmLevel; //GM等级
			_asncpytA(pSessionData->sAccount, pSession->sAccount);
			boResult = true;
		}
	}
	return boResult;
}

VOID CCustomSessionClient::OnConnected()
{
	Inherited::OnConnected();
}



