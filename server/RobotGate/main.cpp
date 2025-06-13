#include "StdAfx.h"
//#include "WinService.h"
#include "DataProcess.h"
#include "SockProcess.h"
#include "SelectSockProcess.h"
#include "IOCPSockProcess.h"
#include "GateServer.h"
#include "GateServerConfig.h"
#include "FileLogger.h"
#include <string.h>
#ifndef WIN32
#include "client/linux/handler/exception_handler.h"
#endif
#ifndef WIN32
DWORD GetTickCount()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec*1000 + ts.tv_nsec/1000000);
}
#endif
 
 bool GateEngineRunning = true;
 
static void signal_handler(int sig_num)
{
	if( sig_num == SIGHUP ||
		sig_num == SIGINT ||
		sig_num == SIGTERM )
	{
		GateEngineRunning = false;
		OutputMsg( rmTip, _T("[SIGNAL] signal(%d) signal_handler... "), sig_num);
	}
	else
	{
		OutputMsg( rmError, _T("[SIGNAL] signal(%d) signal_handler... "), sig_num);
	}
	
}

void ServerMain(int argc, char** argv);
VOID ServiceMain(int argc, char** argv);

bool CreateAndStartGateServers(CBaseList<CRunSockProcesser*> &gateList,char *pFileName=NULL);
void DestroyGateServers(CBaseList<CRunSockProcesser*> &gateList);

//服务器是否在运行
bool g_ServerIsRunning =true;

void TestMemory()
{
	DWORD dwStart = GetTickCount();
	for(int i = 0; i < 1000000; i++) { char *p = (char*)malloc(i%1000000); free(p); } 

	DWORD dwEnd = GetTickCount();
	printf("***********************",dwEnd - dwStart);
	printf("100wMemoryTest:%d ms\n",dwEnd - dwStart);
}


#ifdef WIN32
class CGateService : public CWinService
{
public:
	CGateService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{		
		CFileLogger flog(_T("GateServer_%s.log"), getCurrentTimeDesc());
		CBaseList<CRunSockProcesser*> gateList;
		SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下

		
		if ( CreateAndStartGateServers(gateList) )
		{
			in_addr ia;
			ia.s_addr = RUNGATE_KRN_VERSION;

			OutputMsg( rmTip, _T("-------------------------------------------") );
			OutputMsg( rmTip, _T("网关服务器启动成功，核心版本号是%s"), inet_ntoa(ia) );
			OutputMsg( rmTip, _T("quit命令可以停止服务并退出程序") );
			OutputMsg( rmTip, _T("-------------------------------------------") );
			while (!m_boServiceExit)
			{
				Sleep(1000);
			}
		}
		DestroyGateServers(gateList);
		Sleep(2000);//休眠一秒等待日志全部刷新到文件
		return 0;
	}
};
#endif

