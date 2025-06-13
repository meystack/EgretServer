#pragma once

class CCustomServerSocket;
class CSQLConenction;
class CSrvConfig;
class CAMClientSocket;
class CProduct;

using namespace wylib::sync::lock;
using namespace wylib::container;

//typedef std::map<UINT, string > CONTRACT_ID_MAP;

class CAMServerSocket:
	public CCustomServerSocket
{
public:
	typedef CCustomServerSocket ServerInherited ;

	CAMServerSocket(char *pszFileName);
	~CAMServerSocket(void);

	CProduct* GetProduct(char* gName,char* spid);
	HttpParamParser& GetParamParser(){ return Parser; };
	void OutPutContractIdMap();
	void RefreshProduct();
	void OutPutAllClientSocket();
	char* GetServerName();
protected:
	virtual CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	virtual VOID DestroyClientSocket(CCustomServerClientSocket *pClientSocket);
	VOID SingleRun();
	VOID HandleHttpCommand(char *sDocument);
private:
	bool CheckAuthorizationIP(long long * ipList, int size, char * ip );
	CAMClientSocket* GetSpidClientPro(char* spid);

	CQueueList<CAMClientSocket*>	m_AMClientList;		//客户端列表
	CBaseList<CProduct*>			m_Products;         //产品列表
	CSrvConfig						Config;				//配置文件
	HttpParamParser					Parser;				//HTTPServer
	string 							m_ConfigPath;
	//CONTRACT_ID_MAP           m_ContractIdMap;
};

