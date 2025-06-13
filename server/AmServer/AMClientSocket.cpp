#include "stdafx.h"
#include "AMProcto.h"
#include "CommonDef.h"
using namespace AMProcto;

using namespace jxSrvDef;

CAMClientSocket::CAMClientSocket(void):
	Inherited()
{
	memset(m_Spid,0,sizeof(m_Spid));
	memset(m_gName,0,sizeof(m_gName));
	m_sMsgList.setLock(&m_sMsgListLock);
	m_sFreeList.setLock(&m_sFreeListLock);
	InitializeCriticalSection( &m_TaskMsgVecLock );
	m_Product = NULL;
	m_IsRegist = false;
	m_sClientName[0] =0;
	m_eClientType =InvalidServer;
	m_boRegisted =0;
	m_nClientSrvIdx =0;
	m_pSrv =NULL;
}

CAMClientSocket::CAMClientSocket(CAMServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr):
	Inherited()
{
	m_pSrv  = pSrv;
	SetClientSocket(nSocket,pClientAddr);
	memset(m_Spid,0,sizeof(m_Spid));
	memset(m_gName,0,sizeof(m_gName));
	m_sMsgList.setLock(&m_sMsgListLock);
	m_sFreeList.setLock(&m_sFreeListLock);
	InitializeCriticalSection( &m_TaskMsgVecLock );
	m_Product = NULL;
	m_IsRegist = false;
	m_sClientName[0] =0;
	m_eClientType =InvalidServer;
	m_boRegisted =0;
	m_nClientSrvIdx =0;

}

CAMClientSocket::~CAMClientSocket(void)
{
	DeleteCriticalSection( &m_TaskMsgVecLock );
	DestroyPacketList(m_sMsgList);
	DestroyPacketList(m_sFreeList);
}

VOID CAMClientSocket::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if (!m_pSrv || nCmd ==0)
	{
		return;
	}
	//__try
	{
		AddMessage(nCmd,inPacket);
	}
	//__except(DefaultExceptHandler(GetExceptionInformation()))
	{

	}
}


CDataPacket& CAMClientSocket::allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd)
{
	CDataPacket &Packet = allocSendPacket();
	PDATAHEADER pPackHdr;
	//预留通信数据头空间
	Packet.setLength(sizeof(*pPackHdr));
	Packet.setPosition(sizeof(*pPackHdr));
	pPackHdr = (PDATAHEADER)Packet.getMemoryPtr();
	pPackHdr->tag = DEFAULT_TAG_VALUE;
	//写入通信消息号
	Packet << nCmd;
	return Packet;
}

VOID  CAMClientSocket::SendKeepAlive()
{
	CDataPacket& packet = allocProtoPacket(0);
	flushProtoPacket(packet);
}

VOID CAMClientSocket::flushProtoPacket(CDataPacket& packet)
{
	PDATAHEADER pPackHdr = (PDATAHEADER)packet.getMemoryPtr();
	//计算并向协议头中写入通信数据长度
	INT_PTR nDataSize = packet.getLength() - sizeof(*pPackHdr);

	if(nDataSize > INT_MAX)
	{
		WORD   wHead= *(WORD*)((char*)packet.getMemoryPtr() + sizeof(*pPackHdr));
		OutputMsg(rmError,"严重错误CCustomJXServerClientSocket::flushProtoPacket 数据长度%d过长,head=%d",(int)nDataSize,(int)wHead);
	}
	pPackHdr->len = nDataSize;
	flushSendPacket(packet);
}

