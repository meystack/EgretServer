#include "StdAfx.h"

CSessionServer::CSessionServer(CSSManager *lpSSManager)
:Inherited()
{
	m_pSSManager = lpSSManager;
	m_nDBClientCount = 0;
	m_nEngineClientCount = 0;
	m_dwCheckOnlineLogTick = 0;
	m_boOnlineLoged = FALSE;
	//m_isFcmOpen =false;
	m_dwCheckOnlineLogTick =0;
	m_dwSaveRankTick = _getTickCount() + SAVE_RANK_INTERVAL ; //下一次存盘的时间

	m_SessionList.setLock(&m_SessionListLock);
	SetServiceName(_T("会话"));
}

CSessionServer::~CSessionServer()
{

}

VOID CSessionServer::SendLogOnlineCount()
{
	// OutputMsg(rmNormal,"SendLogOnlineCount start");
	INT_PTR i;
	INT nTotalUser;
	CSessionClient *pClient;
	LogSender *pLog = m_pSSManager->getLogClient();
	if (NULL ==pLog) return;
	nTotalUser = 0;
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		//如果连接对象是一个逻辑服务器
		if ( pClient && pClient->connected() && pClient->registed() 
			&& pClient->getClientType() == GameServer && pClient->getClientServerIndex() > 0 )
		{
			//统计总人数
			nTotalUser += pClient->m_nMaxUserCount;
			//记录此服务器的在线人数
			
			//报告这段时间最高在线
			pLog->SendOnlineLog(pClient->getClientServerIndex(),pClient->m_nMaxUserCount);
				
			//还原此服务器的最高在线人数
			pClient->m_nMaxUserCount = pClient->m_nUserCount;
		}
	}
	// OutputMsg(rmNormal,"SendLogOnlineCount end");
	//向日志服务器发送记录所有服务器总在线人数的消息，服务器ID值为0
	//pLog->SendOnlineLog(0,nTotalUser); 
}

typedef struct tagIpCount
{
	long long  lIp; //IP
	long long nCount; //登陆的数目

} IPCOUNT;
	
//发送当前ip的状况
void CSessionServer::SendOnlineIpStatus()
{
	PGAMESESSION *pSessionList;


	CBaseList<IPCOUNT> ipList; //当前的ip的列表
	
	pSessionList = m_SessionList;
	INT_PTR i,j,k;
	
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if(pSessionList[i]->nIsRawLogin ==0) continue; //是否在跨服登录

		long long lIp = pSessionList[i]->nIPAddr; //当前的ip
		bool flag=true;
		for( j =ipList.count() -1; j >=0; --j )
		{
			if(ipList[j].lIp == lIp)
			{
				ipList[j].nCount ++;

				//保持降序排列，最前面的ip是最多的
				//从队列的头开始找1个比这个个数小的进行交换
				for(k = 0; k < j; k ++ )
				{
					if( ipList[k].nCount <  ipList[j].nCount)
					{
						//先临时存起来
						IPCOUNT temp;
						temp.lIp = ipList[k].lIp;
						temp.nCount= ipList[k].nCount;					
						
						ipList[k] = ipList[j]; //进行交换
						ipList[j].lIp = temp.lIp;
						ipList[j].nCount = temp.nCount;
						break;
					}
				}
				flag =false;
				break;
			}
		}
		
		//没有找到
		if(flag)
		{
			IPCOUNT temp;
			temp.lIp= lIp;
			temp.nCount =1;
			ipList.add(temp);
		}
	}

	LogSender *pLog = m_pSSManager->getLogClient();
	if (NULL ==pLog) return;

	int nLogIpMinCount =  m_pSSManager->GetIpLogCount(); //登陆了多少个账户就记录日志

	INT_PTR nCount = ipList.count();
	for(i= 0; i< nCount; i++ )
	{
		//如果少于这个数，就不记录了
		if(ipList[i].nCount < nLogIpMinCount)
		{
			break;
		}
		else
		{
			//pLog->SetServerIndex(0); //设置服务器id0
			pLog->SendLoginLog(ltHotIp ,(int)ipList[i].nCount,
				"",inet_ntoa(*((in_addr*)&ipList[i].lIp))  ,"");
		}
	}
	
	//上面已经对ip的数目列表进行了一次降序排列
}


