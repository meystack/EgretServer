#pragma once

/************************************************************************/
/* 
/*                     游戏计分器模块，实现计分器的核心功能
/*  
/*   目前计分器的实现全部采用多通知对象的形式。通知对象已经实现队伍、帮会、阵营、副本等
/* 其中能够容纳多个玩家的集体对象的通知支持。向计分器中添加一个队伍，那么此队伍所有在线的
/* 玩家都会在计分器变更后收到通知数据。
/* 
/*   目前没有实现针对玩家为通知单位的计分器，因为暂时没有这种需求且实现起来有个小难点。即
/* 队伍、帮会、阵营、副本等对象作为通知对象，不受玩家下线的影响。而将玩家加入到通知列表，则
/* 一旦玩家下线后，肯定要将其从通知列表移除，从而导致上线后无法再重新取得计分器数据，因为
/* 他已经不是这个计分器的通知对象了。由于目前没有这种需求，所以暂时不解决此问题！
/*
/*   请注意：当一个玩家所在的队伍、帮会、阵营或副本都是同一个计分器的通知对象时，服务器会下
/* 发多遍关于创建此计分器的数据包，而且此计分器在客户端进行了创建判断，确保只会创建一个并且
/* 会通过引用计数来描述此计分器被关注了几次。当玩家离开副本、阵营、帮会或是队伍时，服务端会
/* 向其发送销毁计分器的消息，客户端会减少计分器的引用计数，直到计数为0才会销毁计分器，因此不
/* 必担心玩家所在的集体对象中有多个对象成为同一个计分器的通知对象而导致离开其中一个集体对象
/* 导致计分器在客户端被删除的问题。
/* 
/************************************************************************/

//分数记录结构
typedef struct tagScoreRecord
{
	int		nScore;		//分数
	TCHAR	boChanged;	//分数是否变化了
	TCHAR	sName[31];	//计分项名称

	inline tagScoreRecord(){}//构造函数用于消除编译警告
}SCORERECORD, *PSCORERECORD;


class CActor;
class CTeam;
class CGuild;
class CCamp;
class CFuBen;
class CCustomScoreRecorder;


/**************
 **
 **          计分器列表管理类  
 **
 *********************************************/
class CScoreRecorderManager :
	protected wylib::container::CBaseList<CCustomScoreRecorder*>
{
protected:
	typedef wylib::container::CBaseList<CCustomScoreRecorder*> Inherited;

public:
	CScoreRecorderManager();
	~CScoreRecorderManager();
	//向计分器列表中添加一个计分器
	INT_PTR Add(CCustomScoreRecorder *pRecorder);
	//清空计分器列表
	void Clear();
	//例行处理函数
	void Run(TICKCOUNT dwCurTick);
	//向玩家发送列表内的所有能够与之关联的计分器数据
	//玩家登录时需调用此函数
	void ActorLogin(CActor *pActor);
	//玩家加入计分器通知列表中的通知对象时，向此玩家发送计分器数据
	//在玩家加入队伍、加入帮会、加入阵营以及进入副本（以上不包含上线时）时需调用此函数
	void ActorJoinTarget(CActor *pActor, LPCVOID pTarget);
	//玩家离开计分器列表中的通知对象
	//在玩家离开队伍、玩家离开帮会、玩家离开阵营、玩家离开副本的时候需调用此函数
	void ActorLeaveTarget(CActor *pActor, LPCVOID pTarget);

private:
	TICKCOUNT	m_dwNextCheckChangeTick;	//下次检查所有变更过的计分器的时间
};



/*****
	计分器抽象类
	计分器不会在分数改变后立刻广播分数的变更，
	广播分数变更时在计分器所在的管理器中调用的，调用周期为2秒

	★★★该类的生存是完全由CScoreRecorderList管理的，
	【切勿调用delete释放此类以及子类】
	【将此类长期保存在其他成员中时，必须调用addRef函数以增加引用计数】
	【将此类从其他类的成员中移除时，必须调用release函数以减少引用计数】
*********************************************************************/
class CCustomScoreRecorder :
	public CBaseScriptExportObject,
	protected CCustomHashTable<SCORERECORD>
{
public:
	typedef CBaseScriptExportObject	Inherited;

public:
	CCustomScoreRecorder(LPCTSTR sName);
	~CCustomScoreRecorder();

	//覆盖父类的析构函数
	void destroy();
	//获取类名
	inline const char* getClassName() const { return CCustomScoreRecorder::szClassName; }

	//判断分数项是否存在
	bool itemExists(LPCTSTR sItemName) const;
	//获取分数项的分数
	INT getScore(LPCTSTR sItemName) const;
	//增加分数项的分数，如果boCreateable为True且分数是负数则减少分数
	INT incScore(LPCTSTR sItemName, INT nScoreToAdd, bool boCreateable);
	//设置分数项的分数，如果boCreateable为True且分数项不存在则创建分数项
	INT setScore(LPCTSTR sItemName, INT nScore, bool boCreateable);
	//广播计分器被创建
	void BoardCastScoreCreated();
	//广播分数变更
	void BoardCastScoreChanged();
	//停止计分器
	void stop();
	//向角色发送计分数据，当一个角色被关联到计分器中的通知对象的时候，
	//例如玩家登录时需要发送国家、军团、队伍等计分器的数据
	virtual void ActorLogin(CActor *pActor);
	//玩家加入计分器通知列表中的通知对象时，向此玩家发送计分器数据
	//在玩家加入队伍、加入帮会、加入阵营以及进入副本（以上不包含上线时）时需调用此函数
	virtual void ActorJoinTarget(CActor *pActor, LPCVOID pTarget);
	//玩家离开计分器列表中的通知对象
	//在玩家离开队伍、玩家离开帮会、玩家离开阵营、玩家离开副本的时候需调用此函数
	virtual void ActorLeaveTarget(CActor *pActor, LPCVOID pTarget);

	inline LPCTSTR getName() const{ return m_sName; }
	inline bool isChanged() const{ return m_boChanged; }
	inline bool isStoped() const{ return m_boStoped; }
	inline bool isMultiNotifyRecorder() const{ return m_boIsMultiNotifyRecorder; }
	inline const CCustomHashTable<SCORERECORD>& getScoreDataTable() const { return *this; }

protected:
	typedef CCustomHashTable<SCORERECORD> Inherited2;
	
protected:
	//向关心计分器数据变更的玩家发送数据
	virtual void SendBuffer(LPCVOID lpBuffer, size_t dwSize) = 0;
	//生成计分项数据变更的数据包到pack中，boAll表示是否生成所有项的数据，这在向玩家发送整个计分器的时候会用到。
	//为false则表示只生成自上次调用此函数依赖分数改变过的记录项数据
	size_t MakeScoreChangedBuffer(CDataPacket &pack, bool boAll = false);
	//生成下建整个计分器（含有计分项）的数据包
	size_t MakeScoreCreatedBuffer(CDataPacket &pack);
	//生成计分器停止的数据包
	size_t MakeScoreStopedBuffer(CDataPacket &pack);

protected:
	TCHAR		m_sName[32];	//计分器名称
	bool		m_boChanged;	//计分器的计分值是否已经变化
	bool		m_boStoped;		//计分器是否已经停止
	bool		m_boIsMultiNotifyRecorder;//是否是多通知类型的计分器（是否是TMultiObjectScoreRecorder及其子类）
	BYTE		m_btReserve;	//

private:
	static LPCTSTR szClassName;	//该类的导出类名
};

