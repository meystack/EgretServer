#include "StdAfx.h"
#define CONSIGN_SEND_COUNT_EACH_TIME	300 //寄卖数据，每个包发送的物品数
#define INCOME_SEND_COUNT_EACH_TIME		1000 //收益数据，每个包发送数量
void CLogicDBReqestHandler::SaveActorConsignment(CDataPacketReader &inPacket)
{
	unsigned int nActorId = 0, nSellTime = 0, nPrice = 0,nIncome = 0, nItemId = 0, nCount = 0;
	int nServerId = 0, nRawServerId = 0;
	// CUserItem data;
	BYTE nState= 0;
	WORD wItemCount = 0;
	int nAutoId = 0;
	inPacket >> nRawServerId >> nServerId ;
	inPacket >> nActorId;
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		//保存收益
		inPacket >> nIncome;
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteConsignmentIncome, nActorId,nServerId);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
			if (nIncome > 0)
			{
				char buff[256];
				sprintf(m_pHugeSQLBuffer, szSQLSP_AddCharConsignmentIncome);
				sprintf(buff,"(%u,%u,%d)",nActorId,nIncome,nServerId);
				strcat(m_pHugeSQLBuffer,buff);
				nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if (!nErrorCode)
				{
					m_pSQLConnection->ResetQuery();
				}
				else
				{
					OutputMsg(rmError,"szSQLSP_AddCharConsignmentIncome Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
				}
			}
		}
		else
		{
			OutputMsg(rmError,"szSQLSP_DeleteConsignmentIncome Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
		}
		//保存物品
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteConsignmentItem, nActorId,nServerId);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmError,"szSQLSP_DeleteConsignmentItem Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
		}
		inPacket >> wItemCount;
		if (wItemCount > 0)
		{
			int nValue = 0;
			char  buff[1024];
			sprintf(m_pHugeSQLBuffer, szSQLSP_AddCharConsignmentItem);
			for (int i = 1; i <= wItemCount; i++)
			{
				// memset(&data,0,sizeof(data));
				strcat(m_pHugeSQLBuffer, "(");
				inPacket >> nItemId >> nCount;
				inPacket >> nSellTime >> nPrice >> nActorId >> nState >> nAutoId;
				char cBestAttr[50];//极品属性
				memset(cBestAttr, 0, sizeof(cBestAttr));
				inPacket.readString(cBestAttr, ArrayCount(cBestAttr));
				cBestAttr[sizeof(cBestAttr)-1] = 0;
				nValue = MAKELONG( nItemId, nCount);
				sprintf(buff,"%u,%d,",nActorId,nValue); //actotID,//itemid, itemcount
				strcat(m_pHugeSQLBuffer,buff);

				sprintf(buff,"%u,%d,%d,%d,%d,", nSellTime, nPrice, nState,nServerId,nAutoId);
				strcat(m_pHugeSQLBuffer,buff);
				sprintf(buff,"\"%s\"",cBestAttr);
				strcat(m_pHugeSQLBuffer, ")");
				if (i != wItemCount)
				{			
					strcat(m_pHugeSQLBuffer, ",");
				}
			}
			nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if (!nErrorCode)
			{
				m_pSQLConnection->ResetQuery();
			}
			else
			{
				OutputMsg(rmError,"szSQLSP_AddCharConsignmentItem Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
			}
		}
	}
}

//分包保存,只保存系统的
void CLogicDBReqestHandler::SaveConsignmentItem(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	unsigned int nActorId = 0;
	unsigned int nSellTime = 0;
	unsigned int nPrice = 0;
	unsigned int nItemId = 0;
	unsigned int nCount = 0;
	int nServerId = 0, nRawServerId = 0;
	WORD wStar = 0;
	int nAutoId = 0;
	int nCurPacketCount=0;
	CUserItem data;
	
	BYTE nState= 0;
	inPacket >> nRawServerId >> nServerId ;
	inPacket >> nCurPacketCount;
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		if (nCurPacketCount == 1)
		{
			nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteConsignmentItem, 0,nServerId);//nactorid=0
		}
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();

			WORD nItemCount = 0;
			inPacket >> nItemCount;
			if (nItemCount > 0)
			{
				int nValue = 0;
				char  buff[1024];
				sprintf(m_pHugeSQLBuffer, szSQLSP_AddCharConsignmentItem);
				for (int i = 1; i <= nItemCount; i++)
				{
					memset(&data,0,sizeof(data));
					data << inPacket; //装载入物品的数据
					inPacket >> nSellTime >> nPrice >> nActorId >> nState>> nAutoId;;
					strcat(m_pHugeSQLBuffer," (");

					sprintf(buff,"%u,%lld,",nActorId,data.series.llId); //actotID,guid
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
					sprintf(buff,"\"%s\",\"%s\"",data.cBestAttr, data.cSourceName);
					strcat(m_pHugeSQLBuffer,buff);
					strcat(m_pHugeSQLBuffer,",");

					sprintf(buff,"%d,%d,%d,%d,%d",nSellTime, nPrice, nState, nServerId,nAutoId); //actotID, //itemid, itemcount
					strcat(m_pHugeSQLBuffer,buff);
					strcat(m_pHugeSQLBuffer," )");
					if(i <= nItemCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}
				nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if (!nErrorCode)
				{

					m_pSQLConnection->ResetQuery();
				}
				else
				{
					OutputMsg(rmError,"szSQLSP_AddCharConsignmentItem Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
				}
			}
		}
		else
		{
			OutputMsg(rmError,"szSQLSP_DeleteConsignmentItem Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
		}
	}
}



