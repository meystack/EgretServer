#include "stdafx.h"

using namespace jxSrvDef;
CLoggerClientSocket::CLoggerClientSocket(void)
{

}
CLoggerClientSocket::CLoggerClientSocket(CLoggerServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr)
{
	this->pSrv = pSrv;
	this->m_pSQLConnection = pSrv->GetSqlConnection();
	this->SetClientSocket(nSocket,pClientAddr);
#ifndef WIN32
	this->m_pLocalSender = new CLocalSender();
#endif
}

CLoggerClientSocket::~CLoggerClientSocket(void)
{
	OutputMsg(rmNormal,_T("log client[%s] Destroy"),getClientName());

	for(int i = 0; i < m_dataList.count(); i++)
	{
		CDataPacket* dp = m_dataList[i];
		dp->setPosition(0);
		flushSendPacket(*dp);
	}
	m_dataList.clear();

	for(int i = 0; i < m_freeDataList.count(); i++)
	{
		CDataPacket* dp = m_freeDataList[i];
		dp->setPosition(0);
		flushSendPacket(*dp);
	}
	m_freeDataList.clear();
#ifndef WIN32
	if(this->m_pLocalSender)
		delete this->m_pLocalSender;
#endif

}

bool CLoggerClientSocket::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
{
	//游戏服和db能够登陆
	if ( pRegData && pRegData->GameType == SERVER_REGDATA::GT_JianXiaoJiangHu && 
		( pRegData->ServerType == GameServer || pRegData->ServerType == DBServer
		 || pRegData->ServerType == SessionServer) )
	{
		return true;
	}
	else
	{
		return false;
	}
}
VOID CLoggerClientSocket::OnRun()
{
	Inherited::OnRun();

	//没有连接上就不处理
	
	/*
	if(!m_pSQLConnection->Connected())
	{
		return;
	}
	*/


	//OutputMsg(rmTip,"SingleRun thread id=%d",(int)GetCurrentThread());
	jxSrvDef::INTERSRVCMD nCmd =0;
	TICKCOUNT nStartTick = _getTickCount(); //获取当前的时间tick
	while(m_dataList.count() >0)
	{
		int nServerIndex = 0;
		CDataPacket* pDataPatck = m_dataList[0];
		
		CDataPacketReader inPacket(pDataPatck->getMemoryPtr(),pDataPatck->getPosition());
		inPacket >> nCmd  ; 
#ifdef WIN32
		__try
#endif
		{

			switch(nCmd)
			{
			case LOG_LOGIN_CMD: //登陆日志
				InsertLogin(inPacket);
				break;
			case LOG_CONSUME_CMD: //消费日志
				InsertConsume(inPacket);
				break;
			case LOG_DEAL_CMD: //交易日志
				InsertDeal(inPacket);
				break;

			case LOG_ITEAM_CMD: //道具日志
				InsertItem(inPacket);
				break;
			case LOG_ONLINE_CMD: //在线日志
				InsertOnline(inPacket);
				break;
			case LOG_KILLDROP_CMD: //击杀掉落日志
				InsertKillDrop(inPacket);
				break;
			case LOG_JOINATV_CMD: //
				InsertJoinAtv(inPacket);
				break;
			case LOG_COMMON_CMD: //公共操作日志
				
				break;
			case LOG_SUGGEST_CMD: //建议
				//InsertSuggest(inPacket);
				break;
			case LOG_SENDSERVERINDEX_CMD:
				inPacket >> nServerIndex;
				m_nServerIndex = nServerIndex;
				if(GetLogServer() && GetLogServer()->pLoggerSrv)
				{
					GetLogServer()->pLoggerSrv->AddServerClient(this);
					GetLogServer()->pLoggerSrv->SendOpenChatLogFlag(nServerIndex,1);
				}
				break;

			case LOG_SENDCHATRECORD_CMD:	//聊天记录
				SendChatLogToClient(inPacket);
				break;
			default:
				break;
			}
		}
#ifdef WIN32
		__except(DefaultExceptHandler(GetExceptionInformation()))
		{
			m_pSQLConnection->ResetQuery();
		}
#endif
		m_freeDataList.add(pDataPatck);
		m_dataList.remove(0);		// 删除此消息

		//如果执行的时间大于200毫秒，则退出执行
		TICKCOUNT nCurrentTick = _getTickCount() ;
		if(nCurrentTick -  nStartTick > 200)
		{
			break;
		}
	}
}
VOID CLoggerClientSocket::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if (!pSrv || nCmd ==0)
	{
		return;
	}
#ifdef WIN32
	__try
