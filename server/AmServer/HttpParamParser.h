#pragma once
/*
HTTP参数分析类 
*/
class CAMServerSocket;

typedef struct tagHttpParam
{
	char sName[64];  //参数名称
	char sValue[256]; //参数值
	tagHttpParam()
	{
		memset(this, 0, sizeof(*this));
	}
}HTTPPARAM;

const int MAX_PARAM_NUM = 20;

class HttpParamParser
{
public:
	HttpParamParser(void)
	{
		memset(m_GameName,0,sizeof(m_GameName));
		memset(m_Spid,0,sizeof(m_Spid));
		memset(m_Interface,0,sizeof(m_Interface));
		InitializeCriticalSection( &m_ParseLock );
	};
	~HttpParamParser(void){DeleteCriticalSection( &m_ParseLock );};
public:
	bool DocumentParser(char * sDocument);
	unsigned int GetParamUIntValue(char* sName);
	int GetParamIntValue(char* sName);
	const char* GetParamCharValue(char* sName);
	double GetParamDoubleValue(char* sName);

	void CreateHttpServer(char* sAddr,int nPort);
	char* HttpCommandGet();
	void CloseHttpConnet();
	void SendResponse(LPCSTR sContent);
	void SendResponse(SOCKET nsocket, LPCSTR sContent);

	char m_GameName[20];	//游戏英文缩写
	char m_Spid[20];		//运营商英文缩写
	char m_Interface[20];   //接口名称
	inline char * GetClientIP() { return m_ClientIP;}
	inline SOCKET GetServerSocket() { return m_ServerSocket; }
	inline SOCKET GetClientSocket() { return m_ClientSocket; }
private:
	HTTPPARAM Params[MAX_PARAM_NUM];   //参数
	SOCKET m_ServerSocket;
	SOCKET m_ClientSocket;
	fd_set m_fdSocket;
	SOCKADDR_IN m_AddrClient;
	SOCKADDR_IN m_AddrServer;
	RTL_CRITICAL_SECTION	m_ParseLock;
	char*		m_ClientIP;

};

