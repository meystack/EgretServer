#include "StdAfx.h"
#include "../base/Container.hpp"
using namespace jxInterSrvComm::DbServerProto;

CMsgSystem::CMsgSystem()
{
}

CMsgSystem::~CMsgSystem()
{

}

bool CMsgSystem::Initialize(void *data,SIZE_T size)
{
	return true;
}

void CMsgSystem::OnEnterGame()
{
	SendDbLoadMsg(0);
}


void CMsgSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	switch (nCmd)
	{
	case cReadMsg:
		{
			INT64 nMsgId = 0;
			packet >> nMsgId;
			ProcessMsg(nMsgId);
			break;
		}
	default:
		break;
	}
}
/*
bool CMsgSystem::SendFireMsgInfo(INT64 nMsgId, byte nType, WORD nCount, char * sName, char * sBuff)
{
	CActorPacket ap;
	CDataPacket& netPack = m_pEntity->AllocPacket(ap);
	netPack << (BYTE)enMiscSystemID << (BYTE)sOpenRedGift;
	netPack << (INT64)nMsgId << (BYTE)nType;
	netPack.writeString(sName);
	netPack << nCount;
	netPack.writeString(sBuff);
	ap.flush();
	return true;
}
*/
void CMsgSystem::SendMsgToClient(MsgData& msg)
{
	if (!m_pEntity)
	{
		return;
	}
	if (msg.Msgtype <= mtNoType || msg.Msgtype >= mtMessageCount)
	{
		return;
	}
	LPCSTR sTitle = NULL;
	LPCSTR sBtnTxt = NULL;
	msg.GetTxt(sTitle,sBtnTxt);
	/*if (msg.Msgtype == mtSendFire || msg.Msgtype == mtSendRedPacket)
	{
		LPCSTR sTitle = NULL;
		LPCSTR sBtnTxt = NULL;
		CDataPacketReader packet(msg.Msg,MAX_MSG_COUNT);
		packet >> sTitle;
		packet >> sBtnTxt;

		byte nType;
		WORD nCount;
		ACTORNAME sName;
		char sBuff[1024];
		packet >> nType >> nCount;
		packet.readString(sName, ArrayCount(sName));
		packet.readString(sBuff, ArrayCount(sBuff));

		SendFireMsgInfo(msg.Msgid, nType, nCount, sName, sBuff);
	}
	else*/
	{
		CActorPacket AP;
		CDataPacket& packet = m_pEntity->AllocPacket(AP);
		AP << (BYTE)enMsgSystemID << (BYTE)sAddMsg;
		AP << (WORD)1;
	
		AP << (INT64)msg.Msgid;
		AP << (BYTE)msg.Msgtype;
		//发送文字
		sTitle?packet.writeString(sTitle):packet.writeString("");
		sBtnTxt?packet.writeString(sBtnTxt):packet.writeString("");	// 按钮格式: 按钮1名称;按钮2名称/操作类型,参数1 默认“确定”则使用""
		AP.flush();
	}
}


void CMsgSystem::OnAddOfflineMsgDbReturn(INT_PTR nCmd,char * data,SIZE_T size)
{
	if ( nCmd != dcAddMsg)
	{
		return;
	}
	LPCSTR sErrStr = NULL;
	int nMsgType = ttTipmsgWindow;
	CDataPacketReader reader(data,size);
	int nActorid = 0,nSrcActorId = 0;
	BYTE nErrorCode = 0;	
	int nRawServerId, nLoginServerId;
	reader >> nRawServerId >> nLoginServerId;
	reader >> nActorid;//
	reader >> nSrcActorId;
	reader >> nErrorCode;
	if (nErrorCode != reSucc)
	{
		sErrStr = _T("Error!!the msg send to %d fail!");
		nMsgType = ttDialog;
		OutputMsg(rmError,sErrStr,nActorid);
	}
	else
	{
		sErrStr = _T("the msg send to%d ok!");
		//如果用户在线，通知其从数据库中读出
		CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorid);
		if (pActor)
		{
			INT64 nMsgId = 0;
			reader >> nMsgId;
			pActor->GetMsgSystem().ReloadFromDb(nMsgId);
		}
	}
	//通知发送的原角色(一般是GM）错误消息
	/*CActor* pSrcActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nSrcActorId);
	if (pSrcActor)
	{
	pSrcActor->SendTipmsgFormat(sErrStr,nMsgType,nActorid);
	}*/
}