//禁止ip并且封掉所有的账户
void CSessionServer::ForbidIpAndSealAll(long long lIP,bool bDirectSealIp  )
{
	return;

	PGAMESESSION *pSessionList;
	pSessionList = m_SessionList;
	INT_PTR i;
	bool sealAccount  =false;

	bool sealIp =false;

	//bool isFirst =true;
	int  nTotalInterval = 0 ;   //总间隔
	TICKCOUNT  nLastTick ;
	INT_PTR nCount =0;
	int nShortInterval =0;  //间隔特别短的登陆比较多

	char *sIp =inet_ntoa(*((in_addr*)&lIP)); //这个是IP
	if(! bDirectSealIp )
	{
		int nForbidInterval = m_pSSManager->GetForbidInterval() * 1000; //获取间隔
		int nKickCount =  m_pSSManager->GetKickCount();

		int nIpCount =0;  //IP登陆的个数
		int nLastInterval =0; //上1个间隔的

		for ( i=m_SessionList.count() - 1; i>-1; --i )
		{
			long long lAccountIp = pSessionList[i]->nIPAddr; //当前的ip
			if(lAccountIp == lIP &&  pSessionList[i]->nIsRawLogin)
			{
				
				
				if (nIpCount >= 1)
				{
					int nInterval=  (int)(pSessionList[i]->dwSessionTick -nLastTick); //间隔
					if(nInterval <0)
					{
						nInterval = -nInterval;
					}

					if(nInterval <= nForbidInterval)
					{
						nCount ++; //间隔的个数
					}
					
					if(nIpCount >=2 )
					{
						int nNewInterval = nLastInterval - nInterval;  //登陆的间隔差
						if(nNewInterval <0)
						{
							nNewInterval = -nNewInterval;
						}

						if(nNewInterval < 3000)  //间隔3秒登陆就有点问题
						{
							nShortInterval ++;
						}
					}

					nLastInterval = nInterval;
					
				}
				nLastTick = pSessionList[i]->dwSessionTick; //上1次的间隔
				nIpCount ++;
			}
		}

		// //平均登陆的间隔少于一定的数
		// if( nCount >= nKickCount || nShortInterval >= 7) //如果间隔小于这个数目，表示
		// {
			
		// 	INT_PTR nFirbidCount= GetIpForbidCount(lIP);  //IP被封了多少次
		// 	nFirbidCount ++;
		// 	SetIpForbidCount(lIP,(int)nFirbidCount);

		// 	OutputMsg(rmNormal,"Is Robot IP=%s,nCount=%d,nForbidInterval =%d, kickcount=%d,nShortInterval=%d,nFirbidCount=%d",sIp,nCount,nForbidInterval,nKickCount,nShortInterval,(int)nFirbidCount);

		// 	//如果是第3次犯案
		// 	if(nFirbidCount >=3 )
		// 	{
		// 		sealAccount =true;
		// 		sealIp =true;
		// 	}
		// }
		// else
		// {
		// 	OutputMsg(rmNormal,"MayBe Robot IP=%s,nCount=%d,nForbidInterval =%d ms, kickcount=%d",sIp,nCount,nForbidInterval,nKickCount);

		// }
	}
	else
	{
		OutputMsg(rmNormal,"Seal account login IP=%s,Seal",sIp);
		if (m_pSSManager->IsAutoSealIp())
		{
			sealIp =true;
		}
		
	}
	
	
	//OutputMsg(rmNormal,"nForbidInterval =%d ms, kickcount=%d",nForbidInterval,nKickCount);

	LogSender *pLog = m_pSSManager->getLogClient();

	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nIPAddr == lIP )
		{
			
			OutputMsg(rmTip,"kick ip=%s,account=%s,serverindex=%d",sIp, pSessionList[i]->sAccount, (int)pSessionList[i]->nServerIndex); // 解封这个ip
		
			if(sealAccount)
			{
				int nError = m_SQLConnection.Exec("update globaluser set passwd =\"@autoSeal\" where account=\"%s\"", pSessionList[i]->sAccount);

				if(!nError)
				{
					m_SQLConnection.ResetQuery();
				}
				OutputMsg(rmTip,"Auto Seal account=%s,serverindex=%d",pSessionList[i]->sAccount, (int)pSessionList[i]->nServerIndex); // 解封这个ip

				//封停账户
				pLog->SendLoginLog(itSealAccount ,0,"", pSessionList[i]->sAccount  ,"");

			}
			

			ProcessKickCrossActor((unsigned int)pSessionList[i]->nSessionID,pSessionList[i]->nServerIndex);
			CloseSessionAtListIndex(i);
		}
	}

	if(sealIp)
	{
		CBaseList<unsigned long long>&  blackips= m_pSSManager->GetBlackIpList(); //获取黑名单列表
		nCount = blackips.count();
		bool flag =false;

		for( i=0; i< nCount ;i++)
		{
			if(blackips[i] == lIP)
			{
				flag =true;
				break;
			}
		}

		if(!flag)
		{	
			OutputMsg(rmTip,"Auto Seal ip=%s ",sIp); // 解封这个ip
			blackips.add(lIP);

			pLog->SendLoginLog(ltSealIp ,0,"",sIp ,"");

		}
	}
}


CCustomServerClientSocket* CSessionServer::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	CSessionClient * pClient = new CSessionClient ;
	pClient->SetClientSocket(nSocket, pAddrIn);

	//CSessionClient(this, &m_SQLConnection, nSocket, pAddrIn);
	pClient->SetParam(this,&m_SQLConnection,false);
	return pClient;
}

VOID CSessionServer::ProcessClients()
{
	INT_PTR i;
	INT_PTR nDBCount = 0, nEngineCount = 0;
	CSessionClient *pClient;
	TICKCOUNT dwCurTick = _getTickCount();

	//循环处理每个客户端连接
	m_ClientList.flush();
	//必须降序循环，因为列表中的数据可能在循环中被移除
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		//处理活动的连接
		pClient->Run();
		//删除断开的连接
		if ( !pClient->connected() )
		{
			//连接在断开后的5分钟后删除
			if ( dwCurTick - pClient->m_dwDisconnectedTick >= 5 * 60 * 1000 )
			{
				m_ClientList.lock();
				m_ClientList.remove(i);
				m_ClientList.unlock();
				delete pClient;
			}
			continue;
		}
		//统计各个种类的客户端数量
		if ( pClient->registed() )
		{
			if ( pClient->getClientType() == DBServer )
				nDBCount++;
			else if ( pClient->getClientType() == GameServer )
				nEngineCount++;
		}
	}

	m_nDBClientCount = nDBCount;
	m_nEngineClientCount = nEngineCount;
}

BOOL CSessionServer::DoStart()
{
	return TRUE;
}

VOID CSessionServer::DoStop()
{

}

