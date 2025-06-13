#include "StdAfx.h"
#ifdef WIN32
#include <dbghelp.h>
#include <mbctype.h>
#include <conio.h>
#else
// #include "client/linux/handler/exception_handler.h"
#endif
#include <string>
#include  <iostream>
#include "PathThreadLocale.h"

bool DBEngineRunning = true ;
//定义转储文件名称
extern const TCHAR szExceptionDumpFile[] = _T("./DBEngine.dmp");
BOOL SetupDBEngineConfig(CDBServer *lpDBEngine, const char *pszConfigFileName = NULL);
VOID ServerMain(int argc, char **argv);
VOID ServiceRun(int argc, char** argv);

//数据引擎版本号
#define DBEGN_KN_VERION			MAKEFOURCC(17, 3, 1, 1)
//数据引擎的数据结构版本

//#define	DBSDATATYPE_VERSION		0x010A1B0A
#ifndef WIN32
/*
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
	void* context,
	bool succeeded)
{
	printf("Dump path: %s\n", descriptor.path());
	return succeeded;
}

*/
static void signal_handler(int sig_num)
{
	if( sig_num == SIGHUP ||
		sig_num == SIGINT ||
		sig_num == SIGTERM )
	{
		OutputMsg( rmTip, _T("正在停止网关管理器...") );
		DBEngineRunning = false;
	}
}
#endif

#ifndef WIN32
int _kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}
#endif

#ifdef WIN32
int initLocalePath()
{
	//_MB_CP_SBCS   视所有字符均作为单字节字符，以便同时支持UTF8以及MBCS
	//Use single-byte code page. 
	//When the code page is set to _MB_CP_SBCS, a routine such as _ismbblead always returns false.
	_setmbcp(_MB_CP_SBCS);
	//★关键★ 设置"C"locale，视所有字符均作为单字节字符，以便同时支持UTF8以及MBCS
	return InstallThreadLocalePath("C");
}
#endif

void TestMemory()
{
#ifdef _DEBUG

#else
	/*
	DWORD dwStart = GetTickCount();
	for(int i = 0; i < 1000000; i++) { char *p = (char*)malloc(i%1000000); free(p); } 

	DWORD dwEnd = GetTickCount();
	printf("***********************",dwEnd - dwStart);
	printf("100wMemoryTest:%d ms\n",dwEnd - dwStart);
	*/

#endif
}

#ifdef WIN32
class CDbService : public CWinService
{
public:
	CDbService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{	
		SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下		
		CFileLogger flog(_T("log/DBServer_%s.log"), getCurrentTimeDesc());
		CDBServer *pDBEngine = new CDBServer();

		if ( !SetupDBEngineConfig(pDBEngine) )
		{
			OutputMsg( rmError, _T("读入配置文件失败，服务停止！"));
		}	
		else if ( pDBEngine->Startup() )
		{
			while (!m_boServiceExit)
			{
				Sleep(1000);
			}
			pDBEngine->Shutdown();
		}

		delete pDBEngine;

		return 0;
	}
};
#endif

