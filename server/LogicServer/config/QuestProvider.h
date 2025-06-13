#pragma once
#include<algorithm>
struct ChangeQuestState
{
	int nQid; //任务id
	int nState;//状态
	ChangeQuestState()
	{
		memset(this, 0,sizeof(*this));
	}
};

struct TeleportInfo
{
	int nMapid; //地图id
	int nX;//x
	int nId;//
	int nY;//y
	int nRange;//范围
	int nIsOpen;//nIsOpen
	int nTelNpcId;//
	int nAuto;
	TeleportInfo()
	{
		memset(this, 0,sizeof(*this));
	}
};
struct QuestAward
{
	int nType;
	int nCount;
	int nId;
	QuestAward()
	{
		memset(this, 0, sizeof(*this));
	}
};

struct QuestCostFlyshoe
{
	int times;
	QuestAward costs;
	QuestCostFlyshoe()
	{
		memset(this, 0 , sizeof(this));
	}
};

struct QuestInfo
{
	int nType; //任务类型
	int nValue; //需求
	int nSubType; //子任务类型
	std::vector<int> nIds;//
	std::vector<int> nMaps;//
	QuestInfo(){
		nType = 0;
		nSubType = 0;
		nValue = 0;
		nIds.clear();
		nMaps.clear();
	}
};

typedef struct tagQuestInfoCfg
{
	int nQid; //任务id
	int nState;//

	std::vector<ChangeQuestState> button; //按钮改变
	std::vector<ChangeQuestState> nAuto; //自动改变
	std::vector<QuestCostFlyshoe> m_nCostFlyShoes;
	std::map<int ,std::vector<TeleportInfo> > m_nTelinfo; //传送
	// TeleportInfo nFlyShoeTel; //飞鞋传送
	// std::vector<TeleportInfo> m_nTeleportpos;//传送坐标
	// TeleportInfo nQuestTel;//任务传送
	std::vector<QuestAward> awards;//奖励
	QuestInfo mQuestInfo;//
	int nLevelLimit;//等级限制
	int nOpenDayLimit;// 开服天数
	int nCircle;//转生等级
	int nisFirst;//是否为初始任务
	int nIscomplete;//任务是否已完成
	int nAnnular;//是否为环形任务
	int nShowSchedule;//显示进度
	int nFrontQid;//前置任务
	int ntasktype;
	int nAcceptState;//
	std::vector<int> nGroups;//组
	tagQuestInfoCfg()
	{
		nQid = 0;
		nState = 0;
		m_nCostFlyShoes.clear();
		m_nTelinfo.clear();
		awards.clear();
		nLevelLimit = 0;
		nOpenDayLimit= 0;
		nCircle = 0;
		nisFirst = 0;
		nAnnular = 0;
		nIscomplete = 0;
		nShowSchedule = 0;
		ntasktype = 0;
		nAcceptState = 0;
		nGroups.clear();
	}

	void  GetCostFsValue(int nTimes, QuestAward& pCost)
	{
		int nCost = 0;
		int nNowCostTimes = 0;
		if(m_nCostFlyShoes.size() > 0)
		{	
			for(int i = 0; i < m_nCostFlyShoes.size(); i++)
			{
				if(nTimes > m_nCostFlyShoes[i].times && m_nCostFlyShoes[i].times > nNowCostTimes)
				{
					nCost = m_nCostFlyShoes[i].costs.nCount;
					nNowCostTimes = m_nCostFlyShoes[i].times;
					pCost = m_nCostFlyShoes[i].costs;
				}
			}
		}
	}

	bool checkIdMapidInLimitIds(int nId, int nSceneId)
	{
		bool nflag = false;
		if(mQuestInfo.nIds.size() == 0)
			nflag = true;
		if(!nflag)
		{
			if(mQuestInfo.nIds.size() > 0)
			{
				for(int i = 0; i < mQuestInfo.nIds.size(); i++ )
				{
					if(mQuestInfo.nIds[i] == nId)
						nflag = true;
				}
			}
		}
		
		if(!nflag)
			return nflag;
		
		nflag = false;
		if(mQuestInfo.nMaps.size() == 0)
			nflag = true;
		if(!nflag)
		{
			if(mQuestInfo.nMaps.size() > 0)
			{
				for(int i = 0; i < mQuestInfo.nMaps.size(); i++ )
				{
					if(mQuestInfo.nMaps[i] == nSceneId)
						nflag = true;
				}
			}
		}
		
		return nflag;		
	}

	int GetLimitValue()
	{
		return mQuestInfo.nValue;
	}
	int GeLimitType()
	{
		return mQuestInfo.nType;
	}

	int GeLimitSubType()
	{
		return mQuestInfo.nSubType;
	}

	bool inGroup(int nGroupId) {
		if(nGroups.size() == 0)
			return true;
		auto it = std::find(nGroups.begin(), nGroups.end(), nGroupId);
		if(it != nGroups.end())
			return true;
		return false;
	}

	TeleportInfo* GetTelInfo(int nType, int nId = 0)
	{
		
		std::map<int ,std::vector<TeleportInfo> >::iterator it = m_nTelinfo.find(nType);
		if(it != m_nTelinfo.end())
		{
			std::vector<TeleportInfo>& tels = it->second;
			if(tels.size() <= 0)
				return NULL;
			std::vector<TeleportInfo*> pTeles;
			pTeles.clear();
			for(int i = 0; i < tels.size(); i++)
			{
				if(tels[i].nId == nId)
					pTeles.push_back( &(tels[i]));
			}
			if(pTeles.size() > 0)
			{
				
				int nSize = pTeles.size();
				int randIndex = wrand(nSize) - 1 ;
				if (randIndex < 0)
					randIndex = 0;
				if(randIndex >= nSize)
					randIndex = nSize-1;
				return pTeles[randIndex];
			}
		}
		return NULL;
	}
}QuestInfoCfg;

class CQuestProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

	CQuestProvider();
	~CQuestProvider();
public:
	inline QuestInfoCfg* GetQuestConfig(INT_PTR qid, int nState)
	{ 
		allQuestIterator it = m_vAllQuestConfig.find(qid);
		if(it != m_vAllQuestConfig.end())
		{
			OneQuestIterator tt = it->second.find(nState);
			if(tt != it->second.end())
			{
				return &(tt->second);
			}
		}
		return NULL;
	}
	//每个任务开始状态的list
	std::map<int, QuestInfoCfg*>& GetAllQuestStartList()
	{
		return m_vMainRootQuestConfig;
	}
	//从文件加载标准任务配置
	bool LoadQuest(LPCTSTR sFilePath);

	void ReadTelePortInfo(QuestInfoCfg& item , int nType);

protected:
	//以下函数为覆盖父类的相关数据处理函数
	bool readItemData(QuestInfoCfg& item );
	bool ReadQuestItem();

private:
	CDataAllocator m_DataAllocator;	//物品属性对象申请器
public:
 
	std::map<int, std::map<int, QuestInfoCfg> > m_vAllQuestConfig;//所有任务
	typedef std::map<int, std::map<int, QuestInfoCfg> >::iterator allQuestIterator;
	typedef std::map<int, QuestInfoCfg>::iterator OneQuestIterator;
	std::map<int, QuestInfoCfg*> m_vMainRootQuestConfig;// 主任务
};
