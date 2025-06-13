#include "StdAfx.h"
#define MAX_NAME_SIZE 8
CGateDBReqestHandler::CGateDBReqestHandler()	
{
}

CGateDBReqestHandler::~CGateDBReqestHandler()	
{
}

bool CGateDBReqestHandler::Init(CSQLConenction *pConn, CDBDataServer *pServer)
{
	m_pSQLConnection = pConn;
	m_pDBServer = pServer;
	if (!pConn || !pServer)
	{
		OutputMsg(rmError, _T("%s Init Failed"), __FUNCTION__);
		return false;
	}	

	return true;
}

void CGateDBReqestHandler::OnDeleteActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient )
{
	DECLARE_FUN_TIME_PROF()
	//首先判断网关用户是否通过查询角色消息设定了全局会话ID和帐号
	
	
	int nError =0;

	unsigned int nAccountId =0;
	int nRawServerId=0; //原始的服务器编号
	int nServerId =0;  //当前的服务器编号
	UINT64 lKey =0;
	BYTE bGateIndex =0;
	unsigned int nActorId =0;
	
	//读出玩家的账户id,服务器id 以及在逻辑网关的key
	inPacket >> nRawServerId >>nServerId >> bGateIndex >> lKey >> nAccountId >> nActorId;
	char	accountName[jxSrvDef::ACCOUNT_NAME_BUFF_LENGTH];		//账户名称
	inPacket.readString(accountName,sizeof(accountName));
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nError = ERR_SQL_NOT_CONNECT;
	}
	else
	{
		MYSQL_ROW pRow;
		nError = m_pSQLConnection->Query(szSQLSP_SelecteGuildData,nActorId);
		bool errFlag = false;
		if ( !nError )
		{
			pRow = m_pSQLConnection->CurrentRow();
			if (pRow && pRow[0] && atoi(pRow[0]) >=1)
			{
				nError = ERR_GUILD;
			}
			m_pSQLConnection->ResetQuery();

		}
		else
		{
			nError = ERR_SQL;	 //执行sql错误
		}
	}
	
	

	if(nError == 0)
	{
		//查询数据库
		char szSql[100];
		sprintf(szSql,szSQLSP_ClientDeleteCharactor,nActorId,accountName);
		nError = m_pSQLConnection->RealExec(szSql,strlen(szSql));
		if (nError != 0)
		{
			nError =ERR_SQL;//-1是错误码
		}
		else
		{
			m_pSQLConnection->ResetQuery();
		
			/*
			char szAn[32];
			sprintf(szAn,"%d",ReqData.Actoid);
			
			m_pDBServer->getLogClient()->SendLoginLog(ltDelActor,pGateUser->nGlobalSessionId,pGateUser->sAccount,
				pGateUser->sIPAddr,szAn);//最后的描述字段保存用户删除的角色id
			*/

			//OutputMsg( rmTip, _T("发送删除角色列表回应包！，用户id:%d,角色：%d,错误码：0"),pGateUser->nGlobalSessionId,ReqData.Actoid);	
		}	
	}
	//向客户端发包
	CDataPacket &retPack= pDataClient->AllocDataPacket(dcDeleteActor);

	retPack << nRawServerId << nServerId << bGateIndex <<lKey << nActorId << (BYTE)nError  ;//合并起来
	pDataClient->FlushDataPacket(retPack);//下发数据

}

