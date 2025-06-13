
#include "StdAfx.h"
#include "SessionDataHandle.h"


// Table of CRC-32's of all single-byte values (made by make_crc_table)
const unsigned int crc_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

const unsigned int * get_crc_table()
{ return (const unsigned int *)crc_table;
}

#define CRC_DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define CRC_DO2(buf)  CRC_DO1(buf); CRC_DO1(buf);
#define CRC_DO4(buf)  CRC_DO2(buf); CRC_DO2(buf);
#define CRC_DO8(buf)  CRC_DO4(buf); CRC_DO4(buf);

unsigned int ucrc32(unsigned int crc, const char *buf, unsigned int len)
{ if (buf == 0) return 0L;
crc = crc ^ 0xffffffffL;
while (len >= 8)  {CRC_DO8(buf); len -= 8;}
if (len) do {CRC_DO1(buf);} while (--len);
return crc ^ 0xffffffffL;
}



using namespace jxInterSrvComm::SessionServerProto;

const CSessionDataHandle::OnHandleSockPacket CSessionDataHandle::SSSockPacketHandlers[] = 
{
	&CSessionDataHandle::CatchDefaultPacket,
	&CSessionDataHandle::CatchQuerySessionResult,
	&CSessionDataHandle::CatchCloseSession,
	&CSessionDataHandle::CatchUpdateSession,
	&CSessionDataHandle::CatchOnlineCount,
	&CSessionDataHandle::CatchLoadUserItem,
	&CSessionDataHandle::CatchGetUserItem,
	&CSessionDataHandle::CatchRemoveUserItem,
	&CSessionDataHandle::CatchAddValueCard,
	&CSessionDataHandle::CatchQueryAddValueCard,
	&CSessionDataHandle::CatchQueryYuanbaoCount,//10
	&CSessionDataHandle::CatchWithdrawYuanbao,
	&CSessionDataHandle::CatchConformOpenSession, //确认收到了session的请求
	&CSessionDataHandle::CatchRequestTransmit,		//跨服请求传送
	&CSessionDataHandle::CatchGroupMessage,	
	&CSessionDataHandle::CatchBroadcastMessage,
	&CSessionDataHandle::CatchSaveCsRank,
	&CSessionDataHandle::CatchLoadCsRank,
	&CSessionDataHandle::CatchClearCsRank,
	&CSessionDataHandle::CatchCheckUserPassword,
	&CSessionDataHandle::CatchCreateAccount,//20
	&CSessionDataHandle::SetCommonServerId,  //设置中心服务器
	&CSessionDataHandle::CatGetContrlCenterMsg,  //收到控制后台的消息
	&CSessionDataHandle::CatQQMsg,  //腾讯平台有关的消息
	&CSessionDataHandle::CatSendCommonPlatformData,
	&CSessionDataHandle::CatRetranToCommonPlatform,		//请求传送到跨平台
	
};




CSessionDataHandle::CSessionDataHandle()
{
	//SetClientSocket(nSocket, pAddrIn);
	m_nCommServerId = 0; //设置自身的服务器ID和公共服务器的ID
	m_nSelfServerId =0;  
	m_nUserCount = 0;
	m_sTrServerIndex[0] =0;

	m_nUserLimit = 10000;
	m_nMaxUserCount = 0;
	m_dwDisconnectedTick = 0;
	m_pSSServer = 0;
	m_pSQLConnection = 0;

	m_pForwardDataBuff = NULL;	
	m_pHugeSQLBuffer = new char[dwHugeSQLBufferSize];
	//m_bIsCrossSession =isCrossSession;  //是否是
}


CSessionDataHandle::~CSessionDataHandle()
{
	//m_Allocator.FreeBuffer(m_pHugeSQLBuffer);
	if(m_pHugeSQLBuffer)
	{
		delete []m_pHugeSQLBuffer;
		m_pHugeSQLBuffer = NULL;
	}
	if(m_pForwardDataBuff)
	{
		delete []m_pForwardDataBuff;
		m_pForwardDataBuff =0;
	}
}

void  CSessionDataHandle::SetParam(CSessionServer *pSessionSrv,CSQLConenction *pSqlConn, bool isSessonConnect  )
{
	m_bIsCrossSession =isSessonConnect;  //是否是
	m_pSSServer = pSessionSrv;
	m_pSQLConnection = pSqlConn;
}


VOID CSessionDataHandle::CatchDefaultPacket(CDataPacketReader &inPacket)
{

}


VOID CSessionDataHandle::CatchConformOpenSession(CDataPacketReader &inPacket)
{
	//确定收到了opensession的数据包
	int nAccountid=0;
	int nServerIndex=0;
	inPacket >> nAccountid >> nServerIndex; //收到了确认包
	
	//需要传递给网关
	/*
	if (m_pSSServer)
	{
		m_pSSServer->GetSSManager()->getGateManager()->PostInternalMessage(GTIM_CONFIRM_OPEN_SESSION,nAccountid,nServerIndex,0);
	}
	*/

}

VOID CSessionDataHandle::CatchRequestTransmit(CDataPacketReader &inPacket)
{
	unsigned int nAccountId = 0, nActorId = 0, nDestServerId = 0, nType = 0;
	inPacket >> nAccountId >> nActorId >> nDestServerId >> nType;
	// 查询目标服务器是否连接好（通过SessionServer来查询）
	bool bCanTransmit = false;
	if (m_pSSServer)
	{
		bCanTransmit = m_pSSServer->UserCanLoginToServer(nDestServerId) == 0 ? true : false;
	}

	// 标准的格式：actorId + errorcode 
	CDataPacket &packet = AllocDataPacket(SessionServerProto::sReqestRransmitAck);
	packet << nActorId << (BYTE)SessionServerProto::neSuccess;
	packet << nAccountId << nDestServerId << bCanTransmit << nType;
	FlushDataPacket(packet);
}

VOID CSessionDataHandle::CatchGroupMessage(CDataPacketReader &inPacket)
{
	if (!m_pSSServer)
		return;

	int nCommServerId	= GetCommonServerId(); //公共服的id
	
	CSSManager *pSSMgr	= m_pSSServer->GetSSManager();
	//CBaseList<int> clientList;
	//pSSMgr->GetClientLogicServerList(nCommServerId, clientList);
	//if (clientList.count() > 0)
	//暂时先开启来
	{
		if (!m_pForwardDataBuff)
		{
			m_pForwardDataBuff = (char *) new char [MaxForwardLogicDataSize];
		}

		const size_t headerSize = sizeof(DATAHEADER);
		size_t nPacketSize = inPacket.getAvaliableLength();
		size_t nTotalSize = headerSize + sizeof(WORD) + nPacketSize;
		if (nTotalSize > MaxForwardLogicDataSize)
		{
			OutputMsg(rmError, _T("too large data packet,size=%d"), nTotalSize);
			return;
		}
		PDATAHEADER pHeader = (PDATAHEADER)m_pForwardDataBuff;
		ZeroMemory(pHeader, headerSize);		
		pHeader->tag = DEFAULT_TAG_VALUE;		
		pHeader->len = nTotalSize - headerSize;
		char *pData = (char *)(pHeader+1);
		CDataPacket packet(pData, MaxForwardLogicDataSize-headerSize);
		packet << (WORD)SessionServerProto::sRecvGroupMessage;
		packet.writeBuf(inPacket.getOffsetPtr(), nPacketSize);		
		m_pSSServer->SendGroupLogicClientMsg(nCommServerId, m_pForwardDataBuff, nTotalSize);
	}
}

