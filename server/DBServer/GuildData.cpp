#include "StdAfx.h"
#include "../LogicDef.h"
#include "GuildDb.h"
#include<string>
#include<iostream>

void CLogicDBReqestHandler::LoadGuildList(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	const int max_guild = 100;
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	CDataPacket& tmp = allocProtoPacket(dcLoadGuildList);
	tmp << nRawServerId << nLoginServerId;
	CDataPacket* out = &tmp;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		(*out) << (BYTE)reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildBasicData, nRawServerId);
		if ( !nError )
		{
			(*out) << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			INT_PTR pos = out->getPosition();
			(*out) << (int)0;
			nCount = 0;
			while ( pRow)
			{
				nCount++;
				if (nCount > max_guild)
				{
					int* pCount = (int*)out->getPositionPtr(pos);
					*pCount = nCount-1;
					flushProtoPacket((*out));

					CDataPacket& guildNew = allocProtoPacket((WORD)dcLoadGuildList);
					guildNew << nRawServerId << nLoginServerId;
					out = &guildNew;
					(*out) << (BYTE)reSucc;
					pos = out->getPosition();
					(*out) <<(int)0;

					nCount = 1;
				}
				unsigned int nGuildId = 0;
				sscanf(pRow[0], "%u", &nGuildId);
				(*out) << nGuildId;
				int value =0;
				unsigned int uValue =0;
				sscanf(pRow[1], "%d", &value);
				(*out) << (WORD)value;
				pRow[2]?out->writeString(pRow[2]):out->writeString(""); //guildname
				pRow[3]?out->writeString(pRow[3]):out->writeString(""); //leadername
				pRow[4]?out->writeString(pRow[4]):out->writeString(""); //inmemo
				pRow[5]?out->writeString(pRow[5]):out->writeString(""); //outmemo
				sscanf(pRow[6], "%d", &value); //guildcoin
				(*out) << value;
				// pRow[7]?out->writeString(pRow[7]):out->writeString("");
				// sscanf(pRow[8], "%d", &value);
				// (*out) << value;
				// pRow[9]?out->writeString(pRow[9]):out->writeString("");
				// pRow[10]?out->writeString(pRow[10]):out->writeString("");
				// pRow[11]?out->writeString(pRow[11]):out->writeString("");
				sscanf(pRow[7], "%d", &value); //nbidcoin
				(*out) << value;
				sscanf(pRow[8], "%d", &value); //createtime
				(*out) << value;
				sscanf(pRow[9], "%d", &value); //nmask
				(*out) << value;
				// pRow[15]?out->writeString(pRow[10]):out->writeString("");
				sscanf(pRow[10], "%d", &value); //halllevel
				(*out) << value;
				sscanf(pRow[11], "%d", &value);//exerciseroomlv
				(*out) << value;
				sscanf(pRow[12], "%d", &value);//assemblyhalllv
				(*out) << value;
				sscanf(pRow[13], "%d", &value);//autojoinlevel
				(*out) << value;
				sscanf(pRow[14], "%u", &uValue);//actorId
				(*out) << uValue;
				// sscanf(pRow[15], "%d", &value);//autojoinlevel
				// (*out) << value;
				// sscanf(pRow[16], "%d", &value);		//nupnexttime
				// (*out) << value;
				// sscanf(pRow[17], "%d", &value);		//reserver
				// (*out) << value;
				// sscanf(pRow[18], "%d", &uValue);	//autojoinlevel
				// (*out) << uValue;
				// sscanf(pRow[25], "%d", &uValue);	//upgradeitemnum1
				// (*out) << uValue;
				// sscanf(pRow[26], "%d", &uValue);	//upgradeitemnum2
				// (*out) << uValue;
				// sscanf(pRow[27], "%d", &uValue);	//upgradeitemnum3
				// (*out) << uValue;
				// sscanf(pRow[28], "%d", &uValue);	//m_nExchangeCheckCircleMin，  替换treelevel
				// (*out) << uValue;
				// sscanf(pRow[29], "%d", &uValue);	//treedegree
				// (*out) << uValue;
				// sscanf(pRow[30], "%d", &uValue);	//fruitlevel
				// (*out) << uValue;
				// sscanf(pRow[31], "%d", &uValue);	//fruitdegree
				// (*out) << uValue;
				// sscanf(pRow[32], "%d", &uValue);	//fruitnum
				// (*out) << uValue;
				// sscanf(pRow[33], "%u", &uValue);	//fruittime
				// (*out) << uValue;
				// sscanf(pRow[34], "%d", &uValue);	//taskid
				// (*out) << uValue;
				// sscanf(pRow[35], "%d", &uValue);	//tasksche
				// (*out) << uValue;
				// sscanf(pRow[36], "%u", &uValue);	//dailycoindonated
				// (*out) << uValue;
				sscanf(pRow[15], "%d", &value);	//certification
				(*out) << value;

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

VOID CLogicDBReqestHandler::LoadGuildEventDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId >> Guildid;
	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID = reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildEvent,Guildid);
		if ( !nError )
		{
			CDataPacket& out = allocProtoPacket(dcLoadGuildEvent);
			out << nRawServerId << nLoginServerId;
			out << (BYTE)reSucc;
			out << Guildid;

			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << nCount;

			while ( pRow)
			{
				GUILDEVENTRECOUD mGuildEvent;

				sscanf(pRow[0], "%d", &mGuildEvent.aTime);
				//sscanf(pRow[1], "%s", mGuildEvent.nEventMsg);

				out<<mGuildEvent.aTime;

				sscanf(pRow[1], "%d", &mGuildEvent.mEventId);
				out<<mGuildEvent.mEventId;

				sscanf(pRow[2], "%d", &mGuildEvent.mParam1);
				out<<mGuildEvent.mParam1;

				sscanf(pRow[3], "%d", &mGuildEvent.mParam2);
				out<<mGuildEvent.mParam2;

				sscanf(pRow[4], "%d", &mGuildEvent.mParam3);
				out<<mGuildEvent.mParam3;

				pRow[5]?out.writeString(pRow[5]):out.writeString("");
				pRow[6]?out.writeString(pRow[6]):out.writeString("");

				pRow = m_pSQLConnection->NextRow();
			}

			flushProtoPacket(out);
			m_pSQLConnection->ResetQuery();

			//printf("结果：%d\n",nCount);
		}
		else
		{
			nErroID = reDbErr;
		}
	}

	if( nErroID != reSucc)
	{
		CDataPacket& retPack = allocProtoPacket(dcLoadGuildEvent);
		retPack << nRawServerId << nLoginServerId;
		retPack << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::LoadGuildSkillDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF();
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId >> Guildid;

	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID = reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildSKILL,Guildid);
		//printf("帮派技能:%d",nError);
		if ( !nError )
		{
			CDataPacket& out = allocProtoPacket(dcLoadGuildSkill);
			out << nRawServerId << nLoginServerId;
			out << (BYTE)reSucc;
			out << Guildid;

			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << nCount;

			while ( pRow)
			{
				GUILDSKILLS mGuildSkill;

				sscanf(pRow[0], "%d", &mGuildSkill.nSkillId);
				sscanf(pRow[1], "%d", &mGuildSkill.nLevel);

				out<<mGuildSkill;

				pRow = m_pSQLConnection->NextRow();
			}

			flushProtoPacket(out);
			m_pSQLConnection->ResetQuery();

			//printf("结果：%d\n",nCount);
		}
		else
		{
			nErroID = reDbErr;
		}
	}

	if( nErroID != reSucc)
	{
		CDataPacket& retPack = allocProtoPacket(dcLoadGuildSkill);
		retPack << nRawServerId << nLoginServerId;
		retPack << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::LoadGuildDetail(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId >> Guildid;
	CDataPacket& out = allocProtoPacket(dcLoadGuildDetail);
	out << nRawServerId << nLoginServerId;
	int pos = (int)out.getPosition();
	out << (BYTE)reSucc;
	out << Guildid;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
		*bRet = reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildMembers,Guildid);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << nCount;
			while ( pRow)
			{
				ActorCommonNode Node;
				memset(Node.sActorName, 0 ,sizeof(Node.sActorName));
				sscanf(pRow[0], "%u", &Node.BasicData.nActorId);
				if (pRow[1])
				{
					_asncpytA(Node.sActorName,pRow[1]);
				}

				unsigned int temp = 0;
				if (pRow[2])
				{
					sscanf(pRow[2], "%d", &temp);
					Node.BasicData.nGuildGX = temp;	
				}
				else
				{
					Node.BasicData.nGuildGX = 0;
				}
				if (pRow[3])
				{
					sscanf(pRow[3], "%d", &temp);//typetk
					Node.BasicData.nTK = (BYTE)temp;
					Node.BasicData.nType = BYTE(temp >> 16);
				}
				else
				{
					Node.BasicData.nTK = (BYTE)0;
					Node.BasicData.nType = (BYTE)0;
						
				}
				if (pRow[4])
				{
					sscanf(pRow[4], "%d", &temp);//sex
					Node.BasicData.nSex = (BYTE)temp;
				}
				else
				{					
					Node.BasicData.nSex = (BYTE)0;
				}

				if(pRow[5])
				{
					sscanf(pRow[5], "%d", &temp);//nlevel
					Node.BasicData.nLevel = (BYTE)temp;
				}
				else
				{
					Node.BasicData.nLevel = (BYTE)0;
				}
				if (pRow[6])
				{
					sscanf(pRow[6], "%d", &temp);//job
					Node.BasicData.nJob = (BYTE)temp;
				}
				else
				{
					Node.BasicData.nJob = (BYTE)0;
				}
				if (pRow[7])
				{
					sscanf(pRow[7], "%u", &(Node.BasicData.nZzNext));//
				}
				if(pRow[8])
				{
					sscanf(pRow[8], "%u", &(Node.BasicData.nZjNext));//
				}
				if (pRow[9])
				{
					sscanf(pRow[9], "%d", &(Node.BasicData.nGuildTiTle));//
				}
				if (pRow[10])
				{
					sscanf(pRow[10], "%d", &(Node.BasicData.nTime));
				}
				if (pRow[11])
				{
					sscanf(pRow[11], "%d", &(Node.BasicData.nTotalGx));
				}
				if (pRow[12])
				{
					sscanf(pRow[12], "%d", &(Node.BasicData.nIcon));
				}
				if (pRow[13])
				{
					sscanf(pRow[13], "%d", &(Node.BasicData.nBattleValue));
				}
				if(pRow[14])
				{
					sscanf(pRow[14], "%d", &(Node.BasicData.nGuildCityPos));
				}
				else
				{
					Node.BasicData.nGuildCityPos =0;
				}
				if(pRow[15])
				{
					sscanf(pRow[15], "%d", &Node.BasicData.nModelId);
				}
				else
				{
					Node.BasicData.nModelId =0;
				}
				if(pRow[16])
				{
					sscanf(pRow[16], "%d", &Node.BasicData.nSwingId);
				}
				else
				{
					Node.BasicData.nSwingId =0;
				}

				if(pRow[17])
				{
					sscanf(pRow[17], "%u", &temp);
				}
				else
				{
					temp =0;
				}
				
				Node.BasicData.nVipGrade = (unsigned int)temp;

				if( pRow[18] )			//加入行会的时间
				{
					sscanf( pRow[18], "%u", &Node.BasicData.nJoinTime );
				}
				else
				{
					Node.BasicData.nJoinTime =0;
				}

				if(pRow[19])
				{
					sscanf(pRow[19], "%d", &temp);
				}
				else
				{
					temp =0;
				}
				Node.BasicData.nCircle =(BYTE)(temp);

				out << Node;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();

			nError = m_pSQLConnection->Query(szSQLSP_LoadGuildWar,Guildid);
			if ( !nError )
			{
				MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
				int nCount = m_pSQLConnection->GetRowCount();
				out << nCount;
				while ( pRow)
				{
					WarRelationData War;
					int nWarGuildId = 0;
					sscanf(pRow[0], "%d", &War.m_WarData.nRelationShip);//type
					out << War.m_WarData.nRelationShip;
					sscanf(pRow[1], "%d", &War.m_WarData.nState);
					out << War.m_WarData.nState;
					sscanf(pRow[2], "%u", &War.nGuildId);//id
					out << War.nGuildId;
					sscanf(pRow[3], "%d", &War.m_WarData.nPkCount);//pkcount
					out << War.m_WarData.nPkCount;
					sscanf(pRow[4], "%d", &War.m_WarData.nDieCount);//dieCount
					out << War.m_WarData.nDieCount;
					sscanf(pRow[5], "%d", &War.m_WarData.nNext);//nNext
					out << War.m_WarData.nNext;
					pRow = m_pSQLConnection->NextRow();
				}
				m_pSQLConnection->ResetQuery();
			}
			else
			{
				BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
				*bRet = reDbErr;
			}


			// int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildApplyLists,Guildid);
			// if ( !nError )
			// {
			// 	MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			// 	int nCount = m_pSQLConnection->GetRowCount();
			// 	out << nCount;
			// 	while ( pRow)
			// 	{
			// 		unsigned int actorid = 0;
			// 		sscanf(pRow[0], "%u", &actorid);
			// 		out << actorid;
			// 		ACTORNAME name;
			// 		if (pRow[1])
			// 		{
			// 			_asncpytA(name,pRow[1]);
			// 		}else
			// 		{
			// 			name[0]=0;
			// 		}
			// 		out.writeString(name);
			// 		int nvalue = 0; 
			// 		if (pRow[2])
			// 		{
			// 			sscanf(pRow[2], "%d", &nvalue); //sex
			// 		}
			// 		out << nvalue;

			// 		if (pRow[3])
			// 		{
			// 			sscanf(pRow[3], "%d", &nvalue);//level
			// 		}
			// 		out << nvalue;
					
			// 		if (pRow[4])
			// 		{
			// 			sscanf(pRow[4], "%d", &nvalue);//circle
			// 		}
			// 		out << nvalue;

			// 		if(pRow[5])
			// 		{
			// 			sscanf(pRow[5], "%d", &nvalue);//job
			// 		}
			// 		out << nvalue;
			// 		pRow = m_pSQLConnection->NextRow();
			// 	}
			// 	m_pSQLConnection->ResetQuery();
			// }
			// else
			// {
			// 	BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
			// 	*bRet = nError;
			// }
		}
		else
		{
			BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
			*bRet = reDbErr;
		}

	}
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::AddGuild(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nIndex = 0;
	Uint64 hActor = 0;
	char sGuildName[33];
	char sFoundname[33];
	BYTE nZTType = 0;
	int nActor = 0;
	WORD nIcon = 0;
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActor;
	inPacket >> nIndex;
	inPacket >> hActor;
	inPacket.readString(sGuildName,ArrayCount(sGuildName));
	sGuildName[sizeof(sGuildName)-1] = 0;
	inPacket.readString(sFoundname,ArrayCount(sFoundname));
	sFoundname[sizeof(sFoundname)-1] = 0;

	int ret = m_pHost->CatchCreateGuild(nActor,sGuildName,sFoundname,nIndex,hActor,nZTType,nIcon);
	
	if (ret != reSucc)
	{
		//出现错误，不用名字服务器返回
		SendAddGuildResult(nRawServerId, nLoginServerId, ret,nIndex,hActor,0,sGuildName,sFoundname,nZTType,nIcon);
		return;
	}
	//等待名称服务器返回
	
}

