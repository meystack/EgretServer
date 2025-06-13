

#include "StdAfx.h"
#include "FeeDb.h"

bool CDBDataServer::m_bGableIsClose =false;

CDBDataServer::CDBDataServer(CDBServer* lpDBEngine)
{
	m_pDBServer = lpDBEngine;
	SetServiceName("数据");
	m_dwDeleteOutDataTick = 0;
	m_SQLConnection.SetMultiThread(TRUE);	
	TICKCOUNT curTick = _getTickCount();
	m_dwNextDumpTime = curTick + sDumpInterval;	
	m_dwSaveNameFile = curTick + sSaveNameInterval;	
	m_dwNextUpdateClientListTick = curTick + sUpdateLogicClientListInterval;
	m_jobzyMgr.SetParam(&m_NowSQLConnection);
	m_nNextZyJobSaveTime = curTick +  43200000; //12小时存一次盘
	m_pCurClient = NULL;
	m_CloseClientList.empty();
}

CDBDataServer::~CDBDataServer()
{	
}

CCustomServerClientSocket* CDBDataServer::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	// 记录这个DBDataClient
	m_pCurClient = new CDBDataClient(this, &m_NowSQLConnection, nSocket, pAddrIn);
	m_pCurClient->SetNewSQLConnection(&m_SQLConnection);
	return m_pCurClient;
}

VOID CDBDataServer::DestroyClientSocket(CCustomServerClientSocket *pClientSocket)
{
	m_CloseClientLock.Lock();
	CDBDataClient* pDC = (CDBDataClient*)pClientSocket;	
	for(INT_PTR i= m_CloseClientList.count() -1; i >=0; i-- )
	{
		if(m_CloseClientList[i] == pDC)
		{
			m_CloseClientLock.Unlock();
			return;
		}
	}
	OutputMsg(rmNormal, _T("Start to call datahandler.stop()"));
	pDC->m_sDataHandler.Stop();
	OutputMsg(rmNormal, _T("Call datahandler.stop() return"));
	m_CloseClientList.add(pDC);
	m_CloseClientLock.Unlock();
	if ((void*)pClientSocket == (void*)m_pCurClient)
	{
		m_pCurClient = NULL;
	}
}

VOID CDBDataServer::ProcessClients()
{

	Inherited::ProcessClients();
	ProcessClosedClients();
}

VOID CDBDataServer::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	TICKCOUNT nCurrentTick;
	OutputMsg(rmTip, "CDBDataServer::DispatchInternalMessage:%d", uMsg);
	switch(uMsg)
	{
	//向指定的服务器发送打开角色加载认证的消息(Param1=服务器ID,Param2=会话ID,Param3=角色ID)
	case DSIM_POST_OPEN_CHAR_LOAD:
		{
			nCurrentTick = _getTickCount();
			int nTime = (int)(nCurrentTick - uParam4);
			OutputMsg(rmTip,"actorid=%lld,to DispatchInternalMessage, pass=%d ms",uParam3,nTime);

			PostClientInternalMessages(uParam1, DCIM_SEND_LOAD_ACTOR, uParam2, uParam3, nCurrentTick,0);
		}
		break;
	case DCIM_POST_CREATE_ACTOR_RESULT:
		{
			PostClientInternalMessages(uParam1, DCIM_CREATE_ACTOR_RESULT, uParam2, uParam3, uParam4,0);
			break;
		}
	case DCIM_POST_CREATE_CS_ACTOR_RESULT:
		{
			PostClientInternalMessages(uParam1, DCIM_CREATE_CS_ACTOR_RESULT, uParam2, uParam3, uParam4,0);
			break;
		}
	//向指定的服务器返回重命名角色的结果(Param1=服务器ID,Param2=操作结果,Param3=申请更名操作时传递的操作唯一标识)
	case DSIM_POST_RENAMECHAR_RESULT:
		//将此消息转发到指定ID内部数据客户端
		PostClientInternalMessages(uParam1, DCIM_RENAMECHAR_RESULT, uParam2, uParam3, 0);
		break;
	//名称客户端向指定的服务器返回申请帮会ID的结果(Param1=服务器ID,
	//Param2=（2个INT_PTR的数组，[0]保存操作结果，[1]保存帮会ID，处理消息后必须对数组进行free释放）,
	//Param3=申请帮会ID操作时传递的操作唯一标识)
	case DSIM_POST_ALLOC_GUILID_RESULT:
		//将此消息转发到指定ID内部数据客户端
		PostClientInternalMessages(uParam1, DCIM_ALLOC_GUILID_RESULT, ((PINT_PTR)uParam2)[0], ((PINT_PTR)uParam2)[1], uParam3);
		//释放返回值数组
		free((LPVOID)uParam2);
		break;
	}
}

