#include "StdAfx.h"

#define GUILD_NAME_MAX_LEN 7

CDBDataClient::CDBDataClient(CDBDataServer *lpDataServer, CSQLConenction *lpSQLConnection, SOCKET nSocket, PSOCKADDR_IN pAddrIn)
:Inherited()
{
	SetClientSocket(nSocket, pAddrIn);	
	m_pDataServer = lpDataServer;
	m_pSQLConnection = lpSQLConnection;
	m_dwClosedTick = 0;
	m_ASyncNameOPList.setLock(&m_ASyncNameOPListLock);
	m_pHugeSQLBuffer = (PCHAR)m_Allocator.AllocBuffer(dwHugeSQLBufferSize);	
	// m_sDataHandler.Start(this, m_pSQLConnection);
	m_loginHandler.Init(m_pSQLConnection,m_pDataServer);

}
void CDBDataClient::SetNewSQLConnection(CSQLConenction *lpSQLConnection)
{
	m_sDataHandler.Start(this, lpSQLConnection);	
}

CDBDataClient::~CDBDataClient()
{	
	m_Allocator.FreeBuffer(m_pHugeSQLBuffer);
	ClearASyncNameOPList();	
	m_sDataHandler.Stop();
}

CDataPacket& CDBDataClient::AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd)
{
	return allocProtoPacket(nCmd);
}

void CDBDataClient::FlushDataPacket(CDataPacket &packet)
{
	flushProtoPacket(packet);
}

VOID CDBDataClient::OnDisconnected()
{
	m_dwClosedTick = _getTickCount();
	OutputMsg( rmWaning, _T("数据客户端(%s:%d)连接已断开"), GetRemoteHost(), GetRemotePort() );
	
	//不能调用父类的断开连接函数，因为希望在连接断开后继续保留套接字中的数据以便处理	
	// TODO.更新连接的逻辑服务器列表	
}

VOID CDBDataClient::Disconnected()
{
	Inherited::Disconnected();
	if (m_pDataServer)
		m_pDataServer->UpdateLogicClientList();
}

VOID CDBDataClient::OnError(INT errorCode)
{
	//输出错误消息
	OutputError( errorCode, _T("数据客户端(%s:%d)套接字发生错误"), GetRemoteHost(), GetRemotePort() );
	//断开连接
	close();
}


VOID CDBDataClient::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	TICKCOUNT nTick;

	switch(uMsg)
	{
	case DCIM_CREATE_ACTOR_RESULT:
		//ProcessCreateActor((INT_PTR)uParam2,(unsigned int)uParam3,(PCREATEACTORDATA)uParam4);
		m_loginHandler.OnCreateActorResult((INT_PTR)uParam1,(unsigned int)uParam2,(PCREATEACTORDATA)uParam3,this );
		break;
	case DCIM_CREATE_CS_ACTOR_RESULT:
		m_loginHandler.OnCreateCrossServerActorResult((INT_PTR)uParam1,(unsigned int)uParam2,(PCREATEACTORDATA)uParam3,this );
		break;
	case DCIM_RENAMECHAR_RESULT:
		//名称客户端返回重命名角色的结果(Param1=操作结果,Param2=申请更名操作时传递的操作唯一标识)
		ProcessRenameCharResult((PASYNCNAMEOPDATA)uParam2, (int)uParam1);
		break;
	case DCIM_ALLOC_GUILID_RESULT:
		//名称客户端返回申请帮会ID的结果(Param1=操作结果,Param2=帮会ID，Param3=申请帮会ID操作时传递的操作唯一标识)
		ProcessAllocGuildResult((PASYNCNAMEOPDATA)uParam3, (unsigned int)uParam1, (unsigned int)uParam2);
		break;
		
	case DCIM_SEND_LOAD_ACTOR:
		//发送认证信息
		nTick = _getTickCount();
		
		//输出这一步到服务器浪费的时间
		OutputMsg(rmTip,"actorid=%lld,CDBDataClient send loadactor to ligic, pass=%d ms",uParam2, (int)(nTick -uParam3 ));

		CDataPacket& retPack = allocProtoPacket(dcLoadActor);
		retPack << (unsigned int)uParam1;//sessionid,见LOADACTORSTRUCT定义，是unsigned int类型
		retPack << (unsigned int)uParam2;//角色id
		flushProtoPacket(retPack);
		break;
	}
	
}

