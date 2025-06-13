#include "StdAfx.h"
#include "SessionCenter.h"

CSessionCenter::CSessionCenter(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName,CSSManager *lpSSManager)
{
	m_pSManager = lpSSManager;
	this->ServerType = ServerType;
	this->ServerIndex = ServerIndex;
	strcpy(this->ServerName,ServerName);
	this->SetClientName(_T("会话中心服"));

	m_pSqlConect = m_pSManager->getSessionServer()->GetSql();
	memset(sCommonPaltformIp,0,sizeof(sCommonPaltformIp));
	nCommonPort = 0;
}

CSessionCenter::CSessionCenter()
{
	this->SetClientName(_T("会话中心服"));
	memset(sCommonPaltformIp,0,sizeof(sCommonPaltformIp));
	nCommonPort = 0;
	m_pSManager = NULL;
	m_pSqlConect = 0;
}


CSessionCenter::~CSessionCenter()
{
	
}

VOID CSessionCenter::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	switch(nCmd)
	{
	case jxInterSrvComm::SessionServerProto::sReqestPlatformAck:		//返回跨平台消息
		{
			OnRecvRequsetCommonPlat(inPacket);
			break;
		}

	case jxInterSrvComm::SessionServerProto::cLogicCheckPasswd:		//校验密码
		{
			CheckPlatformUserPassword(inPacket);
			break;
		}

	case jxInterSrvComm::SessionServerProto::CTransToCommonPlatform:		//请求返回原来服
		{
			OnReqestRawServer(inPacket);
			break;
		}

	default:
		{
			break;
		}
	}
}

VOID CSessionCenter::OnConnected()
{
	CDataPacket &pdata = allocProtoPacket(cSendSpid); //分配一个 网络包
	pdata << (int)(m_pSManager->GetSPGUID());
	flushProtoPacket(pdata);
}

VOID CSessionCenter::SetServerName(LPCTSTR sServerName)
{
	_tcsncpy(ServerName, sServerName, ArrayCount(ServerName) - 1);
	ServerName[ArrayCount(ServerName) - 1] = 0;
}


jxSrvDef::SERVERTYPE CSessionCenter::getLocalServerType()
{
	return ServerType;
}

LPCSTR CSessionCenter::getLocalServerName()
{
	return ServerName;
}

int CSessionCenter::getLocalServerIndex()
{
	return ServerIndex;
}

void CSessionCenter::OnSendConnectCommonPlatform(LPCTSTR sIp,int nPort)
{
	if(sIp == NULL) return;
	_asncpyt(sCommonPaltformIp, sIp);
	nCommonPort = nPort;

	if(connected())
	{
		if(sCommonPaltformIp && sIp && strcmp(sCommonPaltformIp,sIp)==0 && nPort > 0 && nPort == nCommonPort) return;

		SetServerHost(sCommonPaltformIp);
		SetServerPort(nCommonPort);
		if(nCommonPort >0)
		{
			Inherited::Startup();
		}
	}
	else
	{
		SetServerHost(sCommonPaltformIp);
		SetServerPort(nCommonPort);
		if(nCommonPort >0)
		{
			Inherited::Startup();
		}
	}
}

void CSessionCenter::OnRequesTranCommonPlatform(CDataPacketReader &inPacket)
{
	unsigned int nAccountId = 0, nActorId = 0;
	int nServerIndex = 0,nDestServerId = 0, nType = 0;
	inPacket >> nDestServerId >> nAccountId >> nActorId;
	inPacket >> nServerIndex >> nType;

	//nServerIndex = MAKELONG(nServerIndex,m_pSManager->GetSPGUID())	//spgid为服务器的高字节

	CDataPacket &pdata = allocProtoPacket(jxInterSrvComm::SessionServerProto::CTransToCommonPlatform);		//分配一个 网络包
	pdata << (int)nDestServerId << (int)nServerIndex;
	pdata << (unsigned int)nAccountId << (unsigned int)nActorId;
	pdata << (int)nType;
	flushProtoPacket(pdata);
}

void CSessionCenter::OnRecvRequsetCommonPlat(CDataPacketReader &inPacket)
{
	int nServerIndex = 0;
	inPacket >> nServerIndex;
	//nServerIndex = LOWORD(nServerIndex);
	m_pSManager->getSessionServer()->OnSendPlatformResultToClient(nServerIndex,inPacket);
}

