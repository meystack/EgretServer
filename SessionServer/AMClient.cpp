#include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#include <WinSock2.h>
#include <windows.h>
#endif

#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <Lock.h>
#include <Tick.h>
#include <QueueList.h>
#include <Stream.h>
#include <RefString.hpp>
#include <CustomSocket.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "DataPacketReader.hpp"
#include "DataPacket.hpp"
#include "AppItnMsg.h"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "CommonDef.h"
#include "ServerDef.h" 
#include "EDCode.h"

#include "AMProcto.h"
#include "AMClient_tx.h"

using namespace AMProcto;
using namespace jxSrvDef;

/*
CAMClient::CAMClient()
	: Inherited()
{
	SetClientName(_T("AMC"));
	m_sProductName[0] = 0;
	m_sProviderId[0] = 0;
	m_OPResultList.setLock(&m_OPResultLock);
}

CAMClient::~CAMClient()
{
}

LPCSTR CAMClient::GetProductName()
{
	return m_sProductName;
}

VOID CAMClient::SetProductName(LPCSTR sProductName)
{
	_asncpytA(m_sProductName, sProductName);
}

LPCSTR CAMClient::GetSPID()
{
	return m_sProviderId;
}

VOID CAMClient::SetSPID(LPCSTR sSPID)
{
	_asncpytA(m_sProviderId, sSPID);
}

VOID CAMClient::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
{
	char *pBuffer, *pStart, *pEnd;
	size_t nProcessLen = 0;

	if ( pDataBuffer->nOffset > 0 )
	{
		pBuffer = pDataBuffer->pPointer;
		while ( TRUE )
		{
			//搜索包头
			if ( pBuffer[0] != '#' )
				pStart = strchr(pBuffer, '#');
			else pStart = pBuffer;
			if ( !pStart )
				break;
			//搜索包尾
			pEnd = strchr(pStart, '!');
			if ( !pEnd )
				break;

			pBuffer = pEnd + 1;
			pStart++;
			pEnd[0] = 0;
			Assert( *pBuffer || (int)(pBuffer - pDataBuffer->pBuffer) == pDataBuffer->nOffset );
			//处理数据包
			CDataPacketReader packet(pStart, pEnd - pStart);
			ProcessRecvPacket(packet);
		}
		nProcessLen = pBuffer - pDataBuffer->pPointer;
		pDataBuffer->pPointer = pBuffer;
	}

	SwapRecvProcessBuffers();
}

VOID CAMClient::SendKeepAlive()
{
	static const int ReserveSize = 512;
	AMMSG msg;
	msg.nCmd = AMProcto::AMC_KEEP_ALIVE;
	msg.nServerId = 0;
	msg.nUserId = msg.nResult = 0;
	msg.lOPPtr = 0;

	CDataPacket &pack = allocSendPacket();
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));

	flushSendPacket(pack);
}

VOID CAMClient::SendRegisteClient()
{
	static const int ReserveSize = 512;
	AMMSG msg;
	msg.nCmd = AMProcto::AMC_REGIST_CLIENT;
	msg.nServerId = 0;
	msg.nUserId = msg.nResult = 0;
	msg.lOPPtr = 0;

	CDataPacket &pack = allocSendPacket();
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	pack.adjustOffset(EncodeBuffer(m_sProductName, pack.getOffsetPtr(), (DWORD)strlen(m_sProductName), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	pack << (char)'/';
	pack.adjustOffset(EncodeBuffer(m_sProviderId, pack.getOffsetPtr(), (DWORD)strlen(m_sProviderId), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));

	flushSendPacket(pack);
}

VOID CAMClient::ProcessRecvPacket(CDataPacketReader &packet)
{
	if (packet.getLength() < 32)
	{
		TRACE(_T("[AMC]无效的数据包长度%d\n"), packet.getLength());
		return;
	}

	AMMSG msg;
	AMOPData opData;

	DecodeBuffer(packet.getOffsetPtr(), &msg, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(msg));
	packet.adjustOffset(32);
	
	switch(msg.nCmd)
	{
	case AMProcto::AMS_REGIST_CLIENT:
		OutputMsg(rmTip, _T("[AMC]regist client success!"));
		break;
	case AMProcto::AMS_KEEP_ALIVE:
		break;
	case AMProcto::AMS_QUERY_AMOUNT:
		{
			opData.opType = amQueryAmount;
			opData.nUserId = msg.nUserId;
			opData.nServerId = msg.nServerId;
			opData.nResult = msg.nResult;
			opData.lOPPtr = msg.lOPPtr;
			m_OPResultList.append(opData);
		}
		break;
	case AMProcto::AMS_COMSUME:
		{
			opData.opType = amConsume;
			opData.nUserId = msg.nUserId;
			opData.nServerId = msg.nServerId;
			opData.nResult = msg.nResult;
			opData.lOPPtr = msg.lOPPtr;
			m_OPResultList.append(opData);
		}
		break;
	case AMProcto::AMS_INVALID_CMD:
		OutputMsg(rmWaning, _T("[AMC]server does not implement CMD %d"), msg.nResult);
		break;
	default:
		OutputMsg(rmWaning, _T("[AMC]client does not implement CMD %d"), msg.nCmd);
		break;
	}
}

CDataPacket& CAMClient::allocSendPacket()
{
	CDataPacket &pack = Inherited::allocSendPacket();
	pack << (char)'#';
	return pack;
}

VOID CAMClient::flushSendPacket(CDataPacket& packet)
{
	packet << (char)'!';
	Inherited::flushSendPacket(packet);
}

INT_PTR CAMClient::GetAMOPResults(wylib::container::CBaseList<AMOPDATA> &list)
{
	m_OPResultList.flush();

	INT_PTR nCount = m_OPResultList.count();
	if (nCount > 0)
	{
		list.addList(m_OPResultList);
		m_OPResultList.trunc(0);
	}
	return nCount;
}

VOID CAMClient::PostQueryAmount(unsigned int nUserId, INT_PTR nServerId, INT64 lOPPtr)
{
	static const int ReserveSize = 512;
	AMMSG msg;
	msg.nCmd = AMProcto::AMC_QUERY_AMOUNT;
	msg.nServerId = (INT)nServerId;
	msg.nUserId = (INT)nUserId;
	msg.nResult = 0;
	msg.lOPPtr = lOPPtr;

	CDataPacket &pack = allocSendPacket();
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	
	flushSendPacket(pack);
}

VOID CAMClient::PostConsume(unsigned int nUserId, INT_PTR nServerId, INT_PTR nAmount, LPCSTR sCharName, INT64 lOPPtr)
{
	static const int ReserveSize = 512;
	AMMSG msg;
	msg.nCmd = AMProcto::AMC_COMSUME;
	msg.nServerId = (INT)nServerId;
	msg.nUserId = (INT)nUserId;
	msg.nResult = (INT)nAmount;
	msg.lOPPtr = lOPPtr;

	CDataPacket &pack = allocSendPacket();
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	if (sCharName)
	{
		pack.adjustOffset(
			EncodeBuffer(sCharName, pack.getOffsetPtr(), (DWORD)strlen(sCharName), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr() ))
			);
	}

	flushSendPacket(pack);
}
*/


