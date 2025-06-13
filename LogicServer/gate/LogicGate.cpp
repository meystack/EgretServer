#include "StdAfx.h"
#include "LogicGate.h"

int CLogicGate::s_nMaxPacketSizes = 0;

CLogicGate::CLogicGate()
	:Inherited()
{
	m_pLogicServer = NULL;
	m_vFreeList.setLock(&m_vFreeListLock);
	AddAllGateUsersToFreeList();
	m_pSendToGatePacket = NULL;
	m_ullSendPacketSize = 0;
	m_bUseBigPacket = true;
}

CLogicGate::~CLogicGate()
{
	//释放datapacket
	m_vFreeList.flush();
	for(int i = 0; i < m_vFreeList.count(); i++)
	{
		CDataPacket* dp = m_vFreeList[i];
		if ( dp )
		{
			dp->setPosition(0);
			flushSendPacket(*dp);
		}
	}
	m_vFreeList.clear();

	for(int i = 0; i < m_vMsgList.count(); i++)
	{
		CDataPacket* dp = m_vMsgList[i];
		if ( dp )
		{
			dp->setPosition(0);
			flushSendPacket(*dp);
		}
	}
	m_vMsgList.clear();
	//最后的包发向网关：），实际为了释放这个包的内存
	SendToGate();
}


VOID CLogicGate::AddAllGateUsersToFreeList()
{
	int i;
	m_FreeUserList.clear();
	for ( i=ArrayCount(m_GateUsers)-1; i>-1; --i )
	{
		CLogicGateUser* pGateUser = &m_GateUsers[i];
		if ( pGateUser )
		{
			m_FreeUserList.add( pGateUser );
		}
	}
}

CCustomServerGateUser* CLogicGate::CreateGateUser()
{
	INT_PTR nCount = m_FreeUserList.count();
	if ( nCount > 0 )
	{
		nCount--;
		CCustomServerGateUser* pUser = m_FreeUserList[nCount];
		m_FreeUserList.trunc(nCount);
		return pUser;
	}
	return NULL;
}

VOID CLogicGate::DestroyGateUser(CCustomServerGateUser* pUser)
{
	if ( pUser )
	{
		m_FreeUserList.add((CLogicGateUser*)pUser);
	}
}