VOID CDBDataClient::OnRun()
{
}

void CDBDataClient::OnDispatchRecvPacketEx(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader& inPacket)
{
	
	//是登陆的消息包，要最优先处理，让玩家迅速能够拿到角色列表
	if(nCmd >= dcGetActorList && nCmd < dcLoginMsgMax)
	{
		
		//CDataPacket& out = allocProtoPacket(nCmd);
		m_loginHandler.OnRecvLoginMsg(nCmd,inPacket,this); //处理登陆消息，不做缓存，立刻处理
		//flushProtoPacket(out);
		return;
	}
	else
	{
		m_sDataHandler.AddMessage(nCmd, inPacket);
	}

	
}

VOID CDBDataClient::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	if (nCmd == 999)
		OutputMsg(rmNormal, _T("%s recv InitDB Msg"), __FUNCTION__);
	if (nCmd == dcQuery)
		OutputMsg(rmNormal, _T("recv query actor msg"));

	return OnDispatchRecvPacketEx(nCmd, inPacket);
}

VOID CDBDataClient::DefaultPacket(CDataPacketReader &inPacket)
{
}

VOID CDBDataClient::ClearASyncNameOPList()
{
	INT_PTR i;
	PASYNCNAMEOPDATA pNameOPData;

	m_ASyncNameOPList.lock();
	for ( i=m_ASyncNameOPList.count()-1; i>-1; --i )
	{
		pNameOPData = m_ASyncNameOPList[i];
		m_Allocator.FreeBuffer(m_ASyncNameOPList);
	}
	m_ASyncNameOPList.clear();
	m_ASyncNameOPList.unlock();
}

CDBDataClient::PASYNCNAMEOPDATA CDBDataClient::PeekASyncNameOPData(PASYNCNAMEOPDATA lpOPData)
{
	INT_PTR nIndex = m_ASyncNameOPList.index(lpOPData);
	if ( nIndex > - 1 )
	{
		m_ASyncNameOPList.remove(nIndex);
		return lpOPData;
	}
	return NULL;
}

INT_PTR CDBDataClient::RenameCharOnDatabase(const INT_PTR nCharId, LPCSTR sNewCharName)
{
	INT_PTR nError = 0;

	nError = m_pSQLConnection->Query(szSQLSP_RenameCharactor, (unsigned int)nCharId, sNewCharName);
	if ( !nError )
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		if ( !pRow || !pRow[0] )
			nError = 1;
		else if ( pRow[0][0] == '0' )
			nError = 2;
		m_pSQLConnection->ResetQuery();
	}
	else nError = 1;
	

	return nError;
}

VOID CDBDataClient::ProcessRenameCharResult(PASYNCNAMEOPDATA lpOPData, const INT_PTR nRenameError)
{
	INT_PTR nError;

	//如果更名操作数据不存在于此客户端中则不处理（因为允许有多个客户端连接具有同一个服务器ID）
	if ( !PeekASyncNameOPData(lpOPData) )
		return;

	//更名成功
	if ( nRenameError == jxInterSrvComm::NameServerProto::neSuccess )
	{
		//如果数据库连接未就绪则不予处理
		if ( !m_pSQLConnection->Connected() )
		{
			nError = aeLocalInternalError;
			//打印错误消息
			OutputMsg( rmError, _T("数据客户端(%s,%d, %s:%d)申请对角色(%d)改名的操作已经被名称服务器接受，")
				_T("但当前数据库连接尚未就绪"), getClientName(), getClientServerIndex(), GetRemoteHost(), GetRemotePort(), 
				lpOPData->RenameChar.nCharId );
		}
		else
		{
			nError = RenameCharOnDatabase(lpOPData->RenameChar.nCharId, lpOPData->RenameChar.sCharName );
			if ( nError != 0 )
			{
				if ( nError == 1 )//数据库查询失败，当作为服务器内部错误返回
					nError = aeLocalDataBaseError;
				else if ( nError == 2 )//查询返回失败值，当作为角色名称已经存在错误返回
					nError = aeNameInuse;
				else nError = aeLocalInternalError;//如果添加了新的错误且未修改此代码，则全部当做服务器内部错误返回
			}
		}
	}
	//更名失败则转换错误号
	else if ( nRenameError == jxInterSrvComm::NameServerProto::neInvalidName )
		nError = aeInvalidName;
	else if ( nRenameError == jxInterSrvComm::NameServerProto::neTimedOut )
		nError = aeTimedOut;
	else if ( nRenameError == jxInterSrvComm::NameServerProto::neDataBaseError )
		nError = aeServerDataBaseError;
	else nError = aeServerInternalError;

	//返回数据客户端操作结果
#pragma __CPMSG__(返回逻辑服务器，对角色进行改名的结果，nError为表0表示成功，否则表示错误号)

	//释放更名操作占用的内存
	m_Allocator.FreeBuffer(lpOPData);
}





