//保存玩家的物品信息
#include "StdAfx.h"
#include<iostream>

void CLogicDBReqestHandler::SaveItems(CDataPacketReader &inPacket, bool bResp)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	BYTE bItemStoreType = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorID >> bItemStoreType; //这里插入状态
	char *sTableName ="";
	int nRetID =0;
	
	switch (bItemStoreType)
	{
	case itItemBag: //背包物品
		sTableName= "actorbagitem";
		nRetID = dcSaveBagItem;
		break;
	case itItemEquip: //装备物品
		sTableName= "actorequipitem";
		nRetID = dcSaveEquipItem;
		break;
	case itItemDepot: //仓库物品
		sTableName= "actordepotitem";
		nRetID = dcSaveDepotItem;
		break;
	case itItemHero: //宠物物品
		sTableName= "actorpetitem";
		nRetID = dcSaveHeroItem;
		break;
	}
	//OutputMsg(rmNormal,"SaveItem [%d][%s]Item",nActorID,sTableName);
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else if(sTableName ==NULL || nRetID == 0) 
	{
		nErrorCode = reParamError ; //参数错误
	}
	else 
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_CleanCharItems, nActorID,bItemStoreType);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		}
		if(nErrorCode == reSucc)
		{
			int nCount = 0;
			CUserItem data;
			inPacket >> nCount; //取物品的数量

			if(nCount >0 ) //如果有物品的话
			{
				sprintf(m_pHugeSQLBuffer,szSQLSP_AddCharItems,sTableName); //表头放这里

				char buff[1024];
				int nValue =0;
				WORD bValue=0;
				for(INT_PTR i =0;i < nCount ; i++)
				{
					
					memset(&data,0,sizeof(data));
					strcat(m_pHugeSQLBuffer," (");
					inPacket >> data; //装载入物品的数据

					sprintf(buff,"%u,%lld,",nActorID,data.series.llId); //actotID,guid
					strcat(m_pHugeSQLBuffer,buff);

					//ItemId,QUALITY,strong
					nValue = MAKELONG( data.wItemId, MAKEWORD( data.btQuality, data.btStrong));

					sprintf(buff,"%d,%d,",nValue, 0);//MAKELONG(data.wDura,data.wDuraMax)); 
					strcat(m_pHugeSQLBuffer,buff);

					//wPackageType,btFlag,wHoles
					nValue = MAKELONG( MAKEWORD( data.wPackageType, data.btFlag), data.wCount); 
					sprintf(buff," %d,",nValue); 
					strcat(m_pHugeSQLBuffer,buff);

					//BYTE	bCurbLostStarStrongMax;BYTE	bInSourceType;//铭刻等级WORD    bStrongResereLevel;//保留(可使用)
					nValue = MAKELONG( MAKEWORD( data.bLostStar, data.bInSourceType), data.wIdentifySlotNum);	//bStrongResereLevel->wIdentifySlotNum

					sprintf(buff,"%d,",nValue);
					strcat(m_pHugeSQLBuffer,buff);

					//data reserver 
					//MAKELONG(A,B) 这里A是低字节，B是高字节，类似 MAKEWORD
					nValue = MAKELONG( data.nDropMonsterId, data.wStar );	//btSmithCount -> nDropMonsterId
					
					sprintf(buff,"%d,%d,",data.nCreatetime.tv,nValue);

					strcat(m_pHugeSQLBuffer,buff);
					nValue= MAKELONG( MAKEWORD(data.btSharp, data.btDeportId), MAKEWORD(data.btHandPos, data.btLuck)); 

					sprintf(buff,"%d,%d,%d,%d,%d,%d,%d,",data.smithAttrs[0],data.smithAttrs[1],data.smithAttrs[2],data.smithAttrs[3],data.smithAttrs[4],nValue,data.nAreaId); 
					strcat(m_pHugeSQLBuffer,buff);
					sprintf(buff,"\"%s\"",data.cBestAttr);
					strcat(m_pHugeSQLBuffer,buff);
					strcat(m_pHugeSQLBuffer,",");	
					sprintf(buff,"\"%s\", \"%s\"",data.Refining,data.cSourceName);
					strcat(m_pHugeSQLBuffer,buff);
					strcat(m_pHugeSQLBuffer," )");
					// if(i < nCount -1)
					// {
					// 	strcat(m_pHugeSQLBuffer,",");	
					// }


					if ((i>0 && i%50 == 0) || i == nCount-1)
					{
						// printf("m_pHugeSQLBuffer sizeof:%d\n",strlen(m_pHugeSQLBuffer));
						nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
						if(!nErrorCode)  
						{
							m_pSQLConnection->ResetQuery(); 
							sprintf(m_pHugeSQLBuffer,szSQLSP_AddCharItems,sTableName); //表头放这里
							// strcpy(m_pHugeSQLBuffer,szSQLSP_SaveMailAttach);
						}
						else
						{
							OutputMsg(rmError,"saveitem Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
						}
					}
					else if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
					// if(i/)
				}
				// printf("m_pHugeSQLBuffer sizeof:%d\n",strlen(m_pHugeSQLBuffer));
				// nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				// if(!nErrorCode)  
				// 	m_pSQLConnection->ResetQuery(); 
			}
		}
	}
	
	if (nErrorCode)
	{
		CDataPacket& retPack = allocProtoPacket(nRetID);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}

