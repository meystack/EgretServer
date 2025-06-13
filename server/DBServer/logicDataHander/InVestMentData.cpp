#include "StdAfx.h"

VOID CLogicDBReqestHandler::LoadActorNewTitleData(CDataPacketReader &inPacket)
{
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;
	// QueryActorNewTitle(nActorId, nRawServerId, nServerId);
}

void CLogicDBReqestHandler::QueryActorNewTitle(unsigned int nActorId, int nRawServerId, int nServerId)
{
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorNewTitle, nActorId);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadNewTitleData);
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

VOID CLogicDBReqestHandler::SaveActorNewTitleData(CDataPacketReader &inPacket)
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
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelActorNewTitle, nActorId);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
		if(nErrorCode == reSucc && nDataCount > 0)
		{
			sprintf(m_pHugeSQLBuffer, szSQLSP_AddActorNewTitle);
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
					sprintf(m_pHugeSQLBuffer, szSQLSP_AddActorNewTitle);
				}
				else
				{
					strcat(m_pHugeSQLBuffer, ",");
				}
			}
		}

	}
}

VOID CLogicDBReqestHandler::LoadActorCustomTitleData(CDataPacketReader &inPacket)
{
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;
	// QueryActorNewTitle(nActorId, nRawServerId, nServerId);
}

void CLogicDBReqestHandler::QueryActorCustomTitle(unsigned int nActorId, int nRawServerId, int nServerId)
{
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorCustomTitle, nActorId);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadCustomTitleData);
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

VOID CLogicDBReqestHandler::SaveActorCustomTitleData(CDataPacketReader &inPacket)
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
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelActorCustomTitle, nActorId);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
		if(nErrorCode == reSucc && nDataCount > 0)
		{
			sprintf(m_pHugeSQLBuffer, szSQLSP_AddActorCustomTitle);
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
					sprintf(m_pHugeSQLBuffer, szSQLSP_AddActorCustomTitle);
				}
				else
				{
					strcat(m_pHugeSQLBuffer, ",");
				}
			}
		}

	}
}