#include "stdafx.h"

using namespace jxSrvDef;

int CBackClientSocket::g_nSpid = 0;
CBackClientSocket::CBackClientSocket(void)
{
	m_nextCreateTtableTick = 0;
	m_nServerIndex = 0;
	m_nSpid = 0;
	
}
CBackClientSocket::CBackClientSocket(CBackServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr)
{
	this->m_pBackServer = pSrv;
	this->m_pSQLConnection = pSrv->GetSqlConnection();
	this->SetClientSocket(nSocket,pClientAddr);
	m_nextCreateTtableTick = 0;
	m_nServerIndex = 0;
	m_nSpid = 0;
}

CBackClientSocket::~CBackClientSocket(void)
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


}

bool CBackClientSocket::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
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
VOID CBackClientSocket::OnRun()
{
	Inherited::OnRun();
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

			switch(nCmd)
			{
			case REV_LOGICSERVERINFO:		//收到逻辑服的信息
				{
					RevLogicInfo(inPacket);
				}
				break;
			case SEND_COMMAND_RESULT :
				{
					RevCommandResult(inPacket);
					break;
				}

			case SEND_NOTICE_MGR:
				{
					UpdateNotice(inPacket);
					break;
				}

			case SEND_LOGICSPID:
				{
					int nSpid = 0;
					inPacket >> nSpid;
					if(nSpid > 0)
					{
						m_nSpid = nSpid;
					}
					break;
				}

			default:
				break;
			}
		}
#ifdef WIN32
		__except(DefaultExceptHandler(GetExceptionInformation()))
#endif
		{
			//m_pSQLConnection->ResetQuery();
		}
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

int CBackClientSocket::GetSpid()
{
	if(m_nSpid > 0)
	{
		return m_nSpid;
	}

	return g_nSpid;
}

VOID CBackClientSocket::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if (!m_pBackServer || nCmd ==0)
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
#endif
	{

	}

}

void CBackClientSocket::RevLogicInfo(CDataPacketReader &inPacket)
{
	int nServerIndex = 0;
	inPacket >> nServerIndex;

	char sreInfo[512];
	inPacket.readString(sreInfo,ArrayCount(sreInfo));

	m_nServerIndex = nServerIndex;
	/*
	int nError = m_pSQLConnection->Exec(szSQLSP_DelServerInfo,nServerIndex);

	if(!nError)
	{
		m_pSQLConnection->ResetQuery();	
	}

	nError = m_pSQLConnection->Exec(szSQLSP_AddServerInfo,nServerIndex,sreInfo?sreInfo:"");
	if(!nError)
	{
		m_pSQLConnection->ResetQuery();	
	}
	*/
}

void CBackClientSocket::GetNoticeMsg()
{
	CDataPacket &pdata = allocProtoPacket(GET_NOTICE_MGR); //分配一个 网络包
	pdata << (int)m_nServerIndex;
	flushProtoPacket(pdata);
}

void CBackClientSocket::SendCommandToLogic(int nServerIndex,int nOperIndex,char* sCommand,char* sName,int nId)
{
	CDataPacket &pdata = allocProtoPacket(SEND_COMMAND); //分配一个 网络包
	pdata << (int)nServerIndex;
	pdata << (int)nOperIndex;
	pdata << (int)nId;
	pdata.writeString(sName?sName:"");
	pdata.writeString(sCommand?sCommand:"");
	flushProtoPacket(pdata);
}

void CBackClientSocket::RevCommandResult(CDataPacketReader &inPacket)
{
	int nServerIndex = 0;
	inPacket >> nServerIndex;

	int nCmd = 0;
	inPacket >> nCmd;

	int nResult = 0;
	inPacket >> nResult;

	int nId = 0;
	inPacket >> nId;

	char strName[32];
	inPacket.readString(strName,ArrayCount(strName));

	char strCommand[1024];
	inPacket.readString(strCommand,ArrayCount(strCommand));
	char escapeCommand[1024];
	mysql_escape_string(escapeCommand,strCommand, (unsigned long)strlen(strCommand));
	int nError =m_pSQLConnection->Exec(szSQLSP_AddCommandHistory,nId,nServerIndex,nCmd,escapeCommand?escapeCommand:"",nResult,strName?strName:"",GetSpid());
	if (!nError)
	{
		m_pSQLConnection->ResetQuery();
	}
	char sResult[32] = {0};
	sprintf(sResult,"%d,%d,%d",nResult,m_nServerIndex,nCmd);
	m_pBackServer->GetHttpServer().SendResponse(sResult, nId,GetSpid(), nServerIndex);
}

void CBackClientSocket::UpdateNotice(CDataPacketReader &inPacket)
{
	int nServerIndex = 0;
	inPacket >> nServerIndex;

	int operindex = 0;
	inPacket >> operindex;

	int nCount = 0;
	inPacket >> nCount;
	int nError =m_pSQLConnection->Exec(szSQLSP_DelServerNotice,nServerIndex,GetSpid());
	if (!nError)
	{
		m_pSQLConnection->ResetQuery();
	}
	for(int i=0;i<nCount;i++)
	{
		char strMsg[1024],escapeMsg[1024];
		int   nPos,nId,nMins;
		unsigned int nStartTime,nEndTime;
		inPacket >> nId >> nStartTime >> nEndTime >> nMins >> nPos;
		inPacket.readString(strMsg,ArrayCount(strMsg));
		mysql_escape_string(escapeMsg,strMsg,(unsigned long)strlen(strMsg));
		nError =m_pSQLConnection->Exec(szSQLSP_AddServerNotice,nId,nServerIndex,operindex,escapeMsg?escapeMsg:"",GetSpid(),nStartTime,nEndTime,nMins,nPos);
		if (!nError)
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

VOID CBackClientSocket::OnDisconnected()
{
	if(m_pBackServer)
	{
		m_pBackServer->DelClient(this);
	}
}