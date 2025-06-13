/*
*       复活特权
*/

#include "StdAfx.h"

//复活特权
VOID CLogicDBReqestHandler::LoadReviveDurationSystemInfo(CDataPacketReader &inPacket)
{
    int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;
    int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadReviveDurationData, nActorId);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadReviveDurationData);
			OutPacket << nRawServerId << nServerId << nActorId << (byte)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			while(pRow)
			{
				OutPacket << (unsigned int)atoi(pRow[0]);
				OutPacket << (unsigned int)atoi(pRow[1]);		
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
}

VOID CLogicDBReqestHandler::SaveReviveDurationSystemInfo(CDataPacketReader &inPacket)
{
    unsigned int nServerId = 0, nRawServerId = 0, nActorId = 0, nDataCount = 0;
	inPacket >> nRawServerId >> nServerId >> nActorId >> nDataCount;
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelReviveDurationData, nActorId);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
		if(nErrorCode == reSucc && nDataCount > 0)
		{
			sprintf(m_pHugeSQLBuffer, szSQLSP_AddReviveDurationData);
			unsigned int nIdMakeLong = 0, nRemainTime = 0;
			unsigned int nCurCount = 0;
			char sBuff[1024] = {0};
			while (nCurCount++ < nDataCount)
			{
				strcat(m_pHugeSQLBuffer, "(");
				inPacket >> nIdMakeLong >> nRemainTime; 
				sprintf(sBuff,"%u,%u,%u)", nActorId,nIdMakeLong,nRemainTime);
				strcat(m_pHugeSQLBuffer, sBuff);
				if (  nCurCount == nDataCount)
				{
					nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
					if (!nErrorCode)
					{
						m_pSQLConnection->ResetQuery();
					}
					sprintf(m_pHugeSQLBuffer, szSQLSP_AddReviveDurationData);
				}
				else
				{
					strcat(m_pHugeSQLBuffer, ",");
				}
			}
		}
	}
}
