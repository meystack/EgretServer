
#include "StdAfx.h"
//#include "WinService.h"
#include "FileLogger.h"
#include  <iostream>
#ifdef WIN32
	#include <dbghelp.h>
	#include <mbctype.h>
#endif
#include "PathThreadLocale.h"

#include <string>
using namespace std;

std::string g_ConfigPath;

extern const TCHAR szExceptionDumpFile[] = _T("./SessionManager.dmp");
BOOL SetupSessionManagerConfig(CSSManager*);
VOID ServerMain();
VOID ServiceMain(int argc, char** argv);

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

//会话管理器版本号
#define SS_MGR_KN_VERION   MAKEFOURCC(17, 3, 24,1)


/*
class CSessionService : public CWinService
{
public:
	CSessionService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{
		SetCurrentDirectory("../");	
		CSSManager *pSSManager;
		CFileLogger flog(_T("SessionServer_%s.log"), getCurrentTimeDesc());
		

		pSSManager = new CSSManager();

		if ( SetupSessionManagerConfig(pSSManager) )
		{
			if ( pSSManager->Startup() )
			{
				in_addr ia;
				ia.S_un.S_addr = SS_MGR_KN_VERION;
				OutputMsg( rmTip, _T("-------------------------------------------") );
				OutputMsg( rmTip, _T("会话管理器启动成功，核心版本号是%s"), inet_ntoa(ia) );
				OutputMsg( rmTip, _T("quit - 停止服务并退出程序") );
				OutputMsg( rmTip, _T("spfw - 显示性能状态窗口") );
				OutputMsg( rmTip, _T("sscw - 显示服务器连接窗口") );
				OutputMsg( rmTip, _T("lgr  - 重新加载角色网关路由表") );
				OutputMsg( rmTip, _T("lal  - 重新加载管理员登录表") );
				OutputMsg( rmTip, _T("kks  - 踢账号下线") );
				OutputMsg( rmTip, _T("fcmclose  - 关闭fcm") );
				OutputMsg( rmTip, _T("fcmopen  - 开启fcm") );

				OutputMsg( rmTip, _T("-------------------------------------------") );

				//CPerformanceWatcher	watcher(pSSManager);
				//watcher.resume();

				while (!m_boServiceExit)
				{
					Sleep(1000);
				}

				//watcher.terminate();
				//watcher.waitFor();
				pSSManager->Shutdown();
			}
		}
		else
		{
			OutputMsg(rmError, _T("读取会话管理器配置失败"));
		}

		delete pSSManager;
		return 0;
	}
};
*/

int main(int argc, char** argv)
{
	CTimeProfMgr::getSingleton().InitMgr();
#ifdef WIN32
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
	if (initLocalePath())
	{
		OutputMsg( rmError, _T("can not set locale path") );
		getc(stdin);
		return -2;
	}

#endif
	InitDefMsgOut();
	#ifdef WIN32	
	//flog.SetNeedOutput(true);
#else
	if (argc >=2)
	{
		//flog.SetNeedOutput(true);
	}	
	else
	{
		//flog.SetNeedOutput(false);
	}
#endif

	//unsigned long long ip1= inet_addr("111.170.68.213");

	//unsigned long long ip2= inet_addr("111.170.68.214");

	//char *sIp =inet_ntoa(*((in_addr*)&ip1));
	//char *sIp2 =inet_ntoa(*((in_addr*)&ip2));

#ifndef _SERVICE
	//if (argc == 2 && _tcsncicmp("/cmd",argv[1],4)==0)//平时调试用
	{
#ifdef WIN32
		SetCurrentDirectory("./");
#else
		string filename(argv[1]);   
		size_t found = filename.find_last_of("/\\");
		filename = filename.substr(0, found);
		if( filename[0] == '.' && filename.length()==1 )
			filename = "./" ;
		SetCurrentDirectory(filename.c_str());
#endif
		if(argc >=2)
		{
			g_ConfigPath = argv[1];
		}else
		{
			g_ConfigPath="SessionServerLinux.txt";
		}
		ServerMain();
	}
#else
	//else
	{
		//ServiceMain(argc, argv);
	}
#endif
	
	UninitDefMsgOut();
#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif
	

	return 0;
}

BOOL SetupSessionManagerConfig(CSSManager *lpSSManager)
{
	CSSConfig config;
	return config.loadConfig(lpSSManager);
}

/*
VOID ServiceMain(int argc, char** argv)
{
	LPTSTR sCmd = NULL;
	if (argc >= 2)
	{
		sCmd = argv[1];
	}
	CSessionService service("SessionService");
	service.Run(sCmd);
}
*/