CAMClient::CAMClient()
	: Inherited()
{
	SetClientName(_T("AMC"));
	m_sProductName[0] = 0;
	m_sProviderId[0] = 0;
	m_OPResultList.setLock(&m_OPResultLock);
	m_OPRTaskList.setLock(&m_OPTaskLock);
}

CAMClient::~CAMClient()
{
}

LPCSTR CAMClient::GetProductName()
{
	return m_sProductName;
}

VOID CAMClient::SetProductName(LPCSTR sProductName)
{
	_asncpytA(m_sProductName, sProductName);
}

LPCSTR CAMClient::GetSPID()
{
	return m_sProviderId;
}

VOID CAMClient::SetSPID(LPCSTR sSPID)
{
	_asncpytA(m_sProviderId, sSPID);
}

INT_PTR CAMClient::GetAMOPResults(wylib::container::CBaseList<AMOPDATA> &list)
{
	m_OPResultList.flush();

	INT_PTR nCount = m_OPResultList.count();
	if (nCount > 0)
	{
		list.addList(m_OPResultList);
		m_OPResultList.trunc(0);
	}
	return nCount;
}

INT_PTR CAMClient::GetTaskOPResults(wylib::container::CBaseList<TASKMSG> &list)
{
	m_OPRTaskList.flush();

	INT_PTR nCount = m_OPRTaskList.count();
	if (nCount > 0)
	{
		list.addList(m_OPRTaskList);
		m_OPRTaskList.trunc(0);
	}
	return nCount;
}