VOID CLogicGate::OnOpenUser(CCustomServerGateUser *pGateUser)
{
	if ( !pGateUser )
	{
		return;
	}
	CLogicGateUser* pLogicGateUser = (CLogicGateUser*)pGateUser;
	
	//NOTIFY the logicServer
	//GetGlobalLogicEngine()->PostInternalMessage(SSM_GATE_CREATE_ACTOR,)
	
	//create the user
	
	
	//使用当前的tickcount以及一个随机数，生成一个key，用作玩家的本次的校验的
	UINT64 key=0;
	UINT64 nTick= (UINT64)pGateUser->nSocket;   //当前的tickcount
	UINT64 nRandValue = wrandvalue();		 //一个随机数
	//如果是0.那么就设置一个数，避免出现key=0
	if(nRandValue ==0)
	{
		nRandValue =0xffee1133;
	}
	key = (nRandValue << 32 | nTick) ;
	pLogicGateUser->lLogicKey = key; //玩家的key
	
	pLogicGateUser->nGlobalSessionId = 0;
	pLogicGateUser->sAccount[0] = 0;
	pLogicGateUser->nCharId = 0;
	pLogicGateUser->sCharName[0] = 0;
	pLogicGateUser->nHandleIndex =0;
	pLogicGateUser->boHasSecPsw = false;
	pLogicGateUser->boCheckSecPswSucess = false;
	pLogicGateUser->boWhiteIpLogin = false;

	OutputMsg(rmTip,_T("[Login] (3) 添加一个玩家Gate： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),
		pLogicGateUser->lLogicKey,pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex,GetCurrentThreadId());
}

//通过key返回指针
CLogicGateUser * CLogicGate::GetGateUserByKey(UINT64 lKey)
{
	CLogicGateUser *pUser ;
	for(INT_PTR i=0; i< m_UserList.count(); i++)
	{
		pUser = (CLogicGateUser *)m_UserList[i];
		if(pUser && pUser->lLogicKey == lKey)
		{
			return pUser;
		}
	}
	return NULL;
}

//nServerSessionIndex, nHandleIndex,nAccountId
VOID CLogicGate::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	Inherited::DispatchInternalMessage(uMsg,uParam1,uParam2,uParam3,uParam4); //先执行父类的消息处理
	
	if(uMsg == SSM_RECV_LOGIN_DATA)
	{
		//会话服务器发来的
		if(uParam1 == jxInterSrvComm::SessionServerProto::sCheckPasswdResult) //数据服发来玩家的登陆失败的信息
		{
			CDataPacket* pDataPacket = (CDataPacket*)uParam2; //数据包的指针
			if(pDataPacket)
			{
				pDataPacket->setPosition(0);
				CCustomGlobalSession session;
				memset(&session,0,sizeof(session));

				*pDataPacket >> session;
				GetLogicServer()->GetSessionClient()->FreeBackUserDataPacket(pDataPacket);
				
				CLogicGateUser *pGateUser = GetGateUserByKey(session.lKey);
				if(pGateUser )
				{					
					pGateUser->boWhiteIpLogin = (session.nState ==0) ?true:false;
					//ResponseLoginErrorAndCloseUser(pGateUser,0,session.nSessionId);

					//这里要向数据服发消息，取玩家的角色列表
					pGateUser->nGlobalSessionId = session.nSessionId;
					strncpy(pGateUser->sAccount,session.sAccount,sizeof(pGateUser->sAccount));
					SendDbGetActorList(pGateUser); //向数据服发消息，获取角色的列表				
				}
				else
				{
					//OutputMsg(rmError,"收到会话发来的校验密码结果,但是找不到会话的key=%lld",session.lKey);	
					OutputMsg(rmTip,_T("[Login] (8) logickey找不到对应的玩家gate： logickey(%lld)，CurrentThreadId(%d)。"),
						session.lKey,GetCurrentThreadId());	
				}				
			}			
		}
		//数据服务器发来的
		else if( jxInterSrvComm::DbServerProto::dcGetActorList ==uParam1 
			|| jxInterSrvComm::DbServerProto::dcCreateActor ==uParam1 
			|| jxInterSrvComm::DbServerProto::dcDeleteActor ==uParam1 
			|| jxInterSrvComm::DbServerProto::dcGetRandName ==uParam1 
			|| jxInterSrvComm::DbServerProto::dcCheckSecondPsw == uParam1
			|| jxInterSrvComm::DbServerProto::dcCreateSecondPsw == uParam1
			)
		{
			CDataPacket* pDataPacket = (CDataPacket*)uParam2; //数据包的指针
			if(pDataPacket)
			{
				pDataPacket->setPosition(0);
				Uint64 lKey =0;
				*pDataPacket >>lKey; //读取key
				CLogicGateUser *pUser = GetGateUserByKey(lKey);
				if(pUser )
				{
					BYTE cmd =0;
					switch(uParam1)
					{
					case jxInterSrvComm::DbServerProto::dcGetActorList:
						cmd=sGetUserList;
						break;
					case jxInterSrvComm::DbServerProto::dcCreateActor:
						cmd=sCreateActor;
						break;
					case jxInterSrvComm::DbServerProto::dcDeleteActor:
						cmd=sDeleteActor;
						break;
					case jxInterSrvComm::DbServerProto::dcGetRandName:
						cmd=sRandNameActor;
						break;
					case jxInterSrvComm::DbServerProto::dcCheckSecondPsw:
						cmd=sCheckSecondPswResult;
						break;
					case jxInterSrvComm::DbServerProto::dcCreateSecondPsw:
						cmd=sCreateSecondPswRessult;
						break;
					}
					
					CDataPacket &dataPack = AllocGateSendPacket(pUser->nSocket,pUser->nGateSessionIndex,pUser->nServerSessionIndex);
					dataPack <<(BYTE)enLoginSystemId << (BYTE) cmd ;
					
					//名字的随机需要特殊处理，把玩家的账户弄过来
					if(cmd == sRandNameActor)
					{
						BYTE bErrorCode =0,bSex=0;
						*pDataPacket >> pUser->nLastRandNameIndex >>bErrorCode >> bSex ;
						dataPack << bErrorCode << bSex;
						if(bErrorCode ==0 &&  pUser->nLastRandNameIndex >=0 )
						{
							pDataPacket->readString(pUser->sLastRandName,sizeof(pUser->sLastRandName));
							dataPack.writeString(pUser->sLastRandName);
						}
					}
					else if( cmd == sGetUserList)
					{
						INT_PTR nLength = pDataPacket->getLength();
						INT_PTR nOffer = pDataPacket->getPosition();
						pDataPacket->setPosition(nLength - 1);
						BYTE btCode = 0;
						*pDataPacket >> btCode;
						if ( btCode == 3 )
						{
							pUser->boHasSecPsw = true;
						}
						else
						{
							pUser->boHasSecPsw = false;
						}
						pDataPacket->setPosition(nOffer);
						
						dataPack << (int)(GetLogicServer()->GetDaysSinceOpenServer());
						BYTE canCreate = 1;
						int nNosetopenday = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo().nNoSetOpenDay;
						if( nNosetopenday > 0 && (GetLogicServer()->GetDaysSinceOpenServer() > nNosetopenday))
						{
							canCreate = 0;
						}
						dataPack <<(BYTE)canCreate;
						dataPack.writeBuf(pDataPacket->getOffsetPtr(),pDataPacket->getLength() - pDataPacket->getPosition());

						OutputMsg(rmTip,_T("[Login] (9) 发送角色列表： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，SessId(%d), CurrentThreadId(%d)。"),
						pUser->lLogicKey,pUser->nSocket,pUser->nGateSessionIndex,pUser->nServerSessionIndex,pUser->nGlobalSessionId,GetCurrentThreadId());

						if ( !GetGlobalLogicEngine()->GetLoginSecondPswFlag() )
						{
							OutputMsg(rmWaning,_T("secondPsw account%u-thread%u "),pUser->nGlobalSessionId,GetCurrentThreadId());
						}
					}
					else if (cmd == sCheckSecondPswResult)
					{
						BYTE btError = 1;
						*pDataPacket >> btError;
						if ( btError == 0 || GetGlobalLogicEngine()->GetSecondPswFlag() || pUser->boWhiteIpLogin)
						{
							pUser->boCheckSecPswSucess = true;
							btError = 0;
						}
						dataPack << (BYTE)btError;
					}
					else
					{
						dataPack.writeBuf(pDataPacket->getOffsetPtr(),pDataPacket->getLength() - pDataPacket->getPosition());
					}
					
					FlushGateSendPacket(dataPack);
				}
				else
				{
					
					OutputMsg(rmError,"收到数据服发来的校验密码结果,但是找不到会话的key=%lld,cmd=%d",lKey,(int)uParam1 );	
				}
				GetLogicServer()->GetDbClient()->FreeBackUserDataPacket(pDataPacket);

			}
		}

		else if( jxInterSrvComm::SessionServerProto::sSimulateCheckPasswdResult ==uParam1 )
		{
			{
				Uint64 lKey =(Uint64)uParam2;
				CLogicGateUser *pUser = GetGateUserByKey(lKey);
				if(pUser )
				{
					CDataPacket &dataPack = AllocGateSendPacket(pUser->nSocket,pUser->nGateSessionIndex,pUser->nServerSessionIndex);
					dataPack <<(BYTE)enLoginSystemId << (BYTE) sCrossServerInit ;
					dataPack << (unsigned int)uParam4;
					FlushGateSendPacket(dataPack);
				}
				else
				{
					
					OutputMsg(rmError,"跨服模拟发来的校验密码结果,但是找不到会话的key=%lld,cmd=%d",lKey,(int)uParam1 );	
				}

			}
		}

	}
	else if(uMsg == SSM_RECV_LOGIN_CMD) //收到了数据
	{
		if( uParam1 == jxInterSrvComm::SessionServerProto::sCheckPasswdResult) //登陆的结果
		{
			Uint64 lKey =uParam2;
			BYTE   bErrorCode =(BYTE)uParam3;
			CLogicGateUser *pGateUser = GetGateUserByKey(lKey);
			if(pGateUser )
			{
				if(bErrorCode !=0)
				{
					ResponseLoginErrorAndCloseUser(pGateUser,bErrorCode);
					//OutputMsg(rmWaning,"用户session检测失败,code=%d，key=%lld",(int)bErrorCode,lKey);
					//CloseUser(pGateUser->nSocket,pGateUser->nServerSessionIndex); //数据服没有连上来
					OutputMsg(rmTip,_T("[Login] (7) 账户验证失败： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，SessId(%d), CurrentThreadId(%d)，Errer（%d）。"),
						pGateUser->lLogicKey,pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex,pGateUser->nGlobalSessionId,GetCurrentThreadId(),bErrorCode);
				}
				else
				{
					OutputMsg(rmWaning,"actor check passwd from session,errorcode=%d，key=%lld",(int)bErrorCode,lKey);
				}
			}
			else
			{
				//OutputMsg(rmError,"收到会话发来的校验密码结果,code=%d，但是找不到会话的key=%lld",(int)bErrorCode,lKey);
				OutputMsg(rmTip,_T("[Login] (7) 账户验证失败，且logickey找不到对应的玩家gate： logickey(%lld)，CurrentThreadId(%d)，Errer（%d）。"),
					lKey,GetCurrentThreadId(),bErrorCode);
			}
		}
	}
	else if(uMsg == SSM_GATE_CREATE_ACTOR)
	{
		
		//nServerSessionIndex
		//nHandleIndex
		//nAccountId
		bool isValid = true;
		if(uParam1 <0 || uParam1 >= (UINT_PTR)m_UserList.count() )
		{
			OutputMsg(rmWaning,_T("SSM_GATE_CREATE_ACTOR nServerSessionIndex=%u,to large"),uParam1);
			isValid =false;
			
		}

		if(isValid)
		{
			CLogicGateUser * pUser =(CLogicGateUser *) m_UserList[uParam1];
			if(pUser ==NULL)
			{
				OutputMsg(rmWaning,_T("CREATE ACTOR FAIL FOR THE nGateServerIndex=%d, not existing"),uParam1);
				isValid =false;
			}
			else
			{
				if(pUser->nGlobalSessionId != uParam3)
				{
					OutputMsg(rmWaning,_T("CREATE ACTOR FAIL,accouid =%d ,local accountid=%d"),pUser->nGlobalSessionId,uParam3);
					isValid =false;
				}
			
				if(isValid && pUser->nHandleIndex !=0) 
				{
					OutputMsg(rmWaning,_T("CREATE ACTOR FAIL,nHandleIndex!=0 ,nServerSessionIndex=%d"),uParam1);
					isValid =false;
					//return; //已经设置过了handle了
				}

			
				//OutputMsg(rmTip,_T("CREATE ACTOR nHandleIndex=%d"),uParam3);
				if(uParam2 ==0) //create entity fail
				{
					OutputMsg(rmWaning,_T("CREATE ACTOR FAIL FOR THE nServerSessionIndex=%d"),uParam2);
					isValid =false;
					CloseUser(pUser->nSocket,pUser->nServerSessionIndex); //CLOSE the session
				}
				else
				{
					pUser->nHandleIndex = (unsigned int)uParam2;
				}
			}
		}

		if(isValid ==false)
		{
			GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(SSM_GATE_USER_LOGOUT,uParam3);
		}
	}
	else if(uMsg == SSM_KICK_USER)
	{
		CLogicGateUser * pUser=GetGateUserByKey(uParam2);
		if(pUser ==NULL) return;
		PostCloseUser(pUser->nSocket);
		
	}
	else if(SSM_CLOSE_SERVER == uMsg) //关闭服务器
	{
		PostCloseGateServer();
	}
}


