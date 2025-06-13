#include "StdAfx.h"


void CLogicDBReqestHandler::QueryActorScriptData(CDataPacketReader &inPacket)
{
	int nRawServerId, nLoginServerId;
	unsigned int nActorID;
	inPacket >> nRawServerId >> nLoginServerId >> nActorID;
	QueryActorScriptData(nActorID, nRawServerId, nLoginServerId);
}

void CLogicDBReqestHandler::QueryActorScriptData(unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()	
	CDataPacket retPack;
	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID=  reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorScriptData, nActorID);
		if ( !nErroID )
		{
			CDataPacket& retPack = allocProtoPacket(dcLoadVar);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorID << (BYTE) nErroID;
		
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			 unsigned long* pLength =  m_pSQLConnection->GetFieldsLength(); //获取数据的长度

			if ( pRow && pLength && pRow[0])
			{
				unsigned long long lLong =( pLength[0]);
				retPack.writeBuf(LPCVOID ( pRow[0]), (size_t)lLong);
			}
			flushProtoPacket(retPack);
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			return;
		}

	}
	if( nErroID != reSucc)
	{
		CDataPacket& retPack = allocProtoPacket(dcLoadVar);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}


//保存技能的数据
void CLogicDBReqestHandler::SaveScriptDataToDB(CDataPacketReader &inPacket, bool bResp)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorId = 0;
	int nSkillCount =0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId ;
	int nErrorCode =0;
	long long nSize = inPacket.getLength() - inPacket.getPosition();
	if(nSize >0)
	{
		char *ptr = m_pHugeSQLBuffer;
		strcpy(m_pHugeSQLBuffer,szSQLSP_UpdateActorScriptData); //表头放这里
		ptr += strlen(szSQLSP_UpdateActorScriptData);
		ptr[0] = '"';
		ptr += 1;
		ptr += mysql_escape_string(ptr, inPacket.getOffsetPtr(), (unsigned long)nSize);
		ptr[0] = '"';
		ptr[1] = '\0';
		ptr += 2;

		char name[64];
		sprintf(name," where actorid=%u",nActorId);
		strcat(m_pHugeSQLBuffer,name);	
		
		nErrorCode= m_pSQLConnection->RealExec(m_pHugeSQLBuffer,strlen(m_pHugeSQLBuffer));
		if(!nErrorCode) m_pSQLConnection->ResetQuery(); 
	}
	
	if (bResp)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveVar);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorId << char(nErrorCode);
		flushProtoPacket(retPack);
	}
}