VOID CSessionServer::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	INT_PTR nIndex;
	PGAMESESSION pSession;
	//OutputMsg( rmNormal, _T("DispatchInternalMessage MSGID=%d,PARAM1=%d,PARAM2=%d"),uMsg,uParam1,uParam2 );
	switch(uMsg)
	{
	case SSIM_POST_CLOSE_SESSION://按会话ID关闭会话(Param1=会话ID,Param2=会话连续在线时间（用于防沉迷中统计在线时间）)
		{
			pSession = GetSessionPtr((unsigned int)uParam1, &nIndex,uParam2);
			if ( pSession )
			{
				CloseSessionAtListIndex( nIndex );
				TRACE(_T("当前会话数量：%d"), m_SessionList.count());
			}
			else
			{
				OutputMsg(rmError,_T("关闭连接的时候找不到session：%d"), uParam1);
			}
		}
		break;
	case SSIM_CHANGE_SESSION_STATE://网关向SessionServer投递改变会话状态的消息(Param1=会话ID,Param2=会话状态)
		{
  			pSession = GetSessionPtr( (unsigned int)uParam1 );
			if ( pSession && pSession->nState != uParam2 )
			{
				pSession->nState = (GSSTATE)uParam2;
				//如果会话状态被改变为等待选择角色或等待进入游戏，则为会话标记超时时间
				if ( pSession->nState == gsWaitQueryChar )
					pSession->dwTimeOut = _getTickCount() + m_pSSManager->getSessionQueryCharTimeOut();
				else if ( pSession->nState == gsWaitEntryGame )
					pSession->dwTimeOut = _getTickCount() + m_pSSManager->getSessionEntryGameTimeOut();
				else pSession->dwTimeOut = 0;
				//广播改变会话状态
				BroadCastUpdateSessionState(pSession);
			}
		}
		break;
	case SSIM_GATE_USER_CLOSED://网关向SessionServer投递网关用户已关闭的消息(Param1=会话ID)
		//在会话服务器中不对此消息进行处理
		break;
	case SSIM_POST_ALL_CLIENT_MSG://向所有会话服务器的客户端发送消息(Param1=服务器类型,Param2=数据包,Param3=数据包大小)
		{
			SendAllClientMsg((SERVERTYPE)uParam1, (LPCSTR)uParam2, uParam3);
			m_Allocator.FreeBuffer((LPVOID)uParam2);
		}
		break;
	case SSIM_CONFIM_SESSION_ONLINE://确认会话是否在线(Param1=会话ID,Param2=服务器ID)
		{
			pSession = GetSessionPtr( (const unsigned int)uParam1 );
			if ( pSession )
			{
				pSession->nConfimSrvCount = (int)(GetDBClientCount() + GetLogicClientCount(uParam2));
				pSession->dwTimeOut = _getTickCount() + 10 * 1000;
				
				OutputMsg(rmTip,_T("nConfimSrvCount=%d+%d=%d,serverindex=%d"),
					GetDBClientCount(),GetLogicClientCount(uParam2),pSession->nConfimSrvCount,pSession->nServerIndex);
				if ( pSession->nConfimSrvCount > 0 )
					BroadCastSessionConfim(uParam1, pSession->nServerIndex);
				//else m_pSSManager->PostGateUserConfimSessionResult((int)uParam1, FALSE);
			}
			else
			{
				OutputMsg(rmError,_T("SSIM_CONFIM_SESSION_ONLINE 找不到session =%d"),uParam1);
			}
		}
		break;
	case SSIM_CONFIM_SESSION_RESULT://数据或引擎客户端返回会话是否在线(Param1=会话ID,Param2=是否在线（0:1）)
		{
			pSession = GetSessionPtr( (const unsigned int)uParam1 );
			if ( pSession && pSession->nConfimSrvCount )
			{
				pSession->nConfimSrvCount--;//减少需要等待回应的服务器数量
				//如果会话已经在线，则返回网关用户的认证结果
				if ( uParam2 )
				{
					//有一个服务器回应会话在线，则不用等待其他服务器返回了。
					pSession->nConfimSrvCount = 0;
					//m_pSSManager->PostGateUserConfimSessionResult((int)uParam1, TRUE);
				}
				//如果服务器回应会话未在线，且等待回应的服务器数量已为0，则表示会话没有在线了
				else if ( pSession->nConfimSrvCount <= 0 )
				{
					//m_pSSManager->PostGateUserConfimSessionResult((int)uParam1, FALSE);
				}
			}
			else
			{
				OutputMsg(rmError,_T("SSIM_CONFIM_SESSION_RESULT 找不到session =%d"),uParam1);
			}
		}
		break;
	case SSIM_CLOSE_SESSION_BY_ACCOUNT://通过账号字符串查找并关闭会话(Param1=字符串指针)
		{	
			pSession = GetSessionPtrByAccount( (LPCSTR)uParam1, &nIndex );
			if ( pSession )
			{
				CloseSessionAtListIndex(nIndex);
				m_Allocator.FreeBuffer((LPVOID)uParam1);
			}
		}
		break;
		
	case SSIM_CLOSE_SESSION_BY_ACCOUNTID:
		{
			pSession = GetSessionPtr((const unsigned int)uParam1,&nIndex,uParam2 );
			if ( pSession )
			{
				CloseSessionAtListIndex(nIndex);
			}
		}
		break;
	case SSIM_DEBUG_PRINT_SESSIONS://调试消息：打印全局会话(Param1=会话数量)
		{
			for (INT_PTR i=0; i<(INT_PTR)uParam1; ++i )
			{
				if ( i >= m_SessionList.count() )
					break;
				OutputMsg( rmWaning, _T("%s:%d->%d"), m_SessionList[i]->sAccount, m_SessionList[i]->nSessionID, m_SessionList[i]->nServerIndex );
			}
		}
		break;	

		//踢掉在线的跨服用户
	case SSIM_KICK_ONLINE_CROSSSERVER_ACTOR:
		{
			ProcessKickCrossActor((const unsigned int)uParam1,(const int)uParam2);
			break;
		}
	default:
		break;
	}
}


void CSessionServer::ProcessKickCrossActor(const unsigned int nSessionId, const int nServerIndex)
{
	char sBuffer[64];
	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << (WORD)SessionServerProto::sKickCrossServerUser ;
	data << (unsigned int)nSessionId;
	
	
	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;

	OutputMsg(rmNormal,"send Kick cross server actor,accountid=%d,serverindex=%d",(int)nSessionId,(int)nServerIndex);

	SendLogicClientMsg(nServerIndex,data.getMemoryPtr(), data.getLength());
	
}

VOID CSessionServer::SingleRun()
{
	TICKCOUNT dwCurTick = _getTickCount();

	//提交新开启的会话数据
	if ( m_SessionList.appendCount() > 0 )
	{
		m_SessionList.flush();
		TRACE(_T("当前会话数量：%d"), m_SessionList.count());
	}

	//连接数据库
	ConnectSQL();

	//记录各个服务器的在线人数
	if ( dwCurTick >= m_dwCheckOnlineLogTick )
	{
		SYSTEMTIME SysTime;
		GetLocalTime(&SysTime);
		if ( (SysTime.wMinute % 2) == 0 )//每1分钟记录一次在线
		{
			if ( !m_boOnlineLoged )
			{
				SendLogOnlineCount();
				m_boOnlineLoged = TRUE;
			}
		}
		else
		{
			m_boOnlineLoged = FALSE;
		}

		if ( (SysTime.wMinute % 60) == 5 )//每1分钟，发IP的登陆情况过去
		{
			if ( !m_boIpLoged )
			{
				
				//SendOnlineIpStatus();
				m_boIpLoged = TRUE;
			}
		}
		else
		{
			m_boIpLoged = FALSE;
		}

		m_dwCheckOnlineLogTick = dwCurTick + 30 * 1000;
	}

	//1小时存盘1次
	if(dwCurTick >= m_dwSaveRankTick)
	{
		if(m_SQLConnection.Connected())
		{
			SaveRankMsg();
		}
		m_dwSaveRankTick = dwCurTick +SAVE_RANK_INTERVAL;
	}

	//调用父类例行执行
	Inherited::SingleRun();

	// 更新AMC
	m_pSSManager->UpdateAMC();

	m_pSSManager->UpdateTask();
}

