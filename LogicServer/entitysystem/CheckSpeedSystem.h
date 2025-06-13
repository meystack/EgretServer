#pragma once

#define SPEED_COUNT 10

//检测加速外挂的子系统

class CCheckSpeedSystem:
	public CEntitySubSystem<enCheckSpeedSystemID,CCheckSpeedSystem,CActor>
{
public:
	enum tagSpeedJudge
	{
		enErrorJudge,		//误判
		enIntervalSpeed,	//间隔性加速
		enConfirmSpeed,		//肯定在加速
	};
	enum tagUseSpeedToolHandle
	{
		enCloseActor,		//踢下线
		enBlackHouse,		//小黑屋
	};
	enum tagSpeedSystemOp
	{
		enKickToBlackHouse,//踢入黑屋
		enQuitBlackHouse,//退出黑屋
		enCleanBlackHousedata,//清理黑屋数据
	};

public:
	CCheckSpeedSystem();
	~CCheckSpeedSystem() {}

	virtual bool Initialize(void *data,SIZE_T size);

	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	//进入游戏处理
	void OnEnterGame();

	//服务器下发时间校验包
	void SendSpeedTimeVerify();

	//检测是否使用加速外挂
	void OnCheckUseSpeedTime(CDataPacketReader &packet);

	//检查加速
	void CheckUseSpeed(TICKCOUNT nCurrTime, INT_PTR nNextTime);

	//使用加速外挂处理
	void UseSpeedToolHandle(int nWayId = 0);

	void TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1=-1,INT_PTR nParam2=-1,INT_PTR nParam3=-1,INT_PTR nParam4=-1);
private:
	//检查加速
	int CheckSpeed(int nSpeedValue);
	//踢进小黑屋
	void KickToBlackHouse();
public:
	static int				m_IntevalTime;				//间隔时间 单位秒
	static int				m_DeviationTimes;			//校验加速的误差时间 单位毫秒
	static bool				m_bOpenVerifyFlag;			//是否开启检测加速外挂
	static int				m_useSpeedHandle;			//使用外挂之后的处理
	int						m_nCheckCount;				//检测次数
	TICKCOUNT				m_nClearCountTime;			//清空时间

	TICKCOUNT				m_SendVerifyTime;			//服务器下发校验包的时间

	int						m_SpeedValues[SPEED_COUNT]; //存储外挂加速值
	int						m_nCurPos;					//m_SpeedValues当前操作位置
	int						m_nOperCount;				//移动操作次数

	int						m_nCount;					//客户端发包次数					
	static int				m_nTimeRate;				//时间比率
	static int				m_nStartCheck;				//开始检测的次数
	static int				m_nSpeedCheck;				//加速检查值

	static int				m_nUpLimit;				//百分比上限参数
	static int				m_nLowLimit;			//百分比下限参数
	static int				m_nUpLimit2;			//百分比上限参数			
	static int				m_nLowLimit2;			//百分比下限参数

	static int				m_nRideTimes;			//上下坐骑多少次			
	static int				m_nIntervalTime;		//间隔这么久算1次
	static int				m_nClientTime;			//间隔这么久清除

	int						m_nOperateTimes;			// 操作次数
	TICKCOUNT				m_nAccTime;					// 累计操作时间

	TICKCOUNT			m_nLastNextTime;			//保存上次下一次执行的时间

};

