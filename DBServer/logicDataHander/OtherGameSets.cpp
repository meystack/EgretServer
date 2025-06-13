#include "StdAfx.h"
#include<string>

void CLogicDBReqestHandler::QueryOtherGameSetsFromDB(unsigned int nActorID, int nRawServerId, int nLoginServerId)
{


	DECLARE_FUN_TIME_PROF()	
	CDataPacket retPack;
	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID =  reDbErr;
	}
	else
	{
		//（1）查询 按键数据
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadOtherGameSets, nActorID);//增加app的键盘，字段有500够用的
		if ( !nErroID )
		{
			CDataPacket& retPack = allocProtoPacket(dcLoadOtherGameSets);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorID << (BYTE) nErroID;
			GAMESETSDATA data;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			int nValue =0;
			WORD wValue =0;
			memset(&data.m_vKbs,0, sizeof(data.m_vKbs));
			while ( pRow)
			{
				if(pRow[0])
				{
					std::string str = "";
					str = pRow[0];
					if(str != "")
					{	
						//memcpy(&data.m_vKbs, str.c_str(), sizeof(str));
						strcpy(data.m_vKbs, str.c_str());
					}
				}
				//sscanf(pRow[0], "%s", &data.m_vKbs);

				pRow = m_pSQLConnection->NextRow();
			}
			retPack.writeBuf(&data, sizeof(GAMESETSDATA));

			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			
			//（2）向数据库查询 游戏设置数据
			
			nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorGameSetData2, nActorID);
			if ( !nErroID )
			{
				MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
				unsigned long* pLength =  m_pSQLConnection->GetFieldsLength(); //获取数据的长度

				if ( pRow && pLength && pRow[0])
				{
					unsigned long long lLong =( pLength[0]);
					retPack.writeBuf(LPCVOID ( pRow[0]), (size_t)lLong);
				}
				m_pSQLConnection->ResetQuery();
			}

			flushProtoPacket(retPack);
			return;
		}

	}
	if( nErroID != reSucc)
	{
		CDataPacket& retPack = allocProtoPacket(dcLoadOtherGameSets);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::SaveOtherGameSetsToDB(CDataPacketReader &inPacket, bool bResp)
{	
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId;

	INT_PTR nErroID = reSucc;
	if (nActorId < 0 )
	{
		nErroID = reNoActor;
	}
	else
	{

		//先删除
		//nErrorCode =ExecuteSimplyProcedure(szSQLSP_CleanCharSkills,nActorId) ;
		int nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteOtherGameSets, nActorId);
		
		if(!nErrorCode) 
		{
			m_pSQLConnection->ResetQuery(); 
			
		}
		else
		{
			OutputMsg(rmError,_T("%s error"),__FUNCTION__);
			return;
		}
		
		if( nErroID == reSucc)
		{
			if(!m_pSQLConnection->Connect())
			{
				nErroID = reDbErr;
			}
			else
			{
				//（1）保存 按键数据
						
				GAMESETSDATA data;
				inPacket.readBuf(&data, sizeof(GAMESETSDATA));
				//inPacket >> data; //每次读一个的数据
				char buff[200];
				strcpy(m_pHugeSQLBuffer,szSQLSP_AddOtherGameSets); //表头放这里
				strcat(m_pHugeSQLBuffer," (");	
				sprintf(buff,"%u,'%s'",nActorId,data.m_vKbs); //actotID
				strcat(m_pHugeSQLBuffer,buff);

				strcat(m_pHugeSQLBuffer," )");
				//OutputMsg(rmTip, _T("sql:%s, kb:%s"), m_pHugeSQLBuffer, data.m_vKbs);
				nErroID = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if (!nErroID)
				{
					m_pSQLConnection->ResetQuery();
				}

				//（2）保存 游戏设置数据
				int nSize = inPacket.getAvaliableLength();
				if (inPacket.getAvaliableLength() == sizeof(GAMESETSDATA2))
				{
					char *ptr = m_pHugeSQLBuffer;
					strcpy(m_pHugeSQLBuffer,szSQLSP_UpdateGameSetData2); ptr += strlen(szSQLSP_UpdateGameSetData2);
					ptr[0] = '"'; ptr += 1;
					ptr += mysql_escape_string(ptr, inPacket.getOffsetPtr(), (unsigned long)sizeof(GAMESETSDATA2));
					ptr[0] = '"'; ptr[1] = '\0'; ptr += 2;

					char name[64];
					sprintf(name," where actorid=%u",nActorId);
					strcat(m_pHugeSQLBuffer,name);	
					
					nErrorCode= m_pSQLConnection->RealExec(m_pHugeSQLBuffer,strlen(m_pHugeSQLBuffer));
					if(!nErrorCode) m_pSQLConnection->ResetQuery();
				}
				else
				{
					OutputMsg(rmError, _T("[Save GameSet] data len error len=%d ,require len=%d"), inPacket.getAvaliableLength(), sizeof(GAMESETSDATA2));
				}
			}
		}
		else
		{
			OutputMsg(rmWaning,_T("SaveOtherGameSetsToDBerror,actorID=%u"),nActorId);
		}
	}

	if( nErroID != reSucc)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveOtherGameSets);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorId << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}