BOOL CSessionServer::ConnectSQL()
{
	if ( !m_SQLConnection.Connected() )
	{
		if ( _getTickCount() >= m_dwReconnectSQLTick )
		{
			m_pSSManager->SetupSQLConnection(&m_SQLConnection);
			if ( m_SQLConnection.Connect() )
			{
				mysql_options(m_SQLConnection.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
				
				//设置下utf8
				if(m_pSSManager->IsUtf8() )
				{
					mysql_set_character_set(m_SQLConnection.GetMySql(),"utf8");
				}
				InitACCountId();
				InitRankMsg();

				return TRUE;
			}
			//如果连接SQL失败则将在5秒后重试
			m_dwReconnectSQLTick = _getTickCount() + 5 * 1000;
			return FALSE;
		}
	}
	return TRUE;
}


bool CSessionServer::InitRankMsg()
{
	int nError = m_SQLConnection.RealQuery(szLoadCsRank,strlen(szLoadCsRank));
	if ( !nError )
	{
	
		MYSQL_ROW pRow = m_SQLConnection.CurrentRow();
		int nCount = m_SQLConnection.GetRowCount();
		while (pRow)
		{
	
			CRankItem rankItem;
			rankItem.nActorid = strtoul(pRow[0],0,10);
		
			sprintf(rankItem.sCharName, "%s", pRow[1]);
			rankItem.sCharName[sizeof(rankItem.sCharName)-1] = 0;
			
			
			rankItem.nProp1= StrToInt(pRow[2]); //sex
			
			rankItem.nProp2 = StrToInt(pRow[3]); //vocation
			rankItem.nKey = rankItem.nProp2;

			
			rankItem.nServerId = StrToInt(pRow[4]);

			rankItem.nValue =StrToInt(pRow[5]);

			pRow = m_SQLConnection.NextRow();

			PostRankMsg(rankItem);

		}
		m_SQLConnection.ResetQuery();
	}
	else
	{
		return false;
	}

	return true;
}

bool CSessionServer::SaveRankMsg()
{
	INT_PTR nError = m_SQLConnection.Exec(szDeleteCSRank, 0);
	if(nError)
	{
		return false;
	}
	m_SQLConnection.ResetQuery(); //函数调用里没有重置数据的

	wylib::container::CBaseList<CRankItem>  itemList ; 
	
	m_rankMgr.GetRankList(-1,itemList);

	INT_PTR nRankCount = itemList.count();

	char hugebuff[2048];
	if(nRankCount > 0 ) //如果有数据
	{
		sprintf(hugebuff,szSaveCsRank); //表头放这里
		char buff[1024] = {0};
		
		for(INT_PTR i = 0; i < nRankCount; i++)
		{
		
			CRankItem *pItem = &itemList[i];
			sprintf(buff,
				"(%u, \"%s\", %d, %d, %d, %d)",
				pItem->nActorid, 
				pItem->sCharName, 
				pItem->nProp1, 
				pItem->nProp2, 
				pItem->nServerId, 
				pItem->nValue); 
			strcat(hugebuff,buff);
			if (i < nRankCount - 1)
			{
				strcat(hugebuff,",");	
			}
		}

		nError = m_SQLConnection.Exec(hugebuff);
		if (!nError)
		{
			m_SQLConnection.ResetQuery();
		}

	}
	OutputMsg(rmTip, _T("save csrank data to db"));
	return true;
}




//初始化账户
bool CSessionServer::InitACCountId()
{
	//查找最大的玩家的账户ID

	char buff[128];

	strcpy(buff,"select max(userid) from globaluser");

	if ( m_SQLConnection.RealQuery(buff,strlen(buff)) )
	{
		OutputMsg(rmError, _T("can not call select max(userid) from globaluser"));
		return false;
	}
	else
	{
		INT_PTR nSPguid= m_pSSManager->GetSPGUID(); //获取guid
		OutputMsg(rmTip,"InitACCountId spid=%d",(int)nSPguid);

		if(nSPguid ==0) return true; //为0的不需要处理

		unsigned int nSpidMinActoridId = ((unsigned int)nSPguid << 24) ; //该运营商最小的accountid
		OutputMsg(rmTip, _T("max spid account=%u"),nSpidMinActoridId);

		bool bNeedInsert =false; //是否需要插入初始化记录
		MYSQL_ROW pRow = m_SQLConnection.CurrentRow();
		if ( pRow )
		{
			if ( pRow[0]  )
			{
				unsigned int nMaxValue=(unsigned int) strtoul(pRow[0],0,10);
				OutputMsg(rmTip, _T("db max account=%u"),nMaxValue);
				if(nMaxValue < nSpidMinActoridId)
				{
					bNeedInsert =true;
				}
			}
			else
			{

				OutputMsg(rmTip, _T("1 the account db has no account data exist, will init the account"));
				bNeedInsert =true;
			}
		}
		else
		{
			OutputMsg(rmTip, _T("2 the account db has no account data exist, will init the account"));
			bNeedInsert =true;
		}
		m_SQLConnection.ResetQuery();

		if(bNeedInsert) //需要初始化db，往里边插入空记录
		{
			char buff[256];
			char name[32];
			sprintf(name,"__init_account_%d",(int)nSPguid);

			sprintf(buff,"insert into globaluser (userid,account,passwd) values (%u,\"%s\",\"000\")",nSpidMinActoridId,name);
			if(m_SQLConnection.Exec(buff))
			{
				OutputMsg(rmError, _T("init account  spid fail"));
				return false;
			}
			else
			{
				m_SQLConnection.ResetQuery();
				OutputMsg(rmTip, _T("init account  spid succeed, account=%u"),nSpidMinActoridId);
			}

		}
		else
		{
			OutputMsg(rmTip, _T("account has init"));
		}
	}
	return true;
}

PGAMESESSION CSessionServer::GetSessionPtr(const unsigned int nSessionId,PINT_PTR lpIndex ,Uint64 lKey,int nServerIndex)
{
	INT_PTR i;
	PGAMESESSION *pSessionList;

	bool bNeedCheckKey = (lKey == UINT64(-1));

	pSessionList = m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nSessionID == nSessionId && ( bNeedCheckKey==true || lKey ==pSessionList[i]->lKey ) && (nServerIndex ==-1 || nServerIndex ==pSessionList[i]->nServerIndex ))
		{
			if (lpIndex) *lpIndex = i;
			return pSessionList[i];
		}
	}
	return NULL;
}


