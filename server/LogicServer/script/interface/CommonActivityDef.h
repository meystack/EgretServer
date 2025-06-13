#pragma once
//通用活动类型
//tolua_begin
enum CommonActivityId
{
	ActivityType_Normal		= 1,			//常规活动
	CommonActivity_MAX,
};
//tolua_end

#define ATIVITY_TIME_LEN 32

struct OneTimePair
{
	char strStartTime[ATIVITY_TIME_LEN];
    char strEndTime[ATIVITY_TIME_LEN];
};

//全局活动
typedef struct tagGlobalActivity
{
	int nId;			// 活动ID
	int nActivityType;	// 活动类型
	int nTimeType;		// 时间类型
	int nHefuTimes;		// 第几次合服生效
	int nAfterSrvDay;	// 开服N天后开
	int nBeforeSrvDay;	// 开服前N天开
	CMiniDateTime nStartTime;	// 开始时间
	CMiniDateTime nEndTime;		// 结束时间
	CMiniDateTime nRealTimeLt;	// 该日期前开（时间戳）
	CMiniDateTime nRealTimeGt;	// 该日期后开（时间戳）
	CMiniDateTime nHefuTimeLt;	// 该日期前合服不开（时间戳）
	CMiniDateTime nHefuTimeGt;	// 该日期后合服不开（时间戳）
	OneTimePair* pTimeDetail;	// 时间配置
	short nTimeCount;
	short nTimeIdx;
	short nPopLevel;	// 活动弹框等级
	BYTE nPopCircle;	// 活动弹框转生
	bool bPopupWhenStart; // 活动开启时弹框提示
	short nOpenLevel;	// 活动开启等级
	BYTE nOpenCircle;	// 活动开启转生
	BYTE nSupportPAtv;  // 是否拓展支持个人活动
} GLOBALACTIVITY,*PGLOBALACTIVITY;

//个人活动
typedef struct tagPersonActivity
{
	int nId;			// 活动ID
	int nActivityType;	// 活动类型
	int nLevel;			// 开启等级要求
	int nZSLevel;		// 开启转生要求
	int nOpenSrvDate;	// 开服天数要求
	int nDuration;		// 时长
	int nTimeType; 		//时间类型，1：创角时间  2:开服绝对时间
	bool bPopupWhenStart; // 活动开启时弹框提示
	int nHefuTimes;		// 第几次合服生效
	int nAfterSrvDay;	// 开服N天后开
	int nBeforeSrvDay;	// 开服前N天开
	CMiniDateTime nStartTime;	// 开始时间
	CMiniDateTime nEndTime;		// 结束时间
	CMiniDateTime nRealTimeLt;	// 该日期前开（时间戳）
	CMiniDateTime nRealTimeGt;	// 该日期后开（时间戳）
	CMiniDateTime nHefuTimeLt;	// 该日期前合服不开（时间戳）
	CMiniDateTime nHefuTimeGt;	// 该日期后合服不开（时间戳）
	OneTimePair* pTimeDetail;	// 时间配置
	short nTimeCount;
	short nTimeIdx;
} PERSONACTIVITY,*PPERSONACTIVITY;