//关闭连接
VOID CSessionDataHandle::CatchCloseSession(CDataPacketReader &inPacket)
{
	unsigned int nSessionID =0;
	int nOnlineTime =0;
	
	//关闭会话
	UINT64 lKey =0; 
	inPacket >>nSessionID  >> lKey;
	if (nSessionID ==0) return;
	if (m_pSSServer)
	{
		m_pSSServer->PostCloseSessionByAccountId(nSessionID,lKey);
	}

}
//更新状态
VOID CSessionDataHandle::CatchUpdateSession(CDataPacketReader &inPacket)
{
	int nSessionID =0;
	int nState =0;
	inPacket >> nSessionID;
	inPacket >> nState;
	if (nSessionID ==0) return;
	if (m_pSSServer)
	{
		m_pSSServer->PostInternalMessage(SSIM_CHANGE_SESSION_STATE,nSessionID,nState,0);
	}
}

//返回查询在线状态的结果
VOID CSessionDataHandle::CatchQuerySessionResult(CDataPacketReader &inPacket)
{
	int nSessionID =0;
	BYTE nResult =0;
	inPacket >> nSessionID;
	inPacket >> nResult;
	if (nSessionID ==0) return;
	if (m_pSSServer)
	{
		m_pSSServer->PostInternalMessage(SSIM_CONFIM_SESSION_RESULT,nSessionID,nResult,0);
		//OutputMsg(rmTip,_T("recv client msg: query Session result . serverindex=%d,name=%s"),getClientServerIndex(),getClientName());
	}
}


VOID CSessionDataHandle::OnPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if ( nCmd >= ArrayCount(SSSockPacketHandlers) )
	{
		OutputMsg(rmWaning, _T("SessionClient has recv invalid packet cmd=%d"), nCmd);
	}
	else
	{
		(this->*SSSockPacketHandlers[nCmd])(inPacket);
	}
}



VOID CSessionDataHandle::CatchOnlineCount( CDataPacketReader &inPacket )
{
	int nCount = 0;
	inPacket >> nCount;
	m_nUserCount = nCount;//直接覆盖原来的值
	//OutputMsg(rmTip,_T("Online Player Count=%d,ServerIndex=%d"),nCount,getClientServerIndex());
	if (m_nUserCount > m_nMaxUserCount)
	{
		m_nMaxUserCount = m_nUserCount;//记录最大值
	}
}

VOID CSessionDataHandle::CatchLoadUserItem( CDataPacketReader &inPacket )
{
	unsigned int nAccountId = 0;
	unsigned int nActorId = 0;
	inPacket >> nAccountId;
	inPacket >> nActorId;

	CDataPacket& out = AllocDataPacket(sUserItemList);
	out << nActorId;
	
	if(m_bIsCrossSession)
	{
		
		out << (BYTE)neSuccess;
		out << (int)0;
		FlushDataPacket(out);
		return ;
	}

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)neSSErr;
	}
	else
	{
		char buff[128];
		sprintf(buff,szSessionLoadUserItem,nAccountId,nActorId,GetServerId());

		int nError = m_pSQLConnection->RealQuery(buff,strlen(buff));
		if ( !nError )
		{
			out << (BYTE)neSuccess;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << (int)nCount;
			while ( pRow)
			{
				INT64 id;
				id = StrToInt64(pRow[0]);
				int nItemId = StrToInt(pRow[1]);
				out << (INT64)id;
				out << (int)nItemId;
				int value = StrToInt(pRow[2]);
				out << (BYTE)value;	//bind
				value = StrToInt(pRow[3]);
				out << (BYTE)value;	//strong
				value = StrToInt(pRow[4]);
				out << (BYTE)value;	//quality
				value = StrToInt(pRow[5]);
				out << (int)value;	//itemcount
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			out << (BYTE)neSSErr;
		}

	}
	FlushDataPacket(out);
}

VOID CSessionDataHandle::CatchGetUserItem( CDataPacketReader &inPacket )
{
	//如果是跨服中心服
	

	unsigned int nAccountId = 0;
	unsigned int nActorId = 0;
	INT64 nId = 0;
	inPacket >> nAccountId;
	inPacket >> nActorId;
	inPacket >> nId;

	CDataPacket& out = AllocDataPacket(sGetUserItem);
	out << nActorId;

	if(m_bIsCrossSession)
	{
		
		out << (BYTE)neSuccess;
		out << (INT64)nId;
		out << (INT64)neSSErr;
		FlushDataPacket(out);
		return ;
	}

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)neSSErr;
		FlushDataPacket(out);
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSessionGetUserItem,nAccountId,nActorId,nId,GetServerId());
		if ( !nError )
		{
			out << (BYTE)neSuccess;
			out << (INT64)nId;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();			
			if ( pRow)
			{
				int nItemId = StrToInt(pRow[0]);//如果没数据，这个数值是0
				out << (int)nItemId;
				int value = StrToInt(pRow[1]);
				out << (BYTE)value;	//bind
				value = StrToInt(pRow[2]);
				out << (BYTE)value;	//nstrong
				value = StrToInt(pRow[3]);
				out << (BYTE)value;	//nquality
				value = StrToInt(pRow[4]);
				out << (int)value;	//ncount
				if (pRow[5])
				{
					out.writeString(pRow[5]);
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
			out << (BYTE)neSSErr;
		}

	}
	FlushDataPacket(out);
}

VOID CSessionDataHandle::CatchRemoveUserItem( CDataPacketReader &inPacket )
{
	unsigned int nAccountId = 0;
	unsigned int nActorId = 0;
	INT64 nId = 0;
	inPacket >> nActorId;
	inPacket >> nId;

	CDataPacket& out = AllocDataPacket(sDeleteUserItem);
	out << nActorId;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)neSSErr;
	}
	else
	{
		int nError = m_pSQLConnection->Exec(szSessionDelUserItem,nId);
		if ( !nError )
		{
			out << (BYTE)neSuccess;
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			out << (BYTE)neSSErr;
		}
	}
	FlushDataPacket(out);
}



//新手卡的生成算法计算hashkey，hashkey=  md5(用户的账户,卡的类型(1)，服务器id,spid(字符串), md5(spid(字符串,spguid)))
//取前面16位的字符串=L1，取后面16位的字符串得到L2
//计算Crc32(L1)，不够10个字母的在后面补0，得到R1， 计算Crc32(L2)，得到R2
//结果取( R1,R2 )中间取18位字符串，得到结果为18位的长整形的数字