//返回1个ip登陆了多少个号
int CSessionServer::GetLoginAccountCount(LONGLONG lIp)
{
	INT_PTR i;
	PGAMESESSION *pSessionList;

	int nCount =0;
	pSessionList = m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nIPAddr == lIp &&   pSessionList[i]->nIsRawLogin)
		{
			nCount ++;
		}
	}
	return nCount;
}


PGAMESESSION CSessionServer::GetSessionPtrByAccount(LPCSTR sAccount, PINT_PTR lpIndex)
{
	INT_PTR i;
	PGAMESESSION *pSessionList;

	pSessionList = m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( _stricmp(pSessionList[i]->sAccount, sAccount) == 0 )
		{
			if (lpIndex) *lpIndex = i;
			return pSessionList[i];
		}
	}
	return NULL;
}

BOOL CSessionServer::CloseSessionAtListIndex(const INT_PTR nIndex)
{
	unsigned int nSessionId;
	PGAMESESSION pSession;
	BOOL boResult = FALSE;
	
	//OutputMsg(rmNormal,"Current Thread =%d",(int)GetCurrentThreadId());

	if ( m_SessionList.count() > nIndex )
	{
		boResult = TRUE;

		pSession = m_SessionList[nIndex];
		nSessionId = (unsigned int)pSession->nSessionID;
		
		//关闭会话
		OutputMsg(rmTip,_T("close Session,ID=%u,account=%s,serverin=%d"),nSessionId,pSession->sAccount,(int)pSession->nServerIndex);

		//发给自己的服务器，需要删除这个会话
		BroadCastCloseSession(nSessionId,pSession->lKey,pSession->nServerIndex);

		//将会话关闭的行为记录到日志服务器中
		/*
		char sLastLoginIPAddr[64];
		_asncpytA(sLastLoginIPAddr, inet_ntoa(*((in_addr*)&pSession->nIPAddr)));
		
		
		m_pSSManager->getLogClient()->SetServerIndex(pSession->nServerIndex); //设置服务器的index

		m_pSSManager->getLogClient()->SendLoginLog(ltLogout ,(int)nSessionId,
			 pSession->sAccount,sLastLoginIPAddr  ,"");
		*/
		
		//删除会话指针
		m_SessionList.lock();
		m_SessionList.remove(nIndex);
		m_SessionList.unlock();
		m_Allocator.FreeBuffer(pSession);
		
		TICKCOUNT nCurrentTick  = _timeGetTime();

		int nOnlineSeconds = (int)((nCurrentTick - pSession->dwSessionTick)/1000); //得到秒数

		
		//如果有在线时间，则将在线时间更新到数据库中
		if ( nOnlineSeconds > 0 )
		{
			if ( m_SQLConnection.Connected() )
			{
				char buff[256];
				sprintf(buff,szUpdateUsernlineTime,nSessionId,nOnlineSeconds);
				
				
				if (0 == m_SQLConnection.RealExec(buff,strlen(buff) ))
				{
					m_SQLConnection.ResetQuery();
				}	
				else
				{
					OutputMsg(rmWaning,_T("sql %serror"),buff);
				}
			}
			else
			{
				OutputMsg(rmError, _T("在关闭会话是试图更新账号数据(%d,%d)，但SQL未就绪"), nSessionId, nOnlineSeconds);
			}
		}
	}
	else
	{
		OutputMsg(rmWaning,_T("close nIndex =%d,fail"),nIndex);
	}

	return boResult;
}

INT_PTR CSessionServer::SendAllClientMsg(const SERVERTYPE eServerType, LPCSTR sMsg, const size_t dwSize)
{
	INT_PTR i, nSendCount = 0;
	CSessionClient *pClient;

	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if ( pClient && pClient->connected() && pClient->registed() )
		{
			if ( eServerType == InvalidServer || pClient->getClientType() == eServerType )
			{
				//OutputMsg(rmTip,_T("SendAllClientMsg send msg to client:serverindex=%d,clientName=%s"),
				//	pClient->getClientServerIndex(),pClient->getClientName());
				pClient->AppendSendBuffer(sMsg, dwSize );
				nSendCount++;
			}
		}
	}
	return nSendCount;
}

INT_PTR CSessionServer::SendLogicClientMsg(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize)
{
	INT_PTR i, nSendCount = 0;
	CSessionClient *pClient;

	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if ( pClient && pClient->connected() && pClient->registed() && pClient->getClientType() == GameServer && pClient->getClientServerIndex() == (int)nServerIndex)
		{
			pClient->AppendSendBuffer(sMsg, dwSize );
			//OutputMsg(rmTip,"Send LogicMsg, serverindex=%d,size=%d",(int)nServerIndex,(int)dwSize);
			nSendCount++;
		}
	}
	return nSendCount;
}

INT_PTR CSessionServer::SendGroupLogicClientMsg(const int nCommServerId, LPVOID data, const SIZE_T nSize)
{
	INT_PTR i, nSendCount = 0;
	CSessionClient *pClient;
	for (i = m_ClientList.count() - 1; i > -1; --i)
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if ( pClient && pClient->registed() 
			&& GameServer == pClient->getClientType()
			&& nCommServerId == pClient->GetCommonServerId())
		{
			pClient->AppendSendBuffer(data, nSize);
			nSendCount++;
		}
	}

	return nSendCount;
}