void CLogicDBReqestHandler::SendAddGuildResult(int nRawServerId, int nLoginServerId,
	int nErr,int index,Uint64 hHandle,unsigned int nGid,LPCSTR sGuildName,LPCSTR sFoundName,BYTE nZY,INT_PTR nIcon)
{
	CDataPacket& out = allocProtoPacket(dcAddGuild);
	out << nRawServerId << nLoginServerId;
	out << (BYTE)nErr;//内部错误
	out << (int)index;
	out << (Uint64)hHandle;
	if (nErr == jxSrvDef::aeSuccess)
	{
		out << (unsigned int)nGid;
		out.writeString(sGuildName);
		out.writeString(sFoundName);
	}
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::DeleteMember(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorid = 0;
	int nOperActor = 0;//这个是操作的帮主的id
	//这个是脱离帮派的角色id
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorid;
	inPacket >> nOperActor;

	CDataPacket& out = allocProtoPacket(dcDeleteMember);
	out << nRawServerId << nLoginServerId;
	out << nOperActor;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_DelGuildMember,nActorid);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();	
			out << (BYTE)reSucc;
		}
		else
		{
			out << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(out);
}

void CLogicDBReqestHandler::SaveActorGuild(CDataPacketReader &inPacket, bool bResp)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int Guildid = 0;
	unsigned int nActorid = 0;
	int nGX;
	int tktype = 0;
	UINT	nZjNext;
	UINT	nZzNext;
	int nScore = 0;
	int nTime = 0;
	int nPosType = 0;
	int nModelId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> Guildid;
	inPacket >> nActorid;
	inPacket >> nGX;
	inPacket >> tktype;
	inPacket >> nZjNext;
	inPacket >> nZzNext;
	inPacket >> nScore;
	inPacket >> nTime;
	inPacket >> nPosType;
	inPacket >> nModelId;

	int nTodayGx = 0;
	inPacket >> nTodayGx;

	int nHasShxTimes = 0;
	inPacket >> nHasShxTimes;

	int nAddShxTimes = 0;
	inPacket >> nAddShxTimes;

	int nExploreTimes = 0;
	inPacket >> nExploreTimes;

	int nExploreItemId = 0;
	inPacket >> nExploreItemId;

	int nChallengeTimes = 0;
	inPacket >> nChallengeTimes;

	int nAddChallengeTimes = 0;
	inPacket >> nAddChallengeTimes;

	unsigned int nJoinTime = 0;
	inPacket >> nJoinTime;

	/*CDataPacket& out = allocProtoPacket(dcSaveActorGuild);
	out << nActorid;*/
	// CDataPacket* pPacket = &allocProtoPacket(dcSaveActorGuild);
	// if (pPacket)
	// {
	// 	*pPacket << nRawServerId << nLoginServerId;
	// 	*pPacket << nActorid;
	// }

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		/*out << (BYTE)reDbErr;*/
		// if (pPacket)
		// 	*pPacket << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_UpdateGuildMember,Guildid,nActorid,tktype,nZjNext,nZzNext,nScore,nGX,nPosType,nTodayGx,nHasShxTimes,
			nAddShxTimes,nExploreTimes,nExploreItemId,nChallengeTimes,nAddChallengeTimes,nModelId,nJoinTime);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();		
			/*out << (BYTE)reSucc;*/
			// if (pPacket)
			// 	*pPacket << (BYTE)reSucc;
		}
		else
		{
			/*out << (BYTE)reDbErr;*/
			// if (pPacket)
			// 	*pPacket << (BYTE)reDbErr;
		}
	}

	// if (pPacket)
	// 	flushProtoPacket(*pPacket);
}

