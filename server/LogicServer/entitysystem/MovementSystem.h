#pragma once
/***************************************************************/
/*
/* 实体移动子系统
/* 处理实体的移动包，跳跃包
/* 模拟实体的移动和跳跃
/* 对于 宠物，怪物等需要模拟运动的实体，这里处理这些运动的模拟
/***************************************************************/
#pragma once

//如果需要开启a*寻路的话，就使用这个
#ifndef __USE_ASTAR__SEARCH_
#define  __USE_ASTAR__SEARCH_
#endif 

class CMovementSystem:
	public CEntitySubSystem<enMoveSystemID,CMovementSystem,CAnimal>
{
public:
	

void  HandFlyShoes(CDataPacketReader &packet) ;
	//实体的移动的状态
	typedef enum tagMotionType
	{
		mtMotionTypeIdle,    //发呆状态
		mtMotionTypePatrol,   //巡逻，以出生点为中心一个半径的范围里移动
		mtMotionTypeChase,    //追逐
		mtMotionTypeFleeing,  //逃避
		mtMotionTypeFollow,    //跟随（主要是宠物）
		mtMotionTypePoint,  //往一个点移动
		mtMotionTypeReturnHome, //回巢的移动
		mtMotionTypeRandomFlee, // 随机乱跑。和flee的差别在于无目标的概念；和乱走的差别在于每次跑都是单独的运动，不能在一次逃跑中左右晃动。
		mtMotionTypeConfused, //迷糊状态,乱走
	}MotionType;
	#define   ASTAR_MAX_MOVE_STEP 1000  //A*算法最大执行多少
	#define MOVECHECKTIMER 480000		//多久没动就踢下线
	typedef CEntitySubSystem<enMoveSystemID,CMovementSystem,CAnimal> Inherid;
	friend class CEntitySubSystem<enMoveSystemID,CMovementSystem,CAnimal>;

	enum tagMoveFlag
	{
		mfTransport =0, //移动的标记
		mfMaxMoveFlagID,//非法的标记
	};
	//处理移动数据
	void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );
	
	CMovementSystem()
	{
		m_canMoveTick =0;
		m_useAStar =false;
		nFlyShoeNpcId = 0;
	}
	
	
	/*
	* Comments: 移动实体移动，每次往指定方向上移动nStep格
	* Param int nDir: 方向
	* Param int nPosX:当前点的坐标x
	* Param int nPosY:当前点的坐标y
	* Param int nStep: 格子的数量
	* Param int nSendPkgTime: 客户端发送包的时间戳。针对移动数据包监测用
	* Param  bool telSelf： 是否广播给自己 
	* Param INT_PTR nMoveSpeed: 移动速度，为了支持野蛮冲撞，增加了移动速度，因为需要移动一些不可移动的怪物，这个移动速度如果为0无效
	* Param INT_PTR nCmd : 移动时候的命令，用于一些特殊场合，野蛮这些技能使用
	* Param INT_PTR nClientFrame: 客户端移动所用的帧，野蛮这些技能使用，野蛮冲撞的技能，使用跑步的动作模拟冲撞，一次1格，为了更真实，一步只能使用6帧的跑步帧的3帧
	                          这里 0表示是起步的动作，1表示是非起步的动作，一般的是忽略的
	* Param INT_PTR nSimulateReboundSpeed: 失败了模拟反弹(野蛮冲撞里用到)的速度，为0表示不模拟反弹
	* @Return bool:成功返回true,否则返回false
	*/
	bool Move(INT_PTR nDir,INT_PTR nPosX, INT_PTR nPosY,INT_PTR nStep=1, unsigned int nSendPkgTime = 0,bool telSelf =false,
		INT_PTR nMoveSpeed =0,INT_PTR nCmd =0,INT_PTR nClientFrame=0,INT_PTR nSimulateReboundSpeed =0) ;

	/*
	* Comments: 跳跃
	* Param int nPosX: 落点X
	* Param int nPosY: 落点Y
	* @Return bool: 跳跃成功否
	*/
	bool Jump(int nPosX, int nPosY);
	

	/*
	* Comments: 瞬间移动到一个点
	* Param int nPosX: 目标点的x
	* Param int nPosY: 目标点的y
	* Param int isUseRand: 目标点不存在时是否使用回字查找周边点 1使用
	* @Return bool: 成功返回true,否则返回false
	*/
	bool InstantMove(int nPosX,int nPosY,int isUseRand=1, unsigned nType=0,unsigned nValue=0);
	

	/*
	* Comments:获取一个点周围的可移动的点
	* Param CScene * pScene:场景的指针
	* Param INT_PTR & nPosX:坐标x
	* Param INT_PTR & nPosY:坐标y
	* Param INT_PTR nRange:长度
	* @Return bool:成功返回true，否则返回false
	*/
	bool 	GetMoveablePoint(CScene *pScene,INT_PTR &nPosX,INT_PTR &nPosY,INT_PTR nRange=1);

	/*
	* Comments:推迟移动时间
	* Param INT_PTR nMs:单位ms
	* @Return void:
	*/
	void PostponeMoveTime(INT_PTR nMs);
	

	/*
	* Comments:往身前或者身后冲锋,或者被击退
	* Param int nStep:步子，整数表示往前冲，负数表示往后冲
	* @Return bool:成功返回true,否则返回false
	*/
	bool RapidMove(int nStep);

	//向目标冲锋
	bool TargetRapidMove(int nTargetPosX, int nTargetPosY, int nMaxStep=0, int nSpeed=40);

	/*
	* Comments:将一个实体/击飞击退几个坐标
	* Param INT_PTR nDir:击退的方向
	* Param INT_PTR nStep:击退的步子
	* @Return bool:成功返回true，否则返回false
	*/
	bool BeatBack(INT_PTR nDir,INT_PTR nStep,bool isBeatback=true);
	

	//定时检查，主要是怪物寻路用的
	VOID OnTimeCheck(TICKCOUNT nTick);

	/*
	* Comments: 设置移动标记
	* Param INT_PTR nID: 标记的ID，例如mfTransport
	* Param bool flag:true 或者false
	* @Return void:
	*/
	inline void SetMoveFlag(INT_PTR nID,bool flag)
	{
		if(nID <0 || nID >= mfMaxMoveFlagID) return;
		/*
		if(flag )
		{
			OutputMsg(rmTip,_T("设置标记true"));
		}
		else
		{
			OutputMsg(rmTip,_T("设置标记false"));
		}
		*/
		m_moveFlag[nID] = flag ?1:0;
	}
	/*
	* Comments: 获取移动标记
	* Param INT_PTR nID: 标记的ID，例如mfTransport
	* @Return bool: 是返回true ,否则返回false
	*/
	inline bool GetMoveFlag(INT_PTR nID)
	{
		if(nID <0 || nID >= mfMaxMoveFlagID) return false;
		
		return m_moveFlag[nID]?true:false;
	}
	virtual  bool Initialize(void *data,SIZE_T size)
	{
		memset( &m_moveFlag,0,sizeof(m_moveFlag));
		return true;
	}
	
	//不移动
	void MoveIdle();

	//归位
	void MoveTargetedHome();

	//不回巢怪物
	void MoveNoReturnHome(INT_PTR nRadius =10,INT_PTR nMinSleepTime =4, INT_PTR nMaxSleepTime=10);

	
	/*
	* Comments: 能否跳跃到目标点
	* Param CScene * pScene:场景的指针
	* Param INT_PTR nPosX:作用点x
	* Param INT_PTR nPosY:作用点y
	* Param bool bFailSendTipmsg:如果失败是否发送tipmsg
	* @Return bool:如果能够跳过去返回true，否则返回false
	*/
	inline bool CanJumpTo(CScene * pScene, INT_PTR nPosX,INT_PTR nPosY,bool bFailSendTipmsg =false);
	

	/*
	* Comments: 跟随一个实体，跟随一般用于宠物，跟随着它的主人
	* Param CAnimal * pTarget: 目标的指针
	* Param INT_PTR nDistance:保持间隔的距离，单位是格子数量
	* Param unsigned int nMaxDist: 最大跟随间距。如果超过这个距离，停止跟随，并且通知被跟随者
	* Param bool bStopWhenFailed: 跟随失败后就停止跟随
	* @Return void:
	*/
	void MoveFollow(CAnimal* pTarget,INT_PTR nDistance =2, unsigned int nMaxDist = 50, bool bStopWhenFailed = false);


	/*
	* Comments: 已当前点为中心取一个巡逻半径，进行巡逻
		巡逻中间支持随机休息
	* Param INT_PTR nRadius: 巡逻的半径
	* Param bool bRandStop : 是否中间随机停止休息
	* Param INT_PTR nMinSleepTime: 最小的休息时间，单位秒
	* Param INT_PTR nMaxSleepTime: 最大的休息时间，单位秒
	* @Return void:
	*/
	void MovePatrol(INT_PTR nRadius =10,INT_PTR nMinSleepTime =4, INT_PTR nMaxSleepTime=10);

	/*
	* Comments: 追一个目标，主要用于怪物，追玩家
	* Param CAnimal * pTarget:目标的handle
	* Param INT_PTR nMinDistance: 和目标的距离小于这个距离将不再追逐
	* Param INT_PTR nMaxDistance: 和目标的距离大于这个距离将不再追逐
	* @Return void:
	*/
	void MoveChase(CAnimal * pTarget,INT_PTR nMinDistance= 2,INT_PTR nMaxDistance = 10);
	
	/*
	* Comments: 蛊惑一个目标，将乱跑
	* Param INT_PTR nSeconds: 蛊惑的时间
	* @Return void:
	*/
	void MoveConfused(INT_PTR nSeconds= 10);
	
	//逃跑，向一个目标相反的方向逃跑
	void MoveFleeing(CAnimal * pTarget);
	
	//往一个点移动
	void MovePoint(INT_PTR nPosX,INT_PTR nPosY);

	/*
	* Comments: 随机逃跑
	* Param INT_PTR nMaxDist: 单次逃跑最大逃跑距离
	* Param int nMinTime: 停留最短时间
	* Param int nMaxTime: 停留最长时间
	* Param bool bFleeImmediately: 是否立刻开始逃跑
	* @Return void:
	*/
	void MoveRandomFlee(INT_PTR nMaxDist, int nMinTime, int nMaxTime, bool bFleeImmediately = false);


	inline static INT_PTR GetDisSqare(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY,INT_PTR nTargetX, INT_PTR nTargetY)
	{
		nCurrentPosX -= nTargetX;
		nCurrentPosY -= nTargetY;
		return nCurrentPosX* nCurrentPosX + nCurrentPosY* nCurrentPosY;
	}




	//重置主角的位置
	inline void ResertActorPos(INT_PTR nPosx,INT_PTR nPosY);
	
	//获取当前的运动状态
	inline MotionType GetMotionType()
	{
		return (MotionType)m_motionStack.GetCurrentMotionState().bMotionType;
	}
	
	//去掉当前堆栈顶部的运动状态
	inline void PopCurrentMotionState()
	{
		m_motionStack.PopState();
	}

	inline void ClearMotionStack()
	{
		for (INT_PTR i = 0; i < MAX_MOTION_TYPE_COUNT; i++)
			m_motionStack.PopState();
	}

	/*
	* Comments:广播瞬间/传送移动到一个位置
	* Param INT_PTR nPosX: 目标点的x
	* Param INT_PTR nPosY: 目标点的y
	* Param INT_PTR nCmd:  网络消息命令码
	* @Return void:
	*/
	inline void BroadcastMove(INT_PTR nPosX, INT_PTR nPosY,INT_PTR nCmd,unsigned nType=0,unsigned nValue=0);
	
	/*
	* Comments: 清除移动栈
	* @Return void:
	*/
	inline void ClearMovement(){
		m_motionStack.Clear();
	}

	//设置是否使用A* 寻路算法
	inline void SetUseAstar(bool flag)
	{
		m_useAStar = flag;
	}

	/*
	* Comments:通知客户端自动寻路到npc
	* Param int nSceneId:npc所在的场景
	* Param char * sNpcName:npc名字
	* @Return int:成功返回0
	*/
	int AutoFindPathToNpc( int nSceneId, char * sNpcName);

	//获得方位与步子
	void GetCalculateMoveParam(INT_PTR nStartX,INT_PTR nStartY,INT_PTR nEndX, INT_PTR nEndY, INT_PTR & nDir,INT_PTR &nStep);

	//客户端需要知道飞鞋传送到那个npcid
	void SendHandFlyShoesRelust();

