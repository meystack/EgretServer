#include "stdafx.h"

CAMServerSocket::CAMServerSocket(char *pszFileName):m_ConfigPath(pszFileName)
{
	Config.ReadConfig(pszFileName);
	SetServiceName("");
	SetServiceHost("0.0.0.0");
	SetServicePort(Config.m_nAmport);
	Parser.CreateHttpServer("0.0.0.0",Config.m_nHttpport);
	for (int i=0;i<Config.GetPlatFormCount();i++)
	{
		CProduct * pProduct = new CProduct(Config.GetPlatForm(i));
		m_Products.add(pProduct);
	}
}


CAMServerSocket::~CAMServerSocket(void)
{
	for (INT_PTR i=m_Products.count()-1;i>-1;--i)
	{
		CProduct * pProduct = m_Products[i];
		SafeDelete(pProduct);
	}
	m_Products.clear();
}

CProduct* CAMServerSocket::GetProduct(char* gName,char* spid)
{
	for (int i=0;i<m_Products.count();i++)
	{
		CProduct * pProduct = m_Products[i];
		if (strcmp(pProduct->GetGName(),gName) == 0 && strcmp(pProduct->GetSpid(),spid) == 0)
		{
			return pProduct;
		}
	}
	return NULL;
}
char* CAMServerSocket::GetServerName()
{
	return Config.m_sServerName;
}
CCustomServerClientSocket* CAMServerSocket::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	if (pAddrIn == NULL)
	{
		return NULL;
	}
	OutputMsg( rmTip, "接受客户端连接!<%s>",inet_ntoa(pAddrIn->sin_addr));
	
	CAMClientSocket* pClientSocket = new CAMClientSocket(this,nSocket,pAddrIn);

	if(pClientSocket)
	{ 
		m_AMClientList.add(pClientSocket);
	}

	return (CCustomServerClientSocket* )pClientSocket;
}

VOID CAMServerSocket::DestroyClientSocket(CCustomServerClientSocket *pClientSocket)
{
	for (INT_PTR i=m_AMClientList.count()-1;i>-1;--i)
	{
		CCustomServerClientSocket* pSocket = (CCustomServerClientSocket*)m_AMClientList[i];
		if (pSocket && pSocket == pClientSocket)
		{
			m_AMClientList.lock();
			m_AMClientList.remove(i);
			m_AMClientList.unlock();
			break;
		}
	}
}

bool CAMServerSocket::CheckAuthorizationIP(long long * ipList, int size, char * ip )
{
	//char *pStr = (char*)malloc(strlen(strIP)+3);
	//_asncpytA(pStr,",");
	//strcat(pStr,strIP);
	//strcat(pStr,",");
	//bool bResult = strstr(pStr,IP) != NULL;
	//if (pStr)
	//{
	//	free(pStr);
	//}
	//return bResult;
	long long ipNum = inet_addr( ip ) ;
	for( int i=0 ; i<size; ++i )
	{
		if( ipNum == ipList[i] )
			return true ;
	}
	return false ;
}

CAMClientSocket* CAMServerSocket::GetSpidClientPro(char* spid)
{
	for (int i=0;i<m_AMClientList.count();i++)
	{
		CAMClientSocket* pClient = m_AMClientList[i];
		if (pClient && strcmp(pClient->GetSpid(),spid) == 0)
		{
			return pClient;
		}
	}
	return NULL;
}

void CAMServerSocket::RefreshProduct()
{
	Config.ReadConfig(m_ConfigPath.c_str());
	for (int i=0; i < Config.GetPlatFormCount(); i++)
	{
		PlatForm* pPlatForm = Config.GetPlatForm(i);
		CProduct * pProduct = GetProduct(pPlatForm->code,pPlatForm->spid);
		if (!pProduct)
		{
			CProduct * pProduct = new CProduct(Config.GetPlatForm(i));
			m_Products.add(pProduct);
			OutputMsg( rmTip, "新增了platform[%s]",pPlatForm->spid);
		}
		else
		{
			pProduct->SetConfig(pPlatForm);
		}
	}
}
void CAMServerSocket::OutPutContractIdMap()
{
	//CONTRACT_ID_MAP::iterator it;
	//for( int i=1; i<6; ++i ) // 最多4个任务id
	//{
	//	it = m_ContractIdMap.find(i);
	//	if( it != m_ContractIdMap.end() )
	//	{
	//		OutputMsg( rmTip, "ContractId=[%d], ContractStr=[%s]", it->first, it->second.c_str() );
	//	}
	//	else
	//		break;
	//}
}

