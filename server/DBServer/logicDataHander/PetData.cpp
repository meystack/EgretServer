#include "StdAfx.h"

//读取玩家的英雄的数据
void CLogicDBReqestHandler::QueryPetData(unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()
	
	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID=  reDbErr;
	}
	else
	{
		//SELECT EXP,hp, mp,NAME,circle,LEVEL,ids,modelid,weaponappear,score,vocation,sex,swingappear,reserver1, reserver2,stage,bodyforce,bless FROM actorpets  WHERE `actorid` = nactorid;
		//查询玩家的的英雄的数据
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadPetData, nActorID);
		if ( !nErroID )
		{
			CDataPacket& retPack = allocProtoPacket(dcLoadHeroData);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorID << (BYTE) nErroID;
			HERODBDATA data;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			retPack << (int)nCount;
			int nValue =0;
			unsigned long long lValue =0;

			WORD wValue =0;
			while ( pRow)
			{
				
				sscanf(pRow[0], "%lld", &lValue);
				data.nExp = lValue;
				
				sscanf(pRow[1], "%d", &nValue);
				data.nHp = nValue;

				sscanf(pRow[2], "%d", &nValue);
				data.nMp = nValue;

				strncpy(data.name,pRow[3],sizeof(data.name));

				sscanf(pRow[4], "%d", &nValue);
				data.bCircle = nValue;

			
				sscanf(pRow[5], "%d", &nValue);
				data.bLevel = nValue;

				
				//IDs
				sscanf(pRow[6], "%d", &nValue);
				data.wIconId = LOWORD(nValue);
				wValue =  HIWORD(nValue);
				data.bID = LOBYTE(wValue);
				data.bState = HIBYTE(wValue);


				sscanf(pRow[7], "%d", &nValue);
				data.nModel = nValue;

				sscanf(pRow[8], "%d", &nValue);
				data.nWeaponAppear = nValue;

				sscanf(pRow[9], "%d", &nValue);
				data.nScore = nValue;

				sscanf(pRow[10], "%d", &nValue);
				data.bVocation = (BYTE)nValue;

				

				sscanf(pRow[11], "%d", &nValue);
				data.bSex = (BYTE)nValue;

				sscanf(pRow[12], "%d", &nValue);
				data.nSwingAppear = nValue;


				sscanf(pRow[13], "%d", &nValue);
				data.nReserver1 = nValue;

				sscanf(pRow[14], "%d", &nValue);
				data.nReserver2 = nValue;

				sscanf(pRow[15], "%d", &nValue);
				data.bStage= (BYTE)nValue;

				sscanf(pRow[16], "%d", &nValue);
				data.nBodyforce = nValue;

				sscanf(pRow[17], "%d", &nValue);
				data.nBless  = nValue;

				retPack << data;
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(retPack);
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			return;
		}

	}
	if( nErroID != reSucc)
	{
		OutputMsg(rmError,_T("%s装载玩家的英雄数据失败%u"),__FUNCTION__,nActorID);
		CDataPacket& retPack = allocProtoPacket(dcLoadHeroData);
		retPack << nRawServerId << nLoginServerId << nActorID << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
	


}
void CLogicDBReqestHandler::QueryPetDetailData(CDataPacketReader &inPacket)
{
	//unsigned int nActorId = 0;
	//inPacket >> nActorId ;
	//QueryItems(nActorId,itItemPet); //查询英雄物品
	//QueryPetSkillData(nActorId);
}