#endif
	{
	
		//如果数据不够，每次都申请512个数据包
		if(m_freeDataList.count() <=0 )
		{
			allocSendPacketList(m_freeDataList, 512);
		}
		
		CDataPacket* pDataPatck = m_freeDataList.pop();//得到一个空闲的Datapacket
		pDataPatck->setPosition(0);
		//TRACE("free m_TempData:%d\n",(int)(m_TempData));

		//复制到内存块中
		(*pDataPatck) << nCmd;
		pDataPatck->writeBuf(inPacket.getOffsetPtr(),inPacket.getLength());

		m_dataList.add(pDataPatck);	
		
		//OutputMsg(rmTip,"OnDispatchRecvPacket thread id=%d,ncmd=%d",(int)GetCurrentThread(),(int)nCmd);
		
		//每次都申请一批数据包
		/*
		
		*/
		
	}
#ifdef WIN32
	__except(DefaultExceptHandler(GetExceptionInformation()))
	{
		OutputMsg(rmError,"error,nCmd=%d",nCmd);
	}
#endif

}

VOID CLoggerClientSocket::InsertLogin(CDataPacketReader &inPacket)
{
	LOGDATALOGIN data;
	
	inPacket >> data;

	data.szAccount[sizeof(data.szAccount)-1]=0;
	data.szLoginDescr[sizeof(data.szLoginDescr)-1] =0;
	data.szLoginIp[sizeof(data.szLoginIp)-1] =0;
	data.szSpid[sizeof(data.szSpid)-1] = 0;
	
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[1024];
	char sztime[1024] = {0};
	// struct tm gm = {st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth-1, st.wYear-1900, st.wDayOfWeek, 0, 0};
	// int time1 = mktime(&gm);
	// sprintf(sztime,"%d/%02d/%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond); //actotID
	sprintf(szSql,m_szLoginLogSql,st.wYear,st.wMonth,st.wDay,
		data.nLogid,data.nUserid,data.szAccount,data.nServerIndex,
		data.szLoginIp,data.nServerType,data.szLoginDescr,data.szSpid,data.nLevel, data.nYb, data.nOnlineTime,data.nSrvId);
	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	// printf("%s\n",szSql);
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		char szError[1024];
		sprintf(szError,_T("exec sql error!!:%s"),szSql);
		OutputMsg( rmError, szError);
	}
	else
	{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}

}

VOID CLoggerClientSocket::InsertConsume(CDataPacketReader &inPacket)
{
	LOGDATACONSUME data;
	inPacket >> data;

	data.szAccount[sizeof(data.szAccount)-1]=0;
	data.szCharName[ sizeof(data.szCharName)-1] =0;
	data.szConsumeDescr[sizeof(data.szConsumeDescr)-1] =0;
	data.szSpid[sizeof(data.szSpid)-1] = 0;

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[2048];
	char sztime[1024] = {0};
	// sprintf(sztime,"%d/%02d/%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond); //actotID
	sprintf(szSql,m_szConsumeLogSql,st.wYear,st.wMonth,st.wDay,
		data.nLogid,data.nActorid,data.szAccount,data.nServerIndex,
		data.szCharName,data.cMoneytype,data.nConsumecount,data.nPaymentCount,data.szConsumeDescr,data.nConsumeLevel,data.nBalance,data.szSpid,data.nSrvId);
	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
	}
	else{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}
}

VOID CLoggerClientSocket::InsertDeal(CDataPacketReader &inPacket)
{
	LOGACTORDEALSAVE data;
	inPacket >> data;

	data.szSpid[sizeof(data.szSpid)-1] = 0;

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[2048];
	
	sprintf(szSql,m_szDealLogSql,st.wYear,st.wMonth,st.wDay,
		data.nServerIndex, data.nLogId, data.nMiniTime, data.nActorid, data.nTarActorid, data.nItemId, data.nItemNum, data.nMoneyType, data.nMoneyNum, data.nDues, data.szSpid,data.nSrvId);
	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
	}
	else{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}
}


VOID CLoggerClientSocket::InsertItem(CDataPacketReader &inPacket)
{
	LOGDATAITEM data;
	inPacket >> data;

	data.szAccount[sizeof(data.szAccount)-1]=0;
	data.szCharName[ sizeof(data.szCharName)-1] =0;
	data.szConsumeDescr[sizeof(data.szConsumeDescr)-1] =0;
	data.szSpid[sizeof(data.szSpid)-1] = 0;

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[2048];
	char sztime[1024] = {0};
	// sprintf(sztime,"%d/%02d/%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond); //actotID
	sprintf(szSql,m_szItemLogSql,st.wYear,st.wMonth,st.wDay,
		data.nLogid,data.nActorid,data.szAccount,data.nServerIndex,
		data.szCharName,data.nType,data.wItemId,data.nCount,data.szConsumeDescr,data.szSpid,data.nSrvId);

	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
	}
	else{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}
}

