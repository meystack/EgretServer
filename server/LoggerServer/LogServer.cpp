#include "stdafx.h"

CLogServer::CLogServer(char *pszFileName)
{
	g_ConfigPath = pszFileName;
	pLoggerSrv = NULL;
	pChatLogSrv = NULL;
}

bool CLogServer::StartServer()
{
	pLoggerSrv = new CLoggerServerSocket();
	if(pLoggerSrv)
	{
		//SetConsoleTitle( pLoggerSrv->Config.SrvConf.szServiceName );
		//char *pBuff  = (char*)malloc(256);
		char pBuff[512];
		in_addr ia;
		ia.s_addr =DBEGN_KN_VERION;

		
		strcpy(pBuff,"lcwl-");

		strncat(pBuff, pLoggerSrv->Config.SrvConf.szServiceName,sizeof(pBuff) -10);
		strcat(pBuff,"-V");
		strcat(pBuff,inet_ntoa(ia));
		SetConsoleTitle(pBuff);
		


		if  (!pLoggerSrv->Startup())
		{
			return false;
		}

		pChatLogSrv = new CChatLogServerSocket();
		if(pChatLogSrv)
		{
			if  (!pChatLogSrv->Startup())
			{
				return false;
			}
		}
	}

	return true;
}

bool CLogServer::StopServer()
{
	if(pLoggerSrv)
	{
		pLoggerSrv->Stop();
	}

	if(pChatLogSrv)
	{
		pChatLogSrv->Stop();
	}

	SafeDelete(pChatLogSrv);
	SafeDelete(pLoggerSrv);

	return true;
}
