
#include "StdAfx.h"


bool CDBGate::m_bGableIsClose =false;

CDBGate::CDBGate()
:Inherited()
{
	ZeroMemory(&m_MaxTimeUserMsgPerformance, sizeof(m_MaxTimeUserMsgPerformance));
	AddAllGateUsersToFreeList();
}

CDBGate::~CDBGate()
{
}


VOID CDBGate::AddAllGateUsersToFreeList()
{
	int i;

	m_FreeUserList.clear();
	for ( i=ArrayCount(m_GateUsers)-1; i>-1; --i )
	{
		m_FreeUserList.add( &m_GateUsers[i] );
	}
}

CCustomServerGateUser* CDBGate::CreateGateUser()
{
	INT_PTR nCount = m_FreeUserList.count();
	if ( nCount > 0 )
	{
		nCount--;
		CCustomServerGateUser* pUser = m_FreeUserList[nCount];
		m_FreeUserList.remove(nCount);
		return pUser;
	}
	return NULL;
}

VOID CDBGate::DestroyGateUser(CCustomServerGateUser* pUser)
{
	m_FreeUserList.add(pUser);
}

VOID CDBGate::OnOpenUser(CCustomServerGateUser *pGateUser)
{
	CDBGateUser* pDBGateUser = (CDBGateUser*)pGateUser;
	pDBGateUser->nGlobalSessionId = 0;
	pDBGateUser->sAccount[0] = 0;
	pDBGateUser->dwCreateCharTimeOut = 0;
	pDBGateUser->pCreateCharStruct = NULL;
}

VOID CDBGate::OnCloseUser(CCustomServerGateUser *pGateUser)
{
	//OutputMsg(rmTip,"DBGate OnCloseUser!");
	CDBGateUser* pDBGateUser = (CDBGateUser*)pGateUser;
	int nSessionId = pDBGateUser->nGlobalSessionId;

	pDBGateUser->nGlobalSessionId = 0;
	pDBGateUser->sAccount[0] = 0;
	pDBGateUser->dwCreateCharTimeOut = 0;
	if ( pDBGateUser->pCreateCharStruct )
	{
		m_Allocator.FreeBuffer(pDBGateUser->pCreateCharStruct);
		pDBGateUser->pCreateCharStruct = NULL;
	}
	if ( nSessionId != 0 )
	{
		
		//如果会话状态不是“查询角色”则关闭会话w

		/*
		GLOBALSESSIONOPENDATA session;
		if (m_pDBServer->GetSessionData(nSessionId, &session) && session.eState != gsSelChar)
		{
			m_pDBServer->PostSSClientUserClosed(nSessionId);
		}
		*/
	}
}

VOID CDBGate::OnGateClosed()
{
}

void CDBGate::OnDispatchUserMsg(CCustomServerGateUser *pGateUser, char* lpData, SIZE_T nSize)
{
	CDBGateUser* pDBGateUser = (CDBGateUser*)pGateUser;
	DECLARE_FUN_TIME_PROF()
	m_MaxTimeUserMsgPerformance.dwTickBegin = _getTickCount();

	CDataPacket* pResult = NULL;
	WORD nCmdId = (WORD)(*lpData);
	switch (nCmdId)
	{
		case QUERYACTORLISTREQ_CMD://处理查询角色请求包
			{
				pResult = QueryActorList(pDBGateUser,lpData,nSize);
				DeleteOldActor(pDBGateUser);
				break;
			}
		case CREATEACTORDATAREQ_CMD:
			{
				pResult = CreateActor(pDBGateUser,lpData,nSize);
				break;
			}
		case DELETEACTORDATAREQ_CMD:
			{
				pResult = DeleteActor(pDBGateUser,lpData,nSize);
				break;
			}
		case ENTRYGAMEREQ_CMD:
			{
				pResult = EntryGameReq(pDBGateUser,lpData,nSize);
				break;
			}
		case RANDNAMEREQ_CMD:
			{
				pResult = RandNameReq(pDBGateUser,lpData,nSize);
				break;
			}
		case LESSJOBREQ_CMD:
			{
				pResult = QueryLessJobReq(pDBGateUser,lpData,nSize);
				break;
			}
		case LESSZYREQ_CMD:
			{
				pResult = QueryZYReq(pDBGateUser,lpData,nSize);
				break;
			}
		default:
			break;
	}
	/********end zac*********/

	/********zac 2010-11-23*/
	if (pResult != NULL)
	{
		//发送结果包
		FlushGateSendPacket(*pResult);
	}
	
	m_MaxTimeUserMsgPerformance.dwLastTick = _getTickCount() - m_MaxTimeUserMsgPerformance.dwTickBegin;
	if ( m_MaxTimeUserMsgPerformance.dwLastTick < m_MaxTimeUserMsgPerformance.dwMinTick ) 
	{
		m_MaxTimeUserMsgPerformance.dwMinTick = m_MaxTimeUserMsgPerformance.dwLastTick;
	}
	if ( m_MaxTimeUserMsgPerformance.dwLastTick > m_MaxTimeUserMsgPerformance.dwMaxTick )
	{
		m_MaxTimeUserMsgPerformance.dwMaxTick = m_MaxTimeUserMsgPerformance.dwLastTick;
	}
	return;
}