void CLogicDBReqestHandler::LoadActorGuild(unsigned int nActorid)
{
	DECLARE_FUN_TIME_PROF()
	CDataPacket& out = allocProtoPacket(dcLoadActorGuild);
	out << nActorid;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildMemberBasic,nActorid);
		if ( !nError )
		{
			out << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow)
			{
				unsigned int nGuildId = 0;
				sscanf(pRow[0], "%u", &nGuildId);//gid
				out << nGuildId;
				INT64 nGx = 0;
				sscanf(pRow[1], "%lld", &nGx);//gx
				out << nGx;
				int value;
				sscanf(pRow[2], "%d", &value);	//typeTK
				out << value;
				UINT nzzNext = 0;
				sscanf(pRow[3], "%u", &nzzNext);//zjnext
				out << nzzNext;
				sscanf(pRow[4], "%u", &nzzNext);//zznext
				out << nzzNext;
			}else
			{
				//如果从来没加入任何帮派（数据库没记录）,gid传个-1
				out << (unsigned int)0;
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			out << (BYTE)reDbErr;
		}

	}
	flushProtoPacket(out);
}

/*
CGuild::AddMemberToList()发送dcAddMember消息
*/
void CLogicDBReqestHandler::AddGuildMember(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	unsigned int nActorid = 0;
	int nType = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> Guildid;
	inPacket >> nActorid;
	inPacket >> nType;

	int nGx = 0;
	inPacket >> nGx;

	int nHasShxTimes = 0;
	inPacket >> nHasShxTimes;

	int nAddShxTimes = 0;
	inPacket >> nAddShxTimes;

	int nExploreTimes = 0;
	inPacket >> nExploreTimes;

	int nExploreItemId = 0;
	inPacket >> nExploreItemId;

	int nChallengeTimes = 0;
	inPacket >> nChallengeTimes;

	int AddChallengeTimes = 0;
	inPacket >> AddChallengeTimes;

	unsigned int nJoinTime = 0;
	inPacket >> nJoinTime;

	CDataPacket& out = allocProtoPacket(dcAddMember);
	out << nRawServerId << nLoginServerId;
	out << nActorid;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;
		out << Guildid;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_DeleteGuildMember,nActorid);

		if(!nError)
		{
			m_pSQLConnection->ResetQuery();	
		}

		nError = m_pSQLConnection->Exec(szSQLSP_AddGuildMember,Guildid,nActorid,nType,nGx,nHasShxTimes,nAddShxTimes,nExploreTimes,
			nExploreItemId,nChallengeTimes,AddChallengeTimes,nJoinTime);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();			
			out << (BYTE)reSucc;
			out << Guildid;
		}
		else
		{
			out << (BYTE)reDbErr;
			out << Guildid;
		}
	}
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::InitGuildSkillInfo(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> Guildid;
	int nCount=0;
	inPacket >> nCount;

	int skillId = 0;
	int skillLevel = 0;

	int reDbReult = reSucc;
	CDataPacket& out = allocProtoPacket(dcInitGuildSkill);
	out << nRawServerId << nLoginServerId;
	if ( !m_pSQLConnection->Connected() )
	{
		reDbReult=reDbErr;
	}

	int nError;
	for(int i=0;i<nCount;i++)
	{
		inPacket >> skillId;
		inPacket >> skillLevel;

		nError = m_pSQLConnection->Exec(szSQLSP_InitGuildSkill,Guildid,skillId,skillLevel);
		reDbReult=reSucc;

		if(!nError)
		{
			m_pSQLConnection->ResetQuery();	
		}
		else
		{
			reDbReult=reDbErr;
		}
	}

	out << reDbReult;
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::SaveGuildSkill(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	int nSkillid = 0;
	int nLevel = 0;
	unsigned int nActorid = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorid;
	inPacket >> Guildid;
	inPacket >> nSkillid;
	inPacket >> nLevel;

	CDataPacket& out = allocProtoPacket(dcSaveGuildSkill);
	out << nRawServerId << nLoginServerId;
	out << nActorid;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_DeleteGuildSkill,Guildid,nSkillid);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();
		}

		nError = m_pSQLConnection->Exec(szSQLSP_InitGuildSkill,Guildid,nSkillid,nLevel);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();			
			out << (BYTE)reSucc;
			out << Guildid;
			out << nSkillid;
			out << nLevel;
		}
		else
		{
			out << (BYTE)reDbErr;
		}
	}
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::SaveGuildEventDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	int nType = 0;
	int nTime = 0;

	int nEventId = 0;
	int nParam1 = 0;
	int nParam2 = 0;
	int nParam3 = 0;
	char nParam4[32];
	char nParam5[32];

	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nType;
	inPacket >> Guildid;
	inPacket >> nTime;

	inPacket >> nEventId;

	inPacket >> nParam1;
	inPacket >> nParam2;
	inPacket >> nParam3;

	inPacket.readString(nParam4,ArrayCount(nParam4));
	nParam4[sizeof(nParam4)-1] = 0;

	inPacket.readString(nParam5,ArrayCount(nParam5));
	nParam5[sizeof(nParam5)-1] = 0;

	//char nRecord[100];
	//inPacket.readString(nRecord,ArrayCount(nRecord));

	//nRecord[sizeof(nRecord)-1] = 0;
	// CDataPacket& out = allocProtoPacket(dcSaveGuildEvent);
	// out << nRawServerId << nLoginServerId;
	// out << Guildid;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		// out << (BYTE)reDbErr;
	}
	else
	{
		int nError;
		if(nType == 0)
		{
			nError= m_pSQLConnection->Exec(szSQLSP_DeleteGuildEvent,Guildid);
			if ( !nError )
			{
				m_pSQLConnection->ResetQuery();
			}
		}

		nError = m_pSQLConnection->Exec(szSQLSP_AddeGuildEvent,Guildid,nTime,nEventId,nParam1,nParam2,nParam3,nParam4,nParam5);
		//OutputMsg(rmTip,_T("save guild event to db:%d",nError));
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();			
			// out << (BYTE)reSucc;
		}
		else
		{
			// out << (BYTE)reDbErr;
		}
	}

	// flushProtoPacket(out);
}


