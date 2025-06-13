#ifndef _DB_CONFIG_H_
#define _DB_CONFIG_H_

class CCustomLuaConfig;


typedef	struct AddrPortConf
{
	char	szAddr[20];
	int		nPort;
} ADDRPORTCONF,*PADDRPORTCONF;

typedef	struct DbConf{
	char	szHost[20];
	int		nPort;
	char	szDbName[20];
	char	szUser[20];
	char	szPassWord[128];
	char	szKey[17];//用于解密的key，16位
	int    boUtf8;
}DBCONF,*PDBCONF;

class CDBConfig:
	public CCustomLuaConfig
{
public:
	/*
	* 只读取同目录下的指定配置文件
	*/
	bool ReadConfig(const char *pszConfigFileName);

	//装置跨服配置文件
	bool LoadCrossServerConfig();


public:
	CDBConfig(void);
	~CDBConfig(void);
public:
	char			ServerName[128];
	int				ServerIndex;
	ADDRPORTCONF	GateAddr;
	ADDRPORTCONF	DataAddr;
	ADDRPORTCONF	LogAddr;
	ADDRPORTCONF	SessionAddr;
	ADDRPORTCONF	NameAddr;
	ADDRPORTCONF	DBCenterAddr;
	DBCONF			DbConf;

	CHAR			m_EsqlToolPath[128];	//esqltool的配置路径

	CHAR			m_sPrefixList[32];
	//CHAR			m_sConnectList[32];
	CHAR			m_sStuffixList[32];
protected:
	void ShowError(const LPCTSTR sError);
};

#endif

