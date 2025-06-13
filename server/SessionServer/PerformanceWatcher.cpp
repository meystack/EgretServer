/*
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <Lock.h>
#include <Tick.h>
#include <QueueList.h>
#include <CustomSocket.h>
#include <NamedPipe.h>
#include <ClassedWnd.h>
#include <commctrl.h>
#include <time.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "SockModel/CustomWorkSocket.h"
#include "SockModel/CustomClientSocket.h"
#include "SockModel/CustomServerClientSocket.h"
#include "SockModel/CustomServerSocket.h"
#include "gate/GateProto.h"
#include "gate/CustomServerGateUser.h"
#include "gate/CustomServerGate.h"
#include "gate/CustomGateManager.h"
#include "ServerDef.h"
#include "SQL.h"
#include "SSProto.h"
#include "SSGateUser.h"
#include "SSGate.h"
#include "SessionClient.h"
#include "SessionServer.h"
#include "SSGateManager.h"
#include "SSManager.h"
*/

#include "StdAfx.h"


/*
#include "PerformanceWatcher.h"
#pragma comment(lib, "comctl32")

class CPerformanceWnd : public wylib::window::CClassedWnd
{
public:
	typedef CClassedWnd Inherited;
	static const TCHAR WndClassName[];

private:
	HFONT	m_hEditFont;
	HWND	m_hEdit;
	BOOL	m_boShowing;

private:
	VOID RegistWndClass()
	{
		WNDCLASS wndclass;
		HINSTANCE hInstance = GetModuleHandle(NULL);
		BOOL boClassRegisted = GetClassInfo(hInstance, WndClassName, &wndclass);

		if ( !boClassRegisted || wndclass.lpfnWndProc != getClassedWndProc() )
		{
			if ( boClassRegisted )
			{
				UnregisterClass(WndClassName, hInstance);
			}
			//int       nCmdShow;
			//此窗口对象的一些属性设置
			wndclass.style = CS_HREDRAW | CS_VREDRAW;
			wndclass.lpfnWndProc = getClassedWndProc();
			wndclass.cbClsExtra = 0 ;
			wndclass.cbWndExtra = 0 ;
			wndclass.hInstance = hInstance;
			wndclass.hIcon = NULL;//LoadIcon(NULL, IDI_ASTERISK);  //IDI_APPLICATION 普通应用程序图标, IDI_HAND 打叉红色图标
			// IDI_QUESTION 问号图标, IDI_EXCLAMATION 感叹号图标 IDI_ASTERISK 信息图标 
			wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
			wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			wndclass.lpszMenuName = NULL;
			wndclass.lpszClassName = WndClassName;

			//用上边声明的窗口对象注册此窗口
			if (!RegisterClass(&wndclass))
			{
				MessageBox(NULL, TEXT("This program requires Windows NT!"),
					WndClassName, MB_ICONERROR);
				return;
			}
		}
	}
	VOID CreatePerformanceWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		//注册完了就创建此窗口咯
		HWND hwnd = MyCreateWindowEx(0, 
			WndClassName,	//window class name
			_T("会话状态"),	//window caption
			WS_CAPTION | WS_BORDER | WS_SYSMENU,	//window style
			CW_USEDEFAULT,	//initial x position
			CW_USEDEFAULT,	//initial y position
			520,120,
			//CW_USEDEFAULT,	//initial x size
			//CW_USEDEFAULT,	//initial x size
			NULL,	//parent window handle
			NULL,	//window menu handle
			hInstance,	//program instance handle
			NULL);	//creation parameters
	}
protected:
	LRESULT WndProc(HWND hwnd,UINT message,WPARAM wParam, LPARAM lParam)
	{
		switch (message) 
		{ 
		case WM_CREATE: 
			m_hEdit = CreateWindow( 
				_T("EDIT"),     // predefined class 
				NULL,       // no window title 
				WS_CHILD | WS_VISIBLE |
				ES_CENTER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 
				0, 0, 0, 0, // set size in WM_SIZE message 
				hwnd,       // parent window 
				(HMENU) 101, // edit control ID 
				NULL, 
				NULL);                // pointer not needed 
			SendMessage(m_hEdit, EM_SETREADONLY, TRUE, 0);//输入框只读

			m_hEditFont = CreateFont(12, 0, 0, 0, 100, FALSE, FALSE, 0, 
				GB2312_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
				DEFAULT_PITCH | FF_MODERN, _T("宋体") );
			SendMessage(m_hEdit, WM_SETFONT ,(WPARAM)m_hEditFont, 0);//WM_SETFONT
			return 0; 

		case WM_SETFOCUS: 
			SetFocus(m_hEdit); 
			return 0; 

		case WM_SIZE: 
			MoveWindow(m_hEdit, 
				0, 0,           // starting x- and y-coordinates 
				LOWORD(lParam), // width of client area 
				HIWORD(lParam), // height of client area 
				TRUE);          // repaint window 
			return 0; 

		case WM_CLOSE:
			m_boShowing = FALSE;
			ShowWindow(hwnd, SW_HIDE);
			return 0;

		case WM_DESTROY:
			DeleteObject(m_hEditFont);
			return 0;
		} 
		return DefWindowProc(hwnd, message, wParam, lParam);  
	} 
public:
	CPerformanceWnd()
		:Inherited()
	{
		m_hEditFont = 0;
		m_hEdit = 0;
		m_boShowing = FALSE;
		
		RegistWndClass();
		CreatePerformanceWindow();
	}
	~CPerformanceWnd()
	{
		if ( getHwnd() )
		{
			if ( m_hEdit )
			{
				DestroyWindow(m_hEdit);
				m_hEdit = NULL;
			}
			DestroyWindow(getHwnd());
		}
	}
	VOID SetEditText(LPCTSTR sText)
	{
		SetWindowText(m_hEdit, " ");
		SendMessage(m_hEdit, EM_REPLACESEL ,0, (LPARAM)sText);//WM_SETFONT
	}
	VOID Show(INT nShow)
	{
		HWND hWnd = getHwnd();
		m_boShowing = nShow;
		ShowWindow(hWnd, nShow);
		if (nShow)
			UpdateWindow(hWnd);
	}
	inline BOOL showing(){ return m_boShowing; }
};

class CServerConnectionWnd : public wylib::window::CClassedWnd
{
public:
	typedef CClassedWnd Inherited;
	static const TCHAR WndClassName[];
private:
	HWND	m_hListView;
	HFONT	m_hFont;
	BOOL	m_boShowing;
	BOOL	m_boWantUpdate;
private:
	VOID RegistWndClass()
	{
		WNDCLASS wndclass;
		HINSTANCE hInstance = GetModuleHandle(NULL);
		BOOL boClassRegisted = GetClassInfo(hInstance, WndClassName, &wndclass);

		if ( !boClassRegisted || wndclass.lpfnWndProc != getClassedWndProc() )
		{
			if ( boClassRegisted )
			{
				UnregisterClass(WndClassName, hInstance);
			}
			//int       nCmdShow;
			//此窗口对象的一些属性设置
			wndclass.style = CS_HREDRAW | CS_VREDRAW;
			wndclass.lpfnWndProc = getClassedWndProc();
			wndclass.cbClsExtra = 0 ;
			wndclass.cbWndExtra = 0 ;
			wndclass.hInstance = hInstance;
			wndclass.hIcon = NULL;//LoadIcon(NULL, IDI_ASTERISK);  //IDI_APPLICATION 普通应用程序图标, IDI_HAND 打叉红色图标
			// IDI_QUESTION 问号图标, IDI_EXCLAMATION 感叹号图标 IDI_ASTERISK 信息图标 
			wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
			wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			wndclass.lpszMenuName = NULL;
			wndclass.lpszClassName = WndClassName;

			//用上边声明的窗口对象注册此窗口
			if (!RegisterClass(&wndclass))
			{
				MessageBox(NULL, TEXT("This program requires Windows NT!"),
					WndClassName, MB_ICONERROR);
				return;
			}
		}
	}
	VOID CreateDisplayConnectWndWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		//注册完了就创建此窗口咯
		HWND hwnd = MyCreateWindowEx(0, 
			WndClassName,	//window class name
			_T("已连接服务器"),	//window caption
			WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,	//window style
			CW_USEDEFAULT,	//initial x position
			CW_USEDEFAULT,	//initial y position
			520,320,
			//CW_USEDEFAULT,	//initial x size
			//CW_USEDEFAULT,	//initial x size
			NULL,	//parent window handle
			NULL,	//window menu handle
			hInstance,	//program instance handle
			NULL);	//creation parameters
	}
	VOID CreateWndFont()
	{
		m_hFont = CreateFont(12, 0, 0, 0, 100, FALSE, FALSE, 0, 
			GB2312_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
			DEFAULT_PITCH | FF_MODERN, _T("宋体") );
	}
	VOID CreateListView (HWND hWndParent)
	{
		RECT rcl;
		// 确保相关通用控件DLL已导入
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC  = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icex);

		GetClientRect(hWndParent, &rcl); // 获取父窗口客户区域大小

		// 创建客户端更新状态列表控件m_hEdit
		m_hListView = CreateWindowEx(0UL, WC_LISTVIEW, "",
			WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT | LVS_EX_GRIDLINES | LVS_SINGLESEL, 
			0, 0, rcl.right, (rcl.bottom - rcl.top)*3/5, hWndParent, (HMENU)1, 
			GetModuleHandle(NULL), NULL); // 在WM_SIZE消息中处理大小, 让其适应窗口大小

		// 设置列表控件的扩展样式
		ListView_SetExtendedListViewStyleEx(m_hListView, 0, 
			LVS_EX_FLATSB | LVS_EX_FULLROWSELECT |  LVS_EX_GRIDLINES  | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE  );

		//设置字体
		SendMessage(m_hListView, WM_SETFONT ,(WPARAM)m_hFont, 0);


		// 创建列
		char szText[50] = "";
		LV_COLUMN lvC;
		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
		lvC.fmt = LVCFMT_LEFT;
		lvC.pszText = szText;

		char *pszClumnText1[] = { "服务器名称", "连接IP", "连接端口","连接时间"};	
		int ColumnWidths[] = { 200, 100, 60, 120 };

		for (int i = 0; i < sizeof(pszClumnText1)/sizeof(char*); ++i)
		{
			lvC.cx = 120;
			lvC.iSubItem = i; 
			wsprintf(szText, "%s", pszClumnText1[i]);
			lvC.cx = 200;

			if (ListView_InsertColumn(m_hListView, i, &lvC) == -1)
			{
				MessageBox(NULL, "函数ListView_InsertColumn(m_hEdit, i, &lvC)错误!", "", 0);
			}
			ListView_SetColumnWidth(m_hListView, i, ColumnWidths[i]);
		}
	}
protected:
	LRESULT WndProc(HWND hwnd,UINT message,WPARAM wParam, LPARAM lParam)
	{
		switch (message) 
		{ 
		case WM_CREATE: 
			CreateWndFont();
			CreateListView (hwnd);
			return 0; 

		case WM_PAINT:
			{
				PAINTSTRUCT pt;
				RECT rt;
				GetClientRect(hwnd, &rt);
				BeginPaint(hwnd, &pt);
				HGDIOBJ hObj = SelectObject(pt.hdc, (HGDIOBJ)m_hFont);
				TextOut(pt.hdc, (rt.right - rt.left - 120)/2, (rt.bottom - 30), _T("点击此处刷新列表"), 16);
				SelectObject(pt.hdc, hObj);
				EndPaint(hwnd, &pt);
			}
			return 0;

		case WM_SETFOCUS: 
			SetFocus(m_hListView); 
			return 0; 

		case WM_SIZE: 
			MoveWindow(m_hListView, 0, 0, LOWORD(lParam), HIWORD(lParam) - 40, TRUE);
			return 0;

		case WM_CLOSE:
			m_boShowing = FALSE;
			ShowWindow(hwnd, SW_HIDE);
			return 0;

		case WM_DESTROY:
			return 0;

		case WM_LBUTTONUP:
			m_boWantUpdate = TRUE;
			return 0;
		} 
		return DefWindowProc(hwnd, message, wParam, lParam);  
	}
public:
	CServerConnectionWnd():Inherited()
	{
		m_hListView = 0;
		m_hFont = 0;
		m_boShowing = FALSE;
		m_boWantUpdate = FALSE;
		InitCommonControls();
		RegistWndClass();
		CreateDisplayConnectWndWindow();
	}
	~CServerConnectionWnd()
	{
		if ( getHwnd() )
		{
			DestroyWindow(getHwnd());
		}
	}
	VOID Show(INT nShow)
	{
		HWND hWnd = getHwnd();
		m_boShowing = nShow;
		ShowWindow(hWnd, nShow);
		if (nShow)
			UpdateWindow(hWnd);
	}
	VOID DisplayServerConnections(CPerformanceWatcher::CServerConnectionList &ServerList)
	{
		ListView_DeleteAllItems(m_hListView);

		LV_ITEM Item;
		CPerformanceWatcher::ServerConnetionInfo *pSrvInfo = ServerList;
		INT_PTR nCount = ServerList.count();
		CHAR sBuffer[256];
		TICKCOUNT dwCurTick = _getTickCount();

		ZeroMemory(&Item, sizeof(Item));
		for (INT_PTR i=0; i<nCount; ++i)
		{
			Item.iItem = (int)i;
			sprintf(sBuffer,"%s",pSrvInfo->sSrvName);
			ListView_InsertItem(m_hListView, &Item);

			ListView_SetItemText(m_hListView, i, 0, sBuffer);

			strcpy(sBuffer, inet_ntoa(pSrvInfo->s_Addr.sin_addr));
			ListView_SetItemText(m_hListView, i, 1, sBuffer);

			sprintf(sBuffer, "%d", htons(pSrvInfo->s_Addr.sin_port));
			ListView_SetItemText(m_hListView, i, 2, sBuffer);

			sprintf(sBuffer, "%d秒", (dwCurTick - pSrvInfo->dwConnectTick) / 1000 );
			ListView_SetItemText(m_hListView, i, 3, sBuffer);

			pSrvInfo++;
		}
		m_boWantUpdate = FALSE;
	}
	inline BOOL showing(){ return m_boShowing; }
	inline BOOL getWantUpdate(){ return m_boWantUpdate; };//刷新按钮状态
};

const TCHAR CPerformanceWnd::WndClassName[] = TEXT("SSMgrStateClass");
const TCHAR CServerConnectionWnd::WndClassName[] = TEXT("SSMgrSrvConnClass");

CPerformanceWatcher::CPerformanceWatcher(CSSManager *lpSSManager)
	:Inherited(TRUE)
{
	m_pSSManager = lpSSManager;
}


CPerformanceWatcher::~CPerformanceWatcher(void)
{
}


VOID CPerformanceWatcher::OnRountine()
{	
	MSG msg;
	TICKCOUNT dwNextShowTick = 0, dwTick;

	//生成消息队列
	
	PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE);

	m_pPerformanceWnd = new CPerformanceWnd();
	m_pServerConnectionWnd = new CServerConnectionWnd();

	while ( !terminated() )
	{
		dwTick = _getTickCount();
		if ( dwTick >= dwNextShowTick )
		{
			dwNextShowTick = dwTick + 1000;
			if (m_pPerformanceWnd->showing())
				ShowPerformance();
		}

		if (m_pServerConnectionWnd->getWantUpdate())
		{
			ShowServerConnections();
		}
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(16);
	}

	SafeDelete(m_pPerformanceWnd);
	SafeDelete(m_pServerConnectionWnd);
	

}
*/

