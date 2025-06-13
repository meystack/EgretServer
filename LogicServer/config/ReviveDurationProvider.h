#pragma once

typedef struct tagOneReviveDurationConfig
{
	WORD			nReviveDurationId;
	int				nReviveDurationTime;
	char 			sTip[180];

	tagOneReviveDurationConfig()
	{
		memset(this,0,sizeof(*this));
	}
}OneReviveDurationConfig;

class CReviveDurationProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

public:
	CReviveDurationProvider();
	~CReviveDurationProvider();

public:
	bool LoadReviveDurationConfig(LPCTSTR sFilePath);
	OneReviveDurationConfig * GetReviveDurationConfig( WORD wId )const;
	bool IsTimeReviveDuration( WORD wId );

private:
	bool readConfigs();

protected:
	void showError(LPCTSTR sError);
private:
	CDataAllocator								m_DataAllocator;
	OneReviveDurationConfig 					*m_ReviveDurationsConfig;
	int											m_ReviveDurationsCount;
};

