#ifndef	_GATE_SERVER_CONFIG_H_
#define	_GATE_SERVER_CONFIG_H_

/*
 * 配置文件的读写
 *
 */
typedef struct tagGateConfig
{
	struct
	{
		char	sName[40];		//自身的名字
		CHAR	sAddress[40];	//网关服务绑定地址
		int     nPort;			//端口
		int		nMaxSession;	//网关中客户端连接池的最大连接数
		int		SendThreadCount;//数据发送线程池中工作线程数量
	}GateServer;				//网关服务配置
	struct 
	{
		CHAR	sHost[120];		//连接服务器的地址，支持域名
		int     nPort;			//连接服务器的端口
	}BackServer;				//后台服务器配置
}GATECONFIG, *PGATECONFIG;

typedef CBaseList<GATECONFIG>	CGateConfigList;

//配置文件的解析类
class CGateServerConfig:
	public CCustomLuaConfig
{
protected:
	void showError(LPCTSTR sError);
	bool readConfig(CGateConfigList &configList,const char *pszFileName);
public:
	bool loadConfig(CGateConfigList &configList, char *pszFileName=NULL);
	
};



#endif