VOID CDBGate::OnRun()
{
	Inherited::OnRun();
	ProcessUsers();
}

int CDBGate::UserSelCharEntryGame(CDBGateUser *pGateUser, int nCharId,
	char* sGateHost,SIZE_T nHostLen,PINT_PTR nGatePort)
{
	DECLARE_FUN_TIME_PROF()
	//DECLARE_FUN_TIME_PROF();
	GLOBALSESSIONOPENDATA SessionData;
		
	int nError;

	//首先判断网关用户是否通过查询角色消息设定了全局会话ID和帐号
	if ( !pGateUser->nGlobalSessionId || !pGateUser->sAccount[0] )
	{
		OutputMsg( rmError, _T("未发送查询包就开始进入游戏！") );
		return ERR_SESS;
	}

	//检查会话是否有效
	//TRACE(_T("%s选择角色进入游戏，检查会话\n"), pGateUser->sAccount);
	if ( !m_pDBServer->GetSessionData(pGateUser->nGlobalSessionId, &SessionData) 
		|| SessionData.eState != jxSrvDef::gsSelChar )
	{
		//会话已经过期
		OutputMsg( rmError, _T("选择进入游戏时，会话已经过期！state:%d,nGlobalSessionId:%d,Sessionid:%d"),SessionData. eState,pGateUser->nGlobalSessionId,SessionData.nSessionId);
		//return ERR_SESS;
	}

	//角色上一次保存数据是否出现异常
	//TRACE(_T("%s选择角色进入游戏，检查数据是否异常\n"), pGateUser->sAccount);
	if ( m_pDBServer->getDataServer()->IsCharSavedFailure(nCharId) )
	{
		//为保护数据安全，在保存失败的数据被成功保存前禁止进入游戏
		OutputMsg( rmError, _T("角色上一次保存数据出现异常！禁止进入游戏!") );
		return ERR_DATASAVE;
	}

//#pragma __CPMSG__(暂时屏蔽了这个判断“服务器是否已就绪”)
	//服务器是否已就绪
	//TRACE(_T("%s选择角色进入游戏，检查服务器是否已就绪\n"), pGateUser->sAccount);
	
	if ( m_pDBServer->getDataServer()->GetAvailableDataClientCount(SessionData.nServerIndex) <= 0 )
	{
		//逻辑服务器未就绪
		OutputMsg( rmError, _T("逻辑服务器未就绪!") );
		return ERR_GAMESER;
	}

	//数据库是否连接就绪
	//TRACE(_T("%s选择角色进入游戏，检查数据库连接是否已就绪\n"), pGateUser->sAccount);
	if ( !m_pSQLConnection->Connected() )
	{
		OutputMsg( rmError, _T("数据库连接没就绪!") );
		return ERR_SQL;
	}

	

	//从数据库中更新角色登录的关联数据
	//TRACE(_T("%s选择角色进入游戏，从数据库更新信息\n"), pGateUser->sAccount);
	/*nError = m_pSQLConnection->Query(szSQLSP_ClientStartPlay, SessionData.nServerIndex, nCharId,
		SessionData.sAccount,SessionData.nSessionId,SessionData.nClientIPAddr);
	if ( !nError )
	{
		pRow = m_pSQLConnection->CurrentRow();
		if (pRow[0] && StrToInt(pRow[0]))
		{
			nError = NOERR;
		}
		else
		{
			nError = ERR_NOUSER;
		}
		m_pSQLConnection->ResetQuery();
	}*/
	nError = m_reqHandler.StartEnterGame(SessionData.nServerIndex, 
										 SessionData.nRawServerId,
										 SessionData.nSessionId, 
										 nCharId, 
										 SessionData.sAccount,
										 SessionData.nClientIPAddr);
	
	if ( nError )
	{
		OutputMsg( rmError, _T("选择角色错误!") );
		return ERR_SELACTOR;
	}

	//获取游戏网关路由地址
	//TRACE(_T("为%s选择路由，服务器ID:%d\n"), Session.sAccount, Session.nServerIndex);
	if ( !m_pDBServer->SelectGameServerRoute(SessionData.nServerIndex, sGateHost, nHostLen, nGatePort) )
	{
		//无匹配的路由数据
		OutputMsg( rmError, _T("无匹配的路由数据!") );
		return ERR_NOGATE;
	}

	//向会话客户端投递改变会话状态的内部消息
	m_pDBServer->PostChangeSessionState(SessionData.nSessionId, jxSrvDef::gsWaitEntryGame);
	//OutputMsg( rmTip, _T("通知会话服务器改状态！sessionid:%d,状态：gsWaitEntryGame"), SessionData.nSessionId);

	//记录日志
	/*
	char sActorid[32];
	sprintf_s(sActorid,sizeof(sActorid),"%d",nCharId);
	m_pDBServer->getLogClient()->SendLoginLog(ltEntryGame,SessionData.nSessionId,SessionData.sAccount,
		pGateUser->sIPAddr,sActorid);
	*/

	//向数据服务器投递打开角色加载认证的消息
	TICKCOUNT nCurrentTick =_getTickCount(); //获得当前的时间

	
	m_pDBServer->PostDBServerOpenCharLoad(SessionData.nServerIndex, SessionData.nSessionId, nCharId,nCurrentTick);
	//OutputMsg( rmTip, _T("向数据服务器投递打开角色加载认证的消息"));

	return NOERR;
}

