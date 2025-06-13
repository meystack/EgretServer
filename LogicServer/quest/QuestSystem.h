#pragma once
#include<bitset>
/******************************************
*	定义任务系统的相关接口
*	本系统对应每个角色类，记录本角色的任务数据，以及操作的接口
*	zac
*******************************************/

//class CEntitySystem;
class CQuestData;
class CActor;
class CNpc;
struct NpcTeleport
{
	int nTelNpcId; //传送npcid
	int nAuto;//自动
	int nQid;//
	int nQstate;//
	int nIsOpen;//
	NpcTeleport()
	{
		memset(this, 0, sizeof(*this));
	}
};


#define	MAXACTORQUEST	30			//最多30个任务

#define	QUESTMAIN	10000			//
class CQuestSystem:
	public CEntitySubSystem<enQuestSystemID,CQuestSystem,CActor>
{
public:
	enum QuestEvent{
		enOnCompleteQuest = 1,		//任务完成
	};

	virtual bool Initialize(void *data,SIZE_T size);

	//继承父类接口，处理网络数据包
	virtual  void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	//存盘
	//把角色的任务数据保存到数据库中
	void  Save(PACTORDBDATA pData);

	//定时处理
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount);

	//从数据库读出数据,有某些任务是固定时段做的，所以从数据库读出的时候，要判断下这时候还能不能做
	//不能做的话不能加到任务列表中，并把该任务改成超时状态
	VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode, CDataPacketReader &reader);

	// //增加一个任务到列表中,nQid为对应的任务号,添加成功返回TRUE，否则FALSE
	// //能否接一个任务取决于：
	// // 1、正在进行的任务列表是否含有该任务(不可以重复接）、2、所在的场景或者对话npc是否匹配、3是否满足激活条件 4是否完成了父任务 5是否达到周期任务的条件
	// // 6子任务未完成，不能再接此任务 7、已经完成过的任务，是否可以再次接
	// /*
	// * 备注:
	// * 参数 int nQid:任务的id
	// * 参数 BOOL bFlag:如果TRUE，表示符合条件后，会给玩家添加一个任务，如果bFlag是False，表示只是判断是否可以接这个任务，不会自动给玩家增加任务
	// * int* nRepeatTime:计算日常任务在周期内还剩多少次
	// * bool boGoldAccept:是否包含用元宝可接
	// * @Return int: 
	// */
	bool AddQuest(QuestInfoCfg* cfg);

	// int AddQuestToList( int nQid, const QuestConfig* pConfig );

	// //判断某个任务是否已经完成
	// BOOL IsFinish(INT_PTR nQid);

	// //判断是否主线任务并且是否完成
	// bool IsMainQuestFinish(INT_PTR nQid);

	// VOID SetFinish(int nQid,bool boFlag = true);

	// // 正常完成一个任务，nQid为任务id
	// // 正确返回0，否则返回错误码，完成成功会把存盘，并且从正在进行的任务列表删除。
	// // 注意：正常完成的任务需要正确接收到奖励才算完成
	// int FinishQuest(int nQid,const QuestConfig* pConfig, bool boSpeedFin = false);
	
	// 检查是否正在做这个任务
	bool HasQuest(int nQid);

	// int CanGetAward();

	
	// /*
	// * 备注:改变任务目标值的函数
	// * 参数 int nQid:任务id
	// * 参数 int nTargetId:需求（目标）id
	// * 参数 int nId:代表某个类型的id，比如物品id
	// * 参数 int nValue:值
	// * Param boAutoFinish:是否需要判断任务是否自动完成。在任务数据第一次从DB加载的时候，不需要判断任务自动完成（后面的时间会判断），
	// 因为有可能这个时候物品数据还没加载，收取任务奖励后可能会导致背包满了而无法加入用户原有的物品
	// * @Return int: 
	// */
	// VOID SetQuestValue(int nQid,int nId,int nValue,bool boAutoFinish);

	// /*
	// * 备注:每次增加目标值，+nValue
	// * 参数 int nQid:任务id
	// * 参数 int nTargetId:需求（目标）id
	// * 参数 int nId:代表某个类型的id，比如物品id
	// * @Return int: 错误码，0是正确，非0失败
	// */
	// VOID AddQuestValue(int nQid,int nId,int nValue);

	// /*
	// * 备注:得到某个任务的需求值
	// * 参数 int nQid:
	// * 参数 int nTargetId:
	// * 参数 int nId:
	// * 参数 int nValue:
	// * @Return int: 
	// */
	// int GetQuestValue(int nQid,int nId);

	// //获得某个任务配置
	// const QuestConfig* GetQuestCfg(int index);

	// /*
	// * 备注:当npc进入视野时，要计算这个npc身上是否有可领取的任务、或者交接的任务,如果有，要发消息通知客户端
	// * 参数 CNPC * pNpc:
	// * @Return VOID: 
	// */
	// VOID SendNpcQuestState(CNpc* pNpc);


	// /*
	// * Comments:
	// * Param int nTargetType:对应的目标类型,应该只处理杀怪和物品两种情况
	// * Param int nId:对应的怪物id或者物品id等等
	// * Param int nCount:变化的数值，增加是整数，减少是负数
	// * @Return VOID:
	// */
	VOID OnQuestEvent(INT_PTR nTargetType, INT_PTR nCount, INT_PTR nId = 0);

	

	// inline INT_PTR GetQuestCount() {return m_QuestList.size();}


	//进入游戏
	virtual void OnEnterGame();



	// //任务传送
	// void  QuestTelport(int nQuestId,int nTargetId=-1);

	// //主线任务传送
	// void  MainQuestTelport(int  nQuestId,  LPCTSTR  sSceneName,  LPCTSTR  sNpcName , int  nState);