//保存技能的数据
void CLogicDBReqestHandler::SavePetToDB(CDataPacketReader &inPacket, bool bResp)
{	
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int nActorId = 0;
	int nPetCount =0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId >> nPetCount;

	int nErrorCode =0;
	if (nActorId == 0 )
	{
		nErrorCode = reNoActor;
	}
	else
	{
		//先删除玩家的技能列表
		//nErrorCode =ExecuteSimplyProcedure(szSQLSP_CleanCharSkills,nActorId) ;
		int nErrorCode = m_pSQLConnection->Exec(szSQLSP_CleanPetData, nActorId);
		if(! nErrorCode)
		{
			m_pSQLConnection->ResetQuery(); 
		}
		else
		{
			OutputMsg(rmWaning,_T("clean pet error,actorID=%u"),nActorId);
		}

		if( nErrorCode == reSucc)
		{

			strcpy(m_pHugeSQLBuffer,szSQLSP_AddPets); //表头放这里
			char buff[2048];
			HERODBDATA data;
			
			// "insert into skill(attack,defence,agility,physique,exp,hp,mp,name,quality,strong,circle,level,ids,loyalty) values";//添加角色技能(EXEC)
			if(nPetCount >0)
			{
				for(INT_PTR i=0; i< nPetCount; i++)
				{
					inPacket >> data; //每次读一个的数据

					strcat(m_pHugeSQLBuffer," (");	

					int nId  =MAKELONG(data.wIconId,MAKEWORD(data.bID,data.bState));

					
					sprintf(buff,"%lld,%lld,%d,%d,\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",nActorId,data.nExp,data.nHp,
						data.nMp,data.name,data.bCircle,data.bLevel,nId,data.nModel,
						data.nWeaponAppear,data.nScore,data.bVocation,data.bSex,data.nSwingAppear,data.nReserver1,data.nReserver2,data.bStage,data.nBodyforce,data.nBless); //actotID,guid
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if(i < nPetCount -1)
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
					OutputMsg(rmError,"save pet data error,actorid=%u",nActorId);
				}
			}
			

		}
		
	}
	
	if (bResp)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveHeroData);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorId << char(nErrorCode);
		flushProtoPacket(retPack);
	}
}

/*
dcLoadPetSkillData,   //装载英雄的技能的数据
dcSavePetData,       //保存英雄的数据
dcSaveSkillData,     //保存英雄的技能数据/
*/

void CLogicDBReqestHandler::QueryPetSkillData(unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID=  reDbErr;
	}
	else
	{
		//SELECT petid,skillid,skilllevel,reserver,skillexp FROM petskills  WHERE `actorid` = nactorid;
		//查询玩家的的英雄的数据
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadPetSkillData, nActorID);
		if ( !nErroID )
		{
			CDataPacket& retPack = allocProtoPacket(dcLoadHeroSkillData);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorID << (BYTE) nErroID;
			HEROSKILLDATA data;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			retPack << (int)nCount;
			int nValue =0;
			while ( pRow)
			{
				sscanf(pRow[0], "%d", &nValue);
				data.nHeroID = nValue;

				sscanf(pRow[1], "%d", &nValue);
				data.nSkillID = nValue;

				sscanf(pRow[2], "%d", &nValue);
				data.nLevel = nValue;

				sscanf(pRow[3], "%d", &nValue);
				data.nSlotId  = HIWORD(nValue);
				data.bIsClose =(BYTE)(LOWORD (nValue));

				sscanf(pRow[4], "%d", &nValue);
				data.nExp = nValue;

				retPack << data;
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(retPack);
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			return;
		}
	}
	if( nErroID != reSucc)
	{
		OutputMsg(rmError,_T("%s装载玩家的英雄技能数据失败%u"),__FUNCTION__,nActorID);
		CDataPacket& retPack = allocProtoPacket(dcLoadHeroSkillData);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}

}