VOID  CLogicGate::KickUserByGlobalSessionId(const int nGlobalSessionId)
{
	INT_PTR nIndex;
	CCustomServerGateUser *pUserList = UserExistsOfGlobalSessionId(nGlobalSessionId, &nIndex);

	if ( pUserList )
	{
		CLogicGateUser* pLogicGateUser = (CLogicGateUser*)pUserList;
		GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(SSM_SESSION_KICK_USER,pLogicGateUser->nGlobalSessionId);
	}
}

VOID CLogicGate::OnCloseUser(CCustomServerGateUser *pGateUser)
{
	if ( !pGateUser )
	{
		return;
	}
	CLogicGateUser* pLogicGateUser = (CLogicGateUser*)pGateUser;
	int nSessionId = pLogicGateUser->nGlobalSessionId;
	OutputMsg(rmTip,_T("CLogicGate::OnCloseUser. socket=%d, sessionId=%d, serverId=%d, accountId=%u, thread=%u"), (int)pGateUser->nSocket, 
		(int)pGateUser->nGateSessionIndex, (int)pGateUser->nServerSessionIndex, nSessionId,GetCurrentThreadId());
	if ( nSessionId != 0  && m_pLogicServer )
	{
		GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(SSM_GATE_USER_LOGOUT,nSessionId,pLogicGateUser->lLogicKey);

	}

	pLogicGateUser->nGlobalSessionId = 0;
	pLogicGateUser->sAccount[0] = 0;
	pLogicGateUser->nCharId = 0;
	pLogicGateUser->sCharName[0] = 0;
	pLogicGateUser->nHandleIndex =0;
	pLogicGateUser->lLogicKey =0;
	pLogicGateUser->boCheckSecPswSucess = false;
	pLogicGateUser->boHasSecPsw = false;
	pLogicGateUser->boWhiteIpLogin = false;
	//这个玩家关闭连接了，需要通知会话

}