//判断是否可以返回原来服
void CSessionCenter::OnReqestRawServer(CDataPacketReader &inPacket)
{
	unsigned int nAccountId = 0, nActorId = 0;
	int nServerIndex = 0,nDestServerId = 0, nType = 0;
	inPacket >> nDestServerId >> nAccountId >> nActorId;
	inPacket >> nServerIndex >> nType;

	nDestServerId = LOWORD(nDestServerId);

	int nResult = 0;
	if (m_pSManager->getSessionServer() && m_pSManager->getSessionServer()->UserCanLoginToServer(nDestServerId) == 0)
	{
		nResult = 1;
	}

	CDataPacket &pdata = allocProtoPacket(jxInterSrvComm::SessionServerProto::sReqestPlatformAck);
	pdata << (unsigned int)nActorId << (BYTE)jxInterSrvComm::SessionServerProto::neSuccess;
	pdata << (unsigned int)nAccountId << (int)nDestServerId << (int)nResult << (int)nType;
	flushProtoPacket(pdata);
}

//检测用户名密码
VOID CSessionCenter::CheckPlatformUserPassword(CDataPacketReader &reader)
{
	
	BYTE len =0;
	TCHAR name[ACCOUNT_NAME_BUFF_LENGTH];
	TCHAR passwd[MAX_PASSWD_LENGTH];
	int nLoginServerId =0; // 要登陆的服务器的编号
	int nRawServerId =0;   // 源的服务器的编号
	int nGateIndex =0;     //这个玩家在哪个

	UINT64 lKey =0;
	reader >> lKey ;

	unsigned long long lIp =0; //玩家登陆的ip

	reader.readString(name,sizeof(name));
	reader.readString(passwd,sizeof(passwd));
	reader >> nLoginServerId >> nRawServerId >>nGateIndex ; //要登陆的服务器，以及原始的服务器,在网关的编号

	reader >> lIp;
	
	TCHAR md5[128];
	TCHAR currentTime[32];

	TCHAR identity[64]; //身份证号码

	md5[0]=0;  //md5的值
	currentTime[0] =0; //当前的时间
	identity[0] =0; //身份证号码

	BYTE bCheck =0;

	reader >>bCheck;
	//if(bCheck) //如果需要
	{
		
	}

	if( reader.getAvaliableLength() >0 )
	{
		reader.readString( md5,sizeof(md5) -1); 
	}
	if( reader.getAvaliableLength() >0 )
	{
		reader.readString( currentTime,sizeof(currentTime)-1); 
	}
	if( reader.getAvaliableLength() >0)
	{
		reader.readString( identity,sizeof(identity)-1);   //读取身份证号码
	}

	/*
			enPasswdError =1,    //密码错误
			enNoAccount=2,       //没有这个账号
			enIsOnline =3,       //已经在线
			enServerBusy =4,     //服务器忙
			enServerClose =5,    //服务器没有开放 
			enSessionServerError =6 , //session服务器有问题，比如db没有连接好
			enServerNotExisting =7, //不存在这个服务器
			enFcmLimited =8 ,      //账户纳入防沉迷
			enSessionServerClose =9, //会话服务器处于关闭状态
			enDbServerClose =10,     //数据服务器处于关闭状态
			enGMLoginFailError =11,    //gm在非法的ip登陆
		*/
	GLOBALSESSIONOPENDATA sessionData;// 玩家的登陆的账户
	sessionData.nClientIPAddr = lIp;

	bool isLoginCommonSrv = (nRawServerId == nLoginServerId); //是否在普通服登陆

	bool isWhiteIpLogin =false;
	int nErrorID = CheckPlatformUserValid(name,passwd,sessionData,md5,currentTime,bCheck?true:false,isLoginCommonSrv,identity,isWhiteIpLogin);
	if(nErrorID)
	{
		static char* s_erroMsg[]=
		{
			"NoError",
			"password error",
			"No such account",
			"is Real Online",
			"Server busy",
			"Server close",
			"Session Server Error",
			"Server Not exist ",
			"Fcm Limited ",
			"Session Server Closeed",
			"Db Server Close",
			"gm login in invalid ip",
			"ip forbid login",
			"ip login too many", 
			"md5 error",
			"php time is expired",
			"time  format error",
			"Account Seal",
		};

		static int  s_nErrorCount =ArrayCount(s_erroMsg);
		if(nErrorID < s_nErrorCount)
		{
			char *sIp =inet_ntoa(*((in_addr*)&lIp)); //这个是IP
			OutputMsg( rmWaning, _T("CheckPlatformUserValid name=%s,pw=%s，errorcode=%d,errormsg=%s,serverid=%d,ip=%s"),name, passwd,nErrorID,s_erroMsg[nErrorID],nLoginServerId,sIp);
			
		
			if(( nErrorID ==enIpMd5Error || nErrorID == enSignOutofDate || nErrorID == enTimeFormatError)&&
				md5[0] != 0 && currentTime[0] !=0)
			{
				OutputMsg(rmWaning, _T("md5=%s,time=%s"),md5,currentTime );
			}
		}
		
		//pSSGateUser->nUserState = CSSGateUser::guLoginFail;//登陆失败
		//ResponseUserLoginState(pSSGateUser,nErrorID); //如果不成功，可以告诉客户端现在的错误码了
		
		//如果失败了，则向客户端发一个消息，登陆失败了
		CDataPacket &out = allocProtoPacket(jxInterSrvComm::SessionServerProto::sCheckPasswdResult);
		out << int (nGateIndex); //1字节的结果
		out << (Uint64)lKey;
		out << BYTE (nErrorID); //1字节的结果
		flushProtoPacket(out);

		//如果登陆的IP特别多，则进行自动封号和封账户处理	(跨平台不处理)

		OutputMsg(rmTip,_T("[Login] 验证账号失败 AccountName(%s), CurrentThreadId(%d)。"),name,GetCurrentThreadId());
	}
	else //成功了要做进一步的检测,检测是否已经登录过了，等等，这里可以做顶号的处理
	{
		sessionData.nServerIndex = nLoginServerId; //当前登陆的服务器id

		GAMESESSION cuRuentSession;
		
		//如果当前存在一个会话，那么就发消息去删除这么一条消息
		//如果在本服登陆，并且在跨服登陆，先踢下跨服的账户
		//if(nRawServerId == nLoginServerId) //如果是在普通服登陆
		//{
		//	//把公共服的踢掉
		//	if( m_pSSServer->GetSpecialServerSession(sessionData.nSessionId,m_nCommServerId))
		//	{
		//		OutputMsg(rmTip,"Kick cross server actor,accountid=%d,serverindex=%d",(int)sessionData.nSessionId,(int)m_nCommServerId);
		//		m_pSSServer->PostKickCrossActor(sessionData.nSessionId,m_nCommServerId);
		//	}

		//}
		int nSrcSrv = LOWORD(nRawServerId);
		if ( m_pSManager->getSessionServer()->GetSession( sessionData.nSessionId ,&cuRuentSession,nSrcSrv) )
		{
			m_pSManager->getSessionServer()->PostCloseSessionByAccountId(sessionData.nSessionId,cuRuentSession.lKey); 
		}
		
		m_pSManager->getSessionServer()->PostOpenSession(nLoginServerId,nRawServerId,sessionData.sAccount,
			sessionData.nSessionId,sessionData.nClientIPAddr,sessionData.dwFCMOnlineSec,sessionData.nGmLevel,lKey,nGateIndex,isWhiteIpLogin,true);

		OutputMsg(rmTip,_T("[Login] 验证账号成功 AccountName(%s), CurrentThreadId(%d)。"),name,GetCurrentThreadId());
	}

	
}