void CMsgSystem::OnAddOfflineMsgByNameDbReturn(INT_PTR nCmd,char * data,SIZE_T size)
{

	if ( nCmd != dcAddMsgByActorName)
	{
		return;
	}	
	

	LPCSTR sErrStr = NULL;
	int nMsgType = ttTipmsgWindow;
	CDataPacketReader reader(data,size);
	int nActorid = 0,nSrcActorId = 0;
	LPCSTR sName = NULL;
	BYTE nErrorCode = 0;
	int nRawServerId, nLoginServerId;
	reader >> nRawServerId >> nLoginServerId;
	reader >> sName;
	reader >> nSrcActorId;
	reader >> nErrorCode;
	if (nErrorCode != reSucc)
	{
		sErrStr = _T("Error!!the msg send to %s fail!");
		nMsgType = ttDialog;
		OutputMsg(rmError,sErrStr,sName);
	}
	else
	{
		//如果用户在线，通知其从数据库中读出		
		CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName((char*)sName);
		if (pActor)
		{
			INT64 nMsgId = 0;
			reader >> nMsgId;
			pActor->GetMsgSystem().ReloadFromDb(nMsgId);
		}
		sErrStr = _T("the msg send to %s ok!");
	}
	//通知发送的原角色(一般是GM）错误消息
	/*CActor* pSrcActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nSrcActorId);
	if (pSrcActor)
	{
	pSrcActor->SendTipmsgFormat(sErrStr,nMsgType,sName);
	}*/

}


VOID CMsgSystem::OnDbRetData( INT_PTR nCmd,INT_PTR nErrorCode, CDataPacketReader &reader )
{
	switch(nCmd)
	{
	case dcLoadMsg://接收自己的消息
		{
			//不可以加重复Msgid的消息
			if (nErrorCode != reSucc)
			{
				OutputMsg(rmError,_T("load actor msg error!"));
			}else
			{
				WORD nCount = 0;
				reader >> nCount;
				for(int i = 0; i < nCount; i++)
				{
					MsgData msg;
					reader >> msg.Msgid;
					reader >> msg.Msgtype;
					reader >> msg.nBufSize;
					reader.readBuf(msg.Msg,msg.nBufSize);
					if (AddToList(msg))
					{
						SendMsgToClient(msg);
					}
				}
			}
			
			break;
		}
	case dcDeleteMsg:	//删除自己的消息
		{
			if (nErrorCode != reSucc)
			{
				OutputMsg(rmError,_T("delete actor msg error!"));
			}
			break;
		}
	}
}

bool CMsgSystem::ProcessMsg( INT64 msgId )
{
	
	bool ret = false;
	for (INT_PTR i = m_msgList.count()-1; i >= 0; i--)
	{
		MsgData& msgInList = m_msgList[i];
		if (msgInList.Msgid == msgId)
		{
			ret = ProcessMsg(msgInList);
			if (ret)
			{
				CActorPacket AP;
				m_pEntity->AllocPacket(AP);
				AP << (BYTE)enMsgSystemID << (BYTE)sDeleteMsg << (INT64)msgId;
				AP.flush();

				m_msgList.remove(i);

				SendDbDeleteMsg(msgId);//通知数据库删除
			}
			break;
		}
	}

	return ret;
}

bool CMsgSystem::ProcessMsg( MsgData& msg )
{
	if (!m_pEntity) return false;
	bool ret = false;
	//前两个字段固定是字符串
	LPCSTR sTitle = NULL;
	LPCSTR sBtnTxt = NULL;
	CDataPacketReader packet(msg.Msg,MAX_MSG_COUNT);
	packet >> sTitle;
	packet >> sBtnTxt;
	//分发给各个子系统处理
	switch(msg.Msgtype)	
	{
	case mtTxtMsg:
	case mtGmTxtMsg:
	case mtChangeMoney:
	case mtAddIndexItem:
	case mtSeverMaster:
	case mtExpelPupil:
	case mtDeleteGuild:
	case mtReturnSuggestMsg:
	case mtGuildDepotMsg:
	case mtAuctionItem:
	case mtGiveStoreItem:
	case mtPaTaMasterAward:
	case mtCorpsBattleAward:
	case mtCombatRankAward:
	case mtWholeActivityAward:
	case mtGiveFirstSbkGuildAward:
	case mtDefendSbkThreeTimes:
	case mtDefendSbkSecAwards:
	case mtGiveFirstCombineSbkAward:
	case mtGiveFirstCombineSecSbkAward:
	case mtGiveCombineDefendSbkAward:
	case mtGiveCombineDefendSbkSecAward:
	case mtGiveCombineActivityAward:
	case mtHundredYestodayConsumeRank:
	case mtSurpriseRet:
	case mtFestivalWithDraw:
	case mtLuckAward:
	case mtFireTop1:
	case mtOldPlayerBack:
	case mtSpecialConsumeYb:
	case mtCombatLog:
		{
			ret = m_pEntity->ProcessOfflineMsg(msg.Msgtype,packet.getOffsetPtr(),MAX_MSG_COUNT-packet.getPosition(),msg.Msgid);
			if (msg.Msgtype == mtAuctionItem)
			{
				CActorPacket AP;
				m_pEntity->AllocPacket(AP);
				AP << (BYTE)enMsgSystemID << (BYTE)sResultMsg << (INT64)msg.Msgid << (BYTE)ret;
				AP.flush();
			}
			break;
		}
	case mtUnMarry:
		{
			//ret = m_pEntity->GetInterActionSystem().ProcessOfflineMsg(msg.Msgtype,packet.getOffsetPtr(),MAX_MSG_COUNT-packet.getPosition());
			break;
		}
	}
	return ret;
}