void CLogicDBReqestHandler::QueryItemsImpl(MYSQL_ROW pRow, CUserItem& data)
{
	int  nValue = 0;
	WORD wValue = 0;
	memset(&data,0,sizeof(data));
	data.series.llId= StrToInt64(pRow[0]);
	//itemID,Quality,strong
	sscanf(pRow[1], "%d",&nValue);//itemIdQuaStrong
	data.wItemId = LOWORD(nValue);
	wValue = HIWORD(nValue);
	data.btQuality = LOBYTE(wValue);
	data.btStrong = HIBYTE(wValue);

	// //耐久度
	// sscanf(pRow[2], "%d",&nValue);
	// data.wDura =  LOWORD(nValue);
	// data.wDuraMax = HIWORD(nValue);

	//背包类型与flag,镶嵌的的第1字节
	sscanf(pRow[3], "%d",&nValue);
	wValue = LOWORD(nValue);
	data.wPackageType = LOBYTE(wValue);
	data.btFlag = HIBYTE(wValue);
	data.wCount =  HIWORD(nValue);

	//镶嵌的2,3个word
	sscanf(pRow[4], "%d",&nValue);
	wValue = LOWORD(nValue);
	data.wIdentifySlotNum = HIWORD(nValue);		//bStrongResereLevel->wIdentifySlotNum
	data.bLostStar = LOBYTE(wValue);
	data.bInSourceType = HIBYTE(wValue);

	//时间
	sscanf(pRow[5], "%d",&data.nCreatetime.tv);

	//reserverd
	sscanf(pRow[6], "%d",&nValue);

	data.nDropMonsterId = LOWORD(nValue);			//btSmithCount -> nDropMonsterId	
	wValue = HIWORD(nValue);
	data.wStar= wValue; 

	//smith1
	sscanf(pRow[7], "%d",&data.smithAttrs[0]);
	//smith2
	sscanf(pRow[8], "%d",&data.smithAttrs[1]);
	//smith3
	sscanf(pRow[9], "%d",&data.smithAttrs[2]);
	sscanf(pRow[10], "%d",&data.smithAttrs[3]);
	//smith3
	sscanf(pRow[11], "%d",&data.smithAttrs[4]);
	sscanf(pRow[12], "%d",&nValue);
	wValue = HIWORD(nValue);
	data.btLuck    =  HIBYTE(wValue);
	data.btHandPos =  LOBYTE(wValue);
	wValue = LOWORD(nValue);
	data.btDeportId = HIBYTE(wValue);
	data.btSharp = LOBYTE(wValue);

	if(pRow[13])
	{
		sscanf(pRow[13], "%d",&data.nAreaId);
	}
	memset(&data.cBestAttr, 0 ,sizeof(data.cBestAttr));
	int pos = 0;
	
	std::string str = "";
	if(pRow[14])
	{
		str = pRow[14];
		//memcpy(&data.cBestAttr,pRow[14],sizeof(data.cBestAttr));
	}
	if(str != "")
	{
		//memcpy(&data.cBestAttr,str.c_str(),sizeof(str));
		strcpy(data.cBestAttr, str.c_str());
	}
	data.cBestAttr[sizeof(data.cBestAttr)-1] = '0';

	memset(&data.Refining, 0 ,sizeof(data.Refining));
	str = "";
	if(pRow[15])
	{
		str = pRow[15];
	}
	if(str != "")
	{
		strcpy(data.Refining, str.c_str());
	}
	data.Refining[sizeof(data.Refining)-1] = '0';
	str = "";
	if(pRow[16])
	{
		str = pRow[16];
	}
	if(str != "")
	{
		strcpy(data.cSourceName, str.c_str());
	}
	data.cSourceName[sizeof(data.cSourceName)-1] = '0';
}