void CGateDBReqestHandler::OnRecvLoginMsg(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient)
{
	if(dcGetActorList ==nCmd)
	{
		unsigned int nAccountId =0;
		int nRawServerId=0; //原始的服务器编号
		int nServerId =0;  //当前的服务器编号
		UINT64 lKey =0;
		BYTE bGateIndex =0;

		//读出玩家的账户id,服务器id 以及在逻辑网关的key
		inPacket >>nRawServerId >>nServerId >> bGateIndex >> lKey >> nAccountId;

		CDataPacket &retPack= pDataClient->AllocDataPacket(nCmd);
		retPack << nRawServerId <<nServerId << bGateIndex <<lKey << nAccountId  ;//合并起来
		GetActorList(nAccountId,nRawServerId,retPack);
		//GetActorSecondPsw(nAccountId,nRawServerId,retPack);
		pDataClient->FlushDataPacket(retPack);//下发数据

	}
	else if( dcCreateActor == nCmd) //创建角色
	{
		OnCreateActor(inPacket,pDataClient);
	}
	else if(dcDeleteActor == nCmd)
	{
		OnDeleteActor(inPacket,pDataClient);
	}
	else if(dcGetRandName ==nCmd)
	{
		OnRandName(inPacket,pDataClient); //随机一下名字
	}
	else if(dcChangeActorName == nCmd)	//玩家改名字
	{
		OnChaneActorNameToDb(inPacket,pDataClient);
	}
	else if(dcCheckSecondPsw == nCmd)
	{
		OnCheckSecondPsw(inPacket,pDataClient);
	}
	else if( dcCreateSecondPsw == nCmd )
	{
		OnCreateSecondPsw(inPacket,pDataClient);
	}
	else if(dcGetCSActorId == nCmd)
	{
		QueryCrossServerActor(inPacket,pDataClient);
	}
	else if(dcCreateCSActorId == nCmd)
	{
		CreateCrossServerActor(inPacket,pDataClient);
	}
	

}
void CGateDBReqestHandler::OnCreateActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient )
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR resultCode = NOERR;
	
	BYTE bGateIndex =0;
	Uint64 lKey=0;
	int   nRawServerId =0;
	int   nServerId =0;
	unsigned int nAccountId=0;
	int   nLastRandNameIndex =-1;

	inPacket >> nRawServerId >> nServerId >> bGateIndex >> lKey >>nAccountId; //读入数据
	unsigned long long sip =0; //地址
	inPacket >> sip;
	
	char	actorName[jxSrvDef::ACTOR_NAME_BUFF_LENGTH];		//角色名称
	char	accountName[jxSrvDef::ACCOUNT_NAME_BUFF_LENGTH];		//账户名称

	char    strToken[jxSrvDef::MAX_TOKEN_LENGTH];

	INT_PTR nActorCount = ArrayCount(actorName);
	inPacket.readString(actorName,nActorCount);
	
	
	BYTE sex =0,job=0,icon=0,zy=0;
	BYTE nEnterId = 0;
	int nAdID = 0;
	
	//要改成小写
	//m_pDBServer->GetDBEngine()->LowerCaseNameStr(actorName,nActorCount);

	INT_PTR nLen = CDBServer::GetStrLenUtf8(actorName);

	//读取账户名字
	INT_PTR nAccountCount = ArrayCount(accountName);
	inPacket.readString(accountName,nAccountCount);
	
	if ( nLen <= 0 || nLen > MAX_NAME_SIZE || !m_pDBServer->GetDBEngine()->CheckNameStr(actorName))
	{
		resultCode = jxInterSrvComm::NameServerProto::neInvalidName;
	}
	else
	{
		inPacket >> sex;
		inPacket >> job;
		inPacket >> zy;
		inPacket >> icon;
		inPacket >>nLastRandNameIndex;

		inPacket.readString(strToken,ArrayCount(strToken));
		strToken[sizeof(strToken)-1] = 0;

		inPacket >> nAdID;
		inPacket >> nEnterId;
		BYTE bCanCreate = 1;
		inPacket >> bCanCreate;
		

		// 检测阵营合法性
		bool zyVali = true;
		if (zy < 0 || zy > 3)
		{
			zyVali = false;
		}

		if ( !zyVali )//选择的阵营必须是之前服务器下发给他的
		{
			resultCode = ERR_ZY;
		}
		else
		{
			/*
			if (zy == 0)//如果是选了随机，则给个最少人选的阵营
			{
				resultCode = QueryZYReq(nRawServerId);
			}
			*/

				
			if (job <= enVocNone || job >= enMaxVocCount )
			{
				resultCode = ERR_JOB;
			}


		if(!bCanCreate)
		{
			//第一个创建的角色，阵营是不能选择的，只能分配个最少人选的
				if (QueryActorCount(nAccountId,nRawServerId) <= 0)
				{
					resultCode = jxInterSrvComm::NameServerProto::neDBlimitCreate;
				}
		}
		}
	}

	if (resultCode == NOERR)
	{
		
		if (m_pDBServer->GetJobZyMgr()->IsActorNameExist(actorName,-1) )
		{
			resultCode = jxInterSrvComm::NameServerProto::neNameinuse;
		}
	}

	if (resultCode != NOERR)
	{
		
		
		OutputMsg(rmError, _T("Create Actor Failed, code=%d,name=%s"), (int)resultCode,actorName);
		//发给玩家出错
		CDataPacket &retPack= pDataClient->AllocDataPacket(dcCreateActor);
		retPack <<nRawServerId  << nServerId;

		retPack << bGateIndex <<lKey << (unsigned int)0 <<(BYTE)resultCode ;
		pDataClient->FlushDataPacket(retPack);
		return;
		
		//GetActorList(nAccountId,nServerId,retPack);


		//OutputMsg( rmTip, _T("create actor error!actorid:%d,actorname:%s,error:%d"),Resp.ActorId,ReqData->ActorName,Resp.Ret);
		//m_Allocator.FreeBuffer(ReqData);
		//return &Packet;
	}
	
	
	//OutputMsg(rmTip,_T("post alloc name msg to nameserver!name=%s"),ReqData->ActorName);

	CREATEACTORDATA *pData= new(CREATEACTORDATA);
	if(pData ==NULL)
	{
		OutputMsg( rmError, _T("alloc CREATEACTORDATA memery fail "));
		return ;
	}
	strncpy(pData->actorName,actorName,nActorCount);
	strncpy(pData->accountName,accountName,nAccountCount);

	if(!strToken)
	{
		strncpy(pData->strToken,"",ArrayCount(pData->strToken));
	}
	else
	{
		strncpy(pData->strToken,strToken,ArrayCount(strToken));
	}

	pData->icon = icon;
	pData->job =job;
	pData->lKey =lKey;
	pData->nGateIndex = bGateIndex;
	pData->nServerIndex = nServerId;
	pData->sex = sex;
	pData->zy= zy;
	pData->lIp = sip; //通过逻辑服把ip发过来
	pData->nAccountId = nAccountId;
	pData->nLastRandNameIndex = nLastRandNameIndex;
	pData->nAdId = nAdID;
	pData->nRawServerindex = nRawServerId;

	// 向名称服务器查询该名字是否可用
	m_pDBServer->GetDBEngine()->getNameSyncClient()->PostAllocateCharId(nAccountId,nRawServerId,actorName,pData);
	
	// 保存创建角色所需要的数据
	//((CDBGateUser*)pGateUser)->pCreateCharStruct = ReqData;

	//保存到日志服务器
	/*
	m_pDBServer->getLogClient()->SendLoginLog(ltCreateActor,pGateUser->nGlobalSessionId,pGateUser->sAccount,
		pGateUser->sIPAddr,ReqData->ActorName);//最后的描述字段保存用户申请的角色名称
	*/
	
}