int  CSessionDataHandle::isInvalidNewUserCard(unsigned int nAccountId, UINT64 lCard,char * sInAccountName  )
{
	
	
	UINT64 lMin = 100000000000000000L; //18位
	UINT64 lMax = 999999999999999999L; //18位
	
	//不在这个范围里
	if(lCard <lMin || lCard > lMax) return -2;
	
	if( !m_pSSServer->GetSSManager()->GetCardMd5() )
	{
		OutputMsg(rmTip,"auto card fail,please set crccard=1 to config");
		return -2;
	}
	char *pAccountName =NULL;
	if(sInAccountName != NULL && sInAccountName[0] !=0)
	{
		pAccountName = sInAccountName;
	}
	else
	{
		PGAMESESSION pSessionPtr = m_pSSServer->GetSessionPtr(nAccountId, NULL,Uint64(-1),-1);
		if(pSessionPtr ==NULL)
		{
			OutputMsg(rmTip,"no accountid=%u where draw card",nAccountId);
			return -2;
		}
		pAccountName = pSessionPtr->sAccount;
	}
	
	
	int nServerId = GetServerId(); //获取卡的ID
	char sResult[512];
	//char number[20];
	
	sResult[0] =0;
	strncpy(sResult, pAccountName,sizeof(sResult));
	
	//卡的类型
	//sprintf(number,"%d",nCardType);
	strncat(sResult, "1",sizeof(sResult));   //卡的类型

	strncat(sResult, m_sTrServerIndex,sizeof(sResult));  //服务器的ID
	
	
	//获取spid
	strncat(sResult,  m_pSSServer->GetSSManager()->GetSPID() ,sizeof(sResult));  //服务器的ID
	
	//把spid的md5放进去
	strncat(sResult,  (char *)m_pSSServer->GetSSManager()->GetSpidMd5() ,sizeof(sResult));  
	

	unsigned char mdkey[64];
	MD5_CTX   ctx; 
	//使用 MD5加密玩家的密码
	MD5Init(&ctx); 
	MD5Update(&ctx,  (unsigned char *)sResult,   (unsigned int)strlen(sResult) );
	MD5Final(mdkey,&ctx);
	mdkey[32]=0;

	
	

	
	unsigned int L1 = ucrc32(0L, 0, 0);  //初始化crc

	//计算并对比缓存文件整个数据段的CRC32值
	L1 = ucrc32(L1,(const  char*)mdkey , 16);

	unsigned int L2 =  ucrc32(0L, 0, 0);
	L2 = ucrc32(L2,(const  char*)&mdkey[16] , 16);
	
	
	
	char sL1[20];  //分别计算下这个的字符串
	sL1[0] =0;

	char sL2[20];  
	sL2[0] =0;

	sprintf(sL1,"%u",L1);
	sprintf(sL2,"%u",L2);

	

	//低位补0
	size_t nLen1=strlen(sL1);
	for(size_t i=nLen1 ; i<10; i++ )
	{
		sL1[i] ='0';
	}
	sL1[10] =0;
	
	//在低位补0
	size_t nLen2= strlen(sL2);
	for(size_t i=nLen2 ; i<10; i++ )
	{
		sL2[i] ='0';
	}
	sL2[10] =0;

	//这是个18位的字符串，然后要转换成整数
	char sR[32];
	strcpy(sR,sL1);
	strncat(sR,sL2,8); //写入
	
	char sInput[64];
	sprintf(sInput,"%lld",lCard);
	
	bool bResult= (strcmp(sInput,sR) ==0);
	
	if(bResult)
	{
		//OutputMsg(rmTip,"ok result=%s------mdkey=%s,crc1=%u,crc2=%u,sL1=%s,sL2=%s,input=%s,local=%s",sResult,mdkey,L1,L2,sL1,sL2,sInput,sR);
		return 0;
	}
	else
	{
		OutputMsg(rmTip,"fail,result=%s------mdkey=%s,crc1=%u,crc2=%u,sL1=%s,sL2=%s,input=%s,local=%s",sResult,mdkey,L1,L2,sL1,sL2,sInput,sR);
		return -1;
	}

}

VOID CSessionDataHandle::CatchAddValueCard( CDataPacketReader &inPacket )
{
	unsigned int nAcccountId = 0;
	unsigned int nActorId = 0;
	UINT64 nSN = 0;
	char szActorName[64] = {0};
	inPacket >> nAcccountId >> nActorId >> nSN;
	inPacket.readString(szActorName, ArrayCount(szActorName));  // uid | cid | sn | actorname	
	
	char szAccountName[64] = {0};
	inPacket.readString(szAccountName, ArrayCount(szAccountName));

	CDataPacket& out = AllocDataPacket(sAddValueCard);
	out << nActorId;
	
	int nNewUserCard = isInvalidNewUserCard(nAcccountId,nSN,szAccountName);
	if(nNewUserCard  ==0) //如果表示是
	{
		out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeSucc;
		out << nSN << (int)1 << (int)1;
		char szDateTime[32] = {0};

		out.writeString(szDateTime);
		FlushDataPacket(out);
		return ;
	}
	else if(nNewUserCard == -1) //找不到这个卡
	{
		OutputMsg(rmTip,"000 card fail,%lld",nSN);
		out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeNoCardInfo << nSN;	
		FlushDataPacket(out);
		return ;
	}
	
	char buff[512];

	
	if (!m_pSQLConnection->Connect())
	{
		out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeDBErr << nSN;
	}
	else
	{
		
		if(nSN  <= 0xFFFFFFFE ) //如果是1个unsigned的数字，这里监测下是不是新手卡
		{
			
		}
		sprintf(buff,szGetSASeriesInfo,nSN);

		int nError = m_pSQLConnection->RealQuery(buff,strlen(buff));
		if (!nError)
		{			
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			bool bExist = false;
			int nType = 0, nSubType = 0;
			char szDateTime[32] = {0};
			if (pRow)
			{			
				bExist = true;
				if(pRow[1] !=NULL)
				{
					nType = StrToInt(pRow[1]);
					if(pRow[2])
					{
						nSubType = StrToInt(pRow[2]);
					}
					if (pRow[3])
						strcpy(szDateTime, pRow[3]);
				}
				else
				{
					OutputMsg(rmError,_T("%s,卡%lld没初始化"),__FUNCTION__,nSN);
					bExist = false;
				}
			
			}	
			m_pSQLConnection->ResetQuery();

			if (bExist)
			{				
				sprintf(buff,szDeleteSASeries,nSN,nAcccountId,nActorId,szActorName);
				if (!m_pSQLConnection->RealExec(buff,strlen(buff)))
				{
					out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeSucc;
					out << nSN << nType << nSubType;
					out.writeString(szDateTime);
					m_pSQLConnection->ResetQuery();
				}
				else
				{
					OutputMsg(rmTip,"111 db error,card=%lld",nSN);
					out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeDBErr << nSN;		// db无此记录 
				}
			}
			else
			{
				OutputMsg(rmTip,"222 no card in db,%lld",nSN);
				out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeNoCardInfo << nSN;		// db无此记录 
			}
		}
		else
		{
			OutputMsg(rmTip,"333 exec sql error,card=%lld",nSN);
			out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeNoCardInfo << nSN;		// db无此记录 
		}
	}
	FlushDataPacket(out);
}