INT_PTR CSessionServer::SendAllDBAndIndexEngineMsg(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize)
{
	INT_PTR i, nSendCount = 0;
	CSessionClient *pClient;

	m_ClientList.lock();
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		////debug
		//if (pClient)
		//{
		//	OutputMsg(rmTip,_T("SendAllDBAndIndexEngineMsg pclient:connected=%d,registed=%d,clienttype=%d,serverindex=%d,clientName=%s"),
		//		(int)(pClient->connected()),(int)(pClient->registed()),pClient->getClientType(),
		//		pClient->getClientServerIndex(),pClient->getClientName());
		//}
		////end debug
		if ( !pClient || !pClient->connected() || !pClient->registed() )
			continue;

		if ( pClient->getClientType() == DBServer || 
			(pClient->getClientType() == GameServer && pClient->getClientServerIndex() == nServerIndex) )
		{
			pClient->AppendSendBuffer(sMsg, dwSize );
			nSendCount++;
			//OutputMsg(rmTip,_T("SendAllDBAndIndexEngineMsg send msg to client:serverindex=%d,clientName=%s"),
			//	pClient->getClientServerIndex(),pClient->getClientName());
		}
	}
	m_ClientList.unlock();
	return nSendCount;
}

INT_PTR CSessionServer::BroadCastUpdateSessionState(PGAMESESSION pSession)
{
	char sBuffer[32];
	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << (WORD)SessionServerProto::sUpdateSession ;
	data << (int)pSession->nSessionID;
	data << (int)pSession->nState;

	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;

	//向所有数据服务器发送会话状态变更消息
	INT_PTR nResult = SendAllClientMsg(DBServer,data.getMemoryPtr(), data.getLength());

	//如果会话登录到某个逻辑服务器，则向此逻辑服务器发送会话状态变更消息
	if ( pSession->nServerIndex > 0 )
	{
		nResult += SendLogicClientMsg(pSession->nServerIndex,data.getMemoryPtr(), data.getLength());
	}

	return nResult;
}

INT_PTR CSessionServer::BroadCastCloseSession(const UINT64 nSessionId,Uint64 lKey,const int nServerIndex)
{
	char sBuffer[64];
	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	//关闭号
	data.setPosition(sizeof(*pheader));
	data << (WORD)SessionServerProto::sCloseSession ;
	data <<(unsigned int) nSessionId;
	data <<(Uint64) lKey;

	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;
	//向所有数据服务器、引擎服务器发送删除会话消息
	
	if(nServerIndex >0 )
	{
		return SendLogicClientMsg(nServerIndex, data.getMemoryPtr(), data.getLength()) ;
	}
	else
	{
		return SendAllClientMsg(LogServer, data.getMemoryPtr(), data.getLength() );
	}
	
	//return  SendAllClientMsg(InvalidServer, data.getMemoryPtr(), data.getLength());
}

INT_PTR CSessionServer::BroadCastSessionConfim(const UINT64 nSessionId, const INT_PTR nServerIndex)
{

	INT_PTR nResult =0;
	char sBuffer[32];

	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << (WORD)SessionServerProto::sQuerySessionExist ;
	data << (unsigned int)nSessionId;

	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;
	
	//如果nServerIndex为0，则向所有数据服务器、引擎服务器发送确认会话是否在线的消息
	if ( !nServerIndex )
	{
		nResult = SendAllClientMsg(InvalidServer, data.getMemoryPtr(), data.getLength());
	}
	else 
	{
		nResult = SendAllDBAndIndexEngineMsg(nServerIndex, data.getMemoryPtr(), data.getLength());
	}

	return nResult;
}

//踢掉在线的用户
VOID CSessionServer::PostKickCrossActor(const INT_PTR nSessionId, const int nServerIndex)
{
	PostInternalMessage(SSIM_KICK_ONLINE_CROSSSERVER_ACTOR, nSessionId,nServerIndex,0);
}



VOID CSessionServer::PostAllClientMsg(const SERVERTYPE eServerType, LPCSTR sMsg)
{
	size_t dwSize = strlen(sMsg);
	LPVOID lpData = m_Allocator.AllocBuffer(dwSize);
	memcpy(lpData, sMsg, dwSize);
	PostInternalMessage(SSIM_POST_ALL_CLIENT_MSG, eServerType, (UINT_PTR)lpData, dwSize);
}

BOOL CSessionServer::GetSession(const INT_PTR nSessionId, OUT PGAMESESSION pSession,int nServerIndex)
{
	BOOL boResult = FALSE;
	CSafeLock sl(&m_SessionListLock);

	PGAMESESSION pSessionPtr = GetSessionPtr((unsigned int)nSessionId, NULL,Uint64(-1),nServerIndex);
	if ( pSessionPtr )
	{
		*pSession = *pSessionPtr;
		boResult = TRUE;
	}

	return boResult;
}

PGAMESESSION CSessionServer::GetSpecialServerSession(const INT_PTR nSessionId, const int nServerIndex)
{
	INT_PTR i;
	PGAMESESSION *pSessionList= m_SessionList;
	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nSessionID == nSessionId && pSessionList[i]->nServerIndex == nServerIndex)
		{
			return pSessionList[i];
			
		}
	}
	return NULL;
}
//logic server count
INT_PTR CSessionServer::GetLogicClientCount(const INT_PTR nServerIndex)
{
	OutputMsg(rmTip,_T("GetLogicClientCount:%d"),nServerIndex);

	if ( !nServerIndex )
		return m_nEngineClientCount;

	INT_PTR i, nResult = 0;
	CSessionClient *pClient;

	m_ClientList.lock();
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		////debug
		//if (pClient)
		//{
		//	OutputMsg(rmTip,_T("GetLogicClientCount pClient:connected=%d,registed=%d,clienttype=%d,serverindex=%d,clientName=%s"),
		//		(int)(pClient->connected()),(int)(pClient->registed()),pClient->getClientType(),
		//		pClient->getClientServerIndex(),pClient->getClientName());
		//}
		////end debug
		if ( pClient && pClient->connected() && pClient->registed() 
			&& pClient->getClientType() == GameServer && pClient->getClientServerIndex() == nServerIndex )
		{
			nResult ++;
			//OutputMsg(rmTip,_T("--------------------------%d"),pClient->getClientServerIndex());
			////debug
			//if (pClient)
			//{
			//	OutputMsg(rmTip,_T("GetLogicClientCount pClient2:connected=%d,registed=%d,clienttype=%d,serverindex=%d,clientName=%s"),
			//		(int)(pClient->connected()),(int)(pClient->registed()),pClient->getClientType(),
			//		pClient->getClientServerIndex(),pClient->getClientName());
			//}
			////end debug
		}
	}
	m_ClientList.unlock();

	return nResult;
}

