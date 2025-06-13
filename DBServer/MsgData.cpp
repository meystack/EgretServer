#include "StdAfx.h"
#include<iostream>
#define MAIL_DATA_SEND_COUNT_EACH_TIME  50//每次发送的邮件数
#define MAILATTACH_DATA_SEND_COUNT_EACH_TIME  500//每次发送的附件数
#define LoadDbTypeServerMail 0
#define LoadDbTypeServerMailAttach 1
#define LoadDbTypeServerMailComplete 2
void CLogicDBReqestHandler::LoadMsgList(CDataPacketReader &inPacket)
{
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorid = 0;
	INT64 nMsgId = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorid;
	inPacket >> nMsgId;
	LoadMsgListFromId(nActorid, nMsgId, nRawServerId, nLoginServerId);
}

void CLogicDBReqestHandler::LoadMsgListFromId(unsigned int nActorId,INT64 nMsgId, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()

	CDataPacket& tmp = allocProtoPacket(dcLoadMsg);
	tmp << nRawServerId << nLoginServerId;
	CDataPacket* out = &tmp;
	(*out) << (unsigned int)nActorId;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		(*out)  << (BYTE)reDbErr ;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadCharMsgList, nActorId,nMsgId);
		if ( !nError )
		{
			(*out) << (BYTE)reSucc ;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			
			INT_PTR pos = out->getPosition();
			(*out) << (WORD)0;
			nCount = 0;


			while ( pRow)
			{
				nCount++;
				if (nCount > 100)
				{
					WORD* pCount = (WORD*)out->getPositionPtr(pos);
					*pCount = (WORD)(nCount-1);
					flushProtoPacket((*out));

					CDataPacket& msgNew = allocProtoPacket(dcLoadMsg);
					msgNew << nRawServerId << nLoginServerId;
					out = &msgNew;
					(*out) << (unsigned int)nActorId;

					(*out) << (BYTE)reSucc;
					pos = out->getPosition();
					(*out) <<(WORD)0;
					nCount = 1;
				}
				INT64 nMsgid = StrToInt64(pRow[0]);
				int nMsgtype = _StrToInt(pRow[1]);
				int nBufSize = LOWORD(nMsgtype);//大小是低字节
				nMsgtype = HIWORD(nMsgtype);	//消息类型是高字节
				
				if (pRow[2] == NULL)
				{
					nBufSize = 0;
				}

				(*out) << (INT64)nMsgid;
				(*out) << (BYTE)nMsgtype;
				(*out) << (WORD)nBufSize;
				if (pRow[2])
				{
					out->writeBuf(pRow[2],nBufSize);
				}
				pRow = m_pSQLConnection->NextRow();
			}
			WORD* pCount = (WORD*)out->getPositionPtr(pos);
			*pCount = (WORD)nCount;

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			(*out)  << (BYTE)reDbErr ;
		}

	}

	flushProtoPacket(*out);
}