private:
	
	
	//处理
	void HandError(CDataPacketReader &packet){}

	//收到移动包，进行处理
	void HandMove(CDataPacketReader &packet);

	//收到移动包，进行处理
	void HandRun(CDataPacketReader &packet);

	//收到跳跃包，进行处理
	void HandJump(CDataPacketReader &packet);

	//收到转向包，进行处理
	void HandTurn(CDataPacketReader &packet);


	//下面这些函数是移动的函数，如果移动成功,那么返回true和下一个点的坐标
	//如果移动失败的话，将把这个移动的状态在堆栈删除掉
	// 往一点的移
	bool OnPointMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nNextPosY, INT_PTR &nDir,INT_PTR & nStep);

	//巡逻移动
	bool OnPatrolMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nNextPosY, INT_PTR &nDir,INT_PTR & nStep);

	//追逐目标
	bool OnChaseMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nNextPosY, INT_PTR &nDir,INT_PTR & nStep);

	//逃跑
	bool OnFleeMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nNextPosY, INT_PTR &nDir,INT_PTR & nStep);

	//跟随
	bool OnFollowMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nNextPosY, INT_PTR &nDir,INT_PTR & nStep);

	//迷惑
	bool OnConfusedMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nNextPosY, INT_PTR &nDir,INT_PTR & nStep);

	// 随机逃跑
	bool OnRandomFlee(MotionData &data, INT_PTR nCurrPosX, INT_PTR nCurrPosY, INT_PTR &nDir, INT_PTR &nStep);


	//计算移动的方向和步子
	inline bool CalculateMoveParam(INT_PTR nStartX,INT_PTR nStartY,INT_PTR nEndX, INT_PTR nEndY, INT_PTR & nDir,INT_PTR &nStep,INT_PTR nMinDisSquare=0,INT_PTR nMaxDisSquare =0);

	/*
	* Comments: 获取
	* Param INT_PTR nCurrentPosX:
	* Param INT_PTR nCurrentPosY:
	* @Return INT_PTR:
	*/
	//inline INT_PTR GetPosRandMovableDir(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY);

	//在一个扇形的区间里查找一个能用的运动点
	bool GetFanAvailablePos(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY,INT_PTR &nDir,INT_PTR &nStep);


	/*
	* Comments: 计算下一步可移动方向
	* Param INT_PTR nCurrentPosX: 当前位置X
	* Param INT_PTR nCurrentPosY: 当前位置X
	* Param INT_PTR & nDir: 当前方向。并且返回下一步可以移动的方向，优先选择当前移动方向	
	* Param INT_PTR nPrevPosX: 之前位置X
	* Param INT_PTR nPrevPosY: 之前位置X
	* @Return bool:成功返回true；失败返回false
	*/
	bool GetNexAvailablePos(INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir, INT_PTR nPrevPosX, INT_PTR nPrevPosY);

	//寻找路径，用于追逐的复杂情况使用
	bool FindWay(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY, INT_PTR nTargetPosX,INT_PTR nTargetPosY,INT_PTR &nDir,INT_PTR &nStep);
	
	//当一个移动结束的时候调用
	void OnMotionFinish(INT_PTR nMotionType);
	
	/* 
	* Comments: 获取下一个移动点
	* Param INT_PTR nCurretnPosX: 当前的移动点X
	* Param INT_PTR nCurrentPosY: 当前的移动点y
	* Param INT_PTR nDesPosX: 目标点x
	* Param INT_PTR nDesPosY: 目标点y
	* Param INT_PTR nServerchRadius: 查找的半径
	* @Return INT_PTR: 移动的方向
	*/
	//INT_PTR  GetNextMoveDir(INT_PTR nCurretnPosX, INT_PTR nCurrentPosY,INT_PTR nDesPosX,INT_PTR nDesPosY,INT_PTR nServerchRadius=6);
	
	//在90度空间里寻找能够走的方向
	//bool Get90AngleAvaibleDir(CScene *pScene,INT_PTR nCurretnPosX, INT_PTR nCurrentPosY,INT_PTR nDesPosX,INT_PTR nDesPosY ,INT_PTR &nDir,bool bOnlyCheckCrossPoint=false);
	
private:
	const static int MAX_MID_POINT_COUNT =32;
	CTimer<500> m_timer; //怪物移动计时用的
	CTimer<MOVECHECKTIMER> m_tmIdleLowPly;
	BYTE m_moveFlag[4] ; //在4个移动标记
	CMotionStack m_motionStack; //移动状态堆栈	

	
	bool m_useAStar;  //是否使用使用A*寻路，因为性能比较低下，需要使用
	CVector<SMALLCPOINT> m_searchNodes;  //寻路点，把这个记录下来，降低寻路的次数
	TICKCOUNT  m_canMoveTick; //能够运动的tick,用于野蛮冲撞中，正在野蛮中，将丢弃移动数据包

public:
	static CAdvanceAstar * s_pAStar;  //寻路的指针
	static bool s_boCheckIdleLowPly;	//是否检查超低级玩家无操作时踢下线
	static byte s_btMinIdlePlyLevel;	//小于该等级玩家踢下线
	int nFlyShoeNpcId;//npcid
};

