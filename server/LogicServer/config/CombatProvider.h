#pragma once
//职业竞技
#define MAXCOMBATCOLUMN 16
#define MAXPICKITEMNUM	5

typedef struct tagCombatRankInfo
{
	char nRankName[32];			//排行榜名称
	char nFileName[32];			//排行榜文件名称
}COMBATRANKINFO,PCOMBATRANKINFO;


typedef struct tagCombatRank
{
	COMBATRANKINFO sRankInfo[enMaxVocCount];

	int  nShouCount;			//每页显示的数量
	int  nMaxRankNum;			//计入排名的数量
	int  nLevLimit;				//计入排行榜的最低等级
	int  nCircleLimit;			//计入排行榜的最低转数
	int  nMaxChallangeRecord;	//保存的竞技日志数
	int  nMaxChallageTimes;		//每日可以挑战的最大次数（改为每次挑战消耗的魔神令数）
	int  nMaxPickTimes;			//每日最大筛选次数
	int	 nMaxCD;				//达到禁止挑战最大cd
	int	 nAddCD;				//每次挑战增加cd


}COMBATRANK,*PCOMBATRANK;


typedef struct tagPickItem
{
	int nStartPickRank;
	int nEndPIckRank;
}PICKITEM, *PPICKITEM;

//筛选分组信息
typedef struct tagCombatRankGroup
{
	int nStartRank;				//排名的起始位
	int nEndRank;				//排名的最终位
	PICKITEM sPickitems[MAXPICKITEMNUM]; //选择项

	tagCombatRankGroup()
	{
		memset(this,0,sizeof(*this));
	}
}COMBATGROUP,*PCOMBATGROUP;


class CCombatRankProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

public:
	CCombatRankProvider();
	~CCombatRankProvider();

	//读取战力竞技配置文件
	bool LoadCombatConfig(LPCTSTR sFilePath);

protected:
	bool ReadCombatConfigs();

public:
	COMBATRANK				m_CombatRankConfig;		//战力竞技排行榜的基本信息

	COMBATGROUP				m_CombatGroupList[MAXCOMBATCOLUMN];		//筛选分组信息 CVector

	int						m_Count;

private:
	CDataAllocator			m_DataAllocator;	//对象申请器
};