VOID CAMClientSocket::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
{
	//如果连接已断开则丢弃所有数据
	if ( !connected() )
	{
		Inherited::ProcessRecvBuffers(pDataBuffer);
		SwapRecvProcessBuffers();
		return;
	}

	if ( pDataBuffer->nOffset <= 0 )
	{
		SwapRecvProcessBuffers();
		return;
	}

	//如果客户端尚未注册则处理注册数据包
	if ( !m_boRegisted )
	{
		PSERVER_REGDATA pRegData;
		if ( pDataBuffer->nOffset >= sizeof(*pRegData) )
		{
			pRegData = (PSERVER_REGDATA)pDataBuffer->pBuffer;
			if ( !OnValidateRegData(pRegData) )
			{
				close();
				SwapRecvProcessBuffers();
				return;
			}

			m_boRegisted = 1;
			m_eClientType =(jxSrvDef::SERVERTYPE) pRegData->ServerType;
			m_nClientSrvIdx = pRegData->ServerIndex;
			strcpy(m_sClientName, pRegData->ServerName);
			
			pDataBuffer->pPointer += sizeof(*pRegData);
			//OnRegDataValidated();
			OutputMsg(rmTip, _T("[%s] client ([%s]:[%d]) registe success [%s]"), getServerTypeName(m_eClientType),
				GetRemoteHost(), GetRemotePort(), m_sClientName);
		}
	}

	jxSrvDef::INTERSRVCMD nCmd;
	INT_PTR dwRemainSize;
	PDATAHEADER pPackHdr;
	char* pDataEnd = pDataBuffer->pBuffer + pDataBuffer->nOffset;

	while ( TRUE )
	{
		dwRemainSize = (INT_PTR)(pDataEnd - pDataBuffer->pPointer);
		//如果缓冲区中的剩余长度小于通信协议头的长度，则交换缓冲并在以后继续进行处理
		if ( dwRemainSize < sizeof(*pPackHdr) )
		{
			SwapRecvProcessBuffers();
			break;
		}

		pPackHdr = (PDATAHEADER)pDataBuffer->pPointer;
		//检查包头标志是否有效，如果包头标志无效则需要遍历数据包查找包头
		if ( pPackHdr->tag != DEFAULT_TAG_VALUE )
		{
			char* sCurPtr = pDataBuffer->pPointer;
			do 
			{
				pDataBuffer->pPointer++;
				pPackHdr = (PDATAHEADER)pDataBuffer->pPointer;
				//找到包头标记则终止查找
				if ( pPackHdr->tag == DEFAULT_TAG_VALUE )
					break;
			} 
			while (pDataBuffer->pPointer < pDataEnd - 1);
			//如果无法查找到包头，则保留接收缓冲末尾的最后一个字符并交换接收/处理缓冲以便下次继续连接新接收的数据后重新查找包头
			if ( pPackHdr->tag != DEFAULT_TAG_VALUE )
			{
				SwapRecvProcessBuffers();
				//找不到协议头标志，输出错误消息
				OutputMsg(rmError, _T("sessionsrv recv invalid server data, proto header can not be found") );
				break;
			}
			//输出找到包头的消息
			OutputMsg(rmError, _T("%s recv invalid server data, proto header refound after %d bytes"), 
				getClientName(), (int)(pDataBuffer->pPointer - sCurPtr));
		}
		//如果处理接收数据的缓冲中的剩余数据长度不足协议头中的数据长度，则交换缓冲并在下次继续处理
		dwRemainSize -= sizeof(*pPackHdr);
		if ( pPackHdr->len > dwRemainSize )
		{
			SwapRecvProcessBuffers();
			break;
		}
		//将缓冲读取指针调整到下一个通信数据包的位置
		pDataBuffer->pPointer += sizeof(*pPackHdr) + pPackHdr->len;
		if ( pPackHdr->len >= sizeof(nCmd) )
		{
			//将通信数据段保存在packet中
			CDataPacketReader packet(pPackHdr + 1, pPackHdr->len);
			//分派数据包处理
			packet >> nCmd;
			OnDispatchRecvPacket(nCmd, packet);
		}
		else
		{
			OutputMsg(rmError, _T("%s recv invalid server packet, packet size to few(%d)"), 
				getClientName(), pPackHdr->len);
		}
	}
}

bool CAMClientSocket::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
{
	//游戏服和db能够登陆
	if ( pRegData && pRegData->GameType == SERVER_REGDATA::GT_JianXiaoJiangHu && 
		pRegData->ServerType == SessionServer )
	{
		return true;
	}
	else
	{
		return false;
	}
}

