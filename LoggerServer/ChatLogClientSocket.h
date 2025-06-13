#ifndef	_Test_CLIENT_SOCKET_H_
#define	_Test_CLIENT_SOCKET_H_

#define MAXCOUNTSERVER 10000

//发送消息
#define SEND_CHATLOG_CMD 1			//发送聊天消息		
#define SEND_GET_NEW_SERVER_LIST 2	//发送获取最新远程服务器配置
//接收消息
#define REV_CHECKSEVERINDEX_CMD 1	//查看某个服务器的日志
#define REV_SHUTUPCHAT_CMD 2		//禁言

typedef struct tagLookServerList
{
	int		nServerIndex;			
}LOOKSERVERLIST;

class CCustomJXServerClientSocket;
class CChatLogServerSocket;
//class CNetworkDataHandler;

class CChatLogClientSocket:
	public CCustomJXServerClientSocket
{

public:
	CChatLogClientSocket(void);
	CChatLogClientSocket(CChatLogServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	~CChatLogClientSocket(void);

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

	void RemoveSerindex(int nIndex);

	VOID OnRevShutUpOp(CDataPacketReader &inPacket);
public:
	int m_ServerList[MAXCOUNTSERVER];			//查看的服务器列表
	int m_nCount;
private:
	/*
	* 处理逻辑包
	* pDataBuff:为真正需要处理的数据包，这里只处理日志类型的数据包，每次处理一个数据包;nLen:数据的长度
	* 返回：实际处理的字节数，事实上pDataBuff的大小会比数据包大,返回0表示出错没处理
	*/
	int ProcessData(void* pDataBuff,int nLen);

	/*****以下四个函数分别保存4大类的日志内容*****/	
	VOID OnRevLookServerIndex(CDataPacketReader &inPacket);
protected:
	virtual VOID OnRun();
	VOID OnDisconnected();
private:
	//把数据包传递给这个类保存到数据库
	CChatLogServerSocket*		pSrv;
	CSQLConenction*				m_pSQLConnection;		//数据库连接对象

	CBaseList<CDataPacket* >    m_dataList;             //待处理的数据列表
	
	CBaseList<CDataPacket* >    m_freeDataList;          //空闲的数据列表

};

#endif