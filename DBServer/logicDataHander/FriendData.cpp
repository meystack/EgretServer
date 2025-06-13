#include "StdAfx.h"

VOID CLogicDBReqestHandler::LoadActorFriendsData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadFriendsData);

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
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadFriendsData,nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << (BYTE)reSucc;
			retPack << (int)nRowCount;	

			unsigned int FriendId = 0;
			int nType = 0;
			int nSex = 0;
			int nLevel = 0;
			int nJop = 0;
			int nIcon = 0;
			int nintimacy = 0;
			int ncolor = 0;
			int ncircle = 0;
			long long nTeamId = 0;
			int nTime = 0;

			while ( pRow)
			{
				sscanf(pRow[0], "%u", &FriendId);
				retPack << FriendId;

				sscanf(pRow[1], "%d", &nType);
				retPack << nType;

				pRow[2]?retPack.writeString(pRow[2]):retPack.writeString("");

				sscanf(pRow[3], "%d", &nSex);
				retPack << nSex;

				sscanf(pRow[4], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[5], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[6], "%d", &nIcon);
				retPack << nIcon;

				pRow[7]?retPack.writeString(pRow[7]):retPack.writeString("");

				sscanf(pRow[8], "%d", &nintimacy);
				retPack << nintimacy;

				sscanf(pRow[9], "%lld", &nTeamId);
				retPack << nTeamId;

				sscanf(pRow[10], "%d", &nTime);
				retPack << nTime;
				sscanf(pRow[11], "%d", &ncolor);
				retPack << ncolor;
				sscanf(pRow[12], "%d", &ncircle);
				retPack << ncircle;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
}



VOID CLogicDBReqestHandler::LoadActorFriendsData(unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadFriendsData);

	retPack << nRawServerId;
	retPack << nLoginServerId;
	retPack << (unsigned int)nActorID;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadFriendsData,nActorID);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << (BYTE)reSucc;
			retPack << (int)nRowCount;	

			unsigned int FriendId = 0;
			int nType = 0;
			int nSex = 0;
			int nLevel = 0;
			int nJop = 0;
			int nIcon = 0;
			int nintimacy = 0;
			int ncolor = 0;
			int ncircle = 0;
			long long nTeamId = 0;
			int nTime = 0;
			unsigned int nVip = 0;

			while ( pRow)
			{
				sscanf(pRow[0], "%u", &FriendId);
				retPack << FriendId;

				sscanf(pRow[1], "%d", &nType);
				retPack << nType;

				pRow[2]?retPack.writeString(pRow[2]):retPack.writeString("");

				sscanf(pRow[3], "%d", &nSex);
				retPack << nSex;

				sscanf(pRow[4], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[5], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[6], "%d", &nIcon);
				retPack << nIcon;

				pRow[7]?retPack.writeString(pRow[7]):retPack.writeString("");

				sscanf(pRow[8], "%d", &nintimacy);
				retPack << nintimacy;

				sscanf(pRow[9], "%lld", &nTeamId);
				retPack << nTeamId;

				sscanf(pRow[10], "%d", &nTime);
				retPack << nTime;
				sscanf(pRow[11], "%d", &ncolor);
				retPack << ncolor;
				sscanf(pRow[12], "%d", &ncircle);
				retPack << ncircle;
				sscanf(pRow[13], "%u", &nVip);
				retPack << nVip;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
}


VOID CLogicDBReqestHandler::SaveActorFriendsData(CDataPacketReader &inPacket)
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
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_SaveFriendsData,nActorId);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}

		
		int nCount = 0;
		inPacket >> nCount;

		//应该是算有问题的了
		if(nCount >300 )
		{
			OutputMsg(rmError,"Actorid =%u, friend count=%d,save failed",nActorId,nCount);
			return ;
		}

		if(nCount>0)
		{
			char buff[1024];

			strcpy(m_pHugeSQLBuffer,szSQLSP_AddFriendsData); //表头放这里

			unsigned int nFriendId = 0;
			int nType = 0;
			int nintimacy = 0;
			long long nTeamId = 0;
			int nTime = 0;
			int ncolor = 0;
			for(int i=0;i<nCount;i++)
			{
				inPacket >> nFriendId;
				inPacket >> nType;
				inPacket >> nintimacy;
				inPacket >> nTeamId;
				inPacket >> nTime;
				inPacket >> ncolor;
				if(nFriendId !=0)
				{
					strcat(m_pHugeSQLBuffer," (");
					sprintf(buff,"%u,%u,%d,%d,%lld,%d, %d",nActorId,nFriendId,nType,nintimacy,nTeamId, nTime,ncolor);
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}
				
				
			}
			nErrorCode= m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if(!nErrorCode) m_pSQLConnection->ResetQuery(); 
		}
	}

}