VOID CAMClientSocket::OnRun()
{
	Inherited::OnRun();
	//static const int ReserveSize = 256;

	if (m_sMsgList.appendCount() > 0)
		m_sMsgList.flush();
	
	while (m_sMsgList.count() > 0)
	{	
		CDataPacket* packet = m_sMsgList[0];
		//CDataPacket &tmpPacket = allocProtoPacket(AMProcto::AMC_COMSUME);
		//tmpPacket.reserve(ReserveSize);
		//tmpPacket.adjustOffset( DecodeBuffer(packet->getOffsetPtr(),tmpPacket.getOffsetPtr(), packet->getAvaliableLength(), ReserveSize-(DWORD)(tmpPacket.getOffsetPtr()-tmpPacket.getMemoryPtr())) );
		jxSrvDef::INTERSRVCMD nCmd;
		*packet >> nCmd;
		//__try
		{
			HandleAMMessage(nCmd, *packet);
		}
		//__except(DefaultExceptHandler(GetExceptionInformation()))
		{

		}
		m_sMsgList.remove(0);
		//FreePacket(&tmpPacket);	
		FreePacket(packet);			
	}
	

}

void CAMClientSocket::AddMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader& packet)
{
	CDataPacket *cache_pkg = AllocPacket();	
	if (!cache_pkg)
	{
		OutputMsg(rmError, _T("AMServer DataCacheError allocPacket failed"));
		return;
	}
	size_t nLen = packet.getAvaliableLength();
	//cache_pkg->setLength(nLen + sizeof(WORD));
	(*cache_pkg) << (nCmd);	
	//cache_pkg->writeAtom(WORD(nCmd));

	cache_pkg->writeBuf(packet.getOffsetPtr(), nLen);
	cache_pkg->setPosition(0);
	//const char* pData = cache_pkg->getMemoryPtr();
	m_sMsgList.append(cache_pkg);  
}

CDataPacket* CAMClientSocket::AllocPacket()
{
	if (m_sFreeList.count() > 0)
	{
		CDataPacket* pkg = m_sFreeList.pop();
		pkg->setLength(0);
		return pkg;
	}

	if (m_sFreeList.appendCount() > 0)
		m_sFreeList.flush();
	if (m_sFreeList.count() <= 0 )
		allocSendPacketList(m_sFreeList, 512);
	CDataPacket* pkg = m_sFreeList.pop();
	pkg->setLength(0);
	return pkg;
}

void CAMClientSocket::FreePacket(CDataPacket* packet)
{
	m_sFreeList.append(packet);
}

void CAMClientSocket::DestroyPacketList(CQueueList<CDataPacket*>& pkgList)
{
	if (pkgList.appendCount() > 0)
		pkgList.flush();

	for (INT_PTR i = pkgList.count()-1; i >= 0; i--)
	{
		CDataPacket* pkg = pkgList[i];
		pkg->~CDataPacket();
	}
	pkgList.clear();
}

void CAMClientSocket::HandleAMMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacket &packet)
{

	switch (nCmd)
	{
	case AMC_REGIST_CLIENT:
		{
			RegistClient(packet);
			break;
		}
	case AMC_QUERY_AMOUNT:
		{
			QueryAmount(packet);
			break;
		}
	case AMC_COMSUME:
		{
			Consume(packet);
			break;
		}
	case AMC_TASK_SITUATION:
		{
			SendTaskSituation(packet);
			break;
		}
	}


}

void CAMClientSocket::SendPaySucceed(int nServerIndex,UINT uUserId,UINT nActorId, UINT uMoney)
{
	static const int ReserveSize = 256;
	AMMSG msg;
	msg.nServerId = (INT)nServerIndex;
	msg.nUserId = (unsigned int)uUserId;
	msg.nResult = (INT)uMoney;
	msg.lOPPtr = (unsigned int)nActorId;

	CDataPacket &pack = allocProtoPacket(AMProcto::AMS_PAYSUCCESS);
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));

	flushProtoPacket(pack);
	OutputMsg( rmTip, "通知[成功充值]成功" );
}