/*
VOID CPerformanceWatcher::ShowPerformance()
{


	PERFORMANCEINFO m2001p1;
	PERFORMANCEINFO m104p1, m104p3, m104p4;
	
	int count = 0;
	char buf[4*1024];

	count += _stprintf(buf + count, "会话数量:%d\r\n",  
		m_pSSManager->getSessionServer()->GetSessionCount());

	count += _stprintf(buf + count,"数据服务器:%3d  引擎服务器:%3d\r\n",  
		m_pSSManager->getSessionServer()->GetDBClientCount(), 
		m_pSSManager->getSessionServer()->GetLogicClientCount(0) );

	m_pSSManager->getGateManager()->getMainLoopPerformance( &MainLoop );
	m_pSSManager->getGateManager()->getMainProcessPerformance( &ProcPerformance );
	m_pSSManager->getGateManager()->getMainSleepPerformance( &SleepPerformance );
	m_pSSManager->getGateManager()->getInternalMessageProcessPerformance( &IntMsgProcPerformance );
	m_pSSManager->getGateManager()->getExecuteGatesPerformance( &ExecGatesPerformance );
	count += _stprintf(buf + count,"网关管理器性能(LP:%d/%d/%d  MP:%d/%d/%d IMP:%d/%d/%d EG:%d/%d/%d)\r\n", 
		(UINT)MainLoop.nLastLoop, (UINT)MainLoop.nMinLoop, (UINT)MainLoop.nMaxLoop, 
		(UINT)ProcPerformance.dwLastTick, (UINT)ProcPerformance.dwMinTick, (UINT)ProcPerformance.dwMaxTick,
		(UINT)SleepPerformance.dwLastTick, (UINT)SleepPerformance.dwMinTick, (UINT)SleepPerformance.dwMaxTick,
		(UINT)IntMsgProcPerformance.dwLastTick, (UINT)IntMsgProcPerformance.dwMinTick, (UINT)IntMsgProcPerformance.dwMaxTick,
		(UINT)ExecGatesPerformance.dwLastTick, (UINT)ExecGatesPerformance.dwMinTick, (UINT)ExecGatesPerformance.dwMaxTick
		);

	//CSSGate *pGate = (CSSGate*)m_pSSManager->getGateManager()->getGate(0);
	pGate->getReadPerformance( &gRead );
	pGate->getProcessReadPerformance( &gProcessRead );
	pGate->getSendPerformance( &gSend );
	pGate->getProcessUserPerformance( &gProcessUser );
	pGate->getProcessUserMsgPerformance( &gProcUserMsg );

	count += _stprintf(buf + count,"网关[0]性能(RC:%d/%d/%d  PRC:%d/%d/%d SD:%d/%d/%d PU:%d/%d/%d PUM:%d/%d/%d[%d])\r\n", 
		(UINT)gRead.dwLastTick, (UINT)gRead.dwMinTick, (UINT)gRead.dwMaxTick,
		(UINT)gProcessRead.dwLastTick, (UINT)gProcessRead.dwMinTick, (UINT)gProcessRead.dwMaxTick,
		(UINT)gSend.dwLastTick, (UINT)gSend.dwMinTick, (UINT)gSend.dwMaxTick,
		(UINT)gProcessUser.dwLastTick, (UINT)gProcessUser.dwMinTick, (UINT)gProcessUser.dwMaxTick,
		(UINT)gProcUserMsg.dwLastTick, (UINT)gProcUserMsg.dwMinTick, (UINT)gProcUserMsg.dwMaxTick, pGate->getMaxTimeUserMsgIdent()
		);

	//pGate->getLoadUserLoginDataPerformance( &m2001p1 );
	
	count += _stprintf(buf + count,"M2001性能(LULD:%d/%d/%d)\r\n", 
		(UINT)m2001p1.dwLastTick, (UINT)m2001p1.dwMinTick, (UINT)m2001p1.dwMaxTick
		);

	pGate->getCheckServerReadyPerformance(&m104p1);
	pGate->getPostOpenSessionPerformance(&m104p3);
	pGate->getUpdateUserLoginPerformance(&m104p4);
	count += _stprintf(buf + count, "M104性能(CSR:%d/%d/%d POS:%d/%d/%d UUL:%d/%d/%d)\r\n", 
		(UINT)m104p1.dwLastTick, (UINT)m104p1.dwMinTick, (UINT)m104p1.dwMaxTick,
		(UINT)m104p3.dwLastTick, (UINT)m104p3.dwMinTick, (UINT)m104p3.dwMaxTick,
		(UINT)m104p4.dwLastTick, (UINT)m104p4.dwMinTick, (UINT)m104p4.dwMaxTick
		);
	

	//m_pPerformanceWnd->SetEditText(buf);
}
*/

