#ifndef			_LOCALLOG_SENDER_H_
#define			_LOCALLOG_SENDER_H_

//消费金币、元宝的日志 
typedef struct tagMoneyConsumeLog
{
	int		nLogid;			
	int		nMentyType;
	int		nMoneyValue;
	char	sComment[256];
}LOGMONEYCONSUME;

//全局日志消费金币、元宝的日志 
typedef struct tagActorConsumeLog
{
	int			nLogid;				// 日志id
	int			nActorid;
	ACCOUNT		srtAccount;
	ACTORNAME	strName;

	int			nMentyType;
	int			nMoneyValue;
	char		sComment[256];
	int			nLevel;
	int			nCount;				//购买数量
	int			nBalance;			//余额
	int			nServerId;			//
}LOGCONSUMESAVE;	

typedef struct tagKillDropLog
{
	int			nActorid;
	ACCOUNT		srtAccount;
	ACTORNAME	strName;
	ACTORNAME	strMonsetName;
	ACTORNAME	strSceneName;
	ACTORNAME	strItemName;
	int			nCount;
	int         nKilltime;
	int         nPosX; 
	int         nPosY;
	int			nServerId;			//
}LOGKILLDROPSAVE;


typedef struct tagActivitySchedule
{
	int			nActorid;
	ACCOUNT		srtAccount;
	ACTORNAME	strName;
	int			nAtvID;
	int         nIndex;
	int         nJoinTime;
	int			nServerId;			//
}LOGACTIVITYSCHEDULE;

class CActor;
//class CNetworkDataHandler;
class CCustomJXClientSocket;

class CLocalSender:
	public CCustomJXClientSocket
{
public:

	/**
	 * @brief 发送资产变动日志
	 * @param nLogId 消费点ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nLogType 日志类型（1产出，2消费）
	 * @param nCurrencyType 货币类型（1金币 2绑金 3银两 4元宝 ）
	 * @param nAmount 消费金额
	 * @param nBalance 当前余额
	 * @param nIsShop 是否商城消费（1是 2否）
	 */
	VOID SendCurrencyLog(WORD nLogId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop, int nSrvId = 0);
	VOID SendCurrencyLog(WORD nLogId,CActor *pActor,WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop,int nSrvId = 0);
	
	/**
	 * @brief 发送商城购买日志
	 * @param nItemId 道具ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nShopType 商城类型
	 * @param nItemType 道具类型ID
	 * @param nItemNum 购买道具数
	 * @param nCurrencyType 货币类型（1金币 2绑金 3银两 4元宝 ）
	 * @param nConsumeNum 消耗货币总额
	 */
	VOID SendShopLog(INT nItemId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum,int nSrvId = 0);
	VOID SendShopLog(INT nItemId,CActor *pActor,WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum,int nSrvId = 0);
	
	/**
	 * @brief 发送道具获得/消耗日志
	 * @param nItemId 道具ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nItemType 道具类型ID
	 * @param nItemNum 获得/消耗道具数
	 * @param nOperType 操作类型（1获得，2消耗）
	 * @param sReason 获得/消耗道具的原因
	 */
	VOID SendItemLog(INT nItemId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nItemType,WORD nItemNum,WORD nOperType,LPCTSTR sReason,int nSrvId = 0);
	VOID SendItemLog(INT nItemId,CActor *pActor,WORD nItemType,WORD nItemNum,WORD nOperType,LPCTSTR sReason,int nSrvId = 0);
	
	/**
	 * @brief 发送任务接受完成日志
	 * @param nTaskId 任务ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nTaskType 任务类型
	 * @param nStatu 状态（1接收，2完成）
	 */
	VOID SendTaskLog(INT nTaskId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nTaskType,WORD nStatu,int nSrvId = 0);
	VOID SendTaskLog(INT nTaskId,CActor *pActor,WORD nTaskType,WORD nStatu,int nSrvId = 0);
	
	/**
	 * @brief 发送活动参与日志
	 * @param nAtvId 活动ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nAtvType 活动类型
	 * @param nStatu 状态（1接收，2完成）
	 */
	VOID SendActivityLog(INT nAtvId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nAtvType,WORD nStatu,int nSrvId = 0);
	VOID SendActivityLog(INT nAtvId,CActor *pActor,WORD nAtvType,WORD nStatu,int nSrvId = 0);
	
	inline INT GetServerIndex(){return ServerIndex;}
	inline jxSrvDef::SERVERTYPE GetServerType(){return ServerType;}
	inline LPCSTR GetServerName() {return ServerName;}
	VOID SetServerIndex(const INT nSerIdx){ ServerIndex = nSerIdx;}
	VOID SetServerType(const jxSrvDef::SERVERTYPE nSrvType){ ServerType = nSrvType;}
	VOID SetServerName(LPCTSTR sServerName);
	
	VOID SendCreateLocalLog(int nSpid,int Serverindex);

public:
	CLocalSender();
	CLocalSender(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName);
	~CLocalSender(void);

	virtual void OnRountine()
	{
		CCustomJXClientSocket::OnRountine();
	}

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

private:
	//CNetworkDataHandler DataHandler;//用于解网络包

	//以下4个值用于发送注册包
	jxSrvDef::SERVERTYPE			ServerType;		//服务器类型，值为SERVERTYPE枚举类型中的枚举值
	INT			ServerIndex;	//服务器唯一编号（服务器编号仅对游戏服务器有意义）
	CHAR		ServerName[128];	//服务器名称，需包含0终止符
	CMiniDateTime			m_tomorrowDateTime;			// 指示明天的开始时间，也就是今天的结束时间，用于24点的刷新处理（活动、任务等逻辑）
};

#endif