void CDBDataClient::SendAddGuildResult(int nRawServerId, int nLoginServerId,
	int nErr,int index,Uint64 hHandle,unsigned int nGid,LPCSTR sGuildName,LPCSTR sFoundName,BYTE nZY,INT_PTR nIcon)
{
	CDataPacket& out = allocProtoPacket(dcAddGuild);
	out << nRawServerId << nLoginServerId;
	out << (BYTE)nErr;//内部错误
	out << (int)index;
	out << (Uint64)hHandle;
	if (nErr == jxSrvDef::aeSuccess)
	{
		out << (unsigned int)nGid;
		out.writeString(sGuildName);
		out.writeString(sFoundName);
	}
	flushProtoPacket(out);
}

VOID CDBDataClient::ProcessAllocGuildResult(PASYNCNAMEOPDATA lpOPData, const INT_PTR nResult, INT_PTR nGuildId)
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR nError = 0;

	if(lpOPData ==NULL) return;

	//如果创建帮会操作数据不存在于此客户端中则不处理（因为允许有多个客户端连接具有同一个服务器ID）
	if ( !PeekASyncNameOPData(lpOPData) )
		return;
	

	//申请帮会ID成功
	if ( nResult == jxInterSrvComm::NameServerProto::neSuccess )
	{
		//如果数据库连接未就绪则不予处理
		if ( !m_pSQLConnection->Connected() )
		{
			nError = reDbErr;
			//打印错误消息
			OutputMsg( rmError, _T("数据客户端(%s:%d, %s:%d)申请创建的帮会(%s)已经被名称服务器分配ID(%u)，")
				_T("但当前数据库连接尚未就绪"), getClientName(), getClientServerIndex(), GetRemoteHost(), GetRemotePort(), 
				lpOPData->CreateGuild.sGuildName, nGuildId );
		}
		else
		{
			//如果存在了这个名字，则不创建
			if(m_pDataServer->GetJobZyMgr()->IsGuildNameExist(lpOPData->CreateGuild.sGuildName,lpOPData->CreateGuild.nServerIndex)  )
			{
				nError = reNameInUse;
			}
			else
			{
				nError = m_pSQLConnection->Exec(szSQLSP_CreateGuild,
					(unsigned int)nGuildId,
					lpOPData->CreateGuild.nActorid,
					lpOPData->CreateGuild.sGuildName,
					lpOPData->CreateGuild.sFoundName,
					lpOPData->CreateGuild.nServerIndex);

				if ( nError != 0 )
				{
					if ( nError == 1 )//数据库查询失败，当作为服务器内部错误返回
						nError = reDbErr;
					else if ( nError == 2 )//查询返回失败值，当作为角色名称已经存在错误返回
						nError = reNameInUse;
					else nError = reInterErr;//如果添加了新的错误且未修改此代码，则全部当做服务器内部错误返回
				}
				else
				{
					m_pSQLConnection->ResetQuery();
					//创建成功
					m_pDataServer->GetJobZyMgr()->OnNewGuildNameCreate(lpOPData->CreateGuild.nServerIndex,lpOPData->CreateGuild.sGuildName, (unsigned int)nGuildId);
					nError = reSucc;//正确
				}
			}
			
		}
	}
	//申请帮会ID失败则转换错误号
	else if ( nResult == jxInterSrvComm::NameServerProto::neInvalidName )
		nError = reNameError;
	else if ( nResult == jxInterSrvComm::NameServerProto::neTimedOut )
		nError = reTimeOut;
	else if ( nResult == jxInterSrvComm::NameServerProto::neDataBaseError )
		nError = reDbErr;
	else if (nResult == jxInterSrvComm::NameServerProto::neNameinuse)
		nError = reNameInUse;
	else nError = reInterErr;

	//返回数据客户端操作结果
	SendAddGuildResult(0, 0, (int)nError,lpOPData->CreateGuild.nIndex,lpOPData->CreateGuild.hActor,(unsigned int)nGuildId,lpOPData->CreateGuild.sGuildName,
		lpOPData->CreateGuild.sFoundName,lpOPData->CreateGuild.nZY,lpOPData->CreateGuild.wIcon);

	//释放创建帮会操作占用的内存
	m_Allocator.FreeBuffer(lpOPData);
}