BOOL CDBGate::NameServerAllocCharIdResult(const int nSessionId, const INT_PTR nError, const int nCharId)
{
	/*
	CDBGateUser *pGateUser = GetGateUserPtrBySessionId(nSessionId, NULL);
	if ( pGateUser )
	{
		if ( !pGateUser->boMarkToClose && pGateUser->sAccount[0] && pGateUser->pCreateCharStruct )
		{
			// 错误码在tagNameServerOPError里定义
			if ( pGateUser->pCreateCharStruct )
			{
				ProcessNameSrvResult(pGateUser,nCharId,nError);
				//释放前需要在判断以便，因为用户可能在此前被关闭，而关闭用户会导致释放pCreateCharStruct指针
				m_Allocator.FreeBuffer(pGateUser->pCreateCharStruct);
			}
			else
			{
				OutputMsg( rmError, _T("名称服务器返回正确结果，但会话已被关闭！"));
			}
			
			if (nError != jxInterSrvComm::NameServerProto::neSuccess)		
			{
				OutputMsg( rmWaning, _T("名称服务器返回创建角色申请失败，错误码：(%d)"), nError);
				OutputMsg( rmWaning, _T("alloc name[%s] failed."), pGateUser->sRandomName);
			}
			
			if (pGateUser->nNameLibIndex >= 0)
			{
				PCREATEACTORDATAREQ data = pGateUser->pCreateCharStruct;
				if (data)
					m_pDBServer->AllocRandomName(pGateUser->nNameLibIndex, data->Sex);
			}
			pGateUser->pCreateCharStruct = NULL;
		}
		else
		{
			OutputMsg( rmWaning, _T("名称服务器返回创建角色申请ID结果(%d/%d)，但用户已经不处于创建角色状态,isClose=%d,account=%s,charstruct=%d"), 
				nError, nCharId,(int)pGateUser->boMarkToClose,pGateUser->sAccount,(long long)pGateUser->pCreateCharStruct);
		}
		return TRUE;
	}
	return FALSE;
	*/
	return true;
}


void CDBGate::OnLogicRsponseSessionData(const unsigned int nSessionID,const unsigned int nActorId)
{
	
	
	PINT_PTR lpIndex=NULL;
	CDBGateUser * pUser= GetGateUserPtrBySessionId(nSessionID,lpIndex);
	if(pUser ==NULL) return;
	Inherited::PostInternalMessage(SSM_LOGIC_RESPONSE_SESSION_DATA,nSessionID,nActorId,0);
	
}

VOID CDBGate::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	Inherited::DispatchInternalMessage(uMsg,uParam1,uParam2,uParam3,uParam4); //先执行父类的消息处理
	if(uMsg == SSM_LOGIC_RESPONSE_SESSION_DATA)
	{
		SendClientLogin((const unsigned int)uParam1,(const unsigned int)uParam2);
	}
	
}
void CDBGate::SendClientLogin(const unsigned int nSessionID,const unsigned int nActorId)
{
	PINT_PTR lpIndex=NULL;
	CDBGateUser * pUser= GetGateUserPtrBySessionId(nSessionID,lpIndex);
	if(pUser ==NULL)
	{
		OutputMsg( rmWaning, _T("SendClientLogin 找不到 nSessionID=%d"),nSessionID);
		return;
	}

	//回应包
	CDataPacket& Packet = AllocGateSendPacket(pUser->nSocket,pUser->nGateSessionIndex,pUser->nServerSessionIndex);
	Packet << (WORD)ENTRYGAMERESP_CMD;//设置包类型
	Packet << (char)NOERR;
	Packet <<pUser->szIP;
	Packet << (int)pUser->nPort ;
	Packet << (bool)CDBGate::m_bGableIsClose ;
	FlushGateSendPacket(Packet);//发送结果

}

