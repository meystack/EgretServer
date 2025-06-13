#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif
#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "CustomServerClientSocket.h"
#include "ServerDef.h"
#include "CommonDef.h"
//#include "../encrypt/CRC.h"
//#include "../encrypt/Encrypt.h"
//#include "../dataProcess/NetworkDataHandler.h"
#include "CustomJXServerClientSocket.h"

using namespace jxSrvDef;

CCustomJXServerClientSocket::CCustomJXServerClientSocket()
	:Inherited()
{
	m_boRegisted = false;
	m_nClientSrvIdx = 0;
	m_sClientName[0] = 0;
	m_eClientType = InvalidServer;
}
CDataPacket& CCustomJXServerClientSocket::allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd)
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
VOID  CCustomJXServerClientSocket::SendKeepAlive()
{
	CDataPacket& packet = allocProtoPacket(0);
	flushProtoPacket(packet);
}
VOID CCustomJXServerClientSocket::flushProtoPacket(CDataPacket& packet)
{
	PDATAHEADER pPackHdr = (PDATAHEADER)packet.getMemoryPtr();
	//计算并向协议头中写入通信数据长度
	INT_PTR nDataSize = packet.getLength() - sizeof(*pPackHdr);
	
	if(nDataSize >INT_MAX)
	{
		WORD   wHead= *(WORD*)((char*)packet.getMemoryPtr() + sizeof(*pPackHdr));
		OutputMsg(rmError,"严重错误CCustomJXServerClientSocket::flushProtoPacket 数据长度%d过长,head=%d",(int)nDataSize,(int)wHead);
	}
	pPackHdr->len = nDataSize;
	flushSendPacket(packet);
}

VOID CCustomJXServerClientSocket::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
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

			m_boRegisted = true;
			m_eClientType = (SERVERTYPE)pRegData->ServerType;
			m_nClientSrvIdx = pRegData->ServerIndex;
			_asncpytA(m_sClientName, pRegData->ServerName);
			pDataBuffer->pPointer += sizeof(*pRegData);
			OnRegDataValidated();
			OutputMsg(rmTip, _T("%s client (%s:%d) registe success (%s)"), getServerTypeName(m_eClientType),
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
				OutputMsg(rmError, _T("%s recv invalid server data, proto header can not be found"), getClientName());
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

VOID CCustomJXServerClientSocket::Disconnected()
{
	Inherited::Disconnected();
	m_boRegisted = false;
	m_sClientName[0] = 0;
	m_nClientSrvIdx = 0;
	m_eClientType = InvalidServer;
}
