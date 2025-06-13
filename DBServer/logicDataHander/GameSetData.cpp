#include "StdAfx.h"

#define MAXGAMESETDATANUM  16

VOID CLogicDBReqestHandler::LoadActorGameSetData(CDataPacketReader &inPacket)
{
   	DECLARE_TIME_PROF("CLogicDBReqestHandler::LoadActorGameSetData");

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadGameSetData);

	retPack << nRawServerId;
	retPack << nLoginServerId;
	retPack << (unsigned int)nActorId;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorGameSetData,nActorId);
		bool bFlag = true;
		if (!nErrorCode)
		{
			retPack << (char)reSucc;
			int nValue[MAXGAMESETDATANUM];
			memset(nValue,0,sizeof(nValue));
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow)
			{
				retPack << (int)MAXGAMESETDATANUM;
				for(int i=0;i<MAXGAMESETDATANUM;i++)
				{
					sscanf(pRow[i], "%d", &(nValue[i]));
					retPack << (int)(nValue[i]);
				}
				bFlag = false;
			}
			else
			{
				retPack << (int)0;
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}

		if(bFlag)
		{
			nErrorCode = m_pSQLConnection->Exec(szSQLSP_AddActorGameSetData,nActorId,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
			if (!nErrorCode)
			{
				m_pSQLConnection->ResetQuery();
			}
		}
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::SaveActorGameSetData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	unsigned int nActorId = 0;
	inPacket >> nActorId;

	//预防数据错乱处理
	if(nActorId <= 0) return;	

	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		int nValue[MAXGAMESETDATANUM];
		for(int i=0;i<MAXGAMESETDATANUM;i++)
		{
			inPacket >> nValue[i];
		}

		nErrorCode = m_pSQLConnection->Exec(szSQLSP_UpdateGameSetData,nActorId,nValue[0],nValue[1],nValue[2],nValue[3],nValue[4],nValue[5],nValue[6],nValue[7],nValue[8],nValue[9],nValue[10],nValue[11],nValue[12],nValue[13],nValue[14],nValue[15]);
		
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();


			//nErrorCode = m_pSQLConnection->Exec(szSQLSP_AddActorGameSetData,nActorId,nValue[0],nValue[1],nValue[2],nValue[3],nValue[4],nValue[5],nValue[6],nValue[7],nValue[8],nValue[9],nValue[10],nValue[11],nValue[12],nValue[13],nValue[14],nValue[15]);
			//if (!nErrorCode)
			//{
			//	m_pSQLConnection->ResetQuery();
			//}
		}
		else
		{
			//m_pSQLConnection->ResetQuery();
		}
	}
}