bool CMsgSystem::AddOfflineMsg( unsigned int nActorId,INT_PTR nMsgType,char* pMsgBuf,SIZE_T size ,unsigned int nSrcActorId)
{
	if (size > MAX_MSG_COUNT) 
	{
		OutputMsg(rmError,_T("Add OfflineMsg error!!!!"));
		return false;
	}
		
	CDataPacket& DataPacket =
		GetLogicServer()->GetDbClient()->allocProtoPacket(dcAddMsg);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;	
	DataPacket << (unsigned int)nActorId;//
	DataPacket << (unsigned int)nSrcActorId;//发出消息的角色id
	DataPacket << (int)nMsgType;
	DataPacket << (int)size;
	if (size > 0 && pMsgBuf)
	{
		DataPacket.writeBuf(pMsgBuf,size);
	}
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

	return true;
}

bool CMsgSystem::AddOfflineMsgByName(LPCSTR pActorName, INT_PTR nMsgType,char* pMsgBuf,SIZE_T size ,unsigned int nSrcActorId)
{
	if (size > MAX_MSG_COUNT || !pActorName || !pMsgBuf) 
	{
		OutputMsg(rmError,_T("Add OfflineMsg error!!!!"));
		return false;
	}

	CDataPacket& DataPacket =
		GetLogicServer()->GetDbClient()->allocProtoPacket(dcAddMsgByActorName);

	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;	
	DataPacket << (unsigned int)nSrcActorId;//发出消息的角色id
	DataPacket << (int)(GetLogicServer()->GetServerIndex());
	DataPacket.writeString(pActorName);
	DataPacket << (int)nMsgType;
	DataPacket << (int)size;
	if (size > 0 && pMsgBuf)
	{
		DataPacket.writeBuf(pMsgBuf,size);
	}
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

	return true;
}

void CMsgSystem::SendDbLoadMsg(INT64 nMsgId)
{
	if (!m_pEntity)
	{
 		return;
	}

    if (((CActor*)m_pEntity)->OnGetIsTestSimulator() )
    {
        return; 
    } 
	CDataPacket& DataPacket =
		GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadMsg);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;	
	DataPacket << (unsigned int)m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);////如果nMsgid==0，读入所有消息，否则读入固定id的消息
	DataPacket << (INT64)nMsgId ;	

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket); 
}

void CMsgSystem::SendDbDeleteMsg( INT64 nMsgId )
{
	if (!m_pEntity) return;

	CDataPacket& DataPacket =
		GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteMsg);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;	
	DataPacket << (unsigned int)m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
	DataPacket << (INT64)nMsgId;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CMsgSystem::ReloadFromDb(INT64 msgId)
{
	//检查内存的列表中是否有这条消息，避免重复读入数据
	INT_PTR nCount = m_msgList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		MsgData& msg = m_msgList[i];
		if (msg.Msgid == msgId)
		{
			return;
		}
	}
	//向数据库发送读入新消息
	SendDbLoadMsg(msgId);
}

bool CMsgSystem::AddToList( MsgData& msg )
{
	INT_PTR nCount = m_msgList.count();	
	for (INT_PTR i = 0; i < nCount; i++)
	{
		MsgData& msgInList = m_msgList[i];
		if (msgInList.Msgid == msg.Msgid)
		{
			return false;
		}
	}

	if(msg.Msgtype == mtPaTaMasterAward )
	{
		int nNowDt = GetGlobalLogicEngine()->getMiniDateTime();
		char sTitle[512];
		char sBtnTxt[512];
		byte bType = 0;
		int nFubenId = 0;
		int nDeadLine = 0;
		CDataPacketReader tempPacket(msg.Msg, MAX_MSG_COUNT);
		tempPacket.readString(sTitle, ArrayCount(sTitle));
		tempPacket.readString(sBtnTxt, ArrayCount(sBtnTxt));
		tempPacket >> bType >> nFubenId >> nDeadLine;
		if (nDeadLine <= nNowDt)
		{
			SendDbDeleteMsg(msg.Msgid);//通知数据库删除
		}		
	}
	else if(msg.Msgtype == mtSendFire)
	{
		SendDbDeleteMsg(msg.Msgid);
	}
	m_msgList.add(msg);
	return true;
}


void CMsgSystem::SendMoveToMsg(LPCSTR sSceneName, INT_PTR nX, INT_PTR nY, LPCSTR sTitle, LPCSTR sTips)
{
	CActorPacket AP;
	CDataPacket& packet = m_pEntity->AllocPacket(AP);
	AP << (BYTE)enMsgSystemID << (BYTE)sMoveToMsg;
	//发送文字
	sTips?packet.writeString(sTips):packet.writeString("");	//
	sTitle?packet.writeString(sTitle):packet.writeString("");	// 寻路径坐标格式  
	sSceneName?packet.writeString(sSceneName):packet.writeString("");	//
	packet << (WORD)nX << (WORD)nY;
	AP.flush();
}