void CAMServerSocket::OutPutAllClientSocket()
{
	
}
#ifdef WIN32
//转换文字编码为UTF-8
size_t convertToUTF8(const char* lpMsg, const size_t nMsgLen, char *pBuffer, const size_t BufferLen)
{
	WCHAR sWCBuf[4096];
	DWORD cch = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpMsg, (int)nMsgLen, sWCBuf, ArrayCount(sWCBuf)-1);
	if ( cch <= 0 )
		cch = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpMsg, (int)nMsgLen, sWCBuf, ArrayCount(sWCBuf)-1);
	if ( cch > 0 )
	{
		cch = (DWORD)WideCharToMultiByte(CP_UTF8, 0, sWCBuf, cch, pBuffer, (int)(BufferLen-1), NULL, NULL);
		if ( cch > 0 )
		{
			pBuffer[cch] = 0;
			return cch;
		}
	}
	return 0;
}
#endif
VOID CAMServerSocket::HandleHttpCommand(char *sDocument)
{
	int nType = 0 ;
	CProduct* pProduct =NULL;
	// sDocument解析后会被销毁
	if (!Parser.DocumentParser(sDocument))
	{
		Parser.SendResponse("Get Http Param Parser Error");
		OutputMsg( rmError, "Get Http Param Parser Error");
		goto ResultCode;
	}
	pProduct = GetProduct(Parser.m_GameName,Parser.m_Spid);
	if (!pProduct)
	{
		Parser.SendResponse("Cannot Find Product");
		OutputMsg( rmError, "Cannot Find Product");
		goto ResultCode;
	}
	if (!CheckAuthorizationIP(pProduct->GetIPList(), pProduct->GetIPListSize(), Parser.GetClientIP() ))
	{
		Parser.SendResponse("UnAuthorized IP");
		OutputMsg( rmError, "UnAuthorized IP");
		goto ResultCode;
	}
	nType = 0 ;
	if(strcmp(Parser.m_Interface,"pay") == 0) 
		nType = 1 ;
	else if(strcmp(Parser.m_Interface,"awd") == 0) 
		nType = 2 ;
	else if(strcmp(Parser.m_Interface,"pay2") == 0) 
		nType = 3 ;
	//else if(strcmp(Parser.m_Interface,"market") == 0)
	//	nType = 4 ;
	if ( nType == 0 )
	{
		Parser.SendResponse(Parser.m_Interface);
		OutputMsg( rmError, "Invalid interface : [%s]", Parser.m_Interface);
		goto ResultCode;
	}
	if( nType == 1 ||
		nType == 2 ||
		nType == 3 )
	{
		const char * opid = Parser.GetParamCharValue("opid");
		const char * account = Parser.GetParamCharValue("account");
		UINT money = Parser.GetParamIntValue("money");
		if (money == 0)
		{
			Parser.SendResponse("Invalid Money Value");
			OutputMsg( rmError, "Invalid Money Value");
			goto ResultCode;
		}
		UINT server = Parser.GetParamIntValue("server");
		UINT actorid = Parser.GetParamUIntValue("actorid");
		int level = Parser.GetParamIntValue("level");
		const char * channel = Parser.GetParamCharValue("channel");
		const char * sign = Parser.GetParamCharValue("sign");
		const char * actorname = Parser.GetParamCharValue("actorname");
		double rmb = Parser.GetParamDoubleValue("ram");
		char signCheck[200];
		memset(signCheck,0,sizeof(signCheck));
		if( nType == 1 || nType == 3 )
			sprintf(signCheck,"opid=%s&account=%s&actorid=%u&money=%u&server=%u&key=%s",opid,account, actorid,money,server,pProduct->GetPayKey() );
		else if( nType == 2 )
			sprintf(signCheck,"opid=%s&account=%s&actorid=%u&money=%u&server=%u&key=%s",opid,account, actorid,money,server,pProduct->GetAwdKey() );
		//char md5string[33];
		//_asncpytA(md5string,MD5(signCheck).toString().data());
		unsigned char md5string[64];
		MD5_CTX   ctx; 
		//使用 MD5加密玩家的密码
		MD5Init(&ctx); 
		MD5Update(&ctx,(unsigned char *)signCheck, (unsigned int)strlen(signCheck) );
		MD5Final(md5string,&ctx);
		md5string[32]=0;
		if (strcmp((char*)md5string,sign) != 0)
		{
			//if( nType == 2 )
			//{
			//	char awdSign[200];
			//	memset(awdSign,0,sizeof(awdSign));
			//	sprintf(awdSign,"opid=%s&account=%s&money=%u&server=%u&key=%s",opid,account,money,server,pProduct->GetPayKey() );
			//	unsigned char md5str[64];
			//	MD5_CTX   ctx; 
			//	//使用 MD5加密玩家的密码
			//	MD5Init(&ctx); 
			//	MD5Update(&ctx,(unsigned char *)awdSign, (unsigned int)strlen(awdSign) );
			//	MD5Final(md5str,&ctx);
			//	md5str[32]=0;
			//	if (strcmp((char*)md5str,sign) != 0)
			//	{
			//		Parser.SendResponse("Check Sign fail");
			//		OutputMsg( rmError, "Check Sign fail, the error sign:[%s],  the upload opid=%s, account=%s, money=%u, server=%u, key=%s", sign, opid,account,money,server,pProduct->GetAwdKey() );
			//		goto ResultCode;
			//	}
			//}
			//else
			//{
				Parser.SendResponse("Check Sign fail");
				if( nType == 2 )
					OutputMsg( rmError, "Check Sign fail, the error sign:[%s],  the upload opid=%s, account=%s, money=%u, server=%u, key=%s", sign, opid,account,money,server,pProduct->GetAwdKey() );
				else
					OutputMsg( rmError, "Check Sign fail, the error sign:[%s],  the upload opid=%s, account=%s, money=%u, server=%u, key=%s", sign, opid,account,money,server,pProduct->GetPayKey() );
				goto ResultCode;
			//}
		}
		UINT UserId = 0;

		OutputMsg( rmTip, _T("[%s] want to recharge [%u] YuanBao or HongLi"), account, money);
		//char sMsgBuf[512];
		//convertToUTF8(account, ArrayCount(sMsgBuf), sMsgBuf, ArrayCount(sMsgBuf));
	

		//actorid 目前由后台php判断其合法性
		int Result = pProduct->Pay(opid,account,money,rmb,server,nType,UserId,actorid,actorname,level,channel);
		LPCSTR sResponse = "";
		switch (Result)
		{
		case ERROR_NOT_CONNECT_DB:
			{
				sResponse = "Cannot Connect DB";
				break;
			}
		case ERROR_QUERY_DB:
			{
				sResponse = "Query DB Error";
				break;
			}
		case ERROR_NOT_REGDIST:
			{
				sResponse = "UnRegistered";
				break;
			}
		case ERROR_NOTFIND_PRODUCT:
			{
				sResponse = "Cannot Find Product";
				break;
			}
		case 0:
			{
				sResponse = "OK";// 0表示成功
				break;
			}
		case 1:
			{
				sResponse = "UNE"; // 1表示无此账号
				break;
			}
		case 2:
			{
				sResponse = "FA";// 2表示余额不足
				break;
			}
		case 3:
			{
				sResponse = "DOI";// 3表示订单号重复
				break;
			}
		default: sResponse = "UEE";// 异常出错
		}
		Parser.SendResponse(sResponse);

		if( strncmp(sResponse,"OK", 2) == 0 )
		{
			if(nType == 1 || nType == 3 )
				//OutputMsg( rmTip, _T("充值成功: %s充值了%u元宝"), account, money);
				OutputMsg( rmTip, _T("recharge succeed, [%s] recharge [%u] YuanBao, UserID=[%u]"), account, money, UserId);
			else 
				//OutputMsg( rmTip, _T("成功，发放%s红利%u"), account, money);
				OutputMsg( rmTip, _T("a grant of HongLi succeed, [%s] get [%u] HongLi, UserID=[%u]"), account, money, UserId);
		}
		else
		{
			if(nType == 1 || nType == 3 )
				//OutputMsg( rmError, _T("[%s]充值失败，错误：[%s]"), account, sResponse);
				OutputMsg( rmError, _T("[%s] recharge failed, errmsg: [%s], UserID=[%u]"), account, sResponse, UserId);
			else 
				//OutputMsg( rmError, _T("%s发放红利失败，错误：[%s]"), account, sResponse);
				OutputMsg( rmError, _T("a grant of HongLi for [%s] failed , errmsg: [%s], UserID=[%u]"), account, sResponse, UserId );
		}

		if (Result == 0)
		{
			CAMClientSocket* pClient = GetSpidClientPro(Parser.m_Spid);
			if (pClient)
			{
				money = pProduct->QueryAmount(UserId, actorid);
				pClient->SendPaySucceed(server,UserId, actorid, money);
			}
		}
	}
	
ResultCode:
	Parser.CloseHttpConnet();
}

VOID CAMServerSocket::SingleRun()
{
	ServerInherited::SingleRun();
	
	char* sDocument = Parser.HttpCommandGet();
	if (strlen(sDocument) > 0)
	{
		OutputMsg( rmTip, _T("%s"), HttpUtility::URLDecode(sDocument).c_str() );
		HandleHttpCommand(sDocument);
	}

}