VOID CLogicGate::OnGateClosed()
{
}

//收到了登陆包，先判断一下是否已经登陆了，如果能够登陆就通知逻辑服务器
VOID CLogicGate::OnUserLogin(CLogicGateUser *pGateUser,unsigned nAccountID, unsigned nActorID,int nLoginIndex)
{
	//jxSrvDef::GLOBALSESSIONOPENDATA sessionData;
	if ( !pGateUser )
	{
		return;
	}
	pGateUser->nGlobalSessionId = nAccountID; //帐户id，也就是全局的sessionID
	pGateUser->nCharId = nActorID;
	//OutputMsg(rmNormal, _T("CLogicGate::OnUserLogin: socket=%d, sessionId=%d, serverId=%d, accountId=%u, actorid=%u"), (int)pGateUser->nSocket, 
	//	(int)pGateUser->nGateSessionIndex, (int)pGateUser->nServerSessionIndex, nAccountID, nActorID);

	
	OutputMsg(rmTip,_T("[Login] (10) 收到玩家登陆角色请求： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，SessId(%d), AccountId(%u)，ActorId(%d)，CurrentThreadId(%d)。"),
		pGateUser->lLogicKey,pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex,pGateUser->nGlobalSessionId, nAccountID, nActorID,GetCurrentThreadId());

	//获取是否有这个用户在
	bool isValid =true;
	INT_PTR  nErrorID =0; 
	int nServerIndex = 0; // TODO.这里客户端登录的时候传递ServerIndex数据

	//通知逻辑有人登陆了
	GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(SSM_GATE_CREATE_ACTOR,
						(UINT_PTR)pGateUser->nSocket,
						nActorID,
						m_nGateIndex,
						pGateUser->nGateSessionIndex,
						pGateUser->nServerSessionIndex,
						pGateUser->nGlobalSessionId,
						pGateUser->lLogicKey,
						nLoginIndex
						);
}

//告诉玩家的结果
void CLogicGate::ResponseLoginErrorAndCloseUser(CLogicGateUser *pUser,int nErrorCode)
{
	if ( !pUser )
	{
		return;
	}
	//向用户发消息，登陆失败
	CDataPacket &dataPack = AllocGateSendPacket(pUser->nSocket,pUser->nGateSessionIndex,pUser->nServerSessionIndex);
	dataPack <<(BYTE)enLoginSystemId << (BYTE) sLoginErrorCode << (BYTE)nErrorCode ;
	FlushGateSendPacket(dataPack);
	OutputMsg(rmWaning,"login close user,errocode=%d",nErrorCode);
	
	//这里先不踢下线，让客户端主动断开连接
	if(pUser->nSocket != INVALID_SOCKET)
	{
		//CloseUser(pUser->nSocket,pUser->nServerSessionIndex); //CLOSE the session
		//PostCloseUser();
		//PostCloseUser(pUser->nSocket,pUser->nServerSessionIndex);
	}
}