void CAMClientSocket::AskTaskSituation(int nserverid, UINT uUserId, UINT cmdid, int contractid, int step )
{
	static const int ReserveSize = 256;
	TASKMSG msg;
	msg.nServerId = (INT)nserverid;
	msg.nUserId = (unsigned int)uUserId;
	msg.nCmd = (INT)cmdid;
	msg.nContractid = (INT)contractid;
	msg.nStep = (INT)step;

	CDataPacket &pack = allocProtoPacket(AMProcto::AMS_TASK_SITUATION);
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	flushProtoPacket(pack);

	if(cmdid == 1 )
		OutputMsg( rmTip, "finish task :UserID=[%u], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d]", uUserId, nserverid, cmdid, contractid, step );
	else if(cmdid == 2 )
		OutputMsg( rmTip, "if finish task, give award : UserID=[%u], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d]", uUserId, nserverid, cmdid, contractid, step );
	else if(cmdid == 3 )
		OutputMsg( rmTip, "give award :UserID=[%u], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d]", uUserId, nserverid, cmdid, contractid, step );
	else if(cmdid == 0 )
		OutputMsg( rmTip, "get QQvip award :UserID=[%u], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d]", uUserId, nserverid, cmdid, contractid, step );
}

void CAMClientSocket::AddTaskMsg(SOCKET nsocket, int nserverid, UINT uUserId, UINT cmdid, int contractid, int step )
{
	EnterCriticalSection( &m_TaskMsgVecLock );
	TASKREQ_Map::iterator it;
	it = m_TaskMap.find(uUserId);
	if(it == m_TaskMap.end() )
	{
		TASKREQ_Vec  vec;
		TASKREQ tmp;
		tmp.Serverid = nserverid;
		tmp.Socket = (int)nsocket;
		tmp.Cmdid = cmdid;
		tmp.Contractid = contractid;
		tmp.Step = step;
		vec.push_back(tmp);
		m_TaskMap.insert(TASKREQ_Map::value_type(uUserId, vec));
	}
	else
	{
		TASKREQ tmp;
		tmp.Serverid = nserverid;
		tmp.Socket = (int)nsocket;
		tmp.Cmdid = cmdid;
		tmp.Contractid = contractid;
		tmp.Step = step;
		it->second.push_back(tmp);
	}
	LeaveCriticalSection( &m_TaskMsgVecLock );
	OutputMsg( rmTip, "Accept QQTask Msg : UserID=[%u], Socket=[%d], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d]", uUserId, nsocket, nserverid , cmdid, contractid, step );
}

bool CAMClientSocket::DelTaskMsg(SOCKET nsocket, int nserverid, UINT uUserId, UINT cmdid, int contractid, int step )
{
	EnterCriticalSection( &m_TaskMsgVecLock );
	bool ret = false;
	TASKREQ_Map::iterator it;
	it = m_TaskMap.find(uUserId);
	if(it == m_TaskMap.end() )
	{
		ret = false;
	}
	else
	{
		TASKREQ tmp;
		tmp.Serverid = nserverid;
		tmp.Cmdid = cmdid;
		tmp.Contractid = contractid;
		tmp.Step = step;
		for(TASKREQ_Vec::iterator it1 = it->second.begin();  it1 != it->second.end(); )
		{
			if( *it1 == tmp )
			{
				it1 = it->second.erase(it1);
				ret = true;
				break;
			}
			else
			{
				it1++;
			}
		}
	}
	LeaveCriticalSection( &m_TaskMsgVecLock );
	return ret;
}

void CAMClientSocket::RegistClient(CDataPacket &packet)
{
	if (m_IsRegist) return;
	m_IsRegist = true;
	char buff[256];
	Decode6BitBuf( packet.getOffsetPtr(), buff, (unsigned int)packet.getAvaliableLength(), sizeof(buff) ); //解开数据
	const char *d = "/";
	char * p = strtok(buff,d);
	strncpy(m_gName, p, strlen(p));    //游戏的编号，现在一般用djrm
	p=strtok(NULL,d);
	strncpy(m_Spid, p, strlen(p));     //游戏的spid
	//packet.readString(m_gName,ArrayCount(m_gName));
	//m_gName[sizeof(m_gName)-1] =0;
	//packet.readString(m_Spid,ArrayCount(m_Spid));
	//m_Spid[sizeof(m_Spid)-1] =0;
	if (m_pSrv)
	{
		m_Product = m_pSrv->GetProduct(m_gName,m_Spid);
		if( m_Product )
		{
			OutputMsg( rmTip, "注册成功" );
			CDataPacket &pack = allocProtoPacket(AMS_REGIST_CLIENT);
			flushProtoPacket(pack);
		}
		else
			OutputMsg( rmTip, "注册失败，接受到的产品名字：[%s]，运营商ID：[%s]", m_gName, m_Spid );
	}
	else
		OutputMsg( rmTip, "产品客户端创建失败" );
}

