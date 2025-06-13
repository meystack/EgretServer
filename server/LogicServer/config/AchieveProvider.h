#pragma once

#define GROUP_ARCHIEVE_MAX  20           //单组成就最大数量

typedef struct tagAchieveAward
{
	WORD wID;   //ID
	BYTE bType; //类型
	BYTE bStrong; //强化
	
	WORD wReserver; //保留
	BYTE bQuality; //品质
	BYTE bBind;   //绑定
	unsigned int nCount; //数量

	int nSex; //性别
	int nVocation; //职业
}ACHIEVEAWARD,*PACHIEVEAWARD;

//成就的分组
typedef struct tagAchiveGroup
{
	int nLevel; //需要的等级
	int nCamp;  //需要的阵营
	int nSex;   //需要的性别
	int nVocation; //需要的职业
	int titleId;
	int mTitleNeedAchieves[GROUP_ARCHIEVE_MAX];   //
	tagAchiveGroup()
	{
		for(INT_PTR i=0; i< GROUP_ARCHIEVE_MAX; i++)
		{
			mTitleNeedAchieves[i] = -1;		//因为有成就id为0
		}
	}
}ACHIEVEGROUP,*PACHIEVEGROUP;


//成就的数据
typedef struct tagAchieveData
{
	char name[48];        //成就的名字
	WORD  nCondCount;      //成就完成需要几个成就的事件
	bool bNeedBroadcast;   //是否需要广播
	bool bIsDelete;       //是否已经删除了
	bool isDefaultActive;  //默认是激活的
	bool isDayRefresh;    //是否每天都清除数据，默认是false，就是不刷新，一些特殊的成就，比如活动需要每天刷新
	bool isMonthRefresh;	//是否每月都清除数据，默认是false，就是不刷新，先判断是否每日刷新，如果isDayRefresh=true，则无视此配置
	BYTE showPos;          //成就的显示的位置，0表示普通的成就界面,1表示充值奖励界面，2表示活动的活跃度界面
	int  groupId ;        //成就的分组   
	WORD  nBadgeId;			//徽章id
	DataList<ACHIEVECONDITION> conditions; //成就的各种条件
	DataList<ACHIEVEAWARD> awards; //成就的奖励
}ACHIEVEDATA,*PACHIEVEDATA;


//成就事件的条件
typedef struct tagAchieveEventCond
{
	WORD wReserve;  //
	BYTE bParamID;  //第几个参数
	BYTE bParamType; //参数的类型，1表示枚举，2表示范围
	DataList<int> params; //如果是枚举类型的话，是枚举类型的指针
}ACHIEVEEVENTCOND,*PACHIEVEEVENTCOND;

//称号的buff数据
typedef struct  tagTitleBuff
{
	BYTE bType;   //buff的类型
	BYTE bGroup;   //Buff的组
	WORD wReserver;
	float value;  //数值
}TITLEBUFF,*PTITLEBUFF;

//称号的配置
typedef struct tagTitleConfig
{
	char name[64]; //称号的名
	int nGroupId; //组的
	int nDurTime;  //有效时间
	DataList<TITLEBUFF> buffs; 
	bool isDelete;  //称号是否删除
	bool isFamous;  //是否是名人称号

	int nHeadTitleId;		//对应的头衔id
}TITLECONFIG,*PTITLECONFIG;
   
//头衔的配置
typedef struct tagTopTitleConfig
{
	int nRankId; //排行榜ID
	int nMinRank;  //排名范围最少值
	int nMaxRank;  //排名范围最大值
	int nTitleId;	//头衔ID 
	CStdItem::AttributeGroup	staticAttrs;	//静态属性表(同物品类似)
	tagTopTitleConfig()
	{
		memset(this,0,sizeof(*this)); //清空
	}
}TOPTITLECONFIG,*PTOPTITLECONFIG;

