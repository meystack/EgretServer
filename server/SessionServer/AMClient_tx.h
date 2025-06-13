#pragma once

/************************************************************************/
/*
/*                  AM(Amount Manager)金额管理系统客户端类
/*
/*                  提供查询用户余额以及对用户进行扣费的操作接口。
/*
/************************************************************************/
class CSSManager;

class CAMClient :
	public CCustomClientSocket
{
public:
	typedef CCustomClientSocket Inherited;

	//定义AM操作类型
	enum eAMOPType
	{
		amQueryAmount = 1,	//查询余额
		amConsume = 2,		//扣费
		amPaySucceed =3,    //充值成功了
	};

	//定义AM操作数据
	typedef struct AMOPData
	{
		int opType;	//操作类型
		unsigned int	nUserId;	//用户ID
		INT		nServerId;	//服务器ID
		INT		nResult;	//负数表示失败或发生错误，否则表示操作结果（查询余额则表示余额，扣费时为0表示成功-1表示余额不足）
		INT64	lOPPtr;		//操作对象（客户端穿透服务器的数据）
	}AMOPDATA, *PAMOPDATA;

	typedef struct tagTASKMsg
	{
		INT nServerId;//服务器Id
		unsigned int nUserId; //UserID 账号id
		INT nCmd;
		INT nContractid;		//任务id
		INT nStep;
		INT nResult; //返回值
		INT64 lOPPtr;//操作对象（客户端穿透服务器的数据）
	}TASKMSG, *PTASKMSG;
public:
	CAMClient();
	~CAMClient();

	static char * GetOpCode(int nOp)
	{
		switch(nOp)
		{
		case amQueryAmount:
			return "Query Yb";
			break;
		case amConsume:
			return "Draw Yb";
			break;
		case amPaySucceed:
			return "Pay Ok";
			break;
		default:	
			return "Error op";
		}
	}


	/*
	* Comments: 获取AM操作结果队列
	* Param wylib::container::CBaseList<AMOPDATA> & list: 保存操作结果的列表
	* @Return INT_PTR: 函数返回向list中保存了多少个操作结果
	* @REMARKS: 将操作结果保存到list后会清空类本身内部的操作结果
	*/
	INT_PTR GetAMOPResults(wylib::container::CBaseList<AMOPDATA> &list);

	INT_PTR GetTaskOPResults(wylib::container::CBaseList<TASKMSG> &list);

	/*
	* Comments: 发送查询用户余额的数据
	* Param INT_PTR nUserId: 用户ID
	* Param INT_PTR nServerId: 服务器ID
	* Param INT64 lOPPtr: 穿透数据
	* @Return VOID:
	*/
	VOID PostQueryAmount(unsigned int nUserId, INT_PTR nServerId, INT64 lOPPtr);
	/*
	* Comments: 发送对用户进行扣费的数据
	* Param INT_PTR nUserId: 用户ID
	* Param INT_PTR nServerId: 服务器ID
	* Param INT_PTR nAmount: 扣费数量，必须是正整数
	* Param LPCSTR sCharName: 进行消费的角色名称
	* Param INT64 lOPPtr:提取的角色id
	* Param BYTE nLevel: 提取等级
	* @Return VOID:
	*/
	VOID PostConsume(unsigned int nUserId, INT_PTR nServerId, INT_PTR nAmount, LPCSTR sCharName, INT64 lOPPtr, BYTE nLevel);


	//发送集市任务等信息给am
	VOID PostAmTaskData(tagTASKMsg &data);
public:
	//获取产品名称
	LPCSTR GetProductName();

	//设置产品名称
	VOID SetProductName(LPCSTR sProductName);

	//获取SPID
	LPCSTR GetSPID();

	//设置SPID
	VOID SetSPID(LPCSTR sSPID);

protected:
	//处理接受到的服务器数据包
	VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	//连接到服务器成功后发送注册客户端的消息
	VOID SendRegisteClient();
	//当长时间没有通信后发送保持连接的消息
	VOID SendKeepAlive();

private:
	//处理单个通信数据包
	VOID ProcessRecvPacket(CDataPacketReader &packet);
	//重载申请数据包的函数，在包头写入数据包起始标志
	//CDataPacket& allocSendPacket();

	CDataPacket& allocProtoPacket(int  nCmd);

	//重载提交数据包的函数，在包尾写入数据包起始标志
	VOID flushProtoPacket(CDataPacket& packet);

	VOID OnDispatchRecvPacket(int nCmd, CDataPacketReader &inPacket) ;

private:
	//CHAR	m_RecvForCompilerError[1024];//VC2010 编译器BUG，编译后会使得此类的成员地址与父类成员相交，因此此处腾出1K字节避开此BUG
	CHAR	m_sProductName[32];	//产品名称
	CHAR	m_sProviderId[32];	//SPID
	wylib::container::CQueueList<AMOPDATA> m_OPResultList;//操作结果队列

	wylib::container::CQueueList<TASKMSG> m_OPRTaskList;//操作结果队列
	CCSLock	m_OPResultLock;		//操作结果队列锁

	CCSLock	m_OPTaskLock;		//操作结果队列锁
};