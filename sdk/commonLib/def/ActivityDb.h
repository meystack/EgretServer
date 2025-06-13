#pragma once

struct CMiniDateTime;

#pragma pack (push, 4)

struct GlobalActivityData
{
	int nId;			        // 活动ID
	int nActivityType;	        // 活动类型
	int nTimeType;		        // 时间类型
	CMiniDateTime nStartTime;	// 开始时间
	CMiniDateTime nEndTime;		// 结束时间
	bool nNextTime; //下一阶段时间
	GlobalActivityData()
	{
		memset(this, 0 ,sizeof(*this));
	}
};

struct PersonActivtyData
{
	int nId;					// 活动ID
	int nActivityType;			// 活动类型
	int nTimeType;		        // 时间类型
	CMiniDateTime nStartTime;	// 到期时间
	CMiniDateTime nExpiredTime;	// 到期时间（如果为0，说明已经结束）
	bool nNextTime; //下一阶段时间
	PersonActivtyData()
	{
		memset(this, 0 ,sizeof(*this));
	}
};

struct PersonStopedActivity
{
	int nId;					// 活动ID
	int nActivityType;			// 活动类型
};

#pragma pack(pop)
