// AMServer.cpp : Defines the entry point for the console application.
//
#include <iostream>  
#include <fstream>  
#include "stdafx.h"
#include "utf8_output.h"
#ifdef WIN32
#include <dbghelp.h>
#include <mbctype.h>
#include <conio.h>
#endif
#include "PathThreadLocale.h"
#include <string>
#include <vector>
#include <signal.h>
#include <iostream>
using namespace std;

#define BUFSIZE 1024

extern const TCHAR szExceptionDumpFile[] = _T("./AMServer.dmp");

#define DBEGN_KN_VERION	 MAKEFOURCC(17, 3, 28, 1)

bool AMEngineRunning = true ;
bool bAMEngineStartSucceed = true;

#ifndef WIN32

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


static void signal_handler(int sig_num)
{
        if( sig_num == SIGHUP ||
            sig_num == SIGINT ||
            sig_num == SIGTERM )
        {
		OutputMsg( rmTip, _T("正在退出...") );
                AMEngineRunning = false;
        }
}

#endif

void ServerCreate(int argc, char ** argv)
{
#ifdef WIN32
	SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
#else
	(void)signal(SIGHUP, SIG_IGN);
        (void)signal(SIGINT, signal_handler);
        (void)signal(SIGTERM, signal_handler);	
#endif
	InitDefMsgOut();
#ifdef WIN32
	CCustomWorkSocket::InitSocketLib();
	_setmbcp(_MB_CP_SBCS);
	//★关键★ 设置"C"locale，视所有字符均作为单字节字符，以便同时支持UTF8以及MBCS
	InstallThreadLocalePath("C");
	SetCurrentDirectory("./");
#else
	string filename(argv[0]);   
	size_t found = filename.find_last_of("/\\");
	filename = filename.substr(0, found);
	if( filename[0] == '.' && filename.length()==1 )
		filename = "./" ;
	SetCurrentDirectory(filename.c_str());
#endif
	char *pBuff  = (char*)malloc(256);
	in_addr ia;
	ia.s_addr = DBEGN_KN_VERION;
	strcpy(pBuff,_T("AM充值服务"));
	strcat(pBuff,"-V");
	strcat(pBuff,inet_ntoa(ia));
	SetConsoleTitle(pBuff);
	free(pBuff);
	//OutputMsg( rmTip, _T("AM充值服务-V%s"), inet_ntoa(ia1) );
}

void ServerDestroy()
{
#ifdef WIN32
	CCustomWorkSocket::UnintSocketLib();
#endif
	UninitDefMsgOut();
}

int main(int argc, char** argv)
{
	ServerCreate( argc, argv );
	if (!FDOP::IsDirectory(_T("log")))
	{
		FDOP::DeepCreateDirectory(_T("log"));
	}
	CFileLogger flog(_T("./log/AMServer_%s.log.txt"), getCurrentTimeDesc());
	
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

	char *pFileName =NULL;
	if(argc >=2)
	{
		pFileName = argv[1];
	}else
	{
		pFileName="AMServerLinux.txt";
	}

	OutputMsg( rmTip, _T("正在启动AM充值服务……") );

	CAMServerSocket* pAMSrv = new CAMServerSocket(pFileName);
	if( ! bAMEngineStartSucceed )
	{
		OutputMsg( rmError, _T("启动AM充值服务失败……") );
		return -1 ;
	}

	if (pAMSrv->Startup())
	{
		OutputMsg( rmTip, _T("启动AM充值服务成功!") );
		char cmd[100];
#ifndef WIN32
		changemode(1);
#endif
		while( AMEngineRunning )
		{
			if(_kbhit())
			{
				std::cin >> (cmd);
			}
			else
			{
				Sleep(500);
				continue;
			}	
			
			//_getts(cmd);
			
			if (strcmp(cmd,_T("exit")) == 0 || strcmp(cmd, _T("quit")) == 0) 
			{
				OutputMsg( rmTip, _T("正在退出...") );
				break;
			}
			else if( strcmp(cmd,_T("contractid")) == 0 )
			{
				pAMSrv->OutPutContractIdMap();
			}
		}
#ifndef WIN32
		changemode(0);
#endif
	}
	else	//启动服务失败
	{
		OutputMsg( rmError, _T("启动AM充值服务失败！") );
		Sleep(3000);
	}
	pAMSrv->Stop();
	SafeDelete(pAMSrv);
	ServerDestroy();
	return 0;
}
