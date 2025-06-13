#pragma once

//////////////////////////////////////////////////////////////////////////
// 后台资源加载模块，负责后台加载一些程序需要优先级不太高的配置和数据。

enum BackResLoadType
{
	enResLoad_Null,			// 无效类型
	enResLoad_Item,			// 加载物品配置
	enResLoad_Monster,		// 加载怪物配置
};

enum ResLoadResultType
{
	enResLR_succ = 1,		// 加载成功
	enResLR_fail,			// 加载失败
	enResLR_Unknown,		// 位置原因
};

class CResLoadTask
{
public:
	CResLoadTask() : m_resLoadType(enResLoad_Null){}	

	BackResLoadType	m_resLoadType;

	DECLARE_OBJECT_COUNTER(CResLoadTask)
};

class CBackResLoadModule : public wylib::thread::CBaseThread
{
public:
	CBackResLoadModule();
	~CBackResLoadModule();

	// 线程执行处理
	void OnRountine();

	// 开始工作线程
	void Start();

	// 停止工作线程
	void Stop();

	// 供逻辑工作线程调用的例行处理
	void RunOne(TICKCOUNT nCurrentTick);
		
	// 加载物品配置数据	
	void LoadItemConfig();
	void LoadConfig(BackResLoadType resLoadType);
	void LoadMonsterConfig();

protected:
	// 获取下一个资源加载任务
	CResLoadTask GetNextResLoadData();
	// 添加一个完成任务
	void AddFinishedTask(const CResLoadTask &task);


	// 加载物品资源
	bool LoadItemResImpl();
	// 加载怪物资源
	bool LoadMonsterResImpl();

private:
	enum 
	{
		IDLE_SLEEP_INTERVAL			= 500,					// 空闲时Sleep时间
		TASK_END_SLEEP_INTERVAL		= 10,					// 任务结束Sleep时间
	};
	CBaseLinkedList<CResLoadTask>		m_resLoadList;				// 资源加载列表	
	CCSLock								m_resLock;					// 添加资源列表访问锁
	CCSLock								m_finishedResLock;			// 完成资源列表访问锁
		
	CVector<CResLoadTask>				m_finishedList;				// 资源加载完成列表
	CVector<CResLoadTask>				m_finished2List;			// 资源加载完成列表
	CTimer<2000>						m_UpdateFinishListTimer;	// 更新完成列表定时器
#ifdef WIN32
	HANDLE								m_hStartEvent;				// 线程开始同步事件
	HANDLE								m_hStopEvent;				// 线程停止同步事件
#else
	sem_t								m_hStartEvent;              // 线程开始同步事件
	sem_t		                        m_hStopEvent;               // 线程停止同步事件
#endif
	volatile LONG						m_bThreadStarted;			// 线程启动标记
	volatile LONG						m_bThreadRuning;			// 线程启动的标记
};