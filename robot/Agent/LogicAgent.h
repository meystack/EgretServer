#pragma once

/*
* 与逻辑服务器通信的类，处理逻辑服务器的各种消息
*/
class CLogicAgent:
	public CAgent
{
public:
	typedef CAgent Inheried;
	CLogicAgent();
	virtual ~CLogicAgent();

	//初始化函数
	virtual void Init(CRobotClient * pClient)
	{
		Inheried::Init(pClient);
		m_defaultSystem.Init(pClient);
		m_skillSystem.Init(pClient);
		m_itemSystem.Init(pClient);
		m_equipSystem.Init(pClient);
		m_hasEnterGame =false;
		m_MovePoint.MakePoint(0,0);
		m_HookPoint.SetInvalid();
	}

	//与服务器断开了连接
	virtual void OnConnected();

	//于服务器断开了
	virtual void OnDisConnected();

	//派发网络消息
	virtual void OnRecv(CDataPacketReader & pack);

	//定期执行
	virtual void OnTimeRun(TICKCOUNT tick);

	//登录游戏的时候调用
	inline void OnEnterGame()
	{
		m_hasEnterGame =true;
		m_defaultSystem.OnEnterGame();
		m_skillSystem.OnEnterGame();
		m_itemSystem.OnEnterGame();
		m_equipSystem.OnEnterGame();
		//m_ActionMgr.SetClient(m_pClient);
		//m_ActionMgr.Init();
		//m_ActionMgr.Start();
	}

	//是否登录了游戏
	inline bool HasEnterGame()
	{
		return m_hasEnterGame;
	}

	inline CDefaultSystem& GetDefaultSystem() { return m_defaultSystem; }
	inline CSkillSystem& GetSkillSystem() { return m_skillSystem; }
	inline CEquipSystem& GetEquipSystem() {return m_equipSystem;}
	inline CItemSystem& GetItemSystem() { return m_itemSystem; }
	void OnEvent(INT_PTR nEvtId, INT_PTR nParam1 = 0, INT_PTR nParam2 = 0, INT_PTR nParam3 = 0, INT_PTR nParam4 = 0);

	void SetNextAction(int millsec = 750);
	CClientEntity* GetTarget();
	EntityHandle GetTargetHandle() { return m_TargetHandle; }
	void SetTargetHandle(EntityHandle handle) { m_TargetHandle = handle; }

	// 移动相关
	bool CanAction();
	SMALLCPOINT& GetMovePoint() { return m_MovePoint; }
	void SetMovePoint(int x, int y) { m_MovePoint.x = x; m_MovePoint.y = y; }
	bool CalcPath(bool isLoopLimit = true);
	int GetAndPopNextMovePosi(int& x,int& y,int& dir);
	std::vector<SMALLCPOINT>& GetMovePath(){ return m_searchNodes; }
	void ClearMovePath(){ m_searchNodes.clear(); }

	SMALLCPOINT& GetHookPoint() { return m_HookPoint; }
	void SetHookPoint(int x, int y) { m_HookPoint.x = x; m_HookPoint.y = y; }

	AStar& GetAStar(){ return m_Astar; }

	// 行为状态相关
	void SetActionMove() { m_ActionState |= 0x1; }
	void SetActionAttack() { m_ActionState |= 0x2; }
	void SetActionSkill() { m_ActionState |= 0x4; }
	bool IsActionMove() { return m_ActionState & 0x1; }
	bool IsActionAttack() { return m_ActionState & 0x2; }
	bool IsActionSkill() { return m_ActionState & 0x4; }
	void clearActionState() { m_ActionState = 0; }
	bool IsNoActionState() { return m_ActionState == 0; }

protected:
	// 分配一个网络数据包
	CDataPacket* AllocPacket();
	// 释放网络数据包
	void FreePacket(CDataPacket* packet);
	void FreePacket(CBaseList<CDataPacket*> &list);
	// 处理网络数据包
	bool ProcessNetMsg();
	// 派发消息
	void DispatchMsg(BYTE ucSysId, BYTE ucMsgId, CDataPacketReader& packet);
	// 丢弃所有未处理消息
	void DiscardUnHandledMsg();

	//发送登陆的数据包
	void SendLogin();

	//发送创建账户的数据包
	void SendCreateActor();

	//收到登陆的消息包
	void OnRecvLoginMsg(BYTE ucMsgId,CDataPacketReader & pack);

		//发送数据包去查询角色
	void SendQueryActorList(); 
	
	
	/*
	* Comments:选择角色登入游戏
	* Param unsigned int nActorID:玩家的actorid
	* Param char * sName:玩家的名字
	* @Return void:
	*/
	void SelActorEnterGame(unsigned int nActorID,char * sName=NULL);

	//随机一个名字
	void RandActorName();

	
	/*
	* Comments:创建一个角色
	* Param char * sName:玩家的名字
	* Param int nSex:性别
	* Param int nJob:职业
	* Param int nCamp:阵营
	* @Return void:
	*/
	void CreateActor(char * sName,int nSex=1,int nJob=1, int nCamp =1);

	/*
	* Comments:通过errorid获取错误码的描述
	* Param INT_PTR nErrorCode:错误码
	* @Return char *:返回错误的描述
	*/
	char * GetLoginErrorStr(INT_PTR nErrorCode);

	// 用以A星算法寻路传入的跨格判断
	bool CanCrossCheck(INT_PTR x, INT_PTR y);

protected:
	CDefaultSystem				m_defaultSystem; 	//默认子 系统
	CSkillSystem				m_skillSystem;		// 技能子系统
	CItemSystem					m_itemSystem;
	CEquipSystem				m_equipSystem;		//装备系统

	bool						m_hasEnterGame;    //主角是否已经登录游戏了
	CSwapQueue<CDataPacket*>	m_netMsgList; // 接收到的网络消息列表
	CSwapQueue<CDataPacket*>	m_freeMsgList; // 自由消息包列表
	CBufferAllocator			m_allocator;
	bool						m_bHasTrans;

	AStar						m_Astar;
	TICKCOUNT					m_NextActionTick;
	int							m_ActionState;//动作状态（0x1走/0x2跑/0x4普攻/0x8技能/）
	EntityHandle				m_TargetHandle;//目标句柄
	SMALLCPOINT					m_MovePoint;//移动点
	SMALLCPOINT					m_HookPoint;//挂机点
	std::vector<SMALLCPOINT>    m_searchNodes;//寻路点
};