void CLogicDBReqestHandler::DeleteMsg(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	INT64 msgid = 0;
	unsigned int nActorId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorId;
	inPacket >> msgid;

	CDataPacket& retPack = allocProtoPacket(dcDeleteMsg);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorId;
	
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		retPack << (BYTE)reDbErr ;
		retPack << msgid;//如果失败了，要告诉逻辑服务器那条失败了
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_DeleteCharMsg, nActorId,msgid);
		if ( !nError )
		{			
			m_pSQLConnection->ResetQuery();
			retPack << (BYTE)reSucc ;
			retPack << msgid;//如果失败了，要告诉逻辑服务器那条失败了
		}
		else
		{
			retPack << (BYTE)reDbErr ;
			retPack << msgid;//如果失败了，要告诉逻辑服务器那条失败了
		}

	}

	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::AddMsg(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorId = 0,nSrcActorId = 0;
	int nMsgType = 0;
	int bufsize = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorId;
	inPacket >> nSrcActorId;
	inPacket >> nMsgType;
	inPacket >> bufsize;

	CDataPacket& retPack = allocProtoPacket(dcAddMsg);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorId;
	retPack << nSrcActorId;
	nMsgType = MAKELONG(bufsize,nMsgType);//a是低直接
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		retPack << (BYTE)reDbErr ;
	}
	else
	{
		char *ptr = m_pHugeSQLBuffer;

		ptr += sprintf(ptr, szSQLSP_AddCharMsg, nActorId,nMsgType);
		ptr[0] = '"';
		ptr += 1;
		ptr += mysql_escape_string(ptr, inPacket.getOffsetPtr(), bufsize);
		ptr[0] = '"';
		ptr[1] = ')';
		ptr += 2;

		int nError = m_pSQLConnection->RealQuery(m_pHugeSQLBuffer,ptr - m_pHugeSQLBuffer);
		if ( !nError )
		{
			retPack << (BYTE)reSucc ;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if (pRow)
			{
				INT64 Msgid = StrToInt64(pRow[0]);
				retPack << (INT64)Msgid;
			}
			m_pSQLConnection->ResetQuery();
			
		}
		else
		{
			retPack << (BYTE)reDbErr ;
		}

	}

	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::AddMsgByName(CDataPacketReader &inPacket)
{	
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	int nSrcActorId = 0;
	int nServerIdx = -1;
	int nMsgType = 0;
	int bufsize = 0;
	ACTORNAME actorName;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nSrcActorId;
	inPacket >> nServerIdx;
	inPacket.readString(actorName,ArrayCount(actorName));
	inPacket >> nMsgType;
	inPacket >> bufsize;
	
	int actorId = 0;
	CDataPacket& retPack = allocProtoPacket(dcAddMsgByActorName);	
	retPack << nRawServerId << nLoginServerId;
	retPack.writeString(actorName);
	retPack << (int)nSrcActorId;

	nMsgType = MAKELONG(bufsize,nMsgType);//a是低直接
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		retPack << (BYTE)reDbErr ;
	}
	else
	{
		char *ptr = m_pHugeSQLBuffer;

		ptr += sprintf(ptr, szSQLSP_AddCharMsgByName, nServerIdx, actorName,nMsgType);
		ptr[0] = '"';
		ptr += 1;		
		ptr += mysql_escape_string(ptr, inPacket.getOffsetPtr(), bufsize);
		ptr[0] = '"';
		ptr[1] = ')';
		ptr += 2;		

		int nError = m_pSQLConnection->RealQuery(m_pHugeSQLBuffer, ptr - m_pHugeSQLBuffer);
		if ( !nError )
		{
			retPack << (BYTE)reSucc ;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if (pRow)
			{
				unsigned int nActorId = _StrToInt(pRow[0]);
				INT64 Msgid = StrToInt64(pRow[1]);
				retPack << (INT64)Msgid;
			}
			m_pSQLConnection->ResetQuery();

		}
		else
		{
			retPack << (BYTE)reDbErr ;
		}

	}

	flushProtoPacket(retPack);
}


//邮件相关接口

void CLogicDBReqestHandler::AddActorOfflineMail(CDataPacketReader & inPacket)
{
	unsigned int nActorId = 0, nSrcId = 0, nCreateDt = 0;
	byte btState = 0, btIsDel = 0; 
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId >> nActorId;
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connect())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		sprintf(m_pHugeSQLBuffer, szSQLSP_SaveMail);
		INT64 nMailId = 0;
		char sTitle[200], sContent[600], sBuff[1024];
		int nCurCount = 0;

		strcat(m_pHugeSQLBuffer, "(");
		inPacket >> nMailId >> nSrcId;
		inPacket.readString(sTitle, ArrayCount(sTitle));
		inPacket.readString(sContent, ArrayCount(sContent));
		inPacket >> nCreateDt >> btState >> btIsDel;
		char sRealContent[700];
		mysql_escape_string(sRealContent,sContent,(unsigned long)strlen(sContent));
		sprintf(sBuff,"%lld,%u,%u,\"%s\",\"%s\",%d,%d,%d)", nMailId,nActorId,nSrcId,sTitle,sRealContent,nCreateDt,btState,btIsDel);
		strcat(m_pHugeSQLBuffer, sBuff);
		nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
		BYTE nMailAttachCount = 0;
		inPacket >> nMailAttachCount;
		if (nMailAttachCount > 0)
		{
			sprintf(m_pHugeSQLBuffer, szSQLSP_SaveMailAttach);
			int nValue = 0;BYTE nType = 0;
			for (int i = 0; i <nMailAttachCount ; i++)
			{
				strcat(m_pHugeSQLBuffer," (");
				CUserItem data;
				inPacket >> nType;
				data << inPacket;
				sprintf(sBuff,"%lld,%u,%d,%lld,", nMailId,nActorId, nType,data.series.llId); //mailid actotID,guid,type
				strcat(m_pHugeSQLBuffer,sBuff);

				//ItemId,QUALITY,strong
				nValue = MAKELONG( data.wItemId, MAKEWORD( data.btQuality, data.btStrong));

				sprintf(sBuff,"%d,%d,",nValue, data.wCount);//MAKELONG(data.wDura,data.wCount)); 
				strcat(m_pHugeSQLBuffer,sBuff);

				//wPackageType,btFlag,wHoles
				nValue = MAKELONG( MAKEWORD( data.wPackageType, data.btFlag), 0); 
				sprintf(sBuff," %d,",nValue); 
				strcat(m_pHugeSQLBuffer,sBuff);
				//BYTE	bLostStar;BYTE	bInSourceType;//铭刻等级WORD    bStrongResereLevel;//保留(可使用)
				nValue = MAKELONG( MAKEWORD( data.bLostStar, data.bInSourceType), data.wIdentifySlotNum);	//bStrongResereLevel->wIdentifySlotNum

				sprintf(sBuff,"%d,",nValue);
				strcat(m_pHugeSQLBuffer,sBuff);

				//data reserver 
				//MAKELONG(A,B) 这里A是低字节，B是高字节，类似 MAKEWORD
				nValue = MAKELONG( data.nDropMonsterId, data.wStar );			//btSmithCount -> nDropMonsterId	

				sprintf(sBuff,"%d,%d,",data.nCreatetime.tv,nValue); 

				strcat(m_pHugeSQLBuffer,sBuff);
				nValue= MAKELONG( MAKEWORD(data.btSharp, data.btDeportId), MAKEWORD(data.btHandPos, data.btLuck)); 

				sprintf(sBuff,"%d,%d,%d,%d,%d,%d,%d",data.smithAttrs[0],data.smithAttrs[1],data.smithAttrs[2],data.smithAttrs[3],data.smithAttrs[4],nValue,data.nAreaId); 
				strcat(m_pHugeSQLBuffer,sBuff);
				strcat(m_pHugeSQLBuffer," )");	
				if(i < nMailAttachCount -1)
				{
					strcat(m_pHugeSQLBuffer,",");	
				}
			}
			nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if (!nErrorCode)
			{
				m_pSQLConnection->ResetQuery();
			}
		}
	}
}