/*
UINT STDCALL CPerformanceWatcher::EnumServerConnectionCallBack(CServerConnectionList *pServerList, CSessionClient *lpConnection)
{
	ServerConnetionInfo SrvInfo;

	_asncpytA(SrvInfo.sSrvName, lpConnection->getClientName());
	SrvInfo.s_Addr = lpConnection->GetRemoteAddrIn();
	SrvInfo.dwConnectTick = lpConnection->getConnectTick();

	pServerList->add(SrvInfo);
	return 0;
}

VOID CPerformanceWatcher::ShowServerConnections()
{
	CServerConnectionList ServerList;
	m_pSSManager->getSessionServer()->EnumConnections((CSessionServer::EnumConnectionFn)EnumServerConnectionCallBack, &ServerList);
	m_pServerConnectionWnd->DisplayServerConnections(ServerList);
}

VOID CPerformanceWatcher::ShowPerformanceWindow(const BOOL boShow)
{
	if ( m_pPerformanceWnd )
	{
		ShowPerformance();
		m_pPerformanceWnd->Show(boShow ? SW_SHOW : SW_HIDE);
	}
}

VOID CPerformanceWatcher::ShowServerConnectionWindow(const BOOL boShow)
{
	if ( m_pServerConnectionWnd )
	{
		ShowServerConnections();
		m_pServerConnectionWnd->Show(boShow ? SW_SHOW : SW_HIDE);
	}
}
*/
