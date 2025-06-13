#pragma once

struct GlobalActivityData;

class CActivityComponent:
	public CComponent
{
public:

    CActivityComponent();
    ~CActivityComponent();

    // 请求DBServer加载数据
    VOID Load();
    // 推送数据到DBServer进行存储
    VOID Save();
    // 处理DBServer返回数据
    VOID OnDbReturnData(INT_PTR nCmd, char * data, SIZE_T size);
    // 玩家登录时
    VOID OnEnterScene(CActor* pActor);
   
    // 帧处理
    VOID RunOne(TICKCOUNT nTickCount);

public: //功能接口

    // 获取活动
    GlobalActivityData* GetActivity(int nAtvId) {
        if(m_RunningActivity.find(nAtvId) != m_RunningActivity.end())
            return &m_RunningActivity[nAtvId];
        return NULL;
    }

    // 获取等待活动
    GlobalActivityData* GetWaitingActivity(int nAtvId) {
        if(m_WaitingActivity.find(nAtvId) != m_WaitingActivity.end())
            return &m_WaitingActivity[nAtvId];
        return NULL;
    }
    // 添加活动（只加入等待列表，待RunOne中处理）
    bool AddActivity(PGLOBALACTIVITY pActivity);
    // 是否已从数据库加载数据
    bool IsInited() { return m_IsInited; }
    // 活动是否在进行中
    bool IsRunning(int nAtvId) { return m_RunningActivity.find(nAtvId) != m_RunningActivity.end(); }
    // 是否有在进行的该类型活动
    bool IsTypeRunning(int nAtvType);
    // 发送一个活动数据
    void SendOneActivity(CActor *pActor,int nAtvId);
    // 获取某活动的开启时间
    unsigned int GetStartTime(int nAtvId);
    // 获取某活动的结束时间
    unsigned int GetEndTime(int nAtvId);
    //結束活動
    void CloseActivity(int nAtvId, bool nNextTime = false);
    void UpdateActivityData(int nAtvId, CActor* pActor, int nParam1 = 0, int nParma2 = 0,int nParma3 = 0);
    VOID OnUserLogin(CActor* pActor) ;
    VOID CheckGPStartEnd(CActor* pActor,int atvId, unsigned int ScriptEndTime);

public: //继承接口

    virtual	bool  Initialize(){return true;}
	virtual VOID  Destroy(){}

private: //数据

    CTimer<1000>					m_1sTimer;
    CTimer<5000>					m_5sTimer;
    CTimer<3600000>					m_1hTimer;

    bool                            m_IsInited; // 是否已从数据库加载数据

    CTimer<3000>					m_2sSpecialTimer;//三秒刷跨服领主
    struct RunningActivityCmpter
    {
        bool operator()(const GlobalActivityData* _x, const GlobalActivityData* _y) const
        { return (unsigned int)(_x->nEndTime) == (unsigned int)(_y->nEndTime)? _x < _y : (unsigned int)(_x->nEndTime) < (unsigned int)(_y->nEndTime); }
    };

    struct WaitingActivityCmpter
    {
        bool operator()(const GlobalActivityData* _x, const GlobalActivityData* _y) const
        { return (unsigned int)(_x->nStartTime) == (unsigned int)(_y->nStartTime)? _x < _y : (unsigned int)(_x->nStartTime) < (unsigned int)(_y->nStartTime); }
    };


    std::map<int, GlobalActivityData> m_RunningActivity;                // <id,activity> 进行中的活动数据
    std::map<int, GlobalActivityData> m_WaitingActivity;                // <id,activity> 等待中的活动数据
    std::set<GlobalActivityData*,RunningActivityCmpter> m_RunningSeq;   // 排序以结束时间小的为优先
    std::set<GlobalActivityData*,WaitingActivityCmpter> m_WaitingSeq;   // 排序以开始时间小的为优先
    std::map<int, GlobalActivityData*> m_CircleActivityMap;             // <id,pActivity> 记录进行中的循环活动

    typedef std::set<GlobalActivityData*,RunningActivityCmpter>::iterator RunIterator;
    typedef std::set<GlobalActivityData*,WaitingActivityCmpter>::iterator WaitIterator;

    struct RunningNoticeCmpter {
        bool operator()(GLOBALNOTICE const* l, GLOBALNOTICE const* r) const noexcept
        {
            return (unsigned int)(l->nEndTime) == (unsigned int)(r->nEndTime)? l < r : (unsigned int)(l->nEndTime) < (unsigned int)(r->nEndTime);
        }
    };
    std::set<GLOBALNOTICE*, RunningNoticeCmpter> m_RunningNotice;

    struct WaittingNoticeCmpter {
        bool operator()(GLOBALNOTICE const* l, GLOBALNOTICE const* r) const noexcept
        {
            return (unsigned int)(l->nStartTime) == (unsigned int)(r->nStartTime)? l < r : (unsigned int)(l->nStartTime) < (unsigned int)(r->nStartTime);;
        }
    };
    std::set<GLOBALNOTICE*, WaittingNoticeCmpter> m_WaittingNotice;
};
