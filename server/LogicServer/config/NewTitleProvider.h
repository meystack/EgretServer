#pragma once

typedef struct tagOneNewTitleConfig
{
	char name[64]; //称号的名
	bool			bBroadcast;	//是否广播
	bool			bAutoUse;
	WORD			nNewTitleId;
	int				nNewTitleTime;
	PGAMEATTR		PropList;
	PGAMEATTR		ConditionList;
	int				nPropCount;
	int				nConditionCount;
	int 			nVIPnotice;
	int 			nNoticeLv;
	int             nOfflieDelete;
	bool			bCrossServerHide; // 是否跨服隐藏

	tagOneNewTitleConfig()
	{
		memset(this,0,sizeof(*this));
	}
}OneNewTitleConfig;

class CNewTitleProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

public:
	CNewTitleProvider();
	~CNewTitleProvider();

public:
	bool LoadNewTitleConfig(LPCTSTR sFilePath);
	OneNewTitleConfig * GetNetTitleConfig( WORD wId )const;
	bool IsTimeNewTitle( WORD wId );
	OneNewTitleConfig * GetCustomTitleConfig( WORD wId )const;
	bool IsTimeCustomTitle( WORD wId );

private:
	bool readNewTitleConfigs();
	bool readmCustomTitleConfigs();

protected:
	void showError(LPCTSTR sError);
private:
	CDataAllocator			m_DataAllocator;
	OneNewTitleConfig*		m_NewTitlesConfig;
	int						m_NewTitlesCount;

	CDataAllocator			m_CustomTitlesDataAllocator;
	OneNewTitleConfig*		m_CustomTitlesConfig;
	int						m_CustomTitlesCount;
};

