#pragma once

class CNameServer;

class CNameServerClientConnection : 
	public CCustomJXServerClientSocket
{
	friend class CNameServer;

typedef VOID (CNameServerClientConnection::*OnHandleSockPacket)(CDataPacketReader &packet);

public:
	typedef CCustomJXServerClientSocket Inherited;

public:
	CNameServerClientConnection(CNameServer *lpNameServer);
	~CNameServerClientConnection();

protected:
	//当长时间没有通信后发送保持连接的消息
	//VOID SendKeepAlive();
	//覆盖处理断开连接的回调
	VOID OnDisconnected();
	//处理套接字错误
	VOID OnError(INT errorCode);
private:
	//分派数据包处理
	VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	//验证服务器注册数据
	bool OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData);
	//各个通信数据包处理函数
	VOID DefaultHandle(CDataPacketReader &inPacket);
	VOID AllocCrossServerCharIdHandle(CDataPacketReader &inPacket);
	VOID AllocCharIdHandle(CDataPacketReader &inPacket);
	VOID AllocGuildIdHandle(CDataPacketReader &inPacket);
	VOID RenameCharHandle(CDataPacketReader &inPacket);
	VOID RenameGuildHandle(CDataPacketReader &inPacket);
	//申请角色、帮会名称和ID通用的处理函数
	VOID CommAllocIdProc(CDataPacketReader &inPacket, int nRetCmd, LPCSTR sSQLCall);
	//重命名角色、帮会的通用处理函数
	VOID CommRenameProc(CDataPacketReader &inPacket, int nRetCmd, LPCSTR sSQLCall);

private:
	CNameServer				*m_pNameServer;		//所属名称服务器
	CSQLConenction			*m_pSQLConnection;	//使用的数据库连接对象

	//定义结构化网络数据包处理函数列表
	static const OnHandleSockPacket		SockPacketHandlers[];	//结构化网络数据包处理函数列表
};