#include "StdAfx.h"
#ifdef WIN32
#include <dbghelp.h>
#include <mbctype.h>
#include <conio.h>
#else
//#include "client/linux/handler/exception_handler.h"
#endif
#include "PathThreadLocale.h"
#include  <iostream>
bool LogicEngineRunning = true ;
const TCHAR szExceptionDumpFile[] = _T(".\\LogicServer.dmp");
BOOL SetupLogicServerConfig(CLogicServer *lpLogicServer, const char *pszConfigFileName = NULL);
VOID ServerMain(int argc, char **argv);
//VOID ServiceMain(int argc, char** argv);

CLogicServer * GameServerEntry::g_pLogicServer  = NULL;
#ifndef WIN32
DWORD GetTickCount()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec*1000 + ts.tv_nsec/1000000);
}
#endif
 
void TestMemory()
{
#ifdef _DEBUG

#else

	printf("***********************");
	for(int j=1; j<= 1; j++)
	{
		printf("Times=:%d\n",j);
		DWORD dwStart = GetTickCount();
		for(int i = 0; i < 1000000; i++) { char *p = (char*)malloc(i%1000000); free(p); } 

		DWORD dwEnd = GetTickCount();
		
		printf("100wMemoryTest:%d ms\n",dwEnd - dwStart);
		/*
		dwStart = GetTickCount();

		CBufferAllocator data ;
		for(int i = 0; i < 1000000; i++) 
		{ 
			char *p =(char*) data.AllocBuffer(i%1000000);

			data.FreeBuffer(PVOID (p));
		} 
		dwEnd = GetTickCount();
		printf("100wMemoryTest2:%d ms\n",dwEnd - dwStart);
		*/

	}
	
	

#endif
}

#ifdef WIN32
class CLogicService : public CWinService
{
public:
	CLogicService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{
		SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下		
		CFileLogger flog(_T("data/log/LogicServer_%s.log"), getCurrentTimeDesc());

		GameServerEntry::g_pLogicServer  = new CLogicServer();

		if ( SetupLogicServerConfig(GameServerEntry::g_pLogicServer) )
		{
			if ( GameServerEntry::g_pLogicServer->StartServer() )
			{
				//in_addr ia;
				//ia.S_un.S_addr = LOGIC_KN_VERSION;

				OutputMsg( rmTip, _T("-------------------------------------------") );
				OutputMsg( rmTip, _T("逻辑服务器启动成功，核心版本号是%s"),CLogicServer::GetLogicVersion() );
				OutputMsg( rmTip, _T("-------------------------------------------") );
				while (!m_boServiceExit  && GameServerEntry::g_pLogicServer->IsStart() )
				{
					Sleep(1000);
				}
				GameServerEntry::g_pLogicServer->StopServer();
			}			
		}

		delete GameServerEntry::g_pLogicServer;

		return 0;
	}
};

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

#ifdef WIN32
#else
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
		OutputMsg( rmTip, _T("signal_handler...") );
		LogicEngineRunning = false;
		if(GetGlobalLogicEngine()->GetStatisticMgr())
			GetGlobalLogicEngine()->GetStatisticMgr()->LogTimeFile();
		CSingleObjectAllocStatMgr::getSingleton().logToFile();
		CounterManager::getSingleton().logToFile();
		HandleMgrCollector::getSingleton().logToFile();
		CTimeProfMgr::getSingleton().dump();
		CFileLogger::DumpToFile();
	}
}
#endif
int main(int argc, char** argv)
{		
#ifdef WIN32
	SetMiniDumpFlag(MiniDumpWithFullMemory);
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );	
#else
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
	//InitChatMsgOut();
	CTimeProfMgr::getSingleton().InitMgr();
	TestMemory();
#ifdef WIN32
	//安装线程UTF-8的locale补丁
	if (initLocalePath())
	{
		OutputMsg( rmError, _T("can not set locale path") );
		getc(stdin);
		return -2;
	}
