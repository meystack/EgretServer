#ifndef			_SRV_CONFIG_H_
#define			_SRV_CONFIG_H_


//服务器端口地址等相关配置
typedef	struct SrvConf
{
	char	szAddr[20];
	int		nPort;
	char	szServiceName[20];
} SRVCONF,*PSRVCONF;

typedef	struct DbConf{
	char	szHost[100];
	int		nPort;
	char	szDbName[100];
	char	szUser[100];
	char	szPassWord[100];
	char	szKey[100];//用于解密的key，16位
	bool    m_bUtf8;

}DBCONF,*PDBCONF;

class CSrvConfig:
	public CCustomLuaConfig
{
public:
	/*
	* 只读取同目录下的指定配置文件
	*/
	void ReadConfig(const char *pszFileName);
public:
	CSrvConfig(void);
	~CSrvConfig(void);
public:
	SRVCONF		SrvConf;
	DBCONF		DbConf;

	int			nSpid;
	int			nServerIndex;
protected:
	void ShowError(const LPCTSTR sError);
};

#endif