//转发到会话服务器的
void CLogicGate::Client2SesionCheckUserPasswd(CDataPacketReader &pack,CLogicGateUser* pLogicGateUser)
{
	//首先检测会话服和数据服是否是ok的，如果没有ok告诉玩家不能登陆
	if( m_pLogicServer->GetSessionClient()->connected()) 
	{
		if( m_pLogicServer->GetDbClient()->connected() )
		{

			CLogicSSClient* pSSClient = GetLogicServer()->GetSessionClient();
			if(pSSClient)
			{
				CDataPacket &data = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cLogicCheckPasswd); //发给会话校验密码

				TCHAR name[ACCOUNT_NAME_BUFF_LENGTH];
				TCHAR passwd[MAX_PASSWD_LENGTH];
				char identity[64]; //身份证号码
				name[0]=0;
				passwd[0]=0;
				identity[0] =0;
				
				int nLoginServerId =0; // 要登陆的服务器的编号
				int nRawServerId =0;   // 要登陆的服务器的编号

				pack.readString(name,sizeof(name));
				pack.readString(passwd,sizeof(passwd));
				pack >> nLoginServerId;
				pack >> nRawServerId;
				

				pLogicGateUser->nRawServerIndex = nRawServerId;
				data << (Uint64)pLogicGateUser->lLogicKey; //发key过去
				data.writeString(name);
				data.writeString(passwd);
				data << (int)nLoginServerId;  //要登陆的服务器id
				data << (int)nRawServerId;    //要连接的服务器的id
				data << (int)m_nGateIndex;    //将网关的编号发过去

				unsigned long long sip = (inet_addr(pLogicGateUser->sIPAddr));
				data << sip;  //将登陆的ip发过去

				char md5[64];
				char currenttime[32];
				md5[0]=0;
				currenttime[0] =0;
				data << BYTE(1);  //增加1个字节过去，表示需要校验md5值

				if(pack.getAvaliableLength() >0)
				{
					pack.readString(md5,sizeof(md5));
					data.writeString(md5);
				}
				if(pack.getAvaliableLength() >0)
				{
					pack.readString(currenttime,sizeof(currenttime));
					data.writeString(currenttime);
				}

				//读取身份证号码
				if(pack.getAvaliableLength() >0)
				{
					pack.readString(identity,sizeof(identity));
					data.writeString(identity);
				}

				pSSClient->flushProtoPacket(data);
				
				OutputMsg(rmTip,_T("[Login] (6) 发送到SessionServer验证账号： AccountName(%s), logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),
					name,pLogicGateUser->lLogicKey,pLogicGateUser->nSocket,pLogicGateUser->nGateSessionIndex,pLogicGateUser->nServerSessionIndex,GetCurrentThreadId());
			}
		}
		else
		{
			ResponseLoginErrorAndCloseUser(pLogicGateUser,enDbServerClose); 
			OutputMsg(rmError,_T("[Login] (6) CDataClient获取失效(数据服没连接)： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),
				pLogicGateUser->lLogicKey,pLogicGateUser->nSocket,pLogicGateUser->nGateSessionIndex,pLogicGateUser->nServerSessionIndex,GetCurrentThreadId());//数据服没有连上来
			return ;
		}

	}
	else
	{
		ResponseLoginErrorAndCloseUser(pLogicGateUser,enSessionServerClose);
		OutputMsg(rmError,_T("[Login] (6) CLogicSSClient获取失效(会话服没连接)： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),GetCurrentThreadId());//玩家登陆的时候，会话或者数据服是关闭的
		return ;
	}
}

//向会话服务器创建账户
void  CLogicGate::Client2SesionCreateAccount(CDataPacketReader &pack,CLogicGateUser* pGateUser)
{
	//首先检测会话服和数据服是否是ok的，如果没有ok告诉玩家不能登陆
	if( m_pLogicServer->GetSessionClient()->connected()) 
	{
		if( m_pLogicServer->GetDbClient()->connected() )
		{

			CLogicSSClient* pSSClient = GetLogicServer()->GetSessionClient();
			if(pSSClient)
			{
				CDataPacket &data = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cLogicCreateAccount); //发给会话校验密码

				TCHAR name[ACCOUNT_NAME_BUFF_LENGTH];
				TCHAR passwd[MAX_PASSWD_LENGTH];
				TCHAR identity[128];
				
				name[0]=0;
				passwd[0]=0;
				identity[0] =0;
				pack.readString(name,sizeof(name));
				pack.readString(passwd,sizeof(passwd));
				pack.readString(identity,sizeof(identity));
				data.writeString(name);
				data.writeString(passwd);
				data.writeString(identity);

				unsigned long long sip = (inet_addr(pGateUser->sIPAddr));
				data << sip;  //将登陆的ip发过去

				pSSClient->flushProtoPacket(data);
			}

		}
		else
		{
			ResponseLoginErrorAndCloseUser(pGateUser,enDbServerClose);
			//CloseUser(pLogicGateUser->nSocket,pLogicGateUser->nServerSessionIndex); //数据服没有连上来
			return ;
		}

	}
	else
	{
		//OutputMsg(rmWaning,"actor login,but session or db is disconnect, login fail"); //玩家登陆的时候，会话或者数据服是关闭的
		ResponseLoginErrorAndCloseUser(pGateUser,enSessionServerClose);

		return ;
	}
}

//获取玩家角色列表
void CLogicGate::SendDbGetActorList(CLogicGateUser* pGateUser)
{
	if ( !pGateUser )
	{
		return;
	}
	CDataClient * pDbClient= m_pLogicServer->GetDbClient();
	if(pDbClient ==NULL || !pDbClient->connected())
	{
		ResponseLoginErrorAndCloseUser(pGateUser,enDbServerClose);
		OutputMsg(rmTip,_T("[Login] (8) 数据库连接失败，没法获取角色列表： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，SessId(%d), CurrentThreadId(%d)。"),
			pGateUser->lLogicKey,pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex,pGateUser->nGlobalSessionId,GetCurrentThreadId());	
	}
	else
	{
		//向数据服发消息，获取当前的玩家列表
		CDataPacket &packet = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcGetActorList);
		int nServerId = GetLogicServer()->GetServerIndex(); //当前的服务器的编号
		int nRawServerId= pGateUser->nRawServerIndex ;
		//协议约定key放在最前面
		packet << nRawServerId << nServerId  << (BYTE)m_nGateIndex <<pGateUser->lLogicKey << pGateUser->nGlobalSessionId ;
		pDbClient->flushProtoPacket(packet);
		
		OutputMsg(rmTip,_T("[Login] (8) 获取角色列表： logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，SessId(%d), CurrentThreadId(%d)。"),
			pGateUser->lLogicKey,pGateUser->nSocket,pGateUser->nGateSessionIndex,pGateUser->nServerSessionIndex,pGateUser->nGlobalSessionId,GetCurrentThreadId());	
	}
}
void CLogicGate::Client2DbCreateActor(CDataPacketReader &reader,CLogicGateUser* pGateUser)
{
	if ( !pGateUser )
	{
		return;
	}
	CDataClient * pDbClient= m_pLogicServer->GetDbClient();
	if(pDbClient ==NULL || !pDbClient->connected())
	{
		ResponseLoginErrorAndCloseUser(pGateUser,enDbServerClose);
		return;
	}

	char actorName[ACTOR_NAME_BUFF_LENGTH];
	actorName[0]=0;
	reader.readString(actorName,ArrayCount(actorName));

	char	Sex=0;				//性别
	char 	Job=0;			//职业的编码
	char	Icon=0;              //头像
	BYTE	Zy=0;
	reader >>Sex >> Job >> Icon >> Zy ;

	char strToken[MAX_TOKEN_LENGTH];
	strToken[0] = 0;
	reader.readString(strToken,ArrayCount(strToken));

	int nAdid = 0;
	reader >> nAdid;

	BYTE nEnterId = 0;
	//reader >> nEnterId;

	CDataPacket &packet = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcCreateActor);
	int nServerId = GetLogicServer()->GetServerIndex(); //当前的服务器的编号
	int nRawServerId= pGateUser->nRawServerIndex ;
	
	//IP地址
	unsigned long long sip = (inet_addr(pGateUser->sIPAddr));
	
	//协议约定key放在最前面
	packet <<nRawServerId <<nServerId << (BYTE)m_nGateIndex <<  pGateUser->lLogicKey  << pGateUser->nGlobalSessionId  ;
	packet << sip;
	packet.writeString(actorName);
	packet.writeString(pGateUser->sAccount); //账户名字

	packet<< Sex <<Job <<  Icon << Zy;

	bool bUseRandName =false;
	if(pGateUser->nLastRandNameIndex >=0 )
	{
		if(strncmp(actorName,pGateUser->sLastRandName,sizeof(actorName)) ==0)
		{
			bUseRandName =true;
		}
	}
	//如果是随机名字生效，那么发上次的index上去
	if(bUseRandName)
	{
		packet << pGateUser->nLastRandNameIndex;
	}
	else
	{
		packet <<(int) -1;
	}

	if(strToken[0] == 0)
	{
		packet.writeString(" ");
	}
	else
	{
		packet.writeString(strToken);
	}
	packet << nAdid;
	packet << nEnterId;
	BYTE canCreate = 1;
	int nNosetopenday = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo().nNoSetOpenDay;
	if( nNosetopenday > 0 && (GetLogicServer()->GetDaysSinceOpenServer() > nNosetopenday))
	{
		canCreate = 0;
	}
	packet <<(BYTE)canCreate;
	
	//这些数据已经没有用了,清掉
	if(pGateUser->nLastRandNameIndex >=0)
	{
		pGateUser->nLastRandNameIndex =-1;
		pGateUser->sLastRandName[0] =0;
	}
	
	OutputMsg(rmTip,_T("CreateActor[1] accounName:%s,actorName:%s, "),pGateUser->sAccount,actorName);
	pDbClient->flushProtoPacket(packet);
}

