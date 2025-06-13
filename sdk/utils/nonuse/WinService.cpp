/*/////////////////////////////////////////////////////////////////////////

文件名: WinService.cpp

功能描述: Windows 服务程序基本框架类实现文件. 从此类继承并实现指定的自定义
          功能函数就可以将程序作为服务程序运行.

创建人: 谭建冲

创建时间: 2010年06月04日

/////////////////////////////////////////////////////////////////////////*/

#ifdef WIN32

#include "WinService.h"
#include <string.h>
#include <TCHAR.h>

// 全局服务对象地址,用于静态成员函数访问内部成员
static CWinService *g_pWinService = NULL;


CWinService::CWinService(LPCTSTR pServiceName, DWORD dwServiceType)
{
	g_pWinService = this; //保存此对象地址(注:全局只能存在一个此对象)

	_tcscpy(m_szServiceName, pServiceName);
	m_dwServiceType = dwServiceType;
	m_boServiceExit = FALSE;

	memset(&m_ServiceStatus, 0, sizeof(m_ServiceStatus));
	m_ServiceStatus.dwServiceType				= dwServiceType;
	m_ServiceStatus.dwCurrentState				= SERVICE_STOPPED;
	m_ServiceStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	m_ServiceStatus.dwWin32ExitCode				= 0;
	m_ServiceStatus.dwServiceSpecificExitCode	= 0;
	m_ServiceStatus.dwCheckPoint				= 0;
	m_ServiceStatus.dwWaitHint					= 0;

	// 设置 "当前目录" 为程序文件的目录(负责系统会默认为system32目录)
	TCHAR AppPath[MAX_PATH + 1] = {0};
	GetModuleFileName(NULL, AppPath, sizeof(AppPath));
	signed __int64 len = _tcslen(AppPath)-1;
	while (len > 0 && AppPath[len] != '\\')
	{
		len--;
	}
	AppPath[len] = 0;
	SetCurrentDirectory(AppPath);
}

CWinService::~CWinService(void)
{
}

int CWinService::SetStatus( DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
{
	m_dwServiceStatus = dwState;
	m_ServiceStatus.dwCurrentState				= m_dwServiceStatus;
	m_ServiceStatus.dwWin32ExitCode				= dwExitCode;
	m_ServiceStatus.dwServiceSpecificExitCode	= 0;
	m_ServiceStatus.dwCheckPoint				= dwProgress;
	m_ServiceStatus.dwWaitHint					= 0;
	return SetServiceStatus(m_hServiceStatus, &m_ServiceStatus);
}

BOOL CWinService::ServiceInstalled()
{
	BOOL bResult = FALSE;
	SERVICE_STATUS svcState;

	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM != NULL)
	{
		SC_HANDLE hService = OpenService(hSCM, m_szServiceName, SERVICE_ALL_ACCESS );
		if (hService != NULL)
		{
			if ( QueryServiceStatus(hService, &svcState) )
			{
				if ( svcState.dwCurrentState == SERVICE_RUNNING )
				{
					bResult = TRUE;
				}
				else if ( svcState.dwCurrentState == SERVICE_STOPPED )
				{
					bResult = ::StartService(hService, 0, NULL);
					if ( !bResult )
					{
						if (!DeleteService(hService))
							bResult = FALSE;
					}
				}
			}
			CloseServiceHandle(hService);
		}
		CloseServiceHandle(hSCM);
	}
	return bResult;
}

BOOL CWinService::InstallService()
{
	if (ServiceInstalled())
		return TRUE;

	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		return FALSE;
	}

	TCHAR sPath[MAX_PATH];
	GetModuleFileName(NULL, sPath, sizeof(sPath));
	SC_HANDLE hService = CreateService(	hSCM, 
		m_szServiceName, 
		m_szServiceName,
		SERVICE_ALL_ACCESS,
		m_dwServiceType,
		SERVICE_AUTO_START, 
		SERVICE_ERROR_NORMAL,
		sPath,
		NULL,
		NULL,
		_T(""), 
		NULL, 
		NULL);
	if (hService == NULL)
	{
		CloseServiceHandle(hSCM);
		return FALSE;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return TRUE;
}

