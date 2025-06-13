#include "StdAfx.h"
#include "FeeDb.h"

CLogicDBReqestHandler::CLogicDBReqestHandler()
{
	m_pHugeSQLBuffer = (PCHAR)m_Allocator.AllocBuffer(dwHugeSQLBufferSize);	
}

CLogicDBReqestHandler::~CLogicDBReqestHandler()
{
	m_Allocator.FreeBuffer(m_pHugeSQLBuffer);
}

CDataPacket& CLogicDBReqestHandler::allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd)
{
	return m_pHost->AllocDataPacket(nCmd);
}

void CLogicDBReqestHandler::flushProtoPacket(CDataPacket &packet)
{
	m_pHost->FlushDataPacket(packet);
}

//int CLogicDBReqestHandler::getClientServerIndex()
//{
//	return m_pHost->GetClientServerIndex();
//}

void CLogicDBReqestHandler::ExecuInitDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0;
	inPacket >> nRawServerId;
	if (m_pSQLConnection->Connected())
	{
		int nError = m_pSQLConnection->Exec(szSQL_InitDB, nRawServerId);
		if (!nError)
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

void CLogicDBReqestHandler::Trace ()
{
	OutputMsg(rmNormal,"---CLogicDBReqestHandler memory start---");
	CBufferAllocator::ALLOCATOR_MEMORY_INFO memoryInfo;
	m_Allocator.GetMemoryInfo(&memoryInfo);
	OutputMsg(rmNormal,"super alloc=%d,free=%d",(int)memoryInfo.SuperBuffer.dwAllocSize,(int)memoryInfo.SuperBuffer.dwFreeSize);
	OutputMsg(rmNormal,"large alloc=%d,free=%d",(int)memoryInfo.LargeBuffer.dwAllocSize,(int)memoryInfo.LargeBuffer.dwFreeSize);
	OutputMsg(rmNormal,"middle alloc=%d,free=%d",(int)memoryInfo.MiddleBuffer.dwAllocSize,(int)memoryInfo.MiddleBuffer.dwFreeSize);
	OutputMsg(rmNormal,"small alloc=%d,free=%d",(int)memoryInfo.SmallBuffer.dwAllocSize,(int)memoryInfo.SmallBuffer.dwFreeSize);
	OutputMsg(rmNormal,"---CLogicDBReqestHandler memory end---");
}

void CLogicDBReqestHandler::OnResponseDbSessionData(CDataPacketReader &inPacket)
{
	unsigned int nSessionId=0,nActorId=0;
	inPacket >> nSessionId;
	inPacket >> nActorId;
	//////GetGlobalDBEngine()->getGateManager()->PostLogicResponseSessionData(nSessionId, nActorId);
}

void CLogicDBReqestHandler::UpdateLogicGateUserInfo(CDataPacketReader &inPacket)
{
	GetGlobalDBEngine()->UpdateLogicGateUserInfo(inPacket);
}


void CLogicDBReqestHandler::LogicSendStopDbServerCmd(CDataPacketReader &inPacket)
{
	if(GetGlobalDBEngine())
	{
		GetGlobalDBEngine()->SetDbStartFlag(false);
	}
}

void CLogicDBReqestHandler::OnReloadCrossConfig(CDataPacketReader &inPacket)
{
	/*
	if(GetGlobalDBEngine())
	{
	
		char	szAddr[128];
		szAddr[0]=0;
		int		nPort=0;
		int nServerIndex =0;
		int nRawServerIndex =0;
		inPacket >> nServerIndex >> nRawServerIndex;
		
		inPacket.readString(szAddr,sizeof(szAddr));
		inPacket >> nPort;
		CDBCenterClient* pClient = GetGlobalDBEngine()->getDBCenterClient();
		if(pClient ==NULL)
		{
			OutputMsg(rmError,"CDBCenterClient is NULL");
			return;
		}
		OutputMsg(rmTip,"serverindex=%d,dbcenter=%s,port=%d ",nRawServerIndex,szAddr?szAddr:"",nPort);

		//如果当前正在连接
		if(pClient->connected())
		{
			
			//如果不需要改变，则不改变
			if(pClient->GetServerPort() == nPort && strcmp( pClient->GetServerHost(),szAddr)==0 )
			{
				OutputMsg(rmTip,"No need change,port=%d,add=%s",nPort,szAddr);
				return;
			}
			else
			{
				pClient->Inherited::Stop();

				if(nPort >0)
				{
					GetGlobalDBEngine()->SetDBCenterAddress(szAddr, nPort);
					pClient->Inherited::Startup();
				}
			
				return;
			}
			
		}
		else
		{
			GetGlobalDBEngine()->SetDBCenterAddress(szAddr, nPort);
			pClient->Inherited::Startup();
			return;
		}
	}
	*/
}

void CLogicDBReqestHandler::LoadFee(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	CDataPacket& out = allocProtoPacket(dcLoadFee);
	ACCOUNT sAccount;
	out << nRawServerId << nLoginServerId;
	out << nActorID;
	inPacket.readString(sAccount,sizeof(sAccount));

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query("call loadfee_actor(%d,%d,\"%s\");", nRawServerId, nActorID, sAccount);
		if ( !nError )
		{
			out << (BYTE)reSucc;
			if (MYSQL_ROW pRow = m_pSQLConnection->CurrentRow())
			{
				int nCount = m_pSQLConnection->GetRowCount();
				INT_PTR pos = out.getPosition();
				out << (int)0;
				nCount = 0;
				FeeData feedata;
				char token[64];
				do
				{
					nCount++;
					sscanf(pRow[0], "%u", &feedata.nId);
					sscanf(pRow[1], "%s", &feedata.sPf);
					sscanf(pRow[2], "%u", &feedata.nActorId);
					sscanf(pRow[4], "%s", &feedata.sProdId);
					sscanf(pRow[5], "%u", &feedata.nNum);
					strlcpy((feedata.sAccount), (pRow[3]? pRow[3] : ""), sizeof(feedata.sAccount));
					strlcpy((token), (pRow[6]? pRow[6] : ""), sizeof(token));
					OutputMsg(rmTip,_T("[Fee]1 请求充值数据： Id（%d），PfId(%s), ActorId（%d），Account（%s），ProdId（%s），Token（%s）"),
						feedata.nId, feedata.sPf, feedata.nActorId, feedata.sAccount, feedata.sProdId, token
					);
					out.writeBuf(&feedata, sizeof(FeeData));
				}
				while(pRow = m_pSQLConnection->NextRow());
				int* pCount = (int*)out.getPositionPtr(pos);
				*pCount = nCount;
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			out << (BYTE)reDbErr;
		}
	}
    flushProtoPacket(out);
}

void CLogicDBReqestHandler::DelFee(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	DECLARE_TIME_PROF("CLogicDBReqestHandler::DelFee");
	
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return;
	}
	else
	{
		int nCount = 0;
		inPacket >> nCount;
		if (nCount > 0)
		{
			char buff[16];
			strcpy(m_pHugeSQLBuffer,"update feecallback set flag = 999 where id in ");
			unsigned int nId = 0;
			strcat(m_pHugeSQLBuffer, " (");
			bool isFirst = true;
			for (size_t i = 0; i < nCount; i++)
			{
				inPacket >> nId;
				if(isFirst) { sprintf(buff, "%u", nId); isFirst = false;}
				else { sprintf(buff, ",%u", nId); }
				strcat(m_pHugeSQLBuffer, buff);
				OutputMsg(rmTip,_T("[Fee]3 完成充值： Id（%d），ActorId（%d）"),nId,nActorID);
			}
			strcat(m_pHugeSQLBuffer, " )");	
			int nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if(!nErrorCode)
			{
				m_pSQLConnection->ResetQuery(); 
			}
			else
			{
				OutputMsg(rmError,"feecallback Finish Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
			}
		}
	}
}