CDBGateUser* CDBGate::GetGateUserPtrBySessionId(const int nSessionId, PINT_PTR lpIndex)
{
	INT_PTR i;
	CDBGateUser *pGateUser, *pResult = NULL;

	for ( i=m_UserList.count()-1; i>-1; --i )
	{
		pGateUser = (CDBGateUser*)m_UserList[i];
		if ( pGateUser && pGateUser->nGlobalSessionId == nSessionId && !pGateUser->boMarkToClose )
		{
			if ( lpIndex ) *lpIndex = i;
			pResult = pGateUser;
			break;
		}
	}

	return pResult;
}

VOID CDBGate::ProcessUsers()
{
	INT_PTR i;
	CDBGateUser *pGateUser;
	TICKCOUNT dwCurTick = _getTickCount();

	//必须降序循环
	for (i=m_UserList.count()-1; i>-1; --i)
	{
		pGateUser = (CDBGateUser*)m_UserList[i];
		if ( !pGateUser )
			continue;
		//检查延时关闭
		if ( pGateUser->dwDelayCloseTick && dwCurTick >= pGateUser->dwDelayCloseTick )
			pGateUser->boMarkToClose = TRUE;
		//如果长时间无通信数据则关闭用户
		if ( pGateUser->boMarkToClose || dwCurTick - pGateUser->dwLastMsgTick >= 10 * 60 * 1000 )
		{
			OutputMsg(rmWaning,_T("expired kick out, accountid=%d"),pGateUser->nGlobalSessionId);
			CloseUser( pGateUser->nSocket, pGateUser->nServerSessionIndex );
			continue;
		}
	}
}

//void CDBGate::GetActorList(const int nAccountId, const int nServerId, CDataPacket &packet)
//{
//	
//}

VOID CDBGate::DeleteOldActor(CDBGateUser *pSqlGateUser)
{
	int nAccountId = pSqlGateUser->nGlobalSessionId;
	if(nAccountId ==0) return;
	//必须降序循环
	for (INT_PTR i=m_UserList.count()-1; i>-1; --i)
	{
		CDBGateUser * pGateUser = (CDBGateUser*)m_UserList[i];
		if ( !pGateUser )
			continue;
		//检查延时关闭
		if(pGateUser->boMarkToClose ) continue;
		if(pGateUser->nGlobalSessionId == nAccountId  && pGateUser!= pSqlGateUser)
		{
			OutputMsg(rmWaning,"DeleteOldActor account=[%d] old gate user found,delete",nAccountId);
			pGateUser->boMarkToClose =true;
		}
	}
	
}


CDataPacket* CDBGate::QueryActorList(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize)
{
	DECLARE_FUN_TIME_PROF()
	GLOBALSESSIONOPENDATA Session;
	QUERYACTORLISTREQ ReqData;
	CDataPacketReader Reader(lpData,nSize);
	Reader >> ReqData.CmdId;
	Reader >> ReqData.AccountId;
	CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket, pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
	Packet << (WORD)QUERYACTORLISTRESP_CMD;//设置包类型
	Packet << ReqData.AccountId;//帐号id

	if ( !m_pDBServer->GetSessionData(ReqData.AccountId, &Session) )
	{
		Packet << (char)ERR_SESS;
		OutputMsg( rmTip, _T("收到未登陆的用户连接，accountid=%d"),ReqData.AccountId );
		return &Packet;
	}

	pGateUser->nGlobalSessionId = Session.nSessionId;
	_asncpytA(pGateUser->sAccount, Session.sAccount);

	//首先要传递状态给会话服务器,状态改成 gsSelChar=已经连接到DB服务器，出于创建、选择角色状态
	//m_pDBServer->PostChangeSessionState(ReqData.AccountId,gsSelChar);
	pGateUser->nGlobalSessionId = ReqData.AccountId;
	// 发送到日志服务器中
	
	/*
	m_pDBServer->getLogClient()->SendLoginLog(ltQueryActor,pGateUser->nGlobalSessionId,pGateUser->sAccount,
		pGateUser->sIPAddr,NULL);
	*/

	//OutputMsg( rmTip, _T("传递状态给会话服务器，状态：gsSelChar") );
	m_reqHandler.GetActorList(ReqData.AccountId, Session.nServerIndex, Packet);
	
	return &Packet;
}

