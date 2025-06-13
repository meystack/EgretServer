#include "StdAfx.h"

int CLogicDBReqestHandler::SaveGoingQuest(unsigned int nActorid, int nTaskId, int id, int value)
{
	DECLARE_FUN_TIME_PROF()
	int ret = reSucc;
	if ( !m_pSQLConnection->Connected() )
	{
		ret = reDbErr;
	}
	else
	{
		//insert
		int nError = m_pSQLConnection->Exec(szSQLSP_AddCharNewRoleItem, nActorid,nTaskId,id,value);
		if ( !nError )
		{
			ret = reSucc;
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			ret = reDbErr;
		}
	}
	if (ret != reSucc)
	{
		OutputMsg(rmError,_T("save quest data error!id=%u,taskid=%d,value=%d"),
			nActorid,nTaskId,value);
	}
	return ret;
}

int CLogicDBReqestHandler::CleanQuestData(unsigned int nActorId)
{
	DECLARE_FUN_TIME_PROF()
	int ret = reSucc;
	if ( !m_pSQLConnection->Connected() )
	{
		ret = reDbErr;
	}
	else
	{
		//先删除
		int nError = m_pSQLConnection->Exec(szSQLSP_CleanCharRoleItems, nActorId);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();		
		}else
		{
			ret = reDbErr;
		}
		// nError = m_pSQLConnection->Exec(szSQLSP_CleanCharFinishRoles, nActorId);
		// if ( !nError )
		// {
		// 	m_pSQLConnection->ResetQuery();
		// }else
		// {
		// 	ret = reDbErr;
		// }
	}
	if (ret != reSucc)
	{
		OutputMsg(rmError,_T("CleanQuestData error!id=%u"),
			nActorId);
	}
	return ret;
}

int CLogicDBReqestHandler::SaveRepeatQuest(unsigned int nActorid,int nTaskId,int qtime)
{
	// DECLARE_FUN_TIME_PROF()
	// int ret = reSucc;
	// if ( !m_pSQLConnection->Connected() )
	// {
	// 	ret = reDbErr;
	// }
	// else
	// {
	// 	//先删除
	// 	int nError = m_pSQLConnection->Exec(szSQLSP_AddCharNewFinishRole, nActorid,nTaskId,qtime);
	// 	if ( !nError )
	// 	{
	// 		ret = reSucc;
	// 		m_pSQLConnection->ResetQuery();
	// 	}
	// 	else
	// 	{
	// 		ret = reDbErr;
	// 	}		
	// }
	// if (ret != reSucc)
	// {
	// 	OutputMsg(rmError,_T("save quest data error!id=%u,taskid=%d,qtime=%d"),
	// 		nActorid,nTaskId,qtime);
	// }
	// return ret;
}

void CLogicDBReqestHandler::QueryQuestFromDB(unsigned int nActorId, int nRawServerId, int nLoginServerId)
{	
	DECLARE_FUN_TIME_PROF()
	CDataPacket& retPack = allocProtoPacket(dcLoadQuest);
	retPack << nRawServerId << nLoginServerId << nActorId;
	int pos = (int)retPack.getPosition();
	retPack << (char)reSucc ;
	BYTE ret =(char) GetGoingQuest(nActorId,retPack);
	if (ret != reSucc)
	{
		BYTE* bRet = (BYTE*)(retPack.getMemoryPtr() + pos);
		*bRet = ret;
		flushProtoPacket(retPack);
		return;
	}
	ret = (char)GetRepeatQuest(nActorId,retPack);
	if (ret != reSucc)
	{
		BYTE* bRet = (BYTE*)(retPack.getMemoryPtr() + pos);
		*bRet = ret;
		flushProtoPacket(retPack);
		return;
	}	

	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::QueryQuestFromDB(CDataPacketReader &inPacket)
{
	int nRawServerId = 0, nLoginServerId;
	unsigned int nID = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nID;
	QueryQuestFromDB(nID, nRawServerId, nLoginServerId);
}

int CLogicDBReqestHandler::GetGoingQuest(unsigned int nActorid,CDataPacket& out)
{
	int result = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		result = reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadCharRoleItems, nActorid);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			WORD nCount = m_pSQLConnection->GetRowCount();
			out << nCount;
			while ( pRow)
			{
				int value;
				sscanf(pRow[0], "%d", &value);
				out << value;
				sscanf(pRow[1], "%d", &value);
				out << value;
				sscanf(pRow[2], "%d", &value);
				out << value;
				sscanf(pRow[3], "%d", &value);
				out << value;
				sscanf(pRow[4], "%d", &value);
				out << value;
				sscanf(pRow[5], "%d", &value);
				out << value;
				sscanf(pRow[6], "%d", &value);
				out << value;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			result = reDbErr;
		}

	}
	return result;
}


int CLogicDBReqestHandler::GetRepeatQuest(unsigned int nActorid,CDataPacket& out)
{
	DECLARE_FUN_TIME_PROF()
	int result = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		result = reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadCharFinishRoles, nActorid);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			WORD nCount = m_pSQLConnection->GetRowCount();
			out << nCount;
			while ( pRow)
			{
				int value;
				sscanf(pRow[0], "%d", &value);
				out << value;
				sscanf(pRow[1], "%d", &value);
				out << value;

				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			result = reDbErr;
		}

	}
	return result;
}

void CLogicDBReqestHandler::SaveQuestToDB(CDataPacketReader& packet, bool bResp)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId >> nActorID;
	
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		 nErrorCode = m_pSQLConnection->Exec(szSQLSP_CleanCharRoleItems, nActorID);
		 if(!nErrorCode)
		 {
			 m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		 }
		 if(nErrorCode == reSucc)
		 {
			 int nCount = 0;
			
			 packet >> nCount; //取物品的数量
			 //insert into xxx (actorID,...) Values 
			 if(nCount >0 ) //如果有物品的话
			 {
				 sprintf(m_pHugeSQLBuffer,szSQLSP_AddCharNewRoleItem); //表头放这里

				 char buff[1024];
				 int nType =0;
				 int nValue=0;
				 for(INT_PTR i =0;i < nCount ; i++)
				 {
					 strcat(m_pHugeSQLBuffer," (");
					//  packet >> nType>>nValue; //装载入物品的数据
					 QuestItem item;
					packet.readBuf(&item, sizeof(item));
					 sprintf(buff,"%u,%d,%d,%d,%d,%d,%d,%d",nActorID, item.nQid,item.nType, item.nValue, item.nState ,item.nComplete ,item.nUseFsTimes,item.nStartId);
					 strcat(m_pHugeSQLBuffer,buff);
					 strcat(m_pHugeSQLBuffer," )");
					 if(i < nCount -1)
					 {
						 strcat(m_pHugeSQLBuffer,",");	
					 }
				 }
				 //std::cout <<m_pHugeSQLBuffer <<std::endl;
				 nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				 if(!nErrorCode)  
					 m_pSQLConnection->ResetQuery(); 
			 }
		 }
	}
	
	// if (nErrorCode)
	// {
	// 	CDataPacket& retPack = allocProtoPacket(dcSaveQuest);
	// 	retPack << nRawServerId << nLoginServerId;
	// 	retPack << nActorID << (BYTE) nErrorCode;
	// 	flushProtoPacket(retPack);
	// }
}



