#pragma once

#define BUFSIZE 2048
#define MAX_PARAM_NUM 20
#define MAXPARAMNAMELENGTH 32
#define MAXPARAMVALUELENGTH 2048

typedef struct tagHttpParam
{
	char sName[MAXPARAMNAMELENGTH];  //参数名称
	char sValue[MAXPARAMVALUELENGTH]; //参数值
	tagHttpParam()
	{
		memset(this, 0, sizeof(*this));
	}
}HTTPPARAM;
typedef struct tagSocketInfo
{
	SOCKET socket;
	int	   id;
	int    spid;
	int	   serverindex;
	char   data[BUFSIZE];
	tagSocketInfo ()
	{
		memset(this,0,sizeof(*this));
	}
}SocketInfo;


class CBackHttpServer
{
public:
	CBackHttpServer(void);
	~CBackHttpServer(void);

	void ClearParams()
	{
		memset(m_Params,0,sizeof(m_Params));
	}
public:
	//************************************
	// Method:    DoHttpServerCreate 创建http服务器
	// FullName:  CBackHttpServer::DoHttpServerCreate
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: char * sAddr  绑定地址
	// Parameter: int nPort     绑定端口
	//************************************
	void DoHttpServerCreate(char* sAddr,int nPort);

	//************************************
	// Method:    HttpGetData 获取serversocket接收到的数据
	// FullName:  CBackHttpServer::HttpGetData
	// Access:    public 
	// Returns:   void
	// Qualifier:
	//************************************
	void HttpGetData();
	//************************************
	// Method:    DocumentParser 解析http信息
	// FullName:  CBackHttpServer::DocumentParser
	// Access:    public 
	// Returns:   bool
	// Qualifier:
	// Parameter: char * sDocument 数据
	//************************************
	bool DocumentParser(char * sDocument);
	//************************************
	// Method:    SendResponse 返回server信息给php
	// FullName:  CBackHttpServer::SendResponse
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: LPCSTR sContent 结果字符串
	// Parameter: int nId			后台指令id
	// Parameter: int nSpid			spid
	// Parameter: int nServerindex	服务器id
	//************************************
	void SendResponse(LPCSTR sResult,int nId =0, int nSpid = 0, int nServerindex = 0);
	void SendResponse(SOCKET socket, LPCSTR sResult);
	//************************************
	// Method:    GetParamCharValue 获取字符串参数值
	// FullName:  CBackHttpServer::GetParamCharValue
	// Access:    public 
	// Returns:   LPCTSTR
	// Qualifier:
	// Parameter: char * sName 参数名
	//************************************
	LPCTSTR GetParamCharValue(char* sName);
	//************************************
	// Method:    GetParamIntValue 获取整型参数值
	// FullName:  CBackHttpServer::GetParamIntValue
	// Access:    public 
	// Returns:   int
	// Qualifier:
	// Parameter: char * sName 参数名
	//************************************
	int GetParamIntValue(char* sName);
	//************************************
	// Method:    GetClientSocket 获取客户端socket
	// FullName:  CBackHttpServer::GetClientSocket
	// Access:    public 
	// Returns:   SOCKET 
	// Qualifier:
	//************************************
	SOCKET GetClientSocket();

	void AddToSocketInfo(SocketInfo& sInfo,int nId, int nSpid, int nServerIndex);
private:
	BOOL CreateSocketInfo(SOCKET   s) ;
	void RemoveSocketInfo(int index);
	void RemoveSocketInfoBySocket(SOCKET s);
private:
	SOCKET					m_ServerSocket;			//httpServerSocket
	SOCKET					m_ClientSocket;			//httpClientSocket
	fd_set					m_fdSocket;				//socket 元素
	SOCKADDR_IN				m_AddrClient;			//client地址
	SOCKADDR_IN				m_AddrServer;			//server地址
	RTL_CRITICAL_SECTION	m_HttpServerLock;		//httpServer锁
	char*					m_ClientIP;				//连接clientip
	HTTPPARAM				m_Params[MAX_PARAM_NUM];   //参数
public:
	SocketInfo				m_SocketInfoArr[FD_SETSIZE];
	int						m_fdSetCount;
};