VOID CSessionDataHandle::CatchQueryAddValueCard(CDataPacketReader &inPacket)
{
	unsigned int nAcccountId = 0;
	unsigned int nActorId = 0;
	UINT64 nSN = 0;
	char szActorName[64] = {0};
	inPacket >> nAcccountId >> nActorId >> nSN;
	inPacket.readString(szActorName, ArrayCount(szActorName));  // uid | cid | sn | actorname	

	char szAccountName[64] = {0};
	inPacket.readString(szAccountName, ArrayCount(szAccountName));


	CDataPacket& out = AllocDataPacket(sQueryAddValueCard);
	out << nActorId;

	int nNewUserCard = isInvalidNewUserCard(nAcccountId,nSN,szAccountName);
	if(nNewUserCard  ==0) //如果表示是
	{
		out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeSucc;
		out << nSN << (int)1 << (int)0;
		char szDateTime[32] = {0};
		out.writeString(szDateTime);
		FlushDataPacket(out);
		return ;
	}
	else if(nNewUserCard == -1) //找不到这个卡
	{
		out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeNoCardInfo << nSN;	
		FlushDataPacket(out);
		return ;
	}


	if (!m_pSQLConnection->Connect())
	{
		out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeDBErr << nSN;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szGetSASeriesInfo, nSN);
		if (!nError)
		{			
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			bool bExist = false;
			int nType = 0, nSubType = 0;
			char szDateTime[32] = {0};
			if (pRow  )
			{			
				bExist = true;
				if(  pRow[1] == NULL )
				{
					OutputMsg(rmError,_T("提取的时候发现有卡%lld没初始化"),nSN);
					bExist =false;
				}
				else
				{
					bExist = true;
					nType = StrToInt(pRow[1]);
					if(pRow[2])
					{
						nSubType = StrToInt(pRow[2]);
					}
					if (pRow[3])
					{
						strcpy(szDateTime, pRow[3]);
					}
				}
				
			}	
			m_pSQLConnection->ResetQuery();

			if (bExist)
			{		
				out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeSucc;
				out << nSN << nType << nSubType;
				out.writeString(szDateTime);
			}
			else
			{
				out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeNoCardInfo << nSN;		// db无此记录 
			}
		}
		else
		{
			out << (BYTE)jxInterSrvComm::AsyncOperatorCode::soeNoCardInfo << nSN;		// db无此记录 
		}
	}
	FlushDataPacket(out);
}

void CSessionDataHandle::CatchQueryYuanbaoCount(CDataPacketReader &inPacket)
{
	if(m_bIsCrossSession) return ; //如果是公共服，则不能提取元宝
	unsigned int nAcccountId = 0, nActorId=0;
	inPacket >> nAcccountId >> nActorId;	
	INT svr_idx = GetServerId();	

	//OutputMsg(rmTip,"CatchQueryYuanbaoCount serverindex=%d,nAcccountId=%u,nActorId=%u",svr_idx,nAcccountId,nActorId);

	if (m_pSSServer)
	{
		CAMClient *pAMClient = m_pSSServer->GetSSManager()->getAMClient();
		if (pAMClient)
		{
			pAMClient->PostQueryAmount(nAcccountId, svr_idx, (INT64)nActorId);
		}
	}
}

void CSessionDataHandle::CatchWithdrawYuanbao(CDataPacketReader &inPacket)
{
	if(m_bIsCrossSession) return ;  //不能提取元宝

	unsigned int nAcccountId = 0, nActorId=0, nCount = 0;
	BYTE nLevel = 0;//0充值到账号，1充值到角色
	inPacket >> nAcccountId >> nActorId >> nCount;
	INT svr_idx = GetServerId();
	char szActorName[64] = {0};	
	inPacket.readString(szActorName, ArrayCount(szActorName)); 
	inPacket >> nLevel;
	szActorName[63]=0;

	if (m_pSSServer)
	{
		CAMClient *pAMClient = m_pSSServer->GetSSManager()->getAMClient();
		if (pAMClient)
		{
			pAMClient->PostConsume(nAcccountId, svr_idx, nCount, szActorName, (INT64)nActorId, nLevel);
			OutputMsg(rmTip,"[SessionClient->AMclient]Post draw yb, nAcccountId=%u,svr_idx=%d,nCount=%d,account=%s,actorid=%u",nAcccountId,svr_idx,nCount,szActorName, nActorId);
		}
	}	
}

void CSessionDataHandle::SendAmTaskAck(const CAMClient::TASKMSG & data)
{
	CDataPacket &pack = AllocDataPacket(sTaskAck);	
	pack << data;
	FlushDataPacket(pack);
}

void CSessionDataHandle::sendAMCMsgAck(const CAMClient::AMOPDATA & data)
{	
	CDataPacket &pack = AllocDataPacket(sQueryYuanbaoCount);	
	pack << data;
	FlushDataPacket(pack);

	OutputMsg(rmTip,"[SessionClient->logic] opType=%d,nServerId=%d,nResult=%d",(int)data.opType,(int)data.nServerId,(int)data.nResult );

}


VOID CSessionDataHandle::CatchBroadcastMessage(CDataPacketReader &inPacket)
{
	int nDestServerId = 0;
	inPacket >> nDestServerId;	//要转发到的服务器ID 0则广播到各个服务器

	if (!m_pSSServer && nDestServerId < 0)
		return;

	//暂时使用公共逻辑服务器SessionClient用于转发公共服务器消息的Buff，长度固定MaxForwardLogicDataSize 以后有必要再扩展
	if (!m_pForwardDataBuff)	
	{
		m_pForwardDataBuff = (char *)new char[MaxForwardLogicDataSize];
	}

	const size_t headerSize = sizeof(DATAHEADER);
	size_t nPacketSize = inPacket.getAvaliableLength();
	size_t nTotalSize = headerSize + sizeof(WORD) + sizeof(int) + nPacketSize;
	if (nTotalSize > MaxForwardLogicDataSize)
	{
		OutputMsg(rmError, _T("too large data packet,size=%d"), nTotalSize);
		return;
	}
	PDATAHEADER pHeader = (PDATAHEADER)m_pForwardDataBuff;
	ZeroMemory(pHeader, headerSize);		
	pHeader->tag = DEFAULT_TAG_VALUE;		
	pHeader->len = nTotalSize - headerSize;
	char *pData = (char *)(pHeader+1);
	CDataPacket packet(pData, MaxForwardLogicDataSize-headerSize);
	packet << (WORD)SessionServerProto::sOtherLogicMessage;
	packet << (int)nDestServerId;
	packet.writeBuf(inPacket.getOffsetPtr(), nPacketSize);
	if (0 == nDestServerId)
		m_pSSServer->SendAllClientMsg(GameServer, m_pForwardDataBuff, nTotalSize);
	else
		m_pSSServer->SendLogicClientMsg(nDestServerId, m_pForwardDataBuff, nTotalSize);
}