void CAMClientSocket::QueryAmount(CDataPacket &packet)
{
	AMMSG recv_msg;
	const int ReserveSize = 256;
	DecodeBuffer(packet.getOffsetPtr(), &recv_msg, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(recv_msg));
	UINT nActorId = 0;
	int nResult = GetErrorNo();
	if (nResult == 0)
	{
		UINT uUserId = 0;
		uUserId = recv_msg.nUserId ;
		nActorId = (UINT)recv_msg.lOPPtr;
		nResult = m_Product->QueryAmount(uUserId, nActorId);
	}

	AMMSG msg;
	msg.nUserId = recv_msg.nUserId ;
	msg.nServerId = recv_msg.nServerId ;
	msg.nResult = (INT)nResult;
	msg.lOPPtr = nActorId;

	CDataPacket &pack = allocProtoPacket(AMS_QUERY_AMOUNT);
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));

	flushProtoPacket(pack);
	if (nResult > 0)
	{
		OutputMsg( rmTip, "元宝数为：[%d], 回复[查询账户数据包]成功, UserID=[%u], actorid=[%u]", (INT)nResult, recv_msg.nUserId,(UINT) recv_msg.lOPPtr);
	}
}

void CAMClientSocket::Consume(CDataPacket &packet)
{
	char recv_msg[300];
	const int ReserveSize = 256;
	memset( recv_msg, 0, sizeof(recv_msg) );
	DecodeBuffer(packet.getOffsetPtr(), recv_msg, 28, (DWORD)sizeof(recv_msg));
	packet.adjustOffset(28);

	int nResult = GetErrorNo();
	if (nResult == 0)
	{
		UINT uUserId = 0;
		UINT nServerIndex = 0;
		UINT nAmount = 0;
		UINT nActorId = 0;
		char sCharName[128];
		memset( sCharName, 0, sizeof(sCharName) );
		AMMSG * pMsg = (AMMSG *)recv_msg ;
		uUserId = pMsg->nUserId ;
		nServerIndex = pMsg->nServerId ;
		nAmount = pMsg->nResult ;
		nActorId = (UINT)pMsg->lOPPtr;
		BYTE nLevel = pMsg->nLevel;
		//char * pName = recv_msg + sizeof(AMMSG) ;
		//strncpy( sCharName,  pName, strlen(pName) );
		//sCharName[strlen(sCharName)] = 0 ;
		DecodeBuffer(packet.getOffsetPtr(), sCharName, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(sCharName));
		nResult = m_Product->Consume(uUserId,nAmount,sCharName,nServerIndex,nActorId, nLevel);
		if(nResult >= 0 )
		{
			//OutputMsg( rmTip, "提取成功，用户ID：[%u], 元宝数：[%d], 用户名：[%s], ServerID：[%d]", uUserId,nAmount,sCharName,nServerIndex );
			OutputMsg( rmTip, _T("Consume succeed, UserID:[%u], YuanBao:[%d], Account:[%s], ServerID:[%d],ActorId:[%u]"), uUserId,nAmount,sCharName,nServerIndex, nActorId);
		}
		else
		{
			//OutputMsg( rmTip, "提取失败，用户ID：[%u], 元宝数：[%d], 用户名：[%s], ServerID：[%d]", uUserId,nAmount,sCharName,nServerIndex );
			OutputMsg( rmError, _T("Consume failed, UserID:[%u], YuanBao:[%d], Account:[%s], ServerID:[%d],ActorId:[%u]"), uUserId,nAmount,sCharName,nServerIndex,nActorId);
			OutputMsg( rmError, _T("Account:[%s], Length of Account:[%d]"), sCharName,strlen(sCharName) );
		}
	}

	AMMSG msg;
	AMMSG * pMsg = (AMMSG *)recv_msg ;
	msg.nUserId = pMsg->nUserId ;
	msg.nServerId = pMsg->nServerId ;
	if(nResult >= 0 )
		msg.nResult = pMsg->nResult ;
	else
		msg.nResult = 0;
	msg.lOPPtr = pMsg->lOPPtr ;

	CDataPacket &pack = allocProtoPacket(AMS_COMSUME);
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));

	flushProtoPacket(pack);
}