void CLogicDBReqestHandler::LoadActorMail(CDataPacketReader & inPacket)
{
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;
	unsigned int nActorId = 0;
	inPacket >> nActorId;
	QueryActorMail(nActorId, nRawServerId, nServerId);
}

void CLogicDBReqestHandler::QueryActorMail(unsigned int nActorId, int nRawServerId, int nServerId)
{
	int nErrorCode = 0;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadMail, nActorId);
		if (!nErrorCode)
		{			
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();
			if(nRowCount > 0)
			{
				for (int i=0,packetCount = 1; i < nRowCount; i++)
				{
					WORD wMailCount = 0;
					CDataPacket & outPacket = allocProtoPacket(dcLoadMail);
					outPacket << nRawServerId << nServerId << nActorId << (byte)nErrorCode;
					outPacket << (BYTE)packetCount++;//第几个包
					size_t wMailCountPos = outPacket.getPosition();
					outPacket << (WORD)wMailCount;//邮件数量,读完再修改这个值
					BYTE nMailAttachCount = 0;		//记录每封邮件附件数量
					size_t mailAttachCountPos = 0;
					INT64 nLastMailId = 0;
					while(pRow)
					{
						INT64 nCurMailId = (INT64)StrToInt64(pRow[0]);
						if (nLastMailId != nCurMailId)
						{
							wMailCount++;
							mailAttachCountPos = 0;
							nMailAttachCount = 0;
							nLastMailId = nCurMailId;

							outPacket << nCurMailId;				 //mailid
							outPacket << (unsigned int)atoi(pRow[1]);//srcid
							outPacket.writeString(pRow[2]?pRow[2]:"");//title
							outPacket.writeString(pRow[3]?pRow[3]:"");//content
							outPacket << (unsigned int)atoi(pRow[4]);//createdt
							outPacket << (byte)atoi(pRow[5]);//state
							mailAttachCountPos = outPacket.getPosition();
							outPacket << nMailAttachCount;
						}
						if (pRow[6] != NULL)//有附件
						{
							nMailAttachCount++;
							int  nValue = 0;
							WORD wValue = 0;
							CUserItem userItem;
							memset(&userItem,0,sizeof(userItem));

							outPacket << (BYTE)atoi(pRow[6]);			//type

							userItem.series.llId = StrToInt64(pRow[7]);	//itemguid

							nValue = atoi(pRow[8]);					//itemidquastrong
							userItem.wItemId = LOWORD(nValue);
							wValue = HIWORD(nValue);
							userItem.btQuality = LOBYTE(wValue);
							userItem.btStrong = HIBYTE(wValue);


							userItem.wCount = atoi(pRow[9]);					//wCount

							nValue = atoi(pRow[10]);					//itemcountflag
							wValue = LOWORD(nValue);
							userItem.wPackageType = LOBYTE(wValue);
							userItem.btFlag = HIBYTE(wValue);
							int nCount =  HIWORD(nValue);
							if(nCount > userItem.wCount)
								userItem.wCount = nCount;
							
							nValue = atoi(pRow[11]);	//nStrongStar ->bLostStar(BYTE) +  bInSourceType(BYTE) + bStrongResereLevel(WORD)
							wValue = LOWORD(nValue);
							userItem.wIdentifySlotNum = HIWORD(nValue);	//bStrongResereLevel->wIdentifySlotNum
							userItem.bLostStar = LOBYTE(wValue);
							userItem.bInSourceType = HIBYTE(wValue);

							userItem.nCreatetime = atoi(pRow[12]);			//itemtime

							nValue = atoi(pRow[13]);				//itemreservs
							userItem.nDropMonsterId = LOWORD(nValue);				//btSmithCount -> nDropMonsterId
							wValue = HIWORD(nValue);
							userItem.wStar = wValue;	

							sscanf(pRow[14], "%d",&userItem.smithAttrs[0]); //smith1-5
							sscanf(pRow[15], "%d",&userItem.smithAttrs[1]); 
							sscanf(pRow[16], "%d",&userItem.smithAttrs[2]); 
							sscanf(pRow[17], "%d",&userItem.smithAttrs[3]); 
							sscanf(pRow[18], "%d",&userItem.smithAttrs[4]);

							nValue = atoi(pRow[19]);			//itemreservs2
							wValue = HIWORD(nValue);
							userItem.btLuck    =  HIBYTE(wValue);
							userItem.btHandPos =  LOBYTE(wValue);
							wValue = LOWORD(nValue);
							userItem.btDeportId = HIBYTE(wValue);
							userItem.btSharp = LOBYTE(wValue);

							sscanf(pRow[20], "%d",&userItem.nAreaId);
							// outPacket << userItem;
							userItem >> outPacket;
						}	
						//附件
						if (nMailAttachCount != 0)
						{
							BYTE* pAttachCount = (BYTE*)outPacket.getPositionPtr(mailAttachCountPos);
							*pAttachCount = nMailAttachCount;
						}
						pRow = m_pSQLConnection->NextRow();
						if (pRow && nCurMailId == (INT64)StrToInt64(pRow[0]))
						{
							//邮件附件没读完
							continue;
						}
						if (/*wMailCount == MAIL_DATA_SEND_COUNT_EACH_TIME ||*/ pRow == NULL)
						{
							WORD* pMailCount = (WORD*)outPacket.getPositionPtr(wMailCountPos);
							*pMailCount = wMailCount;
							bool isEnd = pRow == NULL?true:false;
							outPacket << isEnd;
							flushProtoPacket(outPacket);	
							break;
						}
					}
					if (!pRow)
					{
						break;
					}
				}
			}
			else
			{
				CDataPacket & outPacket = allocProtoPacket(dcLoadMail);
				outPacket << nRawServerId << nServerId << nActorId << (byte)nErrorCode;
				outPacket << (BYTE)0;
				outPacket << (WORD)0;
				bool isEnd = true;
				outPacket << isEnd;
				flushProtoPacket(outPacket);	
			}
			
			m_pSQLConnection->ResetQuery();
		}

		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
}