CDataPacket* CDBGate::CreateActor(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize)
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR resultCode = NOERR;
	//读出数据包内容，主要就是角色名称
	CDataPacketReader Reader(lpData,nSize);
	PCREATEACTORDATAREQ ReqData = (PCREATEACTORDATAREQ)m_Allocator.AllocBuffer(sizeof(CREATEACTORDATAREQ));

	//首先判断网关用户是否通过查询角色消息设定了全局会话ID和帐号
	GLOBALSESSIONOPENDATA Session;
	if ( !pGateUser->nGlobalSessionId || !pGateUser->sAccount[0] || 
		!m_pDBServer->GetSessionData(pGateUser->nGlobalSessionId, &Session) )
	{
		OutputMsg( rmTip, _T("未发送查询包就开始创建角色！") );
		resultCode = ERR_SESS;
	}
	else
	{
		Reader >> ReqData->CmdId;
		Reader.readString(ReqData->ActorName,ArrayCount(ReqData->ActorName));
		ReqData->ActorName[sizeof(ReqData->ActorName)-1] = 0;
		//要改成小写
		m_pDBServer->LowerCaseNameStr(ReqData->ActorName,sizeof(ReqData->ActorName));
		INT_PTR nLen = m_pDBServer->GetStrLenUtf8(ReqData->ActorName);

		if ( nLen <= 0 || nLen > 6 || !m_pDBServer->CheckNameStr(ReqData->ActorName))
		{
			resultCode = jxInterSrvComm::NameServerProto::neInvalidName;
		}
		else
		{
			Reader >> ReqData->Sex;
			Reader >> ReqData->Job;
			Reader >> ReqData->Icon;
			Reader >> ReqData->Zy;

			// 检测阵营合法性
			bool zyVali = true;
			if (ReqData->Zy < 0 || ReqData->Zy > 3)
			{
				zyVali = false;
			}
			
			if ( !zyVali )//选择的阵营必须是之前服务器下发给他的
			{
				resultCode = ERR_ZY;
			}
			else
			{
				////第一个创建的角色，阵营是不能选择的，只能分配个最少人选的
				//if (QueryActorCount(pGateUser->nGlobalSessionId,Session.nServerIndex) <= 0)
				//{
				//	ReqData->Zy = 0;
				//}
				
				if (ReqData->Zy == 0)//如果是选了随机，则给个最少人选的阵营
				{
					resultCode = QueryZYReq(ReqData->Zy,Session.nServerIndex);
				}
				
				if (ReqData->Job <= enVocNone || ReqData->Job >= enMaxVocCount )
				{
					resultCode = ERR_JOB;
				}
				
			}
			
			
		}
	}
	
	if (resultCode != NOERR)
	{
		CREATEACTORDATARESP Resp;
		Resp.CmdId = CREATEACTORDATARESP_CMD;
		Resp.ActorId = 0;
		Resp.Ret = (BYTE)resultCode;
		CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
		Packet << Resp.CmdId;
		Packet << Resp.ActorId;
		Packet << Resp.Ret;
		if (Resp.Ret != 0)
		{
			OutputMsg(rmError, _T("Create Actor Failed, code=%d"), (int)Resp.Ret);
		}

		//OutputMsg( rmTip, _T("create actor error!actorid:%d,actorname:%s,error:%d"),Resp.ActorId,ReqData->ActorName,Resp.Ret);
		m_Allocator.FreeBuffer(ReqData);

		return &Packet;
	}
	
	// 向名称服务器查询该名字是否可用
	//m_pDBServer->getNameSyncClient()->PostAllocateCharId(pGateUser->nGlobalSessionId,Session.nServerIndex,ReqData->ActorName);
	
	// 保存创建角色所需要的数据
	((CDBGateUser*)pGateUser)->pCreateCharStruct = ReqData;

	//保存到日志服务器
	/*
	m_pDBServer->getLogClient()->SendLoginLog(ltCreateActor,pGateUser->nGlobalSessionId,pGateUser->sAccount,
		pGateUser->sIPAddr,ReqData->ActorName);//最后的描述字段保存用户申请的角色名称
	*/

	return NULL;
}

