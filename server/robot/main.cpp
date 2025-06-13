
#include "stdafx.h"
//#include "WinService.h"
#include "FileLogger.h"



 CRobotManager* CRobotManager::s_pRobotManager =NULL;



#define ROBOT_KN_VERION   MAKEFOURCC(1, 11, 9, 20) //机器人的版本号

extern const TCHAR szExceptionDumpFile[] = _T("./Robot.dmp");
BOOL SetupSessionManagerConfig(CRobotManager*);
VOID ServerMain(int argc, char **argv);


int main(int argc, char** argv)
{
	//SetCurrentDirectory("../");		//CWinService里会把当前目录改成exe允许的目录，所以这里再转一下	
	CTimeProfMgr::getSingleton().InitMgr();  //性能统计
	//SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter ); //异常处理
	InitDefMsgOut(); 
  
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
	
	UninitDefMsgOut();
#ifdef	_MLIB_DUMP_MEMORY_LEAKS_
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

BOOL SetupRobotManagerConfig(CRobotManager *lpSSManager, const char * sConfigFile)
{
	CRobotConfig config;
	return config.loadConfig(lpSSManager, sConfigFile);
}



VOID ServerMain(int argc, char **argv)
{
	const char * sConfigFile = NULL;
	if (argc > 0)
	{
		//sConfigFile = argv[1];
		//sConfigFile = _T("data/robot/RobotConfig.txt");//argv[1];
	}
	//主线程的代码
	INT_PTR nError;
	TCHAR sCmdBuf[512];
	//CRobotManager *pSSManager;
	CFileLogger flog(_T("./log/Robot_%s.log.txt"), getCurrentTimeDesc());
	//CFileLogger flog(_T("Robot.log.html"));
	
	//pSSManager = new CRobotManager();
	if(CRobotManager::s_pRobotManager !=NULL ) return;
	CRobotManager::s_pRobotManager = new(CRobotManager);
	
	winitseed((unsigned int)_getTickCount()); //初始化随机数函数
	OutputMsg( rmTip, _T("Starting Robot, Waiting.."));
	if ( SetupRobotManagerConfig(CRobotManager::s_pRobotManager, sConfigFile) )
	{
		char* sTitleName = CRobotManager::s_pRobotManager->m_sConsoleName;
		//GetRobotMgr()->GetGlobalConfig().convertToACP(CRobotManager::s_pRobotManager->m_sConsoleName, sizeof(CRobotManager::s_pRobotManager->m_sConsoleName), sTitleName, ArrayCount(sTitleName));
		sprintf_s(sTitleName, "%s-%d", sTitleName, GetCurrentProcessId());
		SetConsoleTitle(sTitleName);	
		if ( CRobotManager::s_pRobotManager->Startup() )
		{			
			in_addr ia;
			OutputMsg( rmTip, _T("-------------------------------------------") );
			OutputMsg( rmTip, _T("Robot 启动成功，核心版本号是%s"), inet_ntoa(ia) );
			OutputMsg( rmTip, _T("quit - 停止测试并退出程序") );
			OutputMsg( rmTip, _T("count -查看当前的在线情况") );
			OutputMsg( rmTip, _T("dmp -输出dmp信息") );
			OutputMsg( rmTip, _T("-------------------------------------------") );
			
			while ( TRUE )
			{
				_getts(sCmdBuf);
				//滤掉末尾的换行符
				nError = (int)strlen(sCmdBuf);
				while ( nError > 0 && sCmdBuf[nError-1] < 0x20 )
				{
					nError--;
					sCmdBuf[nError] = 0;
				}
				
				//退出命令
				if ( _tcsncicmp(sCmdBuf, _T("\\q"), 2) == 0 
					|| _tcsncicmp(sCmdBuf, _T("exit"), 4) == 0
					|| _tcsncicmp(sCmdBuf, _T("quit"), 4) == 0 )
				{
					OutputMsg( rmTip, _T("正在停止Robot...") );
					break;
				}
				else if ( _tcsncicmp(sCmdBuf, _T("count"), 5) == 0 )
				{
					CRobotManager::s_pRobotManager->ShowServerAgentCount();
				}
				else if(_tcsncicmp(sCmdBuf, "dmp", 3) == 0)
				{
					DebugBreak();
				}
				Sleep(10);
			}

			CRobotManager::s_pRobotManager->Shutdown();
			//Sleep(3000); //等所有的线程都退出
		}
		else
		{
			OutputMsg(rmError, _T("启动失败"));
			char c;
			scanf(&c,"%c");
		}
	}
	else
	{
		OutputMsg(rmError, _T("读取配置文件失败"));
		char c;
		scanf(&c,"%c");
	}

	delete CRobotManager::s_pRobotManager;
	CRobotManager::s_pRobotManager=NULL;
}

