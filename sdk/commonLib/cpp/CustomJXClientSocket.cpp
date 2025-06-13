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
#include "ServerDef.h"
#include "CommonDef.h"

//#include "../encrypt/CRC.h"
//#include "../encrypt/Encrypt.h"
//#include "../dataProcess/NetworkDataHandler.h"
#include "CustomJXClientSocket.h"

using namespace jxSrvDef;

CDataPacket& CCustomJXClientSocket::allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd)
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

VOID CCustomJXClientSocket::SendKeepAlive()
{
	CDataPacket& packet = allocProtoPacket(0);
	flushProtoPacket(packet);
}

VOID CCustomJXClientSocket::flushProtoPacket(CDataPacket& packet)
{
	PDATAHEADER pPackHdr = (PDATAHEADER)packet.getMemoryPtr();
	//计算并向协议头中写入通信数据长度
	// printf("packet.getLength:%d, *pPackHdr:%d\n",packet.getLength(), sizeof(*pPackHdr));
	// pPackHdr->len = (WORD)packet.getLength() - sizeof(*pPackHdr);
	pPackHdr->len = packet.getLength() - sizeof(*pPackHdr);
	flushSendPacket(packet);
}

int CCustomJXClientSocket::getLocalServerIndex()
{
	return 0;
}

VOID CCustomJXClientSocket::SendRegisteClient()
{
	SERVER_REGDATA regData;

	ZeroMemory( &regData, sizeof(regData) );
	regData.GameType	= SERVER_REGDATA::GT_JianXiaoJiangHu;
	regData.ServerType	= getLocalServerType();
	regData.ServerIndex	= getLocalServerIndex();
	_asncpytA( regData.ServerName, getLocalServerName() );
	//OutputMsg( rmTip, _T("发送注册包,name=%s,ServerType=%d，ServerIndex=%d"),
	//	getLocalServerName(), getLocalServerType(),getServerIndex());
	send(&regData, sizeof(regData));
}

VOID CCustomJXClientSocket::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
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
