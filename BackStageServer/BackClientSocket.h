#ifndef	_LOGGER_CLIENT_SOCKET_H_
#define	_LOGGER_CLIENT_SOCKET_H_

class CCustomJXServerClientSocket;
class CBackServerSocket;
//class CNetworkDataHandler;

//发送命令出去
#define SEND_COMMAND	1		//发送操作码给逻辑服 
#define GET_NOTICE_MGR  2		//请求公告

//收到命令
#define REV_LOGICSERVERINFO 1		//收到逻辑服的信息
#define SEND_COMMAND_RESULT 2 //收到服务器发送来的关于命令是否执行的包！
#define SEND_NOTICE_MGR 3			//收到公告
#define SEND_LOGICSPID 4			//收到逻辑服spid


class CBackClientSocket:
	public CCustomJXServerClientSocket
{

public:
	const static int CREATE_LOCALTABLE_INTERVAL = 86400000;  //1天调用一次

	CBackClientSocket(void);
	CBackClientSocket(CBackServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	~CBackClientSocket(void);

	//继承基类的方法，处理收到的数据包
	//virtual VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
		/*** 子类需覆盖的函数集 ***/
	/* 处理单个通信数据包 
	 * nCmd		通信消息命令
	 * inPacket	已经读取出通信命令的数据包，数据包的读取位置指向命令数据后的内容
	 */
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	/* 验证客户端注册数据是否有效
	 *@return 返回true表示注册数据有效并，返回false则关闭连接
	 */
	virtual bool OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData);

	//获取服务器id
	int GetnServerIndex() {return m_nServerIndex;}

	//发送命令给逻辑服 
	//nServerIndex 服务器id
	// nOperIndex 操作码
	// sCommand   操作命令
	void SendCommandToLogic(int nServerIndex,int nOperIndex,char* sCommand,char* sName,int nId);

	//请求公告信息
	void GetNoticeMsg();

	//收到逻辑服的信息
	void RevLogicInfo(CDataPacketReader &inPacket);

	//收到刷命令的结果
	void RevCommandResult(CDataPacketReader &inPacket);

	//插入公告
	void UpdateNotice(CDataPacketReader &inPacket);

	int GetSpid();
public:
	static int  g_nSpid;

private:
	/*
	* 处理逻辑包
	* pDataBuff:为真正需要处理的数据包，这里只处理日志类型的数据包，每次处理一个数据包;nLen:数据的长度
	* 返回：实际处理的字节数，事实上pDataBuff的大小会比数据包大,返回0表示出错没处理
	*/
	int ProcessData(void* pDataBuff,int nLen);

protected:
	virtual VOID OnRun();
	VOID OnDisconnected();
private:
	//把数据包传递给这个类保存到数据库
	CBackServerSocket*			m_pBackServer;
	CSQLConenction*				m_pSQLConnection;		//数据库连接对象
	TICKCOUNT					m_nextCreateTtableTick;            //上一次的调用的tickCount
	CBaseList<CDataPacket* >    m_dataList;             //待处理的数据列表
	CBaseList<CDataPacket* >    m_freeDataList;          //空闲的数据列表
	int							m_nServerIndex;
	int							m_nSpid;
};

#endif