int CSessionCenter::CheckPlatformUserValid(PCHAR name, PCHAR passwd,GLOBALSESSIONOPENDATA & sessiDonata,PCHAR md5,PCHAR sCurrentTime,bool checkMd5,bool bCommonSrvLogin,PCHAR iIndentity,bool &isWhiteIpLogin)
{
	if (m_pSqlConect ==NULL || m_pSqlConect->Connected() ==FALSE) 
	{
		OutputMsg( rmError, "无法连接数据库");
		return enSessionServerError; 
	}

	int nColumnCount =0;
	int nRowCount =0;

	char sSqlStr[512];

	static bool bPhpCheck = m_pSManager->isCheckPhpSign(); //是否要监测php的秘钥
	static char * sPguid = m_pSManager->GetSPID();   //获取用户的spguid
	static char * sKey = m_pSManager->GetPhpKey(); //获得这个php的key

	char sServerId[24];
	sprintf(sServerId,"%d",sessiDonata.nRawServerId);

	//是不是通过白名单的IP登陆的
	isWhiteIpLogin =false;

	int nResult = enSuccess;

	unsigned long long lLoginIp =sessiDonata.nClientIPAddr; //登陆的ip
	bool isWhite =false; //是否在白名单中

	CBaseList<unsigned long long > &whiteips = m_pSManager->GetWhiteIpList();
	bool flag =false;
	if(whiteips.count() >0)
	{
		//判断是否在合法的ip范围里
		for(INT_PTR i=0;i < whiteips.count(); i++)
		{
			if(whiteips[i]== lLoginIp)
			{
				isWhite =true;
				isWhiteIpLogin =true;
				break;
			}
		}
	}

	//进行校验才进行验证
	if(!isWhite && bPhpCheck && checkMd5 && bCommonSrvLogin   ) //如果开启了监测php的，并且需要兼容一起的版本
	{

		//这里发生了错误
		if(sCurrentTime ==NULL || sCurrentTime[0]==0 )
		{
			nResult = enTimeFormatError;
		}
		else
		{
			size_t nNumLen = strlen(sCurrentTime);
			for(size_t j=0; j< nNumLen; j++)
			{
				if(sCurrentTime[j] > '9' || sCurrentTime[j] <'0')
				{
					nResult = enTimeFormatError;
					break;
				}
			}	

		}

		if(!nResult)
		{
			if( md5==NULL ||  md5[0] ==0)
			{
				nResult = enIpMd5Error;
			}
		}


		if( !nResult )
		{

			char buff[1024]; 

			strcpy(buff,sCurrentTime);
			strcat(buff,sServerId); //数字化的服务器的id
			strcat(buff,sPguid); //spguid
			strcat(buff,name);
			strcat(buff,sKey);

			unsigned char mdkey[64];
			MD5_CTX   ctx; 
			//使用 MD5加密玩家的密码
			MD5Init(&ctx); 
			MD5Update(&ctx,  (unsigned char *)buff,   (unsigned int)strlen(buff) );
			MD5Final(mdkey,&ctx);
			mdkey[32]=0;

			if( strcmp(md5,(char*)mdkey) != 0 )
			{
				OutputMsg(rmError,"Local raw string=%s,md5 error",buff);
				nResult = enIpMd5Error; //md5错误了
			}
			else
			{

				unsigned int nCurrentTime = strtoul(sCurrentTime,NULL,10);
				time_t sSrvTime;
				//struct  tm * timeInfo;
				time(&sSrvTime); //取了当前的时间

				unsigned int nDif = (unsigned int)(abs(sSrvTime -nCurrentTime));  //服务器的时间减去传入的时间

				static unsigned int s_timeDif =   m_pSManager->GetPhpTime(); //获取php的时间

				if(nDif > s_timeDif) //如果超过了3分钟
				{
					OutputMsg(rmError,"%s login ,time dif=%u seconds",name,nDif);

					nResult = enSignOutofDate; //md5错误了
				}

			}

		}
	}

	//////////////////下面判断是否在合法的ip里访问了
	bool bWhitePass = m_pSManager->GetWhitePass(); //是否只要白名单的才能通过
	if(bWhitePass  && isWhite ==false)
	{
		nResult =enIpError; 
	}

	//如果在白名单中，则不需要判断，否则需要判断是否在黑名单中
	if(!nResult && !isWhite )
	{
		CBaseList<unsigned long long > &blackips = m_pSManager->GetBlackIpList();
		bool flag =false;
		if(blackips.count() >0)
		{
			//判断是否在合法的ip范围里
			for(INT_PTR i=0;i < blackips.count(); i++)
			{
				if(blackips[i] == lLoginIp)
				{
					flag =true;
					break;
				}
			}
		}
		if(flag)
		{
			nResult =enIpError; 
		}
		else
		{
			//判断登陆的个数是否超过了限制
			//int nLoginLimitCount= m_pSSServer->GetLoginAccountCount(lLoginIp); 
			//if(nLoginLimitCount >  pMgr->GetIpLoginCount())
			//{
			//	//nResult =enIpTooManyConnect; 			
			//	OutputMsg(rmTip,"IP=%s,connect count=%d,limit=%d",
			//		inet_ntoa(*((in_addr*)&lLoginIp)),nLoginLimitCount,(int) pMgr->GetIpLoginCount());
			//	nResult =enIpTooManyConnect; 
			//}
		}
	}

	if(nResult) return nResult; //先做下判断，如果超过了，直接不让注册，避免数据库大量垃圾数据


	if(m_pSManager->IsAutoAccount())
	{
		char *sIndentity = NULL;
		if(iIndentity != NULL && iIndentity[0] !=0)
		{
			sIndentity = iIndentity;
		}
		else
		{
			sIndentity="000000198010100000"; 
		}
		sprintf(sSqlStr,"call djimportspuser(\"%s\",\"%s\",%d,\"%s\"),%lld",name,passwd,sServerId,sIndentity, lLoginIp);

		if(nResult != enSuccess)
		{
			return nResult;
		}

		//先都调用一下
		if( m_pSqlConect->RealQuery( sSqlStr,strlen(sSqlStr) ) )
		{
			OutputMsg( rmError, "数据库执行sql操作%s失败",sSqlStr);
			nResult = enSessionServerError;
			return nResult;
		}
		else
		{
			m_pSqlConect->ResetQuery();
		}
	}



	//__try
	{


		_snprintf(sSqlStr,sizeof(sSqlStr),szUserLoginGetGlobalUser,name);

		unsigned int nAccountId = 0; //账户的ID
		if ( m_pSqlConect->RealQuery( sSqlStr,strlen(sSqlStr) ) )
		{
			OutputMsg( rmError, "数据库执行sql操作%s失败",sSqlStr);
			nResult = enSessionServerError;
		}
		else 
		{
			nColumnCount = m_pSqlConect->GetFieldCount();  //获取列数
			nRowCount  = m_pSqlConect->GetRowCount(); //获取行数

			//读取角色网关路由表
			//OutputMsg( rmTip, _T("Read user data from Db"),sSqlStr);
			MYSQL_ROW pRow = m_pSqlConect->CurrentRow();
			if ( pRow && nRowCount >=1 )
			{

				if(pRow[0] && nColumnCount >=1)
				{
					sscanf(pRow[0], "%u", &nAccountId); //账户名字
				}
				TCHAR sDbPasswd[ACTOR_NAME_BUFF_LENGTH];
				if(pRow[1] && nColumnCount >=2)
				{
					_asncpytA(sDbPasswd,pRow[1]);
				}
				else
				{
					sDbPasswd[0] ='\0';
				}


				if (!isWhite && 0 != strcmp(sDbPasswd,passwd)  )
				{
					OutputMsg( rmError, "%s，dbpasswd=%s,inputwd=%s",name,sDbPasswd,passwd);
					if(sDbPasswd[0] =='@')
					{
						nResult =enAccountSeal; //账户被封停
					}
					else
					{
						nResult = enPasswdError;
					}

				}
				else
				{

					//OutputMsg( rmNormal, "%s密码确认成功,accountID =%d",name,nActorId);
					strncpy(sessiDonata.sAccount,name,sizeof(sessiDonata.sAccount));

					sessiDonata.nSessionId = nAccountId;
					if(pRow[6] && nColumnCount >=7)
					{
						sscanf(pRow[6], "%d", &sessiDonata.nGmLevel);
					}
					else
					{
						sessiDonata.nGmLevel = 0;
					}

					bool flag = m_pSManager->GetGmIpLogin(); //是否限制了IP登陆

					//GM登陆，需要判断是否是在合法的ip范围里登陆
					if(sessiDonata.nGmLevel >0 && flag  )
					{
						//获得ip列表
						CBaseList<unsigned long long > &ips = m_pSManager->GetGmIpList();
						bool isGMinvalid =false;


						//判断是否在合法的ip范围里
						for(INT_PTR i=0;i < ips.count(); i++)
						{
							if(ips[i]==  lLoginIp)
							{
								isGMinvalid =true;
								break;
							}
						}


						if(!isGMinvalid)
						{
							nResult =enGMLoginFailError; 
						}
					}
				}
			}
			else
			{
				//OutputMsg(rmWaning,_T("No Account  %s Found on db"),name);
				nResult = enNoAccount;
			}
			m_pSqlConect->ResetQuery();
		}



		if(nResult == enSuccess)
		{
			//只要在防沉迷关闭的时候才去判断防沉迷的时间，否则不判断
			if(m_pSManager->isFcmOpen() )
			{
				int nTime = -1;
				char sSqlStr[200];
				sprintf(sSqlStr,szGetFcmTime,nAccountId);

				if ( m_pSqlConect->RealQuery( sSqlStr, strlen(sSqlStr)) )
				{
					OutputMsg( rmError, "数据库执行sql操作%s失败",sSqlStr);
					nTime = -1;
				}
				else
				{
					MYSQL_ROW pRow = m_pSqlConect->CurrentRow();

					nColumnCount = m_pSqlConect->GetFieldCount();  //获取列数
					nRowCount  = m_pSqlConect->GetRowCount(); //获取行数

					if ( pRow && nColumnCount >=1 && nRowCount >=1 )
					{
						sscanf(pRow[0], "%d", &nTime); 
					}
					m_pSqlConect->ResetQuery();
				}
				sessiDonata.dwFCMOnlineSec = nTime;

			}
			else //否则全部的玩家都纳入防沉迷
			{
				sessiDonata.dwFCMOnlineSec =-1;
			}
		}

	}
	//__except( CustomExceptHandler(GetExceptionInformation() ))
	{
		//OutputMsg(rmError,"ACCOUNT=%s",name);
		//return nResult;
	}

	return nResult;
}


int CSessionCenter::SendSSCommonMsg(const int nServerIndex, LPCSTR sMsg, const size_t dwSize)
{
	AppendSendBuffer(sMsg, dwSize );

	return 0;
}

