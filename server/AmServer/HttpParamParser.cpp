#include "stdafx.h"

#define MAX_PARAM_COUNT 20
#define BUFSIZE 1024

static char sName[64];

void HttpParamParser::CreateHttpServer(char* sAddr,int nPort)
{
	m_ServerSocket = socket(AF_INET,SOCK_STREAM,0); 
	m_AddrServer.sin_addr.s_addr = inet_addr(sAddr); 
	m_AddrServer.sin_family = AF_INET; 
	m_AddrServer.sin_port = htons(nPort); 

	bind(m_ServerSocket,(SOCKADDR*)&m_AddrServer,sizeof(SOCKADDR)); 
	listen(m_ServerSocket,10); 

	FD_ZERO(&m_fdSocket);
	FD_SET(m_ServerSocket,&m_fdSocket);
}

char* HttpParamParser::HttpCommandGet()
{
	struct timeval timeout={0,200000}; //超时时间200豪秒
	fd_set fdRead = m_fdSocket;
#ifdef WIN32
	int nRet = select(NULL,&fdRead,NULL,NULL,&timeout);
#else
	int nRet = select(m_ServerSocket+1,&fdRead,NULL,NULL,&timeout);
#endif
	if (nRet <= 0)
	{
		return "";
	}
	if(FD_ISSET(m_ServerSocket,&fdRead)) 
	{
		int len=sizeof(SOCKADDR); 
#ifdef WIN32
		m_ClientSocket = accept(m_ServerSocket,(SOCKADDR*)(&m_AddrClient),&len); 
#else
		m_ClientSocket = accept(m_ServerSocket,(SOCKADDR*)(&m_AddrClient),(socklen_t*)&len); 
#endif
		BOOL bReuseaddr=FALSE;
		setsockopt(m_ClientSocket, SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(BOOL));
		m_ClientIP = inet_ntoa(m_AddrClient.sin_addr);
		char * recvBuf = (char *)malloc(BUFSIZE*sizeof(char)); 
		memset(recvBuf, 0, BUFSIZE); 
		recv(m_ClientSocket,recvBuf,BUFSIZE,0);
		return recvBuf;
	}

	return "";
}

void HttpParamParser::CloseHttpConnet()
{
	closesocket(m_ClientSocket);
}

void HttpParamParser::SendResponse(LPCSTR sContent)
{
	char sendBuf[1024]; 
	memset(sendBuf, 0, 1024); 
	strcat(sendBuf, "HTTP/1.1 200\r\n");  
	strcat(sendBuf, "Server: AMServer\r\n");  
	strcat(sendBuf, "Content-Type:text/html;charset=utf8\r\n\r\n");  
	strcat(sendBuf, sContent);  
	size_t len = strlen(sendBuf);
	send(m_ClientSocket,sendBuf,(int)len,(int)0);
}

void HttpParamParser::SendResponse(SOCKET nsocket, LPCSTR sContent)
{
	char sendBuf[1024]; 
	memset(sendBuf, 0, 1024); 
	strcat(sendBuf, "HTTP/1.1 200\r\n");  
	strcat(sendBuf, "Server: AMServer\r\n");  
	strcat(sendBuf, "Content-Type:text/html;charset=utf8\r\n\r\n");  
	strcat(sendBuf, sContent);  
	size_t len = strlen(sendBuf);
	::send(nsocket, sendBuf, (int)len, 0);
	/*
	int cnt = 0;
	int succ = 0;
	while( succ < len )
	{
		cnt++;
		if( cnt > 3 )
			return ;

		int ret = ::send(nsocket, sendBuf+succ, len-succ, 0);
		if( ret <= 0 )
		{
#ifdef WIN32
			int nError = WSAGetLastError();
			if( nError == WSAETIMEDOUT || nError == WSAEWOULDBLOCK )  continue; 
			OutputMsg( rmError, _T("send error = [%d]"),  nError );
			return ;
#else
			int nError = GetLastError();
			if( nError == ETIMEDOUT || nError == EAGAIN || nError == 0 ) continue;
			OutputMsg( rmError, _T("send error = [%d]   msg = [%s]"), nError, strerror(nError) );
			return ;
#endif
		}

		succ += ret;
	}
	*/
}

