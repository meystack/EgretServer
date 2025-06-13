#pragma once

/************************************************************************/
/*                           个人活动子系统
/*
/************************************************************************/
class CAnimal;
class CActivitySystem;
struct PersonActivtyData;

#include<map>


class PActivityAllocator: public std::allocator<std::pair<const int, PersonActivtyData> >
{
    typedef PersonActivtyData* pointer;
    pointer allocate(size_type _n, const void* = 0);
    void deallocate(pointer _p, size_type);
    void construct(pointer, const PersonActivtyData&) { }
    void destroy(pointer) { }
};

class CActivitySystem :
	public CEntitySubSystem<enActivityID,CBuffSystem,CAnimal>
{
public:

	enum EnActivityEvent{
        enOnLoad            = 1,            //活动加载                  [活动id]              注意：若为个人活动，传入 '玩家指针'
        enOnInit            = 2,            //活动初始化                [活动id, 玩家指针]    注意：用于处理再次开启的全局活动（同一活动id），重置初始化旧数据
        enOnStart           = 3,            //活动开始                  [活动id]              注意：若为个人活动，传入 '玩家指针'
        enOnEnd             = 4,            //活动结束                  [活动id]              注意：若为个人活动，传入 '玩家指针'
        enOnUpdate          = 5,            //活动帧更新                [活动id，当前时间]    注意：若为个人活动，传入 '玩家指针'
        enOnReqData         = 6,            //请求活动数据              [活动id, 玩家指针, outPackage]
        enOnOperator        = 7,            //通用操作                  [活动id，玩家指针，inPack]
        enOnEnterArea       = 8,            //进入活动区域              [活动id, 玩家指针]
        enOnExitArea        = 9,            //离开活动区域              [活动id, 玩家指针]
        enOnEnterFuben      = 10,           //玩家进入活动副本          [活动id，玩家指针，副本指针, pOwner]
        enOnExitFuben       = 11,           //玩家离开活动副本          [活动id，玩家指针，副本指针, pOwner]
        enOnEntityDeath     = 12,           //副本实体死亡              [活动id, 被杀者指针，击杀者指针，副本指针, pOwner]
        enOnEntityAttacked  = 13,           //活动副本实体受击          [活动id, 副本指针，受击者，攻击者, pOwner]
        enOnFubenFinish     = 14,           //活动副本结束              [活动id, 副本指针，结果, pOwner] 1为完成，0为失败，nil则结果未知（需要对应副本设置结果）
        enOnFubenAward      = 15,           //请求领取副本奖励          [活动id, 副本指针，pActor, pOwner]
        enOnGetRedPoint     = 16,           //请求红点数据              [活动id, 玩家指针]
        enOnLoginGame       = 17,           //活动与登录有关            [活动id，玩家指针] 个人活动登录签到
        enUpdateActivityData= 18,           //更新活动内部数据          [活动id，玩家指针, ...]
        enOnAtvAreaDeath    = 19,           //普通场景活动区域死亡      [活动id,死亡实体指针]
        enOnCombineSrv      = 20,           //合服后首次登陆            [活动id，合服后的开服天数差值,玩家指针] 个人活动
        enOnGPStart         = 21,           //全局个人活动的玩家Start   [活动id，玩家指针] Global_Person_Activity
        enOnGPEnd           = 22,           //全局个人活动的玩家End     [活动id，玩家指针] Global_Person_Activity
        enOnAtvAreaAtk      = 24,           //普通场景活动区域实体受击     [活动id, 副本指针，受击者，攻击者, pOwner]
        enOnAtvRank         = 25,           //副本排行榜刷新     [活动id, 当前时间]
    };

	typedef CEntitySubSystem<enActivityID,CBuffSystem,CAnimal> Inherited;
    friend class PActivityAllocator;
public:

    CActivitySystem();
    ~CActivitySystem();

public: // 继承接口

    //初始化的回调接口
	virtual bool Initialize(void *,SIZE_T);
    //DB返回的回调接口
    virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );
	//进入游戏
	virtual VOID OnEnterGame();
    //保存到db
	virtual VOID Save(PACTORDBDATA  pData);
    //处理协议
	virtual VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

public: // 功能接口

    VOID RunOne_5s(TICKCOUNT nCurrentTime);
    VOID OnLevelUp();
    VOID OnCircleUp();
    VOID OnChargeYuanBao();
    VOID OnNewDayArrive();
    VOID OnEnterScene();//进入场景
    VOID OnUserLogin(); //活动相关的登录事件
    VOID OnCombineServer(int diffDay) ; //合服事件
    //更新活动内部数据
    void UpdateActivityData(int nAtvId, int nParam1 = 0 , int nParma2 = 0, int nParam3 = 0);

    // 获取活动
    PersonActivtyData* GetActivity(int nAtvId) {
        if(m_RunningActivity.find(nAtvId) != m_RunningActivity.end())
            return &m_RunningActivity[nAtvId];
        return NULL;
    }
    // 活动是否进行中
    bool IsActivityRunning(int nId)
    {
        return m_RunningActivity.find(nId) != m_RunningActivity.end();
    }
    // 活动是否已停止
    bool IsActivityStop(int nId)
    {
        return m_StopActivity.find(nId) != m_StopActivity.end();
    }
    // 活动是否开过
    bool IsActivityHasBeenOpened(int nId)
    {
        return IsActivityStop(nId) || IsActivityRunning(nId);
    }
    void CloseActivity(int nAtvId);
    // 是否有在进行的该类型活动
    bool IsTypeRunning(int nAtvType);
    // 检查活动，若满足条件则开启
    bool CheckActivityBegin();
    // 发送一个活动数据
    void SendOneActivity(int nAtvId);
    //手动刷新运行中的活动数据
    void FlushRunningAtvData();


private: // 数据

    struct PActivityCmpter
    {
        bool operator()(const PersonActivtyData* _x, const PersonActivtyData* _y) const
        {
            return (unsigned int)(_x->nExpiredTime) < (unsigned int)(_y->nExpiredTime);
        }
    };

    typedef std::set<PersonActivtyData*,PActivityCmpter>::iterator RunIterator;
    typedef std::map<int, int>::iterator StopIterator;

    static 	CBufferAllocator*		m_pAllocator;

    bool                            m_IsInited; // 是否已从数据库加载数据
    
    std::map<int, PersonActivtyData,std::less<int>,PActivityAllocator> m_RunningActivity;  // <id,activity> 进行中的活动数据
    std::set<PersonActivtyData*,PActivityCmpter> m_RunningSeq;   // 排序以结束时间小的为优先
    std::map<int, int> m_StopActivity;  //<id,type> 已停止的活动，用以活动开启判断
    std::map<int, PersonActivtyData> m_Waitting; // 等待中的活动（只遍历这里面的）
};