VOID CLogicDBReqestHandler::DeleteActorFriendData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	unsigned int nActorId = 0;
	inPacket >> nActorId;

	unsigned int FriendId = 0;
	inPacket >> FriendId;

	// int nState = 0;
	// inPacket >> nState;

	// int nIntimacy = 0;
	// inPacket >> nIntimacy;

	// long long nTeamId = 0;
	// inPacket >> nTeamId;
	
	// int nType = 0;
	// inPacket >> nType;

	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		// nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteFriendsData,nState,nIntimacy,nTeamId,nActorId,FriendId);
		// if (!nErrorCode)
		// {
		// 	m_pSQLConnection->ResetQuery();
		// }
		//删除对方好友位
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteOtherFriendsData, nActorId, FriendId);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}


VOID CLogicDBReqestHandler::AddFriendChatMsg(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	unsigned int nActorId = 0;
	inPacket >> nActorId;

	unsigned int tagetId = 0;
	inPacket >> tagetId;

	int nTime = 0;
	inPacket >> nTime;

	char chatMsg[256];
	inPacket.readString(chatMsg,ArrayCount(chatMsg));
	chatMsg[sizeof(chatMsg) - 1] = 0;

	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetChatMsgCount,nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			int nCount = 0;

			if ( pRow)
			{
				sscanf(pRow[0], "%d", &nCount);
			}

			m_pSQLConnection->ResetQuery();

			if(nCount >= 20)
				return;
		}

		nErrorCode = m_pSQLConnection->Exec(szSQLSP_AddFriendsChatMsg,nActorId,tagetId,nTime,chatMsg);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

VOID CLogicDBReqestHandler::LoadFriendChatMsgData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	unsigned int nActorId = 0;
	inPacket >> nActorId;

	//预防数据错乱处理
	if(nActorId <= 0) return;	

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadFriendChatMsg);

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
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadFriendsChatMsgData,nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << (BYTE)reSucc;
			retPack << (int)nRowCount;	

			unsigned int tagetId = 0;
			int nTime = 0;
			int nSex = 0;
			int nLevel = 0;
			int nIcon = 0;

			while (pRow)
			{
				sscanf(pRow[0], "%u", &tagetId);
				retPack << tagetId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nTime);
				retPack << nTime;

				pRow[3]?retPack.writeString(pRow[3]):retPack.writeString("");

				sscanf(pRow[4], "%d", &nSex);
				retPack << nSex;

				sscanf(pRow[5], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[6], "%d", &nIcon);
				retPack << nIcon;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);

	nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteFriendsChatMsgData,nActorId);
	if (!nErrorCode)
	{
		m_pSQLConnection->ResetQuery();
	}
}

