//成就的条件
#pragma once
#pragma pack (push, 1)

typedef struct tagAchieveCondition
{
	WORD wEventID;  //成就事件ID
	int  nCount;    //事件的数量
}ACHIEVECONDITION,*PACHIEVECONDITION;

//成就徽章
typedef struct tagAchieveBabge
{
	WORD wBabgeID;  //徽章ID
	int  nCount;    //完成的数量
}ACHIEVEBABGE,*PACHIEVEBABGE;


typedef struct tagOffLineAchieveData
{
    int nType;
    int nSubType;
    int nWay;
    int nValue;
    tagOffLineAchieveData()
    {
        memset(this, 0, sizeof(*this));
    }
}OffLineAchieveData;

#pragma pack(pop)