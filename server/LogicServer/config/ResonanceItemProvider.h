#pragma once

typedef struct tagResonanceItemConfig
{
	WORD			nItemId;
	AttriGroup		attri;	//属性

	tagResonanceItemConfig()
	{
		memset(this,0,sizeof(*this));
	}
}RESONANCEITEMCONFIG;


class CResonanceItemProvider :protected CLuaConfigBase
{
public:
	typedef CLuaConfigBase	Inherited;
    typedef CObjectAllocator<char> CDataAllocator;

public:
	CResonanceItemProvider();
	~CResonanceItemProvider();

	/*
	*	nResonanceId : 共鸣ID
	*	nItemId		 ：道具ID
	*/
	AttriGroup* GetResonanceItemConfig(WORD nResonanceId, WORD nItemId)
	{
		if (m_ResonanceItems.find(nResonanceId) != m_ResonanceItems.end())
		{
			if (m_ResonanceItems[nResonanceId].find(nItemId) != m_ResonanceItems[nResonanceId].end())
			{
				return &m_ResonanceItems[nResonanceId][nItemId];
			}
		}
		return nullptr;
	}
public:
    bool Load(LPCTSTR sFilePath);
	bool ReadAllConfig();


private:
    CDataAllocator m_DataAllocator;
    std::map<WORD, std::map<WORD, AttriGroup> >m_ResonanceItems;
};