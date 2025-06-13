#pragma once
#include <vector>
#include <map>
//成就

typedef struct tagNewAchieveAward
{
	WORD wID;				//ID
	BYTE bType;				//类型
	BYTE bStrong;			//强化
	WORD wReserver;			//保留
	BYTE bQuality;			//品质
	BYTE bBind;				//绑定
	unsigned int nCount;	//数量
}NEWACHIEVEAWARD,*NEWPACHIEVEAWARD;

//星级的配置
typedef struct tagAchieveLevelConfig
{
	int				nId;			//成就id
	int				nCount;			//属性数量
	int				nPoint;			//升级功勋值
	int				nPrivilege;		//成就的特权
	int				nValue;			//特权的值
	int				nQId;			//关联任务id
	PGAMEATTR		PropList;		//加的基础属性
	NEWPACHIEVEAWARD awards;		//成就的奖励;		
	tagAchieveLevelConfig()
	{
		memset(this,0,sizeof(*this));
	}
}LEVELACHIECECONFIG;

//总配置
typedef struct tagAchieveConfig
{
	int nCount;										//一共多少级
	tagAchieveLevelConfig *nAchieve;				//各级的信息
	tagAchieveConfig()
	{
		memset(this,0,sizeof(*this));
	}
}ACHIEVECONFIG;

typedef struct tagAchieveAwardConfig
{
	int nId; //任务id
	int nType; //
	int nCount;//
	tagAchieveAwardConfig()
	{
		memset(this, 0, sizeof(*this));
	}
	
}ACHIEVEAWARDCONFIG;

typedef struct tagAchieveConditionConfig
{
	int nType; //任务id
	std::vector<int> nValue;
	int nLimitValue; //
	tagAchieveConditionConfig()
	{
		nType = 0;
		nLimitValue = 0;
		nValue.clear();
	}
	int GetConditionNum()
	{
		if(nType > 0)
			return nLimitValue;
		
		return nValue.size();
	}
}ACHIEVECONDITIONCONFIG;


typedef struct tagAchieveTaskConfig
{
	int nId; //任务id
	int nTab; //页签
	std::vector<ACHIEVEAWARDCONFIG> nAwards; //奖励
	ACHIEVECONDITIONCONFIG nConditionOne; //
	ACHIEVECONDITIONCONFIG nConditionTwo; //
	int nParameter1; //
	int nParameter2;// 
	int nLevel;//等级限制
	int nCircleLv;//转生限制
	int nOpenday;//开服天数
	int nPreachievement;//前置成就
	tagAchieveTaskConfig()
	{
		nId = 0;
		nTab = 0;
		nParameter1 = 0;
		nParameter2 = 0;
		nAwards.clear();
		nPreachievement = 0;
		nOpenday = 0;
		nCircleLv = 0;
	}
	int GetAchieveTaskConditionNum(int Id)
	{
		if(Id == 1)
			return nConditionOne.GetConditionNum();
		else
			return nConditionTwo.GetConditionNum();
		return 0;
	}

}ACHIEVETASKCONFIG;

typedef struct goods_tag
{
	int type; //
	int id; //
	int count;//
	goods_tag()
	{
		memset(this, 0, sizeof(*this));
	}
	
}GOODS_TAG;

typedef struct tagAchieveTypeCfg
{
	int nType;//类型
	int nSubType ;// 子类型
	int nValueType;//类型
	int nLog; //是否统计
	std::vector<int > activityId;
	int nWay;//途径
	tagAchieveTypeCfg()
	{
		nType = 0;
		nSubType = 0;
		nValueType = 0;
		nLog = 0;
		nWay = 0;
		activityId.clear();
	}

}ACHIEVETYPECFG;

typedef struct elem_MedalCfg{
	int level ;
	int need_popularity;
	int need_level ;
	int need_openday;
	int need_circle;
	std::vector<int> need_achieve_id_vec ;
	std::vector<GOODS_TAG> need_goods_vec ;
	int now_medal_id ;
	int next_medal_id;

} ELEM_MEDALCFG;



class CAchieve :
	protected CCustomLogicLuaConfig 
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;
public:
	CAchieve();
	~CAchieve();
	//加载配置
	bool LoadConfig(LPCTSTR sFilePath);
	bool readConfigs();
	bool readAchieveTaskConfigs();
	bool readAchieveTypeConfigs();
	bool readMedalConfigs();
	ACHIEVETASKCONFIG* GetAchieveTask(int nTaskId);
public:
	std::map<int, ACHIEVETASKCONFIG> m_AchieveTasks; //成就任务
	std::map<int, std::map<int, ACHIEVETYPECFG> > m_nAchieveTypes;               // 成就类型
	std::map<int, ELEM_MEDALCFG> m_MedalMap ;			//勋章配置
private:
	CDataAllocator		m_NewDataAllocator;		//对象申请器
};
