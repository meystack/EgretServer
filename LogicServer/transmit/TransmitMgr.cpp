#include "StdAfx.h"
using namespace TRANSMIT;
#include "../base/Container.hpp"
bool CTransmitMgr::ReqTransmitTo(CActor *pActor, const char *pSceneName, const int nPosX, const int nPosY)
{
	if (!pActor) 
		return false;

	// 0、判断此玩家是否在等待传送阶段（发送了传送请求，还没收到应答），如果是，返回false；否则继续下面处理
	// 1、找到目标逻辑服务器，如果没连接上，返回false，否则继续下面处理
	// 2、调用对应的连接对象分配数据包DataPacket，并且先写入本服务器的ServerIndex以及是否是公共服务器标记（1Byte）
	// 3、将要传送到的目标场景和位置写入到DataPacket（以便目标服务器检测是否可传入）
	// 4、将数据包发送给目标服务器

	// 5、将此玩家加入到等待传送列表，记录当前时间用于超时。
	// 当前角色所在的逻辑服务器ServerIndex
	int nServerIdx	= GetLogicServer()->GetServerIndex();
	unsigned int nActorId	= pActor->GetId();
	// 角色原始服务器的ServerIndex
	int nRawServerIdx = GetActorRawServerIndex(pActor);
	char szData[256];
	CDataPacket packet(szData, sizeof(szData));
	packet.setLength(0);
	packet << (WORD)L2L_ReqTransmit << nServerIdx << nActorId;	
	SendMessage(packet.getMemoryPtr(), packet.getPosition(), nRawServerIdx);

	return true;
}

void CTransmitMgr::OnRecvReqTransmitTo(CDataPacketReader &reader)
{	
	unsigned int nSrcServerIdx = 0, nActorId = 0;
	reader >> nSrcServerIdx >> nActorId; // TODO 场景名称、坐标XY
	char szData[256];
	CDataPacket packet(szData, sizeof(szData));
	packet.setLength(0);
	packet << (WORD)L2L_ReqTransmitAck << (bool)true << nSrcServerIdx << nActorId;
	SendMessage(packet.getMemoryPtr(), packet.getPosition(), nSrcServerIdx);
}

void CTransmitMgr::OnRecvReqTransmitToAck(CDataPacketReader &reader)
{
	unsigned int nServerIdx = 0, nActorId = 0;
	reader >> nServerIdx >> nActorId;
	// TODO 对于公共服务器应该根据角色ID和ServerID一起来唯一确定一个CActor对象
	CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	StartTransmitTo(pActor);
}

void CTransmitMgr::StartTransmitTo(CActor *pActor)
{
	int nServerIdx	= GetLogicServer()->GetServerIndex(); 
	unsigned int nActorId	= 0;	
	char szData[128];
	CDataPacket packet(szData, sizeof(szData));
	packet << (WORD)L2L_StartTransmit << nServerIdx << nActorId;
	SendMessage(packet.getMemoryPtr(), packet.getPosition(), GetActorRawServerIndex(pActor));

	// TODO.调用Actor的TransmitTo函数开始传送
	EndTransmitTo(pActor);
}

void CTransmitMgr::OnRecvStartTransmitTo(CDataPacketReader &reader)
{
	unsigned int nServerIdx = 0, nActorId = 0;
	reader >> nServerIdx >> nActorId; // 场景名称、坐标XY
		
	if (nServerIdx < 0 || nServerIdx >= SERVER_INDEX_MAX)
	{
		OutputMsg(rmError, _T("%s error occur: invalid serverindex=%d"), __FUNCTION__, nServerIdx);
		return;
	}

	if (nServerIdx == GetLogicServer()->GetServerIndex())
	{
		OutputMsg(rmError, _T("%s error occur: transmit from same server!!!"), __FUNCTION__);
		return;
	}

	// TODO.设置角色的会话状态、创建角色缓存数据对象
	CServerActorInfo &serverActorInfo = m_ServerActorInfo[nServerIdx];
	serverActorInfo.AddActorCache(nActorId); // 到目前为止，还没有缓存任何角色传送数据！
}

bool CTransmitMgr::ExistActorTransSessionData(int nServerIdx, unsigned int nActorId)
{
	if (nServerIdx < 0 || nServerIdx > SERVER_INDEX_MAX)
	{
		OutputMsg(rmError, _T("%s invalid serverindex:%d"), __FUNCTION__, nServerIdx);
		return false;
	}

	TRANSMIT::CServerActorInfo &serverActorInfo = m_ServerActorInfo[nServerIdx];
	return serverActorInfo.GetActorCache(nActorId) != NULL ? true : false;
}

bool CTransmitMgr::ExistAccountTransSessionData(int nServerIdx, unsigned int nAccount)
{
	if (nServerIdx < 0 || nServerIdx > SERVER_INDEX_MAX)
	{
		OutputMsg(rmError, _T("%s invalid serverindex:%d"), __FUNCTION__, nServerIdx);
		return false;
	}

	TRANSMIT::CServerActorInfo &serverActorInfo = m_ServerActorInfo[nServerIdx];
	return serverActorInfo.GetActorCacheByAccountId(nAccount) != NULL ? true : false;
}

void CTransmitMgr::OnLogicServerDisconnected(int nServerIdx)
{
	CLogicServer *pLogicServer = GetLogicServer();
	if (pLogicServer->IsCommonServer())
	{		
		((CCSTransmitMgr *)this)->KillActorByServerIdx(nServerIdx);
	}
	else
	{
		// 将那些传送到公共服务器的玩家会话关闭。
		((CNSTransmitMgr *)this)->CloseTransedActorSession();
	}
}

