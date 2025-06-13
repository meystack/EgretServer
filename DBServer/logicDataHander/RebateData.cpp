#include "StdAfx.h"

void CLogicDBReqestHandler::QueryRebateData(CDataPacketReader& packet)
{
    DECLARE_FUN_TIME_PROF()
	
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorId = 0;

	packet >> nRawServerId >> nLoginServerId >> nActorId;

	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadRebateData, nActorId);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadRebateData);
			OutPacket << nRawServerId << nLoginServerId << nActorId << (byte)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();

			int nActorId = 0, nId = 0, nTimeType = 0, nStartTime = 0, nEndTime = 0, nClearTime = 0, nOfflineType = 0, nAtonce = 0, nOverlay = 0, nYBValue = 0, nStatus = 0;

			while(pRow)
			{
				int nActorId = 0, nId = 0, nTimeType = 0, nStartTime = 0, nEndTime = 0, nClearTime = 0, nOfflineType = 0, nAtonce = 0, nOverlay = 0, nYBValue = 0, nStatus = 0;						
				sscanf(pRow[0], "%d", &nActorId);
				sscanf(pRow[1], "%d", &nId);
				sscanf(pRow[2], "%d", &nTimeType);
				sscanf(pRow[3], "%d", &nStartTime);
				sscanf(pRow[4], "%d", &nEndTime);
				sscanf(pRow[5], "%d", &nClearTime);
				sscanf(pRow[6], "%d", &nOfflineType);
				sscanf(pRow[7], "%d", &nAtonce);
				sscanf(pRow[8], "%d", &nOverlay);
				sscanf(pRow[9], "%d", &nYBValue);
				sscanf(pRow[10], "%d", &nStatus);

				OutPacket << nActorId << nId << nTimeType << nStartTime << nEndTime << nClearTime << nOfflineType << nAtonce << nOverlay << nYBValue << nStatus;
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
	}
	if( nErrorCode != reSucc)
	{
		OutputMsg(rmError,_T("%s装载玩家的 返利数据 失败%u"),__FUNCTION__,nActorId);
		CDataPacket& retPack = allocProtoPacket(dcLoadRebateData);
		retPack << nRawServerId << nLoginServerId << nActorId << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::QueryNextDayRebateData(CDataPacketReader& packet)
{
    DECLARE_FUN_TIME_PROF()
	
	int nRawServerId = 0, nLoginServerId = 0, nAtonce = 0;
	unsigned int nActorId = 0;

	packet >> nRawServerId >> nLoginServerId >> nActorId >> nAtonce;

	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadNextDayRebateData, nActorId, nAtonce);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadNextDayRebateData);
			OutPacket << nRawServerId << nLoginServerId << nActorId << (byte)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();

			int nActorId = 0, nId = 0, nTimeType = 0, nStartTime = 0, nEndTime = 0, nClearTime = 0, nOfflineType = 0, nAtonce = 0, nOverlay = 0, nYBValue = 0, nStatus = 0;

			while(pRow)
			{
				int nActorId = 0, nId = 0, nTimeType = 0, nStartTime = 0, nEndTime = 0, nClearTime = 0, nOfflineType = 0, nAtonce = 0, nOverlay = 0, nYBValue = 0, nStatus = 0;						
				sscanf(pRow[0], "%d", &nActorId);
				sscanf(pRow[1], "%d", &nId);
				sscanf(pRow[2], "%d", &nTimeType);
				sscanf(pRow[3], "%d", &nStartTime);
				sscanf(pRow[4], "%d", &nEndTime);
				sscanf(pRow[5], "%d", &nClearTime);
				sscanf(pRow[6], "%d", &nOfflineType);
				sscanf(pRow[7], "%d", &nAtonce);
				sscanf(pRow[8], "%d", &nOverlay);
				sscanf(pRow[9], "%d", &nYBValue);
				sscanf(pRow[10], "%d", &nStatus);

				OutPacket << nActorId << nId << nTimeType << nStartTime << nEndTime << nClearTime << nOfflineType << nAtonce << nOverlay << nYBValue << nStatus;
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
	}
	if( nErrorCode != reSucc)
	{
		OutputMsg(rmError,_T("%s装载玩家的 返利数据 失败%u"),__FUNCTION__,nActorId);
		CDataPacket& retPack = allocProtoPacket(dcLoadNextDayRebateData);
		retPack << nRawServerId << nLoginServerId << nActorId << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::UpdateRebateData(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	
	int nRawServerId = 0, nLoginServerId = 0, nId = 0, nTimeType = 0, nStartTime = 0, nEndTime = 0, nClearTime = 0, nOfflineType = 0, nAtonce = 0, nOverlay = 0, nYBValue = 0, nStatus = 0;
	unsigned int nActorId = 0, nCount = 0;

	packet >> nRawServerId >> nLoginServerId >> nActorId >> nCount;

	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		for (int i = 0; i < nCount; i++)
		{
			packet >> nId >> nTimeType >> nStartTime >> nEndTime >> nClearTime >> nOfflineType >> nAtonce >> nOverlay >> nYBValue >> nStatus;
			nErrorCode = m_pSQLConnection->Exec(szSQLSP_UpdateRebateData, nActorId, nId, nTimeType, nStartTime, nEndTime, nClearTime, nOfflineType, nAtonce, nOverlay, nYBValue, nStatus);
			if (!nErrorCode)
			{
				m_pSQLConnection->ResetQuery();
			}
		}
	}
	if( nErrorCode != reSucc)
	{
		OutputMsg(rmError,_T("%s更新玩家的 返利数据 失败%u"),__FUNCTION__,nActorId);
		CDataPacket& retPack = allocProtoPacket(dcUpdateRebateData);
		retPack << nRawServerId << nLoginServerId << nActorId << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::SaveRebateToDB(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	
	int nRawServerId = 0, nLoginServerId = 0, nId = 0, nTimeType = 0, nStartTime = 0, nEndTime = 0, nClearTime = 0, nOfflineType = 0, nAtonce = 0, nOverlay = 0, nYBValue = 0, nStatus = 0;
	unsigned int nActorId = 0, nCount = 0;

	packet >> nRawServerId >> nLoginServerId >> nActorId >> nCount; 

	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		for (int i = 0; i < nCount; i++)
		{
			packet >> nId >> nTimeType >> nStartTime >> nEndTime >> nClearTime >> nOfflineType >> nAtonce >> nOverlay >> nYBValue >> nStatus;
			nErrorCode = m_pSQLConnection->Exec(szSQLSP_SaveRebateData, nActorId, nId, nTimeType, nStartTime, nEndTime, nClearTime, nOfflineType, nAtonce, nOverlay, nYBValue, nStatus);
			if (!nErrorCode)
			{
				m_pSQLConnection->ResetQuery();
			}
		}
	}
	if( nErrorCode != reSucc)
	{
		OutputMsg(rmError,_T("%s保存玩家的 返利数据 失败%u"),__FUNCTION__,nActorId);
		CDataPacket& retPack = allocProtoPacket(dcSaveRebateData);
		retPack << nRawServerId << nLoginServerId << nActorId << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::ClearRebateData(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	
	int nRawServerId = 0, nLoginServerId = 0, nId = 0, nTimeType = 0, nStartTime = 0, nEndTime = 0, nClearTime = 0, nOfflineType = 0, nAtonce = 0, nOverlay = 0, nYBValue = 0, nStatus = 0;
	unsigned int nActorId = 0, nCount = 0;

	packet >> nRawServerId >> nLoginServerId >> nActorId >> nCount;

	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		for (int i = 0; i < nCount; i++)
		{
			packet >> nId >> nTimeType >> nStartTime >> nEndTime >> nClearTime >> nOfflineType >> nAtonce >> nOverlay >> nYBValue >> nStatus;
			nErrorCode = m_pSQLConnection->Exec(szSQLSP_ClearRebateData, nActorId, nId, nTimeType, nStartTime, nEndTime, nClearTime, nOfflineType, nAtonce, nOverlay, nYBValue, nStatus);
			if (!nErrorCode)
			{
				m_pSQLConnection->ResetQuery();
			}
		}
	}
	if( nErrorCode != reSucc)
	{
		OutputMsg(rmError,_T("%s清理玩家的 返利数据 失败%u"),__FUNCTION__,nActorId);
		CDataPacket& retPack = allocProtoPacket(dcClearRebateData);
		retPack << nRawServerId << nLoginServerId << nActorId << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}
