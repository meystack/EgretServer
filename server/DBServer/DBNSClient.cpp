
#include "StdAfx.h"
const CNameSyncClient::OnHandleSockPacket CNameSyncClient::NSSockPacketHandlers[] = 
{
	NULL,
};

CNameSyncClient::CNameSyncClient(CDBServer *lpDBEngine)
:Inherited()
{
	m_pDBServer = lpDBEngine;
	SetClientName(_T("名称"));
	m_NewNameOperateList.setLock(&m_NewNameOperateListLock);
}

CNameSyncClient::~CNameSyncClient()
{

}
jxSrvDef::SERVERTYPE CNameSyncClient::getLocalServerType()
{
	return DBServer;
}
LPCSTR CNameSyncClient::getLocalServerName()
{
	return m_pDBServer->getServerName();
}

//VOID CNameSyncClient::SendRegisteClient()
//{
//	SERVER_REGDATA regData;
//
//	ZeroMemory( &regData, sizeof(regData) );
//	regData.GameType	= SERVER_REGDATA::GT_JianXiaoJiangHu;
//	regData.ServerType	= getLocalServerType();
//	regData.ServerIndex	= m_pDBServer->getServerIndex();
//	_asncpytA( regData.ServerName, m_pDBServer->getServerName() );
//
//	send(&regData, sizeof(regData));
//}
/*
VOID CNameSyncClient::SendKeepAlive()
{
	flushProtoPacket(allocProtoPacket(jxInterSrvComm::NameServerProto::cKeepAlive));
}
*/


VOID CNameSyncClient::OnRun()
{
	INT_PTR i;
	NameOperateData *pNameOP;
	TICKCOUNT dwCurTick = _getTickCount();

	//如果有新的名称操作数据，则发送数据并追加到操作中列表
	if ( m_NewNameOperateList.count() > 0 )
	{
		m_NewNameOperateList.lock();
		//向名称服务器发送新增加的名称操作消息
		SendNewNameOperateRequest();
		//将新操作的数据添加到才做中的列表中
		m_NameOperatingList.addList(m_NewNameOperateList);
		m_NewNameOperateList.clear();
		m_NewNameOperateList.unlock();
	}

	//检查每个正在操作的名称请求的状态
	//必须降序循环，因为列表中的数据可能被删除
	for ( i=m_NameOperatingList.count()-1; i>-1; --i )
	{
		pNameOP = m_NameOperatingList[i];
		//检查超时
		if ( dwCurTick >= pNameOP->dwTimeOut )
		{
			NameOperateTimedOut(pNameOP);
			m_NameOperatingList.remove(i);
			m_Allocator.FreeBuffer(pNameOP);
		}
	}
}

VOID CNameSyncClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	int nId;
	NAMESERVER_OPERROR eError;
	LPCSTR sName;
	INT_PTR nIndex;
	int nServerIndex = 0;
	NameOperateData *pNameOP;

	inPacket >> nServerIndex;
	inPacket >> sName;

	switch(nCmd)
	{
	case jxInterSrvComm::NameServerProto::sAllocCharId://申请角色ID返回 TODO 创角不成功，很多情况是这个 NameServer 没有返回这个协议导致的
		{
			pNameOP = GetNameOperatePtr(NameOperateData::noAllocCharId, sName, &nIndex,nServerIndex);
			if ( pNameOP )
			{
				inPacket >> eError;
				inPacket >> nId;
				//向网关管理器投递申请角色名称返回的消息
				
				m_pDBServer->getDataServer()->PostCreateActorResult(
					pNameOP->nServerIndex, eError, nId, (PCREATEACTORDATA)pNameOP->pData);
				m_NameOperatingList.remove(nIndex);
				m_Allocator.FreeBuffer(pNameOP);
			}
		}
		break;
	case jxInterSrvComm::NameServerProto::sCrossAllocCharId://申请角色ID返回 TODO 创角不成功，很多情况是这个 NameServer 没有返回这个协议导致的
		{
			pNameOP = GetNameOperatePtr(NameOperateData::noCSAllocCharId, sName, &nIndex,nServerIndex);
			if ( pNameOP )
			{
				inPacket >> eError;
				inPacket >> nId;
				//向网关管理器投递申请角色名称返回的消息
				
				m_pDBServer->getDataServer()->PostCreateCrossServerActorResult(
					pNameOP->nServerIndex, eError, nId, (PCREATEACTORDATA)pNameOP->pData);
				m_NameOperatingList.remove(nIndex);
				m_Allocator.FreeBuffer(pNameOP);
			}
		}
		break;
	case jxInterSrvComm::NameServerProto::sAllocGuildId://申请帮会ID返回
		{
			pNameOP = GetNameOperatePtr(NameOperateData::noAllocGuildId, sName, &nIndex,nServerIndex);
			if ( pNameOP )
			{
				inPacket >> eError;
				inPacket >> nId;
				//使用一个包含2个INT的数组，[0]保存操作结果，[1]保存帮会ID
				PINT_PTR pResult = (PINT_PTR)malloc(sizeof(*pResult) * 2);
				pResult[0] = eError;
				pResult[1] = nId;
				//向数据服务器投递申请帮会ID返回的消息
				m_pDBServer->getDataServer()->PostAllocGuildNameResult(
					pNameOP->nServerIndex, pResult, pNameOP->allocGuildId.nOPIdent);
				m_NameOperatingList.remove(nIndex);
				m_Allocator.FreeBuffer(pNameOP);
			}
		}
		break;
	case jxInterSrvComm::NameServerProto::sRenameChar://重命名角色返回
		{
			pNameOP = GetNameOperatePtr(NameOperateData::noRenameChar, sName, &nIndex);
			if ( pNameOP )
			{
				inPacket >> eError;
				//向数据服务器投递角色重命名返回的消息
				m_pDBServer->getDataServer()->PostRenameCharResult(pNameOP->nServerIndex, 
					eError, pNameOP->renameChar.nOPIdent);
				m_NameOperatingList.remove(nIndex);
				m_Allocator.FreeBuffer(pNameOP);
			}
		}
		break;
	case jxInterSrvComm::NameServerProto::sRenameGuild://重命名帮会返回
		{
			pNameOP = GetNameOperatePtr(NameOperateData::noRenameGuild, sName, &nIndex);
			if ( pNameOP )
			{
				m_NameOperatingList.remove(nIndex);
				m_Allocator.FreeBuffer(pNameOP);
			}
		}
		break;
	}
}