VOID ServerMain()
{
	if (!FDOP::IsDirectory(_T("log")))
	{
		FDOP::DeepCreateDirectory(_T("log"));
	}
	CFileLogger flog(_T("./log/SessionServer_%s.log"), getCurrentTimeDesc());
	INT_PTR nError;
	TCHAR sCmdBuf[512];
	CSSManager *pSSManager;
	
	pSSManager = new CSSManager();
	
#ifdef WIN32
	if (initLocalePath())
	{
		OutputMsg( rmError, _T("can not set locale path") );
		getc(stdin);
		return ;
	}
#endif

	if ( SetupSessionManagerConfig(pSSManager) )
	{
		if ( pSSManager->Startup() )
		{
			in_addr ia;
			ia.s_addr= SS_MGR_KN_VERION;
			OutputMsg( rmTip, _T("-------------------------------------------") );
			OutputMsg( rmTip, _T("会话管理器启动成功，核心版本号是%s"), inet_ntoa(ia) );
			OutputMsg( rmTip, _T("quit - 停止服务并退出程序") );
			OutputMsg( rmTip, _T("spfw - 显示性能状态窗口") );
			OutputMsg( rmTip, _T("sscw - 显示服务器连接窗口") );
			OutputMsg( rmTip, _T("lgr  - 重新加载角色网关路由表") );
			OutputMsg( rmTip, _T("lal  - 重新加载管理员登录表") );
			OutputMsg( rmTip, _T("kks  - 踢账号下线") );
			OutputMsg( rmTip, _T("fcmclose  - 关闭fcm") );
			OutputMsg( rmTip, _T("fcmopen  - 开启fcm") );
			OutputMsg( rmTip, _T("-------------------------------------------") );
			
			char pBuff[256] ;
			
			strcpy(pBuff,"lcwl-");
			strcat(pBuff, pSSManager->getServerName());
			strcat(pBuff,"-V");
			strcat(pBuff,inet_ntoa(ia));
			SetConsoleTitle( pBuff );

			
			//CPerformanceWatcher	watcher(pSSManager);
			//watcher.resume();

			while ( TRUE )
			{
				//_getts(sCmdBuf);
				std::cin >> (sCmdBuf);
				//滤掉末尾的换行符
				nError = (int)strlen(sCmdBuf);
				while ( nError > 0 && sCmdBuf[nError-1] < 0x20 )
				{
					nError--;
					sCmdBuf[nError] = 0;
				}
				
				//重新加载路由命令
				if ( _tcsncicmp(sCmdBuf, _T("lgr"), 3) == 0 )
				{
					nError = pSSManager->LoadCharGateRouteTable();
					if ( nError >= 0 )
						OutputMsg( rmTip, _T("已加载%d个角色网关路由数据"), nError);
					else OutputMsg( rmError, _T("加载角色网关路由数据失败"));
					continue;
				}
				//重新加载管理登录命令
				if ( _tcsncicmp(sCmdBuf, _T("lal"), 3) == 0 )
				{
					nError = pSSManager->LoadAdministLoginTable();
					if ( nError >= 0 )
						OutputMsg( rmTip, _T("已加载%d个管理员登录记录"), nError);
					else OutputMsg( rmError, _T("加载管理员登录记录数据失败"));
					continue;
				}
				//踢账号下线
				if ( _tcsncicmp(sCmdBuf, _T("kks"), 3) == 0 )
				{
					pSSManager->getSessionServer()->PostCloseSessionByAccount(&sCmdBuf[4]);
					OutputMsg( rmTip, _T("发出关闭会话(%s)的消息"), &sCmdBuf[4] );
					continue;
				}
				//显示性能状态窗口
				if ( _tcsncicmp(sCmdBuf, _T("spfw"), 4) == 0 )
				{
					//watcher.ShowPerformanceWindow(TRUE);
					continue;
				}
				if ( _tcsncicmp(sCmdBuf, _T("sscw"), 4) == 0 )
				{
					//watcher.ShowServerConnectionWindow(TRUE);
					continue;
				}

				//测试打印会话
				if ( _tcsncicmp(sCmdBuf, _T("pss"), 3) == 0 )
				{
					pSSManager->getSessionServer()->PostInternalMessage(SSIM_DEBUG_PRINT_SESSIONS, 10, 0, 0);
					continue;
				}
				//开启防沉迷
				if ( _tcsncicmp(sCmdBuf, _T("fcmopen"), 7) == 0 )
				{
					pSSManager->SetFcmOpen(true);
					continue;
				}
				//关闭防沉迷
				if ( _tcsncicmp(sCmdBuf, _T("fcmclose"), 8) == 0 )
				{
					pSSManager->SetFcmOpen(false);
					continue;
				}

				//重新刷新下
				if ( _tcsncicmp(sCmdBuf, _T("rgm"), 3) == 0 )
				{
					CSSConfig config;
					config.readGMConfig(pSSManager);
					continue;
				}


				 if ( _tcsncicmp(sCmdBuf, _T("spf"), 3) == 0 )
				{
					CTimeProfMgr::getSingleton().dump();
					CounterManager::getSingleton().logToFile();
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
		
				Sleep(10);
			}

			//watcher.terminate();
			//watcher.waitFor();
			pSSManager->Shutdown();
		}
		else
		{
			OutputMsg(rmError, _T("pSSManager->Startup() 失败"));
			system("pause");
		}
	}
	else
	{
		OutputMsg(rmError, _T("读取会话管理器配置失败"));
		system("pause");
	}

	delete pSSManager;
	
}

