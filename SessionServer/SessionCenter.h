#ifndef			_CENTER_SENDER_H_
#define			_CENTER_SENDER_H_

//跨平台连接会话中心服

//与会话中心服消息定义
typedef enum eCommonSessionMsgCmd
{
	cSendSpid = 50,					//发送spgid给会话中心服
	cRequestTranPlatform = 2,		//请求跨平台

	sRecvRequsestPlatform = 1,		//收到返回请求跨平台消息
};

//class CNetworkDataHandler;
class CCustomJXClientSocket;
class CSSManager;

class CSessionCenter:
	public CCustomJXClientSocket
{
public:
	CSessionCenter();
	CSessionCenter(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName,CSSManager *lpSSManager);
	~CSessionCenter();


	VOID SetServerName(LPCTSTR sServerName);

	//连接会话中心服
	void OnSendConnectCommonPlatform(LPCTSTR sIp,int nPort);

	//发送给会话中心判断是否可以跨服
	void OnRequesTranCommonPlatform(CDataPacketReader &inPacket);

	//收到回话中心服返回是否可以跨服的消息
	void OnRecvRequsetCommonPlat(CDataPacketReader &inPacket);

	//跨平台登陆检查用户密码
	void CheckPlatformUserPassword(CDataPacketReader &reader);

	int CheckPlatformUserValid(PCHAR name, PCHAR passwd,GLOBALSESSIONOPENDATA & sessiDonata,PCHAR md5,PCHAR sCurrentTime,bool checkMd5,bool bCommonSrvLogin,PCHAR iIndentity,bool &isWhiteIpLogin);
	

	//向指定ID的逻辑户端发送消息，消息将被立刻追加到客户端发的发送缓冲中。数据包必须是完成的格式（包含包头和包尾）
	int SendSSCommonMsg(const int nServerIndex, LPCSTR sMsg, const size_t dwSize);

	//请求返回原来逻辑服
	void OnReqestRawServer(CDataPacketReader &reader);

protected:
	/*** 子类需覆盖的函数集 ***/
	/* 处理单个通信数据包 
	 * nCmd		通信消息命令
	 * inPacket	已经读取出通信命令的数据包，数据包的读取位置指向命令数据后的内容
	 */
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);

	/*** 下列函数为子类可能有必要覆盖的函数集 ***/
	/* ★查询本地服务器的类型，以便正确的发送注册数据 */
	virtual jxSrvDef::SERVERTYPE getLocalServerType();

	/* ★查询本地服务器的名称，以便正确的发送注册数据 */
	virtual LPCSTR getLocalServerName();

	/* ★查询本地服务器ID，以便正确的发送注册数据，默认的返回值是0 */
	virtual int getLocalServerIndex();

	//提供向最外层逻辑通知连接建立的事件
	virtual VOID OnConnected();
private:
	jxSrvDef::SERVERTYPE			ServerType;		//服务器类型，值为SERVERTYPE枚举类型中的枚举值
	INT			ServerIndex;	//服务器唯一编号（服务器编号仅对游戏服务器有意义）
	CHAR		ServerName[128];	//服务器名称，需包含0终止符

	char		sCommonPaltformIp[64];		//连接中心会话服的ip
	int			nCommonPort;				//连接中心会话服的端口

	CSSManager*					m_pSManager;		//所属会话管理器

	CSQLConenction*			m_pSqlConect;	//数据库连接程序
};

#endif