void CLogicDBReqestHandler::SaveGuild(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> Guildid;

	// CDataPacket& out = allocProtoPacket(dcSaveGuild);
	// out << nRawServerId << nLoginServerId;
	// int pos = (int)out.getPosition();
	// out << (BYTE)reSucc;
	// out << Guildid;

	char sLeaderName[32];
	inPacket.readString(sLeaderName,ArrayCount(sLeaderName));//帮主名字，只保存给后台查询，不load

	WORD nLevel = 0;
	inPacket >> nLevel;

	int nGuildCoin = 0;
	inPacket >> nGuildCoin;

	char sOutMemo[500];
	char sInMemo[500];
	inPacket.readString(sOutMemo,ArrayCount(sOutMemo));
	sOutMemo[sizeof(sOutMemo)-1] = 0;
	inPacket.readString(sInMemo,ArrayCount(sInMemo));
	sInMemo[sizeof(sInMemo)-1] = 0;

	// int nYyType = 0;
	// inPacket >> nYyType;

	// char sQqGroupId[32];
	// inPacket.readString(sQqGroupId,ArrayCount(sQqGroupId));
	// sQqGroupId[sizeof(sQqGroupId)-1] = 0;

	// char sYGroupId[32];
	// inPacket.readString(sYGroupId,ArrayCount(sYGroupId));
	// sYGroupId[sizeof(sYGroupId)-1] = 0;

	// ACTORNAME sGrilCompere;
	// inPacket.readString(sGrilCompere, ArrayCount(sGrilCompere));
	// char sYyMemo[500];
	// inPacket.readString(sYyMemo,ArrayCount(sYyMemo));
	// sYyMemo[sizeof(sYyMemo)-1] = 0;

	int nBinCoin = 0;
	inPacket >> nBinCoin;

	int nFlag = 0;
	inPacket >> nFlag;

	// char sGroupMemo[500];
	// inPacket.readString(sGroupMemo,ArrayCount(sGroupMemo));
	// sGroupMemo[sizeof(sGroupMemo)-1] = 0;

	// int sFlag = 0;
	// inPacket >> sFlag;

	int nMainHallLev = 0;
	inPacket >> nMainHallLev;

	int nExerciseRoomlv = 0;
	inPacket >> nExerciseRoomlv;

	int nAssemblyHallLv = 0;
	inPacket >> nAssemblyHallLv;

	// int nGuildBossLevel = 0;
	// inPacket >> nGuildBossLevel;

	// int nGuildChangleTimes = 0;
	// inPacket >> nGuildChangleTimes;

	// int nUpLeftTime = 0;			//字段upnexttime，建筑升级cd到期时间
	// inPacket >> nUpLeftTime;

	/*
	unsigned int nTechMakeLong = 0;
	inPacket >> nTechMakeLong;
	unsigned int nThunderPower = 0;
	inPacket >> nThunderPower;
	*/
	// unsigned int nCallTime = 0;		//字段reserver，行会召集时间
	// inPacket >> nCallTime;

	int nAutoJoinLevel = 0;
	inPacket >> nAutoJoinLevel;
	// int	nUpgradeItemNum1;
	// inPacket >> nUpgradeItemNum1;
	// int	nUpgradeItemNum2;
	// inPacket >> nUpgradeItemNum2;
	// int	nUpgradeItemNum3;
	// inPacket >> nUpgradeItemNum3;
	// int	m_nExchangeCheckCircleMin;					//替换nTreeLevel;
	// inPacket >> m_nExchangeCheckCircleMin;			//nTreeLevel;
	// int	nTreeDegree;
	// inPacket >> nTreeDegree;
	// int	nFruitLevel;
	// inPacket >> nFruitLevel;
	// int	nFruitDegree;
	// inPacket >> nFruitDegree;
	// int	nFruitNum;
	// inPacket >> nFruitNum;
	// unsigned int nFruitTime;
	// inPacket >> nFruitTime;
	// int	nTaskId;
	// inPacket >> nTaskId;
	// int	nTaskSche;
	// inPacket >> nTaskSche;
	// int	nDailyCoinDonated;
	// inPacket >> nDailyCoinDonated;
	int nCertification;
	inPacket >> nCertification;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		// BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
		// *bRet = reDbErr;
	}
	else
	{
		// int nError = m_pSQLConnection->Exec(szSQLSP_UpdateGuildBasicData,Guildid,sLeaderName,nLevel,nYyType,nGuildCoin,nBinCoin,nFlag,sFlag,
		// 	sInMemo,sOutMemo,sQqGroupId,sYGroupId,sGrilCompere,sYyMemo,sGroupMemo,nMainHallLev,nGuildShopLev,nMoonBoxLevel,
		// 	nGuildBossLevel, nGuildChangleTimes, nUpLeftTime, nCallTime, 
		// 	nAutoJoinLevel, nUpgradeItemNum1, nUpgradeItemNum2, nUpgradeItemNum3,
		// 	m_nExchangeCheckCircleMin/*nTreeLevel*/, 
		// 	nTreeDegree, nFruitLevel, nFruitDegree, nFruitNum, nFruitTime,
		// 	nTaskId, nTaskSche, nDailyCoinDonated,nCertification);
		int nError = m_pSQLConnection->Exec(szSQLSP_UpdateGuildBasicData,Guildid,sLeaderName,nLevel,nGuildCoin,nBinCoin,nFlag,
			sInMemo,sOutMemo,nMainHallLev,nExerciseRoomlv,nAssemblyHallLv,
			nAutoJoinLevel,nCertification);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();

			nError = m_pSQLConnection->Exec(szSQLSP_DeleteGuildWarData,Guildid);//先删除原有数据
			if ( !nError )
			{
				m_pSQLConnection->ResetQuery();
			}
			// else
			// {
			// 	BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
			// 	*bRet = reDbErr;
			// }
			int nWarCount = 0;
			inPacket >> nWarCount;
			for (int i = 0; i < nWarCount;i++)
			{
				WarRelationData war;
				unsigned int nWarGuildId =0;//敌对的帮派id
				inPacket >> war.nGuildId;
				inPacket >> war.m_WarData.nRelationShip;
				inPacket >> war.m_WarData.nState;
				inPacket >> war.m_WarData.nPkCount;
				inPacket >> war.m_WarData.nDieCount;
				inPacket >> war.m_WarData.nNext;
				nError = m_pSQLConnection->Exec(szSQLSP_AddGuildWarData,Guildid,war.m_WarData.nRelationShip,war.m_WarData.nState,war.nGuildId,
					war.m_WarData.nPkCount,war.m_WarData.nDieCount,war.m_WarData.nNext);
				if ( !nError )
				{
					m_pSQLConnection->ResetQuery();
				}
				else
				{
					// BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
					// *bRet = reDbErr;
					break;
				}
			}
		}
		else
		{
			// BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
			// *bRet = reDbErr;
		}
	}
	// flushProtoPacket(out);
}