VOID CNameSyncClient::SendNewNameOperateRequest()
{
	INT_PTR i, nCount;
	NameOperateData *pNameOP;

	nCount = m_NewNameOperateList.count();
	for ( i=0; i<nCount; ++i )
	{
		pNameOP = m_NewNameOperateList[i];

		//构造通信数据包
		switch(pNameOP->nOPType)
		{
		case NameOperateData::noAllocCharId://申请角色ID
			{
				CDataPacket &packet = allocProtoPacket(jxInterSrvComm::NameServerProto::cAllocCharId);
				packet << (int)pNameOP->nServerIndex;
				packet << pNameOP->sName;
				flushProtoPacket(packet);
			}
			break;
		case NameOperateData::noCSAllocCharId://申请跨服角色ID
			{
				CDataPacket &packet = allocProtoPacket(jxInterSrvComm::NameServerProto::cCrossAllocCharId);
				packet << (int)pNameOP->nServerIndex;
				packet << pNameOP->sName;
				flushProtoPacket(packet);
			}
			break;
		case NameOperateData::noAllocGuildId://申请帮会ID
			{
				CDataPacket &packet = allocProtoPacket(jxInterSrvComm::NameServerProto::cAllocGuildId);
				packet << (int)pNameOP->nServerIndex;
				packet << pNameOP->sName;
				flushProtoPacket(packet);
			}
			break;
		case NameOperateData::noRenameChar://重命名角色
			{
				CDataPacket &packet = allocProtoPacket(jxInterSrvComm::NameServerProto::cRenameChar);
				packet << pNameOP->renameChar.nCharId;
				packet << pNameOP->sName;
				flushProtoPacket(packet);
			}
			break;
		case NameOperateData::noRenameGuild://重命名帮会
			{
				CDataPacket &packet = allocProtoPacket(jxInterSrvComm::NameServerProto::cRenameGuild);
				packet << pNameOP->guildRename.nGuildId;
				packet << pNameOP->sName;
				flushProtoPacket(packet);
			}
			break;
		default:
			break;
		}
	}
}

CNameSyncClient::NameOperateData* CNameSyncClient::GetNameOperatePtr(
	const NameOperateData::eNameOperateType eOPType, LPCSTR sName, OUT PINT_PTR lpIndex,int nServerIndex)
{

	INT_PTR i;
	NameOperateData *pNameOP;

	//检查每个正在操作的名称请求的状态
	//必须降序循环，因为列表中的数据可能被删除
	for ( i=m_NameOperatingList.count()-1; i>-1; --i )
	{
		pNameOP = m_NameOperatingList[i];
		if ( pNameOP->nOPType == eOPType && !_stricmp(sName, pNameOP->sName) &&
			(nServerIndex == -1 || nServerIndex == pNameOP->nServerIndex))
		{
			if ( lpIndex ) *lpIndex = i;
			return pNameOP;
		}
	}

	return NULL;
}

