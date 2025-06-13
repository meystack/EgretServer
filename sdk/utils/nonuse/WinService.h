/*/////////////////////////////////////////////////////////////////////////

文件名: WinService.h 

功能描述: Windows 服务程序基本框架类定义头文件. 从此类继承并实现指定的自定义
          功能函数就可以将程序作为服务程序运行.

创建人: 谭建冲

创建时间: 2010年06月04日

// 用法示例:

#include "WinService.h"

// 首先继承并实现功能主函数
class CMyAppService : public CWinService
{
public:
    CMyAppService(LPCSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS):CWinService(pServiceName, dwServiceType){};
    int AppMain()
    {
        // TODO: 在这里添加自定义代码

        // 示例:
        while ( !m_boServiceExit )
        {
            Sleep(1000);
        }
        return 0;
    }
};

// 其次是创建对象后运行即可.
void main(int argc, char *argv[])
{
    CMyAppService MyAppService("testServiceName");
    MyAppService.Run(argv[1]);
}

// 或者

int	CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                       LPSTR lpCmdLine, int nShowCmd)
{
    CMyAppService MyAppService("testServiceName");
    MyAppService.Run(lpCmdLine);
    return 0;
}

/////////////////////////////////////////////////////////////////////////*/

#pragma once

#ifdef WIN32

#include <Windows.h>

class CWinService
{
public:
	CWinService(LPCTSTR pServiceName, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS);
	virtual ~CWinService(void);

public:
	BOOL InstallService();     // 安装服务程序到系统中
	BOOL StartService();       // 启动服务程序
	BOOL StopService();        // 停止服务程序
	BOOL UninstallService();   // 卸载服务程序
	VOID Run(LPTSTR lpCmdLine); // 执行服务程序 参数 lpCmdLine 表示系统传入的参数(用于安装,卸载服务等操作)
	virtual int AppMain() = 0; // 程序的主要功能函数, 子类必须实现此函数.
	static VOID WINAPI ServiceControl(DWORD dwOption);
	static VOID WINAPI ServiceMain(DWORD dwArgc, LPSTR *lpszArgv);

protected:
	int SetStatus( DWORD dwState, DWORD dwExitCode, DWORD dwProgress);
	BOOL ServiceInstalled();

public:
	TCHAR					m_szServiceName[128]; 
	BOOL					m_boServiceExit; //服务程序是否退出的开关
	DWORD					m_dwServiceType;      
	SERVICE_STATUS			m_ServiceStatus;
	DWORD					m_dwServiceStatus;
	SERVICE_STATUS_HANDLE   m_hServiceStatus;
};

#endif