VOID CDBDataServer::OnSocketDataThreadStop()
{
	Inherited::OnSocketDataThreadStop();//调用父类的处理函数以便关闭所有连接

	OutputMsg( rmTip, _T("正在等待所有数据客户端的数据处理完毕……"));
	do 
	{
		SingleRun();
		Sleep(10);
	}
	while ( !AllDataProcessed() );

	//强制释放所有客户端连接占用的内存（销毁连接对象）
	FreeAllClient();
	OutputMsg( rmTip, _T("所有数据客户端的数据已处理完毕！"));
}

VOID CDBDataServer::SingleRun()
{
	//连接数据库
	ConnectSQL();

	Inherited::SingleRun();
}

VOID CDBDataServer::OnRun()
{
	TICKCOUNT dwCurTick = _getTickCount();
	
	////每分钟从数据库中删除一次过期数据，过期数据在保存角色数据的时候将角色对应的
	////物品、技能、任务等数据的charid字段更新为了0（将多次零散的SQL delete操作合并
	////为多次update，一次delete以提高数据库操作性能）。
	if ( m_NowSQLConnection.Connected() && dwCurTick >= m_dwDeleteOutDataTick )
	{
		if(m_dwDeleteOutDataTick)
		{
			//	//删除过期背包物品
			if ( !m_NowSQLConnection.Exec( "delete from actorbagitem where actorid = 0;" ) )
				m_NowSQLConnection.ResetQuery();
			//删除过期装备物品
			if ( !m_NowSQLConnection.Exec( "delete from actorequipitem where actorid = 0;" ) )
				m_NowSQLConnection.ResetQuery();
			//删除过期仓库物品
			if ( !m_NowSQLConnection.Exec( "delete from actordepotitem where actorid = 0;" ) )
				m_NowSQLConnection.ResetQuery();

			//删除过期技能
			if ( !m_NowSQLConnection.Exec( "delete from skill where actorid = 0;" ) )
				m_NowSQLConnection.ResetQuery();
			//删除过期任务进度
			if ( !m_NowSQLConnection.Exec( "delete from goingquest where actorid = 0;" ) )
				m_NowSQLConnection.ResetQuery();
			//删除过期完成任务数据
			if ( !m_NowSQLConnection.Exec( "delete from repeatquest where actorid = 0;" ) )
				m_NowSQLConnection.ResetQuery();
			//删除过期的好友数据
			if ( !m_NowSQLConnection.Exec( "delete from actorfriends where actorid = 0;" ) )
				m_NowSQLConnection.ResetQuery();
		
			//删除过期的宠物数据
			if (!m_NowSQLConnection.Exec("delete from actorpets where actorid=0;"))
				m_NowSQLConnection.ResetQuery();

			if (!m_NowSQLConnection.Exec("delete from actorpetitem where actorid=0;"))
				m_NowSQLConnection.ResetQuery();
		
			if (!m_NowSQLConnection.Exec("delete from petskills where actorid=0;"))
				m_NowSQLConnection.ResetQuery();

			if (!m_NowSQLConnection.Exec("delete from actorrelation where actorid=0;"))
				m_NowSQLConnection.ResetQuery();
			
		}
		m_dwDeleteOutDataTick = dwCurTick + 60000;
		
	}

	if(m_SQLConnection.Connected() && dwCurTick >= m_dwNextDumpTime)
	{
		//每5分钟dump一次
		if(m_dwNextDumpTime)
		{
			CTimeProfMgr::getSingleton().dump();
		}
		m_dwNextDumpTime = dwCurTick + sDumpInterval; //5分钟执行一次dump
	}

	if ( dwCurTick >= m_dwSaveNameFile)//保存随机名字的文件
	{
		if(m_dwSaveNameFile)
		{
			//m_pDBServer->SaveNameFile();
		}
		m_dwSaveNameFile += sSaveNameInterval;//5分钟保存一次
	}

	if (dwCurTick >= m_dwNextUpdateClientListTick)
	{
		UpdateLogicClientList();
		m_dwNextUpdateClientListTick = dwCurTick + sUpdateLogicClientListInterval;
	}

	if (m_nNextZyJobSaveTime <= dwCurTick)
	{
		//现在只有职业数据需要存盘
		if (m_jobzyMgr.HasLoadJobData() && m_NowSQLConnection.Connected())
		{
			m_jobzyMgr.SaveJobData();
		}
		m_nNextZyJobSaveTime = dwCurTick +  43200000; //12个小时存一次
	}

	// 读取充值指令
	if (m_FeeSQLConnection.Connected() && dwCurTick >= m_nNextReadFeeTime)
	{
		
		char token[128];
		if (m_pCurClient && m_pCurClient->connected())
		{
			int nError = m_FeeSQLConnection.Query("call loadfee_notice(%d);", CDBServer::s_pDBEngine->getServerIndex());
			if ( !nError )
			{
				if (MYSQL_ROW pRow = m_FeeSQLConnection.CurrentRow())
				{
					int nCount = m_FeeSQLConnection.GetRowCount();
					CDataPacket& out = m_pCurClient->allocProtoPacket(dcNoticeFee);
					INT_PTR pos = out.getPosition();
					out << (int)0;
					nCount = 0;
					unsigned int nActorId;
					ACCOUNT sAccount;
					do
					{
						sscanf(pRow[0], "%u", &nActorId);
						strlcpy((sAccount), (pRow[1]? pRow[1] : ""), sizeof(sAccount));
						if(nActorId != 0) {
							if (m_ActorIdMap[nActorId])
							{
								nCount++;
								OutputMsg(rmTip,_T("[Fee]0-1 充值通知（在线）：ActorId（%d），Account（%s）"),nActorId,sAccount);
								out << nActorId;
								out.writeString(sAccount);
							} else {
								OutputMsg(rmTip,_T("[Fee]0-2 充值通知（离线）：ActorId（%d），Account（%s）"),nActorId,sAccount);
							}
						} else {
							nCount++;
							OutputMsg(rmTip,_T("[Fee]0-3 充值通知（不指定角色）：ActorId（%d），Account（%s）"),nActorId,sAccount);
							out << nActorId;
							out.writeString(sAccount);
						}
					}
					while(pRow = m_FeeSQLConnection.NextRow());
					int* pCount = (int*)out.getPositionPtr(pos);
					*pCount = nCount;
					m_pCurClient->flushProtoPacket(out);
				}
				m_FeeSQLConnection.ResetQuery();
			}
			else
			{
				OutputMsg(rmTip, _T("m_FeeSQLConnection serverindex:%d nError:%d"),CDBServer::s_pDBEngine->getServerIndex(), nError);
			}
			m_nNextReadFeeTime = dwCurTick + 1000;
		}
	}
}

