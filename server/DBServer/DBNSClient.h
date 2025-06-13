#ifndef _DB_NAME_SYNC_CLIENT_H_
#define _DB_NAME_SYNC_CLIENT_H_

class CDBServer;

class CNameSyncClient :
	public CCustomJXClientSocket
{
public:
	typedef CCustomJXClientSocket Inherited;
	typedef VOID (CNameSyncClient::*OnHandleSockPacket)(CDataPacketReader &packet);

public:
	CNameSyncClient(CDBServer *lpDBEngine);
	~CNameSyncClient();

	//投递申请角色ID的内部消息，申请角色ID成功或失败后（会处理超时），会向网关线程投递?消息
	BOOL PostAllocateCharId(const int nSessionId, const int nServerIndex, LPCSTR sCharName,CREATEACTORDATA *pData);
	//投递申请跨服角色ID的内部消息，申请角色ID成功或失败后（会处理超时），会向网关线程投递?消息
	BOOL PostCrossServerAllocateCharId(const int nSessionId, const int nServerIndex, LPCSTR sCharName,CREATEACTORDATA *pData);

	//投递申请帮会ID的内部消息，申请帮会ID成功或失败后（会处理超时），会向数据服务器线程投递?消息
	//参数nOPIdent是调用者生成的具有唯一性的操作标识数据，当操作具有结果（成功、失败或超时）后
	//向数据服务器投递的对应消息中将此值原样返回
	BOOL PostAllocateGuildId(const int nServerIndex, LPCSTR sGuildName, const UINT_PTR nOPIdent);
	//投递更改角色名称的内部消息，操作成功或失败后（会处理超时），会向数据服务器线程投递?消息
	//参数nOPIdent是调用者生成的具有唯一性的操作标识数据，当改名操作具有结果（成功、失败或超时）后
	//向数据服务器投递的对应消息中将此值原样返回
	BOOL PostCharRename(const int nCharId, const int nServerIndex, LPCSTR sNewCharName, const UINT_PTR nOPIdent);

protected:
	//覆盖发送注册客户端的消息
//	VOID SendRegisteClient();
	//覆盖发送保持连接消息的函数
	//VOID SendKeepAlive();
	//覆盖例行执行的函数，以便处理名称操作数据
	VOID OnRun();
	/* ★查询本地服务器的类型，以便正确的向会话服务器注册连接 */
	jxSrvDef::SERVERTYPE getLocalServerType();
		/*** 子类需覆盖的函数集 ***/
	/* 处理单个通信数据包 
	 * nCmd		通信消息命令
	 * inPacket	已经读取出通信命令的数据包，数据包的读取位置指向命令数据后的内容
	 */
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);

	/* ★查询本地服务器的名称，以便正确的发送注册数据 */
	virtual LPCSTR getLocalServerName();
private:
	//向名称服务器发送新增加的名称操作数据
	VOID SendNewNameOperateRequest();
	VOID DefaultSockPacketHandler(CDataPacketReader &inPacket);
	VOID AllocCharIdResultHandler(CDataPacketReader &inPacket);
	VOID AllocGuildIdResultHandler(CDataPacketReader &inPacket);
	VOID RenameCharResultHandler(CDataPacketReader &inPacket);
	VOID RenameGuildResultHandler(CDataPacketReader &inPacket);

private:
	//定义名称客户端与名称和ID操作相关的数据结构
	struct NameOperateData
	{
		enum eNameOperateType
		{
			noAllocCharId = 1,	//申请角色ID（客户端创建角色的时候用）
			noAllocGuildId,		//申请帮会ID（引擎服务器创建帮会的时候用）
			noRenameChar,		//角色重命名（引擎服务器修改角色名称的时候用）
			noRenameGuild,		//帮会冲命名（引擎服务器修改帮会名称的时候用）
			noCSAllocCharId,		//申请跨服角色ID（）
		}			nOPType;		//名称操作的类型
		int			nServerIndex;	//服务器ID
		char		sName[128];		//名称字符串
		TICKCOUNT	dwTimeOut;		//超时时间
		union
		{
			//申请角色ID的数据
			struct  
			{
				int	nSesionId;		//全局会话ID
			}	allocCharId;
			//申请帮会ID的数据
			struct  
			{
				UINT_PTR nOPIdent;		//操作唯一标识（由数据客户端处理类内部产生，在操作具有结果原样返回）
			}	allocGuildId;
			//申请修改帮会名称的数据
			struct  
			{
				unsigned int		 nGuildId;		//帮会ID
				UINT_PTR nOPIdent;		//操作唯一标识（由数据客户端处理类内部产生，在操作具有结果原样返回）
			}	guildRename;
			//申请修改角色名称的数据
			struct  
			{
				int		 nCharId;		//角色ID
				UINT_PTR nOPIdent;		//操作唯一标识（由数据客户端处理类内部产生，在操作具有结果原样返回）
			}	renameChar;
		};
		void * pData;                  //缓存的数据指针
	};

	//查找名称操作指针，如果找到对应的数据，则返回数据指针，如果lpIndex非空则填充*lpIndex为数据在操作中列表中的索引
	//如果serverindex为-1，则不限定查找条件
	NameOperateData* GetNameOperatePtr(const NameOperateData::eNameOperateType eOPType, LPCSTR sName, OUT PINT_PTR lpIndex,int nServerIndex=-1);
	//名称操作请求超时
	VOID NameOperateTimedOut(const NameOperateData* lpOperate);

private:
	CDBServer*						m_pDBServer;			//所属DB引擎
	CBaseList<NameOperateData*>		m_NameOperatingList;	//正在操作的名称数据列表
	CLockList<NameOperateData*>		m_NewNameOperateList;	//新增加的名称数据处理列表
	CCSLock							m_NewNameOperateListLock;//增加名称处理数据的列表锁
	static const OnHandleSockPacket	NSSockPacketHandlers[];	//结构化网络数据包处理函数列表

};

#endif

