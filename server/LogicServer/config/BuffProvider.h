#pragma once
#include<map>

class CBuffProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef struct tagBuff
	{
		WORD nId;
		WORD nType;
		WORD nGroup;
		char sName[127];
		double dValue;
		int nTimes;
		bool boTimeOverlay;
		bool boFullDel;
		bool boDieRemove;
		WORD nInterval;
		WORD nDuration;
		int nParam;
		int nParam2;
		int nParam3;
		int nEffectType;
		int nEffectId;
		int nColor;
		int nPropColor;
		byte btIcon;
		bool boSave;
		byte btDisplay;
		byte btDelEvent; // 删除的时候是否触发事件(1被动删除，2所有删除)
		bool boDelGiverByLeaveGuild;//是否退出行会的时候清除释放者
		int  nEffectTotalCount;
		int	 nEffectDirCount;
		int  nDebuff;	//1)若Debuff=1，则目标栏下面显示。
	}BUFFCONFIG, *PBUFFCONFIG;

	CBuffProvider(void);
	~CBuffProvider(void);

	//加载buff配置
	bool ReadBuffConfig();

	//加载
	bool Load(LPCTSTR sFilePath);

	CVector<PBUFFCONFIG> * GetBuffByteType(int ntype)
	{
		if(m_typeConfMap.find(ntype) != m_typeConfMap.end())
		{
			return &m_typeConfMap[ntype];
		}
		return NULL;
	}

	PBUFFCONFIG GetBuffBySkillId(int nskill)
	{
		if(m_SkillBuffMap.find(nskill) != m_SkillBuffMap.end())
		{
			return m_SkillBuffMap[nskill];
		}
		return NULL;
	}

	inline BUFFCONFIG * GetBuff(int nId)
	{
		if (nId <= 0 || nId > m_vectorBuff.count())
		{
			return NULL;
		}
		return &m_vectorBuff[nId-1];
	}

	//获取buff的数量
	inline INT_PTR GetBuffCount()
	{
		return m_vectorBuff.count();
	}
private:
	CVector<BUFFCONFIG> m_vectorBuff;
	std::map<int, CVector<PBUFFCONFIG> > m_typeConfMap;
	std::map<int, PBUFFCONFIG> m_SkillBuffMap;
};