//处理名字服务器的返回
VOID CGateDBReqestHandler::OnCreateActorResult( INT_PTR nError,unsigned int nActorId,PCREATEACTORDATA pData,ILogicDBRequestHost * pDataClient)
{
	if (!pData || ArrayCount(pData->actorName) > jxSrvDef::ACTOR_NAME_BUFF_LENGTH )
	{
		OutputMsg(rmError,_T("OnCreateActorResult NULL"));
		delete pData;
		return;
	}
	
	char szSql[500];

	//名称服务器返回正确结果，才执行插入数据库操作
	if (nError == jxInterSrvComm::NameServerProto::neSuccess)
	{

		unsigned long long sip = pData->lIp;
		sprintf_s(szSql,sizeof(szSql),szSQLSP_ClientCreateNewCharactor,pData->nAccountId,pData->accountName,
			sip,
			nActorId,pData->actorName,pData->icon,pData->sex,pData->job,pData->zy,pData->nServerIndex,pData->strToken,pData->nAdId);	
		nError = m_pSQLConnection->RealExec(szSql,strlen(szSql));
		if (nError == 0)
		{
			m_pSQLConnection->ResetQuery();
			m_pDBServer->GetJobZyMgr()->OnNewPlayerCreated(pData->nServerIndex, (tagZhenying)pData->zy,(tagActorVocation)pData->job,pData->actorName);

		}
		else
		{
			OutputMsg(rmWaning,"%s call fail",szSql);
			nError = ERR_SQL;
		}
	}	

	if (nError != 0)
	{
		OutputMsg(rmError, _T("Create Actor Failed, code=%d,actorid=%u,name =%s"), (int)nError,nActorId,pData->actorName);
	}


	CDataPacket& retPack = pDataClient->AllocDataPacket(dcCreateActor);
	retPack << pData->nRawServerindex << pData->nServerIndex ;

	retPack << (BYTE)pData->nGateIndex;
	retPack << pData->lKey;
	retPack << (unsigned int)nActorId;
	retPack << (BYTE)nError;//角色id
	pDataClient->FlushDataPacket(retPack);

	//OutputMsg( rmTip, _T("send create actor response!actorid:%d,actorname:%s,errcode:%d"),Resp.ActorId,data->ActorName,Resp.Ret);

	delete pData; //把这个内存删除掉

}