void CLogicDBReqestHandler::DeleteGuild(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int Guildid = 0;
	int nIndex = 0;
	UINT64 hHandle = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> Guildid;
	inPacket >> nIndex;
	inPacket >> hHandle;

	CDataPacket& out = allocProtoPacket(dcDeleteGuild);
	out << nRawServerId << nLoginServerId;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		int nError = m_pSQLConnection->Exec(szSQLSP_DeleteGuild,Guildid);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();
			out << (BYTE)reSucc;
			
			
			int ret = m_pHost->CatchRemoveGuild(Guildid);
			
			if (ret != reSucc)
			{
				//出现错误，不用名字服务器返回
				//SendAddGuildResult(nRawServerId, nLoginServerId, ret,nIndex,hActor,0,sGuildName,sFoundname,nZTType,nIcon);
				return;
			} 
		}
		else
		{
			out << (BYTE)reDbErr;
			OutputMsg(rmError,_T("delete Guild error!id=%u"),	Guildid);
		}
	}
	out << Guildid;
	out << nIndex;
	out << hHandle;
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::AddMemberOffLine(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	unsigned int nActorid = 0;//操作人的id
	unsigned int nAddActorid = 0;//新加入的人的id
	unsigned int nGuildid = 0;
	int nGuildPos = 0;
	inPacket >> nActorid;
	inPacket >> nAddActorid;
	inPacket >> nGuildid;
	inPacket >> nGuildPos;
	CDataPacket& out = allocProtoPacket(dcAddMemberOffLine);

	
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;

	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_AddOfflineGuildMember,nGuildid,nAddActorid,nGuildPos);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow)
			{
				out << (BYTE)reSucc;
				ActorCommonNode Node;
				sscanf(pRow[0], "%u", &Node.BasicData.nActorId);
				pRow[1]?sscanf(pRow[1], "%s", &Node.sActorName):Node.sActorName[0]=0;
				sscanf(pRow[2], "%d", &Node.BasicData.nGuildGX);
				int temp;
				sscanf(pRow[3], "%d", &temp);//typetk
				Node.BasicData.nTK = (BYTE)temp;
				Node.BasicData.nType = BYTE(temp >> 16);
				sscanf(pRow[4], "%d", &temp);//sex
				Node.BasicData.nSex = temp;
				sscanf(pRow[5], "%d", &temp);//nlevel
				Node.BasicData.nLevel = temp;
				sscanf(pRow[6], "%d", &temp);//job
				Node.BasicData.nJob = temp;
				sscanf(pRow[7], "%u", &(Node.BasicData.nZzNext));//
				sscanf(pRow[8], "%u", &(Node.BasicData.nZjNext));//
				out << Node;
			}
			else
			{
				out << (BYTE)reDbErr;
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			out << (BYTE)reDbErr;
		}

	}
	out << nGuildid;
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::SaveGuildDepotDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	unsigned int Guildid = 0;

	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> Guildid;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_DeleteDepotItem,Guildid);

		if ( !nError )
		{
			
			m_pSQLConnection->ResetQuery();

			int nCount = 0;
			inPacket >> nCount;

			if(nCount > 0)
			{
				CUserItem data;
				memset(&data,0,sizeof(data));
				int nValue = 0;

				char buff[1024];
				strcpy(m_pHugeSQLBuffer,szSQLSP_AddGuildDepotItem);
				for(int i=0;i<nCount;i++)
				{
					unsigned int nActorId = 0;
					inPacket >> nActorId;
					int nType = 0;
					inPacket >> nType;
					int nCoinNum = 0;
					inPacket >> nCoinNum;

					unsigned int nExchangeActorId = 0;
					ACTORNAME 	szExchangeActorName;
					unsigned int nExchangeTime = 0;
					inPacket >> nExchangeActorId;
					inPacket.readString(szExchangeActorName, ArrayCount(szExchangeActorName));
					inPacket >> nExchangeTime;

					inPacket >> data; //装载入物品的数据

					strcat(m_pHugeSQLBuffer," (");

					sprintf(buff,"%u,",Guildid);
					strcat(m_pHugeSQLBuffer,buff);

					sprintf(buff,"%u,%lld,",nActorId,data.series.llId); //actotID,guid
					strcat(m_pHugeSQLBuffer,buff);

					nValue = MAKELONG( data.wItemId, MAKEWORD( data.btQuality, data.btStrong));
					
					sprintf(buff,"%d,%d,",nValue, 0); //MAKELONG(data.wDura,data.wDuraMax)); 
					strcat(m_pHugeSQLBuffer,buff);

					//wPackageType,btFlag,wHoles
					nValue = MAKELONG( MAKEWORD( data.wPackageType, data.btFlag), data.wCount); 
					sprintf(buff," %d,",nValue); 
					strcat(m_pHugeSQLBuffer,buff);
					
					//BYTE	bLostStar;BYTE	bInSourceType;//铭刻等级WORD    bStrongResereLevel;//保留(可使用)
					nValue = MAKELONG( MAKEWORD( data.bLostStar, data.bInSourceType), data.wIdentifySlotNum);	//bStrongResereLevel->wIdentifySlotNum

					sprintf(buff,"%d,",nValue);
					strcat(m_pHugeSQLBuffer,buff);

					//data reserver 
					//MAKELONG(A,B) 这里A是低字节，B是高字节，类似 MAKEWORD
					nValue = MAKELONG(   data.nDropMonsterId, data.wStar );		//btSmithCoun->nDropMonsterId:物品已经被鉴定的次数，wStar：强化星级

					sprintf(buff,"%d,%d,",data.nCreatetime.tv,nValue); 

					strcat(m_pHugeSQLBuffer,buff);
					nValue= MAKELONG( MAKEWORD(data.btSharp, data.btDeportId), MAKEWORD(data.btHandPos, data.btLuck)); 

					sprintf(buff,"%d,%d,%d,%d,%d,",data.smithAttrs[0],data.smithAttrs[1],data.smithAttrs[2],nValue,data.nAreaId); 
					strcat(m_pHugeSQLBuffer,buff);

					sprintf(buff,"%d,%d,",nType,nCoinNum);
					strcat(m_pHugeSQLBuffer,buff);

					sprintf(buff,"%u,'%s',%u",nExchangeActorId, szExchangeActorName, nExchangeTime);
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if ((i>0 && i%300 == 0) || i == nCount-1)
					{
						nError = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
						if(!nError)  
						{
							m_pSQLConnection->ResetQuery(); 
							strcpy(m_pHugeSQLBuffer,szSQLSP_AddGuildDepotItem);
						}
					}
					else if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}

				//OutputMsg(rmTip, _T("login SaveGuildDepotDb = %s"), m_pHugeSQLBuffer);

				
			}
		}
	}
}

