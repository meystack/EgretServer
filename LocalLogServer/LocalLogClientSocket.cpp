#include "StdAfx.h"

using namespace jxSrvDef;

int CLocalLogClientSocket::g_nSpid = 0;
CLocalLogClientSocket::CLocalLogClientSocket(void)
{
	m_nextCreateTtableTick = 0;
}
CLocalLogClientSocket::CLocalLogClientSocket(CLocalLogServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr)
{
	this->pSrv = pSrv;
	this->m_pSQLConnection = pSrv->GetSqlConnection();
	this->SetClientSocket(nSocket,pClientAddr);
	m_nextCreateTtableTick = 0;
}

CLocalLogClientSocket::~CLocalLogClientSocket(void)
{
	OutputMsg(rmTip,_T("log client[%s] Destroy"),getClientName());
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


}

bool CLocalLogClientSocket::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
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
VOID CLocalLogClientSocket::OnRun()
{
	Inherited::OnRun();

	//没有连接上就不处理
	
	/*
	if(!m_pSQLConnection->Connected())
	{
		return;
	}
	*/

	char sBuf[1024];
	char szSql[2048];
	sBuf[0] = 0;
	szSql[0] = 0;

	int nSpid = 0,nServerIndex = 0;
	int nSrvId = 0;
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	//OutputMsg(rmTip,"SingleRun thread id=%d",(int)GetCurrentThread());
	jxSrvDef::INTERSRVCMD nCmd =0;
	TICKCOUNT nStartTick = _getTickCount(); //获取当前的时间tick
	while(m_dataList.count() >0)
	{
		CDataPacket* pDataPatck = m_dataList[0];
		
		CDataPacketReader inPacket(pDataPatck->getMemoryPtr(),pDataPatck->getPosition());
		inPacket >> nCmd  ; 
#ifdef WIN32
		__try
#endif
		{
			bool canExec = false;
			switch(nCmd)
			{
			case LOG_CREATETABLE_CMD:
				{
					CreateCommonTable(inPacket);
					break;
				}
			case LOG_SEND_CURRENCY_CMD: //发送资产日志
				{
					LOGCURRENCY data;
					inPacket >> data;
					inPacket >> nSpid;
					inPacket >> nServerIndex;
					inPacket >> nSrvId;
					data.szName[sizeof(data.szName) -1]=0;
					data.szAccount[sizeof(data.szAccount) -1]=0;
					sprintf(szSql,m_szInsertCurrency,st.wYear,st.wMonth,
						(int)data.nLogId,(int)data.nLogType,(int)data.nCurrencyType,data.nAmount,data.nBalance,(int)data.nIsShop,
						data.szAccount,data.nActorid,data.szName,nServerIndex,nSrvId);
					//OutputMsg(rmTip,"资产日志:%s",szSql);
					canExec = true;
					break;
				}
			case LOG_SEND_SHOP_CMD: //发送商城日志
				{
					LOGSHOP data;
					inPacket >> data;
					inPacket >> nSpid;
					inPacket >> nServerIndex;
					inPacket >> nSrvId;
					data.szName[sizeof(data.szName) -1]=0;
					data.szAccount[sizeof(data.szAccount) -1]=0;
					sprintf(szSql,m_szInsertShop,st.wYear,st.wMonth,
						(int)data.nShopType,(int)data.nItemId,(int)data.nItemType,(int)data.nItemNum,(int)data.nCurrencyType,data.nConsumeNum,
						data.szAccount,data.nActorid,data.szName,nServerIndex,nSrvId);
					canExec = true;
					break;
				}
			case LOG_SEND_ITEM_CMD: //发送道具日志
				{
					LOGITEM data;
					inPacket >> data;
					inPacket >> nSpid;
					inPacket >> nServerIndex;
					inPacket >> nSrvId;
					data.szName[sizeof(data.szName) -1]=0;
					data.szAccount[sizeof(data.szAccount) -1]=0;
					data.sReason[sizeof(data.sReason) -1]=0;
					sprintf(szSql,m_szInsertItem,st.wYear,st.wMonth,
						(int)data.nItemId,(int)data.nItemType,(int)data.nItemNum,(int)data.nOperType,data.sReason,
						data.szAccount,data.nActorid,data.szName,nServerIndex,nSrvId);
					canExec = true;
					break;
				}
			case LOG_SEND_TASK_CMD: //发送任务日志
				{
					LOGTASK data;
					inPacket >> data;
					inPacket >> nSpid;
					inPacket >> nServerIndex;
					inPacket >> nSrvId;
					data.szName[sizeof(data.szName) -1]=0;
					data.szAccount[sizeof(data.szAccount) -1]=0;
					sprintf(szSql,m_szInsertTask,st.wYear,st.wMonth,
						(int)data.nTaskId,(int)data.nTaskType,(int)data.nStatu,
						data.szAccount,data.nActorid,data.szName,nServerIndex,nSrvId);
					canExec = true;
					break;
				}
			case LOG_SEND_ACTIVITY_CMD: //发送活动日志
				{
					LOGACTIVITY data;
					inPacket >> data;
					inPacket >> nSpid;
					inPacket >> nServerIndex;
					inPacket >> nSrvId;
					data.szName[sizeof(data.szName) -1]=0;
					data.szAccount[sizeof(data.szAccount) -1]=0;
					sprintf(szSql,m_szInsertActivity,st.wYear,st.wMonth,
						(int)data.nAtvId,(int)data.nAtvType,(int)data.nStatu,
						data.szAccount,data.nActorid,data.szName,nServerIndex,nSrvId);
					canExec = true;
					break;
				}

			default:
				break;
			}

			if (canExec)
			{
				int nResult = m_pSQLConnection->RealExec(szSql,strlen(szSql));
				if (nResult != 0)
				{
					//执行插入操作出错，要记录文本日志
					OutputMsg( rmError, _T("exec sql error!!:%s"),szSql);
				}
				else m_pSQLConnection->ResetQuery();
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
VOID CLocalLogClientSocket::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
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

	}
#endif

}

VOID CLocalLogClientSocket::InsertCommon(CDataPacketReader &inPacket)
{
	LOGGERDATACOMMON data;
	inPacket >> data;
	int nSpid = 0;
	inPacket >> nSpid;
	if(g_nSpid > 0 && nSpid <= 0)
	{
		nSpid = g_nSpid;
	}
	int nServerIndex = 0;
	inPacket >> nServerIndex;

	data.szShortStr0[sizeof(data.szShortStr0) -1]=0;
	data.szShortStr1[sizeof(data.szShortStr1) -1]=0;
	data.szMidStr0[sizeof(data.szMidStr0) -1]=0;
	data.szMidStr1[sizeof(data.szMidStr1) -1]=0;
	data.szMidStr2[sizeof(data.szMidStr2) -1]=0;
	data.szLongStr0[sizeof(data.szLongStr0) -1]=0;
	data.szLongStr1[sizeof(data.szLongStr1) -1]=0;
	data.szLongStr2[sizeof(data.szLongStr2) -1]=0;
	data.szName[sizeof(data.szName) -1] =0;
	data.szAccount[sizeof(data.szAccount) -1] =0;
	data.szSpid[sizeof(data.szSpid)-1] = 0;
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szSql[2048];
	sprintf(szSql,m_szCommonLogSql,nSpid,data.nServerIndex,st.wYear,st.wMonth,st.wDay,
		data.nLogid,data.nActorid,data.szName,data.szAccount,data.nServerIndex,
		data.nPara0,data.nPara1,data.nPara2,
		data.szShortStr0,data.szShortStr1,
		data.szMidStr0,data.szMidStr1,data.szMidStr2,
		data.szLongStr0,data.szLongStr1,data.szLongStr2,data.szSpid);
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

VOID CLocalLogClientSocket::CreateCommonTable(CDataPacketReader &inPacket)
{
	int nSpid = 0;
	inPacket >> nSpid;
	if(g_nSpid > 0 && nSpid <= 0)
	{
		nSpid = g_nSpid;
	}
	int nServerIndex = 0;
	inPacket >> nServerIndex;

	TICKCOUNT nCurrentTick = _getTickCount();

	if(nCurrentTick < m_nextCreateTtableTick)
	{
		return;
	}

	int nCount = 0;
	const int TOTAL = 3600;

	SYSTEMTIME Systime;
	GetLocalTime( &Systime );
	WORD nDay = Systime.wDay;
	WORD nMonth = Systime.wMonth;
	WORD nYear = Systime.wYear;

	bool isSucceed= true; //是否都执行成功
#ifdef WIN32
	__try
#endif
	{
		//生成未来2个月的表格
		for(int i = 0; i < 2; i++)
		{
			// 创建资产日志表
			if (m_pSQLConnection->Exec(m_szCreateCurrency,nYear,nMonth) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表资产表失败！sql:%s"),m_szCreateCurrency);
				isSucceed =false;
			}
			else m_pSQLConnection->ResetQuery();

			// 创建商城日志表
			if (m_pSQLConnection->Exec(m_szCreateShop,nYear,nMonth) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表商城表失败！sql:%s"),m_szCreateShop);
				isSucceed =false;
			}
			else m_pSQLConnection->ResetQuery();

			// 创建道具日志表
			if (m_pSQLConnection->Exec(m_szCreateItem,nYear,nMonth) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表道具表失败！sql:%s"),m_szCreateItem);
				isSucceed =false;
			}
			else m_pSQLConnection->ResetQuery();

			// 创建任务日志表
			if (m_pSQLConnection->Exec(m_szCreateTask,nYear,nMonth) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表任务表失败！sql:%s"),m_szCreateTask);
				isSucceed =false;
			}
			else m_pSQLConnection->ResetQuery();

			// 创建活动日志表
			if (m_pSQLConnection->Exec(m_szCreateActivity,nYear,nMonth) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表活动表失败！sql:%s"),m_szCreateActivity);
				isSucceed =false;
			}
			else m_pSQLConnection->ResetQuery();

			if (m_pSQLConnection->Exec(m_szCreateRechargeAll) != 0)
			{
				OutputMsg(rmError,_T("建表充值查询表失败！sql:%s"),m_szCreateRechargeAll);
				isSucceed =false;
			}
			else m_pSQLConnection->ResetQuery();
			if (m_pSQLConnection->Exec(m_szCreateRoleAll) != 0)
			{
				OutputMsg(rmError,_T("建表角色查询表失败！sql:%s"),m_szCreateRoleAll);
				isSucceed =false;
			}
			else m_pSQLConnection->ResetQuery();

			//下一月
			nMonth++;
			if (nMonth > 12)
			{
				nMonth = 1;
				nYear++;
			}
		}

		DeleteCommonTable(nSpid,nServerIndex);

		if(isSucceed)
		{
			m_nextCreateTtableTick = nCurrentTick + CREATE_LOCALTABLE_INTERVAL ;  //下次调用是10天以后的事情
		}
	}
#ifdef WIN32
	__except(DefaultExceptHandler(GetExceptionInformation()))
	{
		m_pSQLConnection->ResetQuery();
		m_nextCreateTtableTick =0;
	}
#endif
}

VOID CLocalLogClientSocket::DeleteCommonTable(int nSpid,int nServerIndex)
{
	struct tm; 
	SYSTEMTIME Systime;
	GetLocalTime( &Systime );
	int nDay = Systime.wDay;
	int nMonth = Systime.wMonth;
	int nYear = Systime.wYear;

	nMonth -= 2;
	if(nMonth <= 0)
	{
		nMonth += 12;
		nYear--;
	}

	for(int j=1;j<=nMonth;j++)
	{
		char szSql[1024];

		// 删除旧的资产日志表
		OutputMsg(rmTip,"删表 currency_%d%02d",nYear,j);
		sprintf(szSql,m_szDeleteCurrency,nYear,j);
		if (m_pSQLConnection->RealExec(szSql,strlen(szSql)))
		{
			OutputMsg(rmError,_T("删表失败！sql:%s"),szSql);//失败
		}
		else m_pSQLConnection->ResetQuery();

		// 删除旧的商城日志表
		OutputMsg(rmTip,"删表 shop_%d%02d",nYear,j);
		sprintf(szSql,m_szDeleteShop,nYear,j);
		if (m_pSQLConnection->RealExec(szSql,strlen(szSql)))
		{
			OutputMsg(rmError,_T("删表失败！sql:%s"),szSql);//失败
		}
		else m_pSQLConnection->ResetQuery();

		// 删除旧的道具日志表
		OutputMsg(rmTip,"删表 item_%d%02d",nYear,j);
		sprintf(szSql,m_szDeleteItem,nYear,j);
		if (m_pSQLConnection->RealExec(szSql,strlen(szSql)))
		{
			OutputMsg(rmError,_T("删表失败！sql:%s"),szSql);//失败
		}
		else m_pSQLConnection->ResetQuery();

		// 删除旧的任务日志表
		OutputMsg(rmTip,"删表 task_%d%02d",nYear,j);
		sprintf(szSql,m_szDeleteTask,nYear,j);
		if (m_pSQLConnection->RealExec(szSql,strlen(szSql)))
		{
			OutputMsg(rmError,_T("删表失败！sql:%s"),szSql);//失败
		}
		else m_pSQLConnection->ResetQuery();

		// 删除旧的活动日志表
		OutputMsg(rmTip,"删表 activity_%d%02d",nYear,j);
		sprintf(szSql,m_szDeleteActivity,nYear,j);
		if (m_pSQLConnection->RealExec(szSql,strlen(szSql)))
		{
			OutputMsg(rmError,_T("删表失败！sql:%s"),szSql);//失败
		}
		else m_pSQLConnection->ResetQuery();
	}

	//DelLastYearTable(nSpid,nServerIndex);
}

void CLocalLogClientSocket::DelLastYearTable(int nSpid,int nServerIndex)
{
	int nMonth = 12;
	int nYear = 2012;

	for(int j=1;j<=nMonth;j++)
	{
		for(int i=0;i<30;i++)
		{
			char date[20];
			sprintf(date,"%d%d%d",nYear,j,i);

			char szSql[1024];
			sprintf(szSql,m_szDeleteCommonSql,nSpid,nServerIndex,nYear,j,i);

			//OutputMsg(rmTip,"删表 szSql=%s",szSql);
			if (m_pSQLConnection->RealExec(szSql,strlen(szSql)))
			{
				//失败
				OutputMsg(rmError,_T("删表失败！sql:%s"),szSql);
			}
			else
			{
				m_pSQLConnection->ResetQuery();
			}
		}
	}
}
