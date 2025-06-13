#include "stdafx.h"
#ifdef WIN32
#include <dbghelp.h>
#include <mbctype.h>
#endif 
#include "utf8_output.h"
//#include "WinService.h"

#include "PathThreadLocale.h"
#include <string>
#include <vector>
using namespace std;


const char szExceptionDumpFile[] = ".\\BackServer.dmp";
VOID ServiceMain(int argc, char** argv);
VOID ServerMain(int argc, char** argv);

#define DBEGN_KN_VERION			MAKEFOURCC(17, 3, 24, 1)
#ifdef WIN32
class CLogService : public CWinService
{
public:
	CLogService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{
		// 创建 套接字服务类
		SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下
		CFileLogger flog(_T("BackServer%s.log"), getCurrentTimeDesc());
		
		CBackServerSocket* pLoggerSrv = new CBackServerSocket();
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
	CoInitialize(NULL);
	CCustomWorkSocket::InitSocketLib();

	_setmbcp(_MB_CP_SBCS);
#ifdef WIN32
	InstallThreadLocalePath("C");
#endif
#ifndef _SERVICE
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
	ServerMain(argc, argv);
#else
	ServiceMain(argc, argv);
#endif

	CCustomWorkSocket::UnintSocketLib();
	CoUninitialize();
	UninitDefMsgOut();

#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif	

	return nRetCode;
}


#ifdef WIN32
VOID ServiceMain(int argc, char** argv)
{
	LPTSTR sCmd = NULL;
	if (argc >= 2)
	{
		sCmd = argv[1];
	}
	CLogService service("BackService");
	service.Run(sCmd);
}
#endif

VOID ServerMain(int argc, char** argv)
{
	// 创建 套接字服务类
	if (!FDOP::IsDirectory(_T("log")))
	{
		FDOP::DeepCreateDirectory(_T("log"));
	}
	CFileLogger flog(_T("./log/BackStageServer_%s.log"), getCurrentTimeDesc());

	char *pFileName =NULL;
	if(argc >=2)
	{
		pFileName = argv[1];
	}else
	{
		pFileName="BackStageServerLinux.txt";
	}
	CBackServerSocket* pBackServer = new CBackServerSocket(pFileName);
	char *pBuff  = (char*)malloc(256);
	in_addr ia;
	ia.s_addr = DBEGN_KN_VERION;
	//strcpy(pBuff,_T("xhwl-后台"));

	strcpy( pBuff,pBackServer->Config.SrvConf.szServiceName );
	strcat(pBuff,"-V");
	strcat(pBuff,inet_ntoa(ia));
	SetConsoleTitle(pBuff);
	free(pBuff);

	// 启动服务

	OutputMsg( rmTip, _T("正在启动后台服务……") );

	
	if (pBackServer->Startup())
	{
		OutputMsg( rmTip, _T("启动后台服务成功!") );
		while(TRUE)
		{
			char cmd[10];
			scanf("%s",cmd);
			if (strcmp(cmd,_T("exit")) == 0 || strcmp(cmd, _T("quit")) == 0) 
			{
				break;		
			}
			else if(_tcsncicmp(cmd,"dmp",3) ==0 ) 
			{
				DebugBreak();
			}
			Sleep(1000);
		}
	}
	else	//启动服务失败
	{
		OutputMsg( rmTip, _T("启动服务失败！") );
	}
	pBackServer->Stop();
	SafeDelete(pBackServer);
}