void CLogicDBReqestHandler::SaveNewServerMail(CDataPacketReader & inPacket)
{
	int nErrorCode = 0;
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;
	int nServerMailId = 0;
	inPacket >> nServerMailId;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_SaveNewServerMail, nServerMailId);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmError,"szSQLSP_SaveNewServerMail Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
		}

	}
}



void CLogicDBReqestHandler::AddNewServerMail(CDataPacketReader & inPacket)
{
	int nErrorCode = 0;
	int nServerId = 0, nRawServerId = 0;
	inPacket >> nRawServerId >> nServerId;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		ServerMailDef mail;
		inPacket >> mail.nServerMailId;
		inPacket >> mail.nActorId;
		inPacket.readString(mail.sTitle);
		inPacket.readString(mail.sContent);
		inPacket >> mail.nMinVipLevel >> mail.nMaxVipLevel >> mail.nMinLevel >> mail.nMaxLevel;
		inPacket >> mail.nMinRegisterTime >> mail.nMaxRegisterTime >> mail.nMinLoginTime >> mail.nMaxLoginTime;
		inPacket >> mail.btOnline >> mail.btSex >> mail.nWay;

		sprintf(m_pHugeSQLBuffer, szSQLSP_SaveServerMail);
		INT64 nMailId = 0;
		char sBuff[1024];

		strcat(m_pHugeSQLBuffer, "(");

		sprintf(sBuff,"%u,\"%s\",\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",mail.nActorId,mail.sTitle,mail.sContent,
		mail.nMinVipLevel, mail.nMaxVipLevel,mail.nMinLevel, mail.nMaxLevel,
		mail.nMinRegisterTime, mail.nMaxRegisterTime, mail.nMinLoginTime, mail.nMaxLoginTime,
		mail.btOnline, mail.btSex, mail.nWay);
		strcat(m_pHugeSQLBuffer, sBuff);
		nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
		unsigned int nMaxId = 0;
		nErrorCode = m_pSQLConnection->Query(szSQLSP_GetMaxMailIdServerMail);
		if (!nErrorCode)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow && pRow[0])
			{
				nMaxId = (unsigned int)atoi(pRow[0]);
			}
			m_pSQLConnection->ResetQuery();
		}
		
		BYTE nMailAttachCount = 0;
		inPacket >> nMailAttachCount;
		
		if (nMailAttachCount > 0)
		{
			sprintf(m_pHugeSQLBuffer, szSQLSP_SaveServerMailAttach);
			WORD nValue = 0;BYTE nType = 0;
			int wCount = 0;
			for (int i = 0; i <nMailAttachCount ; i++)
			{
				strcat(m_pHugeSQLBuffer," (");
				inPacket >> nType >> nValue >>wCount;
				sprintf(sBuff,"%lld,%d,%d,%d)",nMaxId,nType, nValue,wCount); //mailid type,id,count
				strcat(m_pHugeSQLBuffer,sBuff);
				if(i < nMailAttachCount -1)
				{
					strcat(m_pHugeSQLBuffer,",");	
				}
			}
			nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if (!nErrorCode)
			{
				m_pSQLConnection->ResetQuery();
			}
		}


	}
}
void CLogicDBReqestHandler::LoadNewServerMail(int nServerId, int nRawServerId, int nMaxServerMailId)
{
	
	int nErrorCode = 0;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadNewServerMail, nMaxServerMailId);
		if (!nErrorCode)
		{
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			int nRowCount = m_pSQLConnection->GetRowCount();
			if (nRowCount == 0)
			{
				m_pSQLConnection->ResetQuery();
				return;
			}
			INT_PTR packetCount = (nRowCount-1)/MAIL_DATA_SEND_COUNT_EACH_TIME+1;
			INT_PTR nNeedSendCount =0;
			int nValue =0;
			for (INT_PTR curPacketCount =1; curPacketCount <= packetCount; curPacketCount++)
			{
				if (curPacketCount* MAIL_DATA_SEND_COUNT_EACH_TIME > nRowCount)
				{
					nNeedSendCount =  nRowCount - (curPacketCount-1)*MAIL_DATA_SEND_COUNT_EACH_TIME;
				}
				else
				{
					nNeedSendCount = MAIL_DATA_SEND_COUNT_EACH_TIME;
				}
				CDataPacket & outPacket = allocProtoPacket(dcLoadNewServerMail);
				outPacket << nServerId << nServerId <<nErrorCode;
				outPacket << (BYTE)LoadDbTypeServerMail<<(WORD)nNeedSendCount;//这个包里有多少邮件
				for (INT_PTR i = (curPacketCount-1)* MAIL_DATA_SEND_COUNT_EACH_TIME; i < curPacketCount* MAIL_DATA_SEND_COUNT_EACH_TIME && i < nRowCount; i++)
				{
					if (pRow)
					{
						// pRow[0]?outPacket << (int)atoi(pRow[0]):outPacket <<(int)0;//nServerMailId
						int nServerMailId = 0;
						if(pRow[0])
						{
							sscanf(pRow[0], "%d", &nServerMailId);//gid
						}
						outPacket << nServerMailId;
						// pRow[1]?outPacket << (unsigned int)atoi(pRow[1]):outPacket <<(unsigned int)0;//nActorId
						unsigned int nActorId = 0;
						if(pRow[1])
						{
							sscanf(pRow[1], "%u", &nActorId);//gid
						}
						outPacket << nActorId;
						// pRow[2]?outPacket.writeString(pRow[2]) :outPacket.writeString("");//sTitle
						char sTitle[1024] = "0";
						if(pRow[2])
						{
							sscanf(pRow[2], "%s", &sTitle);//gid
						}
						outPacket.writeString(sTitle);
						pRow[3]?outPacket.writeString(pRow[3]) :outPacket.writeString("");//sContent
						// char sContent[1024] = "0";
						// if(pRow[3])
						// {
						// 	sscanf(pRow[3], "%s", &sContent);//gid
						// }
						// printf("content:%s",sContent);
						// outPacket.writeString(sContent);
						
						// pRow[4]?outPacket << (int)atoi(pRow[4]):outPacket <<(int)0;//
						int nMinVipLevel = 0;
						if(pRow[4])
						{
							sscanf(pRow[4], "%d", &nMinVipLevel);//nMinVipLevel
						}
						outPacket << nMinVipLevel;
						// pRow[5]?outPacket << (int)atoi(pRow[5]):outPacket <<(int)0;//
						int nMaxVipLevel = 0;
						if(pRow[5])
						{
							sscanf(pRow[5], "%d", &nMaxVipLevel);//gid
						}
						outPacket << nMaxVipLevel;
						// pRow[6]?outPacket << (int)atoi(pRow[6]):outPacket <<(int)0;//nMinLevel
						int nMinLevel = 0;
						if(pRow[6])
						{
							sscanf(pRow[6], "%d", &nMinLevel);//gid
						}
						outPacket << nMinLevel;
						// pRow[7]?outPacket << (int)atoi(pRow[7]):outPacket <<(int)0;//nMaxLevel
						int nMaxLevel = 0;
						if(pRow[7])
						{
							sscanf(pRow[7], "%d", &nMaxLevel);//gid
						}
						outPacket << nMaxLevel;
						// pRow[8]?outPacket << (unsigned int)atoi(pRow[8]):outPacket <<(unsigned int)0;//nMinRegisterTime
						unsigned int nMinRegisterTime = 0;
						if(pRow[8])
						{
							sscanf(pRow[8], "%u", &nMinRegisterTime);//gid
						}
						outPacket << nMinRegisterTime;
						// pRow[9]?outPacket << (unsigned int)atoi(pRow[9]):outPacket <<(unsigned int)0;//nMaxRegisterTime
						unsigned int nMaxRegisterTime = 0;
						if(pRow[9])
						{
							sscanf(pRow[9], "%u", &nMaxRegisterTime);//gid
						}
						outPacket << nMaxRegisterTime;
						// pRow[10]?outPacket << (unsigned int)atoi(pRow[10]):outPacket <<(unsigned int)0;//nMinLoginTime
						unsigned int nMinLoginTime = 0;
						if(pRow[10])
						{
							sscanf(pRow[10], "%u", &nMinLoginTime);//gid
						}
						outPacket << nMinLoginTime;
						// pRow[11]?outPacket << (unsigned int)atoi(pRow[11]):outPacket <<(unsigned int)0;//nMaxLoginTime
						unsigned int nMaxLoginTime = 0;
						if(pRow[11])
						{
							sscanf(pRow[11], "%u", &nMaxLoginTime);//gid
						}
						outPacket << nMaxLoginTime;
						// pRow[12]?outPacket << (BYTE)atoi(pRow[12]):outPacket <<(BYTE)0;//btOnline
						int btOnline = 0;
						if(pRow[12])
						{
							sscanf(pRow[12], "%d", &btOnline);//gid
						}
						outPacket << btOnline;
						// pRow[13]?outPacket << (char)atoi(pRow[13]):outPacket <<(char)0;//btSex
						int btSex = 0;
						if(pRow[13])
						{
							sscanf(pRow[13], "%d", &btSex);//gid
						}
						outPacket << btSex;
						// pRow[14]?outPacket << (unsigned int)atoi(pRow[14]):outPacket <<(unsigned int)0;//way
						int way = 0;
						if(pRow[14])
						{
							sscanf(pRow[14], "%d", &way);//gid
						}
						outPacket << way;
						pRow = m_pSQLConnection->NextRow();
					}
				}
				flushProtoPacket(outPacket);
			}	
			m_pSQLConnection->ResetQuery();

			//读附件
			nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadNewServerMailAttach, nMaxServerMailId);
			if (!nErrorCode)
			{
				MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
				int nRowCount = m_pSQLConnection->GetRowCount();
				INT_PTR packetCount = (nRowCount-1)/MAILATTACH_DATA_SEND_COUNT_EACH_TIME+1;
				INT_PTR nNeedSendCount =0;
				int nValue =0;
				for (INT_PTR curPacketCount =1; curPacketCount <= packetCount; curPacketCount++)
				{
					if (curPacketCount* MAILATTACH_DATA_SEND_COUNT_EACH_TIME > nRowCount)
					{
						nNeedSendCount =  nRowCount - (curPacketCount-1)*MAILATTACH_DATA_SEND_COUNT_EACH_TIME;
					}
					else
					{
						nNeedSendCount = MAILATTACH_DATA_SEND_COUNT_EACH_TIME;
					}
					CDataPacket & outPacket = allocProtoPacket(dcLoadNewServerMail);
					outPacket << nServerId << nServerId <<nErrorCode;
					outPacket << (BYTE)LoadDbTypeServerMailAttach<<(WORD)nNeedSendCount;//这个包里有多少附件
					for (INT_PTR i = (curPacketCount-1)* MAILATTACH_DATA_SEND_COUNT_EACH_TIME; i < curPacketCount* MAILATTACH_DATA_SEND_COUNT_EACH_TIME && i < nRowCount; i++)
					{
						if (pRow)
						{

							pRow[0]?outPacket << (int)atoi(pRow[0]):outPacket <<(int)0;//nServerMailId
							pRow[1]?outPacket << (BYTE)atoi(pRow[1]):outPacket <<(BYTE)0;//btType
							pRow[2]?outPacket << (WORD)atoi(pRow[2]):outPacket <<(WORD)0;//wId
							// pRow[3]?outPacket << (BYTE)atoi(pRow[3]):outPacket <<(BYTE)0;//btQuality
							pRow[4]?outPacket << (int)atoi(pRow[4]):outPacket <<(int)0;//数量int，支持了资源数量超过word
							// pRow[5]?outPacket << (BYTE)atoi(pRow[5]):outPacket <<(BYTE)0;//btBind
							// pRow[6]?outPacket << (WORD)atoi(pRow[6]):outPacket <<(WORD)0;//wStar
							// pRow[7]?outPacket << (BYTE)atoi(pRow[7]):outPacket <<(BYTE)0;//btStarLost
							// pRow[8]?outPacket << (BYTE)atoi(pRow[8]):outPacket <<(BYTE)0;//btInscriptlevel
							// pRow[9]?outPacket << (int)atoi(pRow[9]):outPacket <<(int)0;//nIncriptExp
							// pRow[10]?outPacket << (BYTE)atoi(pRow[10]):outPacket <<(BYTE)0;//btLuck
							// pRow[11]?outPacket << (int)atoi(pRow[11]):outPacket <<(int)0;//smith1
							// pRow[12]?outPacket << (int)atoi(pRow[12]):outPacket <<(int)0;//smith2
							// pRow[13]?outPacket << (int)atoi(pRow[13]):outPacket <<(int)0;//smith3
							// pRow[14]?outPacket << (int)atoi(pRow[14]):outPacket <<(int)0;//smith4
							// pRow[15]?outPacket << (int)atoi(pRow[15]):outPacket <<(int)0;//smith5
							// pRow[16]?outPacket << (WORD)atoi(pRow[16]):outPacket <<(WORD)0;//identifyslotnum
							// pRow[17]?outPacket << (WORD)atoi(pRow[17]):outPacket <<(WORD)0;//identifynum
							pRow = m_pSQLConnection->NextRow();
						}
					}
					flushProtoPacket(outPacket);
				}	
				m_pSQLConnection->ResetQuery();
				//再发一个包结束
				CDataPacket & outPacket = allocProtoPacket(dcLoadNewServerMail);
				outPacket << nServerId << nServerId <<nErrorCode;
				outPacket << (BYTE)LoadDbTypeServerMailComplete;
				flushProtoPacket(outPacket);
			}
			else
			{
				OutputMsg(rmError,"szSQLSP_LoadNewServerMail Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
			}
		}
		else
		{
			OutputMsg(rmError,"szSQLSP_LoadNewServerMail Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
		}
		
	}
}
void CLogicDBReqestHandler::SaveActorMail(CDataPacketReader & inPacket, bool bResp)
{
	unsigned int nActorId = 0, nSrcId = 0, nCreateDt = 0;
	byte btState = 0, btIsDel = 0;
	int nServerId = 0, nRawServerId = 0;
	BYTE nCurPacketCount=0, nPacketCount = 0, nPacketType=0, nDelType=0;
	INT64 nMailId = 0;
	inPacket >> nRawServerId >> nServerId >> nActorId ;
	inPacket >> nCurPacketCount >> nPacketType >> nDelType;
	//nPacketTypee :eMailSaveType
	//nDelType: eMailSaveDelType
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		if (nCurPacketCount == 1 && nPacketType == 0 && nDelType == 0)
		{
			nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteMail, nActorId);//现在只删除isdel=0的邮件,因为loadmail只读了isdel=0,。
			if(!nErrorCode)
			{
				m_pSQLConnection->ResetQuery(); 
			}
		}
		if (nPacketType == 0)//每个包单独执行保存
		{
			BYTE nMailCount = 0;
			inPacket >> nMailCount;
			if (nMailCount > 0)
			{
				char sTitle[200], sContent[600], sBuff[1024];
				sprintf(m_pHugeSQLBuffer, szSQLSP_SaveMail);
				for (int i = 0; i < nMailCount; i++)
				{
					strcat(m_pHugeSQLBuffer, "(");
					inPacket >> nMailId >> nSrcId;
					inPacket.readString(sTitle, ArrayCount(sTitle));
					inPacket.readString(sContent, ArrayCount(sContent));
					inPacket >> nCreateDt >> btState >> btIsDel;
					char sRealContent[700];
					mysql_escape_string(sRealContent,sContent,(unsigned long)strlen(sContent));
					sprintf(sBuff,"%lld,%u,%u,\"%s\",\"%s\",%d,%d,%d)", nMailId,nActorId,nSrcId,sTitle,sRealContent,nCreateDt,btState, btIsDel);
					strcat(m_pHugeSQLBuffer, sBuff);
					if (i < nMailCount -1)
					{			
						strcat(m_pHugeSQLBuffer, ",");
					}
				}
				// printf("%s\n", m_pHugeSQLBuffer);
				nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if (!nErrorCode)
				{
					m_pSQLConnection->ResetQuery();
				}
			}
		}
		else if (nPacketType == 1)
		{
			WORD nMailAttachCount = 0;
			inPacket >> nMailAttachCount;
			if (nMailAttachCount > 0)
			{
				sprintf(m_pHugeSQLBuffer, szSQLSP_SaveMailAttach);
				int nValue = 0;
				BYTE nType = 0;
				char  sBuff[1024];
				for (int i = 0; i <nMailAttachCount ; i++)
				{
					CUserItem data;
					inPacket >> nMailId >> nType;
					data << inPacket;

					strcat(m_pHugeSQLBuffer, "(");
					sprintf(sBuff,"%lld,%u,%d,%lld,", nMailId,nActorId,nType,data.series.llId); //mailid actotID,guid,type
					strcat(m_pHugeSQLBuffer,sBuff);

					//ItemId,QUALITY,strong
					nValue = MAKELONG( data.wItemId, MAKEWORD( data.btQuality, data.btStrong));

					sprintf(sBuff,"%d,%d,",nValue, data.wCount);//MAKELONG(data.wDura,data.wCount)); 
					strcat(m_pHugeSQLBuffer,sBuff);

					//wPackageType,btFlag,wHoles
					nValue = MAKELONG( MAKEWORD( data.wPackageType, data.btFlag), 0);
					sprintf(sBuff," %d, ",nValue);
					strcat(m_pHugeSQLBuffer,sBuff);

					//BYTE	bLostStar;BYTE	bInSourceType;//铭刻等级WORD    bStrongResereLevel;//保留(可使用)
					nValue = MAKELONG( MAKEWORD( data.bLostStar, data.bInSourceType), data.wIdentifySlotNum); //bStrongResereLevel->wIdentifySlotNum
					
					sprintf(sBuff,"%d,",nValue);
					strcat(m_pHugeSQLBuffer,sBuff);

					//data reserver 
					//MAKELONG(A,B) 这里A是低字节，B是高字节，类似 MAKEWORD
					nValue = MAKELONG( data.nDropMonsterId, data.wStar );			//btSmithCount -> nDropMonsterId

					sprintf(sBuff,"%d,%d,",data.nCreatetime.tv,nValue); 

					strcat(m_pHugeSQLBuffer,sBuff);
					nValue= MAKELONG( MAKEWORD(data.btSharp, data.btDeportId), MAKEWORD(data.btHandPos, data.btLuck)); 

					sprintf(sBuff,"%d,%d,%d,%d,%d,%d,%d)",data.smithAttrs[0],data.smithAttrs[1],data.smithAttrs[2],data.smithAttrs[3],data.smithAttrs[4],nValue,data.nAreaId); 
					strcat(m_pHugeSQLBuffer,sBuff);
					if ((i>0 && i%200 == 0) || i == nMailAttachCount-1)
					{
						nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
						if(!nErrorCode)  
						{
							m_pSQLConnection->ResetQuery(); 
							strcpy(m_pHugeSQLBuffer,szSQLSP_SaveMailAttach);
						}
						else
						{
							OutputMsg(rmError,"SaveActorMailAttach Error,errorCode=%d,sql=%s", nErrorCode,m_pHugeSQLBuffer);
						}
					}
					else if(i < nMailAttachCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}

				}
			}
			
		}		
	}
	
}