//处理名字服务器的返回
VOID CGateDBReqestHandler::OnCreateCrossServerActorResult( INT_PTR nError,unsigned int nActorId,PCREATEACTORDATA pData,ILogicDBRequestHost * pDataClient)
{
	if (!pData || ArrayCount(pData->actorName) > jxSrvDef::ACTOR_NAME_BUFF_LENGTH )
	{
		OutputMsg(rmError,_T("OnCreateActorResult NULL"));
		delete pData;
		return;
	}
	
	char szSql[500] = {0};

	//名称服务器返回正确结果，才执行插入数据库操作
	if (nError == jxInterSrvComm::NameServerProto::neSuccess)
	{

		unsigned long long sip = pData->lIp;
		sprintf_s(szSql,sizeof(szSql),szSQLSP_CrateCrossServerCharactor,pData->nAccountId,pData->accountName,
			sip,
			nActorId,pData->actorName,pData->icon,pData->sex,pData->job,pData->zy,pData->nServerIndex,pData->strToken,pData->nAdId);	
		nError = m_pSQLConnection->RealExec(szSql,strlen(szSql));
		OutputMsg(rmTip,_T("OnCreateCrossServerActorResult:%s"),szSql);
		if (nError == 0)
		{
			m_pSQLConnection->ResetQuery();
			m_pDBServer->GetJobZyMgr()->OnNewPlayerCreated(pData->nServerIndex, (tagZhenying)pData->zy,(tagActorVocation)pData->job,pData->actorName);

		}
		else
		{
			OutputMsg(rmWaning,"%s call fail",szSql);
			nError = ERR_SQL;
		}
	}	

	if (nError != 0)
	{
		OutputMsg(rmError, _T("Create Actor Failed, code=%d,actorid=%u,name =%s"), (int)nError,nActorId,pData->actorName);
	}

	OutputMsg(rmTip,_T("[CrossLogin 6] OnCreateCrossServerActorResult :%d, nActorId:%d"),(int)pData->lKey,nActorId);
	CDataPacket& retPack = pDataClient->AllocDataPacket(dcCreateCSActorId);
	retPack << pData->nServerIndex << pData->nRawServerindex ;
	retPack << (int)pData->lKey;
	retPack << (BYTE)nError;//角色id
	retPack << (unsigned int)nActorId;
	pDataClient->FlushDataPacket(retPack);

	//OutputMsg( rmTip, _T("send create actor response!actorid:%d,actorname:%s,errcode:%d"),Resp.ActorId,data->ActorName,Resp.Ret);

	delete pData; //把这个内存删除掉

}


