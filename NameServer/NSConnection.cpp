//#include <new>
#include "StdAfx.h"

using namespace jxSrvDef;
using namespace jxInterSrvComm::NameServerProto;

const CNameServerClientConnection::OnHandleSockPacket CNameServerClientConnection::SockPacketHandlers[] = 
{
	&CNameServerClientConnection::DefaultHandle,
	&CNameServerClientConnection::DefaultHandle,
	&CNameServerClientConnection::AllocCharIdHandle,
	&CNameServerClientConnection::AllocGuildIdHandle,
	&CNameServerClientConnection::RenameCharHandle,
	&CNameServerClientConnection::RenameGuildHandle,
	&CNameServerClientConnection::AllocCrossServerCharIdHandle
};

CNameServerClientConnection::CNameServerClientConnection(CNameServer *lpNameServer)
	:Inherited()
{
	m_pNameServer = lpNameServer;
	m_pSQLConnection = &lpNameServer->getSQLConnection();
}

CNameServerClientConnection::~CNameServerClientConnection()
{

}
/*
//VOID CNameServerClientConnection::SendKeepAlive()
//{
//	if ( !registed() )
//		return;
//	flushProtoPacket(allocProtoPacket(jxInterSrvComm::NameServerProto::sKeepAlive));
//}
*/

VOID CNameServerClientConnection::OnDisconnected()
{
	Inherited::OnConnected();
	//释放一次内存池中的内存
	m_Allocator.CheckFreeBuffers();
	OutputMsg(rmWaning, _T("Name client %s(%s:%d) connection closed"), 
		registed() ? getClientName() : _T(""), GetRemoteHost(), GetRemotePort());
}

VOID CNameServerClientConnection::OnError(INT errorCode)
{
	OutputError(errorCode, _T("Name client %s(%s:%d) connection errro"), 
		registed() ? getClientName() : _T(""), GetRemoteHost(), GetRemotePort());
}
								  
VOID CNameServerClientConnection::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	if ( nCmd >= ArrayCount(SockPacketHandlers) )
	{
		OutputMsg(rmWaning, _T("NameClient[%s:%d(%s)]has sent invalid packet(%d)，datalen：%d"),
			GetRemoteHost(), GetRemotePort(), getClientName(), nCmd, inPacket.getLength());
	}
	else
	{
		(this->*SockPacketHandlers[nCmd])(inPacket);
	}
}

bool CNameServerClientConnection::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
{
	if ( pRegData->GameType != SERVER_REGDATA::GT_JianXiaoJiangHu )
		return false;
	if ( pRegData->ServerType == DBServer || pRegData->ServerType == GameServer )  
		return true;
	return false;
}

VOID CNameServerClientConnection::DefaultHandle(CDataPacketReader &inPacket)
{

}

VOID CNameServerClientConnection::AllocCharIdHandle(CDataPacketReader &inPacket)
{
	//通用申请ID和名称的处理
	CommAllocIdProc(inPacket, sAllocCharId, NSSQL_AllocNewCharId);
}

VOID CNameServerClientConnection::AllocGuildIdHandle(CDataPacketReader &inPacket)
{
	//通用申请ID和名称的处理
	CommAllocIdProc(inPacket, sAllocGuildId, NSSQL_AllocNewGuildId);
}

VOID CNameServerClientConnection::RenameCharHandle(CDataPacketReader &inPacket)
{
	CommRenameProc(inPacket, sRenameChar, NSSQL_RenameGuild);
}

VOID CNameServerClientConnection::RenameGuildHandle(CDataPacketReader &inPacket)
{
	CommRenameProc(inPacket, sRenameGuild, NSSQL_RenameGuild);
}


VOID CNameServerClientConnection::AllocCrossServerCharIdHandle(CDataPacketReader &inPacket)
{
	//通用申请ID和名称的处理
	CommAllocIdProc(inPacket, sCrossAllocCharId, NSSQL_AllocNewCharId);
}

VOID CNameServerClientConnection::CommAllocIdProc(CDataPacketReader &inPacket, int nRetCmd, LPCSTR sSQLCall)
{
	LPCSTR sName;
	int nServerIndex = 0;
	inPacket >> nServerIndex;
	inPacket >> sName;

	CDataPacket &retPack = allocProtoPacket(nRetCmd);
	retPack << nServerIndex;
	retPack << sName;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		retPack << neDataBaseError;
		retPack << 0;
	}
	

	//向数据库中添加角色名字并申请ID
	int nError = m_pSQLConnection->Query(sSQLCall, sName,nServerIndex);
	if ( !nError )
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		if ( pRow && pRow[0] )
		{
			unsigned int nId = 0;
			sscanf(pRow[0], "%u", &nId);
			retPack << neSuccess;
			retPack << nId;
			if(nRetCmd == sAllocCharId)
			{
				OutputMsg(rmTip,"server[%d], actorname[%s],id[%u]",(int)nServerIndex,sName,(unsigned int)nId);
			}
			else
			{
				OutputMsg(rmTip,"server[%d], guildname[%s],id[%u]",(int)nServerIndex,sName,(unsigned int)nId);
			}
			
		}
		else
		{
			if(nRetCmd == sAllocCharId)
			{
				OutputMsg(rmTip,"server[%d],actorname[%s] fail",(int)nServerIndex,sName);
			}
			else
			{
				OutputMsg(rmTip,"server[%d],guildname[%s] fail",(int)nServerIndex,sName);
			}
			
			//名称已被使用
			retPack << neNameinuse;
			retPack << 0;
		}
		m_pSQLConnection->ResetQuery();
	}
	else
	{
		retPack << neDataBaseCallError;
		retPack << 0;
	}
		
	
	flushProtoPacket(retPack);
}

VOID CNameServerClientConnection::CommRenameProc(CDataPacketReader &inPacket, int nRetCmd, LPCSTR sSQLCall)
{
	int nId;
	LPCSTR sNewName;
	inPacket >> nId;
	inPacket >> sNewName;

	CDataPacket &retPack = allocProtoPacket(nRetCmd);
	retPack << sNewName;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		retPack << neDataBaseError;
		retPack << 0;
	}
	{
		//向数据库中添加角色名字并申请ID
		int nError = m_pSQLConnection->Query(sSQLCall, nId, sNewName);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow && pRow[0] )
			{
				if ( pRow[0][0] == '0' )
				{
					OutputMsg(rmError,"rename success!name=%s",sNewName);

					retPack << neSuccess;
					retPack << nId;
				}
				else
				{
					OutputMsg(rmError,"the name has been used!name=%s",sNewName);
					//名称已被使用
					retPack << neNameinuse;
					retPack << 0;
				}
			}
			else
			{
				retPack << neDataBaseCallError;
				retPack << 0;
			}
		}
		else
		{
			retPack << neDataBaseCallError;
			retPack << 0;
		}
		m_pSQLConnection->ResetQuery();
	}
	flushProtoPacket(retPack);
}