INT_PTR CSessionServer::UserCanLoginToServer(const INT_PTR nServerIndex)
{
	INT_PTR i, nResult = -1;
	CSessionClient *pClient;

	m_ClientList.lock();
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if ( pClient && pClient->connected() && pClient->registed() 
			&& pClient->getClientType() == GameServer && pClient->getClientServerIndex() == nServerIndex )
		{
			//判断服务器人员是否已满,
			if ( pClient->m_nUserCount < pClient->m_nUserLimit )
				nResult = 0;
			else nResult = -1;
			break;
		}
	}
	m_ClientList.unlock();

	return nResult;
}

//此函数是被网关线程调用的
BOOL CSessionServer::PostOpenSession(const INT_PTR nServerIndex, 
									 const INT_PTR nRawServerIndex,
									 LPCSTR sAccount,
									 const unsigned int nSessionId,
									 const INT_PTR nIPAddr, 
									 const INT_PTR dwTodayOnlineSec,
									 INT_PTR nGmLevel,
									 UINT64  lKey ,
									 INT_PTR  nGateIndex, 
									 bool   bIsWhiteLogin,
									 bool	bCommonMsg
									 )
{	
	char sBuffer[256];
	
	PGAMESESSION pSession;
	BOOL boNewSession = FALSE;
	
	m_SessionList.lock();


	pSession = GetSessionPtr(nSessionId);
	m_SessionList.unlock();
	if ( !pSession )
	//每次都去new一个
	{
		pSession = (PGAMESESSION)m_Allocator.AllocBuffer(sizeof(*pSession));
		boNewSession = TRUE;
	}

	
	ZeroMemory(pSession, sizeof(*pSession));
	pSession->nSessionID = nSessionId;
	pSession->nServerIndex = (int)nServerIndex;
	
	if(nServerIndex == nRawServerIndex)
	{
		pSession->nIsRawLogin =1 ; //原来的服务器ID
	}
	else
	{
		pSession->nIsRawLogin =0;
	}
	
	pSession->nIPAddr = nIPAddr;
	pSession->lKey = lKey; //登陆的key值
	//会话建立的时候就标记为等待查询角色状态,这个时候将连接数据服务器
	pSession->nState = gsWaitQueryChar; 
	pSession->dwSessionTick = _timeGetTime();
	pSession->dwTimeOut = pSession->dwSessionTick + m_pSSManager->getSessionQueryCharTimeOut();
	pSession->nGmLevel= (int)nGmLevel;
	_asncpytA(pSession->sAccount, sAccount);

	//追加会话数据
	if ( boNewSession )
	{
		m_SessionList.append(pSession);
	}
	//向日志服务器记录登录日志，格式为：USERID/ACCOUNT/GAMETYPE/IPADDR/CHARNAME
	
	/*
	m_pSSManager->getLogClient()->SetServerIndex(nServerIndex);
	char sLoginIPAddr[64];
	_asncpytA(sLoginIPAddr, inet_ntoa(*((in_addr*)&nIPAddr)));
	m_pSSManager->getLogClient()->SendLoginLog(ltLogin ,(int)nSessionId,
		pSession->sAccount,sLoginIPAddr  ,"");
	*/
	

	//广播消息	
	GLOBALSESSIONOPENDATA SessionData ;
	SessionData.nSessionId = (INT)nSessionId;
	SessionData.nServerIndex = (INT)nServerIndex; //?	
	SessionData.nRawServerId = (INT)nRawServerIndex;
	SessionData.nClientIPAddr = (LONGLONG)nIPAddr;
	SessionData.dwFCMOnlineSec = (DWORD)dwTodayOnlineSec;
	SessionData.nGmLevel =(int) nGmLevel; //GM等级
	SessionData.lKey =lKey; 
	SessionData.nGateIndex = (int)nGateIndex; //在服务器的index

	SessionData.eState =(GSSTATE) (bIsWhiteLogin?0:1);
	_asncpytA(SessionData.sAccount, sAccount);

	PDATAHEADER pheader;
	CDataPacket data(sBuffer,sizeof(sBuffer));
	data.setLength(sizeof(*pheader));

	data.setPosition(sizeof(*pheader));
	data << (WORD)SessionServerProto::sOpenSession  ; //cmd
	data << SessionData; //数据
	
	pheader = (PDATAHEADER)data.getMemoryPtr();
	pheader->tag=DEFAULT_TAG_VALUE ;
	pheader->len = data.getLength() - sizeof(*pheader) ;

	if(bCommonMsg == false)
	{
		//只向特定的逻辑服发送
		SendLogicClientMsg(nServerIndex, data.getMemoryPtr(), data.getLength());
	}
	else
	{
		//发送给会话中心服
		m_pSSManager->getSessionCenter()->SendSSCommonMsg((const int)nServerIndex, data.getMemoryPtr(), data.getLength());
	}

	return TRUE;
}


VOID CSessionServer::PostCloseSessionByAccountId(Uint64 nAccountId,UINT64 lKey)
{
	PostInternalMessage(SSIM_CLOSE_SESSION_BY_ACCOUNTID, nAccountId, lKey,0,0);
}

VOID CSessionServer::PostCloseSessionByAccount(const LPCSTR sAccount)
{
	LPSTR sAccountStr = (LPSTR)m_Allocator.AllocBuffer(sizeof(sAccountStr[0]) * (strlen(sAccount) + 1));
	strcpy(sAccountStr, sAccount);
	PostInternalMessage(SSIM_CLOSE_SESSION_BY_ACCOUNT, (UINT_PTR)sAccountStr, 0, 0);
}

