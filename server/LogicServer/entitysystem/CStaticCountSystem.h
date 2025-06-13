#pragma once
#include<map>


#define STATIC_TYPE_ITEM_INDEX_MAX 3000     //物品自定义计数器容量--程序内部使用

/*
玩家计数器
用于统计 计数的数据
*/
enum StaticType
{
    //参见道具配置表计数器说明
    eSTATIC_TYPE_DAILY_TIME        = 0,     //每日记录的时间 
    nDailyStaticStart              = 1,     //每日计数器起点   

    nDailyStaticBasicExp           = 301,   //每日基础经验统计           
    nDailyStaticEnd                = 401,   //每日计数器结点

    eSTATIC_TYPE_WEEK_TIME         = 450,   //每周记录的时间
    nWeekStaticStart               = 451,   //每周计数器起点
    nWeekStaticEnd                 = 600,   //每周计数器结点

    NLEFT_GUILDTIME                = 610,   //离开行会时间
    nLastCombineSrvMinTime         = 611,   //上次合服的绝对时间

    nContinueLoginMaxTimes         = 612,   //连续登陆最大天数

    nMedalLevel                    = 613,   //勋章等级  --我是不需要清理的

    nSZBOSS_STATICTYPE             = 614,   // 神装boss 次数
    nSZBOSS_STATICDAY              = 615,   // 神装boss 记录的开服时间
    nPERSONALBOSS_STATICTYPE       = 616,   // 个人boss 次数
    nPERSONALBOSS_STATICDAY        = 617,   // 个人boss 记录的开服时间

    nSTRENGTHRN_NUM_1              = 620,   //强化顺序

    nOrderWard_StaticType          = 622,   //战令积分
    nOrderWardId_StaticType        = 623,   //战令活动
    
    eSTATIC_TYPE_ORDERWARD_MONEY   = 624,   //战令币 

    nRechargTime_StaticType        = 625,   //充值时间 
    eSTATIC_TYPE_DIMENSIONAL_KEY     = 626,   //次元钥匙
    nDimensionalKey_STATICDAY      = 627,   //次元钥匙 记录的开服时间
    
    eSTATIC_TYPE_MONTH_TIME        = 650, //每月记录的时间
    nMonthStaticStart              = 651, //每月计数器起点
    nMonthStaticEnd                = 750, //每月计数器结点
    nForerverStaticStart           = 755, //商城永久购买
    nForerverStaticEnd             = 1000, //商城永久购买
    nPC_AWARD_STATE                = 10005,//微端奖励
    nMAIL_COMBINE_TIME             = 10006,//全服邮件
    nACTOR_MAIL_COMBINETIME        = 10007,//合服补偿邮件


    //程序内部使用的属性 
    //物品使用
    eSTATIC_TYPE_INNER_ITEM_DAILY_START = 30001,   //每日计数器起点--物品     
    eSTATIC_TYPE_INNER_ITEM_DAILY_END   = 33000,   //每日计数器结点--物品
  
    eSTATIC_TYPE_INNER_ITEM_WEEK_START  = 33001,   //每周计数器起点--物品
    eSTATIC_TYPE_INNER_ITEM_WEEK_END    = 36000,   //每周计数器结点--物品
     
    eSTATIC_TYPE_INNER_ITEM_MONTH_START = 36001,   //每月计数器起点--物品
    eSTATIC_TYPE_INNER_ITEM_MONTH_END   = 39000,   //每月计数器结点--物品 

    //其他
};


typedef struct StaticCount
{
    int nStaticType; //对应的计数器
    LONGLONG nCount;       //次数---后续如果需要int64 在修改
};
/*
玩家计数器系统 统计次数等计数器
*/
class CStaticCountSystem :public CEntitySubSystem<enStaticCountSystemID, CStaticCountSystem, CActor>
{
public:

    //初始化
    virtual bool Initialize(void* data, size_t size);
    void OnEnterGame();

    //处理db 数据返回
    virtual void OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet); 
    virtual void SaveToDb();


    //基础方法
    void DailyRefresh();//定时刷新
    LONGLONG GetStaticCount(INT_PTR nStaticType);//获得计数器value
    void SetStaticCount(INT_PTR nStaticType, INT_PTR nValue); 
    void AddStaticCount(INT_PTR nStaticType, INT_PTR nValue);


    //处理消息逻辑
    void ProcessNetData(INT_PTR cmd, CDataPacketReader& packet); 
     
    void SendCircleExchangeCount();//发送兑换次数
    //转生升级
    void UpCircle();

    //兑换修为
    void ExchangeCircleSoul(CDataPacketReader& packet);

    void UpMeridiansLv();

    /*****************************战令相关*************************************/
    void OnAddOrderWardMoney(INT_PTR nValue);
    /*****************************次元钥匙相关*************************************/
    void OnAddDimensionalKey(INT_PTR nValue);//eSTATIC_TYPE_DIMENSIONAL_KEY nDimensionalKey_StaticType
    
    /*****************************物品相关*************************************/
    //物品处理函数  重置状态为0 计数状态为大于等于1
    int UpItemInnerIndexDaily(int ItemIndex);//获得物品处理过的内部ID
    int UpItemInnerIndexWeek(int ItemIndex);//获得物品处理过的内部ID
    int UpItemInnerIndexMonth(int ItemIndex);//获得物品处理过的内部ID
    //改变
    void OnReduceItemInnerStaticCountDaily(INT_PTR nStaticType, INT_PTR nValue);
    void OnReduceItemInnerStaticCountWeek(INT_PTR nStaticType, INT_PTR nValue);
    void OnReduceItemInnerStaticCountMonth(INT_PTR nStaticType, INT_PTR nValue); 
    //获得是否是重置后的值
    bool OnGetItemInnerIndexDailyInit(int ItemIndex);//获得是否是初始状态
    bool OnGetItemInnerIndexWeekInit(int ItemIndex); 
    bool OnGetItemInnerIndexMonthInit(int ItemIndex); 
    //获得
    LONGLONG OnGetItemInnerIndexDaily(int ItemIndex);//获得是否是初始状态
    LONGLONG OnGetItemInnerIndexWeek(int ItemIndex); 
    LONGLONG OnGetItemInnerIndexMonth(int ItemIndex); 
protected:
    void ClearDailyStaticCount();//重置每日计数器 
    void ClearWeekStaticCount();//重置每周计数器 
    void ClearMonthStaticCount();//重置每月计数器 

private:
    std::map<int, LONGLONG> m_StaticCounts;
    bool isInitData = false;
    bool isDataModify = false;
};
 