void CDBDataServer::UpdateLogicClientList()
{	
	char szData[10240];
	CDataPacket packet(szData, sizeof(szData));
	packet.setLength(0);
	size_t nPos = packet.getPosition();
	CBaseList<int> sList;	
	CDBDataClient *pClient;
	for (INT_PTR i = 0; i < m_ClientList.count(); i++)
	{
		pClient = (CDBDataClient *)m_ClientList[i];
		if (pClient->registed())
		{
			sList.add((int)pClient->getClientServerIndex());
		}
	}
	//OutputMsg(rmError, _T("%s update client list, count=%d"), __FUNCTION__, (int)sList.count());
	
	/*
	CDBCenterClient *pDBCenterClient = GetDBEngine()->getDBCenterClient();
	pDBCenterClient->PostUpdateLogicClientList(sList);
	*/
}


void CDBDataServer::Trace()
{
	OutputMsg(rmNormal,"CDBDataServer Trace start...");

	CDBDataClient *pClient;
	for (INT_PTR i = 0; i < m_ClientList.count(); i++)
	{
		pClient = (CDBDataClient *)m_ClientList[i];
		if (pClient && pClient->registed() )
		{
			pClient->Trace();
		}
	}
	OutputMsg(rmNormal,"CDBDataServer Trace end...");
}



