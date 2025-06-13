#pragma once

/*******************************************************/
/*
/*				逻辑服务器服务配置数据读取类
/*
/*         实现对逻辑服务器运行相关服务设置的读取功能
/*
/*******************************************************/

class CLogicServer;

struct CROSSSERVERCFG
{
	int nPFid;				//渠道id
	int nServerId;			//区服
	char sLocalNamePR[20];	//跨服别名
	// char sClientNet[100];	//跨服转发地址
	// char nCrossIP[20];		//跨服服务器ip
	// int nPort;				//端口号
	int nDestination;		//目的地
    int nPointRange[8];		//传送范围，四个坐标点
	CROSSSERVERCFG()
	{
		memset(this, 0, sizeof(*this));
	}
};
struct CSSourceInfo
{
	int nkfid;				//编号
	int nServerId;			//区服
	char sLocalNamePR[20];	//跨服别名
	int nDestination;		//目的地
    int nPointRange[8];   	//传送范围，四个坐标点
	
	char cCrossIP[20];		//跨服服务器ip
	int nCrossPort;			//端口号

	//网关信息
	char cGateAddr[100];	//跨服转发地址
	int nGatePort;			//端口号
	//开启时间
	int nOpenDayBegin;
	int nOpenDayEnd;

	CSSourceInfo()
	{
		memset(this, 0, sizeof(*this));
	}
};

class CLogicServerConfig :
	public CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig Inherited;
	static const TCHAR ConfigFileName[];	//定义配置文件名称

public:
	CLogicServerConfig();
	~CLogicServerConfig();

	//加载并读取服务器服务配置
	bool loadServerConfig(CLogicServer *lpLogicServer, const char *pszFileName = ConfigFileName);
	//bool LoadCrossServerConfig(CLogicServer *lpLogicServer,bool bReload = true);

	//读取跨服配置总表
	bool LoadLocalCSConfigByCross(CLogicServer *lpLogicServer, bool bReload = false);	//跨服
	bool LoadLocalCSConfigBySource(CLogicServer *lpLogicServer, bool bReload = false);	//原服
	
	//
	bool LoadNewCrossClientConfig(CLogicServer *lpLogicServer, bool bReload = false);
	bool LoadNewCrossServerConfig(CLogicServer *lpLogicServer, bool bReload = false);

	//跨服服务器的配置
	CROSSSERVERCFG* getCSConfigCrossById(int nCSSrvid)
	{
		if(m_mCSConfigCross.find(nCSSrvid) != m_mCSConfigCross.end())
			return &m_mCSConfigCross[nCSSrvid];
		return NULL;
	}
	//原服配置
	CSSourceInfo* getCSConfigSourceById(int index)
	{
		if(m_mCSConfigSource.find(index) != m_mCSConfigSource.end())
			return &m_mCSConfigSource[index];
		return NULL;
	}
	//原服配置 
	CSSourceInfo* getCSConfigSourceByOpenday(int tDaytime);//根据当前时间读取配置
protected:
	void showError(LPCTSTR sError);
	bool throwLoadConfig(CLogicServer *lpLogicServer, const char *pszFileName);
	//读取服务器服务配置
	bool readServerConfig(CLogicServer *lpLogicServer);
	

//跨服相关配置
public:
	std::map<int, CROSSSERVERCFG> m_mCSConfigCross; //跨服列表//跨服读取 
	std::map<int, CSSourceInfo> m_mCSConfigSource; //跨服列表//原服读取 

};