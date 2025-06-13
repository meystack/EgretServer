#include "stdafx.h"

static char sName[64];

CBackHttpServer::CBackHttpServer(void)
{
	InitializeCriticalSection(&m_HttpServerLock);
	ClearParams();
	m_fdSetCount = 0;
}

CBackHttpServer::~CBackHttpServer(void)
{
	DeleteCriticalSection(&m_HttpServerLock);
}

void CBackHttpServer::DoHttpServerCreate(char* sAddr,int nPort)
{
	m_ServerSocket = socket(AF_INET,SOCK_STREAM,0); 
	m_AddrServer.sin_addr.s_addr = inet_addr(sAddr); 
	m_AddrServer.sin_family = AF_INET; 
	m_AddrServer.sin_port = htons(nPort); 
	int tmp =1;
	setsockopt(m_ServerSocket,SOL_SOCKET,SO_REUSEADDR,&tmp, sizeof(tmp));
	if (bind(m_ServerSocket,(SOCKADDR*)&m_AddrServer,sizeof(SOCKADDR)) == -1)
	{
		OutputMsg(rmError,"bind Error Port:%d",nPort);
	}
	if (listen(m_ServerSocket,1024) == -1)
	{
		OutputMsg(rmError,"listen Error Port:%d",nPort);
	}
	
 
	CreateSocketInfo(m_ServerSocket);
	OutputMsg(rmTip,_T("HTTPSERVER启动完成！"));
}


BOOL  CBackHttpServer::CreateSocketInfo(SOCKET s) 
{
	if (m_fdSetCount >= FD_SETSIZE)
	{
		return false;
	}
	SocketInfo info;
	info.socket = s;
	m_SocketInfoArr[m_fdSetCount++] = info;
	return true;
}
void CBackHttpServer::RemoveSocketInfoBySocket(SOCKET s)
{
	for (int i = 0; i < m_fdSetCount; i++)   
	{   
		if (m_SocketInfoArr[i].socket == s)
		{
			RemoveSocketInfo(i);
			return;
		}
	} 
}
void CBackHttpServer::RemoveSocketInfo(int index)
{
	if (index < 0 || index >= FD_SETSIZE || index >= m_fdSetCount)
	{
		return;
	}
	closesocket(m_SocketInfoArr[index].socket);
	for (int i = index; i < m_fdSetCount; i++)   
	{   
		m_SocketInfoArr[i] = m_SocketInfoArr[i+1];   
	} 
	m_fdSetCount--;
}
void CBackHttpServer::AddToSocketInfo(SocketInfo& sInfo,int nId, int nSpid, int nServerIndex)
{
	sInfo.id = nId;
	sInfo.spid = nSpid;
	sInfo.serverindex = nServerIndex;
}
void CBackHttpServer::HttpGetData()
{
	struct timeval timeout={0,0}; 
	FD_ZERO(&m_fdSocket);

	SOCKET maxFds = 0;
	for (int i =0; i < m_fdSetCount; i++)
	{
		SocketInfo& info = m_SocketInfoArr[i];
		FD_SET(info.socket,&m_fdSocket);
		if (info.socket > maxFds)
		{
			maxFds = info.socket;
		}
	}
	
#ifdef WIN32
	int nRet = select(NULL,&m_fdSocket,NULL,NULL,&timeout);
#else
	int nRet = select(maxFds+1,&m_fdSocket,NULL,NULL,&timeout);
#endif

	if (nRet <= 0)
	{
		return ;
	}
	
	for (int i =0; i < m_fdSetCount; i++)
	{
		SocketInfo& info = m_SocketInfoArr[i];
		if (FD_ISSET(info.socket,&m_fdSocket))
		{	
			if(info.socket == m_ServerSocket) 
			{
				int len=sizeof(SOCKADDR); 
#ifdef WIN32
				m_ClientSocket = accept(m_ServerSocket,(SOCKADDR*)(&m_AddrClient),&len); 
#else
				m_ClientSocket = accept(m_ServerSocket,(SOCKADDR*)(&m_AddrClient),(socklen_t*)&len); 
#endif
				if (m_ClientSocket == INVALID_SOCKET)
				{
					continue;
				}
				BOOL bReuseaddr=FALSE;
				setsockopt(m_ClientSocket, SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(BOOL));
				if (!CreateSocketInfo(m_ClientSocket))
				{
					SendResponse(m_ClientSocket,"CreateSocketInfo Error");
					OutputMsg(rmError,"CreateSocketInfo Error");
					return ;
				}
			}
			else
			{
				memset(info.data, 0, BUFSIZE); 
				int recvSize = recv(info.socket,info.data,BUFSIZE,0);		
				if (recvSize == 0 || recvSize == INVALID_SOCKET)
				{
					RemoveSocketInfoBySocket(info.socket);
					continue;
				}
			}
			
		}
	}
}