void CLogicDBReqestHandler::LoadConsignmentItem(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;
	
	int nErrorCode = 0;
	CUserItem data;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadConsignmentItem, nServerId);
		if (!nErrorCode)
		{		
			
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();
			INT_PTR packetCount = (nRowCount-1)/CONSIGN_SEND_COUNT_EACH_TIME+1;
			INT_PTR nNeedSendItemCount =0;
			int nValue =0; WORD wValue = 0;
			bool isEnd = false;
			for (INT_PTR curPacketCount =1; curPacketCount <= packetCount; curPacketCount++)
			{
				if (curPacketCount* CONSIGN_SEND_COUNT_EACH_TIME > nRowCount)
				{
					nNeedSendItemCount =  nRowCount - (curPacketCount-1)*CONSIGN_SEND_COUNT_EACH_TIME;
				}
				else
				{
					nNeedSendItemCount = CONSIGN_SEND_COUNT_EACH_TIME;
				}
				
				if (curPacketCount == packetCount)
				{
					isEnd = true;
				}
				CDataPacket & outPacket = allocProtoPacket(dcLoadConsignmentItem);
				outPacket << nServerId << nServerId <<nErrorCode;
				outPacket << (WORD)nNeedSendItemCount << isEnd;
				for (INT_PTR i = (curPacketCount-1)* CONSIGN_SEND_COUNT_EACH_TIME; i < curPacketCount* CONSIGN_SEND_COUNT_EACH_TIME && i < nRowCount; i++)
				{
					if (pRow)
					{
						memset(&data,0,sizeof(data));
						data.series.llId= StrToInt64(pRow[0]);
						//itemID,Quality,strong
						sscanf(pRow[1], "%d",&nValue);//itemIdQuaStrong
						data.wItemId = LOWORD(nValue);
						wValue = HIWORD(nValue);
						data.btQuality = LOBYTE(wValue);
						data.btStrong = HIBYTE(wValue);

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

						if(pRow[13]) {
							sscanf(pRow[13], "%d",&data.nAreaId);
						}
						memset(&data.cBestAttr, 0 ,sizeof(data.cBestAttr));

						std::string str = "";
						if(pRow[14]) {
							str = pRow[14];
						}

						if(str != "") {
							strcpy(data.cBestAttr, str.c_str());
						}
						data.cBestAttr[sizeof(data.cBestAttr)-1] = '0';
						memset(&data.cSourceName, 0 ,sizeof(data.cSourceName));
						str = "";
						if(pRow[19]) {
							str = pRow[19];
						}
						if(str != "") {
							strcpy(data.cSourceName, str.c_str());
						}
						data.cSourceName[sizeof(data.cSourceName)-1] = '0';
						
						data >> outPacket;
						outPacket << (unsigned int)atoi(pRow[15]);		//actorid
						outPacket << (unsigned int)atoi(pRow[16]);		//selltime
						outPacket << (unsigned int)atoi(pRow[17]);		//price
						outPacket << (BYTE)atoi(pRow[18]);				//state
						outPacket << (int)atoi(pRow[20]);				//autoid
						// memset(&data.cSourceName, 0 ,sizeof(data.cSourceName));
						// str = "";
						// if(pRow[19]) {
						// 	str = pRow[19];
						// }
						// if(str != "") {
						// 	strcpy(data.cSourceName, str.c_str());
						// }
						// data.cSourceName[sizeof(data.cSourceName)-1] = '0';

						pRow = m_pSQLConnection->NextRow();
					}
				}
				flushProtoPacket(outPacket);
			}	
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmError,"szSQLSP_LoadConsignmentItem Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
		}
		
	}
}

