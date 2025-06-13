#pragma once

extern string g_ConfigPath;

class CLogServer
{
public:
	CLogServer(char *pszFileName);
	~CLogServer() {}

	bool StartServer();

	bool StopServer();
public:
	CLoggerServerSocket* pLoggerSrv;
	CChatLogServerSocket* pChatLogSrv;
};

class GameServerLog
{
public:
	static CLogServer * g_pLogServer ;
};

inline  CLogServer* GetLogServer( ){return GameServerLog::g_pLogServer;} 