void CLogicDBReqestHandler::SavePetSkillData(CDataPacketReader &inPacket, bool bResp)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int nActorId = 0;
	int nCount =0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId >> nCount;

	int nErrorCode =0;
	if (nActorId == 0 )
	{
		nErrorCode = reNoActor;
	}
	else
	{
		//先删除玩家的技能列表
		//nErrorCode =ExecuteSimplyProcedure(szSQLSP_CleanCharSkills,nActorId) ;
		int nErrorCode = m_pSQLConnection->Exec(szSQLSP_cleanPetSkillData, nActorId);
		if(! nErrorCode)
		{
			m_pSQLConnection->ResetQuery(); 
		}
		if( nErrorCode == reSucc)
		{

			if(nCount >0)
			{
				strcpy(m_pHugeSQLBuffer,szSQLSP_addPetSkillData); //表头放这里
				char buff[1024];
				HEROSKILLDATA data;

				// "insert into skill(attack,defence,agility,physique,exp,hp,mp,name,quality,strong,circle,level,ids,loyalty) values";//添加角色技能(EXEC)
				for(INT_PTR i=0; i< nCount; i++)
				{
					memset(&data,0,sizeof(data));
					inPacket >> data; //每次读一个的数据

					strcat(m_pHugeSQLBuffer," (");	

					int nValue =MAKELONG(data.bIsClose,data.nSlotId);

					sprintf(buff,"%lld,%d,%d,%d,%d,%d",nActorId,data.nHeroID,data.nSkillID,data.nLevel,nValue,data.nExp); //actotID,guid
					strcat(m_pHugeSQLBuffer,buff);

					strcat(m_pHugeSQLBuffer," )");	
					if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}

				}
				nErrorCode= m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if(!nErrorCode) m_pSQLConnection->ResetQuery(); 
			}
		
		}
		else
		{
			OutputMsg(rmWaning,_T("clean pet error,actorID=%u"),nActorId);
		}
	}

	if (bResp)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveHeroSkillData);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorId << char(nErrorCode);
		flushProtoPacket(retPack);
	}
}




//读取玩家的英雄的数据
void CLogicDBReqestHandler::QueryLootPetData(unsigned int nActorID,  unsigned int nRawServerId,  unsigned int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()
	
	INT_PTR nErroID = reSucc;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nErroID=  reDbErr;
	}
	else
	{
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadLootPetData, nActorID);
		if ( !nErroID )
		{
			CDataPacket& retPack = allocProtoPacket(dcLoadLootPetData);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorID << (BYTE) nErroID;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			retPack << (int)nCount;
			WORD wValue =0;
			int nValue = 0;
			while ( pRow)
			{
				sscanf(pRow[0], "%d", &wValue);
				retPack << wValue;

				sscanf(pRow[1], "%d", &wValue);
				retPack << wValue;
				sscanf(pRow[2], "%d", &nValue);
				retPack << nValue;
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(retPack);
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			return;
		}

	}
	if( nErroID != reSucc)
	{
		OutputMsg(rmError,_T("%s装载玩家的宠物数据失败%u"),__FUNCTION__,nActorID);
		CDataPacket& retPack = allocProtoPacket(dcLoadLootPetData);
		retPack << nRawServerId << nLoginServerId << nActorID << (BYTE) nErroID;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::SaveLootPetToDB(CDataPacketReader &inPacket, bool bResp)
{	
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int nActorId = 0;
	int nPetCount =0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorId >> nPetCount;

	int nErrorCode =0;
	if ( !m_pSQLConnection->Connected() ) {
		nErrorCode=  reDbErr;
	}
	else {
		if (nActorId == 0 )
		{
			nErrorCode = reNoActor;
		}
		else
		{
			int nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelLootPetData, nActorId);
			if(! nErrorCode)
			{
				m_pSQLConnection->ResetQuery(); 
			}
			else
			{
				OutputMsg(rmWaning,_T("clean pet error,actorID=%u"),nActorId);
			}

			if( nErrorCode == reSucc)
			{
				strcpy(m_pHugeSQLBuffer,szSQLSP_InsertLootPetData); //表头放这里
				char buff[2048];
				if(nPetCount >0)
				{
					for(INT_PTR i=0; i< nPetCount; i++)
					{
						WORD wType = 0;
						WORD wId = 0;
						int nTime = 0;
						inPacket >> wId >> wType >> nTime; //每次读一个的数据

						strcat(m_pHugeSQLBuffer," (");	
						sprintf(buff,"%d, %d,%d,%d",nActorId,wType,wId,nTime);
						strcat(m_pHugeSQLBuffer,buff);

						strcat(m_pHugeSQLBuffer," )");	
						if(i < nPetCount -1)
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
						OutputMsg(rmError,"save pet data error,actorid=%u",nActorId);
					}
				}
			}
		}
	}

	if (bResp)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveLootPetData);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorId << char(nErrorCode);
		flushProtoPacket(retPack);
	}
}