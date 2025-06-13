#include "StdAfx.h"

void CLogicDBReqestHandler::QuerySkillImpl(MYSQL_ROW pRow, ONESKILLDBDATA& data)
{
	int nValue =0;
	WORD wValue =0;
	
	//技能的ID,等级，秘籍放一个地方存储
	sscanf(pRow[0], "%d", &nValue);
	
	data.nSkillID = LOWORD(nValue);
	wValue =  HIWORD(nValue);

	data.nLevel = LOBYTE(wValue);
	data.nIsClosed = HIBYTE(wValue);
	
	sscanf(pRow[1], "%d", &data.nCd);

	sscanf(pRow[2], "%d", &data.nExp);

	if(pRow[3])
	{
		sscanf(pRow[3], "%d", &data.nMijiExpiredTime);
	}
	if(pRow[4])
	{
		sscanf(pRow[4], "%d", &data.nMijiId);
	}
}

void CLogicDBReqestHandler::QuerySkillFromDB(unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR nErroID = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErroID = reDbErr;
	}
	else
	{
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadCharSkills, nActorID);
		if ( !nErroID )
		{
			CDataPacket& retPack = allocProtoPacket(dcLoadSkill);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorID << (BYTE) nErroID;
			ONESKILLDBDATA data;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			retPack << (int)nCount;
			while ( pRow)
			{
				QuerySkillImpl(pRow, data);
				retPack << data;
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(retPack);
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			return;
		}
		else
		{
			
			OutputMsg(rmError,_T("%s error"),__FUNCTION__);
			m_pSQLConnection->ResetQuery(); 
			
		}

	}
	if( nErroID != reSucc)
	{
		OutputMsg(rmError,_T("%s装载玩家的技能数据失败%u"),__FUNCTION__,nActorID);
		CDataPacket& retPack = allocProtoPacket(dcLoadSkill);
		retPack << nRawServerId << nLoginServerId << nActorID << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::SaveSkillToDB(CDataPacketReader &inPacket, bool bResp)
{	
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int nActorId = 0;
	int nSkillCount =0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId >> nSkillCount;

	int nErrorCode =0;
	if (nActorId < 0 )
	{
		nErrorCode = reNoActor;
	}
	else
	{
		//先删除玩家的技能列表
		//nErrorCode =ExecuteSimplyProcedure(szSQLSP_CleanCharSkills,nActorId) ;
		int nErrorCode = m_pSQLConnection->Exec(szSQLSP_CleanCharSkills, nActorId);
		
		if(!nErrorCode) 
		{
			m_pSQLConnection->ResetQuery(); 
			
		}
		else
		{
			OutputMsg(rmError,_T("%s error"),__FUNCTION__);
			return;
		}
		
		
		if( nErrorCode == reSucc)
		{
			if(nSkillCount >0)
			{
				strcpy(m_pHugeSQLBuffer,szSQLSP_AddCharAllSkills); //表头放这里
				char buff[2048];
				ONESKILLDBDATA data;
				int nValue;
				for(INT_PTR i=0; i< nSkillCount; i++)
				{
					inPacket >> data; //每次读一个的数据

					strcat(m_pHugeSQLBuffer," (");	

					nValue =MAKELONG(data.nSkillID,MAKEWORD(data.nLevel,data.nIsClosed));
					sprintf(buff,"%u,%d,%d,%d,%d,%d",nActorId,nValue,data.nCd,data.nExp,data.nMijiExpiredTime,data.nMijiId); //actotID,guid
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if(i < nSkillCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}

				}
				nErrorCode= m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if(!nErrorCode) 
				{
					m_pSQLConnection->ResetQuery(); 
				}
				else
				{
					OutputMsg(rmError,_T("%s error"),__FUNCTION__);
				}
			}
		}
		else
		{
			OutputMsg(rmWaning,_T("clean skill error,actorID=%u"),nActorId);
		}
	}

	if (bResp)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveSkill);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorId << char(nErrorCode);
		flushProtoPacket(retPack);
	}
}