void CLogicDBReqestHandler::SaveGuildDepotRecordDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId, nLoginServerId;
	unsigned int Guildid = 0;

	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> Guildid;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_DeleteGuildDepotRecord,Guildid);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();

			int nCount = 0;
			inPacket >> nCount;

			if(nCount > 0)
			{
				char buff[1024];
				strcpy(m_pHugeSQLBuffer,szSQLSP_AddGuildDepotRecord);

				if(nCount > 100)
					nCount = 100;

				for(int i=0;i<nCount;i++)
				{
					GUILDDEPOTMSG nRecord;

					inPacket >> nRecord.nCoinNum;
					inPacket >> nRecord.nCoinType;
					inPacket >> nRecord.nNum;
					inPacket >> nRecord.nTime;
					inPacket >> nRecord.nType;
					inPacket.readString(nRecord.nActorName,ArrayCount(nRecord.nActorName));
					nRecord.nActorName[sizeof(nRecord.nActorName)-1] = 0;
					inPacket.readString(nRecord.nItemName,ArrayCount(nRecord.nItemName));
					nRecord.nItemName[sizeof(nRecord.nItemName)-1] = 0;

					strcat(m_pHugeSQLBuffer," (");

					sprintf(buff,"%u,%d,%d,%d,%d,%d,\"%s\",\"%s\"",Guildid,nRecord.nType,nRecord.nTime,nRecord.nNum,nRecord.nCoinType,nRecord.nCoinNum,nRecord.nActorName,nRecord.nItemName); 
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}

				nError = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if(!nError)  
					m_pSQLConnection->ResetQuery(); 
			}
		}
	}
}