int main(int argc, char **argv)
{
	//TestMemory();
#ifdef WIN32
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
#endif
	(void)signal(SIGHUP, SIG_IGN);
	(void)signal(SIGINT, signal_handler);
	(void)signal(SIGTERM, signal_handler);
	(void)signal(SIGPIPE, SIG_IGN);
	 
	InitDefMsgOut();
	CTimeProfMgr::getSingleton().InitMgr();
	//winitseed((unsigned int)_getTickCount()); //初始化随机数函数
	winitseed(0);

#ifndef _SERVICE
	//if (argc == 2 && strncmp("/cmd",argv[1],4)==0)//平时调试用
	{
#ifdef WIN32		
		SetCurrentDirectory("./");
#else
		std::string filename(argv[0]);
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
		ServiceMain(argc, argv);
	}
#endif

	UninitDefMsgOut();

#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

#ifdef WIN32
VOID ServiceMain(int argc, char** argv)
{
	LPTSTR sCmd = NULL;
	if (argc >= 2)
	{
		sCmd = argv[1];
	}
	CGateService service("GateService");
	
	service.Run(sCmd);
}
#endif

void ServerMain(int argc, char **argv)
{	
	CFileLogger flog(_T("../log/GateServer_%s.log"), getCurrentTimeDesc());
	CBaseList<CRunSockProcesser*> gateList;

	char *pFileName =NULL;
	if(argc >=2)
	{
		pFileName = argv[1];
	}else
	{
		pFileName="GateServerLinux.txt";
	}
	
	if ( CreateAndStartGateServers(gateList,pFileName) )
	{
		TCHAR sCmdBuf[512];
		in_addr ia;
		ia.s_addr = RUNGATE_KRN_VERSION;

		OutputMsg( rmTip, _T("-------------------------------------------") );
		OutputMsg( rmTip, _T("网关服务器启动成功，核心版本号是%s"), inet_ntoa(ia) );
		OutputMsg( rmTip, _T("spf命令可以查看当前的网关状态") );
		OutputMsg( rmTip, _T("quit命令可以停止服务并退出程序") );
		OutputMsg( rmTip, _T("-------------------------------------------") );

		while (g_ServerIsRunning)
		{
#ifdef WIN32
			if(_kbhit())
			{
				_getts(sCmdBuf);
			}
			else
			{
				Sleep(100);
				continue;
			}


			if ( strncmp(sCmdBuf, _T("\\q"), 2) == 0 
				|| strncmp(sCmdBuf, _T("exit"), 4) == 0
				|| strncmp(sCmdBuf, _T("quit"), 4) == 0 )
			{
				OutputMsg( rmTip, _T("正在停止网关服务...") );
				break;
			}
			else if ( strncmp(sCmdBuf, _T("spf"), 3) == 0 ) // 需要刷
			{
				for (INT_PTR i=0; i< gateList.count(); ++i)
				{
					CRunDataProcesser::ShowGateStatus(gateList[i]->GetDataProcesser());
				}

				CTimeProfMgr::getSingleton().dump();
			}
			else if (strncmp(sCmdBuf, _T("stst"), 4) == 0) // stst 10（ms为单位）
			{
				char *pParam = sCmdBuf + 4;
				while (*pParam == ' ')
					pParam++;
				if (pParam)
				{
					UINT_PTR nInterval = atoi(pParam);
					if (nInterval > 0)
					{
						CRunDataProcesser::s_nSndThreadSleepTime = (unsigned int)nInterval;
						OutputMsg(rmNormal, _T("Set SndThreadSleepTime[%d ms] Succ"), nInterval);
					}
				}
			}
			else if(strncmp(sCmdBuf, _T("ignore"), 6) == 0)
			{
				CRunDataProcesser::s_nIgnoreDataPacket = 1;
			}
			else if(strncmp(sCmdBuf, _T("resume"), 6) == 0)
			{
				CRunDataProcesser::s_nIgnoreDataPacket = 0;
			}
			
#else
				if(!GateEngineRunning) break;
#endif
			
			Sleep(10);
		}
	}
	DestroyGateServers(gateList);
	Sleep(2000);//休眠一秒等待日志全部刷新到文件
}

bool CreateAndStartGateServers(CBaseList<CRunSockProcesser*> &gateList,char *pConfig)
{
	CGateConfigList configList;
	CGateServerConfig configReader;
	CRunSockProcesser *pRunSock;
	CRunDataProcesser *pRunData;
	
	if ( configReader.loadConfig(configList,pConfig) )
	{
		INT_PTR i, nCount = configList.count();
		PGATECONFIG pConfigList = configList;
		for (i=0; i<nCount; ++i)
		{
			//创建并配置网关后台服务数据处理器
			pRunData = new CRunDataProcesser(__max(1024, pConfigList[i].GateServer.nMaxSession));
			pRunData->SetSendThreadCount( __max(1,pConfigList[i].GateServer.SendThreadCount) );
			pRunData->SetDataProcessType(dp_Default);
			pRunData->SetServerHost(pConfigList[i].BackServer.sHost); 
			pRunData->SetServerPort(pConfigList[i].BackServer.nPort); 
			pRunData->SetName(pConfigList[i].GateServer.sName);
			//创建并配置网关用户服务器处理器
			pRunSock = new CSelectRunSockProcesser();
			pRunSock->SetBindAddress( pConfigList[i].GateServer.sAddress );
			pRunSock->SetBindPort( pConfigList[i].GateServer.nPort );
			pRunSock->SetDataProcesser(pRunData);
			pRunData->SetRunSockProcesser(pRunSock);
			//将网关添加到列表中
			gateList.add(pRunSock);
			//启动网关
			if ( !pRunSock->InitBase() || !pRunData->Startup() || !pRunSock->Start() )
			{
				OutputMsg(rmError, _T("Start GateServer[%s] Failed"), pConfigList[i].GateServer.sName);
				return false;
			}
			OutputMsg(rmTip, _T("GateServer[%s] Started"), pConfigList[i].GateServer.sName);
		}
		return true;
	}
	return false;
}

void DestroyGateServers(CBaseList<CRunSockProcesser*> &gateList)
{
	CRunSockProcesser *pRunSock;
	CRunDataProcesser *pRunData;
	INT_PTR i, nCount = gateList.count();

	for (i=0; i<nCount; ++i)
	{
		//停止网关用户服务器处理器
		pRunSock = gateList[i];
		pRunSock->Stop();
		//停止网关后台服务数据处理器
		pRunData = pRunSock->GetDataProcesser();
		pRunData->Stop();
		//销毁网关对象
		pRunSock->SetDataProcesser(NULL);
		OutputMsg(rmTip, _T("GateServer[%s] Stoped"), pRunData->GetName());
		delete pRunSock;
		delete pRunData;
	}
	gateList.empty();
}
