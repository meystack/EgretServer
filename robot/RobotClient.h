/*模拟登陆的客户端连接
* 模拟客户端的连接，发起多个连接到服务器去
*每个客户端都需要维护一个当前的状态机
*/
#pragma  once 

//class CLogicAgent;

class CRobotClient:
	public CCustomWorkSocket
{
public:
	typedef CCustomWorkSocket Inherited;

	//客户端连接到的服务器
	enum enServer
	{
		eServerNone,                   //初始化的状
		eServerSession,                //连接到了session网关
		eServerDbsever,                //连接到了db服务器
		eServerLogic,                  //连接到逻辑服务器
	};
	
	//当前的通信状态,于网关的通信的状态
	enum enConmStatus
	{
		eStatusIdle ,                    //空闲状态  
		eStatusEncryReq ,                //请求服务器的密钥状态，这个时候Socket
		eStatusCommunication,            //正常的通信阶段
		eStatusEncryReqDisConnect,      //请求连接状态断开
		eStatusCommuDisConnect,         //连接状态断开
		eStatusMax,                      //最大的状态数目
	};

	CRobotClient()
	{
		m_nCurrentServer = eServerNone; 
		m_nCurrentStatus = eStatusIdle; 
		//每个代理的初始化
		m_logicAgent.Init(this);
		m_dwReconnectTick =0;
		m_nAccountId =0;  
		m_sActorName[0]=0;
	}

	/**** 以下函数集为子类可能需要覆盖的函数 ****/
	//处理接受到的服务器数据包
	virtual VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	//进行内部消息的处理分派
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	//每次逻辑循环的例行逻辑处理函数
	virtual VOID OnRun();

	//例行执行函数
	virtual VOID SingleRun();

	//提供向最外层逻辑通知连接建立的事件
	virtual VOID OnConnected();

	//断开连接的时候
	virtual VOID OnDisconnected();

	//出現錯誤
	virtual VOID OnError(INT errorCode);

	//申请一个带数据头的数据包
	CDataPacket& AllocProtoPacket();

	//发送数据包
	VOID FlushProtoPacket(CDataPacket& packet);

	//获取登陆的账户
	inline LOGINACCOUNT & GetAccount() {return m_account;}

	/*
	* Comments:
	* Param char address:
	* Param int nPort:
	* @Return void:
	*/
	inline void SetServerAddress(char* address, int nPort)
	{
		struct hostent * iterName= gethostbyname(address);
		if(iterName)
		{
			char add[100] ;
			sprintf(add,"%d.%d.%d.%d",
				(iterName->h_addr_list[0][0]&0x00ff),
				(iterName->h_addr_list[0][1]&0x00ff),
				(iterName->h_addr_list[0][2]&0x00ff),
				(iterName->h_addr_list[0][3]&0x00ff));
			strcpy(m_serverIp,add);
		}
		else
		{
			strcpy(m_serverIp,address);
		}
		m_nServerPort = nPort;
	}

	//获取玩家的账户的ID
	inline int GetAccountId(){return m_nAccountId;}

	//设置玩家账户的ID
	inline void SetAccountId(int nAccountId){m_nAccountId =nAccountId; }

	/*
	* Comments:速传到场景id指定点
	* Param int nSceneId:
	* Param int nX:
	* Param int nY:
	* @Return bool:
	*/
	bool MoveBySceneId(int nSceneId, int nX, int nY);

	/*
	* Comments:速传到场景名指定点
	* Param LPCTSTR sSceneName:
	* Param int nX:
	* Param int nY:
	* @Return bool:
	*/
	bool MoveBySceneName(LPCTSTR sSceneName, int nX, int nY);

	//连接到服务器
	BOOL ConnectToServer();
	
	//处理数据
	void ProcData();

	//设置玩家的角色的ID
	inline void SetActorId(unsigned int nActorID){m_nActorID =nActorID;}

	//获取玩家的角色的ID
	inline unsigned int GetActorId(){return m_nActorID;}

	//设置角色的名字
	inline void SetActorName(char * name) { strncpy(m_sActorName, name,sizeof(m_sActorName));}

	//获取角色的名字
	inline char * GetActorName(){return m_sActorName;}

	void ConnectNextServer(); //连接下一个服务器

	//获取逻辑代理
	inline CLogicAgent &GetLogicAgent(){return m_logicAgent;}

	//获取当前所连接的服务器
	int GetCurrentServerType() {return m_nCurrentServer;}

	//获取当前的通信状态
	int GetCurrentStatus(){return m_nCurrentStatus;}

	// 获取玩家数据
	inline CClientActor& GetActorData(){ return m_ActorData;}

	inline void SetCnName(LPCTSTR sName)
	{
		CUtility::CopyCharArrayS(m_szCnName, sName);
	}

	inline char * GetCnName()
	{
		return m_szCnName;
	}

	TICKCOUNT GetCurTick()
	{
		return m_CurTick;
	}

private:
	
	CAgent * GetCurrentAgent() ;//当前那个代理负责处理处理数据
	
public:
	LOGINACCOUNT     m_account;           //玩家的角色
	int              m_nCurrentServer;    //玩家当前连接到的服务器
	int              m_nCurrentStatus;    //当前的通信状态
	Encrypt          m_encryptHander;     //加密器
	CLogicAgent      m_logicAgent;        //逻辑数据代理
	char             m_serverIp[32];      //当前服务器的地址
	int              m_nServerPort;       //当前服务器的端口
	int              m_nAccountId;        //玩家的账户的ID
	unsigned int     m_nActorID;          //玩家的角色的ID
	char             m_sActorName[32];    //玩家的角色的名字
	char			 m_szCnName[32];	  // 解码后的汉字
	uint16_t		 m_nKey;

	TICKCOUNT		 m_CurTick;
	CClientActor	 m_ActorData;		  // 玩家数据


private:
	TICKCOUNT        m_dwReconnectTick;   //连接到服务器的tick
};