VOID CDBDataClient::DispatchRecvMsg(LPCSTR lpData, SIZE_T dwSize)
{
#pragma __CPMSG__(拆分单个通信数据包进行数据处理)
}

int CDBDataClient::CatchRenameChar(LPCSTR lpData, SIZE_T dwSize)
{	
	unsigned int nCharId = 0;
	INT_PTR nError = 0;
	char sNewCharName[128];

	//如果数据库尚未连接则无法处理
	if ( !m_pSQLConnection->Connected() )
	{
		OutputMsg(rmError, _T("数据客户端(%s:%d, %s:%d)请求重命名角色，但数据库连接尚未就绪"), 
			getClientName(), getClientServerIndex(), GetRemoteHost(), GetRemotePort() );
		//返回数据客户端服务器内部错误的消息
		return reDbErr;
	}

	//解码新角色名
	sprintf(sNewCharName,"%s",lpData);
	//检查数据合法性，不合法则返回ASYNCRERR_INVALIDDATA错误
	if ( !sNewCharName[0] || strlen(sNewCharName) < 4 )
	{
		return reNameError;
	}

	//将角色名称的第一个字符之后的字母转换为小写
	//m_pDataServer->GetDBEngine()->LowerCaseNameStr(sNewCharName,sizeof(sNewCharName));

	//从本地数据库检查角色名称是否存在
	int nServerIndex = 0;
	nError = m_pSQLConnection->Query(szSQLSP_GetCharactorIdByName, sNewCharName,nServerIndex);//nServerIndex应该由逻辑服务器传递过来
	if ( !nError )
	{
		if ( m_pSQLConnection->CurrentRow() )
			nError = aeNameInuse;
		m_pSQLConnection->ResetQuery();
	}
	else nError = reDbErr;//查询失败返回服务器内部错误
	
	//发生错误则返回
	if ( nError )
	{
		return reDbErr;
	}

	//通过需要通过名称服务器修改角色名称
	//建立更名操作数据
	PASYNCNAMEOPDATA pASyncNameOPData = (PASYNCNAMEOPDATA)m_Allocator.AllocBuffer(sizeof(*pASyncNameOPData));
	pASyncNameOPData->RenameChar.nCharId = nCharId;
	_asncpytA(pASyncNameOPData->RenameChar.sCharName, sNewCharName);
	m_ASyncNameOPList.add(pASyncNameOPData);

	//向名称同步客户端投递申请角色更名
	m_pDataServer->GetDBEngine()->getNameSyncClient()->PostCharRename(nCharId, getClientServerIndex(), 
		sNewCharName, (UINT_PTR)pASyncNameOPData);

	return reSucc;
}