/*装载N天没上线的玩家数据*/
VOID CLogicDBReqestHandler::LoadFiendOfflineData(CDataPacketReader &inPacket)
{
	return;
	/*
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadFriendOffline);

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
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadFriendsData,nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << (BYTE)reSucc;
			retPack << (int)nRowCount;	

			unsigned int nFriendId = 0;
			char sFriendName[32]={0};
			unsigned int nLastLogouttime=0; 

			while ( pRow)
			{
				if( pRow[0] )
				{
					sscanf(pRow[0], "%d", &nFriendId);
					retPack << nFriendId;
				}
				else
				{
					retPack<< 0;
				}
				
				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				if( pRow[2] )
				{
					sscanf(pRow[2], "%d", &nLastLogouttime);
					retPack << nLastLogouttime;
				}
				else
				{
					retPack << 0;
				}

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
	*/
}
//世界等级查询
VOID CLogicDBReqestHandler::LoadTopLevelActorOneData(CDataPacketReader& inPacket)
{
	int nRawServerId, nLoginServerId, nType;
	inPacket >> nRawServerId >> nLoginServerId >> nType;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadTopLevelActorData);

	retPack << nRawServerId;
	retPack << nLoginServerId;
	retPack << nType;
	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;
		int nLevel = 0;
		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetTopLevelActor,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			BYTE nRowCount = m_pSQLConnection->GetRowCount();
			retPack << nRowCount;
			if (pRow)
			{
				if (pRow[0])
				{
					sscanf(pRow[0], "%d", &nLevel);
				}
				else
				{
					nLevel = 0;
				}
				
				retPack << nLevel;
			}

			m_pSQLConnection->ResetQuery();
		}
	}
	flushProtoPacket(retPack);
}
VOID CLogicDBReqestHandler::LoadTopLevelActorData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadTopLevelActor);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;

		unsigned int nActorId = 0;
		int nJop = 0;
		int nCircle = 0;
		int nLevel = 0;
		Uint64 nExp = 0;
		int nSex = 0;
		int nResult = 0;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetJopTopLevelActor,1,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			if (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nCircle);
				retPack << nCircle;

				sscanf(pRow[4], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[5], "%lld", &nExp);
				retPack << (Uint64)nExp;

				sscanf(pRow[6], "%d", &nSex);
				retPack << nSex;
			}

			m_pSQLConnection->ResetQuery();
		}

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetJopTopLevelActor,2,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			if (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nCircle);
				retPack << nCircle;

				sscanf(pRow[4], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[5], "%lld", &nExp);
				retPack << (Uint64)nExp;

				sscanf(pRow[6], "%d", &nSex);
				retPack << nSex;
			}

			m_pSQLConnection->ResetQuery();
		}

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetJopTopLevelActor,3,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			if (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nCircle);
				retPack << nCircle;

				sscanf(pRow[4], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[5], "%lld", &nExp);
				retPack << (Uint64)nExp;

				sscanf(pRow[6], "%d", &nSex);
				retPack << nSex;
			}

			m_pSQLConnection->ResetQuery();
		}


		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadCombineLevelActorData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadCombinLevelActor);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;

		unsigned int nActorId = 0;
		int nJop = 0;
		int nCircle = 0;
		int nLevel = 0;
		Uint64 nExp = 0;

		int nResult = 0;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetCombineTopLevelActor,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			while (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nCircle);
				retPack << nCircle;

				sscanf(pRow[4], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[5], "%lld", &nExp);
				retPack << nExp;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}

		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;

		size_t npos = retPack.getPosition();
		retPack << (int)0;

		int rCount = 0;
		for(int i=1;i<4;i++)
		{
			nErrorCode = m_pSQLConnection->Query(szSQLSP_GetJopTopLevelActor,i,nRawServerId);

			if (!nErrorCode)
			{
				MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

				if (pRow)
				{
					rCount++;

					sscanf(pRow[0], "%u", &nActorId);
					retPack << nActorId;

					pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

					sscanf(pRow[2], "%d", &nJop);
					retPack << nJop;

					sscanf(pRow[3], "%d", &nCircle);
					retPack << nCircle;

					sscanf(pRow[4], "%d", &nLevel);
					retPack << nLevel;

					sscanf(pRow[5], "%lld", &nExp);
					retPack << (Uint64)nExp;
				}

				m_pSQLConnection->ResetQuery();
			}
		}

		WORD* sbCount = (WORD*)retPack.getPositionPtr(npos);
		*sbCount = rCount;
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadCombineBattleActorData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadCombinBattleActor);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;

		unsigned int nActorId = 0;
		int nJop = 0;
		int nBattle = 0;
		Uint64 nExp = 0;

		int nResult = 0;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetCombineBattleRank,nRawServerId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			while (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nBattle);
				retPack << nBattle;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}

		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;

	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadActorConsumeRank(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadActorConsumeRank);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetActorConsumeRank,nRawServerId);

		int nResult = 0;

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			unsigned int nActorId = 0;
			int nConsumeYb = 0;

			while(pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nConsumeYb);
				retPack << nConsumeYb;

				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}

		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadActorTopicBattle(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadTopicBattleData);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;
		unsigned int nActorId = 0;
		int nJop = 0;
		int nBattle = 0;
		int nSex = 0;
		int nResult = 0;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetActorBattleRank,1,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			if (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nBattle);
				retPack << nBattle;
				sscanf(pRow[4], "%d", &nSex);
				retPack << nSex;
			}

			m_pSQLConnection->ResetQuery();
		}

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetActorBattleRank,2,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			if (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nBattle);
				retPack << nBattle;
				sscanf(pRow[4], "%d", &nSex);
				retPack << nSex;
			}

			m_pSQLConnection->ResetQuery();
		}

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetActorBattleRank,3,nRawServerId);

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			if (pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nJop);
				retPack << nJop;

				sscanf(pRow[3], "%d", &nBattle);
				retPack << nBattle;
				sscanf(pRow[4], "%d", &nSex);
				retPack << nSex;
			}

			m_pSQLConnection->ResetQuery();
		}


		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadActorSwingLevelData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadSwingLevelData);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetActorSwingLevelRank,nRawServerId);

		int nResult = 0;

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			unsigned int nActorId = 0;
			int nBattlePower = 0;
			int nSwingId = 0;
			int nSwingLevel = 0;
			int nJob = 0;
			int nSex = 0;
			while(pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nBattlePower);
				retPack << nBattlePower;

				sscanf(pRow[3], "%d", &nSwingId);
				retPack << nSwingId;

				sscanf(pRow[4], "%d", &nSwingLevel);
				retPack << nSwingLevel;
				sscanf(pRow[5], "%d", &nJob);
				retPack << nJob;
				sscanf(pRow[6], "%d", &nSex);
				retPack << nSex;

				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}

		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadActorHeroTopicRankData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadTopicHeroData);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetActorHeroTopicRank,nRawServerId);

		int nResult = 0;

		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			unsigned int nActorId = 0;
			int nLevel = 0;
			Uint64 nExp = 0;
			int nCircle = 0;

			while(pRow)
			{
				nResult++;

				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				pRow[1]?retPack.writeString(pRow[1]):retPack.writeString("");

				sscanf(pRow[2], "%d", &nCircle);
				retPack << nCircle;

				sscanf(pRow[3], "%d", &nLevel);
				retPack << nLevel;

				sscanf(pRow[4], "%lld", &nExp);
				retPack << nExp;

				pRow[5]?retPack.writeString(pRow[5]):retPack.writeString("");

				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}

		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadAllActivityData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	int nDays = 0;
	int nValue = 0;
	inPacket >> nDays;
	inPacket >> nValue;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadAllActivityData);

	retPack << nRawServerId;
	retPack << nLoginServerId;

	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		retPack << (BYTE)reSucc;
		retPack << (int)nDays;
		retPack << (int)nValue;

		size_t pos = retPack.getPosition();
		retPack << (int)0;

		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetAllWholeActivityData,nRawServerId,nDays,nValue);

		int nResult = 0;
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			unsigned int nActorId = 0;
			int nJob = 0;
			int nSex = 0;
			while(pRow)
			{
				nResult++;
				if (pRow[0])
					sscanf(pRow[0], "%u", &nActorId);
				if (pRow[1])
					sscanf(pRow[1], "%d", &nJob);
				if (pRow[2])
					sscanf(pRow[2], "%d", &nSex);
				retPack << nActorId;
				retPack << nJob;
				retPack << nSex;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}

		WORD* pCount = (WORD*)retPack.getPositionPtr(pos);
		*pCount = nResult;
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::AddFilterWordsDbMsg(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	int nValue = 0, nRecog = 0;
	inPacket >> nValue >> nRecog;
	char strFilter[128];
	inPacket.readString(strFilter,ArrayCount(strFilter));
	strFilter[sizeof(strFilter)-1] = 0;

	CDataPacket& retPack = allocProtoPacket((WORD)dcAddFilterWordsMsg);
	retPack << nRawServerId;
	retPack << nLoginServerId;
	retPack << nValue;
	retPack << nRecog;
	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)2;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_QueryFilterWordsData,strFilter);
		if(!nErrorCode)
		{
			int nRowCount = m_pSQLConnection->GetRowCount();
			m_pSQLConnection->ResetQuery();

			if (nValue == 1) //添加
			{
				if (nRowCount > 0)
				{
					retPack << (BYTE)1;
				}
				else	//没有就添加
				{
					nErrorCode = m_pSQLConnection->Exec(szSQLSP_AddFilterWordsData,strFilter);
					if (!nErrorCode)
					{
						GetGlobalDBEngine()->AddFilterName(strFilter);
						m_pSQLConnection->ResetQuery();
						retPack << (BYTE)reSucc;
					}
					else
					{
						retPack << (BYTE)2;
					}
				}
				
			}
			else			//删除
			{
				if (nRowCount > 0)
				{
					//retPack << (BYTE)1;
					nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelFilterWordsData,strFilter);
					if (!nErrorCode)
					{
						m_pSQLConnection->ResetQuery();
						retPack << (BYTE)reSucc;
					}
					else
					{
						retPack << (BYTE)2;
					}
				}
				else	//没有就添加
				{
					retPack << (BYTE)2;
				}

				
			}

			
			
		}
		else
		{
			retPack << (BYTE)2;
		}
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::BackRemoveItem(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;

	ACTORNAME sName;
	char strName[33];//后台操作用户名
	char srcStr[512];//参数字符串
	WORD nHost = 0;
	int nRecog = 0;
	CUserItem::ItemSeries nGuid;
	inPacket.readString(sName, ArrayCount(sName));
	inPacket.readString(srcStr, ArrayCount(srcStr));
	inPacket.readString(strName, ArrayCount(strName));
	inPacket >> nHost >> nGuid >> nRecog;

	CDataPacket& retPack = allocProtoPacket((WORD)dcBackRemoveItem);
	retPack << nRawServerId;
	retPack << nLoginServerId;
	retPack << (int)nHost << nRecog;
	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteGuidItem, nHost, nGuid);
		if(!nErrorCode)
		{
			if (m_pSQLConnection->GetRowsAffected() > 0)
			{
				nErrorCode = reSucc;
			}
			else
			{
				nErrorCode = reDbErr;
			}
			m_pSQLConnection->ResetQuery();
		}
	}
	retPack << (byte)nErrorCode;
	retPack.writeString(sName);
	retPack.writeString(srcStr);
	retPack.writeString(strName);
	retPack << nGuid;
	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::BackRemoveMoney(CDataPacketReader & inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	ACTORNAME sName;
	char strName[33];//后台操作用户名
	char srcStr[512];//参数字符串
	WORD nType = 0;
	int nRecog = 0, nVal = 0;
	inPacket.readString(sName, ArrayCount(sName));
	inPacket.readString(srcStr, ArrayCount(srcStr));
	inPacket.readString(strName, ArrayCount(strName));
	inPacket >> nType >> nVal >> nRecog;
	CDataPacket& retPack = allocProtoPacket((WORD)dcBackRemoveMoney);
	retPack << nRawServerId;
	retPack << nLoginServerId;
	retPack << (int)nVal << nRecog;
	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_DeleteActorMoney, sName, nType, nVal);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow)
			{
				sscanf(pRow[0], "%d", &nErrorCode);
			}
			m_pSQLConnection->ResetQuery();
		}
	}
	retPack << (byte)nErrorCode;
	retPack.writeString(sName);
	retPack.writeString(srcStr);
	retPack.writeString(strName);
	retPack << (int)nType;
	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::LoadActorDeathData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadDeathData);

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
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorDeathData,nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << (BYTE)reSucc;
			retPack << (int)nRowCount;	

			unsigned int nActorId  = 0;
			unsigned int nKillerId = 0;
			unsigned int nKillTime = 0;
			char sKillerName[32]={0};
			// int nSceneId = 0;
			// int nKillPosX = 0;
			// int nKillPosY = 0;

			while ( pRow)
			{
				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				sscanf(pRow[1], "%u", &nKillerId);
				retPack << nKillerId;

				sscanf(pRow[2], "%u", &nKillTime);
				retPack << nKillTime;

				pRow[3]?retPack.writeString(pRow[3]):retPack.writeString("");

				// sscanf(pRow[4], "%d", &nSceneId);
				// retPack << nSceneId;

				// sscanf(pRow[5], "%d", &nKillPosX);
				// retPack << nKillPosX;

				// sscanf(pRow[6], "%d", &nKillPosY);
				// retPack << nKillPosY;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
}



