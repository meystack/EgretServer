#include "stdafx.h"
using namespace wylib::stream;

const CConfigLoader::DataProviderLoadTerm CConfigLoader::sLoader[] = 
{
	{&CConfigLoader::LoadSceneConfig, NULL,"scene config"},
	{&CConfigLoader::LoadAIConfig, _T("./data/robot/AITree.lua"),"AITree config"}//,

	//{&CConfigLoader::LoadItemConfig, _T("data/config/item/StdItem.txt"), "Item Config"}
};

bool CConfigLoader::LoadAllConfig()
{
	for (INT_PTR i = 0; i < ArrayCount(sLoader); i++)
	{
		if ( !((this->*sLoader[i].loadProc)(sLoader[i].sFilePath)) )
		{
			OutputMsg(rmError, _T("Error loading RunTimeData %s"), sLoader[i].comment);
			return false;
		}
	}
	return true;
}

bool CConfigLoader::LoadSceneConfig(LPCTSTR)
{
	return m_SceneConfigLoader.Load();
}

bool CConfigLoader::LoadAIConfig(LPCTSTR pConfig)
{
	return m_AITreeConfigLoader.Load(pConfig);
}

// bool CConfigLoader::LoadItemConfig( LPCTSTR sFilePath )
// {
// 	return m_itemConfigLoader.LoadFile(sFilePath);
// }