//成就的事件
typedef struct tagAchieveEvent
{
	WORD wAtomEventID;  //原子事件id
	BYTE bLogicOp;      // 是逻辑与还是逻辑或
	BYTE bPlusParamId; //进度的计数计数采用第几个参数，默认是0，那么一次事件进度+1，否则加第几个参数的数字
	//BYTE bAchieveID;      //成就的ID
	DataList<int> achieveIDList; //成就id的指针列表
	DataList<ACHIEVEEVENTCOND> conds;
	tagAchieveEvent()
	{
		memset(this,0,sizeof(*this)); //清空
	}
}ACHIEVEEVENT,*PACHIEVEEVENT;
//成就兑换配置
typedef struct tagAchieveBadge
{
	int		nId;//兑换id
	WORD	wItemId;//兑换的物品id
	//BYTE	btQuality;
	//BYTE	btStrong;
	//BYTE	btBindFlag;
	//WORD	wCount;  //数量溢出
	//int		nLeftTime; // 物品剩余时间。用于物品再添加的时候动态指定
	int		nAchievePoint;//兑换所需成就点
	tagAchieveBadge()
	{
		memset(this,0,sizeof(*this)); //清空
	}
}ACHIEVEBADGE,*PACHIEVEBADGE;

#define MAX_ACHIEVE_EVENT_ACHIEVE_COUNT 30  //每个成就事件最多有多少个成就对应