#endif
	{
		if (argc == 2 && _tcsncicmp("/svc",argv[1],4)==0)
		{
			//ServiceMain(argc, argv);
		}
				
#ifdef WIN32		
		SetCurrentDirectory("./");
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


	CSingleObjectAllocStatMgr::getSingleton().logToFile();
	CounterManager::getSingleton().logToFile();
	HandleMgrCollector::getSingleton().logToFile();
	CTimeProfMgr::getSingleton().clear();
	CounterManager::getSingleton().clear();	
	CSingleObjectAllocStatMgr::getSingleton().clear();
	//ClearChatMsgOut();
	UninitDefMsgOut();
#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}




VOID ServerMain(int argc, char **argv)
{	
	const char *pszConfigFileName = NULL;
	if (argc >= 2)
	{
		pszConfigFileName = argv[1];		
	}
	else
	{
		pszConfigFileName = "LogicServerLinux.txt";
	}
	
	CFileLogger flog(_T("./log/LogicServer_%s.log"), getCurrentTimeDesc());
	GameServerEntry::g_pLogicServer  = new CLogicServer();
	if ( SetupLogicServerConfig(GameServerEntry::g_pLogicServer, pszConfigFileName) )
	{
		if ( GameServerEntry::g_pLogicServer->StartServer() )
		{
			TCHAR sCmdBuf[512];
			in_addr ia;
			ia.s_addr = LOGIC_KN_VERSION;
			bool bSaveFlag = false;

			OutputMsg( rmTip, _T("-------------------------------------------") );
			OutputMsg( rmTip, _T("逻辑服务器启动成功，核心版本号是%s"), inet_ntoa(ia) );
			OutputMsg( rmTip, _T("quit   停止服务并退出程序") );
			OutputMsg( rmTip, _T("checks 检查脚本错误") );
			OutputMsg( rmTip, _T("rsf    重新加载脚本") );
			OutputMsg( rmTip, _T("-------------------------------------------") );
			OutputMsg(rmTip,"Main thread id=%d",GetCurrentThreadId());

			while (GameServerEntry::g_pLogicServer->IsStart())
			{
#ifdef WIN32
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

				//退出命令
				if ( _tcsncicmp(sCmdBuf, _T("\\q"), 2) == 0 
					|| _tcsncicmp(sCmdBuf, _T("exit"), 4) == 0
					|| _tcsncicmp(sCmdBuf, _T("quit"), 4) == 0 )
				{
					OutputMsg( rmTip, _T("正在停止逻辑服务...") );
					break;
				}
				else if (_tcsncicmp(sCmdBuf, _T("dmpexit"), 7) == 0)
				{
					OutputMsg( rmTip, _T("正在停止逻辑服务...") );
					bSaveFlag = true;
					break;
				}

				else if (_tcsncicmp(sCmdBuf, _T("memory"), 6) == 0)
				{
					GetGlobalLogicEngine()->DumpDataAllocator();
				}
				//显示脚本内存管理器状态
				else if (_tcsncicmp(sCmdBuf, _T("tsms"), 4) == 0)
				{

					OutputMsg( rmTip, _T("---------------------------------------") );					
					OutputMsg( rmTip, _T("Script Memory Manager Status:") );					
					OutputMsg( rmTip, _T("  Block Count: %lld"), ScriptMemoryManager::getAvaliableMemBlockCount());					
					OutputMsg( rmTip, _T("  Total Size : %lld MB"), ScriptMemoryManager::getAvaliableMemSize()/1024/1024);					
					OutputMsg( rmTip, _T("---------------------------------------") );					
				}
				else if(_tcsncicmp(sCmdBuf,"spf",3) ==0 )
				{
					OutputMsg( rmTip, _T("log the perfermance ") );
					GetGlobalLogicEngine()->GetStatisticMgr()->LogTimeFile();
					CSingleObjectAllocStatMgr::getSingleton().logToFile();
					CounterManager::getSingleton().logToFile();
					HandleMgrCollector::getSingleton().logToFile();
					CTimeProfMgr::getSingleton().dump();
					flog.Dump();
				}
				else if(_tcsncicmp(sCmdBuf,"dmp",3) ==0 ) 
				{
					DebugBreak();
				}
				else if (_tcsncicmp(sCmdBuf, "smdn", 4) == 0) // 设置怪物的逻辑循环等分数目
				{
					char *pParam = sCmdBuf + 4;
					while (*pParam == ' ')
						pParam++;
					if (pParam)
					{
						UINT_PTR nCount = atoi(pParam);
						if (nCount > 0)
							GetGlobalLogicEngine()->GetEntityMgr()->SetMonsterDivNum(nCount);
					}	
				}
				else if (_tcsncicmp(sCmdBuf, "sndn", 4) == 0) // 设置NPC的逻辑循环等分数目
				{
					char *pParam = sCmdBuf + 4;
					while (*pParam == ' ')
						pParam++;
					if (pParam)
					{
						UINT_PTR nCount = atoi(pParam);
						if (nCount > 0)
							GetGlobalLogicEngine()->GetEntityMgr()->SetNpcDivNum(nCount);
					}	
				}
				else if (_tcsncicmp(sCmdBuf, "spdn", 4) == 0) // 设置NPC的逻辑循环等分数目
				{
					char *pParam = sCmdBuf + 4;
					while (*pParam == ' ')
						pParam++;
					if (pParam)
					{
						UINT_PTR nCount = atoi(pParam);
						if (nCount > 0)
							GetGlobalLogicEngine()->GetEntityMgr()->SetPetDivNum(nCount);
					}	
				}
				else if(_tcsncicmp(sCmdBuf, "asi", 3) == 0)
				{
					char *pParam = sCmdBuf + 3;
					while (*pParam == ' ')
						pParam++;
					if (pParam)
					{
						UINT_PTR nInterval = atoi(pParam);
						if (nInterval > 0)
							CActor::m_sSaveDBInterval = (int)nInterval*1000;
					}	
				}
				else if(_tcsncicmp(sCmdBuf, "item", 4) == 0)  // 输出物品的分配情况
				{
					GetGlobalLogicEngine()->TraceItem();
				}
				else if(_tcsncicmp(sCmdBuf, "rkf", 3) == 0)  // 刷新跨服配置
				{
					GetLogicServer()->ReloadCrossConfig();
				}
				else if(_tcsncicmp(sCmdBuf, "kfid", 4) == 0)
				{
					char *pParam = sCmdBuf + 4;
					while (*pParam == ' ')
						pParam++;
					if (pParam)
					{
						int nId = atoi(pParam);
						if (nId >= 0)
							GetLogicServer()->SetCommonServerId(nId);
					}	
				}
				else if(_tcsncicmp(sCmdBuf, "openperf", 8) == 0)
				{
					GetGlobalLogicEngine()->SetOpenPerfLog(true);
				}
				else if(_tcsncicmp(sCmdBuf, "closeperf", 9) == 0)
				{
					GetGlobalLogicEngine()->SetOpenPerfLog(false);
				}
				else if(_tcsncicmp(sCmdBuf, "fb", 2) == 0)
				{
					GetGlobalLogicEngine()->GetFuBenMgr()->Trace();
				}
				else if(_tcsncicmp(sCmdBuf, "ref", 3) == 0)
				{
					GetGlobalLogicEngine()->GetFuBenMgr()->TraceRefreshPos();
				}
				else if(_tcsncicmp(sCmdBuf, "resetfb", 7) == 0)
				{
					GetGlobalLogicEngine()->GetFuBenMgr()->ResetFbRefresh();
				}
				else if(_tcsncicmp(sCmdBuf, "resetkf", 7) == 0)		//开服时间配置错误，十天活动清挡
				{
					
				}
				else if(_tcsncicmp(sCmdBuf,"scpsw",5) == 0)
				{
					GetGlobalLogicEngine()->SetLoginSecondPsw();
				}
				else if(_tcsncicmp(sCmdBuf,"checks",6) == 0)
				{
					char *pParam = sCmdBuf + 6;
					pParam++;
					GetGlobalLogicEngine()->GetGlobalNpc()->CheckAllScript(pParam);
				}
				else if(_tcsncicmp(sCmdBuf,"rsf",3) == 0)
				{
					GetGlobalLogicEngine()->GetGlobalNpc()->ReloadAllNpc();
				}
#else
				if(!LogicEngineRunning) break;
#endif

				Sleep(10);
			}
			GameServerEntry::g_pLogicServer->StopServer(bSaveFlag);
		}
		else
		{
			printf("Start LogicServer failed。\nPress Any Key To Quit...\n");
			int c = getc(stdin);
		}
	}
	delete GameServerEntry::g_pLogicServer;
	CFileLogger::DumpToFile();
}

//VOID ServiceMain(int argc, char** argv)
//{
//	LPTSTR sCmd = NULL;
//	if (argc >= 2)
//	{
//		sCmd = argv[1];
//	}
//	CLogicService logicService("LogicService");
//	logicService.Run(sCmd);
//}

BOOL SetupLogicServerConfig(CLogicServer *lpLogicServer, const char *pszConfigFileName)
{
	CLogicServerConfig config;	
	if (config.loadServerConfig(lpLogicServer, pszConfigFileName) == false)
	{
		return FALSE;
	}
	//client
	config.LoadNewCrossClientConfig(lpLogicServer);
	//server
	config.LoadNewCrossServerConfig(lpLogicServer);//设置原服还是跨服
	// 
	if (lpLogicServer->IsCrossServer())
	{
		config.LoadLocalCSConfigByCross(lpLogicServer);
	}
	else
	{ 
		//config.LoadLocalCSConfigByCross(lpLogicServer);
		config.LoadLocalCSConfigBySource(lpLogicServer);
	}
		
	return TRUE;
} 