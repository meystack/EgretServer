#ifndef			_LOG_SENDER_H_
#define			_LOG_SENDER_H_

//class CNetworkDataHandler;
class CCustomJXClientSocket;

class LogSender:
	public CCustomJXClientSocket
{
public:
	/*
	* Comments:登陆日志
	* Param WORD nLogid:消息类型
	* Param int nUserid:帐号id
	* Param LPCTSTR szAccount:帐号名称
	* Param LPCTSTR szLoginIp:用户的ip地址
	* Param LPCTSTR szLoginDescr:描述
	* Param int nLevel：等级
	* Param int nYb:元宝
	* Param unsigned int nOnlineTime;在线时间
	* Param unsigned int nServerId :原区服id
	* @Return VOID:
	*/
	VOID SendLoginLog(WORD nLogid,int nUserid,LPCTSTR szAccount,
					LPCTSTR szLoginIp,LPCTSTR szLoginDescr=NULL, int nLevel=0,  int nYb=0, unsigned int nOnlineTime=0, unsigned int nActorId=0,unsigned int nSrvId = 0);

	/*
	* Comments:记录元宝的消费
	* Param WORD nLogid:消息类型
	* Param int nActorid:角色id
	* Param LPCTSTR szAccount:帐号名称
	* Param LPCTSTR szCharName:角色名称
	* Param int nCount:余额
	* Param int nPaymentCount:消费总价钱
	* Param LPCTSTR szConsumeDescr:描述，如物品的名字
	* Param int  nConsumeLevel:玩家的消费的等级
	* Param int  nBalance :消费后的余额
	* @Return VOID:
	*/
	VOID SendConsumeLog(WORD nLogid,int nActorid,LPCTSTR szAccount,
		LPCTSTR szCharName,BYTE nMoneytype,int nCount,int nPaymentCount,LPCTSTR szConsumeDescr=NULL,int nConsumeLevel=0, int nBalance = 0,unsigned int nSrvId = 0);


	/*
	* Comments:记录道具日志
	* Param WORD nLogid:消息类型
	* Param BYTE nType:类型
	* Param int nActorid:角色id
	* Param int nAccountid: 
	* Param LPCTSTR szAccount:帐号名称
	* Param LPCTSTR szCharName:角色名称
	* Param int nCount:道具数量
	* Param int nItemId:道具id
	* Param LPCTSTR szConsumeDescr:描述，如物品的名字
	* @Return VOID:
	*/
	VOID SendItemLocalLog(BYTE nType, WORD nLogid, int nActorid, int nAccountid, LPCTSTR szAccount,
		LPCTSTR szCharName, WORD wItemId, int nCount, LPCTSTR szConsumeDescr=NULL, unsigned int nSrvId = 0);
	
	VOID SendDealToLog(int nLogid, unsigned int nTime,
		int nActorid,int nTarActorid,
		INT nItemId=0,WORD nItemNum=0,WORD nMoneyType=0,INT64 nMoneyNum=0, int nDues = 0,unsigned int nSrvId = 0);

	/*
	* Comments:发送在线人数
	* Param int nServerIndex:如果nServerIndex为0，表示所有逻辑服务器的总人数
	* Param int nCount:在线人数
	* @Return VOID:
	*/
	VOID SendOnlineLog(int nServerIndex,int nCount);

	VOID SendCommonLog(WORD nLogid,int nActorId,LPCTSTR sActorName,
						int nPara0=0,int nPara1=0,int nPara2=0,
						LPCTSTR szShortStr0="",LPCTSTR szMidStr0="",LPCTSTR szLongStr0="",
						LPCTSTR szShortStr1="",LPCTSTR szMidStr1="",LPCTSTR szLongStr1="",
						LPCTSTR szMidStr2="",LPCTSTR szLongStr2="",unsigned int nSrvId = 0);
	
	/*
	* Comments:发送建议的日志
	* Param int nServerIndex:服务器的编号
	* Param nType :日志的类型
	* Param char * sActorName:玩家的名字
	* Param char * sAccount:账号的名字
	* Param char * sTitle:标题
	* Param char * sDetail:具体的内容
	* Param int nLevel :玩家的等级
	* @Return VOID:
	*/
	VOID SendSuggestLog(int nType, char * sActorName,char * sAccount, char * sTitle, char * sDetail,int nLevel,unsigned int nServerId = 0);


	/*
	* Comments:发送击杀掉落日志
	* Param int nActorid:角色id
	* Param LPCTSTR sAccount:帐号名称
	* Param LPCTSTR sCharName:角色名称
	* Param LPCTSTR sMonName:怪物名称
	* Param LPCTSTR sSceneName:场景名称
	* Param LPCTSTR sItemName:道具名称
	* Param int  nCount:数量
	* Param int  nKilltime:击杀时间
	* Param int  nPosx:x坐标
	* Param int  nPosy:y坐标
	* @Return VOID:
	*/
	VOID SendKillDropLog(int nActorid,LPCTSTR sAccount,LPCTSTR sCharName,
	LPCTSTR sMonName,LPCTSTR sSceneName,LPCTSTR sItemName,int nCount, int nKilltime, int nPosX, int nPosY,unsigned int nSrvId = 0);
	/*
	* Comments:发送参与活动进度
	* Param int nActorid:角色id
	* Param LPCTSTR sAccount:帐号名称
	* Param LPCTSTR sCharName:角色名称
	* Param int  nAtvId:活动 id
	* Param int  nIndex:进度
	* Param int  nJoinTime:时间
	* @Return VOID:
	*/
	VOID SendJoinAtvToLog(int nActorid,LPCTSTR sAccount,LPCTSTR sCharName,int nAtvId, int nIndex, int nJoinTime,unsigned int nSrvId = 0);

	inline INT GetServerIndex(){return ServerIndex;}
	inline jxSrvDef::SERVERTYPE GetServerType(){return ServerType;}
	inline LPCSTR GetServerName() {return ServerName;}

	//设置服务器的编号
	VOID SetServerIndex(const INT nSerIdx);

	VOID SetServerType(const jxSrvDef::SERVERTYPE nSrvType){ ServerType = nSrvType;}
	VOID SetServerName(LPCTSTR sServerName);

	void SetServerSpid(LPCTSTR sSpid);

	//获取查看聊天记录
	bool GetChatLogFlag() {return m_nFlag;}

	/*
	* Comments:发送聊天消息
	* Param int nChannleID:聊天频道
	* Param char * sActorName:玩家的名字
	* Param char * msg:聊天类容
	* Param int nActorId :玩家的id
	* @Return VOID:
	*/
	VOID SendChatLog(int nChannleID, char * sActorName,unsigned int nActorId, char * msg);

	//设置是否查看聊天记录
	void SetChatLogFlag(int nFlag) 
	{
		if(nFlag == 0)
		{
			m_nFlag = false;
		}
		else
		{
			m_nFlag = true;
		}
	}
public:
	LogSender();
	LogSender(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName);
	~LogSender(void);


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

	//提供向最外层逻辑通知连接建立的事件
	virtual VOID OnConnected();

private:
	//CNetworkDataHandler DataHandler;//用于解网络包

	//以下4个值用于发送注册包
	jxSrvDef::SERVERTYPE	ServerType;		//服务器类型，值为SERVERTYPE枚举类型中的枚举值
	INT						ServerIndex;	//服务器唯一编号（服务器编号仅对游戏服务器有意义）
	CHAR					ServerName[128];	//服务器名称，需包含0终止符
	bool					m_nFlag;			//是否查看聊天记录
	char					m_sSpid[32];

};

#endif