void CLogicDBReqestHandler::LoadConsignmentIncome(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;


	int nErrorCode = 0;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadConsignmentIncome,nServerId);
		if (!nErrorCode)
		{		
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();
			INT_PTR packetCount = (nRowCount-1)/INCOME_SEND_COUNT_EACH_TIME+1;
			INT_PTR nNeedSendCount =0;
			int nValue =0;
			for (INT_PTR curPacketCount =1; curPacketCount <= packetCount; curPacketCount++)
			{
				if (curPacketCount* INCOME_SEND_COUNT_EACH_TIME > nRowCount)
				{
					nNeedSendCount =  nRowCount - (curPacketCount-1)*INCOME_SEND_COUNT_EACH_TIME;
				}
				else
				{
					nNeedSendCount = INCOME_SEND_COUNT_EACH_TIME;
				}
				CDataPacket & outPacket = allocProtoPacket(dcLoadConsignmentIncome);
				outPacket << nServerId << nServerId <<nErrorCode;
				outPacket << (WORD)nNeedSendCount;//这个包里有多少邮件
				for (INT_PTR i = (curPacketCount-1)* INCOME_SEND_COUNT_EACH_TIME; i < curPacketCount* INCOME_SEND_COUNT_EACH_TIME && i < nRowCount; i++)
				{
					if (pRow)
					{
						outPacket <<(unsigned int)atoi(pRow[0]);
						outPacket <<(unsigned int)atoi(pRow[1]);
						pRow = m_pSQLConnection->NextRow();
					}
				}
				flushProtoPacket(outPacket);
			}	
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmError,"szSQLSP_LoadConsignmentIncome Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
		}
	}
}
//旧的保存方式
//void CLogicDBReqestHandler::SaveConsignmentIncome(CDataPacketReader &inPacket)
//{
//	DECLARE_FUN_TIME_PROF()
//	unsigned int nActorId = 0, nSellTime = 0, nPrice = 0;
//	int nServerId = 0, nRawServerId = 0;
//	BYTE nCurPacketCount=0;
//	CUserItem data;
//
//	BYTE nState= 0;
//	inPacket >> nRawServerId >> nServerId ;
//	inPacket >> nCurPacketCount;
//	int nErrorCode = reSucc;
//	if (!m_pSQLConnection->Connected())
//	{
//		nErrorCode = reDbNotConnected; //db没有连接上
//	}
//	else 
//	{
//		if (nCurPacketCount == 1)
//		{
//			nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteConsignmentIncome, nActorId);
//		}
//		if (!nErrorCode)
//		{
//			m_pSQLConnection->ResetQuery();
//
//			WORD nIncomeCount = 0;
//			inPacket >> nIncomeCount;
//			if (nIncomeCount > 0)
//			{
//				char buff[256];
//				sprintf(m_pHugeSQLBuffer, szSQLSP_AddCharConsignmentIncome);
//				for (int i = 1; i <= nIncomeCount; i++)
//				{
//					unsigned int nActorId =0, nIncome = 0;
//					inPacket >> nActorId >> nIncome;
//					sprintf(buff,"(%u,%u)",nActorId,nIncome);
//					strcat(m_pHugeSQLBuffer,buff);
//					if (i != nIncomeCount)
//					{			
//						strcat(m_pHugeSQLBuffer, ",");
//					}
//				}
//				nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
//				if (!nErrorCode)
//				{
//					m_pSQLConnection->ResetQuery();
//				}
//				else
//				{
//					OutputMsg(rmError,"szSQLSP_AddCharConsignmentIncome Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
//				}
//			}
//		}
//		else
//		{
//			OutputMsg(rmError,"szSQLSP_DeleteConsignmentIncome Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
//		}
//	}
//}


void DealSaveItemInfoToMysql(char* buff, unsigned int nActorID, CUserItem& data)
{
	sprintf(buff,"%u,%lld,",nActorID,data.series.llId); //actotID,guid

	//ItemId,QUALITY,strong
	int nValue = MAKELONG( data.wItemId, MAKEWORD( data.btQuality, data.btStrong));

	sprintf(buff,"%d,%d,",nValue, 0);//MAKELONG(data.wDura,data.wDuraMax)); 
	//wPackageType,btFlag,wHoles
	nValue = MAKELONG( MAKEWORD( data.wPackageType, data.btFlag), data.wCount); 
	sprintf(buff," %d,",nValue); 
	//BYTE	bCurbLostStarStrongMax;BYTE	bInSourceType;//铭刻等级WORD    bStrongResereLevel;//保留(可使用)
	nValue = MAKELONG( MAKEWORD( data.bLostStar, data.bInSourceType), data.wIdentifySlotNum);	//bStrongResereLevel->wIdentifySlotNum
	sprintf(buff,"%d,",nValue);
	//MAKELONG(A,B) 这里A是低字节，B是高字节，类似 MAKEWORD
	nValue = MAKELONG( data.nDropMonsterId, data.wStar );	//btSmithCount -> nDropMonsterId
	
	sprintf(buff,"%d,%d,",data.nCreatetime.tv,nValue);
	nValue= MAKELONG( MAKEWORD(data.btSharp, data.btDeportId), MAKEWORD(data.btHandPos, data.btLuck)); 

	sprintf(buff,"%d,%d,%d,%d,%d,%d,%d,",data.smithAttrs[0],data.smithAttrs[1],data.smithAttrs[2],data.smithAttrs[3],data.smithAttrs[4],nValue,data.nAreaId); 
	sprintf(buff,"\"%s\"",data.cBestAttr);
}