int main(int argc, TCHAR* argv[])
{
#ifdef WIN32
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
#else
	//seh_init();
	/*
	google_breakpad::MinidumpDescriptor descriptor(".");
	google_breakpad::ExceptionHandler eh(descriptor,
		NULL,
		dumpCallback,
		NULL,
		true,
		-1);
	*/
	(void)signal(SIGHUP, SIG_IGN);
	(void)signal(SIGINT, signal_handler);
	(void)signal(SIGTERM, signal_handler);
	(void)signal(SIGPIPE, SIG_IGN);
#endif
	InitDefMsgOut();
#ifdef WIN32
	CoInitialize(NULL);
#endif
	CTimeProfDummy::SetOpenFlag(false);

	

#ifdef WIN32
	//flog.SetNeedOutput(true);
#else
	if (argc >=2)
	{
	//	flog.SetNeedOutput(true);
	}	
	else
	{
	//	flog.SetNeedOutput(false);
	}
#endif

#ifdef WIN32
	if (initLocalePath())
	{
		OutputMsg( rmError, _T("can not set locale path") );
		getc(stdin);
		return -2;
	}
#endif
	CTimeProfDummy::SetOpenFlag(false);
#ifndef _SERVICE
	//if (argc == 2 && _tcsncicmp("/cmd",argv[1],4)==0)//平时调试用
	{
#ifdef WIN32		
		SetCurrentDirectory("../");
#else
		std::string filename(argv[1]);
		size_t found = filename.find_last_of("/\\");
		filename = filename.substr(0, found);
		if( filename[0] == '.' && filename.length()==1 )
			filename = "./" ;
		SetCurrentDirectory(filename.c_str());
#endif
	    ServerMain(argc, argv);
	}
#else
	//else
	{
		ServiceRun(argc, argv);
	}
#endif

#ifdef WIN32
	CoUninitialize();
#endif
	UninitDefMsgOut();
#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

#ifdef WIN32
VOID ServiceRun(int argc, char** argv)
{
	LPTSTR sCmd = NULL;
	if (argc >= 2)
	{
		sCmd = argv[1];
	}
	TestMemory();
	CDbService service("DbService");
	service.Run(sCmd);
}

BOOL WINAPI CustomConsoleCtrlHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		Beep( 600, 200 ); 
		printf( "Ctrl-Close event\n\n" );
		return TRUE;
	}
	else
		return FALSE;
}
#endif


VOID ServerMain(int argc, char **argv)
{
	const char *pszConfigFileName = NULL;
	if (argc >= 2)
	{
		pszConfigFileName = argv[1];		
	}else
	{
		pszConfigFileName = "DBServerLinux.txt";
	}
	
	int nError;
	TCHAR sCmdBuf[512];
	CFileLogger flog(_T("./log/DBServer_%s.log"), getCurrentTimeDesc());
	CDBServer *pDBEngine = new CDBServer();
	CDBServer::s_pDBEngine = pDBEngine;
	//SetConsoleCtrlHandler(CustomConsoleCtrlHandler, TRUE);
	//DeleteMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
#ifdef WIN32
	if (initLocalePath())
	{
		OutputMsg( rmError, _T("can not set locale path") );
		getc(stdin);
		return ;
	}
#endif

	
	if ( !SetupDBEngineConfig(pDBEngine,pszConfigFileName) )
	{
		OutputMsg( rmError, _T("读入配置文件失败，服务停止！"));
	}	
	else if ( pDBEngine->Startup() )
	{
		//设置窗口标题
		CTimeProfMgr::getSingleton().InitMgr();
		
		char *pBuff  = (char*)malloc(256);
		in_addr ia;
		ia.s_addr = DBEGN_KN_VERION;
		strcpy(pBuff, pDBEngine->getServerName());
		strcat(pBuff,"-V");
		strcat(pBuff,inet_ntoa(ia));
		SetConsoleTitle(pBuff);
		free(pBuff);
		pBuff =NULL;

		TestMemory();
		
		OutputMsg( rmTip, _T("-------------------------------------------") );
		OutputMsg( rmTip, _T("数据服务器启动成功，核心版本号是%s"), inet_ntoa(ia) );
		
		//OutputMsg( rmTip, _T("数据结构版本号是：%X"), DBSDATATYPE_VERSION );
//		OutputMsg( rmTip, _T("角色基础数据结构大小是：%d"), sizeof(DBCHARBASICDATA) );
		OutputMsg( rmTip, _T("quit命令可以停止服务并退出程序") );
		OutputMsg( rmTip, _T("lgr 命令可以重新加载游戏网关路由表") );
		OutputMsg( rmTip, _T("-------------------------------------------") );
		unsigned long long startTick = _getTickCount();
		while ( pDBEngine->GetDbStartFlag() )
		{
#ifdef _MSC_VER
			sCmdBuf[0] = 0;

			if(_kbhit())
			{
				//_getts(sCmdBuf);
				std::cin >> (sCmdBuf);
			}
			else
			{
				Sleep(100);
				continue;
			}

			//重新加载路由命令
			if ( _tcsncicmp(sCmdBuf, _T("lgr"), 3) == 0 )
			{
				nError = pDBEngine->LoadGameServerRoute();
				if ( nError >= 0 )
					OutputMsg( rmTip, _T("已加载%d个游戏网关路由数据"), nError);
				else OutputMsg( rmError, _T("加载游戏网关路由数据失败"));
				continue;
			}
			//退出命令
			if ( _tcsncicmp(sCmdBuf, _T("\\q"), 2) == 0
				|| _tcsncicmp(sCmdBuf, _T("exit"), 4) == 0
				|| _tcsncicmp(sCmdBuf, _T("quit"), 4) == 0 )
			{
				OutputMsg( rmTip, _T("正在停止网关管理器...") );
				break;
			}
			else if ( _tcsncicmp(sCmdBuf, _T("spf"), 3) ==0) 
			{
				CTimeProfMgr::getSingleton().dump();
				pDBEngine->TraceGameServerRoute();
			}
			// asi interval (interval:save time interval, s)
			else if (_tcsncicmp(sCmdBuf, _T("asi"), 3) == 0)
			{
				char *pParam = sCmdBuf + 3;
				while (*pParam == ' ')
					pParam++;
				if (pParam)
				{
					UINT_PTR nInterval = atoi(pParam);
					if (nInterval > 0)
						CDBDataCache::s_nActorCacheSaveInterval = nInterval*1000;
				}
			}
			else if(_tcsncicmp(sCmdBuf,"dmp",3) ==0 ) 
			{
				DebugBreak();
			}
			else if(_tcsncicmp(sCmdBuf,"opentrace",9) ==0 ) 
			{
				CTimeProfDummy::SetOpenFlag(true);
			}
			else if(_tcsncicmp(sCmdBuf,"closetrace",10) ==0 ) 
			{
				CTimeProfDummy::SetOpenFlag(false);
			}
			else if(_tcsncicmp(sCmdBuf,"memory",6) ==0 ) 
			{
				pDBEngine->Trace();
			}
#else
			if (!DBEngineRunning) break;
#endif
			Sleep(10);
		}

		//pDBEngine->Shutdown();
	}

	delete pDBEngine;
}