bool HttpParamParser::DocumentParser(char * sDocument)
{
	EnterCriticalSection( &m_ParseLock );
	if (strlen(sDocument) == 0) return false;
	char * pStr = strtok(sDocument," ");
	char * s = strtok(NULL," ");
	const char *d = "/?&";
	char *p;
	p = strtok(s,d);
	int iCount = 0;
	int nIndex = 0;
	while(p)
	{
		switch (iCount)
		{
		case 0:
			{
				_asncpytA(m_GameName,p);   // djrm
				break;
			}
		case 1:
			{
				_asncpytA(m_Spid,p);   // 37w
				break;
			}
		case 2:
			{
				_asncpytA(m_Interface,p);   // pay
				break;
			}
		}
		if (iCount > 2)
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
				name_len = (name_len>63)?63:name_len ;
				if (name_len >= 1)
				{
					strncpy(Params[nIndex].sName,sName,name_len);
					Params[nIndex].sName[name_len] = '\0' ;
					value_len = strlen(sValue) ;
					value_len = (value_len>255)?255:value_len ;
					strncpy(Params[nIndex].sValue,sValue,value_len);
					Params[nIndex].sValue[value_len] = '\0' ;
					nIndex++;
				}
			}
			if (nIndex >= MAX_PARAM_NUM) break;
		}
		iCount++;
		p=strtok(NULL,d);
	}

	for (int i=0;i<iCount;i++)
	{
		if(strcmp(Params[i].sName,"account") == 0 || strcmp(Params[i].sName,"channel") == 0 ||strcmp(Params[i].sName,"actorname") == 0)
		{
			string str = HttpUtility::URLDecode(Params[i].sValue);
			str.copy(Params[i].sValue, str.length(), 0);
			Params[i].sValue[str.length()] = 0;
		}
	}

	free(sDocument) ;
	LeaveCriticalSection( &m_ParseLock );
	return true;
}

int StrToInt(char* str)
{
	size_t len = strlen(str) ;
	int result = 0 ;
	for(int i=0; i<len; i++)
	{
		result = result * 10 ;
		result += (int)(str[i]-'0') ;
	}
	return result ;
}
unsigned int HttpParamParser::GetParamUIntValue(char* sName)
{
	for (int i=0;i<MAX_PARAM_NUM;i++)
	{
		if (strcmp(Params[i].sName,sName) == 0)
		{
			 return (unsigned int)_atoi64(Params[i].sValue);
		}
	}
	return 0;
}
int HttpParamParser::GetParamIntValue(char* sName)
{
	for (int i=0;i<MAX_PARAM_NUM;i++)
	{
		if (strcmp(Params[i].sName,sName) == 0)
		{
			return atoi(Params[i].sValue);
		}
	}
	return 0;
}

const char* HttpParamParser::GetParamCharValue(char* sName)
{
	for (int i=0;i<MAX_PARAM_NUM;i++)
	{
		if (strcmp(Params[i].sName,sName) == 0)
		{
			//if(strcmp(Params[i].sName,"account") == 0)
			//{
			//	return HttpUtility::URLDecode(Params[i].sValue).c_str();
			//}
			//else
				return Params[i].sValue;
		}
	}
	return "";
}

double HttpParamParser::GetParamDoubleValue(char* sName)
{
	for (int i=0;i<MAX_PARAM_NUM;i++)
	{
		if (strcmp(Params[i].sName,sName) == 0)
		{
			double dValue = 0;
			_stscanf(Params[i].sValue, _T("%lf"), &dValue) ;
			int iTemp = (int)(dValue * 10);
			dValue = double(iTemp) / 10 ;
			return dValue;
		}
	}
	return 0;
}

