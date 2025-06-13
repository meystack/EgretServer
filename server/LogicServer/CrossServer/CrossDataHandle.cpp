
#include "StdAfx.h"
#include "CrossDataHandle.h"

using namespace jxInterSrvComm::SessionServerProto;

CCrossDataHandle::CCrossDataHandle()
{
	m_nSelfServerId =0;
	m_dwDisconnectedTick = 0;
	m_pSSServer = NULL;
	m_pForwardDataBuff= NULL;
}


CCrossDataHandle::~CCrossDataHandle()
{
	//m_Allocator.FreeBuffer(m_pHugeSQLBuffer);
	if(m_pForwardDataBuff)
	{
		delete []m_pForwardDataBuff;
		m_pForwardDataBuff =0;
	}
}

void  CCrossDataHandle::SetParam(CCrossServer *pSessionSrv, bool isSessonConnect  )
{
	m_pSSServer = pSessionSrv;
}



//处理logic发送到的消息
VOID CCrossDataHandle::OnPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	switch (nCmd)
	{
		case jxInterSrvComm::CrossServerProto::cReqCrossLogin:
		case jxInterSrvComm::CrossServerProto::cSendCrossData:
		case jxInterSrvComm::CrossServerProto::cSendCloseActor:
		case jxInterSrvComm::CrossServerProto::cGetCrossActorId: 
		case jxInterSrvComm::CrossServerProto::cCSGetRankList:
		case jxInterSrvComm::CrossServerProto::cGetActorOfflineData:
		case jxInterSrvComm::CrossServerProto::cSendReqChat: 
		{
			int nSrvId = 0;
			unsigned int nActorId = 0;
			// BYTE nType = 0;
			// inPacket >> nSrvId >> nActorId >> nType;
			// printf("C2L_ReqLoginData nSrvId:%d, nActorId:%d, nType:%d\n",nSrvId,nActorId,nType);
			CDataPacket &packet = AllocDataPacket(0);
			packet.setPosition(0);
			// inPacket >> nSrvId >> nActorId;
			// packet<< nSrvId << nActorId;
			// packet.adjustOffset(0-sizeof(jxSrvDef::INTERSRVCMD));	// 将Offset回退到消息ID的位置
			packet.writeBuf(inPacket.getOffsetPtr(), inPacket.getLength()- inPacket.getPosition());
			
			//通知逻辑可以初始化数据
			GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(SSM_LOGIC_2_CROSS_DATA,
						nCmd,
						(Uint64)(&packet)
						);
			
		}break;
		default:
			break;
	}
}



VOID CCrossDataHandle::BroadcastMessage2Logic(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize)
{
	if (!m_pSSServer && nServerIndex < 0)
		return;
	if (0 == nServerIndex)
		m_pSSServer->SendMsg2AllClient(GameServer, sMsg, dwSize);
	else
		m_pSSServer->SendMsg2LogicClient(nServerIndex, sMsg, dwSize);
}