void CGateDBReqestHandler::OnRandName( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient )
{
	DECLARE_FUN_TIME_PROF()
	
	INT_PTR index = 1;

	BYTE bGateIndex =0,bSex =0;
	Uint64 lKey=0;
	int   nRawServerId =0,nServerindex=0;
	//读取性别
	inPacket >> nRawServerId >>nServerindex >> bGateIndex >> lKey   >>bSex;
	INT_PTR resultCode = NOERR;
	char sActorName[32];
	
	CDBServer* pDbServer = m_pDBServer->GetDBEngine();
	
	//DECLARE_TIME_PROF("RandNameReq_GetRandomName");
	
	if (!m_pSQLConnection->Connected())
	{
		resultCode = ERR_SQL_NOT_CONNECT;
	}
	else
	{
		
			while(true)
			{
				if(!pDbServer->GetRandomGroupName(sActorName, ArrayCount(sActorName)))
				{
					resultCode = ERR_NORANDOMNAME;
					break;
				}
				//index = pDbServer->GetRandomName(sActorName,bSex);
				//if (index < 0)
				//{ 
				//	resultCode = ERR_NORANDOMNAME;
				//	break;
				//}
				//// 如果此名字属于屏蔽字，过滤掉
				//if (pDbServer->hasFilterWordsInclude(sActorName))
				//{
				//	OutputMsg(rmTip,"name =%s has filtername",sActorName);
				//	pDbServer->AllocRandomName(index, bSex);
				//	continue;
				//}

				//查找本地数据库有没有这个名字
				//查找本地库是否存在这个名字
				if ( !m_pDBServer->GetJobZyMgr()->IsActorNameExist((char*)sActorName,-1) )
				{
					//OutputMsg(rmNormal,_T("alloc name[%s],serverid=%d"),sActorName,(int)nRawServerId);
					break; //找到了
				}
				else //查询失败
				{
					continue;
				}
			}
	}
	
	//如果是成功的，就返回这个
	CDataPacket &retPack= pDataClient->AllocDataPacket(dcGetRandName);	
	retPack << nRawServerId << nServerindex << bGateIndex <<lKey << (int)index <<(BYTE)resultCode <<bSex;
	if(resultCode == NOERR)
	{
		retPack.writeString(sActorName); 
	}
	pDataClient->FlushDataPacket(retPack);

}
void CGateDBReqestHandler::GetActorList(const unsigned int nAccountId, const int nServerId,CDataPacket &packet )
{
	
	DECLARE_FUN_TIME_PROF()


	char szSql[256];
	OutputMsg(rmNormal, _T("Start GetActorList nAccountId[%u] nServerId[%d]"), nAccountId,nServerId);
	sprintf(szSql, szSQLSP_LoadActorListByAccountId, nAccountId, nServerId);
	int nError = m_pSQLConnection->Query(szSql);

	if (nError != 0)
	{
		packet << (char)ERR_SQL; //-1是错误码
		return;
	}

	INT_PTR nActorCount	= m_pSQLConnection->GetRowCount();
	MYSQL_ROW pRow		= m_pSQLConnection->CurrentRow();
	packet << (char)nActorCount;

	INT_PTR nLastActor = 0;
	INT_PTR nIndex = 0;
	while ( pRow )
	{
		Actors actor;
		sscanf(pRow[0], "%d", &(actor.ActorId));
		_asncpytA(actor.ActorName, pRow[1]);		
		actor.Icon = _StrToInt(pRow[2]);
		sscanf(pRow[3], "%d", &(actor.Sex));
		sscanf(pRow[4], "%d", &(actor.Level));
		sscanf(pRow[5], "%d", &(actor.Circle));
		sscanf(pRow[6], "%d", &(actor.Job));
		int status = 0;
		sscanf(pRow[8], "%d", &status);
		actor.Zy = _StrToInt(pRow[9]);

		if( pRow[10] )			//要判断下是否为空
		{
			_asncpytA(actor.GuildName, pRow[10]);
		}
		else
		{
			_asncpytA(actor.GuildName, "");
		}

		// Write data to packet
		packet << (UINT)actor.ActorId;
		packet.writeString(actor.ActorName);
		packet << (char)actor.Icon;
		packet << (char)actor.Sex;
		packet << (short)actor.Level;
		packet << (short)actor.Circle;
		packet << (char)actor.Job;
		packet << (char)actor.Zy;
		packet << (char)status;
		packet.writeString(actor.GuildName);

		// 如果第三位是1，表示这个是上次登陆的角色
		status = (status >> 2) & 1 ;
		if (1 == status)
		{
			nLastActor = nIndex;
		}

		pRow = m_pSQLConnection->NextRow();
		nIndex++;
	}

	m_pSQLConnection->ResetQuery();
	

	packet << (BYTE)nLastActor;
	packet << (BYTE)QueryLessJobReq(nServerId);
	BYTE nCamp = 1;
	if (nActorCount <= 0)
	{
		packet << (WORD)7;		
		packet <<(BYTE)1;
	}
	else
	{
		// 获取可选的阵营列表		
		packet <<(WORD)1;
		packet <<(BYTE)1; // 这里的1是无效的，防止读错
	}

	BYTE btCode = 0;
	sprintf(szSql, szSQLSP_LoadAccountPsw, nAccountId, nServerId);
	nError = m_pSQLConnection->Query(szSql);
	if ( nError != 0 )
	{
		packet << (BYTE)btCode;
		return;
	}
	pRow = m_pSQLConnection->CurrentRow();
	if ( pRow && pRow[0] && strlen(pRow[0]) > 30)
	{
		btCode = 3;
	}
	else
	{
		btCode = 0;
	}
	packet << (BYTE)btCode;
	m_pSQLConnection->ResetQuery();
	OutputMsg(rmNormal, _T("End GetActorList nAccountId[%u], nIndex[%d], nLastActor[%d]"), nAccountId,nIndex,nLastActor);
}

