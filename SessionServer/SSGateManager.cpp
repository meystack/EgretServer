
#include "StdAfx.h"


/*

CSSGateManager::CSSGateManager(CSSManager *lpSSManager)
:Inherited()
{
	m_dwReconnectSQLTick = 0;
	m_pSSManager = lpSSManager;
	m_SQLConnection.SetMultiThread(TRUE);
}

CSSGateManager::~CSSGateManager()
{
}

BOOL CSSGateManager::ConnectSQL()
{
	TICKCOUNT nCurrentTick = _getTickCount();
	if ( !m_SQLConnection.Connected() )
	{
		
		if (  nCurrentTick >= m_dwReconnectSQLTick )
		{
			if (NULL ==m_pSSManager) return FALSE;
			m_pSSManager->SetupSQLConnection(&m_SQLConnection);
			if ( m_SQLConnection.Connect() )
			{
				mysql_options(m_SQLConnection.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
				//if (m_SQLConnection.Exec("charset utf8"))
				//{
				//	OutputMsg( rmError, _T("设置utf8字符编码出错！！！"));
				//}
				//else
				//{
				//	m_SQLConnection.ResetQuery();
				//}
				return TRUE;
			}
			OutputMsg( rmError, "连接数据库ip=%s,port=%d失败",
				m_SQLConnection.GetServerHost(),m_SQLConnection.GetServerPort());

			//如果连接SQL失败则将在5秒后重试
			m_dwReconnectSQLTick = _getTickCount() + 5 * 1000;
			return FALSE;
		}
	}
	else
	{
		if(  nCurrentTick> m_dwDumpTime)
		{
			if(m_dwDumpTime )
			{
				if(m_dwDumpTime)
				{
					CTimeProfMgr::getSingleton().dump();
				}
			}
			m_dwDumpTime = nCurrentTick + 300000; //5分钟打印一次
		}
	}
	return TRUE;
}

VOID CSSGateManager::ResultUserSessionConfim(const int nSessionId, const BOOL boOnline)
{
	CSSGate *pGate;
	for ( int i=0; i<ArrayCount(m_Gates); ++i )
	{
		pGate = &m_Gates[i];
		if (pGate && pGate->m_UserList.count() > 0)
		{
			if ( pGate->ResultGateUserSessionConfim(nSessionId, boOnline) )
			{
				break;
			}
		}
	}
}

//打开一个session
VOID CSSGateManager::OnResultConformOpenSession(int nSessionId, int nServerIndex)
{
	CSSGate *pGate;
	for ( int i=0; i<ArrayCount(m_Gates); ++i )
	{
		pGate = &m_Gates[i];
		if (pGate && pGate->m_UserList.count() > 0)
		{
			if ( pGate->ResultOpenSession(nSessionId,nServerIndex) )
			{
				break;
			}
		}
	}
}

VOID CSSGateManager::Initialize()
{
	int i;
	for ( i=0; i<ArrayCount(m_Gates); ++i )
	{
		m_Gates[i].m_pSQLConnection = &m_SQLConnection;
		m_Gates[i].m_pSSManager = m_pSSManager;

		//将网关添加到列表
		m_GateList[i] = &m_Gates[i];
	}
	m_dwDumpTime =0;
}

VOID CSSGateManager::Uninitialize()
{
	int i;
	for ( i=0; i<ArrayCount(m_GateList); ++i )
	{
		//将网关从列表移除
		m_GateList[i] = NULL;
	}
}


VOID CSSGateManager::OnResultGateUserExists(int nSessionId, const BOOL boOnline)
{
}

VOID CSSGateManager::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3, UINT64 uParam4)
{
	switch (uMsg)
	{
	case GTIM_CONFIM_SESSION_RESULT://会话服务器向网关用户投递确认会话是否在线的结果(Param1=会话ID，Param2=是否在线(0:1))
		ResultUserSessionConfim((int)uParam1, (int)uParam2);
		break;
		//确认已经打开了会
	case GTIM_CONFIRM_OPEN_SESSION:
		OnResultConformOpenSession((int)uParam1,(int)uParam2);
		break;
	default:
		Inherited::DispatchInternalMessage(uMsg, uParam1, uParam2, uParam3,uParam4);
		break;
	}
} 

VOID CSSGateManager::OnRun()
{
	if ( Started() )
	{
		ConnectSQL();
	}
}
*/