bool CBackHttpServer::DocumentParser(char * sDocument)
{
	
	EnterCriticalSection( &m_HttpServerLock );
	if (strlen(sDocument) == 0)
	{
		LeaveCriticalSection( &m_HttpServerLock );
		return false;
	}
	char * pStr = strtok(sDocument," ");
	char * s = strtok(NULL," ");
	if (pStr == NULL || s == NULL)
	{
		LeaveCriticalSection( &m_HttpServerLock );
		return false;
	}
	const char *d = "?&";
	char *p;
	p = strstr(s,"?");
	p = strtok(p,d);
	int iCount = 0;
	int nIndex = 0;
	while (p)
	{
		size_t value_len = 0 ;
		size_t name_len = 0;
		memset(sName,0,sizeof(sName));
		char* sValue = strstr(p, "=");
		if (sValue && strlen(sValue)> 1)
		{
			strncpy(sName,p,sValue-p);
			sValue++;
			name_len = strlen(sName);
			name_len = (name_len>MAXPARAMNAMELENGTH-1)?MAXPARAMNAMELENGTH-1:name_len ;
			if (name_len >= 1)
			{
				strncpy(m_Params[nIndex].sName,sName,name_len);
				m_Params[nIndex].sName[name_len] = '\0' ;
				value_len = strlen(sValue) ;
				value_len = (value_len>MAXPARAMVALUELENGTH-1)?MAXPARAMVALUELENGTH-1:value_len ;
				strncpy(m_Params[nIndex].sValue,sValue,value_len);
				m_Params[nIndex].sValue[value_len] = '\0' ;
				nIndex++;
			}
		}
		if (nIndex >= MAX_PARAM_NUM)
		{
			break;
		}
		iCount++;
		p=strtok(NULL,d);

	}
	for (int i=0;i<iCount;i++)
	{
		if((strcmp(m_Params[i].sName,"user") == 0) || (strcmp(m_Params[i].sName,"command") == 0))
		{
			string str = BackHttpUtility::URLDecode(m_Params[i].sValue);
			str.copy(m_Params[i].sValue, str.length(), 0);
			m_Params[i].sValue[str.length()] = 0;
			//break;
		}
	}
	memset(sDocument,0,BUFSIZE);
	LeaveCriticalSection( &m_HttpServerLock );
	return true;
}
void CBackHttpServer::SendResponse(SOCKET socket, LPCSTR sResult)
{
	char sendBuf[1024]; 
	memset(sendBuf, 0, 1024); 
	strcat(sendBuf, "HTTP/1.1 200\r\n");  
	strcat(sendBuf, "Server: BackStageServer\r\n");  
	strcat(sendBuf, "Content-Type:text/html;charset=utf8\r\n\r\n");  
	strcat(sendBuf, sResult);  
	size_t len = strlen(sendBuf);
	send(socket,sendBuf,(int)len,(int)0);
	RemoveSocketInfoBySocket(socket);
}
void CBackHttpServer::SendResponse(LPCSTR sResult,int nId, int nSpid, int nServerindex)
{
	for(int i=0; i <m_fdSetCount; i++)
	{
		SocketInfo& info = m_SocketInfoArr[i];
		if (info.id == nId && info.spid == nSpid && (info.serverindex ==0 || info.serverindex == nServerindex))
		{
			SendResponse(info.socket, sResult);
			break;
		}
	}
	OutputMsg(rmTip,"Result:%s Id:%d Spid:%d",sResult, nId, nSpid);
}

LPCTSTR CBackHttpServer::GetParamCharValue(char* sName)
{
	for (int i=0;i<MAX_PARAM_NUM;i++)
	{
		if (strcmp(m_Params[i].sName,sName) == 0)
		{
			return m_Params[i].sValue;
		}
	}
	return "";
}

int CBackHttpServer::GetParamIntValue(char* sName)
{
	for (int i=0;i<MAX_PARAM_NUM;i++)
	{
		if (strcmp(m_Params[i].sName,sName) == 0)
		{
			return atoi(m_Params[i].sValue);
		}
	}
	return 0;
}

SOCKET CBackHttpServer::GetClientSocket()
{
	return m_ClientSocket;
}
