#include "stdafx.h" 

#ifdef WIN32
#include <dbghelp.h>
#include "utf8_output.h"
#include "WinService.h"
#include <mbctype.h>
#include <conio.h>
#else

#include "LinuxTimer.h"
#include "SysApi.h"

#endif

#include "PathThreadLocale.h"
#include <iostream>
#include <string>
using namespace std;

const char szExceptionDumpFile[] = ".\\LoggerServer.dmp";
VOID ServiceMain(int argc, char** argv);
VOID ServerMain(int argc, char** argv);

#ifndef WIN32

string g_ConfigPath;

void changemode(int dir)
{
	static struct termios oldt, newt;

	if ( dir == 1 )
	{
		tcgetattr( STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	}
	else
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}

#endif

CLogServer *GameServerLog::g_pLogServer = NULL;

#ifdef WIN32
class CLogService : public CWinService
{
public:
	CLogService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{
		// 创建 套接字服务类
		SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下
		CFileLogger flog(_T("LoggerServer_%s.log"), getCurrentTimeDesc());
		
		CLoggerServerSocket* pLoggerSrv = new CLoggerServerSocket();
		OutputMsg( rmTip, _T("正在启动网关服务……") );
		if (pLoggerSrv->Startup())
		{
			OutputMsg( rmTip, _T("启动网关服务成功!") );
			while (!m_boServiceExit)
			{
				Sleep(1000);
			}
		}
		else	//启动服务失败
		{
			OutputMsg( rmTip, _T("启动服务失败！") );
		}
		pLoggerSrv->Stop();
		SafeDelete(pLoggerSrv);

		return 0;
	}
};
#endif

int main(int argc, char** argv)
{
#ifdef WIN32
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
#endif

	int nRetCode = 0;
	InitDefMsgOut();

#ifdef WIN32
	CoInitialize(NULL);


	CCustomWorkSocket::InitSocketLib();

	_setmbcp(_MB_CP_SBCS);
	//★关键★ 设置"C"locale，视所有字符均作为单字节字符，以便同时支持UTF8以及MBCS
	InstallThreadLocalePath("C");
	//SetCurrentDirectory("../");
#endif
	//应用支持UTF-8的_output_l函数代码补丁
	/*
	int nErr = __install_utf8_output_patch__();
	if (nErr)
	{
		OutputError(GetLastError(), _T("error patching _output_l code"));
		return -1;
	}
	*/

#ifdef WIN32
      SetCurrentDirectory("../");
#else
	string filename(argv[1]);   
	size_t found = filename.find_last_of("/\\");
	filename = filename.substr(0, found);
	if( filename[0] == '.' && filename.length()==1 )
		filename = "./" ;
	SetCurrentDirectory(filename.c_str());
#endif
	// 创建 套接字服务类
	if (!FDOP::IsDirectory(_T("log")))
	{
		FDOP::DeepCreateDirectory(_T("log"));
	}
	CFileLogger flog(_T("./log/LoggerServer_%s.log"), getCurrentTimeDesc());

#ifdef WIN32
			
	//flog.SetNeedOutput(true);
#else
	if (argc >=2)
	{
		//flog.SetNeedOutput(true);
	}	
	else
	{
		//flog.SetNeedOutput(true);
	}
#endif

#ifndef _SERVICE
	//if (argc == 2 && _tcsncicmp("/cmd",argv[1],4)==0)//平时调试用
	{
		
		ServerMain(argc, argv);
	}
#else
	//else
	{
		ServiceMain(argc, argv);
	}
#endif

	CCustomWorkSocket::UnintSocketLib();
	CoUninitialize();
	UninitDefMsgOut();

#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif	

	return nRetCode;
}



VOID ServiceMain(int argc, char** argv)
{
#ifdef WIN32
	LPTSTR sCmd = NULL;
	if (argc >= 2)
	{
		sCmd = argv[1];
	}

	CLogService service("LogService");
	service.Run(sCmd);
#endif
}


VOID ServerMain(int argc, char** argv)
{
	char *pFileName =NULL;
	if(argc >=2)
	{
		pFileName = argv[1];
	}else
	{
		pFileName="LoggerServerLinux.txt";
	}
	GameServerLog::g_pLogServer = new CLogServer(pFileName);

	//CLoggerServerSocket* pLoggerSrv = new CLoggerServerSocket();
	//SetConsoleTitle( pLoggerSrv->Config.SrvConf.szServiceName );
	// 启动服务

	OutputMsg( rmTip, _T("正在启动日志服务……") );

	TCHAR cmd[512] = {0};

	if (GameServerLog::g_pLogServer->StartServer())
	{
		OutputMsg( rmTip, _T("启动日志服务成功!") );
#ifndef WIN32
		changemode(1);
#endif
		while(TRUE)
		{

			if(_kbhit2())
			{

				std::cin >> (cmd);
			}
			else
			{
				Sleep(100);
				continue;
			}	
			if (strcmp(cmd,_T("exit")) == 0 || strcmp(cmd, _T("quit")) == 0) 
			{
				break;		
			}
			Sleep(200);
		}
#ifndef WIN32
		changemode(0);
#endif
	}
	else	//启动服务失败
	{
		//OutputMsg( rmTip, _T("启动服务失败！") );
		printf ("Start fail");
		Sleep(3000);
	}
	//pLoggerSrv->Stop();
	//SafeDelete(pLoggerSrv);

	GameServerLog::g_pLogServer->StopServer();

	delete GameServerLog::g_pLogServer;

}