VOID CLogicDBReqestHandler::LoadActorDeathData(unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadDeathData);

	retPack << nRawServerId;
	retPack << nLoginServerId;
	retPack << (unsigned int)nActorID;

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
		retPack << (BYTE)nErrorCode;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorDeathData,nActorID);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << (BYTE)reSucc;
			retPack << (int)nRowCount;	

			unsigned int nActorId  = 0;
			unsigned int nKillerId = 0;
			unsigned int nKillTime = 0;
			char sKillerName[32]={0};
			int nSceneId = 0;
			int nType = 0;
			// int nKillPosX = 0;
			// int nKillPosY = 0;

			while ( pRow)
			{
				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				sscanf(pRow[1], "%u", &nKillerId);
				retPack << nKillerId;

				sscanf(pRow[2], "%u", &nKillTime);
				retPack << nKillTime;

				pRow[3]?retPack.writeString(pRow[3]):retPack.writeString("");

				sscanf(pRow[4], "%d", &nSceneId);
				retPack << nSceneId;

				sscanf(pRow[5], "%d", &nType);
				retPack << (BYTE)nType;

				// sscanf(pRow[6], "%d", &nKillPosY);
				// retPack << nKillPosY;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
}


VOID CLogicDBReqestHandler::SaveActorDeathData(CDataPacketReader &inPacket)
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
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelActorDeathData,nActorId);		//先删除旧数据
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}

		int nCount = 0;
		inPacket >> nCount;

		/*
		if( nCount > 4 )		//不超过4条记录
		{
			nCount = 4;
		}
		*/

		if( nCount > 0 )
		{
			char buff[1024];

			strcpy(m_pHugeSQLBuffer,szSQLSP_AddActorDeathData); //表头放这里

			unsigned int nKillerId = 0;
			char sKillerName[32] = {0};
			int nSceneId = 0;
			unsigned int nDeathTime = 0;
			int nKillPoxX = 0;
			int nKillPoxY = 0;
			BYTE nType = 0;

			for(int i=0;i<nCount;i++)
			{
				//OutputMsg(rmNormal,"SaveActorDeathData, i=%d", i);

				inPacket >> nKillerId;
				inPacket.readString(sKillerName, ArrayCount(sKillerName));
			
				inPacket >> nDeathTime;
				inPacket >> nSceneId;
				inPacket >>nType;
				// inPacket >> nKillPoxX;
				// inPacket >> nKillPoxY;

				//if(nFriendId !=0)
				{
					strcat(m_pHugeSQLBuffer," (");
					sprintf(buff,"%u,%u,%u,'%s',%d,%d,%d,%d", nActorId, nKillerId, nDeathTime, sKillerName, nSceneId,
						nKillPoxX, nKillPoxY,nType);
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}


			}

			nErrorCode= m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if(!nErrorCode) m_pSQLConnection->ResetQuery(); 
			//OutputMsg(rmNormal,"SaveActorDeathData, nCount=%d,m_pHugeSQLBuffer=%s", nCount,m_pHugeSQLBuffer);
		}
	}
}