int CGateDBReqestHandler::StartEnterGame(const int nServerId, 
										 const int nRawServerId, 
										 const unsigned int nAccountId, 
										 const unsigned int nActorId, 
										 const char *szAccount, 
										 LONGLONG nIP)
{
	// 验证登录逻辑服务器是否合法。防止玩家随意登录服务器
	int nLastLoginServerId = 0;
	int nError = m_pSQLConnection->Query(szSQLSP_QueryActorLastLoginServerId, 								   
								   nActorId,
								   nAccountId,
								   nRawServerId);
	if (nError)
	{
		OutputMsg(rmError, _T("查询角色[actorid=%u accountid=%u rawserverid=%d]上次登录服务器ID失败，错误码：%d"), 
				  nActorId, nAccountId, nRawServerId, nError);
		return nError;
	}
	
	if (m_pSQLConnection->GetRowCount() >= 1)
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		sscanf(pRow[0], "%d", &nLastLoginServerId);		
	}
	else
	{
		nError = ERR_SQL;
	}
	m_pSQLConnection->ResetQuery();
	if (ERR_SQL == nError)
	{
		return nError;
	}

	// 角色只能登录原始服务器以及上次登录的服务器
	if (nLastLoginServerId != 0)
	{
		if (nServerId != nLastLoginServerId && nServerId != nRawServerId)
		{
			OutputMsg(rmError, 
					 _T("%s 角色[actorid=%u, accountid=%u]选择服务器错误[rserverid:%d, curloginserverid:%d, lastloginserverid:%d]，可能是客户端攻击行为!"),
					__FUNCTION__, 
					nActorId, 
					nAccountId,
					nRawServerId,
					nLastLoginServerId,
					nServerId);
			return ERR_SELSERVER;
		}
	}
		
	nError = m_pSQLConnection->Query(szSQLSP_ClientStartPlay, 
								   nServerId, 
								   nActorId, 
								   szAccount,
								   nAccountId,
								   nIP);
	if (!nError)
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		if (pRow[0] && _StrToInt(pRow[0]))
		{
			nError = NOERR;
		}
		else
		{
			nError = ERR_NOUSER;
		}

		m_pSQLConnection->ResetQuery();
	}

	return nError;
}

int CGateDBReqestHandler::QueryLessJobReq(const int nServerId)
{
	int  job = 1;
	if(m_pDBServer)
	{
		job = m_pDBServer->GetJobZyMgr()->GetPriorityJob(nServerId);
	}
	return job;
}

int CGateDBReqestHandler::QueryZyList(const int nServerId)
{
	int result = 7;

	if(m_pDBServer)
	{
		result = m_pDBServer->GetJobZyMgr()->GetOptionalZy(nServerId);
	}
	return result;
}

tagZhenying CGateDBReqestHandler::QueryZYReq(const int nServerId)
{
	tagZhenying camp = zyWuJi;
	if(m_pDBServer)
	{
		camp = m_pDBServer->GetJobZyMgr()->GetPriorityZY(nServerId);
	}

	return camp;
}

void CGateDBReqestHandler::OnChaneActorNameToDb(CDataPacketReader & inPacket,ILogicDBRequestHost * pDataClient)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nServerId = 0;
	unsigned int nActorId = 0;
	inPacket >> nRawServerId >> nServerId >> nActorId;

	CUserItem::ItemSeries series ;
	inPacket >> series.llId;
	char actorName[32];
	inPacket.readString(actorName,ArrayCount(actorName));

	char sName[32];
	inPacket.readString(sName,ArrayCount(sName));

	INT_PTR resultCode = NOERR;

	//要改成小写
	//m_pDBServer->GetDBEngine()->LowerCaseNameStr(actorName,ArrayCount(actorName));

	INT_PTR nLen = CDBServer::GetStrLenUtf8(actorName);

	if ( nLen <= 0 || nLen > MAX_NAME_SIZE || !m_pDBServer->GetDBEngine()->CheckNameStr(actorName))
	{
		resultCode = jxInterSrvComm::NameServerProto::neInvalidName;
	}

	if (resultCode == NOERR)
	{
	
		if (m_pDBServer->GetJobZyMgr()->IsActorNameExist(actorName,-1))
		{
			resultCode = jxInterSrvComm::NameServerProto::neNameinuse;
		}
	}

	if (resultCode == NOERR)
	{
		int nError = m_pSQLConnection->Exec(szSQLSP_ChangeActorName,actorName,nActorId,nServerId);
		if(!nError)
		{
			m_pSQLConnection->ResetQuery();

			m_pDBServer->GetJobZyMgr()->ChangeNameToList(actorName,sName,nServerId);
		}
	}

	CDataPacket& retPack = pDataClient->AllocDataPacket(dcChangeActorName);
	retPack << (int)nRawServerId <<(int)nServerId << (unsigned int)nActorId;
	retPack << (BYTE)resultCode << (LONG64)series.llId;
	retPack.writeString(actorName?actorName:"");
	pDataClient->FlushDataPacket(retPack);
}