INT_PTR CDBDataServer::SendDataClientMsg(const INT_PTR nServerIndex, LPCVOID lpData, SIZE_T dwSize)
{
	INT_PTR i, nResult = 0;
	CDBDataClient *pClient;

	for (i=m_ClientList.count()-1; i>-1; --i)
	{
		pClient = (CDBDataClient*)m_ClientList[i];
		if ( pClient->registed() )
		{
			if ( !nServerIndex || pClient->getClientServerIndex() == nServerIndex )
			{
				pClient->AppendSendBuffer(lpData, dwSize);
				nResult++;
			}
		}
	}
	return nResult;
}



BOOL CDBDataServer::getGameReady(int nSrvIdx)
{
	INT_PTR i;
	BOOL result = FALSE;
	CDBDataClient *pClient;

	for (i=m_ClientList.count()-1; i>-1; --i)
	{
		pClient = (CDBDataClient*)m_ClientList[i];
		if ( pClient->registed() )
		{
			if ( !nSrvIdx || pClient->getClientServerIndex() == nSrvIdx )
			{
				result = TRUE;
				break;
			}
		}
	}
	return result;
}

INT_PTR CDBDataServer::PostClientInternalMessages(UINT64 nServerIndex, UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	INT_PTR i, nResult = 0;
	CDBDataClient *pClient;

	for (i=m_ClientList.count()-1; i>-1; --i)
	{
		pClient = (CDBDataClient*)m_ClientList[i];
		if ( pClient->registed() )
		{
			if ( !nServerIndex || pClient->getClientServerIndex() == nServerIndex )
			{
				pClient->PostInternalMessage(uMsg, uParam1, uParam2, uParam3,uParam4);
				nResult++;
			}
		}
	}
	return nResult;
}

