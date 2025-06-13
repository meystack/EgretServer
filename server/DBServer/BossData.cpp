#include "StdAfx.h"
#include "../LogicDef.h"
#include "BossData.h"
#include <vector>

void CLogicDBReqestHandler::LoadBossList(CDataPacketReader &inPacket)
{
    DECLARE_TIME_PROF("CLogicDBReqestHandler::LoadBossList");
	const int max_guild = 100;
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	CDataPacket& tmp = allocProtoPacket(dcLoadBossInfo);
	tmp << nRawServerId << nLoginServerId;
	CDataPacket* out = &tmp;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		(*out) << (BYTE)reDbErr;
	}
	else
	{
        //向数据库查询活动数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadBossData);
		if ( !nError )
		{
            (*out) << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
            INT_PTR pos = out->getPosition();
			(*out) << (int)0;
			nCount = 0;
            GLOBALBOSSINFO bossData;
            while (pRow)
			{
                nCount++;
                sscanf(pRow[0], "%d", &bossData.nBossId);
                sscanf(pRow[1], "%d", &bossData.nReferTime);
                sscanf(pRow[2], "%d", &bossData.nDeath);

                sscanf(pRow[3], "%d", &bossData.nBossLv);
                out->writeBuf(&bossData,sizeof(bossData));
				pRow = m_pSQLConnection->NextRow();
            }
            int* pCount = (int*)out->getPositionPtr(pos);
			*pCount = nCount;
			m_pSQLConnection->ResetQuery();
        }
		else
		{
			(*out) << (BYTE)reDbErr;
		}
    }
    flushProtoPacket((*out));
}

void CLogicDBReqestHandler::SaveBossList(CDataPacketReader &inPacket)
{
    DECLARE_TIME_PROF("CLogicDBReqestHandler::SaveBossList");
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return;
	}
	else
	{
		int nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteBossData);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}

		char buff[1024];
		strcpy(m_pHugeSQLBuffer, szSQLSP_SaveBossData); //表头放这里

		int nCount = 0;
		inPacket >> nCount;
        GLOBALBOSSINFO boss;
		for(int i = 0; i < nCount; ++i)
		{
			inPacket.readBuf(&boss,sizeof(boss));
			
			strcat(m_pHugeSQLBuffer, " (");
			sprintf(buff, "%u,%u,%u,%u,%u", boss.nBossId, boss.nReferTime, boss.nDeath, boss.nBossLv);
			strcat(m_pHugeSQLBuffer, buff);
			strcat(m_pHugeSQLBuffer, " )");	

			if(i < nCount-1)
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

void CLogicDBReqestHandler::LoadActorBossList(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
}

void CLogicDBReqestHandler::SaveActorBossList(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
    DECLARE_TIME_PROF("CLogicDBReqestHandler::SaveActorBossList");
}