void CLogicGate::Client2DbDeleteActor(CDataPacketReader &reader,CLogicGateUser* pGateUser)
{
	if ( !pGateUser )
	{
		return;
	}
	CDataClient * pDbClient= m_pLogicServer->GetDbClient();
	if(pDbClient ==NULL || !pDbClient->connected())
	{
		ResponseLoginErrorAndCloseUser(pGateUser,enDbServerClose);
		return;
	}
	unsigned int nActorId=0;
	reader >> nActorId; //删除这个玩家
	CDataPacket &packet = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteActor);
	int nServerId = GetLogicServer()->GetServerIndex(); //当前的服务器的编号
	int nRawServerId= pGateUser->nRawServerIndex ;
	//协议约定key放在最前面
	packet <<nRawServerId << nServerId << (BYTE)m_nGateIndex <<  pGateUser->lLogicKey  << pGateUser->nGlobalSessionId ;
	packet <<nActorId ;
	packet.writeString(pGateUser->sAccount); //将玩家的账户名字发过去
	pDbClient->flushProtoPacket(packet);
}


void CLogicGate::OnDispatchUserMsg(CCustomServerGateUser *pGateUser, char* lpData, SIZE_T nSize)
{
	if ( !pGateUser )
	{
		return;
	}
	pGateUser->dwLastMsgTick = _getTickCount(); 
	CLogicGateUser* pLogicGateUser = (CLogicGateUser*)pGateUser;
	CDataPacketReader pack(lpData,nSize);
	BYTE nStstemID=0, nCmd =0;
	pack >> nStstemID >>  nCmd ;

	if( nStstemID == enLoginSystemId)
	{
		// if(! m_pLogicServer->GetSessionClient()->connected()) 
		// {
		// 	ResponseLoginErrorAndCloseUser(pLogicGateUser,enSessionServerClose);
		// 	return;
		// }

		// if(!  m_pLogicServer->GetDbClient()->connected() )
		// {
		// 	ResponseLoginErrorAndCloseUser(pLogicGateUser,enDbServerClose);
		// 	return;
		// }
		
		switch (nCmd)
		{
		case cCheckUserPasswd:		//检测密码
			{
				if (GetLogicServer()->IsCommonServer())
				{
					INT_PTR nTotalCount = 0;
					INT_PTR nCount = GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineActorCount(nTotalCount);
					// if ( nTotalCount >= GetLogicServer()->GetDataProvider()->GetGlobalConfig().nWuyiCrossMaxCount )
					// {
					// 	ResponseLoginErrorAndCloseUser(pLogicGateUser,enServerOnLineCountMAx);
					// 	return;
					// }
				}
				Client2SesionCheckUserPasswd(pack,pLogicGateUser);
				break;
			}
		case cCreateAccount: //创建账户
			{
				//必须屏蔽
				Client2SesionCreateAccount(pack,pLogicGateUser);
				break;
			}
		case cGetUserList:         //获取玩家的列表
			{
				SendDbGetActorList(pLogicGateUser);
				break;
			}
		case cCreateActor:         //创建角色
			{
				Client2DbCreateActor(pack,pLogicGateUser);
				break;
			}
		case cDeleteActor:        //删除角色
			{
				if ( (pLogicGateUser->boHasSecPsw && !pLogicGateUser->boCheckSecPswSucess) &&
					!GetGlobalLogicEngine()->GetSecondPswFlag() && !pLogicGateUser->boWhiteIpLogin)
				{
					return;
				}
				Client2DbDeleteActor(pack,pLogicGateUser);
				break;
			}
		case cRandName:          //随机名字
			{
				Client2DbRandName(pack,pLogicGateUser);
				break;
			}
		case cCheckSecondPsw:
			{
				Client2DbCheckSecondPsw(pack,pLogicGateUser);
				break;
			}
		case cCreateSecondPsw:
			{
				Client2DbCreateSecondPsw(pack,pLogicGateUser);
				OutputMsg(rmTip,_T("%s(%u) create second password."),pLogicGateUser->sAccount,pLogicGateUser->nGlobalSessionId);
				break;
			}
		case cCrossServerInit:
		{
			SimulateClient2SesionCheckUserPasswd(pack,pLogicGateUser);
			break;
		}
		}
		return ;
	}
	
	/*if(pGateUser->nGlobalSessionId && (CActor::s_nTraceAccountId <0 || CActor::s_nTraceAccountId ==  pGateUser->nGlobalSessionId ))
	{
	OutputMsg(rmTip,_T("1CLogicGate account=%d, system=%d,cmd=%d,size=%d"),pGateUser->nGlobalSessionId,(int)nStstemID,(int)nCmd,(int)nSize);
	}*/

	if(pLogicGateUser->nHandleIndex ==0) //如果收到确认
	{
		unsigned nAccountID =0; //帐户ID
		unsigned nActorID =0; //角色的ID
		int nLogSceneId = 0;
		if(nStstemID == enDefaultEntitySystemID &&nCmd == cLogin)
		{
			pack >> nAccountID >> nActorID;
			pack >> nLogSceneId;
			if ( !GetGlobalLogicEngine()->GetLoginSecondPswFlag() )
			{
				if ((pLogicGateUser->boHasSecPsw && !pLogicGateUser->boCheckSecPswSucess) &&
					!GetGlobalLogicEngine()->GetSecondPswFlag() && !pLogicGateUser->boWhiteIpLogin)
				{
					OutputMsg(rmWaning,_T("cannot login by second password account%u-actor%u-thread%u "),nAccountID,nActorID,GetCurrentThreadId());
					return;
				}
			}
			OnUserLogin(pLogicGateUser,nAccountID,nActorID,nLogSceneId); //通知逻辑服务器有人要登陆了

			//OutputMsg(rmNormal,_T("收到登陆包 nAccountID=%d,nActorID=%d"),nAccountID,nActorID );
		}
		else
		{
			OutputMsg(rmWaning,_T("%s在实体没有创建好的时候收到了未知的数据包,nStstemID=%d,nCmd=%d"),__FUNCTION__,nStstemID,nCmd );
		}
		return;	
	}

	LOGICUSERDATA UserData;
	UserData.nHandleIndex = pLogicGateUser->nHandleIndex;

	if (m_vFreeList.count() <= 0)
	{
		AllocGateSendPacketList(m_vFreeList,512);
	}
	if (m_vFreeList.count() <= 0)
	{
		OutputMsg(rmError, _T("%s这个位置创建新包失败，丢包中,子系统=%d，id=%d"), __FUNCTION__, nStstemID, nCmd);
		return;
	}
	CDataPacket* m_TempData = m_vFreeList.pop();//得到一个空闲的Datapacket
	if (!m_TempData)
	{
		return;
	}
	m_TempData->setPosition(0);
	//TRACE("free m_TempData:%d\n",(int)(m_TempData));

	//复制到内存块中
	(*m_TempData) << (unsigned int)UserData.nHandleIndex << (long long) pGateUser->dwGateTickCount; //将网关的时间拷贝过去
	(*m_TempData) << (long long)_getTickCount(); // 记录逻辑服务器收到此消息的时间
	m_TempData->writeBuf(lpData,nSize);

	m_vMsgList.add(m_TempData);	
}