BOOL SetupDBEngineConfig(CDBServer *lpDBEngine, const char *pszConfigFileName)
{
	CDBConfig	config;
	if ( !config.ReadConfig(pszConfigFileName) )
		return FALSE;

	lpDBEngine->RunEsqlFile(config.m_EsqlToolPath,config.DbConf.szDbName);
	lpDBEngine->SetServerName(config.ServerName);
	lpDBEngine->SetServerIndex(config.ServerIndex);
	lpDBEngine->SetGateServiceAddress(config.GateAddr.szAddr, config.GateAddr.nPort);
	//lpDBEngine->SetSessionServerAddress(config.SessionAddr.szAddr, config.SessionAddr.nPort);	
	lpDBEngine->SetNameSyncServerAddress(config.NameAddr.szAddr, config.NameAddr.nPort);
	lpDBEngine->SetDataServiceAddress(config.DataAddr.szAddr, config.DataAddr.nPort);
	lpDBEngine->SetSQLConfig(config.DbConf.szHost, config.DbConf.nPort, config.DbConf.szDbName, config.DbConf.szUser, config.DbConf.szPassWord,config.DbConf.boUtf8 > 0 ? true : false);
	
	//启动日志客户端	
	lpDBEngine->SetLogServerAddress(config.LogAddr.szAddr, config.LogAddr.nPort);
	//lpDBEngine->SetDBCenterAddress(config.DBCenterAddr.szAddr, config.DBCenterAddr.nPort);
	lpDBEngine->LoadNameGroup(config.m_sPrefixList, /*config.m_sConnectList,*/ config.m_sStuffixList);
	return TRUE;
}
