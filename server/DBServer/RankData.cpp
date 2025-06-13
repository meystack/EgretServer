#include "StdAfx.h"

#define LIMIT_TOPRANK_COUNT 100


void CLogicDBReqestHandler::SaveBaseRank(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerIdx = 0, nLoginServerId = 0;
	inPacket >> nRawServerIdx >> nLoginServerId;

	CDataPacket &retPack = allocProtoPacket((WORD)dcSaveBaseRank);
	retPack << nRawServerIdx << nLoginServerId;
	if (!m_pSQLConnection->Connected())
	{
		retPack << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQL_SaveTopRank, nRawServerIdx);
		if (!nError)
		{
			retPack << (BYTE)reSucc;
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}
	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::SaveBaseRankCS(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerIdx = 0, nLoginServerId = 0;
	inPacket >> nRawServerIdx >> nLoginServerId;
	
	unsigned int now_time;
	inPacket >> now_time;

	CDataPacket &retPack = allocProtoPacket((WORD)dcSaveBaseRankCS);
	retPack << nRawServerIdx << nLoginServerId;
	if (!m_pSQLConnection->Connected())
	{
		retPack << (BYTE)reDbErr;
	}
	else
	{ 
		
		SYSTEMTIME sysTime; 
		CMiniDateTime nStartTime(now_time);
		nStartTime.decode(sysTime);

    	// CMiniDateTime now_time = CMiniDateTime::now();
    	// int nNowTime = time(NULL);
     
		// SYSTEMTIME sysTime;
		// now_time.decode(sysTime);
		//GetLocalTime( &sysTime );
		static char buf[20]={0};
		memset(&buf, 0, sizeof(buf)); 
		sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",sysTime.wYear,sysTime.wMonth,sysTime.wDay,sysTime.wHour,sysTime.wMinute,sysTime.wSecond);
  	 
		int nError = m_pSQLConnection->Exec(szSQL_SaveTopRankCS, nRawServerIdx, buf);
		if (!nError)
		{
			retPack << (BYTE)reSucc;
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}
	flushProtoPacket(retPack);
 
}
void CLogicDBReqestHandler::LoadBaseRank(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadBaseRank);
	retPack << nRawServerId << nLoginServerId;
	if (!m_pSQLConnection->Connected())
	{
		retPack << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQL_LoadTopRank, LIMIT_TOPRANK_COUNT, nRawServerId);
		if (!nError)
		{
			retPack << (BYTE)nError;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			retPack << (int)nCount;
			int nValue = 0;
			while (pRow)
			{
				sscanf(pRow[0], "%d", &nValue);
				retPack << (unsigned int)nValue;
				sscanf(pRow[1], "%d", &nValue);
				retPack << (int)nValue;
				sscanf(pRow[2], "%d", &nValue);
				retPack << (int)nValue;
				sscanf(pRow[3], "%d", &nValue);
				retPack << (int)nValue;
				sscanf(pRow[4], "%d", &nValue);
				retPack << (int)nValue;
				ACTORNAME sName = {0};
				if (pRow[5])
				{
					sscanf(pRow[5], "%s", &sName);	
				}
				retPack.writeString(sName);
				sscanf(pRow[6], "%d", &nValue);
				retPack << (int)nValue;
				sscanf(pRow[7], "%d", &nValue);
				retPack << (int)nValue;
				sscanf(pRow[8], "%d", &nValue);//yy
				retPack << (int)nValue;
				
				ACTORNAME sguildName = {0};
				if (pRow[9])
				{
					sscanf(pRow[9], "%s", &sguildName);	
				}
				retPack.writeString(sguildName);
				sscanf(pRow[10], "%d", &nValue);//特权
				retPack << (int)nValue;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		}
		else
		{
			OutputMsg(rmError,_T("%s装载排行榜数据失败"),__FUNCTION__);
			retPack << (BYTE)nError;
		}
	}
	flushProtoPacket(retPack);
}




