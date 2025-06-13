#ifndef	_LOGGER_CLIENT_SOCKET_H_
#define	_LOGGER_CLIENT_SOCKET_H_

class CCustomJXServerClientSocket;
class CLoggerServerSocket;
//class CNetworkDataHandler;

class CLoggerClientSocket:
	public CCustomJXServerClientSocket
{

public:
	CLoggerClientSocket(void);
	CLoggerClientSocket(CLoggerServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	~CLoggerClientSocket(void);

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

	int GetServerIndex() {return m_nServerIndex;}

	//发送聊天记录给日志查看器
	void SendChatLogToClient(CDataPacketReader &inPacket);
private:
	/*
	* 处理逻辑包
	* pDataBuff:为真正需要处理的数据包，这里只处理日志类型的数据包，每次处理一个数据包;nLen:数据的长度
	* 返回：实际处理的字节数，事实上pDataBuff的大小会比数据包大,返回0表示出错没处理
	*/
	int ProcessData(void* pDataBuff,int nLen);

	/*****以下四个函数分别保存4大类的日志内容*****/	
	VOID InsertLogin(CDataPacketReader &inPacket);
	VOID InsertConsume(CDataPacketReader &inPacket);
	VOID InsertOnline(CDataPacketReader &inPacket);
	VOID InsertKillDrop(CDataPacketReader &inPacket);
	VOID InsertJoinAtv(CDataPacketReader &inPacket);
	VOID InsertDeal(CDataPacketReader &inPacket);
	VOID InsertItem(CDataPacketReader &inPacket);
	//VOID InsertSuggest(CDataPacketReader &inPacket); //插入建议数据

protected:
	virtual VOID OnRun();
	
	VOID OnDisconnected();
private:
	//把数据包传递给这个类保存到数据库
	CLoggerServerSocket*		pSrv;
	CSQLConenction*				m_pSQLConnection;		//数据库连接对象

	CBaseList<CDataPacket* >    m_dataList;             //待处理的数据列表
	
	CBaseList<CDataPacket* >    m_freeDataList;          //空闲的数据列表
	int							m_nServerIndex;
#ifndef WIN32
	CLocalSender*        m_pLocalSender;
#endif
};

#endif