VOID CDBGate::ProcessNameSrvResult(CDBGateUser* pGateUser,int nCharId,INT_PTR nError)
{
	/*
	DECLARE_FUN_TIME_PROF()
	char szSql[500];
	PCREATEACTORDATAREQ data = pGateUser->pCreateCharStruct;

	CREATEACTORDATARESP Resp;
	Resp.CmdId = CREATEACTORDATARESP_CMD;
	Resp.ActorId = nCharId;
	//名称服务器返回正确结果，才执行插入数据库操作
	if (nError == jxInterSrvComm::NameServerProto::neSuccess)
	{
		bool boLogin = false;
		GLOBALSESSIONOPENDATA Session;
		if ( !pGateUser->nGlobalSessionId || !pGateUser->sAccount[0] )
		{
			//会话数据异常
			boLogin = false;
		}
		else
		{
			if ( !m_pDBServer->GetSessionData(pGateUser->nGlobalSessionId, &Session) )
			{
				boLogin = false;
			}
			else
			{
				boLogin = true;
			}
		}

		if (boLogin)
		{
			unsigned long long sip = (inet_addr(pGateUser->sIPAddr));
			sprintf(szSql,szSQLSP_ClientCreateNewCharactor,pGateUser->nGlobalSessionId,pGateUser->sAccount,
				sip,
				nCharId,data->ActorName,data->Icon,data->Sex,data->Job,data->Zy,Session.nServerIndex);	
			nError = m_pSQLConnection->RealExec(szSql,strlen(szSql));
			if (nError == 0)
			{
				m_pSQLConnection->ResetQuery();
				// 插入数据库成功
				if (m_pGateMgr)
					m_pGateMgr->OnNewPlayerCreated(Session.nServerIndex, (tagZhenying)data->Zy, (tagActorVocation)data->Job);
				Resp.Ret = 0;
				
			}
			else
			{
				Resp.Ret = ERR_SQL;
			}
			
		}else
		{
			Resp.Ret = ERR_SESS;
		}
		
	}	
	else
	{		
		Resp.Ret = (char)nError;//名称服务器返回的错误
		if (Resp.Ret != 0)
		{
			OutputMsg(rmError, _T("Create Actor Failed, code=%d"), (int)Resp.Ret);
		}
	}
	//返回结果给客户端
	CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
	Packet << Resp.CmdId;
	Packet << Resp.ActorId;
	Packet << Resp.Ret;
	FlushGateSendPacket(Packet);//发送结果
	//OutputMsg( rmTip, _T("send create actor response!actorid:%d,actorname:%s,errcode:%d"),Resp.ActorId,data->ActorName,Resp.Ret);
	char* szAN = NULL;
	if (Resp.Ret == jxInterSrvComm::NameServerProto::neSuccess)
	{
		//成功
		
		szAN = data->ActorName;
		//如果这个名字是随机生成的，要把这个名字设置成不可再用
		if (pGateUser->nNameLibIndex >= 0 && pGateUser->sRandomName != NULL && strcmp(pGateUser->sRandomName,data->ActorName) == 0)
		{
			m_pDBServer->AllocRandomName(pGateUser->nNameLibIndex,data->Sex);
		}
		

	}
	/*
	m_pDBServer->getLogClient()->SendLoginLog(ltCreateActorResult,pGateUser->nGlobalSessionId,pGateUser->sAccount,
		pGateUser->sIPAddr,szAN);//最后的描述字段保存用户申请的角色名称
	*/

}

CDataPacket* CDBGate::DeleteActor(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize)
{
	DECLARE_FUN_TIME_PROF()
	//首先判断网关用户是否通过查询角色消息设定了全局会话ID和帐号
	if ( !pGateUser->nGlobalSessionId || !pGateUser->sAccount[0] )
	{
		OutputMsg( rmTip, _T("未发送查询包就开始删除角色！") );
		//返回结果给客户端
		DELETEACTORDATARESP Resp;
		Resp.CmdId = DELETEACTORDATARESP_CMD;
		Resp.Actorid = 0;
		Resp.Ret = ERR_SESS;
		CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
		Packet << Resp.CmdId;
		Packet << Resp.Actorid;
		Packet << Resp.Ret;
		return &Packet;
	}
	DELETEACTORDATAREQ ReqData;
	CDataPacketReader Reader(lpData,nSize);
	Reader >> ReqData.CmdId;
	Reader >> ReqData.Actoid;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return NULL;
	}
	MYSQL_ROW pRow;

	//回应包
	CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
	Packet << (WORD)DELETEACTORDATARESP_CMD;//设置包类型
	Packet << ReqData.Actoid;
	
	int nError = m_pSQLConnection->Query(szSQLSP_SelecteGuildData,ReqData.Actoid);
	bool errFlag = false;
	if ( !nError )
	{
		
		pRow = m_pSQLConnection->CurrentRow();
		if (pRow[0] && StrToInt(pRow[0]) >=1)
		{
			errFlag = true;
			Packet << (char)ERR_GUILD;
		}
	
		m_pSQLConnection->ResetQuery();
		
	}
	else
	{
		errFlag = true;
		Packet << (char)ERR_SQL;	 //执行sql错误
	}
	

	if(errFlag == false)
	{
		//查询数据库
		char szSql[100];
		sprintf(szSql,szSQLSP_ClientDeleteCharactor,ReqData.Actoid,pGateUser->sAccount);
		nError = m_pSQLConnection->RealExec(szSql,strlen(szSql));
		if (nError != 0)
		{
			Packet << (char)ERR_SQL;//-1是错误码
		}
		else
		{
			m_pSQLConnection->ResetQuery();
			Packet << (char)0;
			/*
			char szAn[32];
			sprintf(szAn,"%d",ReqData.Actoid);
			
			m_pDBServer->getLogClient()->SendLoginLog(ltDelActor,pGateUser->nGlobalSessionId,pGateUser->sAccount,
				pGateUser->sIPAddr,szAn);//最后的描述字段保存用户删除的角色id
			*/

			//OutputMsg( rmTip, _T("发送删除角色列表回应包！，用户id:%d,角色：%d,错误码：0"),pGateUser->nGlobalSessionId,ReqData.Actoid);	
		}	
	}
	return &Packet;
}