void CLogicDBReqestHandler::LoadGuildDepoyFromDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId >> Guildid;

	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID = reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadDepotItem,Guildid);
		if ( !nError )
		{
			CDataPacket& out = allocProtoPacket(dcLoadGuildDepotItem);
			out << nRawServerId << nLoginServerId;
			out << (BYTE)reSucc;
			out << Guildid;

			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << nCount;

			int  nValue = 0;
			CUserItem data;
			memset(&data,0,sizeof(data));
			WORD wValue = 0;

			while ( pRow)
			{
				data.series.llId= StrToInt64(pRow[0]);
				//itemID,Quality,strong
				sscanf(pRow[1], "%d",&nValue);//itemIdQuaStrong
				data.wItemId = LOWORD(nValue);
				wValue = HIWORD(nValue);
				data.btQuality = LOBYTE(wValue);
				data.btStrong = HIBYTE(wValue);

				//耐久度
				sscanf(pRow[2], "%d",&nValue);
				// data.wDura =  LOWORD(nValue);
				// data.wDuraMax = HIWORD(nValue);

				//包裹类别与flag,镶嵌的的第1字节
				sscanf(pRow[3], "%d",&nValue);
				wValue = LOWORD(nValue);
				data.wPackageType = LOBYTE(wValue);
				data.btFlag = HIBYTE(wValue);
				data.wCount =  HIWORD(nValue);

				//镶嵌的2,3个word
				sscanf(pRow[4], "%d",&nValue);
				wValue = LOWORD(nValue);
				data.wIdentifySlotNum = HIWORD(nValue);	//bStrongResereLevel->wIdentifySlotNum
				data.bLostStar = LOBYTE(wValue);
				data.bInSourceType = HIBYTE(wValue);

				//时间
				sscanf(pRow[5], "%d",&data.nCreatetime.tv);

				//reserverd
				sscanf(pRow[6], "%d",&nValue);

				data.nDropMonsterId = LOWORD(nValue);			//btSmithCount -> nDropMonsterId
				wValue = HIWORD(nValue);
				data.wStar = wValue; //第4个空的道具

				//smith1
				sscanf(pRow[7], "%d",&data.smithAttrs[0]);
				//smith2
				sscanf(pRow[8], "%d",&data.smithAttrs[1]);
				//smith3
				sscanf(pRow[9], "%d",&data.smithAttrs[2]);

				sscanf(pRow[10], "%d",&nValue);
				wValue = HIWORD(nValue);
				data.btLuck =  HIBYTE(wValue);
				wValue = LOWORD(nValue);
				data.btDeportId = HIBYTE(wValue);
				data.btSharp = LOBYTE(wValue);

				if(pRow[11])
				{
					sscanf(pRow[11], "%d",&data.nAreaId);
				}

				unsigned int nActorId = 0;
				sscanf(pRow[12], "%u",&nActorId);
				out << nActorId;

				int nType = 0;
				sscanf(pRow[13], "%d",&nType);
				out << nType;

				int nCoin = 0;
				sscanf(pRow[14], "%d",&nCoin);
				out << nCoin;

				unsigned int nExchangeActorId = 0;				//申请兑换的玩家ID
				pRow[15]? sscanf(pRow[15], "%u",&nExchangeActorId) : nExchangeActorId = 0; 
				out << nExchangeActorId;

				char szExchangeActorName[32]={0};				//申请兑换的玩家ID
				//pRow[16]? _asncpytA(szExchangeActorName, pRow[16]) : szExchangeActorName[0] = 0;
				if( pRow[16] )
				{
					_asncpytA(szExchangeActorName, pRow[16]);
				}
				out.writeString(szExchangeActorName);

				unsigned int nExchangeTime = 0;				//申请兑换的时间
				pRow[17]? sscanf(pRow[17], "%u",&nExchangeTime) : nExchangeTime = 0;
				out << nExchangeTime;

				out << data;
				pRow = m_pSQLConnection->NextRow();
			}

			flushProtoPacket(out);
			m_pSQLConnection->ResetQuery();

		}
		else
		{
			nErroID = reDbErr;
		}
	}

	if( nErroID != reSucc)
	{
		CDataPacket& retPack = allocProtoPacket(dcLoadGuildDepotItem);
		retPack << nRawServerId << nLoginServerId;
		retPack << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}

}