void CLogicDBReqestHandler::QueryItems( unsigned int nActorID,unsigned int nItemStoreType, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR nErroID = reSucc;	
	INT_PTR nRetID = 0;
	char *sTableName ="";
	switch (nItemStoreType)
	{
	case itItemBag:
		nRetID= dcLoadBagItem;
		sTableName= "actorbagitem";
		break;
	case itItemEquip:
		nRetID= dcLoadEquipItem;
		sTableName= "actorequipitem";
		break;
	case itItemDepot:
		sTableName= "actordepotitem";
		nRetID= dcLoadDepotItem;
		break;
	case itItemHero:
		sTableName= "actorpetitem";
		nRetID = dcLoadHeroItem;
		break;
	// case itItemOfflineEquip:
	// 	sTableName = "OfflineEquip";
	// 	nItemStoreType= itItemEquip; //这里还是装载装备的
	// 	nRetID = dcLoadOfflineUserOther;
	// 	break;
	}
	if (0 == nRetID) 
		return; //参数错误
	//OutputMsg(rmNormal,"QueryItems [%d][%s]Item",nActorID,sTableName);
	
	if (!m_pSQLConnection->Connected())
	{
		nErroID = reDbErr;
	}
	else
	{
		//查询这种物品
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadCharItems, nActorID, nItemStoreType);
		if ( !nErroID )
		{
			
			CDataPacket& retPack = allocProtoPacket((WORD)nRetID);
			retPack << nRawServerId << nLoginServerId;
			retPack << nActorID << (BYTE) nErroID;
			CUserItem data;
			

			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			retPack << (int)nCount;
			// OutputMsg(rmTip,_T("玩家actorid:%u 装载%s数据nCount = %d"),nActorID, sTableName,nCount);
			while (pRow)
			{			
				QueryItemsImpl(pRow, data);
				// retPack << data;
				data >> retPack;
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(retPack);
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			return;
		}
		else
		{
			OutputMsg(rmError,
					  _T("%s装载玩家的item数据失败%u,nItemStoreType=%d"),
					  __FUNCTION__,
					  nActorID,
					  nItemStoreType);
			nErroID= reDbErr; 
		}
	}

	//出现了错误，要返回一个错误码
	CDataPacket& retPack = allocProtoPacket((WORD)nRetID);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorID << (BYTE)nErroID;
	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::LoadUserActiveItem(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int nAccountId = 0;
	unsigned int nActorId = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nAccountId;
	inPacket >> nActorId;

	CDataPacket& out = allocProtoPacket(dcLoadActiveBag);
	out << nRawServerId << nLoginServerId;
	out << nActorId;
	if (!m_pSQLConnection->Connected())
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szLoadUserItem,nAccountId,nActorId, nRawServerId);
		if ( !nError )
		{
			out << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << (int)nCount;
			while ( pRow)
			{
				INT64 id;
				id = StrToInt64(pRow[0]);
				int nItemId = _StrToInt(pRow[1]);
				out << (INT64)id;
				out << (int)nItemId;
				int value = _StrToInt(pRow[2]);
				out << (BYTE)value;	//bind
				value = _StrToInt(pRow[3]);
				out << (BYTE)value;	//strong
				value = _StrToInt(pRow[4]);
				out << (BYTE)value;	//quality
				value = _StrToInt(pRow[5]);
				out << (int)value;	//itemcount
				value = 0;
				if(pRow[6])
					value = _StrToInt(pRow[6]);	//smith1
				out << value;

				value = 0;
				if(pRow[7])
					value = _StrToInt(pRow[7]);	//smith2
				out << value;

				value = 0;
				if(pRow[8])
					value = _StrToInt(pRow[8]);	//smith3
				out << value;

				value = 0;
				if(pRow[9])
					value = _StrToInt(pRow[9]);	//smith4
				out << value;

				value = 0;
				if(pRow[10])
					value = _StrToInt(pRow[10]);	//smith5
				out << value;

				value = 0;
				if(pRow[11])
					value = _StrToInt(pRow[11]);	//initsmith
				out << value;

				value = 0;
				if(pRow[12])
					value = _StrToInt(pRow[12]);	//rever1
				out << value;

				value = 0;
				if(pRow[13])
					value = _StrToInt(pRow[13]);	//rever2
				out << value;
				
				pRow = m_pSQLConnection->NextRow();
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

void CLogicDBReqestHandler::GetUserActiveItem(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	unsigned int nAccountId = 0;
	unsigned int nActorId = 0;
	INT64 nId = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nAccountId;
	inPacket >> nActorId;
	inPacket >> nId;

	CDataPacket& out = allocProtoPacket(dcGetActiveItem);
	out << nRawServerId << nLoginServerId;
	out << nActorId;
	if (!m_pSQLConnection->Connected())
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szGetUserItem,nAccountId,nActorId,nId, nRawServerId);
		if ( !nError )
		{
			out << (BYTE)reSucc;
			out << (INT64)nId;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();			
			if ( pRow)
			{
				int nItemId = 0;
				if(pRow[0])
				{
					nItemId = _StrToInt(pRow[0]);
				}
				out << (int)nItemId;
				int value = 0;
				if(pRow[1])
				{
					value = _StrToInt(pRow[1]);
				}
				out << (BYTE)value;	//bind
				value = 0;
				if(pRow[2])
				{
					value = _StrToInt(pRow[2]);
				}
				out << (BYTE)value;	//nstrong
				value = 0;
				if(pRow[3])
				{
					value = _StrToInt(pRow[3]);
				}
				out << (BYTE)value;	//nquality
				value = 0;
				if(pRow[4])
				{
					value = _StrToInt(pRow[4]);
				}
				out << (int)value;	//ncount
				value = 0;
				bool flag =true;

				if(flag)
				{
					if(pRow[5] )
					{
						value = _StrToInt(pRow[5]);
					}
					else
					{
						flag =false;
					}
				}
				
				out << (int)value;	//smith1
				value = 0;
				
				if(flag  )
				{
					if(pRow[6] )
					{
						value = _StrToInt(pRow[6]);
					}
					else
					{
						flag =false;
					}
				}
				out << (int)value;	//smith2
				value = 0;

				if(flag  )
				{
					if(pRow[7] )
					{
						value = _StrToInt(pRow[7]);
					}
					else
					{
						flag =false;
					}
				}
				out << (int)value;	//smith3
				value = 0;

				if(flag  )
				{
					if(pRow[8] )
					{
						value = _StrToInt(pRow[8]);
					}
					else
					{
						flag =false;
					}
				}
				out << (int)value;	//smith4
				value = 0;

				if(flag  )
				{
					if(pRow[9] )
					{
						value = _StrToInt(pRow[9]);
					}
					else
					{
						flag =false;
					}
				}
				out << (int)value;	//smith5
				value = 0;
				if(flag  )
				{
					if(pRow[10] )
					{
						value = _StrToInt(pRow[10]);
					}
					else
					{
						flag =false;
					}
				}
				out << (int)value;	//initsmith

				if(flag  )
				{
					if (pRow[11])
					{
						out.writeString(pRow[11]);
					}
					else
					{
						out.writeString("");
					}
				}
				else
				{
					out.writeString("");
				}

			
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

void CLogicDBReqestHandler::RemoveUserActiveItem( CDataPacketReader &inPacket )
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	int nAccountId = 0;
	unsigned int nActorId = 0;
	INT64 nId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorId;
	inPacket >> nId;

	CDataPacket& out = allocProtoPacket(dcDeleteActiveItem);
	out << nRawServerId << nLoginServerId;
	out << nActorId;
	if (!m_pSQLConnection->Connected())
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szDelUserItem,nId);
		if ( !nError )
		{
			out << (BYTE)reSucc;
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			out << (BYTE)reDbErr;
		}
	}
	flushProtoPacket(out);
}

void CLogicDBReqestHandler::SaveAlmirahItem(CDataPacketReader &inPacket, bool bResp)
{
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	BYTE bItemStoreType = 0;
	WORD nCount = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorID >> nCount;
	// CDataPacket & outPack = allocProtoPacket(dcSaveAlmirahItem);
	// outPack << nRawServerId << nLoginServerId << nActorID;
	if (!m_pSQLConnection->Connected())
	{
		// outPack << (byte)reDbErr;
	}
	else
	{
		int nErrId = m_pSQLConnection->Exec(szSQLSP_DeleteAlmirah, nActorID);
		if (!nErrId)
		{
			m_pSQLConnection->ResetQuery();
		}
		if (nErrId == reSucc && nCount > 0)
		{
			sprintf(m_pHugeSQLBuffer, szSQLSP_InsertAlmirah);
			strcat(m_pHugeSQLBuffer, "(");
			for (WORD i = 0; i < nCount; i++)
			{
				char sBuff[1000];
				CAlmirahItem oneItem;
				inPacket >> oneItem.nModelId >> oneItem.btTakeOn>>oneItem.nLv;
				sprintf_s(sBuff, ArrayCount(sBuff), "%u,%d,%d,%d)", nActorID, oneItem.nModelId, oneItem.nLv, oneItem.btTakeOn);
				strcat(m_pHugeSQLBuffer, sBuff);
				if (i + 1 != nCount)
				{
					strcat(m_pHugeSQLBuffer, ",(");
				}
			}
			nErrId = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if (!nErrId)
			{
				m_pSQLConnection->ResetQuery();
			}
		}
		// outPack << (byte)reSucc;
	}
	// flushProtoPacket(outPack);
}

void CLogicDBReqestHandler::LoadAlmirahItem(unsigned int nActorId, int nRawServerId, int nLoginServerId)
{
	CDataPacket & outPack = allocProtoPacket(dcLoadAlmirahItem);
	outPack << nRawServerId << nLoginServerId << nActorId;

	if (!m_pSQLConnection->Connected())
	{
		outPack << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadAlmirah, nActorId);
		if ( !nError )
		{		
			outPack << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			outPack << (WORD)nCount;
			while ( pRow)
			{
				int nTempInt = 0;
				if (pRow[0])
				{
					sscanf(pRow[0], "%d", &nTempInt);
				}
				outPack <<(WORD)nTempInt;

				nTempInt = 0;
				if (pRow[1])
				{
					sscanf(pRow[1], "%d", &nTempInt);
				}
				outPack <<(int)nTempInt;

				nTempInt = 0;
				if (pRow[2])
				{
					sscanf(pRow[2], "%d", &nTempInt);
				}
				outPack <<(byte)nTempInt;

				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
	}
	flushProtoPacket(outPack);
}