VOID CAMClient::PostQueryAmount(unsigned int nUserId, INT_PTR nServerId, INT64 lOPPtr)
{
	static const int ReserveSize = 256;
	
	
	AMMSG msg;
	msg.nServerId = (INT)nServerId;
	msg.nUserId = (INT)nUserId;
	msg.nResult = 0;
	msg.lOPPtr = lOPPtr;
	
	CDataPacket &pack = allocProtoPacket(AMProcto::AMC_QUERY_AMOUNT);
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	flushProtoPacket(pack);
}

VOID CAMClient::PostAmTaskData(tagTASKMsg &data)
{
	static const int ReserveSize = 256;

	CDataPacket &pack = allocProtoPacket(AMProcto::AMC_TASK_SITUATION);
	pack.reserve(ReserveSize);
	pack.adjustOffset(EncodeBuffer(&data, pack.getOffsetPtr(), (DWORD)sizeof(data), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	flushProtoPacket(pack);
}

VOID CAMClient::PostConsume(unsigned int nUserId, INT_PTR nServerId, INT_PTR nAmount, LPCSTR sCharName, INT64 lOPPtr, BYTE nLevel)
{
	static const int ReserveSize = 256;
	AMMSG msg;
	msg.nServerId = (INT)nServerId;
	msg.nUserId = (unsigned int)nUserId;
	msg.nResult = (INT)nAmount;
	msg.lOPPtr = lOPPtr;
	msg.nLevel = nLevel;
	CDataPacket &pack = allocProtoPacket(AMProcto::AMC_COMSUME);

	pack.reserve(ReserveSize);
	
	pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	if (sCharName)
	{
		pack.adjustOffset(
			EncodeBuffer(sCharName, pack.getOffsetPtr(), (DWORD)strlen(sCharName), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr() ))
			);
	}

	flushProtoPacket(pack);
}

CDataPacket& CAMClient::allocProtoPacket(int  nCmd)
{
	CDataPacket &Packet = allocSendPacket();
	PDATAHEADER pPackHdr;
	//预留通信数据头空间
	Packet.setLength(sizeof(*pPackHdr));
	Packet.setPosition(sizeof(*pPackHdr));
	pPackHdr = (PDATAHEADER)Packet.getMemoryPtr();
	pPackHdr->tag = DEFAULT_TAG_VALUE;
	//写入通信消息号
	Packet <<(unsigned short) nCmd;
	return Packet;
}

VOID CAMClient::SendKeepAlive()
{
	CDataPacket& packet = allocProtoPacket(0);
	flushProtoPacket(packet);
}

VOID CAMClient::flushProtoPacket(CDataPacket& packet)
{
	PDATAHEADER pPackHdr = (PDATAHEADER)packet.getMemoryPtr();
	//计算并向协议头中写入通信数据长度
	pPackHdr->len = packet.getLength() - sizeof(*pPackHdr);
	flushSendPacket(packet);
}


VOID CAMClient::SendRegisteClient()
{
	SERVER_REGDATA regData;

	
	
	//OutputMsg( rmTip, _T("发送注册包,name=%s,ServerType=%d，ServerIndex=%d"),
	//	getLocalServerName(), getLocalServerType(),getServerIndex());
	

	CDataPacket &regpack = allocSendPacket();
	regpack.reserve(sizeof(SERVER_REGDATA));
	regData.GameType	= SERVER_REGDATA::GT_JianXiaoJiangHu;
	regData.ServerType	= SessionServer;
	regData.ServerIndex	= 0;
	_asncpytA( regData.ServerName, "SessionSrv" );
	regpack << regData;

	flushSendPacket(regpack);


	static const int ReserveSize = 128;
	AMMSG msg;
	msg.nServerId = 0;
	msg.nUserId = msg.nResult = 0;
	msg.lOPPtr = 0;

	CDataPacket &pack = allocProtoPacket(AMProcto::AMC_REGIST_CLIENT);
	pack.reserve(ReserveSize);
	//pack.adjustOffset(EncodeBuffer(&msg, pack.getOffsetPtr(), (DWORD)sizeof(msg), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	
	char buff[128];
	strcpy( buff,m_sProductName);
	strcat(buff,"/");
	strcat(buff,m_sProviderId);

	//pack.adjustOffset(EncodeBuffer(m_sProductName, pack.getOffsetPtr(), (DWORD)strlen(m_sProductName), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) ));
	//pack << (char)'/';
	
	//char result[128];
	//int nSize =Encode6BitBuf(buff,result, (DWORD)strlen(buff),sizeof(result));
	
	int nSize = Encode6BitBuf(buff, pack.getOffsetPtr(), (DWORD)strlen(buff), ReserveSize-(DWORD)(pack.getOffsetPtr()-pack.getMemoryPtr()) );
	pack.adjustOffset(nSize);
	flushProtoPacket(pack);
	//int nSize  =strlen(buff);
	//pack.adjustOffset(nSize);

	//pack.writeBuf(buff,strlen(buff));
	
	//OutputMsg(rmTip,"DataSize=%d,write size=%d",(int)pack.getLength(),nSize); 

}


VOID CAMClient::OnDispatchRecvPacket(int nCmd, CDataPacketReader &packet) 
{
	
	AMMSG msg;
	AMOPData opData;

	
	//packet.adjustOffset(32);

	switch(nCmd)
	{
		//心跳包
	case 0:
		break;

	case AMProcto::AMS_REGIST_CLIENT:
		OutputMsg(rmTip, _T("[AMC]regist client success!"));
		break;
		
	case AMProcto::AMS_KEEP_ALIVE:
		break;

	case AMProcto::AMS_QUERY_AMOUNT:
		{
			DecodeBuffer(packet.getOffsetPtr(), &msg, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(msg));
			opData.opType = amQueryAmount;
			opData.nUserId = msg.nUserId;
			opData.nServerId = msg.nServerId;
			opData.nResult = msg.nResult;
			opData.lOPPtr = msg.lOPPtr;
			m_OPResultList.append(opData);
			if (msg.nResult > 0)
			{
				OutputMsg(rmTip,"AMS_QUERY_AMOUNT, nUserId=%d,nServerId=%d,nActorId=%u,mount=%d", msg.nUserId,msg.nServerId, msg.lOPPtr,msg.nResult);
			}
			
		}

		break;
	case AMProcto::AMS_COMSUME:
		{
			DecodeBuffer(packet.getOffsetPtr(), &msg, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(msg));
			opData.opType = amConsume;
			opData.nUserId = msg.nUserId;
			opData.nServerId = msg.nServerId;
			opData.nResult = msg.nResult;
			opData.lOPPtr = msg.lOPPtr;
			OutputMsg(rmTip,"AMS_COMSUME, nUserId=%u,nServerId=%d,nActorId=%u,mount=%d", msg.nUserId,msg.nServerId,msg.lOPPtr,msg.nResult);
			m_OPResultList.append(opData);
		}
		break;

	case AMProcto::AMS_PAYSUCCESS:
		{
			DecodeBuffer(packet.getOffsetPtr(), &msg, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(msg));
			opData.opType = amPaySucceed;
			opData.nUserId = msg.nUserId;
			opData.nServerId = msg.nServerId;
			opData.nResult = msg.nResult;
			opData.lOPPtr = msg.lOPPtr;
			OutputMsg(rmTip,"AMS_PAYSUCCESS, nUserId=%u,nServerId=%d, nActorId=%u,mount=%d", msg.nUserId,msg.nServerId,msg.lOPPtr,msg.nResult);
			m_OPResultList.append(opData);
		}
		break;
	case AMProcto::AMS_TASK_SITUATION:
		{
			TASKMSG tMsg;
			DecodeBuffer(packet.getOffsetPtr(), &tMsg, (DWORD)packet.getAvaliableLength(), (DWORD)sizeof(tMsg));
			m_OPRTaskList.append(tMsg);
		}
		break;

	case AMProcto::AMS_INVALID_CMD:
		OutputMsg(rmWaning, _T("[AMC]server does not implement CMD %d"), nCmd);
		break;
	default:
		OutputMsg(rmWaning, _T("[AMC]client does not implement CMD %d"), nCmd);
		break;
	}
}

VOID CAMClient::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
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
				OutputMsg(rmError, _T("%s recv invalid server data, proto header can not be found"), GetClientName());
				break;
			}
			//输出找到包头的消息
			OutputMsg(rmError, _T("%s recv invalid server data, proto header refound after %d bytes"), 
				GetClientName(), (int)(pDataBuffer->pPointer - sCurPtr));
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
				GetClientName(), pPackHdr->len);
		}
	}
}