void CGateDBReqestHandler::GetActorSecondPsw(const unsigned int nAccountId, const int nServerId, CDataPacket &packet)
{
	char szSql[256];
	sprintf(szSql, szSQLSP_LoadAccountPsw, nAccountId, nServerId);
	int nError = m_pSQLConnection->Query(szSql);
	BYTE btFlag = 0;
	if ( !nError )
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		if ( pRow && pRow[0]  && strlen(pRow[0]) > 30 )
		{
			btFlag = 3;
		}
		packet << (BYTE)btFlag;
		m_pSQLConnection->ResetQuery();
	}
	else
	{
		packet << (BYTE)btFlag;
	}
}

void CGateDBReqestHandler::OnCheckSecondPsw(CDataPacketReader & inPacket,ILogicDBRequestHost * pDataClient)
{
	unsigned int nAccountId =0;
	int nRawServerId=0;
	int nServerId =0;
	UINT64 lKey =0;
	BYTE bGateIndex =0;
	ACTORNAME sGatePsw = {0};
	unsigned int nActorId = 0;

	inPacket >>nRawServerId >>nServerId >> bGateIndex >> lKey >> nAccountId;
	inPacket.readString(sGatePsw,ArrayCount(sGatePsw));
	char szSql[256];
	sprintf(szSql, szSQLSP_LoadAccountPsw, nAccountId, nServerId);
	int nError = m_pSQLConnection->Query(szSql);
	if ( !nError )
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		ACTORNAME sPsw = {0};
		if ( pRow && pRow[0])
		{
			sscanf(pRow[0], "%s", &sPsw);
		}
		if ( !sPsw || strcmp(sPsw, sGatePsw) != 0)
		{
			nError = 2;
		}
		m_pSQLConnection->ResetQuery();
	}
	else
	{
		nError = ERR_SQL;
	}
	CDataPacket &retPack= pDataClient->AllocDataPacket(dcCheckSecondPsw);
	retPack << nRawServerId <<nServerId << bGateIndex <<lKey;
	retPack << (BYTE)nError;

	pDataClient->FlushDataPacket(retPack);
}


void CGateDBReqestHandler::OnCreateSecondPsw(CDataPacketReader & inPacket,ILogicDBRequestHost * pDataClient)
{
	unsigned int nAccountId =0;
	int nRawServerId=0;
	int nServerId =0;
	UINT64 lKey =0;
	BYTE bGateIndex =0;
	ACTORNAME sGatePsw = {0};
	unsigned int nActorId = 0;

	inPacket >>nRawServerId >>nServerId >> bGateIndex >> lKey >> nAccountId ;
	inPacket.readString(sGatePsw,ArrayCount(sGatePsw));


	char sBuff[1024] = {0};
	sprintf(sBuff, szSQLSP_CreateAccountPsw,nAccountId,nServerId,sGatePsw);
	int nErrorCode = m_pSQLConnection->Exec(sBuff);
	if (!nErrorCode)
	{
		m_pSQLConnection->ResetQuery();
	}
	else
	{
		nErrorCode = ERR_SQL; 
	}

	CDataPacket &retPack= pDataClient->AllocDataPacket(dcCreateSecondPsw);
	retPack << nRawServerId <<nServerId << bGateIndex <<lKey << (BYTE)nErrorCode;
	pDataClient->FlushDataPacket(retPack);
}

INT_PTR CGateDBReqestHandler::QueryActorCount( INT_PTR nAccountId, INT_PTR nServerIndex )
{
	DECLARE_FUN_TIME_PROF()
	BYTE result = 0;
	if (!m_pSQLConnection->Connected())
	{
		result = 0;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_QueryActorCount,nAccountId,nServerIndex);
		if (nError != 0)
		{
			result = 0;
		}
		else{
			MYSQL_ROW pRow;
			pRow = m_pSQLConnection->CurrentRow();
			if (pRow && pRow[0])
			{
				result = StrToInt(pRow[0]);
			}
			else
			{
				result = 0;//数据库无数据
			}
			m_pSQLConnection->ResetQuery();
		}	
	}
	return result;
}



