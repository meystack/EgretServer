#include "StdAfx.h"

void CLogicDBReqestHandler::QueryRelationData( unsigned int nActorId, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR nErrorId = reSucc;
	if(!m_pSQLConnection->Connect())
		nErrorId = reDbErr;
	else
	{
		nErrorId = m_pSQLConnection->Query(szSQL_LoadRelation, nActorId);
		if (!nErrorId)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			CDataPacket& retPack = allocProtoPacket(dcLoadRelation);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorId << (char)reSucc << nCount;
			if(nCount > 0)
			{
				unsigned int nRelType, nRelationId, nBonus;
				ACTORNAME aName;
				while(pRow)
				{
					sscanf(pRow[0], "%d", &nRelType);
					sscanf(pRow[1], "%d", &nRelationId);
					sscanf(pRow[2], "%s", &aName);
					sscanf(pRow[3], "%d", &nBonus);

					retPack << (BYTE)nRelType << nRelationId << aName << nBonus;
					pRow = m_pSQLConnection->NextRow();
				}
			}
			m_pSQLConnection->ResetQuery();
			flushProtoPacket(retPack);
		}
	}
}

void CLogicDBReqestHandler::SaveRelation( CDataPacketReader &inPacket )
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorId = 0;
	BYTE  nRowCount = 0, bRelType;
	int nErrorId = reSucc;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId >> nRowCount;
	if(!m_pSQLConnection->Connect())
		nErrorId = reDbErr;
	else
	{
		nErrorId = m_pSQLConnection->Exec(szSQL_CleanRelation, nActorId);		//先清空
		if(!nErrorId)
			m_pSQLConnection->ResetQuery();
		if (nRowCount > 0)
		{
			strcpy(m_pHugeSQLBuffer, szSQL_AddRelation);
		}
		unsigned int nPlayerId, nBonus;
		for (BYTE i = 0; i < nRowCount; i++)
		{
			inPacket >> bRelType >> nPlayerId >> nBonus;
			char temp[1024];
			sprintf(temp, "(%u,%d,%d,%d)", nActorId, bRelType, nPlayerId, nBonus);
			strcat(m_pHugeSQLBuffer, temp);
			if(i < (nRowCount - 1))
			{
				strcat(m_pHugeSQLBuffer, ",");
			}
		}
		nErrorId = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
		if(!nErrorId)
			m_pSQLConnection->ResetQuery();

	}
	if(nErrorId != reSucc)
	{
		OutputMsg(rmError, _T("save relation data error:id=%u"), nActorId);
	}
}
