#ifndef			_SRV_CONFIG_H_
#define			_SRV_CONFIG_H_

////服务器端口地址等相关配置
//typedef	struct tagServerConf
//{
//	char	sAddr[20];
//	int		nPort;
//	tagServerConf()
//	{
//		memset(this, 0, sizeof(*this));
//	}
//} SERVERCONF,*PSERVERCONF;
//
//typedef	struct tagProductConf{
//	char	sName[20];
//	char	sSpid[20];
//	char	sHost[20];
//	int		nPort;
//	char	sDBName[100];
//	char	sDBUser[100];
//	char	sDBPass[100];
//	char	sPayKey[100];
//	LPCSTR	sAuthorizationIP;
//	tagProductConf()
//	{
//		memset(this, 0, sizeof(*this));
//	}
//}PRODUCTCONF,*PPRODUCTCONF;

const static int MAX_HOST_COUNT =32;   ///允许的ip列表，最多32个
const static int MAX_TAST_COUNT = 4;

//平台的信息
struct PlatForm
{
	char spid[8];		//平台的spid
	char code[8];		//游戏的充值代码
	char db[32];		//数据库名字
	char bianma[12];   //连接mysql编码
	char host[128];     //数据库的ip
	char user[128];		//加密用户名
	char pass[128];		//加密密码
	char paykey[128];   //充值key
	char awdkey[128];   //红利key
	long long hosts[MAX_HOST_COUNT];  //允许的ip列表，最多32个
	int  codeid;		//游戏的代码
	int  port;			//数据库的端口
	int  hostcount;     //实际ip的数目
	int taskcount;      //任务个数
	char constractid[MAX_TAST_COUNT][128];   //任务集市id
	PlatForm()
	{
		memset(this,0,sizeof(*this)); //可以清0的
	}
};

class CSrvConfig:
	public CCustomLuaConfig
{
public:
	CSrvConfig(void)
	{
		//memset(m_ServiceName,0,sizeof(m_ServiceName));
	};
	~CSrvConfig(void);
public:
	bool ReadConfig(const char *pszFileName);
	inline INT_PTR GetPlatFormCount() { return m_platforms.count();}
	PlatForm * GetPlatForm(int nIndex);
	 
	//char	m_ServiceName[20];
	//SERVERCONF m_HttpServerConf;
	//SERVERCONF m_AmountServerConf;
	int            m_nHttpport;			//绑定的http回调的端口
	int            m_nAmport;			//连接地址
	char			m_sServerName[128];	//服务器名称
protected:
	void ShowError(const LPCTSTR sError);
private:
	//CBaseList<PRODUCTCONF> m_ProductList;
	CBaseList<PlatForm> m_platforms;    //平台的列表，初始化的时候使用
};

#endif
