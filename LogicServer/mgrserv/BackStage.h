#ifndef			_BACKSTAGE_SENDER_H_
#define			_BACKSTAGE_SENDER_H_

//class CNetworkDataHandler;
class CCustomJXClientSocket;

//收到命令
#define SEND_COMMAND	1		//发送操作码给逻辑服
#define GET_NOTICE_MGR  2		//请求公告

//发送
#define REV_LOGICSERVERINFO 1		//收到逻辑服的信息
#define SEND_COMMAND_RESULT 2		//返回命令执行的结果
#define SEND_NOTICE_MGR 3			//发送公告给后台
#define SEND_LOGICSPID 4			//发送逻辑服spid


class CBackStageSender:
	public CCustomJXClientSocket
{
public:
	

	inline INT GetServerIndex(){return ServerIndex;}
	inline jxSrvDef::SERVERTYPE GetServerType(){return ServerType;}
	inline LPCSTR GetServerName() {return ServerName;}

	VOID SetServerIndex(const INT nSerIdx){ ServerIndex = nSerIdx;}
	VOID SetServerType(const jxSrvDef::SERVERTYPE nSrvType){ ServerType = nSrvType;}
	VOID SetServerName(LPCTSTR sServerName);

	//收到命令
	void RevBackCommandOp(CDataPacketReader &inPacket);

	//处理命令
	void HandleCommand(INT_PTR nCmd, UINT_PTR data,UINT_PTR sName,UINT_PTR nId);

	char* GetStrDest(char* srcStr,char* strDest);
	
	//发送命令
	void SendCommandResult(char* strCmmand,int nCmd,int nResult,char* sName,int nId);

	//发送spid
	void SendSpidToBack();
	void SendServerState();
	//解析参数字符串
	static unsigned int ParseArgStr(char **args, char *pArgStr, int nMaxArgCnt);
	//获取参数值
	int GetArgValueByInt(char* args);
	/// CMiniDateTime
	static void StdTimeStr2MiniTime(const char* sStdTime, CMiniDateTime& miniTime);
	
	virtual void OnRountine()
	{
		CCustomJXClientSocket::OnRountine();
	}

public:
	CBackStageSender();
	CBackStageSender(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName);
	~CBackStageSender(void);

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
	//CNetworkDataHandler DataHandler;//用于解网络包

	//以下4个值用于发送注册包
	jxSrvDef::SERVERTYPE			ServerType;		//服务器类型，值为SERVERTYPE枚举类型中的枚举值
	INT			ServerIndex;	//服务器唯一编号（服务器编号仅对游戏服务器有意义）
	CHAR		ServerName[128];	//服务器名称，需包含0终止符
	CMiniDateTime			m_tomorrowDateTime;			// 指示明天的开始时间，也就是今天的结束时间，用于24点的刷新处理（活动、任务等逻辑）
};

#endif