void CLogicGate::Client2DbRandName(CDataPacketReader &reader,CLogicGateUser* pGateUser)
{
	if ( !pGateUser )
	{
		return;
	}
	CDataClient * pDbClient= m_pLogicServer->GetDbClient();
	if(pDbClient ==NULL || !pDbClient->connected())
	{
		ResponseLoginErrorAndCloseUser(pGateUser,enDbServerClose);
		return;
	}
	//随机名字
	BYTE bSex=0;
	reader >> bSex; //删除这个玩家
	CDataPacket &packet = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcGetRandName);
	int nServerId = GetLogicServer()->GetServerIndex(); //当前的服务器的编号
	int nRawServerId= pGateUser->nRawServerIndex ;
	//协议约定key放在最前面
	packet <<nRawServerId << nServerId <<  (BYTE)m_nGateIndex <<  pGateUser->lLogicKey  ;
	packet <<bSex ;
	pDbClient->flushProtoPacket(packet);
}

VOID CLogicGate::OnRun()
{
	Inherited::OnRun();

	if ( m_vFreeList.appendCount() > 0 )
	{
		m_vFreeList.flush();
	}
	//传递网络包给逻辑线程
	if ( m_vMsgList.count() > 0 )
	{
		GetGlobalLogicEngine()->GetNetWorkHandle()->PostUserDataList(m_nGateIndex, m_vMsgList);
	}

	ProcessUsers();
}