BOOL CDBDataServer::ConnectSQL()
{
	if ( !m_SQLConnection.Connected() )
	{
		if ( _getTickCount() >= m_dwReconnectSQLTick )
		{
			m_pDBServer->SetupSQLConnection(&m_SQLConnection);
			if ( m_SQLConnection.Connect() )
			{
				if (m_pDBServer->IsUtf8())
				{
					if (mysql_set_character_set(m_SQLConnection.GetMySql(),"utf8"))
					{
						OutputMsg( rmError, _T("设置utf8编码出错 !!!") );
					}
				}
				
				//mysql_options(m_SQLConnection.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
				
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
			//如果连接SQL失败则将在5秒后重试
			m_dwReconnectSQLTick = _getTickCount() + 5 * 1000;
			return FALSE;
		}
	}
	
	if (!m_FeeSQLConnection.Connected())
	{
		if ( _getTickCount() >= m_dwReconnectSQLTick )
		{
			m_pDBServer->SetupSQLConnection(&m_FeeSQLConnection);
			OutputMsg(rmTip, _T("m_FeeSQLConnection fail "));
			if ( m_FeeSQLConnection.Connect() )
			{
				OutputMsg(rmTip, _T("m_FeeSQLConnection success"));
				return TRUE;
			}
			//如果连接SQL失败则将在5秒后重试
			m_dwReconnectSQLTick = _getTickCount() + 5 * 1000;
			return FALSE;
		}
	}

	if (!m_NowSQLConnection.Connected())
	{
		if ( _getTickCount() >= m_dwReconnectSQLTick )
		{
			m_pDBServer->SetupSQLConnection(&m_NowSQLConnection);
			OutputMsg(rmTip, _T("m_NowSQLConnection fail "));
			if ( m_NowSQLConnection.Connect() )
			{
				if (m_pDBServer->IsUtf8())
				{
					if (mysql_set_character_set(m_NowSQLConnection.GetMySql(),"utf8"))
					{
						OutputMsg( rmError, _T("设置utf8编码出错 !!!") );
					}
				}
				m_jobzyMgr.LoadJobInitData();
				//m_jobzyMgr.LoadZyInitData(); //阵营的数据暂时屏蔽
				m_jobzyMgr.LoadActorNameInitData();
				m_jobzyMgr.LoadGuildNameInitData();//行会名字
				OutputMsg(rmTip, _T("NowSQL mysql connection character set: %s"), mysql_character_set_name(m_NowSQLConnection.GetMySql()));
				// OutputMsg(rmTip, _T("m_NowSQLConnection success()"));
				return TRUE;
			}
			//如果连接SQL失败则将在5秒后重试
			m_dwReconnectSQLTick = _getTickCount() + 5 * 1000;
			return FALSE;
		}
	}
	
	return TRUE;
}

VOID CDBDataServer::ProcessClosedClients()
{
	INT_PTR i;
	CDBDataClient *pClient;
	TICKCOUNT dwCurTick = _getTickCount();
	//必须降序循环，因为连接可能在循环中被移除
	
	for ( i=m_CloseClientList.count()-1; i>-1; --i )
	{
		pClient = m_CloseClientList[i];
		//连接被关闭后依然要调用Run函数，因为可能有重要的网络数据或逻辑数据没有处理完
		pClient->Run();
		//连接关闭5分钟后再释放连接对象
		if ((dwCurTick - pClient->m_dwClosedTick >= 5 * 60 * 1000) || pClient->m_dwClosedTick == 0)
		{
			m_CloseClientList.remove(i);		
			delete pClient;
		}
	}
	
}

BOOL CDBDataServer::AllDataProcessed()
{
	INT_PTR i;
	CDBDataClient *pClient;

	//如果消息队列非空则仍需继续处理
	if ( getInternalMessageCount() > 0 )
		return FALSE;

	//判断数据客户端的数据是否处理完毕
	for ( i=m_ClientList.count()-1; i>-1; --i )
	{
		pClient = (CDBDataClient*)m_ClientList[i];
		if ( !pClient->registed() )
			continue;
		if ( pClient->HasRemainData() )
			return FALSE;
	}
	for ( i=m_CloseClientList.count()-1; i>-1; --i )
	{
		pClient = m_CloseClientList[i];
		if ( !pClient->registed() )
			continue;
		if ( pClient->HasRemainData() )
			return FALSE;
	}
	return TRUE;
}

VOID CDBDataServer::FreeAllClient()
{
	INT_PTR i;
	CDBDataClient *pClient;

	//关闭所有客户端
	CloseAllClients();

	//销毁所有连接
    
	for ( i=m_CloseClientList.count()-1; i>-1; --i )
	{
		pClient = m_CloseClientList[i];
		//delete pClient;
		pClient->m_dwClosedTick = 0;
	}
	//m_CloseClientList.clear();
	
}

INT_PTR CDBDataServer::GetAvailableDataClientCount(const INT_PTR nServerIndex)
{
	//OutputMsg( rmTip, _T("GetAvailableDataClientCount():nServerIndex=%d"),nServerIndex );
	if ( nServerIndex == 0 )
	{
		
		return m_ClientList.count();
	}

	INT_PTR i, nResult = 0;
	CDBDataClient *pClient;

	m_ClientList.lock();
	for ( i=m_ClientList.count() - 1; i > - 1; --i )
	{
		pClient = (CDBDataClient*)m_ClientList[i];
		if ( pClient->connected() && pClient->getClientServerIndex() == nServerIndex )
		{

			nResult++;
		}
		else
		{
			//test
			//OutputMsg( rmError, _T("connected()=%d,ClientIndex=%d"),(int)(pClient->connected()),pClient->getClientServerIndex() );
		}
	}
	m_ClientList.unlock();
	return nResult;
}

BOOL CDBDataServer::IsCharSavedFailure(const INT_PTR nCharId)
{
	//测试，先返回FALSE
	return FALSE;
}