/*****
	多通知对象计分器抽象类
	T 必须实现一个void SendData(LPCVOID, SIZE_T)函数进行数据发送

*********************************************************************/
class CMultiNotifyScoreRecorder :
	public CCustomScoreRecorder
{
public:
	typedef CCustomScoreRecorder Inherited;

protected:
	/** 
	  计分器类型通知类型定义
	 **/
	enum eScoreRecorderNotifyType
	{
		srnNoTarget,
		srnActor,	//通知玩家
		srnTeam,	//通知队伍
		srnGuild,	//通知帮会
		srnCamp,	//通知阵营
		srnFuBen,	//通知副本
	};
	/**
	   计分器通知目标
	 **/
	struct NotifyTarget
	{
		eScoreRecorderNotifyType btType;
		union
		{
			void	*pTarget;
			CActor	*pAcotr;
			CTeam	*pTeam;
			CGuild	*pGuild;
			CCamp	*pCamp;
			CFuBen	*pFuBen;
		};
		//向通知目标发送数据
		void SendData(LPCVOID lpData, SIZE_T dwSize);
		//判断玩家是否属于此通知目标
		bool InTarget(CActor *pAcotr);
	};

public:
	CMultiNotifyScoreRecorder(LPCTSTR sName);

	//向角色发送计分数据，当一个角色被关联到计分器中的通知对象的时候，
	//实现玩家登录时需要发送国家、军团、队伍等计分器的数据
	void ActorLogin(CActor *pActor);
	//玩家加入计分器通知列表中的通知对象时，向此玩家发送计分器数据
	//在玩家加入队伍、加入帮会、加入阵营以及进入副本（以上不包含上线时）时需调用此函数
	void ActorJoinTarget(CActor *pActor, LPCVOID pTarget);
	//玩家离开计分器列表中的通知对象
	//在玩家离开队伍、玩家离开帮会、玩家离开阵营、玩家离开副本（以上不包含离线时）时需调用此函数
	void ActorLeaveTarget(CActor *pActor, LPCVOID pTarget);

	//获取类名
	inline const char* getClassName() const { return CMultiNotifyScoreRecorder::szClassName; }
	//添加一个队伍到通知列表中
	inline void AddTeam(CTeam *pTarget){ AddNotifyTarget(srnTeam, pTarget); }
	//添加一个帮会到通知列表中
	inline void AddGuild(CGuild *pTarget){ AddNotifyTarget(srnGuild, pTarget); }
	//添加一个阵营到通知列表中
	inline void AddCamp(CCamp *pTarget){ AddNotifyTarget(srnCamp, pTarget); }
	//添加一个副本到通知列表中
	inline void AddFuBen(CFuBen *pTarget){ AddNotifyTarget(srnFuBen, pTarget); }
	//从通知对象列表移除一个通知对象，对象不存在则返回false
	bool RemoveNotifyTarget(void *pTarget);

protected:
	//向关心计分器数据变更的玩家发送数据
	inline void SendBuffer(LPCVOID lpBuffer, size_t dwSize)
	{
		INT_PTR i;
		NotifyTarget *pTargetList = m_TargetList;

		for (i=m_TargetList.count()-1; i>-1; --i)
		{
			pTargetList[i].SendData(lpBuffer, dwSize);
		}
	}

	//添加一个通知对象，如果对象存在则不会重复添加
	INT_PTR AddNotifyTarget(eScoreRecorderNotifyType eType, void *pTarget);
	//获取通知对象在通知列表中的索引，不存在则返回-1
	INT_PTR GetNotifyTargetIndex(eScoreRecorderNotifyType eType, void *pTarget);
protected:
	wylib::container::CBaseList<NotifyTarget>	m_TargetList;//通知对象列表

private:
	static LPCTSTR szClassName;	//该类的导出类名
};