int CDBDataClient::CatchRemoveGuild(int Guild)
{ 
	int nError = 0;
	
	//如果行会名字存在的话
	if (!m_pDataServer->GetJobZyMgr()->RemoveGuildById(Guild) )
	{
		return jxInterSrvComm::DbServerProto::reInterErr;
	}

	return aeSuccess;
}
//处理创建帮会
int CDBDataClient::CatchCreateGuild(int nActor,LPCSTR sGuildName,LPCSTR sFoundName,int nIndex,Uint64 hActor,BYTE nZY,INT_PTR nIcon)
{
	DECLARE_FUN_TIME_PROF()
	int nError = 0;
	CREATEGUILD_DATA CreateGuildData;//创建帮会的数据结构
	
	//如果数据库尚未连接则无法处理
	if ( !m_pSQLConnection->Connected() )
	{
		OutputMsg(rmError, _T("数据客户端(%s:%d, %s:%d)请求创建帮会，但数据库连接尚未就绪"), 
			getClientName(), getClientServerIndex(), GetRemoteHost(), GetRemotePort() );
		return aeLocalDataBaseError;
	}

	//解码新角色名
	//sprintf_s(CreateGuildData.sGuildName,"%s",sGuildName);
	//sprintf_s(CreateGuildData.sFoundName,"%s",sFoundName);
	memcpy(CreateGuildData.sGuildName,sGuildName,sizeof(CreateGuildData.sGuildName));
	memcpy(CreateGuildData.sFoundName,sFoundName,sizeof(CreateGuildData.sFoundName));
	CreateGuildData.nZY = nZY;
	CreateGuildData.nIndex = nIndex;
	CreateGuildData.hActor = hActor;
	CreateGuildData.nActorid = nActor;
	CreateGuildData.nServerIndex = getClientServerIndex();
	CreateGuildData.wIcon =(WORD) nIcon;

	
	//检查数据合法性，不合法则返回ASYNCRERR_INVALIDDATA错误
	if ( !CreateGuildData.sGuildName[0] || strlen(CreateGuildData.sGuildName) < 1 )
	{
		return jxInterSrvComm::DbServerProto::reNameError;
	}

	//将角色名称的第一个字符之后的字母转换为小写
	//m_pDataServer->GetDBEngine()->LowerCaseNameStr(CreateGuildData.sGuildName,sizeof(CreateGuildData.sGuildName));

	INT_PTR nLen = CDBServer::GetStrLenUtf8(CreateGuildData.sGuildName);

	//校验行会名称最大长度 
	if ( nLen <= 0 || nLen > GUILD_NAME_MAX_LEN || !m_pDataServer->GetDBEngine()->CheckNameStr(CreateGuildData.sGuildName))
	{
		OutputMsg(rmError,_T("guild name is invalid!name=%s"),CreateGuildData.sGuildName);
		return jxInterSrvComm::DbServerProto::reNameError;
	}
	int nServerIndex = getClientServerIndex();
	//如果行会名字存在的话
	if (m_pDataServer->GetJobZyMgr()->IsGuildNameExist(CreateGuildData.sGuildName,nServerIndex) )
	{
		return jxInterSrvComm::DbServerProto::reNameInUse;
	}
	
	
	
	//先通过名称服务器同步帮会名称并申请帮会ID，创建帮会的实际操作将在名称服务器返回帮会ID后进行
	//建立创建帮会操作数据
	PASYNCNAMEOPDATA pASyncNameOPData = (PASYNCNAMEOPDATA)m_Allocator.AllocBuffer(sizeof(*pASyncNameOPData));
	pASyncNameOPData->CreateGuild = CreateGuildData;
	m_ASyncNameOPList.add(pASyncNameOPData);

	//向名称同步客户端投递同步帮会名称并申请帮会ID的消息
	m_pDataServer->GetDBEngine()->getNameSyncClient()->PostAllocateGuildId(getClientServerIndex(), 
		CreateGuildData.sGuildName, (UINT_PTR)pASyncNameOPData);

	

	return aeSuccess;
}

BOOL CDBDataClient::HasRemainData()
{
	char *sptr;
	PDATABUFFER pBuffer;

	//如果有异步名称操作数据则返回TRUE
	if ( m_ASyncNameOPList.count() > 0 )
		return TRUE;

	//如果消息队列非空则仍需继续处理
	if ( getInternalMessageCount() > 0 )
		return TRUE;

	//如果网络数据包未处理完毕则返回TRUE
	pBuffer = getRecvBuffer();
	if ( pBuffer->nOffset > 0 )
	{
		if ( sptr = strchr(pBuffer->pPointer, '#') )
		{
			if ( sptr = strchr(sptr, '!') )
				return TRUE;
		}
	}
	pBuffer = getProcRecvBuffer();
	if ( pBuffer->nOffset > 0 )
	{
		if ( sptr = strchr(pBuffer->pPointer, '#') )
		{
			if ( sptr = strchr(sptr, '!') )
				return TRUE;
		}
	}

	return FALSE;
}

bool CDBDataClient::OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData)
{
	return pRegData->GameType == SERVER_REGDATA::GT_JianXiaoJiangHu && pRegData->ServerType == GameServer;	
}

void CDBDataClient::OnRegDataValidated()
{
	if (m_pDataServer)
		m_pDataServer->UpdateLogicClientList();
}