VOID CLogicDBReqestHandler::LoadActorDeathDropData(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	inPacket >> nRawServerId >> nLoginServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;

	int nErrorCode = reDbErr;

	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadDeathDropData);

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
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorDeathDropData,nActorId);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();

			retPack << (BYTE)reSucc;
			retPack << (int)nRowCount;	

			unsigned int nActorId  = 0;
			unsigned int nKillerId = 0;
			unsigned int nDeathTime = 0;
			int nDropId = 0;
			int nDropCount = 0;

			while ( pRow)
			{
				sscanf(pRow[0], "%u", &nActorId);
				retPack << nActorId;

				sscanf(pRow[1], "%u", &nKillerId);
				retPack << nKillerId;

				sscanf(pRow[2], "%u", &nDeathTime);
				retPack << nDeathTime;

				sscanf(pRow[3], "%u", &nDropId);
				retPack << nDropId;

				sscanf(pRow[4], "%u", &nDropCount);
				retPack << nDropCount;

				pRow = m_pSQLConnection->NextRow();
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
}

VOID CLogicDBReqestHandler::SaveActorDeathDropData(CDataPacketReader &inPacket)
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
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelActorDeathDropData,nActorId);		//先删除旧数据
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}

		int nCount = 0;
		inPacket >> nCount;

		if( nCount > 0 )
		{
			char buff[4096];
			strcpy(m_pHugeSQLBuffer,szSQLSP_AddActorDeathDropData); //表头放这里
			unsigned int nKillerId = 0;
			unsigned int nDeathTime = 0;
			int nDropId = 0;
			int nDropCount = 0;
			for(int i=0;i<nCount;i++)
			{
				inPacket >> nKillerId;
				inPacket >> nDeathTime;
				inPacket >> nDropId;
				inPacket >> nDropCount;

				//if(nFriendId !=0)
				{
					strcat(m_pHugeSQLBuffer," (");
					sprintf(buff,"%u,%u,%u,%d,%d", nActorId, nKillerId, nDeathTime, nDropId, nDropCount);
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}
			}
			nErrorCode= m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if(!nErrorCode) m_pSQLConnection->ResetQuery(); 
			//OutputMsg(rmNormal,"SaveActorDeathDropData, m_pHugeSQLBuffer=%s", m_pHugeSQLBuffer);
		}
	}
}
