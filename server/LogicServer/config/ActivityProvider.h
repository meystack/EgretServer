#pragma once

enum ActivityTimeType //时间配置
{
    eActivityTimeType_NULL = 0,     
    eActivityTimeType_KFSJ = 0,    //开服时间  
    eActivityTimeType_GDSJ = 1,    //固定时间 
    eActivityTimeType_HFSJ = 2,    //合服时间 
    eActivityTimeType_XHSJ = 3,    //循环时间 
    eActivityTimeType_MAX, 
};
class CActivityProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig	Inherited;

	typedef std::map<int,GLOBALACTIVITY>::iterator GlobalActivityIterator;
	typedef std::map<int,PERSONACTIVITY>::iterator PersonActivityIterator;  
public:
	CActivityProvider();
	~CActivityProvider();

	bool LoadAll(LPCTSTR sFilePath);
	bool ReadAllActivities();

public:// 功能接口

	// 更新活动开启时间 nNextTime 是否开启下一个时间点的计算
	bool UpdateOpenTime(lua_State * pLuaState, GLOBALACTIVITY& activity, bool nNextTime = false );
	bool UpdateOpenTime(lua_State * pLuaState, PERSONACTIVITY& activity, bool nNextTime = false);
	// 检测该日期后不开，true为可开
	bool CheckOpenTimeLT(GLOBALACTIVITY& activity);
	bool CheckOpenTimeLT(PERSONACTIVITY& activity);
	// 检测该日期前不开，true为可开
	bool CheckOpenTimeGt(GLOBALACTIVITY& activity);
	bool CheckOpenTimeGt(PERSONACTIVITY& activity);
	// 检测该日期前合服开，true为可开
	bool CheckHefuTimeLt(GLOBALACTIVITY& activity);
	bool CheckHefuTimeLt(PERSONACTIVITY& activity);
	// 检测该日期后合服开，true为可开
	bool CheckHefuTimeGt(GLOBALACTIVITY& activity);
	bool CheckHefuTimeGt(PERSONACTIVITY& activity);
	// 检测第几次合服生效，true为可开
	bool CheckHefuTimes(GLOBALACTIVITY& activity);
	bool CheckHefuTimes(PERSONACTIVITY& activity);

	std::map<int,GLOBALACTIVITY>& GetGlobalActivities(){ return m_GlobalActivities; }
	std::map<int,PERSONACTIVITY>& GetPersonActivities(){ return m_PersonActivities; }
 
	std::vector<int>& GetActivityIdList(int nAtvType) { return m_ActType2IdMap[nAtvType]; }

	bool IsGlobalActivity(int nAtvId)
	{
		if (m_GlobalActivities.find(nAtvId) != m_GlobalActivities.end())
		{
			return true;
		}
		return false;
	}
	
	int getPActivityOpenDay(int nAtvId)
	{
		std::map<int, PERSONACTIVITY>::iterator it = m_PersonActivities.find(nAtvId);
		if (it != m_PersonActivities.end())
		{
			return it->second.nOpenSrvDate;
		}
		return 0;
	}
	bool IsPersonActivity(int nAtvId)
	{
		if (m_PersonActivities.find(nAtvId) != m_PersonActivities.end())
		{
			return true;
		}
		return false;
	}
	bool IsGlobalAtvType(int nAtvType)
	{
		if (m_ActType2IdMap.find(nAtvType)!=m_ActType2IdMap.end())
		{
			int atvId = m_ActType2IdMap[nAtvType].front();
			return IsGlobalActivity(atvId);
		}
		return false;
	}
	bool IsPersonAtvType(int nAtvType)
	{
		if (m_ActType2IdMap.find(nAtvType)!=m_ActType2IdMap.end())
		{
			int atvId = m_ActType2IdMap[nAtvType].front();
			return IsPersonActivity(atvId);
		}
		return false;
	} 
private: 
    static 	CBufferAllocator* m_pAllocator; 

	bool ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out);
	bool ReadFixedTimeValue(lua_State * pLuaState, LPCTSTR sKey, int nKeyLen, SYSTEMTIME& out);

	std::map<int,GLOBALACTIVITY> m_GlobalActivities;// 活动id：活动配置
	std::map<int,PERSONACTIVITY> m_PersonActivities;// 活动id：活动配置
	std::map<int,std::vector<int> > m_ActType2IdMap;// 活动类型到活动id列表的映射表
};