LPCSTR CAMClientSocket::GetResultCode(int op)
{
	switch(op)
	{
		case 0: return "0";   // 步骤已完成 或 奖励发放成功
		case 1: return "1";   // 用户尚未在应用内创建角色
		case 2: return "2";   // 用户尚未完成本步骤
		case 3: return "3";   // 该步骤奖励已发放过
		case 4: return "4";   // 发货失败
	}
	return "";
}

void CAMClientSocket::SendTaskSituation(CDataPacket &packet)
{
	TASKMSG recv_msg;
	const int ReserveSize = 256;
	DecodeBuffer(packet.getOffsetPtr(), &recv_msg, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(recv_msg));

	int nResult = GetErrorNo();
	if (nResult == 0)
	{
		UINT uUserId = 0;
		uUserId = recv_msg.nUserId ;
		EnterCriticalSection( &m_TaskMsgVecLock );
		TASKREQ_Map::iterator it;
		it = m_TaskMap.find(uUserId);
		if(it != m_TaskMap.end() )
		{
			TASKREQ tmp;
			tmp.Serverid = recv_msg.nServerId;
			tmp.Cmdid = recv_msg.nCmd;
			tmp.Contractid = recv_msg.nContractid;
			tmp.Step = recv_msg.nStep;
			bool flag = true;
			for(TASKREQ_Vec::iterator it1 = it->second.begin();  it1 != it->second.end(); )
			{
				if( *it1 == tmp )
				{
					if( it1->Socket > 0 )
					{
						m_pSrv->GetParamParser().SendResponse(it1->Socket, GetResultCode(recv_msg.nResult) );
						OutputMsg( rmTip, "UserID=[%u], Socket=[%d], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d], Result=[%d]", uUserId, it1->Socket, recv_msg.nServerId, recv_msg.nCmd, recv_msg.nContractid, recv_msg.nStep, recv_msg.nResult );
#ifdef WIN32
						closesocket(it1->Socket);
#else
						::close(it1->Socket);
#endif
						it1->Socket = 0;
						it1 = it->second.erase(it1);
						flag = false;
						break;
					}
					else
					{
						OutputMsg( rmError, "Socket Error !!! UserID=[%u], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d], Result=[%d]", uUserId, recv_msg.nServerId, recv_msg.nCmd, recv_msg.nContractid, recv_msg.nStep, recv_msg.nResult );
					}
				}
				else
				{
					it1++;
				}
			}
			if(flag)
			{
				OutputMsg( rmWaning, "AM recv unknow msg, UserID=[%u], ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d], Result=[%d]", uUserId, recv_msg.nServerId, recv_msg.nCmd, recv_msg.nContractid, recv_msg.nStep, recv_msg.nResult );
			}
		}
		else
		{
			OutputMsg( rmWaning, "no UserID=[%d] in AM: ServerID=[%d], CmdID=[%d], ContractID=[%d], Step=[%d], Result=[%d]", uUserId, recv_msg.nServerId, recv_msg.nCmd, recv_msg.nContractid, recv_msg.nStep, recv_msg.nResult );
		}
		LeaveCriticalSection( &m_TaskMsgVecLock );
	}
}

int CAMClientSocket::GetErrorNo()
{
	if (!m_IsRegist)
	{
		OutputMsg( rmError, "未注册");
		return ERROR_NOT_REGDIST;
	}
	if (!m_Product)
	{
		OutputMsg( rmError, "无法找到对应的产品");
		return ERROR_NOTFIND_PRODUCT;
	}
	return 0;
}