VOID CSessionServer::EnumConnections(EnumConnectionFn lpFn, LPCVOID lpParam)
{
	CSessionClient *pClient;
	UINT uRetCode;
	m_ClientList.lock();
	for ( INT_PTR i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if ( pClient && pClient->connected())
		{
			uRetCode = lpFn(lpParam, pClient);
			if ( uRetCode )
				break;
		}
	}
	m_ClientList.unlock();
}

void CSessionServer::PostAMCTaskMsg(CAMClient::tagTASKMsg &data)
{
	CSessionClient *pClient = 0;
	//unsigned int nActorid =(unsigned int) data.lOPPtr; //角色id
	int  nServerId = data.nServerId; //服务器的id

	OutputMsg(rmTip,"[SessionServer->SessionClient]PostAMCTaskMsg op=%d,serverindex=%d,accountid=%u",
		data.nCmd,data.nServerId,data.nUserId);

	for (INT_PTR i= m_ClientList.count() - 1; i > -1; --i)
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if (pClient && pClient->connected() && pClient->getClientType() == GameServer)
		{
			if(pClient->GetServerId() == nServerId)
			{
				pClient->SendAmTaskAck(data);
				return;
			}
		}
	}

}

void CSessionServer::PostAMCMsg(const CAMClient::AMOPData &data)
{
	CSessionClient *pClient = 0;
	
	//CSessionClient *client = (CSessionClient *)data.lOPPtr;
	
	unsigned int nActorid =(unsigned int) data.lOPPtr; //角色id
	int  nServerId = data.nServerId; //服务器的id

	OutputMsg(rmTip,"[SessionServer->SessionClient]PostAMCMsg op=%d,serverindex=%d,accountid=%u,actorid=%u",
		data.opType,data.nServerId,data.nUserId,nActorid);

	for (INT_PTR i= m_ClientList.count() - 1; i > -1; --i)
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if (pClient && pClient->connected() && pClient->getClientType() == GameServer)
		{
			if(pClient->GetServerId() == nServerId)
			{
				pClient->sendAMCMsgAck(data);
				return;
			}
			/*
			if(client == NULL ) //这个是用来返回充值的
			{

				//OutputMsg(rmError,"pClient=%d",(int)pClient->getClientServerIndex());

				if(pClient->getClientServerIndex() == data.nServerId)
				{
					pClient->sendAMCMsgAck(data);
					return;
				}
			}
			else if(client == pClient )
			{
				pClient->sendAMCMsgAck(data);
				return;
			}
			*/

			
		}
	}
	OutputMsg(rmError,"[SessionServer->SessionClient]PostAMCMsg No client found,op=%d,serverindex=%d,userid=%u",data.opType,data.nServerId,data.nUserId);
}



/*
* Comments:  解封ip
* Parameter: long long lIp:
* @Return  void:
*/
void CSessionServer::UnSealIp(char *sIp)
{
	long long lIp = GetIntIp(sIp);
	if(lIp ==0 )return;

	CBaseList<unsigned long long>&  blackips= m_pSSManager->GetBlackIpList(); //获取黑名单列表

	INT_PTR nCount = blackips.count();
	bool flag =false;
	for(INT_PTR i=0; i< nCount ;i++)
	{
		if(blackips[i] == lIp)
		{
			blackips.remove(i); 
			OutputMsg(rmTip,"unSeal ip=%s",sIp); // 解封这个i
			break;
		}
	}
}
void CSessionServer::SealIp(char *sIp)
{
	long long lIp = GetIntIp(sIp);
	if(lIp ==0 )return;
	//OutputMsg(rmTip,"Seal ip=%s",sIp); // 解封这个ip
	
	CBaseList<unsigned long long>&  blackips= m_pSSManager->GetBlackIpList(); //获取黑名单列表
	INT_PTR nCount = blackips.count();
	bool flag =false;
	INT_PTR i; 
	for( i=0; i< nCount ;i++)
	{
		if(blackips[i] == lIp)
		{
			flag =true;
			break;
		}
	}

	if(!flag)
	{
		OutputMsg(rmTip,"Seal ip=%s",sIp); // 解封这个ip
		blackips.add(lIp);
	}
	else
	{
		return ;
	}
	

	PGAMESESSION *pSessionList =m_SessionList;


	for ( i=m_SessionList.count() - 1; i>-1; --i )
	{
		if ( pSessionList[i]->nIPAddr == lIp )
		{
			OutputMsg(rmTip,"Seal ip=%s,kick account=%s,serverindex=%d",sIp, pSessionList[i]->sAccount, (int)pSessionList[i]->nServerIndex); // 解封这个ip
			ProcessKickCrossActor((unsigned int)pSessionList[i]->nSessionID,pSessionList[i]->nServerIndex);
			CloseSessionAtListIndex(i);
		}
	}

	
}
long long  CSessionServer::GetIntIp(char *sIp)
{
	INT_PTR nSize = strlen(sIp);
	bool flag =true;
	int nDoutCount =0;
	for(INT_PTR i=0; i < nSize; i++)
	{
		//合法的输入
		if(sIp[i] =='.')
		{
			nDoutCount ++;
			//".不能出现在最前面"
			if( i==0 || i== (nSize-1) )
			{
				flag =false;
				break;
			}

		}
		else if(sIp[i] >='0' &&sIp[i] <='9'  )
		{
			continue;
		}
		else
		{
			flag =false;
			break;
		}
	}
	if(nDoutCount < 3)
	{
		flag =false;
	}

	if(!flag )
	{
		OutputMsg(rmWaning,"InValide ip=%s",sIp);
		return 0;
	}
	else
	{
		return inet_addr (sIp); //计算这个ip
	}

}

void CSessionServer::OnSendPlatformResultToClient(int nServerIndex,CDataPacketReader &inPacket)
{
	CSessionClient *pClient = 0;
	for (INT_PTR i= m_ClientList.count() - 1; i > -1; --i)
	{
		pClient = (CSessionClient*)m_ClientList[i];
		if (pClient && pClient->connected() && pClient->getClientType() == GameServer)
		{
			if(pClient->GetServerId() == nServerIndex)
			{
				pClient->OnSendReqCommonPlatformResult(inPacket);
				return;
			}
		}
	}
}