VOID CLoggerClientSocket::InsertOnline(CDataPacketReader &inPacket)
{
	LOGDATAONLINECOUNT data;
	inPacket >> data;
	data.szSpid[sizeof(data.szSpid)-1] = 0;
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[1024];
	char sztime[1024] = {0};
	// sprintf(sztime,"%d/%02d/%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond); //actotID
	sprintf(szSql,m_szOnlineLogSql,st.wYear,st.wMonth,st.wDay,
		data.nServerIndex,data.nOnlineCount,data.szSpid);
	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
	}
	else
	{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}
}
/*
VOID CLoggerClientSocket::InsertSuggest(CDataPacketReader &inPacket)
{
	LOGGERDATASUGGEST data;
	inPacket >> data;
	data.szAccount[sizeof(data.szAccount) -1]=0;
	data.szCharName[sizeof(data.szCharName) -1]=0;
	
	data.szTitle[sizeof(data.szTitle) -1]=0;
	data.szSuggest[sizeof(data.szSuggest) -1]=0;

	//替换，避免出现字符串把sql出错了
	INT_PTR nCount= strlen(data.szTitle);
	for(INT_PTR i=0; i<nCount; i++ )
	{
		if( data.szTitle[i] =='`' || data.szTitle[i] == '\'')
		{
			data.szTitle[i]= '!';
		}
	}

	nCount= strlen(data.szSuggest);
	for(INT_PTR i=0; i<nCount; i++ )
	{
		if( data.szSuggest[i] =='`' || data.szSuggest[i] == '\'')
		{
			data.szSuggest[i]= '!';
		}
	}
	
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[1024];
	sprintf(szSql,m_szSuggestLogSql,st.wYear,st.wMonth,st.wDay,
		data.szAccount, data.nServerIndex,data.szCharName,data.nType,data.szTitle,data.szSuggest,data.nActorLevel);
	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
	}
	else
	{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}

}
*/

VOID CLoggerClientSocket::OnDisconnected()
{
	if(GetLogServer() && GetLogServer()->pLoggerSrv)
	{
		GetLogServer()->pLoggerSrv->AddServerClient(this,1);
	}
}

void CLoggerClientSocket::SendChatLogToClient(CDataPacketReader &inPacket)
{
	int nChannleID = 0;
	inPacket >> nChannleID;

	int nActorId = 0;
	inPacket >> nActorId;

	char sActorName[32];
	inPacket.readString(sActorName,ArrayCount(sActorName));

	char msg[1024];
	inPacket.readString(msg,ArrayCount(msg));

	//OutputMsg(rmTip,"SendChatLogToClient,nServerIndex=%d,nChannleID=%d",m_nServerIndex,nChannleID);
#ifdef WIN32
	if(GetLogServer() && GetLogServer()->pChatLogSrv)
	{
		GetLogServer()->pChatLogSrv->SendChatRecordToAll(m_nServerIndex,nChannleID,nActorId,sActorName,msg);
	}
#else
	if(m_pLocalSender && m_pLocalSender->IsInUse())
		m_pLocalSender->SendMsgToSP(m_nServerIndex,nChannleID,nActorId,sActorName,msg);
#endif

}


VOID CLoggerClientSocket::InsertKillDrop(CDataPacketReader &inPacket)
{
	LOGKILLDROP data;
	inPacket >> data;
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[1024];
	char sztime[1024] = {0};

	sprintf(szSql,m_szdropLogSql,st.wYear,st.wMonth,st.wDay,
		data.nActorid,data.srtAccount,data.nServerIndex,
		data.strName,data.strMonsetName,data.strSceneName,data.strItemName,data.nCount,data.nKilltime,data.nPosx,data.nPosy,data.nSrvId);

	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
	}
	else
	{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}
}




VOID CLoggerClientSocket::InsertJoinAtv(CDataPacketReader &inPacket)
{
	LOGATVSCHEDULE data;
	inPacket >> data;
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[1024];
	char sztime[1024] = {0};

	sprintf(szSql,m_szAtvLogSql,st.wYear,st.wMonth,st.wDay,
		data.nActorid,data.srtAccount,data.nServerIndex,
		data.strName,data.nAtvID,data.nJoinTime,data.nIndex,data.nSrvId);

	int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
	BOOL bReturn = FALSE;
	if (nResult != 0)
	{
		//执行插入操作出错，要记录文本日志
		OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
	}
	else
	{
		//释放结果集
		m_pSQLConnection->ResetQuery();
	}
}