VOID CLogicGate::ProcessUsers()
{
	INT_PTR i;
	CLogicGateUser *pGateUser;
	Uint64 dwCurTick = _getTickCount();
	if (m_ProcessUserInterval.CheckAndSet(dwCurTick, true))
	{
		//必须降序循环
		for (i=m_UserList.count()-1; i>-1; --i)
		{
			pGateUser = (CLogicGateUser*)m_UserList[i];
			if ( !pGateUser )
				continue;
			//检查延时关闭
			if ( pGateUser->dwDelayCloseTick && dwCurTick >= (Uint64)(pGateUser->dwDelayCloseTick) )//dwDelayCloseTick貌似没设置
				pGateUser->boMarkToClose = TRUE;
			//如果长时间无通信数据则关闭用户

			if ( pGateUser->boMarkToClose  )
			{
				OutputMsg(rmWaning,_T("延迟将玩家踢下线"));
				CloseUser( pGateUser->nSocket, pGateUser->nServerSessionIndex ); 
				continue;
			}
			else if( pGateUser->dwLastMsgTick &&  dwCurTick - pGateUser->dwLastMsgTick >= 900000 && !IsDebuggerPresent()  ) 
			{
				OutputMsg(rmWaning,_T("15分钟没有收到玩家数据出现严重超时，踢下线,socket=%d,accoutid=%d"),(int)pGateUser->nSocket,pGateUser->nGlobalSessionId);
				if(pGateUser->nSocket == INVALID_SOCKET)
				{
					CloseUser( pGateUser->nSocket, pGateUser->nServerSessionIndex ,false); 
				}
				else
				{
					CloseUser( pGateUser->nSocket, pGateUser->nServerSessionIndex ,true); 
				}
				continue;
			}

		}
	}
}

void CLogicGate::Client2DbCheckSecondPsw(CDataPacketReader &reader,CLogicGateUser* pGateUser)
{
	if (!pGateUser)
	{
		return;
	}
	CDataClient * pDbClient= m_pLogicServer->GetDbClient();
	if(pDbClient ==NULL || !pDbClient->connected())
	{
		ResponseLoginErrorAndCloseUser(pGateUser,enDbServerClose);
	}
	else
	{
		ACTORNAME sPsw = {0};
		reader.readString(sPsw,ArrayCount(sPsw));
		CDataPacket &packet = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcCheckSecondPsw);
		int nServerId = GetLogicServer()->GetServerIndex();
		int nRawServerId= pGateUser->nRawServerIndex ;
		packet << nRawServerId << nServerId  << (BYTE)m_nGateIndex <<pGateUser->lLogicKey << pGateUser->nGlobalSessionId;
		
		MD5_CTX md5_context; 
		MD5Init(&md5_context); 
		MD5Update(&md5_context, (unsigned char*)sPsw, ArrayCount(sPsw));
		ACTORNAME smd5 = {0};
		MD5Final((unsigned char*)smd5,&md5_context);
		packet.writeString(smd5);
		
		//packet.writeString(sPsw);
		pDbClient->flushProtoPacket(packet);
	}
}

void CLogicGate::Client2DbCreateSecondPsw(CDataPacketReader &reader,CLogicGateUser* pGateUser)
{
	if ( !pGateUser )
	{
		return;
	}
	CDataClient * pDbClient= m_pLogicServer->GetDbClient();
	if(pDbClient ==NULL || !pDbClient->connected())
	{
		ResponseLoginErrorAndCloseUser(pGateUser,enDbServerClose);
	}
	else
	{
		ACTORNAME sPsw = {0};
		reader.readString(sPsw,ArrayCount(sPsw));
		CDataPacket &packet = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcCreateSecondPsw);
		int nServerId = GetLogicServer()->GetServerIndex();
		int nRawServerId= pGateUser->nRawServerIndex ;
		packet << nRawServerId << nServerId  << (BYTE)m_nGateIndex <<pGateUser->lLogicKey << pGateUser->nGlobalSessionId;
		
		MD5_CTX md5_context; 
		MD5Init(&md5_context); 
		MD5Update(&md5_context, (unsigned char*)sPsw, ArrayCount(sPsw));
		ACTORNAME smd5 = {0};
		MD5Final((unsigned char*)smd5,&md5_context);
		packet.writeString(smd5);
		
		//packet.writeString(sPsw);
		pDbClient->flushProtoPacket(packet);
	}
}

//跨服相关逻辑
//模拟账号验证
void CLogicGate::SimulateClient2SesionCheckUserPasswd(CDataPacketReader &pack,CLogicGateUser* pLogicGateUser)
{
	//模拟
	TCHAR name[ACCOUNT_NAME_BUFF_LENGTH];
	TCHAR passwd[MAX_PASSWD_LENGTH];
	char identity[64]; //身份证号码
	name[0]=0;
	passwd[0]=0;
	identity[0] =0;
	
	unsigned int nCrossActorId =0; //
	int nRawServerId =0;   // 要登陆的服务器的编号

	pack >> nCrossActorId;
	pack >> nRawServerId;
	

	pLogicGateUser->nRawServerIndex = nRawServerId;

	GetGlobalLogicEngine()->GetNetWorkHandle()->PostInternalMessage(SSM_CROSS_CEHCK_ACTORDATA,
		nCrossActorId, nRawServerId, m_nGateIndex,pLogicGateUser->lLogicKey);	
	GetGlobalLogicEngine()->GetCrossMgr().AddLoginActors(nCrossActorId, nRawServerId);
	OutputMsg(rmTip,_T("[CrossLogin 10] 发送到CrossMgr： nCrossActorId(%d),nRawServerId(%d), logickey(%lld)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),
		nCrossActorId,nRawServerId,pLogicGateUser->lLogicKey,pLogicGateUser->nSocket,pLogicGateUser->nGateSessionIndex,pLogicGateUser->nServerSessionIndex,GetCurrentThreadId());
}