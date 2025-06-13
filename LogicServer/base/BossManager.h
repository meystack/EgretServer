#pragma once

#include <map>

typedef struct BossReferInfo
{
    int nBossId;      // bossId
    int nReferTime;   // 刷新时间
    int nDeath;       //是否死亡
    int nBossLv;      //boss等级
    int nReferX; //刷新的x坐标
    int nReferY; //刷新的y坐标
    int nReferRange; //刷新的范围
    int nReferMapId;// 地图id
    int nBuffAddValue;//属性加值百分比
    bool bIsPush = false;//是否已推送
    bool bNeedRefer = false;//是否需要刷新
    uint32_t nBelongId; //归属id
    BossReferInfo()
    {
        memset(this, 0, sizeof(*this));
    }
}BOSSREFERINFO;

class CBossManager:public CComponent
{
public:
    CBossManager();
    ~CBossManager();
    //初始化所有boss信息
    VOID Load();
    //保存怪物数据
    void Save();
    //初始化
    bool Initialize();
    bool IsInited() { return m_IsInited; }
    // 帧处理
    VOID RunOne(TICKCOUNT nTickCount);
    //处理boss数据返回
    void OnDbReturnData(INT_PTR nCmd, char * data, SIZE_T size);
    
    void SetReferBossBasicInfo(BOSSREFERINFO& boss);

public:
    //获取当前boss信息
    BOSSREFERINFO* GetBossInfo(int nBossId);
    //刷boss 
    void CreateBossEntity(CScene* pScene);
    void CreateBossEntity(BOSSREFERINFO& boss);
    //bosss死亡
    void BossDeath(int nBossId);

    bool CheckCondition(int nOpenCircle, int nOpenDay);//判断条件

    //依据bossid判断boss 是不是都已死亡 
    bool GetBossInfoIsDeath(int nBossId);
    //依据bossid 返回已死亡的boss最近刷新时间
    int GetBossMinReferTime(int nBossId);

    std::map<int32_t, BOSSREFERINFO>& GetBossList()
    {
        return m_nBossReferList;
    }

    CEntity* CreateBossEntity(BOSSREFERINFO& boss, REFRESHCONFIG& referCfg, CScene* pScene, int Index);
	virtual VOID  Destroy(){}

    //boss 触发；lua 脚本
    void BossCallLuaScript(int nBossId, int nType, CActor* pActor = NULL, CDataPacket* data = NULL);

private:
    struct RunningBossCmpter
    {
        bool operator()(const BOSSREFERINFO* _x, const BOSSREFERINFO* _y) const
        { return (unsigned int)(_x->nReferTime) <= (unsigned int)(_y->nReferTime); }
    };

    std::map<int32_t, BOSSREFERINFO> m_nBossReferList; //当前可刷新boss 
    std::map<int32_t, BOSSREFERINFO> m_nWaitBossReferList; //等待条件达成可刷新的boss

    std::map<int32_t, std::vector<BOSSREFERINFO*> > m_nMapBossList;//地图刷怪数据

    std::set<BOSSREFERINFO*,RunningBossCmpter> m_RunningReferList;   // 排序以结束时间小的为优先

    typedef std::map<int32_t, BOSSREFERINFO>::iterator BossIterator;
    typedef std::map<int32_t, std::vector<BOSSREFERINFO*> >::iterator MapBossIter;
    typedef std::set<BOSSREFERINFO*,RunningBossCmpter>::iterator  RunBossIter;

    CTimer<2000>					m_2sTimer;
    CTimer<1000>					m_1sTimer;
    bool                            m_IsInited;
};