VOID CSessionDataHandle::CatchSaveCsRank(CDataPacketReader &inPacket)
{
	
	if(m_bIsCrossSession) return ;

	unsigned int nServerId = GetServerId();
	WORD nRankCount = 0;
	inPacket >> nRankCount;
	for( INT_PTR i = 0; i < nRankCount; i++) //如果有数据
	{
		unsigned int  nVocation = 0, nSex = 0, nCamp = 0, nValue = 0;

		CRankItem rankItem;
		inPacket >> rankItem.nActorid;

		inPacket.readBuf(rankItem.sCharName, sizeof(rankItem.sCharName) );
		rankItem.sCharName[sizeof(rankItem.sCharName)-1] = 0;

		inPacket >> nSex >> nVocation >> nServerId >> nValue; 

		rankItem.nKey = nVocation;
		rankItem.nValue = nValue;
		rankItem.nServerId = nServerId;
		rankItem.nProp1 = nSex;
		rankItem.nProp2 = nVocation;
		m_pSSServer->PostRankMsg(rankItem);
	}
	OutputMsg(rmTip, _T("update save csrank data Success:serverindex=%d,count=%d"), nServerId,(int)nRankCount);

	/*
	int nError = neSuccess;
	//数据库连接是否就绪
	if (!m_pSQLConnection->Connected())
	{
		nError = neSSErr;
	}
	else
	{
		nError = m_pSQLConnection->Exec(szDeleteCSRank, nServerId);
		if(!nError)
		{
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		}
		else
		{
			nError = neSSErr;
			OutputMsg(rmError, _T("delete csrank data error:serverindex=%d"), nServerId);
		}

		if(nRankCount > 0 ) //如果有数据
		{
			sprintf(m_pHugeSQLBuffer,szSaveCsRank); //表头放这里
			char buff[1024] = {0};
			unsigned int nActorId = 0, nVocation = 0, nSex = 0, nCamp = 0, nValue = 0;
			for(INT_PTR i = 0; i < nRankCount; i++)
			{
				inPacket >> nActorId;
				char sName[32] = {0};
				inPacket.readBuf(sName, sizeof(sName));
				sName[sizeof(sName)-1] = 0;
				inPacket >> nSex >> nVocation >> nServerId >> nValue; 

				sprintf(buff,
						"(%u, \"%s\", %d, %d, %d, %d)",
						nActorId, 
						sName, 
						nSex, 
						nVocation, 
						nServerId, 
						nValue); 
				strcat(m_pHugeSQLBuffer,buff);
				if (i < nRankCount - 1)
				{
					strcat(m_pHugeSQLBuffer,",");	
				}
			}

			nError = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if (!nError)
			{
				m_pSQLConnection->ResetQuery();
			}
			else
			{
				nError = neSSErr;
			}
		}
	}
	if(nError != neSuccess)
		OutputMsg(rmError, _T("update csrank data error:serverindex=%d"), nServerId);
	else
		OutputMsg(rmTip, _T("update csrank data Success:serverindex=%d"), nServerId);
	*/

}

VOID CSessionDataHandle::CatchLoadCsRank(CDataPacketReader &inPacket)
{
	CDataPacket &out = AllocDataPacket(sLoadCsRank);
	out << (BYTE)0;

	wylib::container::CBaseList<CRankItem>  itemList; //列表

	m_pSSServer->GetRankMsg(-1,itemList);
	
	int nCount =(int)itemList.count();
	out <<(int) nCount;

	for(INT_PTR i=0; i< nCount;i ++)
	{
		CRankItem *pItem = &itemList[i];
		out <<(unsigned int) pItem->nActorid;
		out.writeBuf(pItem->sCharName, sizeof(pItem->sCharName));
		out <<( int) pItem->nProp1;  //sex
		out <<( int) pItem->nProp2;  //vocation
		out <<( int) pItem->nServerId;  //serverid
		out <<( int) pItem->nValue;  //排名的值
	}
	FlushDataPacket(out);

	/*
	CDataPacket &out = AllocDataPacket(sLoadCsRank);
	int nError = neSSErr;


	//数据库连接是否就绪
	if (!m_pSQLConnection->Connected())
	{
		out << (BYTE)neSSErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szLoadCsRank);
		if ( !nError )
		{
			out << (BYTE)neSuccess;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			out << (int)nCount;
			while (pRow)
			{
				unsigned int nActorId = strtoul(pRow[0],0,10);

				out << (unsigned int)nActorId;
				char sName[32] = {0};
				sprintf(sName, "%s", pRow[1]);
				out.writeBuf(sName, sizeof(sName));
				int nSex = StrToInt(pRow[2]);
				out << (int)nSex;
				int nVocation = StrToInt(pRow[3]);
				out	<< (int)nVocation;
				int nServerId = StrToInt(pRow[4]);
				out	<< (int)nServerId;
				int	nValue = StrToInt(pRow[5]);
				out << (int)nValue;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			out << (BYTE)neSSErr;
		}

	}
	FlushDataPacket(out);
	*/


}

VOID CSessionDataHandle::CatchClearCsRank(CDataPacketReader &inPacket)
{
	int nServerId = 0;
	inPacket >> nServerId;
	/*
	int nError = neSSErr;
	//数据库连接是否就绪
	if (!m_pSQLConnection->Connected())
	{
		nError = neSSErr;
	}
	else
	{ 
		nError = m_pSQLConnection->Exec(szDeleteCSRank, nServerId);
		if(!nError)
		{
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		}
	}

	if(nError != neSuccess)
		OutputMsg(rmError, _T("delete csrank data error:serverindex=%d"), nServerId);
	else
		OutputMsg(rmTip, _T("delete csrank data Success:serverindex=%d"), nServerId);	
	*/

}