void CTransmitMgr::EndTransmitTo(CActor *pActor)
{
	int nServerIdx = GetLogicServer()->GetServerIndex(); 
	unsigned int nActorId	= 0;	
	char szData[128];
	CDataPacket packet(szData, sizeof(szData));
	packet << (WORD)L2L_EndTransmit << nServerIdx << nActorId;
	SendMessage(packet.getMemoryPtr(), packet.getPosition(), GetActorRawServerIndex(pActor));
}

void CTransmitMgr::OnRecvEndTransmitTo(CDataPacketReader &reader)
{
	unsigned int nServerIdx = 0, nActorId = 0;
	reader >> nServerIdx >> nActorId;
		
	if (nServerIdx < 0 || nServerIdx >= SERVER_INDEX_MAX)
		return;

	CServerActorInfo &serverActorInfo = m_ServerActorInfo[nServerIdx];
	CActorTransmitCache *pCache = serverActorInfo.GetActorCache(nActorId);
	if (pCache)
	{
		pCache->SetAllDataReady(true);
	}
}

void CTransmitMgr::SendMessage(void *pData, SIZE_T size, int nServerIdx)
{
	CLogicServer *pLogicServer = GetLogicServer();
	if (pLogicServer->IsCommonServer())
	{
		//CCommonServer *pServer = pLogicServer->GetCommonServer();
		//pServer->SendData(nServerIdx, pData, size);
	}
	else
	{
		CCommonClient *pClient = pLogicServer->GetCommonClient();
		// 这里消息号是站位用，pData里头已经有消息号
		CDataPacket &packet = pClient->allocProtoPacket(0);
		packet.adjustOffset(0-sizeof(jxSrvDef::INTERSRVCMD));	// 将Offset回退到消息ID的位置
		packet.writeBuf(pData, size);
		pClient->flushProtoPacket(packet);
	}
}

void CTransmitMgr::TransmitFrom(INT_PTR nCmd, CDataPacketReader &reader)
{
	// 处理传送打包的角色数据
}

int CTransmitMgr::GetActorRawServerIndex(CActor *pActor)
{
	CLogicServer *pLogicServer = GetLogicServer();
	int nServerIdx = pLogicServer->GetServerIndex();
	if (GetLogicServer()->IsCommonServer())
		nServerIdx = pActor->GetRawServerIndex();

	return nServerIdx;
}

void CTransmitMgr::OnRecvMessage(int nDir, int nMsgId, CDataPacketReader &reader)
{
	switch (nMsgId)
	{
	case L2L_ReqTransmit:
		OnRecvReqTransmitTo(reader);
		break;
	case L2L_ReqTransmitAck:
		OnRecvReqTransmitToAck(reader);
		break;
	case L2L_StartTransmit:
		OnRecvStartTransmitTo(reader);
		break;
	case L2L_EndTransmit:
		OnRecvEndTransmitTo(reader);
		break;
	default:
		OutputMsg(rmError, _T("%s Recv unknown message:%d"), __FUNCTION__, nMsgId);
		break;
	}
}

void CCSTransmitMgr::KillActorByServerIdx(int nServerIdx)
{
	if (nServerIdx < 0 || nServerIdx >= SERVER_INDEX_MAX)
	{
		OutputMsg(rmError, _T("%s invalid serveridx:%d"), __FUNCTION__, nServerIdx);
		return;
	}

	TRANSMIT::CServerActorInfo &serverActorInfo = m_ServerActorInfo[nServerIdx];
	serverActorInfo.CloseAllActor();
}

void CNSTransmitMgr::CloseTransedActorSession()
{
	INT_PTR count = m_transedActorList.count();
	if (count > 0)
	{
		CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
		CMiniDateTime nCurrentTime;
		nCurrentTime.tv = GetGlobalLogicEngine()->getMiniDateTime();
		for (INT_PTR i = 0; i < count; i++)
		{
			TRANSMIT::CTransedActorRecord &record = m_transedActorList[i];
			pSSClient->PostCloseSession(record.m_nAccountId, 0);
		}
	}
}

void CTransmitMgr::RunOne(TICKCOUNT nCurrTick)
{
	for (INT_PTR i = 0; i < SERVER_INDEX_MAX; i++)
	{
		TRANSMIT::CServerActorInfo &ServerActorInfo = this->m_ServerActorInfo[i];
		ServerActorInfo.RunOne(nCurrTick);
	}
}

namespace TRANSMIT
{
	void CServerActorInfo::CloseAllActor()
	{
		if (m_actorDBData.size() > 0)
		{
			CEntityManager *pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			std::map<int, CActorTransmitCache*>::iterator iter = m_actorDBData.begin();
			std::map<int, CActorTransmitCache*>::iterator iter_end = m_actorDBData.end();
			for (; iter != iter_end; ++iter)
			{
				unsigned int nActorId = iter->first;
				CActor *pActor = pEntityMgr->GetEntityPtrByActorID(nActorId);
				if (pActor)
					pActor->CloseActor(lwiCloseTransmitAllUser, false);
			}
		}
	}

	void CServerActorInfo::RunOne(TICKCOUNT nCurrTick)
	{
		std::map<int, CActorTransmitCache*>::iterator iter = m_actorDBData.begin();
		std::map<int, CActorTransmitCache*>::iterator iter_end = m_actorDBData.end();
		for (; iter != iter_end; ++iter)
		{
			CActorTransmitCache *pCache = iter->second;			
		}
	}
}
