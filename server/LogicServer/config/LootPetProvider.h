#pragma once


typedef struct tagOneLootPetConfig
{
	WORD			nLootPetId;
    WORD			nLootPetType;
	int				nContinueTime;
    bool			bAutoUse;
    int             nLootPetDistance;
	char 			sLootPetName[128];
	AttriGroup	attri;	//属性

	tagOneLootPetConfig()
	{
		memset(this,0,sizeof(*this));
	}
}ONELOOTPETCONFIG;

class CLootPetProvider :
	protected CLuaConfigBase
{
public:
	typedef CLuaConfigBase	Inherited;
    typedef CObjectAllocator<char> CDataAllocator;
public:
	CLootPetProvider();
	~CLootPetProvider();

public:
	ONELOOTPETCONFIG * GetLootPetConfig( WORD wId );

    bool Load(LPCTSTR sFilePath);
	bool ReadAllConfig();

private:
    CDataAllocator m_DataAllocator;
	std::map<int, ONELOOTPETCONFIG>	m_mLootPetConfs;
};