//检测用户名密码
VOID CSessionDataHandle::CatchCheckUserPassword(CDataPacketReader &reader)
{
	if(m_nSelfServerId == 0)
	{
		OutputMsg(rmTip,"CatchCheckUserPassword,serverid=%d",m_nSelfServerId);
		return;
	}
	
	BYTE len =0;
	TCHAR name[ACCOUNT_NAME_BUFF_LENGTH];
	TCHAR passwd[MAX_PASSWD_LENGTH];
	int nLoginServerId =0; // 要登陆的服务器的编号
	int nRawServerId =0;   // 要登陆的服务器的编号
	int nGateIndex =0;     //这个玩家在哪个

	UINT64 lKey =0;
	reader >> lKey ;

	unsigned long long lIp =0; //玩家登陆的ip

	reader.readString(name,sizeof(name));
	reader.readString(passwd,sizeof(passwd));
	reader >> nLoginServerId >> nRawServerId >>nGateIndex ; //要登陆的服务器，以及原始的服务器,在网关的编号
	
	reader >> lIp;
	
	TCHAR md5[128];
	TCHAR currentTime[32];

	TCHAR identity[64]; //身份证号码

	md5[0]=0;  //md5的值
	currentTime[0] =0; //当前的时间
	identity[0] =0; //身份证号码

	BYTE bCheck =0;

	reader >>bCheck;
	//if(bCheck) //如果需要
	{
		
	}

	if( reader.getAvaliableLength() >0 )
	{
		reader.readString( md5,sizeof(md5) -1); 
	}
	if( reader.getAvaliableLength() >0 )
	{
		reader.readString( currentTime,sizeof(currentTime)-1); 
	}
	if( reader.getAvaliableLength() >0)
	{
		reader.readString( identity,sizeof(identity)-1);   //读取身份证号码
	}

	/*
			enPasswdError =1,    //密码错误
			enNoAccount=2,       //没有这个账号
			enIsOnline =3,       //已经在线
			enServerBusy =4,     //服务器忙
			enServerClose =5,    //服务器没有开放 
			enSessionServerError =6 , //session服务器有问题，比如db没有连接好
			enServerNotExisting =7, //不存在这个服务器
			enFcmLimited =8 ,      //账户纳入防沉迷
			enSessionServerClose =9, //会话服务器处于关闭状态
			enDbServerClose =10,     //数据服务器处于关闭状态
			enGMLoginFailError =11,    //gm在非法的ip登陆
		*/
	GLOBALSESSIONOPENDATA sessionData;// 玩家的登陆的账户
	sessionData.nClientIPAddr = lIp;

	bool isLoginCommonSrv = (nRawServerId == nLoginServerId); //是否在普通服登陆

	bool isWhiteIpLogin =false;
	int nErrorID = CheckUserValid(name,passwd,sessionData,md5,currentTime,bCheck?true:false,isLoginCommonSrv,identity,isWhiteIpLogin,nRawServerId);
	if(nErrorID)
	{
		static char* s_erroMsg[]=
		{
			"NoError",
			"password error",
			"No such account",
			"is Real Online",
			"Server busy",
			"Server close",
			"Session Server Error",
			"Server Not exist ",
			"Fcm Limited ",
			"Session Server Closeed",
			"Db Server Close",
			"gm login in invalid ip",
			"ip forbid login",
			"ip login too many", 
			"md5 error",
			"php time is expired",
			"time  format error",
			"Account Seal",
		};

		static int  s_nErrorCount =sizeof(s_erroMsg)/ sizeof(s_erroMsg[0]);
		if(nErrorID < s_nErrorCount)
		{
			char *sIp =inet_ntoa(*((in_addr*)&lIp)); //这个是IP
			OutputMsg( rmWaning, _T("CheckUserValid name=%s,pw=%s，errorcode=%d,errormsg=%s,serverid=%d,ip=%s"),name, passwd,nErrorID,s_erroMsg[nErrorID],nLoginServerId,sIp);
			
		
			if(( nErrorID ==enIpMd5Error || nErrorID == enSignOutofDate || nErrorID == enTimeFormatError)&&
				md5[0] != 0 && currentTime[0] !=0)
			{
				OutputMsg(rmWaning, _T("md5=%s,time=%s"),md5,currentTime );
			}
		}
		else
		{
			OutputMsg( rmWaning, _T("CheckUserValid name=%s,pw=%s，errorcode=%d,serverid=%d"),name, passwd,nErrorID,nLoginServerId);
			
		}
		
		
		//pSSGateUser->nUserState = CSSGateUser::guLoginFail;//登陆失败
		//ResponseUserLoginState(pSSGateUser,nErrorID); //如果不成功，可以告诉客户端现在的错误码了
		
		//如果失败了，则向客户端发一个消息，登陆失败了
		CDataPacket &out = AllocDataPacket(sCheckPasswdResult);
		out << int (nGateIndex); //1字节的结果
		out << (Uint64)lKey;
		out << BYTE (nErrorID); //1字节的结果
		//out << BYTE (isWhiteIpLogin ?1:0);

		FlushDataPacket(out);

		//如果登陆的IP特别多，则进行自动封号和封账户处理
		if(nErrorID == enIpTooManyConnect )
		{
			m_pSSServer->ForbidIpAndSealAll(lIp,false);
		}
		else if( nErrorID ==enAccountSeal)
		{
			if(m_pSSServer->GetSSManager()->GetSealAccountSealIp())
			{
				m_pSSServer->ForbidIpAndSealAll(lIp,true);
			}
			else
			{
				m_pSSServer->ForbidIpAndSealAll(lIp,false);
			}
			
		}

	}
	else //成功了要做进一步的检测,检测是否已经登录过了，等等，这里可以做顶号的处理
	{
		sessionData.nServerIndex = nLoginServerId; //当前登陆的服务器id

		GAMESESSION cuRuentSession;
		
		//如果当前存在一个会话，那么就发消息去删除这么一条消息
		//如果在本服登陆，并且在跨服登陆，先踢下跨服的账户
		if(nRawServerId == nLoginServerId && m_nCommServerId >0) //如果是在普通服登陆
		{
			//把公共服的踢掉
			if( m_pSSServer->GetSpecialServerSession(sessionData.nSessionId,m_nCommServerId))
			{
				OutputMsg(rmTip,"Kick cross server actor,accountid=%d,serverindex=%d",(int)sessionData.nSessionId,(int)m_nCommServerId);
				m_pSSServer->PostKickCrossActor(sessionData.nSessionId,m_nCommServerId);
			}

		}

		if ( m_pSSServer->GetSession( sessionData.nSessionId ,&cuRuentSession,m_nSelfServerId) )
		{
			m_pSSServer->PostCloseSessionByAccountId(sessionData.nSessionId,cuRuentSession.lKey); 
		}
		

		m_pSSServer->PostOpenSession(nLoginServerId,nRawServerId,sessionData.sAccount,
			sessionData.nSessionId,sessionData.nClientIPAddr,sessionData.dwFCMOnlineSec,sessionData.nGmLevel,lKey,nGateIndex,isWhiteIpLogin);

	}

	
}


//创建账户
VOID CSessionDataHandle::CatchCreateAccount(CDataPacketReader &reader)
{
	//玩家的账户和密码
	char name[32];
	char passwd[64];
	char identity[32]; //身份证号码
	name[0]=0;
	passwd[0]=0;
	identity[0] =0;
	unsigned long long lIp =0; //登陆的ip

	//创建账户
	reader.readString(name,sizeof(name));
	reader.readString(passwd,sizeof(passwd)); 
	reader.readString(identity,sizeof(identity));
	name[sizeof(name) -1] =0;
	passwd[sizeof(passwd) -1] =0;
	identity[sizeof(identity) -1] =0;
	reader >> lIp; //登陆的ip

	if(lIp ==0) return; //没有传入的这个不能算

	//获得ip列表
	CBaseList<unsigned long long>& ips = m_pSSServer->GetSSManager()->GetCreateAcountIpList();
	
	bool isInvalid =true;
	if(ips.count() >0)
	{
		isInvalid =false;
		for(INT_PTR i=0;i < ips.count(); i++)
		{
			if( ips[i] == lIp)
			{
				isInvalid =true;
				break;
			}
		}
	}
	if(isInvalid ==false)
	{
		OutputMsg(rmWaning,"Create Account fail,ip=%lld",lIp);
		return;
	}

	

	DECLARE_FUN_TIME_PROF()
	int nResult = enSuccess;

	if (m_pSQLConnection ==NULL || m_pSQLConnection->Connected() ==FALSE) 
	{
		OutputMsg( rmError, "无法连接数据库");
		nResult = enSessionServerError;
	}
	else
	{
		char sSqlStr[512];
		sprintf(sSqlStr,szCreateAccount,name,passwd,identity);

		if ( m_pSQLConnection->RealQuery( sSqlStr,strlen(sSqlStr) ) )
		{
			OutputMsg( rmError, "数据库执行sql操作%s失败",sSqlStr);
			nResult = enSessionServerError;
		}
		else 
		{
			m_pSQLConnection->ResetQuery();
		}
	}

	CDataPacket &out = AllocDataPacket(sCreateAccountResult);;
	out << BYTE (nResult); //1字节的结果
	FlushDataPacket(out);
}


