#ifndef _CROSSSSERVER_MANAGER_H_
#define _CROSSSSERVER_MANAGER_H_
/*****************************************************************
            Copyright (c) 2021, 上海漫方网络科技有限公司
                    All rights reserved
       
    创建日期：  2021年04月28日 14时01分
    文件名称：  CrossServer.h
    说    明：  跨服引擎管理类
    
    当前版本：  1.00
    作    者：  
    概    述： 	跨服引擎管理类

*****************************************************************/
class CSSGateManager;
class LogSender;

class CCrossServerManager
{
public:
	CCrossServerManager();
	~CCrossServerManager();

	inline LPCSTR getServerName(){ return m_sServerName; };
	inline CCrossServer* getCrossServer(){ return m_pCrossServer; };
	//
	inline VOID PostCrossServerMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
	{
		return m_pCrossServer->PostInternalMessage(uMsg, uParam1, uParam2, uParam3, uParam4);
	}

	//设置服务器名称
	VOID SetServerName(LPCSTR sSrvName);

	//设置跨服服务器地址和端口
	VOID SetCrossServiceAddress(LPCTSTR sHost, const int nPort);

	BOOL Startup();
	VOID Shutdown();
	
private:
	CHAR						m_sServerName[128];	//服务器名称
	CCrossServer*             	m_pCrossServer;//
};

#endif