class CAchieveProvider :
	protected CCustomLogicLuaConfig
{
public:
	
	typedef CCustomLogicLuaConfig	Inherited;
	typedef CObjectAllocator<char>	CDataAllocator;

public:
	
	~CAchieveProvider();
	CAchieveProvider(); 

	/*
	* Comments:通过怪物ID返回怪物数据
	* Param const INT_PTR nId:怪物ID
	* @Return const MONSTERCONFIG*: 怪物的数据指针
	*/
	inline const PACHIEVEDATA GetAchieveData(const INT_PTR nId) const
	{
		//ID为0的
		if ( nId <0 || nId >= m_achieveData.count() )
			return NULL;
		else return &m_achieveData[nId];
	}

	//获取成就的事件
	inline const PACHIEVEEVENT GetAchieveEvent(const INT_PTR nId) const
	{
		//ID为0的
		if ( nId <0 || nId >= m_achieveEvents.count() )
			return NULL;
		else return &m_achieveEvents[nId];
	}
	
	/*
	* Comments:获取成就数量
	* @Return INT_PTR: 
	*/
	inline INT_PTR GetAchieveCount() const{ return m_achieveData.count(); }

	//获取称号的个数
	inline INT_PTR GetTitleCount() const {return m_titles.count();}
	
	inline const PTITLECONFIG GetTitle(const INT_PTR nId) const
	{
		//ID为0的
		if ( nId <0 || nId >= m_titles.count() )
			return NULL;
		else return &m_titles[nId];
	}

	inline CVector<TOPTITLECONFIG>& GetTopTitleList() 
	{
		return m_topTitles;
	}

	inline const PTOPTITLECONFIG GetTopTitle(INT_PTR nTitleId)
	{
		if (nTitleId < 0 || nTitleId > MAX_HEADTITLE_COUNT) 
			return NULL;

		for (INT_PTR i = 0; i < m_topTitles.count(); ++i)
		{
			if (m_topTitles[i].nTitleId == nTitleId)
			{
				return &m_topTitles[i];
			}
		}
		return NULL;
	}
	inline const PACHIEVEBADGE GetBadge(INT_PTR nId)
	{
		if (nId < 0 || nId >= m_BadgeData.count())
			return NULL;
		return &m_BadgeData[nId];
	}

	//计算头衔属性
	CAttrCalc& CalcHeadTitleAttributes(INT_PTR nTitleId, CAttrCalc &calc);

	/*
	* Comments: 从文件里装载怪物的数据
	* Param LPCTSTR sFilePath: 路径的名称
	* @Return bool: 失败返回false
	*/
	bool LoadAchieves(LPCTSTR sFilePath);

	//读取成就的事件
	bool LoadAchieveEvents(LPCTSTR sFilePath);

	//装载称号的配置
	bool LoadTitles(LPCSTR sFilePath);

	//装载头衔的配置
	bool LoadTopTitles(LPCSTR sFilePath);

	//装载成就的分组的数据
	bool LoadAchieveGroup(LPCSTR sFilePath);

	//装载徽章的数据
	bool LoadBadgeConfig(LPCSTR sFilePath);

	//获取成就事件在原子事件里的存储位置，如果没有就返回-1
	inline INT_PTR GetAchieveEventPos(INT_PTR  nAtomEventID,INT_PTR nAchieveEvent )
	{
		if(nAtomEventID >= m_atomEventVec.count() || nAtomEventID < 0)
		{
			return -1;
		}
		else
		{
			for(INT_PTR i=0; i< m_atomEventVec[nAtomEventID].count() ; i++)
			{
				if(m_atomEventVec[nAtomEventID][i] ==nAchieveEvent )
				{
					return i;
				}
			}
			return -1;
		}
		
	}
	/*
	* Comments: 获取原子事件有哪些成就事件在关注
	* Param INT_PTR nAtomEventID:原子事件的ID
	* @Return CVector<int>*:成就事件ID的指针
	*/
	inline CVector<int>* GetAtomEvents(INT_PTR nAtomEventID)
	{
		
		if(nAtomEventID <0 || nAtomEventID >= m_atomEventVec.count()) return NULL;
		return &m_atomEventVec[nAtomEventID];
	}

	//查找成就的分组的数据
	inline PACHIEVEGROUP GetAchiveGroup(INT_PTR nGroupId)
	{
		if(nGroupId <0 || nGroupId >= m_achieveGroup.count()) return NULL;
		return &m_achieveGroup[nGroupId];
	}

	inline INT_PTR GetAchiveGroupCount()
	{
		return m_achieveGroup.count();
	}

	inline INT_PTR GetAchieveBabgeCount()
	{
		return m_BadgeData.count();
	}

protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:
	
	//读取所有的成就的配置
	bool ReadAllAchieve();

	//读取成就的事件
	bool ReadAllAchieveEvent();

	//装载称号的数据
	bool ReadAllTitles();

	//装载头衔配置
	bool ReadAllTopTitle();

	//读取徽章的配置
	bool ReadBabgeConfig();

	/*
	* Comments: 读取一个怪物的数据
	* Param CDataAllocator & dataAllocator: 内存分配器
	* Param AICONFIG * oneAi:  ai指针
	* @Return bool:
	*/
	bool ReadOneAchieve(CDataAllocator &dataAllocator,ACHIEVEDATA *oneAchieve,INT_PTR nAchieveID);
		


	//读取一个成就事件
	bool ReadOneEvent(CDataAllocator &dataAllocator,ACHIEVEEVENT *pEvent,INT_PTR nAchieveEventID);

	//读取一个事件的条件
	bool ReadEventCond(CDataAllocator &dataAllocator,ACHIEVEEVENTCOND * pCond);


	//读取一个称号配置
	bool ReadOneTitle(CDataAllocator &dataAllocator,TITLECONFIG *pTitle);


	//读取一个头衔配置
	bool ReadOneTopTitle(CDataAllocator &dataAllocator,TOPTITLECONFIG *pTitle);

	//读取属性表
	bool ReadAttributeTable(CDataAllocator &dataAllocator, CStdItem::AttributeGroup &AttrGroup);

private:
	CVector<ACHIEVEDATA>   m_achieveData;     //成就的
	CVector<ACHIEVEEVENT>  m_achieveEvents;  // 成就的事件 
	CVector<CVector<int> >  m_atomEventVec;   //每个原子事件对应的成就事件
	CVector<TITLECONFIG>   m_titles;         //称号的配置
	CVector<TOPTITLECONFIG>	m_topTitles;	//头衔的配置
	
	CVector<ACHIEVEGROUP>  m_achieveGroup;   //成就的分组的数据

	CVector<ACHIEVEBADGE>			m_BadgeData;		//徽章的id,兑换列表


	CDataAllocator m_DataAllocator; //内存分配器
	CDataAllocator m_eventAllocator; //内存分配器
	CDataAllocator m_titleAllocator; //内存分配器
	CDataAllocator m_topTitleAllocator;	//内存分配器

};
