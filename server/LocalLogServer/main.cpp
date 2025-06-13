#include "StdAfx.h"

#ifdef WIN32
#include <dbghelp.h>
#include <mbctype.h>
#else
// #include "client/linux/handler/exception_handler.h"
#endif

#include "utf8_output.h"
//#include "WinService.h"
#include "PathThreadLocale.h"

#include <string>

const char szExceptionDumpFile[] = ".\\LocalLogServer.dmp";
VOID ServiceMain(int argc, char** argv);
VOID ServerMain(int argc, char** argv);

#define DBEGN_KN_VERION			MAKEFOURCC(17, 3, 24, 1)

#ifndef WIN32
// static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
// 	void* context,
// 	bool succeeded)
// {
// 	printf("Dump path: %s\n", descriptor.path());
// 	return succeeded;
// }
#endif


#ifdef WIN32
class CLogService : public CWinService
{
public:
	CLogService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
	int AppMain()
	{
		// 创建 套接字服务类
		SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下
		CFileLogger flog(_T("LocalLogServer_%s.log"), getCurrentTimeDesc());
		
		CLocalLogServerSocket* pLoggerSrv = new CLocalLogServerSocket();
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
#else
	//seh_init();
	// google_breakpad::MinidumpDescriptor descriptor(".");
	// google_breakpad::ExceptionHandler eh(descriptor,
	// 	NULL,
	// 	dumpCallback,
	// 	NULL,
	// 	true,
	// 	-1);
#endif
	InitDefMsgOut();
#ifdef WIN32
	CoInitialize(NULL);
#endif

	CCustomWorkSocket::InitSocketLib();

#ifdef WIN32
	_setmbcp(_MB_CP_SBCS);
	//★关键★ 设置"C"locale，视所有字符均作为单字节字符，以便同时支持UTF8以及MBCS
	InstallThreadLocalePath("C");
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
			
                
		ServerMain(argc,argv);
	}
#else
	//else
	{
		ServiceMain(argc, argv);
	}
#endif
	CCustomWorkSocket::UnintSocketLib();

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
VOID ServiceMain(int argc, char** argv)
{
	LPTSTR sCmd = NULL;
	if (argc >= 2)
	{
		sCmd = argv[1];
	}
	CLogService service("LocalLogService");
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
	CFileLogger flog(_T("./log/LocalLogServer_%s.log"), getCurrentTimeDesc());

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

	char *pFileName =NULL;
	if(argc >=2)
	{
		pFileName = argv[1];
	}else
	{
		pFileName="LocallogServerLinux.txt";
	}
	CLocalLogServerSocket* pLoggerSrv = new CLocalLogServerSocket(pFileName);

	char *pBuff  = (char*)malloc(256);
	in_addr ia;
	ia.s_addr = DBEGN_KN_VERION;
	strcpy(pBuff,"lcwl-");

	//SetConsoleTitle( pLoggerSrv->Config.SrvConf.szServiceName );
	strcat(pBuff, pLoggerSrv->Config.SrvConf.szServiceName);
	strcat(pBuff,"-V");
	strcat(pBuff,inet_ntoa(ia));
#ifdef WIN32
	SetConsoleTitle(pBuff);
#else
	printf( "%c]0;%s%c", '\033', pBuff, '\007');
#endif
	free(pBuff);

	// 启动服务

	OutputMsg( rmTip, _T("正在启动本地日志服务……") );

	if (pLoggerSrv->Startup())
	{
		OutputMsg( rmTip, _T("启动本地日志服务成功!") );
		printf("LocalLogServer Start Success...\n");
		while(TRUE)
		{
			char cmd[10] = {0};
			scanf("%s",cmd);
			if (_tcsncicmp(cmd,_T("exit"),4) == 0 || _tcsncicmp(cmd, _T("quit"),4) == 0) 
			{
				OutputMsg( rmTip, _T("正在退出本地日志服务器...") );
				break;		
			}
			else if(_tcsncicmp(cmd,"opentips",8) ==0) //终端显示输出
			{
				//flog.SetNeedOutput(true);
			}
			else if(_tcsncicmp(cmd,"closetips",9) ==0) //关闭终端显示输出
			{
				//flog.SetNeedOutput(false);
			}

			Sleep(1000);
		}
	}
	else	//启动服务失败
	{
		printf ("Start fail" );
		Sleep(3000);
		
	}

	pLoggerSrv->Stop();
	SafeDelete(pLoggerSrv);
	printf("LocalLogServer Stop Success...\n");
}