VOID CNameSyncClient::NameOperateTimedOut(const NameOperateData* pNameOP)
{
	//构造通信数据包
	switch(pNameOP->nOPType)
	{
	case NameOperateData::noAllocCharId://申请角色ID
		{
			//向网关管理器投递申请角色名称返回超时的结果
			/////m_pDBServer->getGateManager()->PostAllocCharIdResult(pNameOP->allocCharId.nSesionId, neTimedOut, 0);
		}
		break;
	case NameOperateData::noAllocGuildId://申请帮会ID
		{
			//向数据服务器投递申请帮会ID色超时的结果
			//使用一个包含2个INT_PTR的数组，[0]保存操作结果，[1]保存帮会ID
			PINT_PTR pResult = (PINT_PTR)malloc(sizeof(*pResult) * 2);
			pResult[0] = aeTimedOut;
			pResult[1] = 0;
			m_pDBServer->getDataServer()->PostAllocGuildNameResult(pNameOP->nServerIndex, 
				pResult, pNameOP->allocGuildId.nOPIdent);
		}
		break;
	case NameOperateData::noRenameChar://重命名角色
		{
			//向数据服务器投递重命名角色超时的结果
			m_pDBServer->getDataServer()->PostRenameCharResult(pNameOP->nServerIndex, 
				neTimedOut, pNameOP->renameChar.nOPIdent);
		}
		break;
	case NameOperateData::noRenameGuild://重命名帮会
		break;
	}
}

BOOL CNameSyncClient::PostAllocateCharId(const int nSessionId, const int nServerIndex, LPCSTR sCharName,CREATEACTORDATA *pData)
{
	if ( !connected() )
	{
		OutputMsg(rmError,"PostAllocateCharId fail, name server not connect");
		return FALSE;
	}

	//构造名称操作数据
	NameOperateData *pNameOP = (NameOperateData*)m_Allocator.AllocBuffer(sizeof(*pNameOP));
	pNameOP->nOPType = NameOperateData::noAllocCharId;
	pNameOP->nServerIndex = nServerIndex;
	_asncpytA(pNameOP->sName, sCharName);
	pNameOP->dwTimeOut = _getTickCount() + 10 * 1000;
	pNameOP->allocCharId.nSesionId = nSessionId;
	pNameOP->pData = (void *)pData;
	
	//添加新的名称操作消息
	m_NewNameOperateList.lock();
	m_NewNameOperateList.add(pNameOP);
	m_NewNameOperateList.unlock();
	TRACE("%d,%d,%s",nSessionId,nServerIndex,sCharName);
	return TRUE;
}



BOOL CNameSyncClient::PostCrossServerAllocateCharId(const int nSessionId, const int nServerIndex, LPCSTR sCharName,CREATEACTORDATA *pData)
{
	if ( !connected() )
	{
		OutputMsg(rmError,"PostCrossServerAllocateCharId fail, name server not connect");
		return FALSE;
	}

	//构造名称操作数据
	NameOperateData *pNameOP = (NameOperateData*)m_Allocator.AllocBuffer(sizeof(*pNameOP));
	pNameOP->nOPType = NameOperateData::noCSAllocCharId;
	pNameOP->nServerIndex = nServerIndex;
	_asncpytA(pNameOP->sName, sCharName);
	pNameOP->dwTimeOut = _getTickCount() + 10 * 1000;
	pNameOP->allocCharId.nSesionId = nSessionId;
	pNameOP->pData = (void *)pData;
	
	//添加新的名称操作消息
	m_NewNameOperateList.lock();
	m_NewNameOperateList.add(pNameOP);
	m_NewNameOperateList.unlock();
	TRACE("%d,%d,%s",nSessionId,nServerIndex,sCharName);
	return TRUE;
}

BOOL CNameSyncClient::PostAllocateGuildId(const int nServerIndex, LPCSTR sGuildName, const UINT_PTR nOPIdent)
{
	if ( !connected() )
		return FALSE;

	//构造名称操作数据
	NameOperateData *pNameOP = (NameOperateData*)m_Allocator.AllocBuffer(sizeof(*pNameOP));
	pNameOP->nOPType = NameOperateData::noAllocGuildId;
	pNameOP->nServerIndex = nServerIndex;
	_asncpytA(pNameOP->sName, sGuildName);
	pNameOP->dwTimeOut = _getTickCount() + 10 * 1000;
	pNameOP->allocGuildId.nOPIdent = nOPIdent;

	//添加新的名称操作消息
	m_NewNameOperateList.lock();
	m_NewNameOperateList.add(pNameOP);
	m_NewNameOperateList.unlock();

	return TRUE;
}

BOOL CNameSyncClient::PostCharRename(const int nCharId, const int nServerIndex, LPCSTR sNewCharName, const UINT_PTR nOPIdent)
{
	if ( !connected() )
		return FALSE;

	//构造名称操作数据
	NameOperateData *pNameOP = (NameOperateData*)m_Allocator.AllocBuffer(sizeof(*pNameOP));
	pNameOP->nOPType = NameOperateData::noRenameChar;
	pNameOP->nServerIndex = nServerIndex;
	_asncpytA(pNameOP->sName, sNewCharName);
	pNameOP->dwTimeOut = _getTickCount() + 10 * 1000;
	pNameOP->renameChar.nCharId = nCharId;
	pNameOP->renameChar.nOPIdent = nOPIdent;

	//添加新的名称操作消息
	m_NewNameOperateList.lock();
	m_NewNameOperateList.add(pNameOP);
	m_NewNameOperateList.unlock();

	return TRUE;
}