INT_PTR CGateDBReqestHandler::QueryCrossServerActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient)
{
	DECLARE_FUN_TIME_PROF()
	int nActorId = 0;
	int nServerId = 0;
	int nCSSrvid = 0;
	int result = 0;
	int nAccountId = 0;
	int nLastActorId = 0;
	inPacket >> nServerId >>nCSSrvid >> nActorId >> nLastActorId;
	OutputMsg(rmTip,_T("[CrossLogin 3] DB收到查询跨服id :%d,nCSSrvid:%d,nLastActorId:%d"),nActorId,nCSSrvid, nLastActorId);
	// int64_t token = nCSSrvid << 32 + nActorId;
	BYTE nError = NOERR;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nError = ERR_SQL_NOT_CONNECT;
	}
	else
	{
		char szSql[100];
		nError = m_pSQLConnection->Query(szSQLSP_QueryeCrossServerCharactor,nLastActorId);
		if (nError != 0)
		{
			nError =ERR_SQL;//-1是错误码
		}
		else
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if (pRow && pRow[0] && atoi(pRow[0]) >=1)
			{
				sscanf(pRow[0], "%d", &(result));
			}
			m_pSQLConnection->ResetQuery();
		}

		if(result > 0)
		{
			OutputMsg(rmTip,_T("[CrossLogin 4] 跨服id查询成功 :%d,nCSSrvid:%d,nLastActorId:%d"),nActorId,nCSSrvid, nLastActorId);
			CDataPacket &retPack= pDataClient->AllocDataPacket(dcGetCSActorId);
			retPack << nServerId << nCSSrvid << nActorId << (BYTE)nError << result  ;//合并起来
			pDataClient->FlushDataPacket(retPack);//下发数据
		}
		else 
		{
			char sactorname[32] = {0};
			inPacket >> nAccountId;
			inPacket.readString(sactorname, sizeof(sactorname));
			CREATEACTORDATA *pData= new(CREATEACTORDATA);
			if(pData ==NULL)
			{
				OutputMsg( rmError, _T("alloc CREATEACTORDATA memery fail "));
				return nError;
			}
			INT_PTR nActorCount = ArrayCount(sactorname);
			strncpy(pData->actorName,sactorname,nActorCount);
			pData->icon = 0;
			pData->job =0;
			pData->lKey = nActorId; //玩家本服真实id
			pData->nGateIndex = 0;
			pData->nServerIndex = nServerId;
			pData->sex = 0;
			pData->zy= 0;
			pData->lIp = 0; //通过逻辑服把ip发过来
			pData->nAccountId = nAccountId;
			pData->nLastRandNameIndex = 0;
			pData->nAdId = 0;
			pData->nRawServerindex = nCSSrvid;
			// OutputMsg(rmTip,_T("CrossLogin 5:actorName:%s,icon:%d,job:%d,lKey:%lld,nGateIndex:%d,sex:%d,zy:%d,lIp:%d, nAccountId:%d,nRawServerindex:%d,accountName:%s"),
			// pData->actorName,pData->icon, pData->job,pData->lKey,pData->nGateIndex,pData->sex,pData->zy,pData->lIp,pData->nAccountId,pData->nRawServerindex,pData->accountName);
			OutputMsg(rmTip,_T("[CrossLogin 5] 需要向NS注册 :%d,nCSSrvid:%d,nLastActorId:%d,nServerId"),nActorId,nCSSrvid, nLastActorId,nServerId);
			// 向名称服务器查询该名字是否可用
			m_pDBServer->GetDBEngine()->getNameSyncClient()->PostCrossServerAllocateCharId(nAccountId,nServerId,sactorname,pData);
		}	
		return result;
	}
}

INT_PTR CGateDBReqestHandler::CreateCrossServerActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient)
{
	int result = 0;

	return result;
	DECLARE_FUN_TIME_PROF()
	int nActorId = 0;
	int nAccountId = 0;
	int nServerId = 0;
	int nCSSrvid = 0;
	int64_t nToken = 0;
	
	char sactorname[32] = {0};
	inPacket >> nServerId >>nCSSrvid >> nActorId >> nAccountId >> nToken;
	inPacket.readString(sactorname, sizeof(sactorname));

	BYTE nError = NOERR;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		nError = ERR_SQL_NOT_CONNECT;
	}
	else
	{
		//查询数据库
		char szSql[100];
		sprintf(szSql,szSQLSP_CrateCrossServerCharactor,nAccountId,nActorId, sactorname,nCSSrvid,nToken);
		nError = m_pSQLConnection->RealExec(szSql,strlen(szSql));
		if (nError != 0)
		{
			nError =ERR_SQL;//-1是错误码
		}
		else
		{
			m_pSQLConnection->ResetQuery();
		}	
	}	
}