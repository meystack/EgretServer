#include "StdAfx.h"

/************************************************************************/
/*  本文件实现了职业宗师数据的查询和保存                         */
/*	函数申明在LogicDBRequestHandler.h文件。								*/
/************************************************************************/

VOID CLogicDBReqestHandler::LoadActorCombatRank(CDataPacketReader &inPacket)
{
	
}

VOID CLogicDBReqestHandler::LoadActorCombatGameInfo(CDataPacketReader &inPacket)
{
	
}

VOID CLogicDBReqestHandler::SaveActorCombatGameInfo(CDataPacketReader &inPacket)
{
	
}

VOID CLogicDBReqestHandler::LoadCombatRecord(CDataPacketReader &inPacket)
{
	
}

VOID CLogicDBReqestHandler::SaveCombatRecordToDb(CDataPacketReader &inPacket)
{
	
}

VOID CLogicDBReqestHandler::UpdateActorCombatAward(CDataPacketReader &inPacket)
{
	
}




VOID CLogicDBReqestHandler::LoadActorCombatBaseInfo(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nErrorCode = reDbErr;
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadCmobatGameInfo);

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorCombatBaseInfo, nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow)
			{
				retPack << nRawServerId;
				retPack << nLoginServerId;
				retPack << (unsigned int)nActorId;
				retPack << (BYTE)reSucc;

				int currank = 0;
				int maxrank = 0;
				int lastrank = 0;
				int combatscore = 0;
				int challegenum = 0;
				int picknum = 0;
				int cleartime = 0;
				int cdflag = 0;

				sscanf(pRow[0], "%d", &currank);
				sscanf(pRow[1], "%d", &maxrank);
				sscanf(pRow[2], "%d", &lastrank);
				sscanf(pRow[3], "%d", &combatscore);
				sscanf(pRow[4], "%d", &challegenum);
				sscanf(pRow[5], "%d", &picknum);
				sscanf(pRow[6], "%d", &cleartime);
				sscanf(pRow[7], "%d", &cdflag);

				retPack << currank;
				retPack << maxrank;
				retPack << lastrank;
				retPack << combatscore;
				retPack << challegenum;
				retPack << picknum;
				retPack << cleartime;
				retPack << cdflag;

				m_pSQLConnection->ResetQuery();
			}
			else
			{
				m_pSQLConnection->ResetQuery();

				nErrorCode = m_pSQLConnection->Exec(szSQLSP_AddActorCombatBaseInfo, nActorId);
				if (!nErrorCode)
				{
					m_pSQLConnection->ResetQuery();
				}

				retPack << (BYTE)reDbErr;
			}
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::SaveActorCombatBaseInfo(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	unsigned int nActorId = 0;
	inPacket >> nActorId;

	//预防数据错乱处理
	if(nActorId <= 0) return;	


	int currank = 0;
	int maxrank = 0;
	int lastrank = 0;
	int combatscore = 0;
	int challegenum = 0;
	int picknum = 0;
	int cleartime = 0;
	int cdflag = 0;

	inPacket >> currank;
	inPacket >> maxrank;
	inPacket >> lastrank;
	inPacket >> combatscore;
	inPacket >> challegenum;
	inPacket >> picknum;
	inPacket >> cleartime;
	inPacket >> cdflag;


	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_SaveActorCombatBaseInfo, currank, maxrank, lastrank, combatscore, challegenum, picknum, cleartime, cdflag, nActorId);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}


VOID CLogicDBReqestHandler::LoadActorCombatLog(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nErrorCode = reDbErr;
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadCombatRecord);

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorCombatLogInfo, nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << nRawServerId << nLoginServerId;
			retPack << (unsigned int)nActorId;
			retPack << (BYTE)reSucc;
			retPack << nRowCount;

			int type = 0;
			int result = 0;
			int targetid = 0;
			int targetname = 0;
			int rank = 0;

			while (pRow)
			{
				sscanf(pRow[0], "%d", &type);
				retPack << type;

				sscanf(pRow[1], "%d", &result);
				retPack << result;

				sscanf(pRow[2], "%d", &targetid);
				retPack << targetid;

				pRow[3]?retPack.writeString(pRow[3]):retPack.writeString("");

				sscanf(pRow[4], "%d", &rank);
				retPack << rank;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::SaveActorCombatLog(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nCount = 0;
	inPacket >> nCount;

	if(nCount <= 0)
		return;

	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteActorCombatLogInfo, nActorId);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}

		char buff[1024];
		strcpy(m_pHugeSQLBuffer, szSQLSP_SaveActorCombatLogInfo); //表头放这里

		int type = 0;
		int result = 0;
		int targetid = 0;
		char targetname[32];
		int rank = 0;

		for(int i = 0; i < nCount; ++i)
		{
			inPacket >> type;
			inPacket >> result;
			inPacket >> targetid;
			inPacket.readString(targetname, ArrayCount(targetname));
			inPacket >> rank;

			strcat(m_pHugeSQLBuffer, " (");
			sprintf(buff, "%d,%d,%d,%d,\"%s\",%d", nActorId, type, result, targetid, targetname, rank);
			strcat(m_pHugeSQLBuffer, buff);

			strcat(m_pHugeSQLBuffer, " )");	
			if(i < nCount -1)
			{
				strcat(m_pHugeSQLBuffer,",");	
			}
		}

		nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery(); 
		}
	}
}