public:
	//构造函数和虚函数
	CQuestSystem();
	virtual ~CQuestSystem();
	

	/*
	* Comments:触发一个事件
	* Param INT_PTR nSubEvent: 子事件的ID
	* Param INT_PTR nParam1:参数1，不需要就直接按默认参数为-1
	* Param INT_PTR nParam2:参数2，不需要就直接按默认参数为-1
	* Param INT_PTR nParam3:参数3，不需要就直接按默认参数为-1
	* Param INT_PTR nParam4:参数4，不需要就直接按默认参数为-1
	* @Return void:
	*/
	// void TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1=-1,INT_PTR nParam2=-1,INT_PTR nParam3=-1,INT_PTR nParam4=-1);

	// /*是否在任务NPC附近*/
	// bool IsNearByQuestNpc( WORD nQuestId, int nType );

	/// 发送正在进行的任务数据
	void SendAllQuestData();
	//设置完成任务
	void SetfinishQuest(int nQid);
	//判断当前任务是否完成
	int GetfinishQuestState(int nQid);
	//检测可接取的任务
	void CheckCanAcceptQuest();
	//处理完成任务是状态的的自动切换
	void DealAutoChangeQuestState(int Type, int nQid, int nState, QuestInfoCfg* cfg = NULL);
	//修改任务状态
	void ChangeQuestItemState(int nQid, int nState, Uint64 npcHandle = 0);
	//获取正在进行的任务数据
	QuestItem* GetGoingQuestData(int nQid);
	//检测当前任务是否完成
	bool CheckComplete(QuestItem* item, int nLimit);
	//修改任务状态
	bool ChangeGoingQuestState(int nQid, int nState);
	//修改任务状态
	bool ChangeGoingQuestState(std::vector<ChangeQuestState>& nChangeList);

	
	bool returnQuestData(QuestItem* item,CDataPacket& DataPacket);
	//是否在已接循环任务中
	bool HasInRingQuest(int nQid);
	//删除任务
	void RemoveGoingQuestData(int nQid);
	//发送新加的任务
	void SendAddNewQuest(QuestItem* item);
	//返回修改状态后的任务信息
	void returnClientQuestState(int nQid, int nCode);
	//返回修改状态后的任务信息	
	void SendClientQuestState(int nQid);
	//保存
	void SaveData();
	//任务传送
	void QuestTeleport(int nQid, int nType, int nId);
	// //完成一个任务
	// VOID SendFinishQuest(int nQid);

	// void SetTelNpcId(int nNpc) { nTelNpcId = nNpc;}
	// int GetTelNpcId() { return nTelNpcId;}

	void SendQuestTelResult(int nCode = 0);

	void ClearAllQuest();
	bool GMSetQuestState(int nQid, int nState);
	//处理以完成任务
	void DealAutoCompleteCfgValue(int nQid);
private:

	/* 
	* Comments:通用完成目标
	* Param CDataPacketReader & packet:
	* @Return void:
	*/
	// void CommonSpeedFinish(CDataPacketReader & packet);

public:
	std::vector<QuestItem>						m_GoingQuestList;	//正在进行的任务列表及其数据
	std::vector<int>   m_RingStartQId; //环形任务初始id 用于判断自动接取不能重复的任务 
	char m_FinishedQuest[QUESTBTYE]; //已完成的任务id
	CTimer<30000>					m_timer;		//定期刷新可接受任务的时间器,1分钟刷一次,
	bool isDataModify ;// 数据变化
	NpcTeleport  m_infos;
};