BOOL CWinService::StartService()
{
	BOOL bRet = FALSE;
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM != NULL)
	{
		SC_HANDLE hService = OpenService(hSCM, m_szServiceName, SERVICE_ALL_ACCESS );
		if (hService != NULL)
		{
			SERVICE_STATUS svcState;
			if ( QueryServiceStatus(hService, &svcState) )
			{
				if (svcState.dwCurrentState == SERVICE_RUNNING )
				{
					bRet = TRUE;
				}
				else
				{
					if (::StartService(hService, 0, NULL))
					{
						bRet = TRUE;
					}
				}
			}
		}
		CloseServiceHandle(hService);
	}

	CloseServiceHandle(hSCM);
	return bRet;
}

BOOL CWinService::StopService()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM != NULL)
	{
		SC_HANDLE hService = OpenService(hSCM, m_szServiceName, SERVICE_ALL_ACCESS );

		if (hService != NULL)
		{
			SERVICE_STATUS svcState;
			if ( QueryServiceStatus(hService, &svcState) )
			{
				if ( svcState.dwCurrentState == SERVICE_RUNNING )
				{
					if (!ControlService(hService, SERVICE_CONTROL_STOP, &svcState))
					{
						return FALSE;
					}
				}
			}
		}

		CloseServiceHandle(hService);
	}

	CloseServiceHandle(hSCM);
	return TRUE;
}

BOOL CWinService::UninstallService()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM != NULL)
	{
		SC_HANDLE hService = OpenService(hSCM, m_szServiceName, SERVICE_ALL_ACCESS );

		if (hService != NULL)
		{
			SERVICE_STATUS svcState;
			if ( QueryServiceStatus(hService, &svcState) )
			{
				if ( svcState.dwCurrentState == SERVICE_RUNNING )
				{
					if (!ControlService(hService, SERVICE_CONTROL_STOP, &svcState))
					{
						return FALSE;
					}
				}
			}

			if (!DeleteService(hService))
			{
				return FALSE;
			}
		}

		CloseServiceHandle(hService);
	}

	CloseServiceHandle(hSCM);
	return TRUE;
}

VOID WINAPI CWinService::ServiceControl(DWORD dwOption)
{
	switch (dwOption)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		g_pWinService->SetStatus( SERVICE_STOP_PENDING, 0, 1);
		g_pWinService->m_boServiceExit = TRUE;
		return;
	case SERVICE_CONTROL_PAUSE:
		return;
	case SERVICE_CONTROL_CONTINUE:
		return;
	case SERVICE_CONTROL_INTERROGATE:
		return;
	default:
		break;
	}

	SetServiceStatus(g_pWinService->m_hServiceStatus, &g_pWinService->m_ServiceStatus);
}

VOID WINAPI CWinService::ServiceMain(DWORD dwArgc, LPSTR *lpszArgv)
{
	HANDLE	hThread = NULL;
	//注册服务控制
	g_pWinService->m_hServiceStatus = RegisterServiceCtrlHandler(g_pWinService->m_szServiceName, &CWinService::ServiceControl);
	if (g_pWinService->m_hServiceStatus == NULL)
	{
		g_pWinService->SetStatus(SERVICE_STOPPED, -1, 0);
		return;
	}
	g_pWinService->SetStatus(SERVICE_START_PENDING, 0, 1);
	g_pWinService->SetStatus(SERVICE_RUNNING, 0, 0);


	//=======================================================================
	// 服务程序功能开始运作
	//=======================================================================
	g_pWinService->AppMain();
	//=======================================================================
	// 服务程序功能运作结束
	//=======================================================================

	// 服务停止
	g_pWinService->SetStatus(SERVICE_STOPPED, 0, 0);
}

VOID CWinService::Run(LPTSTR lpCmdLine)
{
    // 防止strstr函数引起的程序崩溃
    if ( NULL == lpCmdLine )
    {
        lpCmdLine = _T("");
    }

	if (_tcsstr(lpCmdLine, _T("/install")) != NULL )
	{
		InstallService();
	}
	else if (_tcsstr(lpCmdLine, _T("/start")) != NULL)
	{
		StartService();
	}
	else if (_tcsstr(lpCmdLine, _T("/stop")) != NULL)
	{
		StopService();
	}
	else if (_tcsstr(lpCmdLine, _T("/uninstall")) != NULL)
	{
		UninstallService();
	}
	else 
	{
		SERVICE_TABLE_ENTRY	ste[2];
		ste[0].lpServiceName = m_szServiceName;
		ste[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
		memset(&ste[1], 0, sizeof(ste[1]));
		if (!StartServiceCtrlDispatcher(ste))
		{
			while ( !m_boServiceExit )
			{
				Sleep( 10000 );
			}
		}
	}
}


#endif