int CSessionDataHandle::CheckUserValid(PCHAR name, PCHAR passwd,GLOBALSESSIONOPENDATA & sessiDonata,PCHAR md5,PCHAR sCurrentTime,bool checkMd5,bool bCommonSrvLogin,PCHAR iIndentity,bool &isWhiteIpLogin,int nRawServerindex)
{
	//如果找不到db连接
	//DECLARE_FUN_TIME_PROF()
	OutputMsg(rmTip,"%s Login,nRawServerindex = %d,serverid=%d",name,nRawServerindex,m_nSelfServerId);
	int nSelfServerId = m_nSelfServerId;
	if(nSelfServerId == 0)
	{
		nSelfServerId = m_nSelfServerId;
	}

	if (m_pSQLConnection ==NULL || m_pSQLConnection->Connected() ==FALSE) 
	{
		OutputMsg( rmError, "无法连接数据库");
		return enSessionServerError; 
	}
	
	int nColumnCount =0;
	int nRowCount =0;

	char sSqlStr[512];
	
	CSSManager* pMgr = m_pSSServer->GetSSManager();
	

	static bool bPhpCheck = pMgr->isCheckPhpSign(); //是否要监测php的秘钥
	static char * sPguid = pMgr->GetSPID();   //获取用户的spguid
	static char * sKey = pMgr->GetPhpKey(); //获得这个php的key


	//是不是通过白名单的IP登陆的
	isWhiteIpLogin =false;

	int nResult = enSuccess;

	unsigned long long lLoginIp =sessiDonata.nClientIPAddr; //登陆的ip
	bool isWhite =false; //是否在白名单中

	CBaseList<unsigned long long > &whiteips = pMgr->GetWhiteIpList();
	bool flag =false;
	if(whiteips.count() >0)
	{
		//判断是否在合法的ip范围里
		for(INT_PTR i=0;i < whiteips.count(); i++)
		{
			if(whiteips[i]== lLoginIp)
			{
				isWhite =true;
				isWhiteIpLogin =true;
				break;
			}
		}
	}

	//进行校验才进行验证
	if(!isWhite && bPhpCheck && checkMd5 && bCommonSrvLogin   ) //如果开启了监测php的，并且需要兼容一起的版本
	{

		//这里发生了错误
		if(sCurrentTime ==NULL || sCurrentTime[0]==0 )
		{
			nResult = enTimeFormatError;
		}
		else
		{
			size_t nNumLen = strlen(sCurrentTime);
			for(size_t j=0; j< nNumLen; j++)
			{
				if(sCurrentTime[j] > '9' || sCurrentTime[j] <'0')
				{
					nResult = enTimeFormatError;
					break;
				}
			}	

		}

		if(!nResult)
		{
			if( md5==NULL ||  md5[0] ==0)
			{
				nResult = enIpMd5Error;
			}
		}


		if( !nResult )
		{

			char buff[1024]; 

			strcpy(buff,sCurrentTime);
			if(pMgr->GetCheckServerIndex() )
			{
				strcat(buff,m_sTrServerIndex); //数字化的服务器的id
			}
			strcat(buff,sPguid); //spguid
			strcat(buff,name);
			strcat(buff,sKey);

			unsigned char mdkey[64];
			MD5_CTX   ctx; 
			//使用 MD5加密玩家的密码
			MD5Init(&ctx); 
			MD5Update(&ctx,  (unsigned char *)buff,   (unsigned int)strlen(buff) );
			MD5Final(mdkey,&ctx);
			mdkey[32]=0;

			if( strcmp(md5,(char*)mdkey) != 0 )
			{
				OutputMsg(rmError,"Local raw string=%s,md5 error",buff);
				nResult = enIpMd5Error; //md5错误了
			}
			else
			{

				unsigned int nCurrentTime = strtoul(sCurrentTime,NULL,10);
				time_t sSrvTime;
				//struct  tm * timeInfo;
				time(&sSrvTime); //取了当前的时间

				unsigned int nDif = (unsigned int)(abs(sSrvTime -nCurrentTime));  //服务器的时间减去传入的时间

				static unsigned int s_timeDif =   pMgr->GetPhpTime(); //获取php的时间

				if(nDif > s_timeDif) //如果超过了3分钟
				{
					OutputMsg(rmError,"%s login ,time dif=%u seconds",name,nDif);

					nResult = enSignOutofDate; //md5错误了
				}

			}

		}
	}

	//////////////////下面判断是否在合法的ip里访问了
	bool bWhitePass = pMgr->GetWhitePass(); //是否只要白名单的才能通过
	if(bWhitePass  && isWhite ==false)
	{
		nResult =enIpError; 
	}

	//如果在白名单中，则不需要判断，否则需要判断是否在黑名单中
	if(!nResult && !isWhite )
	{
		CBaseList<unsigned long long > &blackips = pMgr->GetBlackIpList();
		bool flag =false;
		if(blackips.count() >0)
		{
			//判断是否在合法的ip范围里
			for(INT_PTR i=0;i < blackips.count(); i++)
			{
				if(blackips[i] == lLoginIp)
				{
					flag =true;
					break;
				}
			}
		}
		if(flag)
		{
			nResult =enIpError; 
		}
		else
		{
			//判断登陆的个数是否超过了限制
			int nLoginLimitCount= m_pSSServer->GetLoginAccountCount(lLoginIp); 
			if(nLoginLimitCount >  pMgr->GetIpLoginCount())
			{
				//nResult =enIpTooManyConnect; 			
				OutputMsg(rmTip,"IP=%s,connect count=%d,limit=%d",
					inet_ntoa(*((in_addr*)&lLoginIp)),nLoginLimitCount,(int) pMgr->GetIpLoginCount());
				nResult =enIpTooManyConnect; 
			}
		}
	}

	if(nResult) return nResult; //先做下判断，如果超过了，直接不让注册，避免数据库大量垃圾数据


	if(pMgr->IsAutoAccount())
	{
		char *sIndentity = NULL;
		if(iIndentity != NULL && iIndentity[0] !=0)
		{
			sIndentity = iIndentity;
		}
		else
		{
			sIndentity="000000198010100000"; 
		}
		sprintf(sSqlStr,"call djimportspuser(\"%s\",\"%s\",%d,\"%s\",%lld)",name,passwd,nSelfServerId,sIndentity,lLoginIp);

		if(nResult != enSuccess)
		{
			return nResult;
		}

		//先都调用一下
		if( m_pSQLConnection->RealQuery( sSqlStr,strlen(sSqlStr) ) )
		{
			OutputMsg( rmError, "数据库执行sql操作%s失败",sSqlStr);
			nResult = enSessionServerError;
			return nResult;
		}
		else
		{
			m_pSQLConnection->ResetQuery();
		}
	}

	

	//__try
	{
	
	
		_snprintf(sSqlStr,sizeof(sSqlStr),szUserLoginGetGlobalUser,name);
	
		unsigned int nAccountId = 0; //账户的ID
		if ( m_pSQLConnection->RealQuery( sSqlStr,strlen(sSqlStr) ) )
		{
			OutputMsg( rmError, "数据库执行sql操作%s失败",sSqlStr);
			nResult = enSessionServerError;
		}
		else 
		{
			nColumnCount = m_pSQLConnection->GetFieldCount();  //获取列数
			nRowCount  = m_pSQLConnection->GetRowCount(); //获取行数

			//读取角色网关路由表
			//OutputMsg( rmTip, _T("Read user data from Db"),sSqlStr);
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow && nRowCount >=1 )
			{
			
				if(pRow[0] && nColumnCount >=1)
				{
					sscanf(pRow[0], "%u", &nAccountId); //账户名字
				}
				TCHAR sDbPasswd[ACTOR_NAME_BUFF_LENGTH];
				if(pRow[1] && nColumnCount >=2)
				{
					_asncpytA(sDbPasswd,pRow[1]);
				}
				else
				{
					sDbPasswd[0] ='\0';
				}

				
				if (!isWhite && 0 != strcmp(sDbPasswd,passwd)  )
				{
					OutputMsg( rmError, "%s，dbpasswd=%s,inputwd=%s",name,sDbPasswd,passwd);
					if(sDbPasswd[0] =='@')
					{
						nResult =enAccountSeal; //账户被封停
					}
					else
					{
						nResult = enPasswdError;
					}
					
				}
				else
				{
				
					//OutputMsg( rmNormal, "%s密码确认成功,accountID =%d",name,nActorId);
					strncpy(sessiDonata.sAccount,name,sizeof(sessiDonata.sAccount));
					
					sessiDonata.nSessionId = nAccountId;
					if(pRow[6] && nColumnCount >=7)
					{
						sscanf(pRow[6], "%d", &sessiDonata.nGmLevel);
					}
					else
					{
						sessiDonata.nGmLevel = 0;
					}
					
					bool flag = pMgr->GetGmIpLogin(); //是否限制了IP登陆

					//GM登陆，需要判断是否是在合法的ip范围里登陆
					if(sessiDonata.nGmLevel >0 && flag  )
					{
						//获得ip列表
						CBaseList<unsigned long long > &ips = pMgr->GetGmIpList();
						bool isGMinvalid =false;

						
						//判断是否在合法的ip范围里
						for(INT_PTR i=0;i < ips.count(); i++)
						{
							if(ips[i]==  lLoginIp)
							{
								isGMinvalid =true;
								break;
							}
						}
						
						
						if(!isGMinvalid)
						{
							nResult =enGMLoginFailError; 
						}
					}
				}
			}
			else
			{
				//OutputMsg(rmWaning,_T("No Account  %s Found on db"),name);
				nResult = enNoAccount;
			}
			m_pSQLConnection->ResetQuery();
		}



		if(nResult == enSuccess)
		{
			//只要在防沉迷关闭的时候才去判断防沉迷的时间，否则不判断
			if(m_pSSServer->GetSSManager()->isFcmOpen() )
			{
				int nTime = -1;
				char sSqlStr[200];
				sprintf(sSqlStr,szGetFcmTime,nAccountId);
				
				if ( m_pSQLConnection->RealQuery( sSqlStr, strlen(sSqlStr)) )
				{
					OutputMsg( rmError, "数据库执行sql操作%s失败",sSqlStr);
					nTime = -1;
				}
				else
				{
					MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

					nColumnCount = m_pSQLConnection->GetFieldCount();  //获取列数
					nRowCount  = m_pSQLConnection->GetRowCount(); //获取行数

					if ( pRow && nColumnCount >=1 && nRowCount >=1 )
					{
						sscanf(pRow[0], "%d", &nTime); 
					}
					m_pSQLConnection->ResetQuery();
				}
				sessiDonata.dwFCMOnlineSec = nTime;

			}
			else //否则全部的玩家都纳入防沉迷
			{
				sessiDonata.dwFCMOnlineSec =-1;
			}
		}
	
	}
	//__except( CustomExceptHandler(GetExceptionInformation() ))
	{
		//OutputMsg(rmError,"ACCOUNT=%s",name);
		//return nResult;
	}
	
	return nResult;
}

