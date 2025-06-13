#ifndef	_LOGGER_CLIENT_SOCKET_H_
#define	_LOGGER_CLIENT_SOCKET_H_

class CCustomJXServerClientSocket;
class CLocalLogServerSocket;
//class CNetworkDataHandler;

class CLocalLogClientSocket:
	public CCustomJXServerClientSocket
{

public:
	const static int CREATE_LOCALTABLE_INTERVAL = 86400000;  //1天调用一次

	CLocalLogClientSocket(void);
	CLocalLogClientSocket(CLocalLogServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	~CLocalLogClientSocket(void);

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

public:
	static int  g_nSpid;

private:
	/*
	* 处理逻辑包
	* pDataBuff:为真正需要处理的数据包，这里只处理日志类型的数据包，每次处理一个数据包;nLen:数据的长度
	* 返回：实际处理的字节数，事实上pDataBuff的大小会比数据包大,返回0表示出错没处理
	*/
	int ProcessData(void* pDataBuff,int nLen);

	/*****以下四个函数分别保存4大类的日志内容*****/	
	VOID InsertCommon(CDataPacketReader &inPacket);
	VOID CreateCommonTable(CDataPacketReader &inPacket);

	//定时删除3个月前的表
	VOID DeleteCommonTable(int nSpid,int nServerIndex);

	//删除去年的表
	void DelLastYearTable(int nSpid,int nServerIndex);
protected:
	virtual VOID OnRun();
	
private:
	//把数据包传递给这个类保存到数据库
	CLocalLogServerSocket*		pSrv;
	CSQLConenction*				m_pSQLConnection;		//数据库连接对象
	TICKCOUNT				m_nextCreateTtableTick;            //上一次的调用的tickCount

	CBaseList<CDataPacket* >    m_dataList;             //待处理的数据列表
	
	CBaseList<CDataPacket* >    m_freeDataList;          //空闲的数据列表

};

#endif