CDataPacket* CDBGate::EntryGameReq(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize)
{
	DECLARE_FUN_TIME_PROF()
	//首先判断网关用户是否通过查询角色消息设定了全局会话ID和帐号
	if ( !pGateUser->nGlobalSessionId || !pGateUser->sAccount[0] )
	{
		OutputMsg( rmTip, _T("未登陆就发送进入游戏包！") );
		//返回结果给客户端
		ENTRYGAMERESP Resp;
		Resp.CmdId = ENTRYGAMERESP_CMD;
		Resp.Ret = ERR_SESS;
		CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
		Packet << Resp.CmdId;
		Packet << Resp.Ret;
		return &Packet;
	}

	ENTRYGAMEREQ ReqData;
	CDataPacketReader Reader(lpData,nSize);
	Reader >> ReqData.CmdId;
	Reader >> ReqData.Actorid;

	ENTRYGAMERESP Resp;
	Resp.CmdId = ENTRYGAMERESP_CMD;
	int nError = UserSelCharEntryGame((CDBGateUser*)pGateUser,ReqData.Actorid,Resp.szIP,sizeof(Resp.szIP),(PINT_PTR)&Resp.Port);

	if (nError != NOERR)
	{
		//出现错误
		//返回结果给客户端		
		Resp.szIP[0] = 0;
		Resp.Port = 0;		
		Resp.Ret = nError;
		CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
		Packet << Resp.CmdId;
		Packet << Resp.Ret;
		OutputMsg(rmTip,"Enter Game Error!Errorcode = %d",Resp.Ret);
		
		return &Packet;
	}
	else
	{
		pGateUser->nPort = Resp.Port;
		strcpy(pGateUser->szIP,Resp.szIP);
	}	
	Resp.Ret = nError;

	return NULL;
}

CDataPacket* CDBGate::RandNameReq( CDBGateUser *pGateUser, char* lpData,SIZE_T nSize )
{
	/*
	DECLARE_FUN_TIME_PROF()
	GLOBALSESSIONOPENDATA Session;
	LPCSTR sActorName = NULL;
	INT_PTR index = -1;
	pGateUser->nNameLibIndex = index;
	pGateUser->sRandomName[0] = 0;

	CDataPacketReader Reader(lpData,nSize);
	WORD wCmdId = 0;
	BYTE bSex = 0, Ret = NOERR;
	Reader >> wCmdId;
	Reader >> bSex;

	//首先判断网关用户是否通过查询角色消息设定了全局会话ID和帐号
	if ( !pGateUser->nGlobalSessionId || !pGateUser->sAccount[0] )
	{
		Ret = ERR_SESS;
	}
	else if ( !m_pDBServer->GetSessionData(pGateUser->nGlobalSessionId, &Session) )
	{
		Ret = ERR_SESS;
	}
	else if (!m_pSQLConnection->Connected())
	{
		Ret = ERR_SQL;
	}
	else
	{
		if (m_pDBServer->GetEnableRandomCharName())
		{
			while(true)
			{
				DECLARE_TIME_PROF("RandNameReq_GetRandomName")
				index = m_pDBServer->GetRandomName(sActorName,bSex);
				if (index < 0)
				{ 
					Ret = ERR_NORANDOMNAME;
					break;
				}
				// 如果此名字属于屏蔽字，过滤掉
				if (m_pDBServer->hasFilterWordsInclude(sActorName))
				{
					m_pDBServer->AllocRandomName(index, bSex);
					continue;
				}

				//查找本地数据库有没有这个名字
				INT_PTR nError = m_pSQLConnection->Query(szSQLSP_GetCharactorIdByName, sActorName, Session.nServerIndex);
				if ( !nError )
				{
					//如果角色名称存在或者不符合基本要求则继续获取下一个名字
					if ( m_pSQLConnection->CurrentRow() || strlen(sActorName) < 4 || !m_pDBServer->CheckNameStr(sActorName))
					{
						m_pDBServer->AllocRandomName(index,bSex);

						m_pSQLConnection->ResetQuery();
						continue;
					}
					else //找到一个可用的名字
					{
						m_pSQLConnection->ResetQuery();
						break;
					}
				}
				else //查询失败
				{
					index = -1;
					m_pSQLConnection->ResetQuery();
					Ret = ERR_SQL;
					break;
				}
			}
		}
	}
	if (Ret != NOERR)
	{
		CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
		Packet << (WORD)RANDNAMERESP_CMD;
		Packet << (BYTE)Ret;
		return &Packet;
	}
	//回应包
	CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
	Packet << (WORD)RANDNAMERESP_CMD;//设置包类型
	Packet << (BYTE)Ret;
	Packet << bSex;
	if (Ret == NOERR)
	{
		Packet.writeString(sActorName);
		pGateUser->nNameLibIndex = index;
		_asncpytA(pGateUser->sRandomName,sActorName);
	}

	return &Packet;
	*/
	return NULL;
}

