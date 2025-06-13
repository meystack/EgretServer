#pragma once

/*
typedef enum tagAMServerMsg
{
	AMC_REGIST = 10000,			//注册
	AMC_QUERY_AMOUNT = 10001,	//查询用户余额
	AMC_COMSUME = 10002,		//发送用户消费请求

	AMS_QUERY_AMOUNT = 20001,	//返回用户余额
	AMS_COMSUME = 20002,		//返回用户消费结果
	AMS_PAY_SUCCED = 20003,     //返回充值成功
};
*/

typedef struct tagTASKReq
{
	int Socket;
	int Serverid;
	int Cmdid;
	int Contractid;
	int Step;
	bool operator==(const tagTASKReq& a)const
	{
		if(Serverid == a.Serverid && Cmdid == a.Cmdid && Contractid == a.Contractid && Step == a.Step)
			return true;
		else
			return false;
	}
}TASKREQ, *PTASKREQ;
typedef std::vector<TASKREQ>  TASKREQ_Vec;
typedef std::map<UINT, TASKREQ_Vec > TASKREQ_Map;

typedef enum tagAMErrorNumber
{
	ERROR_NOT_CONNECT_DB	= -100, //无法连接数据库
	ERROR_QUERY_DB			= -101, //查询数据库错误
	ERROR_NOT_REGDIST		= -102, //未注册
	ERROR_NOTFIND_PRODUCT	= -103, //无法找到对应的产品
};

class CCustomJXServerClientSocket;
class CAMServerSocket;
class CProduct;

class CAMClientSocket:
	protected CCustomServerClientSocket
{
public:
	typedef CCustomServerClientSocket Inherited;

	CAMClientSocket(void);
	CAMClientSocket(CAMServerSocket* pSrv,SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	~CAMClientSocket(void);

	char * GetSpid() {return m_Spid;}
	void SendPaySucceed(int nServerIndex,UINT uUserId,UINT nActorId, UINT uMoney);
	void AskTaskSituation(int nserverid, UINT uUserId, UINT cmdid, int contractid, int step );
	void AddTaskMsg(SOCKET nsocket, int nserverid, UINT uUserId, UINT cmdid, int contractid, int step );
	/* 返回true，存在这个msg并删除；返回false，没有这个msg */
	bool DelTaskMsg(SOCKET nsocket, int nserverid, UINT uUserId, UINT cmdid, int contractid, int step );

protected:
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	virtual bool OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData);
	virtual VOID OnRun();
	//按通信协议申请一份发送数据包，nCmd是通信消息号，
	//数据包填充完毕后需要调用flushProtoPacket提交到发送队列
	CDataPacket& allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd);

	//提交发送数据包到发送缓冲，会自动计算数据长度并向数据包的通信协议头部分写入长度值
	VOID flushProtoPacket(CDataPacket& packet);

	VOID SendKeepAlive();

	/* 覆盖父类的网络数据包处理函数以便实现通信协议的数据包解析 */
	VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
private:
	void AddMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader& packet);
	CDataPacket* AllocPacket();
	void FreePacket(CDataPacket* packet);
	void DestroyPacketList(CQueueList<CDataPacket*>& pkgList);
	void HandleAMMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacket &packet);
	int GetErrorNo();

	void RegistClient(CDataPacket &packet);
	void QueryAmount(CDataPacket &packet);
	void Consume(CDataPacket &packet);
	LPCSTR GetResultCode(int op);
	void SendResponse(SOCKET nsocket, LPCSTR sContent);
	void SendTaskSituation(CDataPacket &packet);
	//获取连接的服务器名称
	inline LPCSTR getClientName(){ return m_sClientName; }
private:

	//char  buff[1024];		             
	char					m_gName[10]; //游戏英文缩写
	char					m_Spid[10];  //运营商英文缩写

	CQueueList<CDataPacket*>			m_sMsgList;
	CCSLock								m_sMsgListLock;
	CQueueList<CDataPacket*>			m_sFreeList;
	CCSLock								m_sFreeListLock;

	//CSQLConenction			            m_Mysql;
	CAMServerSocket*					m_pSrv;

	CProduct *							m_Product;
	bool								m_IsRegist;


	unsigned int						m_nClientSrvIdx;		//客户端的服务器ID

	char								m_sClientName[64];		//客户端注册的服务器名称

	jxSrvDef::SERVERTYPE				m_eClientType;			//客户端注册的服务器类型

	int									m_boRegisted;			//客户端是否已经注册连接

	TASKREQ_Map					m_TaskMap;
	RTL_CRITICAL_SECTION	m_TaskMsgVecLock;
};