void CLogicDBReqestHandler::LoadGuildDepotRecordDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId >> Guildid;

	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID = reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadDepotRecord,Guildid);
		if ( !nError )
		{
			CDataPacket& out = allocProtoPacket(dcLoadGuildDepotRecord);
			out << nRawServerId << nLoginServerId;
			out << (BYTE)reSucc;
			out << Guildid;

			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << nCount;

			while ( pRow)
			{
				int nType = 0;
				sscanf(pRow[0], "%d",&nType);
				out << (BYTE)nType;

				int ntime = 0;
				sscanf(pRow[1], "%d",&ntime);
				out << ntime;

				char sActorName[32];
				pRow[2]?sscanf(pRow[2], "%s", &sActorName):sActorName[0]=0;
				out.writeString(sActorName);

				char itemname[32];
				pRow[3]?sscanf(pRow[3], "%s", &itemname):itemname[0]=0;
				out.writeString(itemname);

				int itemnum = 0;
				sscanf(pRow[4], "%d",&itemnum);
				out << itemnum;

				int cointype = 0;
				sscanf(pRow[5], "%d",&cointype);
				out << cointype;

				int coinnum = 0;
				sscanf(pRow[6], "%d",&coinnum);
				out << coinnum;

				pRow = m_pSQLConnection->NextRow();
			}

			flushProtoPacket(out);
			m_pSQLConnection->ResetQuery();

		}
		else
		{
			nErroID = reDbErr;
		}
	}

	if( nErroID != reSucc)
	{
		CDataPacket& retPack = allocProtoPacket(dcLoadGuildDepotRecord);
		retPack << nRawServerId << nLoginServerId;
		retPack << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::LoadActorBuildingDb(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorId = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		//out << (BYTE)reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadActorBuildingData,nActorId);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow)
			{
				CDataPacket& out = allocProtoPacket(dcLoadActorBuildingData);
				out << nRawServerId << nLoginServerId;
				out << nActorId;

				out << (BYTE)reSucc;
				int value;
				if( pRow[0] )			//可能为null，以防万一
				{
					sscanf(pRow[0], "%d", &value);
					out << value;
				}
				else
				{
					out<<(int)0;
				}

				if( pRow[1] )
				{
					sscanf(pRow[1], "%d", &value);
					out << value;
				}
				else
				{
					out<<(int)0;
				}

				if( pRow[2] )
				{
					sscanf(pRow[2], "%d", &value);
					out << value;
				}
				else
				{
					out<<(int)0;
				}

				if( pRow[3] )
				{
					sscanf(pRow[3], "%d", &value);
					out << value;
				}
				else
				{
					out<<(int)0;
				}

				if( pRow[4] )
				{
					sscanf(pRow[4], "%d", &value);
					out << value;
				}
				else
				{
					out<<(int)0;
				}

				if( pRow[5] )
				{
					sscanf(pRow[5], "%d", &value);
					out << value;
				}
				else
				{
					out<<(int)0;
				}

				if( pRow[6] )
				{
					sscanf(pRow[6], "%d", &value);
					out << value;
				}
				else
				{
					out<<(int)0;
				}

				flushProtoPacket(out);
			}
			else
			{
				//out << (BYTE)reDbErr;
			}

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			//out << (BYTE)reDbErr;
		}
	}
}




void CLogicDBReqestHandler::DealGuildApply(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	unsigned int nActorId = 0;
	int type = 0;
	inPacket >> nRawServerId >> nLoginServerId >> Guildid >>nActorId >> type;
	if(type == 0)
		return;
	// CDataPacket& out = allocProtoPacket(dcDealGuildApply);
	// out << nRawServerId << nLoginServerId;
	// int pos = (int)out.getPosition();
	// out << (BYTE)reSucc;
	// out << Guildid;
	// out <<nActorId;
	// out << type;
	int result = 0;
	int nError = 0;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		// BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
		// *bRet = reDbErr;

		return;
	}
	else
	{
		if(type == 1)
		{
			nError = m_pSQLConnection->Exec(szSQLSP_GuildInsertApply,Guildid, nActorId, 0);
		}
		else
		{
			nError = m_pSQLConnection->Exec(szSQLSP_GuildDeleteApply,Guildid, nActorId);
		}
		if(!nError)
		{
			m_pSQLConnection->ResetQuery();
		}
		
		
		// else
		// {
		// 	BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
		// 	*bRet = reDbErr;
		// }
		// out << result;

	}
	// flushProtoPacket(out);
}


void CLogicDBReqestHandler::LoadGuildApplyList(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int Guildid = 0;
	inPacket >> nRawServerId >> nLoginServerId >> Guildid;
	CDataPacket& out = allocProtoPacket(dcLoadGuildApplyLists);
	out << nRawServerId << nLoginServerId;
	int pos = (int)out.getPosition();
	out << (BYTE)reSucc;
	out << Guildid;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
		*bRet = reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildApplyLists,Guildid);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << nCount;
			while ( pRow)
			{
				unsigned int actorid = 0;
				sscanf(pRow[0], "%u", &actorid);
				out << actorid;
				ACTORNAME name;
				if (pRow[1])
				{
					_asncpytA(name,pRow[1]);
				}else
				{
					name[0]=0;
				}
				out.writeString(name);
				int nvalue = 0; 
				if (pRow[2])
				{
					sscanf(pRow[2], "%d", &nvalue); //sex
				}
				out << nvalue;

				if (pRow[3])
				{
					sscanf(pRow[3], "%d", &nvalue);//level
				}
				out << nvalue;
				
				if (pRow[4])
				{
					sscanf(pRow[4], "%d", &nvalue);//circle
				}
				out << nvalue;

				if(pRow[5])
				{
					sscanf(pRow[5], "%d", &nvalue);//job
				}
				out << nvalue;
				unsigned int guildid = 0;
				if(pRow[6])
				{
					sscanf(pRow[6], "%u", &guildid);//guildid
				}
				out << guildid;
				unsigned int nVip = 0;
				if(pRow[7])
				{
					sscanf(pRow[7], "%u", &guildid);//nVip
				}
				out << nVip;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
			*bRet = nError;
		}

	}
	flushProtoPacket(out);
}



void CLogicDBReqestHandler::LoadActorApplyResult(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	CDataPacket& out = allocProtoPacket(dcLoadActorApplyResult);
	out << nRawServerId << nLoginServerId;
	int pos = (int)out.getPosition();
	out << (BYTE)reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
		*bRet = reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadGuildApplyResult);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << nCount;
			while ( pRow)
			{
				unsigned int actorid = 0;
				sscanf(pRow[0], "%u", &actorid);
				out << actorid;
				unsigned int guildid = 0;
				if(pRow[1])
				{
					sscanf(pRow[1], "%u", &guildid);//guildid
				}
				out << guildid;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			BYTE* bRet = (BYTE*)(out.getMemoryPtr() + pos);
			*bRet = nError;
		}

	}
	flushProtoPacket(out);
}




void CLogicDBReqestHandler::SaveActorApplyResult(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return;
	}
	else
	{
		int nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteActorGuildApplyResult);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}

		char buff[1024];
		strcpy(m_pHugeSQLBuffer, szSQLSP_InsertGuildApplyResult); //表头放这里

		unsigned int nActorId = 0;
		unsigned int nGid = 0;
		int nCount = 0;
		inPacket >> nCount;
		for(int i = 0; i < nCount; ++i)
		{
			inPacket >> nActorId;
			inPacket >> nGid;
			if(!nActorId || !nGid)
				continue;

			strcat(m_pHugeSQLBuffer, " (");
			sprintf(buff, "%u,%u", nActorId, nGid);
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
