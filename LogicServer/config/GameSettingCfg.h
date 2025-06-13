#pragma once
//游戏设置
#include"GameSets.h"

class CGameSetting :
	protected CCustomLogicLuaConfig 
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;
public:
	CGameSetting();
	~CGameSetting();
	//加载配置
	bool LoadConfig(LPCTSTR sFilePath);
	bool readConfigs();
public:
	GAMESETSDATA2		m_DefaultGameSetting;
private:
	CDataAllocator		m_NewDataAllocator;		//对象申请器
};