CDataPacket* CDBGate::QueryLessJobReq( CDBGateUser *pGateUser, char* lpData,SIZE_T nSize )
{
	DECLARE_FUN_TIME_PROF()
	BYTE bJob = 0,bRet =0;
	CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
	Packet << (WORD)LESSJOBRESP_CMD;//设置包类型

	GLOBALSESSIONOPENDATA Session;
	if ( !m_pDBServer->GetSessionData(pGateUser->nGlobalSessionId, &Session) )
	{
		Packet << (BYTE)ERR_SESS;
		return &Packet;
	}

	bRet = QueryLessJobReq(bJob,Session.nServerIndex);

	Packet << (BYTE)bRet;
	Packet << (BYTE)bJob;
	return &Packet;
}

BYTE CDBGate::QueryLessJobReq( BYTE &bJob, int serverindex )
{
	DECLARE_FUN_TIME_PROF()
	BYTE bRet = 1; // 默认是华山
	if (m_pGateMgr)
		bJob = m_pGateMgr->GetPriorityJob(serverindex);

	return reSucc;
}

CDataPacket* CDBGate::QueryZYReq( CDBGateUser *pGateUser, char* lpData,SIZE_T nSize )
{
	DECLARE_FUN_TIME_PROF()
	BYTE bZY = 0,bRet =0;
	CDataPacket& Packet = AllocGateSendPacket(pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex);
	Packet << (WORD)LESSZYRESP_CMD;//设置包类型

	GLOBALSESSIONOPENDATA Session;
	if ( !m_pDBServer->GetSessionData(pGateUser->nGlobalSessionId, &Session) )
	{
		Packet << (BYTE)ERR_SESS;
		return &Packet;
	}

	bRet = QueryZYReq(bZY,Session.nServerIndex);

	Packet << (BYTE)bRet;
	Packet << (BYTE)bZY;
	return &Packet;
}

BYTE CDBGate::QueryZYReq( BYTE& bZY ,int serverindex)
{
	DECLARE_FUN_TIME_PROF()
	// 修改为直接从内存中查询数据 by cap 2011.9.22
	bZY = zyWuJi;
	if (m_pGateMgr)
		bZY = m_pGateMgr->GetPriorityZY(serverindex);

	return reSucc;
}

INT_PTR CDBGate::QueryActorCount( INT_PTR nAccountId, INT_PTR nServerIndex )
{
	DECLARE_FUN_TIME_PROF()
	BYTE result = 0;
	if (!m_pSQLConnection->Connected())
	{
		result = 0;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_QueryActorCount,nAccountId,nServerIndex);
		if (nError != 0)
		{
			result = 0;
		}
		else{
			MYSQL_ROW pRow;
			pRow = m_pSQLConnection->CurrentRow();
			if (pRow && pRow[0])
			{
				result = StrToInt(pRow[0]);
			}
			else
			{
				result = 0;//数据库无数据
			}
			m_pSQLConnection->ResetQuery();
		}	
	}
	return result;
}

WORD CDBGate::QueryZyList( INT_PTR nServerIndex )
{
	DECLARE_FUN_TIME_PROF()
	WORD result = 7;
	if (m_pGateMgr)
		result = (WORD)m_pGateMgr->GetOptionalZy((int)nServerIndex);

	return result;
}
