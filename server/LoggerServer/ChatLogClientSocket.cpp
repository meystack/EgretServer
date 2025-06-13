#include "stdafx.h"

using namespace jxSrvDef;
CChatLogClientSocket::CChatLogClientSocket(void)
{

}
CChatLogClientSocket::CChatLogClientSocket(CChatLogServerSocket* Srv,SOCKET nSocket, SOCKADDR_IN *pClientAddr)
{
	if(Srv)
	{
		pSrv = Srv;
		this->m_pSQLConnection = pSrv->GetSqlConnection();
	}
	this->SetClientSocket(nSocket,pClientAddr);

	m_nCount = 0;
}

CChatLogClientSocket::~CChatLogClientSocket(void)
{
	OutputMsg(rmNormal,_T("chat log client Destroy"));
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

bool CChatLogClientSocket::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
{
	OutputMsg(rmTip,"Chat ClientLogin");
	//游戏服和db能够登陆
	if ( pRegData )/* && pRegData->GameType == SERVER_REGDATA::GT_JianXiaoJiangHu && 
		( pRegData->ServerType == GameServer || pRegData->ServerType == DBServer
		 || pRegData->ServerType == SessionServer) )*/
	{
		return true;
	}
	else
	{
		return false;
	}
}
VOID CChatLogClientSocket::OnRun()
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
		CDataPacket* pDataPatck = m_dataList[0];
		
		CDataPacketReader inPacket(pDataPatck->getMemoryPtr(),pDataPatck->getPosition());
		inPacket >> nCmd  ; 
		//OutputMsg(rmTip,"Rcv Chat Client Msg ID=%d",(int)nCmd);
#ifdef WIN32
		__try
#endif
		{

			switch(nCmd)
			{
			case REV_CHECKSEVERINDEX_CMD:			//查看日志
				OnRevLookServerIndex(inPacket);
				break;
			case REV_SHUTUPCHAT_CMD:				//禁言
				OnRevShutUpOp(inPacket);
				break;
			default:
				break;
			}
		}
#ifdef WIN32		
		__except(DefaultExceptHandler(GetExceptionInformation()))
		{
			//m_pSQLConnection->ResetQuery();
			OutputMsg(rmError,"Error,Rcv Msg ID=%d",(int)nCmd);

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
VOID CChatLogClientSocket::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
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

VOID CChatLogClientSocket::OnRevLookServerIndex(CDataPacketReader &inPacket)
{
	int nServerIndex = 0;
	inPacket >> nServerIndex;

	int nFlag = 0;
	inPacket >> nFlag;


	//OutputMsg(rmTip,"OnRevLookServerIndex,nServerIndex=%d,nFlag = %d",nServerIndex,nFlag);

	if(nServerIndex <= 0) return;

	for(int i=0;i<m_nCount;i++)
	{
		if(nServerIndex == m_ServerList[i])
		{
			return;
		}
	}

	if(nFlag == 1 && GetLogServer() && GetLogServer()->pLoggerSrv)
	{
		if(m_nCount < MAXCOUNTSERVER)
		{
			m_ServerList[m_nCount] = nServerIndex;
			m_nCount++;
		}
		GetLogServer()->pLoggerSrv->SendOpenChatLogFlag(nServerIndex,1);
	}

	if(nFlag == 0 && GetLogServer() && GetLogServer()->pChatLogSrv)
	{
		RemoveSerindex(nServerIndex);
		if (!GetLogServer()->pChatLogSrv->IsLogOnServerIndex(nServerIndex))
		{
			GetLogServer()->pLoggerSrv->SendOpenChatLogFlag(nServerIndex,0);
		}
		
	}
	//CDataPacket &pdata = allocProtoPacket(SEND_CHATLOG_CMD); //分配一个 网络包
	//pdata << (int)nServerIndex;
	//pdata.writeString("test chat");
	//flushProtoPacket(pdata);

}

VOID CChatLogClientSocket::OnRevShutUpOp(CDataPacketReader &inPacket)
{
	int nServerIndex = 0;
	inPacket >> nServerIndex;

	int nType = 0;
	inPacket >> nType;

	char sName[32];
	inPacket.readString(sName,ArrayCount(sName));

	int nTime = 0;
	inPacket >> nTime;

	GetLogServer()->pLoggerSrv->SendShutUpMsg(nServerIndex,nType,sName,nTime);
}

void CChatLogClientSocket::RemoveSerindex(int nIndex)
{
	for(int i=0;i<m_nCount;i++)
	{
		if(m_ServerList[i] == nIndex)
		{
			for(int j=i;j<m_nCount-1;j++)
			{
				m_ServerList[j] = m_ServerList[j+1];
			}
			m_nCount--;
			break;
		}
	}
}


VOID CChatLogClientSocket::OnDisconnected()
{
	for(int i=m_nCount-1;i>=0;i--)
	{
		int nServerIndex = m_ServerList[i];
		RemoveSerindex(nServerIndex);
		if (GetLogServer() && GetLogServer()->pChatLogSrv && !GetLogServer()->pChatLogSrv->IsLogOnServerIndex(nServerIndex))
		{
			if (GetLogServer()->pLoggerSrv)
			{
				GetLogServer()->pLoggerSrv->SendOpenChatLogFlag(nServerIndex,0);
			}

		}

	}

	if(GetLogServer() && GetLogServer()->pChatLogSrv)
	{
		GetLogServer()->pChatLogSrv->AddLogServerClient(this,1);
	}
}