VOID CSessionDataHandle::SetCommonServerId(CDataPacketReader &inPacket)
{
	int nCommonServerId =0;
	int nSelfServerId=0;
	inPacket >> nCommonServerId  >> nSelfServerId;

	if( nSelfServerId <=0)
	{
		OutputMsg(rmError,"SetCommonServerId error,nCommonServerId=%d,nSelfServerId=%d",nCommonServerId,nSelfServerId);
		return;
	}
	m_nCommServerId = nCommonServerId;
	m_nSelfServerId = nSelfServerId;
	sprintf(m_sTrServerIndex,"%d",nSelfServerId);
	//itoa(nSelfServerId,m_sTrServerIndex,10); //计算了服务器的id

	OutputMsg(rmTip,"Client id=%d,set commonserverId=%d",nSelfServerId,nCommonServerId);
}


//收到了后台发来的消息
VOID  CSessionDataHandle::CatGetContrlCenterMsg(CDataPacketReader &inPacket)
{
	char ipName[512]; //ip的名字
	ipName[0]=0;
	inPacket.readString(ipName,sizeof(ipName) -1); //读取ip
	
	OutputMsg(rmTip,"Recv Control Msg =%s", ipName); //收到了消息
	
	
	size_t nSize =strlen(ipName);
	
	int nTokenPos =-1; //分割符的位置
	for(size_t i=0; i < nSize ; i++)
	{
		char c= ipName[i];
		if(c >= '0' && c <= '9') //是数字
		{
			
		}
		else
		{
			if(c == '|') //分割符
			{
				nTokenPos = (int)i;
				break;
			}
			else
			{
				OutputMsg(rmTip,"Bad Control Format =%s", ipName); //收到了消息,找不到分割符号
				return;
			}
		}
	}
	
	if(nTokenPos  <=0)
	{
		return;
	}
	
	ipName[nTokenPos] =0; //把字符串分割下
	
	int nCommand = atoi(ipName); //命令码

	char *pStr = &ipName[nTokenPos +1]; //这个是后面的字符串
	
	
	OutputMsg(rmTip,"Control ID=%d, Msg =%s", nCommand,pStr); //收到了消息

	switch(nCommand)
	{
		//封ip
	case 1:
		m_pSSServer->SealIp(pStr);
		break;

		//解封IP
	case 2:
		m_pSSServer->UnSealIp(pStr);
		break;
	}
}


//收到QQ的消息 (任务集市等)
VOID CSessionDataHandle::CatQQMsg(CDataPacketReader &inPacket)
{
	CAMClient::tagTASKMsg data;
	inPacket >> data; //读取1个字节的消息码
	
	m_pSSServer->GetSSManager()->getAMClient()->PostAmTaskData(data);
}

VOID CSessionDataHandle::CatSendCommonPlatformData(CDataPacketReader &inPacket)
{
	char sIp[64];
	inPacket.readString(sIp,sizeof(sIp));

	int nPort = 0;
	inPacket >> nPort;

	//OutputMsg(rmTip,"CatSendCommonPlatformData nPort=%d, sIp =%s", nPort,sIp);

	m_pSSServer->GetSSManager()->getSessionCenter()->OnSendConnectCommonPlatform(sIp,nPort);
}

VOID CSessionDataHandle::CatRetranToCommonPlatform(CDataPacketReader &inPacket)
{
	m_pSSServer->GetSSManager()->getSessionCenter()->OnRequesTranCommonPlatform(inPacket);

	//unsigned int nAccountId = 0, nActorId = 0, nDestServerId = 0, nType = 0;
	//inPacket >> nAccountId >> nActorId >> nDestServerId >> nType;
	//CDataPacket &pdata = allocProtoPacket(REV_LOGICSERVERINFO); //分配一个 网络包
	//pdata.writeBuf(inPacket.getOffsetPtr(),inPacket.getLength() - inPacket.getPosition());

}
